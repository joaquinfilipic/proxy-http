#include "clients_manager.h"

struct addrinfo hints;

void send_503_to_client(client *c) {
  if (c->is_disconnecting != 0) return;

  char *r503 = response_503();
  printf("Try to disconnect!!!!\n");

  buffer_reset(c->server_write);
  while (*r503 != 0) {
    printf("%c", *r503);
    buffer_write(c->server_write, (uint8_t)*r503);
    r503++;
  }
  c->is_disconnecting = 1;
}

void *placeholder(void *args) {
  struct addrinfo *infoptr;
  thread_args *    t_args = (thread_args *)args;

  hints.ai_family = AF_INET;  // AF_INET means IPv4 only addresses

  int result             = getaddrinfo(t_args->ip, NULL, &hints, &infoptr);
  t_args->c->server_port = t_args->port;

  if (result != 0) {
    t_args->c->did_resolve = -2;
    // no se... creo que va acá
    pthread_kill(t_args->parent_id, SIGUSR1);
    pthread_exit(NULL);
  }

  struct addrinfo *p;

  for (p = infoptr; p != NULL; p = p->ai_next) {
    getnameinfo(p->ai_addr, p->ai_addrlen, t_args->c->server_ip, 256, NULL, 0,
                NI_NUMERICHOST);
  }

  t_args->c->did_resolve = 1;

  pthread_kill(t_args->parent_id, SIGUSR1);
  pthread_exit(0);
}

int parse_client(client *c, clients_list *cl) {
  int result = 0;
  int proxy_server = 0;
  if (buffer_can_read(c->client_read))
    result = parse_request(c->client_read, c->client_write, c->request_info);

  if (c->request_info->ready_to_connect == TRUE) {
    c->request_info->ready_to_connect = FALSE;
    pthread_t thread;

    thread_args *t_args = malloc(sizeof(thread_args));
    t_args->c           = c;
    t_args->parent_id   = pthread_self();
    t_args->ip          = c->request_info->host;
    t_args->port        = c->request_info->port;

    //Basic test
    if((strcmp(c->request_info->host, "127.0.0.1") == 0) &&
        c->request_info->port == 8080) {
      //Disconnect client.
      printf("Proxy as origin server\n" );
      proxy_server = 1;
    }

    if (pthread_create(&thread, NULL, placeholder, (void *)t_args) == -1) {
      perror("thread");
      exit(EXIT_FAILURE);
    }
  }

  if (result == -1 || proxy_server == 1) {
    disconnect_client(cl, c);
    return -1;
  }
  return 0;
}

int parse_server(client *c, clients_list *cl, settings *st) {
  int result = 0;
  if (c->transforming && st->transformations_activated) {
    result = parse_response(c->server_read, c->inter_buffer, c->response_info);
  } else {
    result = parse_response(c->server_read, c->server_write, c->response_info);
  }

  if (result == 1 && c->response_info->check_headers) {
    if (strcmp(c->request_info->msg->status->method, "HEAD") == 0) {
      c->response_info->railing = true;
    }

    // So that we never enter here twice.
    c->response_info->check_headers = false;

    char *header_ctype;
    header_ctype =
        search_header("Content-Type", c->response_info->msg->header_lines,
                      *(c->response_info->msg->header_count));
    printf("Header: %s\n", header_ctype);
    if (isMediaTypeInList(header_ctype, st)) {
      printf("The header is transformable.\n");
      if (server_is_transforming(c, cl, st) == -1) {
        disconnect_client(cl, c);
        return -1;
      }
    }

    char *content_encoding_header =
        search_header("Content-Encoding", c->response_info->msg->header_lines,
                      *(c->response_info->msg->header_count));

    if (content_encoding_header != null &&
        is_value_in_header("gzip", content_encoding_header)) {
      c->transforming = 0;
    }

    if (c->transforming && st->transformations_activated) {
      parse_response(c->server_read, c->inter_buffer, c->response_info);
    } else {
      parse_response(c->server_read, c->server_write, c->response_info);
    }
  }
  return 0;
}

