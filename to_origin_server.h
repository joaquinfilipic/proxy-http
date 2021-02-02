#ifndef TOORIGINSERVER
#define TOORIGINSERVER

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

// SOCKETS
#include <sys/socket.h>
#include <sys/types.h>
// END SOCKETS

#include "clients.h"

int connect_server(client *c);

#endif
