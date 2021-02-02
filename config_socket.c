#include "config_socket.h"
#include "config_utilities.h"
#include "settings.h"

void init_config_socket(int *conf_sock, settings *st) {
  int                conf_reuse = 1;
  struct sockaddr_in conf_addr;

  if ((*conf_sock = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&conf_addr, 0, sizeof(struct sockaddr_in));
  conf_addr.sin_family = AF_INET;

  // conf_addr.sin_addr.s_addr = htonl(INADDR_ANY); //original
  if (strcmp(st->management_url, "any") == 0)
    conf_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  else if (strcmp(st->management_url, "broadcast") == 0)
    conf_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  else if (strcmp(st->management_url, "loopback") == 0)
    conf_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  else
    conf_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  printf("-> config_socket listening on interface: %s\n\n", st->management_url);

  conf_addr.sin_port = htons(st->management_port);

  // Set the conf_reuse of Address
  if (setsockopt(*conf_sock, SOL_SOCKET, SO_REUSEADDR, &conf_reuse, sizeof(int)) < 0) {
    perror("setsockopt");
  }

  // Bind the Addresses
  if (bind(*conf_sock, (struct sockaddr *)&conf_addr, sizeof(struct sockaddr)) < 0) {
    perror("bind");
  }

  if (listen(*conf_sock, 2) < 0) {
    perror("listen");
  }
}

void init_config_options(int conf_sock) {
  int                         hb;
  int                         conf_addr_count = 0;
  struct sctp_event_subscribe event;
  struct sctp_paddrparams     heartbeat;
  struct sctp_rtoinfo         rtoinfo;
  struct sockaddr_in *        laddr[10];

  memset(&event, 1, sizeof(struct sctp_event_subscribe));
  memset(&heartbeat, 0, sizeof(struct sctp_paddrparams));
  memset(&rtoinfo, 0, sizeof(struct sctp_rtoinfo));

  heartbeat.spp_flags      = SPP_HB_ENABLE;
  heartbeat.spp_hbinterval = 5000;
  heartbeat.spp_pathmaxrxt = 1;

  rtoinfo.srto_max = 2000;

  // Set Heartbeats
  if (setsockopt(conf_sock, SOL_SCTP, SCTP_PEER_ADDR_PARAMS, &heartbeat,
                 sizeof(heartbeat)) != 0) {
    perror("setsockopt");
  }

  // Set rto_max
  if (setsockopt(conf_sock, SOL_SCTP, SCTP_RTOINFO, &rtoinfo,
                 sizeof(rtoinfo)) != 0) {
    perror("setsockopt");
  }

  // Set Events
  if (setsockopt(conf_sock, IPPROTO_SCTP, SCTP_EVENTS, &event,
                 sizeof(struct sctp_event_subscribe)) < 0) {
    perror("setsockopt");
  }

  // Get Heartbeat Value
  hb = (sizeof heartbeat);
  getsockopt(conf_sock, SOL_SCTP, SCTP_PEER_ADDR_PARAMS, &heartbeat,
             (socklen_t *)&hb);
  // printf("---CONF---Heartbeat interval %d\n", heartbeat.spp_hbinterval);

  // Print Locally Binded Addresses
  conf_addr_count = sctp_getladdrs(conf_sock, 0, (struct sockaddr **)laddr);
  printf("---CONF---Addresses binded: %d\n", conf_addr_count);
  int k;
  for (k = 0; k < conf_addr_count; k++) {
    printf("---CONF---Address %d: %s:%d\n", k + 1, inet_ntoa((*laddr)[k].sin_addr), (*laddr)[k].sin_port);
  }
  sctp_freeladdrs((struct sockaddr *)*laddr);

  auth_state = NOT_AUTH;
}

void set_fd_for_config_socket(const int *conf_sock, int *max_fd, fd_set *readfds) {
  FD_SET(*conf_sock, readfds);
  if (*conf_sock > *max_fd) {
    *max_fd = *conf_sock;
  }
}

void resolve_config_client(int conf_sock, fd_set *readfds, settings *st) {
  if (FD_ISSET(conf_sock, readfds)) {
    handle_config_socket(conf_sock, st);
  }
}
