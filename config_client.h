#ifndef CONFIG_CLIENT_H
#define CONFIG_CLIENT_H

#include <arpa/inet.h>
#include <getopt.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 128

typedef enum { NOT_AUTH, AUTH } ClientAuthState_t;

FILE*             fp;
int               sock;
int               auth_ok    = 0;
ClientAuthState_t auth_state = NOT_AUTH;

static void handle_signal(const int signum);

static void createMessageToSend(const char* pref, const char* msg);

static void sendAndReceive(const char* msg);

static void checkAuthState(const char* recv_buffer);

#endif