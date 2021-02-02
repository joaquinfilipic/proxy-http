#ifndef CONFIG_SOCKET_H
#define CONFIG_SOCKET_H

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

#include <ctype.h>
#include <limits.h>
#include <net/if.h>
#include <netinet/sctp.h>
#include <sys/ioctl.h>

#include "settings.h"

void init_config_socket(int *conf_sock, settings *st);

void init_config_options(int conf_sock);

void set_fd_for_config_socket(const int *conf_sock, int *max_fd,
                              fd_set *readfds);

void resolve_config_client(int conf_sock, fd_set *readfds, settings *st);

#endif
