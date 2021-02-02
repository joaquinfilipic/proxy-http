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

#include <limits.h>
#include <net/if.h>
#include <netinet/sctp.h>
#include <sys/ioctl.h>

#include "cbuffer.h"
#include "child_process.h"
#include "clients.h"
#include "clients_manager.h"
#include "config_socket.h"
#include "config_utilities.h"
#include "logger.h"
#include "main_options.h"
#include "master_socket.h"
#include "settings.h"
#include "to_origin_server.h"
#include "utilities.h"

#define TRUE 1
#define FALSE 0

void resolve(clients_list *cl, fd_set readfds, fd_set writefds, settings *st);
void handle_signal_action(int sig_number);
int  setup_signals(void);
void shutdown_properly(void);

clients_list *client_l;
int           master_s;
// FILE *        fptr;

static void wake_handler(int signal) { printf("Finished resolving host.\n"); }

int main(int argc, char *argv[]) {
  // Initializing struct with default values referred in the file http.8
  settings *st = NULL;
  st           = init_settings(st);

  // handling options in main arguments, also referref in the file http.8
  if (handle_main_options(argc, argv, st) < 0) exit(EXIT_FAILURE);

  // Initializing log's file
  // fptr = init_log();

  // Initializing list with connected clients
  clients_list *cl = NULL;
  cl               = init_client_list(cl);

  int                master_socket, addrlen, activity, conf_sock;
  int                max_sd = 0;
  struct sockaddr_in address;

  // Set of socket descriptors
  fd_set readfds, writefds;

  // Saving it for other methods.
  client_l = cl;

  if (setup_signals() != 0) exit(EXIT_FAILURE);

  init_master_socket(&master_socket, &address, &addrlen, st);

  init_config_socket(&conf_sock, st);

  init_config_options(conf_sock);

  // Saving it for other methods.
  master_s = master_socket;

  printf("\nListening for config clients on port:   %d\n", st->management_port);
  printf("Listening for http clients on port:     %d\n\n", st->local_port);

  struct sigaction act = {
      .sa_handler = wake_handler,
  };

  sigset_t emptyset, blockset;
  sigemptyset(&blockset);
  sigaddset(&blockset, SIGUSR1);
  sigaddset(&blockset, SIGINT);
  sigprocmask(SIG_BLOCK, &blockset, NULL);
  sigaction(SIGUSR1, &act, NULL);
  sigemptyset(&emptyset);

  while (TRUE) {
    // clear the socket set
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    max_sd = 0;

    set_fd_for_master_socket(&master_socket, &max_sd, &readfds);
    set_fd_for_master_socket(&conf_sock, &max_sd, &readfds);

    client *c                   = cl->head;
    int     client_disconnected = FALSE;
    while (c != NULL) {
      // If this failed, start the loop again.
      if (set_fd_for_client(c, cl, &max_sd, &readfds, &writefds, st) == -1) {
        client_disconnected = TRUE;
        break;
      }
      c = c->next;
    }
    // If the client disconnected, start the loop again so the FD are set from
    // zero.
    if (client_disconnected == TRUE) {
      continue;
    }

    // wait for an activity on one of the sockets , timeout is NULL , so wait
    // indefinitely
    activity = pselect(max_sd + 1, &readfds, &writefds, NULL, NULL, &emptyset);
    if (activity == -1 && errno != EINTR) {
      perror("select");
      exit(EXIT_FAILURE);
    }

    if (errno == EINTR) {
      // Caught the signal from the thread. Reset infinite loop.
      errno = 0;
      continue;
    }

    resolve_config_client(conf_sock, &readfds, st);
    resolve_master_client(master_socket, &readfds, cl, address, addrlen, st);
    resolve(cl, readfds, writefds, st);
  }
}

void resolve(clients_list *cl, fd_set readfds, fd_set writefds, settings *st) {
  client *c = cl->head;
  while (c != NULL) {
    resolve_client(c, cl, &readfds, &writefds, st);
    c = c->next;
  }
}

void handle_signal_action(int sig_number) {
  if (sig_number == SIGINT) {
    printf("SIGINT was catched!\n");
    shutdown_properly();
  } else if (sig_number == SIGPIPE) {
    printf("SIGPIPE was catched!\n");
    perror("pipe");
    shutdown_properly();
  } else if (sig_number == SIGTERM) {
    printf("SIGTERM was catched!\n");
    shutdown_properly();
  } else if (sig_number == SIGSEGV) {
    printf("SIGSEGV was catched!\n");
    shutdown_properly();
  }
}

int setup_signals(void) {
  struct sigaction sa;
  sa.sa_handler = handle_signal_action;
  if (sigaction(SIGINT, &sa, 0) != 0) {
    perror("sigaction()");
    shutdown_properly();
    return -1;
  }
  if (sigaction(SIGPIPE, &sa, 0) != 0) {
    perror("sigaction()");
    shutdown_properly();
    return -1;
  }
  if (sigaction(SIGTERM, &sa, 0) != 0) {
    perror("sigaction()");
    shutdown_properly();
    return -1;
  }
  if (sigaction(SIGSEGV, &sa, 0) != 0) {
    perror("sigaction()");
    shutdown_properly();
    return -1;
  }
  return 0;
}

void shutdown_properly(void) {
  printf("Program interrupted, disconnecting all %i client/s...\n",
         client_l->total);
  client *c = client_l->head;
  while (c != NULL) {
    client *next = c->next;
    printf("Disconnecting and removing client: %i.\n", c->fd_client);
    disconnect_client(client_l, c);
    c = next;
  }

  printf("Closing master socket...\n");
  close(master_s);

  // printf("Closing log file...\n");
  // close_log(fptr);

  exit(1);
}
