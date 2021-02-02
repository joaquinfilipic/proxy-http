#ifndef CLIENTS
#define CLIENTS

#define SRV_ERROR (-1)
#define SRV_RESOLVING 0
#define SRV_RESOLVED 1

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include "cbuffer.h"
#include "child_process.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //strlen
#include <unistd.h>  //close

#include <arpa/inet.h>  //close
#include <netinet/in.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>  //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include "HTTP_parser/parser.h"
#include "HTTP_parser/parser_definitions.h"

#include <netdb.h>

typedef struct client {
  int fd_client;

  struct buffer *request;
  struct buffer *transformed;
  struct buffer *response;

  struct buffer *client_read;
  struct buffer *client_write;
  struct buffer *server_read;
  struct buffer *server_write;

  request_parser_info * request_info;
  response_parser_info *response_info;

  message_type msg_type;

  int                fd_originserver;
  struct sockaddr_in addres;

  int   did_resolve;
  int   is_disconnecting;
  char *server_ip;
  int   server_port;

  struct client *prev;
  struct client *next;
  child_process *child_process;

  int     transforming;
  buffer *parse;
  buffer *inter_buffer;
} client;

typedef struct {
  int     total;
  client *head;
  client *last;
} clients_list;

clients_list *init_client_list(clients_list *cl);

/**
 * Creates a client and adds it to the list.
 * @param cl The client list.
 * @param fd The file descriptor.
 * @return The child struct if successful or NULL if not.
 */
client *new_client(clients_list *cl, int fd);

/**
 * Removes the client from the list and frees it's resources.
 * @param cl The client list.
 * @param fd The client's file descriptor.
 * @return The file descriptor if successful, -1 if not.
 */
int remove_client(clients_list *cl, int fd);

int new_client_2(clients_list *cl, client *c);
int remove_client_2(clients_list *cl, client *c);

/**
 * Disconnect from the client completly.
 * @param cl The client list.
 * @param c The client to disconnect.
 */
void disconnect_client(clients_list *cl, client *c);

int server_is_transforming(client *c, clients_list *cl, settings *st);

void print(clients_list *cl);
void print_client(client *c);

#endif
