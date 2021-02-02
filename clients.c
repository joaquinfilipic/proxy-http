#include <stdio.h>
#include <stdlib.h>

#include "clients.h"
#include "to_origin_server.h"
#include "utilities.h"

#define CLIENT_BUFFER_SIZE 2048

// struct addrinfo hints;p

int new_client_2(clients_list *cl, client *c) {
  c->next = NULL;
  c->prev = NULL;

  if (cl->total == 0) {
    cl->head = c;
    cl->last = c;
  } else {
    cl->last->next       = c;
    cl->last->next->prev = cl->last;
    cl->last             = c;
  }

  cl->total++;
  return c->fd_client;
}

client *new_client(clients_list *cl, int fd) {
  client *c    = malloc(sizeof(client));
  c->fd_client = fd;

  c->child_process = NULL;

  c->request      = initialize_buffer(CLIENT_BUFFER_SIZE);
  c->parse        = initialize_buffer(CLIENT_BUFFER_SIZE);
  c->response     = initialize_buffer(CLIENT_BUFFER_SIZE);
  c->transformed  = initialize_buffer(CLIENT_BUFFER_SIZE);
  c->inter_buffer = initialize_buffer(CLIENT_BUFFER_SIZE);

  c->client_read  = c->request;
  c->client_write = c->parse;
  c->server_read  = c->response;
  c->server_write = c->transformed;

  c->request_info  = initialize_request_info();
  c->response_info = initialize_response_info();

  c->msg_type = REQUEST;

  c->server_ip        = malloc(256 * sizeof(char));
  c->server_port      = 0;
  c->fd_originserver  = 0;
  c->did_resolve      = 0;
  c->is_disconnecting = 0;

  c->transforming = 0;

  new_client_2(cl, c);
  return c;
}

int server_is_transforming(client *c, clients_list *cl, settings *st) {
  c->child_process = malloc(sizeof(child_process));
  if (init_child(c->child_process, st) == ERROR_CREATING_CHILD) {
    perror("creating child");
    c->child_process = NULL;
    return -1;
  }

  c->transforming = 1;
  return 1;
}

int remove_client_2(clients_list *cl, client *c) {
  int fd = c->fd_client;

  if (c->fd_client == cl->head->fd_client) cl->head = c->next;

  if (c->fd_client == cl->last->fd_client) cl->last = c->prev;

  if (c->prev != NULL) c->prev->next = c->next;

  if (c->next != NULL) c->next->prev = c->prev;

  cl->total--;

  free(c->server_ip);
  free(c->response->data);
  free(c->request->data);
  free(c->transformed->data);
  free(c->parse->data);
  free(c->response);
  free(c->request);
  free(c->transformed);
  free(c->parse);

  if (c->child_process != NULL) free_child(c->child_process);

  free_request_parser(c->request_info);
  free_response_parser(c->response_info);
  free(c);

  return fd;
}

int remove_client(clients_list *cl, int fd) {
  int     it = 0;
  client *c  = cl->head;
  for (it = 0; it < cl->total; it++) {
    if (c->fd_client == fd) return remove_client_2(cl, c);

    c = c->next;
  }

  return -1;
}

void disconnect_client(clients_list *cl, client *c) {
  printf("Disconnected client fd: %d - server fd: %d.\n", c->fd_client,
         c->fd_originserver);
  if (c->fd_client != 0) close(c->fd_client);

  if (c->fd_originserver != 0) close(c->fd_originserver);

  remove_client(cl, c->fd_client);
}

void print(clients_list *cl) {
  int     it = 0;
  client *c  = cl->head;
  printf("Total de clientes: %d\n", cl->total);
  for (it = 0; it < cl->total; it++) {
    print_client(c);
    c = c->next;
  }
}

clients_list *init_client_list(clients_list *cl) {
  cl        = malloc(sizeof(clients_list));
  cl->head  = NULL;
  cl->last  = NULL;
  cl->total = 0;
  return cl;
}

void print_client(client *c) {
  printf("Cliente FD: %d - SOCKET: %d\n", c->fd_client, c->fd_originserver);
}