int set_fd_for_client(client *c, clients_list *cl, int *max_fd, fd_set *readfds,
                      fd_set *writefds, settings *st) {
  // If the client is transforming and this is enabled in the settings, set the
  // fd for the child.
  if (c->transforming == 1 && st->transformations_activated) {
    set_fd_for_child(c->child_process, max_fd, readfds, writefds,
                     c->server_write, c->inter_buffer);
  }
  // This moves data from server_read to server_write when not transforming,
  // when transforming it moves from server_read to inter_buffer.
  if (parse_server(c, cl, st) == -1) return -1;

  // This moves data from client_read to client_write after parsing.
  if (parse_client(c, cl) == -1) return -1;

  // Highest file descriptor number, need it for the select function
  *max_fd = max(*max_fd, max(c->fd_client, c->fd_originserver));

  if (c->fd_client > 0) {

    // Did the client write to us?
    if (buffer_can_write(c->client_read)) {
      FD_SET(c->fd_client, readfds);
    }

    // Can we write to the client?
    if (buffer_can_read(c->server_write)) {
      FD_SET(c->fd_client, writefds);
    }

  }

  // Sometimes the proxy sends the client a message telling him there was an
  // error, here we check if such a message was sent, if so, disconnect.
  if (!buffer_can_read(c->server_write) && !buffer_can_read(c->server_read) &&
      c->is_disconnecting == 1) {
    // Disconnect client.
    disconnect_client(cl, c);
    st->concurrent_connections -= 1;
    return -1;
  }

  // Recheck the status of the client, can we connect to the server?
  switch (client_resolved_server(c)) {
    case SRV_RESOLVING:
      return 0;

    case SRV_RESOLVED: {
      // If the server is resolved, then we connect to it.
      if (connect_server(c) < 0) {
        disconnect_client(cl, c);
        st->concurrent_connections -= 1;
        return 0;
      }
      // Here calculate max fd again because connect modifies this when the
      // connection occurred.
      *max_fd = max(*max_fd, c->fd_originserver);
      break;
    }

    case SRV_CONNECTED:
      break;

    case SRV_NAME_NOT_KNOWN:

    case SRV_ERROR:

    default: {
      send_503_to_client(c);  // Here we must disconnect from the client.
      FD_SET(c->fd_client, writefds);  // Set the fd as write because we must
                                       // send the error message.
      return 0;  // Return 0 because we want to keep the connection until the
                 // error message is sent to the client.
    }

  }

  // Are we connected to the server?
  if (c->fd_originserver > 0 && c->server_port != 0) {

    // Did the server write to us?
    if (buffer_can_write(c->server_read)) {
      FD_SET(c->fd_originserver, readfds);
    }

    // Can we write to the server?
    if (buffer_can_read(c->client_write)) {
      FD_SET(c->fd_originserver, writefds);
    }

  }

  return 0;
}

int client_resolved_server(client *c) {
  if (c->did_resolve == 1) {
    if (c->fd_originserver <= 0) {
      return SRV_RESOLVED;
    } else {
      return SRV_CONNECTED;
    }
  } else if (c->did_resolve == -1) {
    return SRV_ERROR;
  } else if (c->did_resolve == -2) {
    return SRV_NAME_NOT_KNOWN;
  } else {
    return SRV_RESOLVING;
  }
}

void resolve_client(client *c, clients_list *cl, fd_set *readfds,
                    fd_set *writefds, settings *st) {
  int          request_read  = 0;
  int          response_read = 0;
  int          optval        = -1;
  unsigned int optlen        = sizeof(optval);

  // This moves data from client_read to client_write after parsing.
  if (parse_client(c, cl)) return;

  // This moves data from server_read to server_write when not transforming,
  // when transforming it moves from server_read to inter_buffer.
  if (parse_server(c, cl, st) == -1) return;

  // If the client is transforming and this is enabled in the settings, set the
  // fd for the child.
  if (c->transforming == 1 && st->transformations_activated) {
    resolve_child(c->child_process, readfds, writefds, c->server_write,
                  c->inter_buffer);
  }

  // Client wrote to the proxy.
  if (FD_ISSET(c->fd_client, readfds)) {
    request_read = doread(&c->fd_client, c->client_read);

    if (request_read == 0) {  // User wants to disconnet.
      disconnect_client(cl, c);
      st->concurrent_connections -= 1;
      return;
    }

    st->clients_transfered_bytes += request_read;
  }

  // Proxy can write to the client.
  if (FD_ISSET(c->fd_client, writefds)) {
    dowrite(&c->fd_client, c->server_write);
  }

  // If the client's server is not resolved yet, stop here or exit if necessary.
  switch (client_resolved_server(c)) {
    case SRV_RESOLVING:
      return;
    case SRV_RESOLVED:
      break;
    case SRV_CONNECTED:
      break;
    case SRV_NAME_NOT_KNOWN:
      break;
    case SRV_ERROR:
    default: {
      // Here we just disconnect the client, we don't want to risk the proxy so
      // no fancy error message.
      disconnect_client(cl, c);
      st->concurrent_connections -= 1;
      return;
    }
  }

  // Proxy can write to the server.
  if (FD_ISSET(c->fd_originserver, writefds)) {
    dowrite(&c->fd_originserver, c->client_write);
  }

  // Server wrote to the proxy.
  if (FD_ISSET(c->fd_originserver, readfds)) {

    // Checking if the connection to the server was successful.
    if (getsockopt(c->fd_originserver, SOL_SOCKET, SO_ERROR, &optval,
                   &optlen) == -1) {

      perror("getsockopt");  // Reading the status was unsuccessful, disconnect.
      disconnect_client(cl, c);  // Here we just disconnect the client, we don't want to risk
                                 // the proxy so no fancy error message.

      st->concurrent_connections -= 1;
      return;
    }
    if (optval != 0) {
      printf("Connecting error: %s\n",
             strerror(optval));  // The connection was bad, disconnect.
      disconnect_client(cl, c);  // Here we just disconnect the client, we don't want to risk
                                 // the proxy so no fancy error message.
      st->concurrent_connections -= 1;
      return;
    }

    response_read = doread(&c->fd_originserver, c->server_read);
    if (response_read == 0) {  // Server wants to disconnect.
      c->server_port = 0;
      send_503_to_client(c);  // Send the user a 503 error message, don't just disconnect.
    }
    st->servers_transfered_bytes += response_read;
  }
}

void accept_new_client(clients_list *cl, int master_socket,
                       struct sockaddr_in address, int addrlen, settings *st) {
  int new_socket;

  errno = 0;
  if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
  }

  client *c = new_client(cl, new_socket);
  if (c == NULL) {
    perror("creating client");
    close(new_socket);
    return;
  }

  st->concurrent_connections += 1;
  st->historical_accesses += 1;
}
