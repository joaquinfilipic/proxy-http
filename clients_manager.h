#ifndef CLIENTS_MANAGER
#define CLIENTS_MANAGER

#define SRV_NAME_NOT_KNOWN (-2)
#define SRV_ERROR (-1)
#define SRV_RESOLVING 0
#define SRV_RESOLVED 1
#define SRV_CONNECTED 2

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //strlen
#include <sys/types.h>
#include <unistd.h>  //close

#include <arpa/inet.h>  //close
#include <netinet/in.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>  //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>

#include <netdb.h>
#include <signal.h>

#include <pthread.h>

#include "cbuffer.h"
#include "child_process.h"
#include "clients.h"
#include "logger.h"
#include "main_options.h"
#include "master_socket.h"
#include "settings.h"
#include "to_origin_server.h"
#include "utilities.h"

typedef struct {
  client *  c;
  pthread_t parent_id;
  char *    ip;
  int       port;
} thread_args;

/**
 * Sets the file descriptor maps acording to the clients needs.
 * Example: If the client wants to read, then sets the bit on.
 * @param c The client.
 * @param cl The client list.
 * @param max_fd A pointer to the max fd, overwrite with the new max, depending
 * on the client's fd.
 * @param readfds The read file descriptor map.
 * @param writefds The write file descriptor map.
 * @return returns 0 if all is OK, returns -1 if the client was disconnected.
 */
int set_fd_for_client(client *c, clients_list *cl, int *max_fd, fd_set *readfds,
                      fd_set *writefds, settings *st);

/**
 * Accepts a new client and adds him to the list.
 * @param cl Client list.
 * @param master_socket The master socket from which to get the new client.
 * @param address The address.
 * @param addrlen The adress length.
 */
void accept_new_client(clients_list *cl, int master_socket,
                       struct sockaddr_in address, int addrlen, settings *st);

/**
 * Placeholder used to resolve the servers ip from the host name.
 * This function runs on a different thread so it is not synced with the main
 * thread.
 * @param args The client to resolve.
 */
void *placeholder(void *args);
/**
 * Connects to server if nesessary.
 * @param c The client.
 * @return SRV_ERROR, SRV_RESOLVED, SRV_RESOLVING, SRV_CONNECTED
 */
int client_resolved_server(client *c);
/**
 * Reads the values set by the select. Reads from the buffers and writes to
 * other buffers.
 * @param c The client.
 * @param cl The client list.
 * @param readfds The read file descriptor map.
 * @param writefds The write file descriptor map.
 */
void resolve_client(client *c, clients_list *cl, fd_set *readfds,
                    fd_set *writefds, settings *st);

void send_503_to_client(client *c);

int parse_client(client *c, clients_list *cl);

int parse_server(client *c, clients_list *cl, settings *st);

#endif
