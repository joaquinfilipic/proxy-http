#include "master_socket.h"
#include "clients.h"
#include "clients_manager.h"
#include "settings.h"

void init_master_socket(int *master_socket, struct sockaddr_in *address,
                        int *addrlen, settings *st) {
  int opt = TRUE;

  // create a master socket
  if ((*master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Setting flags...
  int flags = fcntl(*master_socket, F_GETFL);
  if (fcntl(*master_socket, F_SETFL, flags | O_NONBLOCK) == -1)
    perror("fcntl()");

  // set master socket to allow multiple connections , this is just a good
  // habit, it will work without this
  if (setsockopt(*master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                 sizeof(opt)) < 0) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // type of socket created
  address->sin_family = AF_INET;

  printf("-> master_socket listening on interface: %s\n", st->http_url);
  int value_to_ret = strcmp(st->http_url, "any");

  if (strcmp(st->http_url, "any") == 0)
    address->sin_addr.s_addr = INADDR_ANY;  // by default, it listens in all interfaces (read codagnone_manual)
  else if (strcmp(st->http_url, "broadcast") == 0)
    address->sin_addr.s_addr = INADDR_BROADCAST;
  else if (strcmp(st->http_url, "loopback") == 0)
    address->sin_addr.s_addr = INADDR_LOOPBACK;
  else
    address->sin_addr.s_addr = INADDR_ANY;  // by default, it listens in all interfaces (read codagnone_manual)

  address->sin_port = htons(st->local_port);

  // bind the socket to localhost port 8888
  if (bind(*master_socket, (struct sockaddr *)address, sizeof(*address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // try to specify maximum of 3 pending connections for the master socket
  if (listen(*master_socket, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  *addrlen = sizeof(address);
}

void set_fd_for_master_socket(const int *master_socket, int *max_fd,
                              fd_set *readfds) {
  FD_SET(*master_socket, readfds);
  if (*master_socket > *max_fd) *max_fd = *master_socket;
}

void resolve_master_client(int master_socket, fd_set *readfds, clients_list *cl,
                           struct sockaddr_in address, int addrlen,
                           settings *st) {
  // Is there a new client?
  if (FD_ISSET(master_socket, readfds)) {
    accept_new_client(cl, master_socket, address, addrlen, st);
  }
}
