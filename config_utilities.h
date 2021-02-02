#ifndef CONFIG_UTILITIES_H
#define CONFIG_UTILITIES_H

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

#define CONF_BUFFER_SIZE 256
#define MTYPES_MAX_LEN 128

typedef enum { NOT_AUTH, AUTH, QUIT } ServerAuthState_t;
ServerAuthState_t auth_state;

void handle_config_socket(int conf_sock, settings *st);

void parseAndRespond(int conf_sock, const char *recv_buffer,
                     struct sockaddr_in *source_addr,
                     const socklen_t source_addr_len, settings *st);

int parseMediaTypes(const char *conf_recv_buffer, settings *st);

void freeMediaTypesArray(settings *st);

char *formStringFromMetric(int metric);

#endif