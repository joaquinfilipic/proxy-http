#ifndef PC_2018_02_MASTER_SOCKET_H
#define PC_2018_02_MASTER_SOCKET_H

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

#include <netdb.h>
#include "clients.h"
#include "settings.h"

#define TRUE 1
#define FALSE 0

void init_master_socket(int *master_socket, struct sockaddr_in *address,
                        int *addrlen, settings *st);
void set_fd_for_master_socket(const int *master_socket, int *max_fd,
                              fd_set *readfds);
void resolve_master_client(int master_socket, fd_set *readfds, clients_list *cl,
                           struct sockaddr_in address, int addrlen,
                           settings *st);

#endif  // PC_2018_02_MASTER_SOCKET_H
