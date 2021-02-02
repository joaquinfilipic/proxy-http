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
#include "to_origin_server.h"

int connect_server(client *c) {
  // Create socket.
  c->fd_originserver = socket(AF_INET, SOCK_STREAM, 0);

  if (c->server_port == 0) {
    perror("Server ip/port not set.");
    return -1;
  }

  if (c->fd_originserver < 0) {
    perror("socket()");
    return -1;
  }

  // Set up address.
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(c->server_ip);
  server_addr.sin_port        = htons(c->server_port);

  c->addres = server_addr;

  // Setting flags to non-blocking...
  int flags = fcntl(c->fd_originserver, F_GETFL);
  if (fcntl(c->fd_originserver, F_SETFL, flags | O_NONBLOCK) == -1)
    perror("fcntl()");

  if (connect(c->fd_originserver, (struct sockaddr *)&server_addr,
              sizeof(struct sockaddr)) == -1) {
    if (errno ==
        EINPROGRESS) {  // Connection failed, but as expected. Waiting for
                        // select to find the successful connection.
      printf("Waiting for connection...\n");
      return 1;
    } else {  // Connection failed immediately.
      printf("Connection refused immediately.\n");
      return -1;
    }
  } else {  // Connection made immediately, just continue.
    printf("Connected immediately.\n");
    return 1;
  }
}
