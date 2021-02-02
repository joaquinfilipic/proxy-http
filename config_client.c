#include "config_client.h"

int main(int argc, char **argv) {
  int  i, addr_count, option;
  int  counter = 0;
  int  asconf  = 1;
  char address[16];

  struct sockaddr_in          addr;
  struct sockaddr_in *        paddrs[5];
  struct sockaddr_in *        laddrs[5];
  struct sctp_event_subscribe events;
  struct sigaction            sig_handler;
  struct sctp_paddrparams     heartbeat;
  struct sctp_rtoinfo         rtoinfo;

  memset(&addr, 0, sizeof(struct sockaddr_in));
  memset(&events, 1, sizeof(struct sctp_event_subscribe));
  memset(&heartbeat, 0, sizeof(struct sctp_paddrparams));
  memset(&rtoinfo, 0, sizeof(struct sctp_rtoinfo));

  if (argc < 3 || (inet_addr(argv[1]) == INADDR_NONE)) {
    printf("Usage: ./client <proxy_address> <proxy_port> [OPTIONS]\n");
    printf("Option '-h' for help\n");
    return 0;
  }

  strncpy(address, argv[1], 15);
  address[15] = 0;

  addr.sin_family = AF_INET;
  inet_aton(address, &(addr.sin_addr));
  addr.sin_port = htons(atoi(argv[2]));

  heartbeat.spp_flags      = SPP_HB_ENABLE;
  heartbeat.spp_hbinterval = 5000;
  heartbeat.spp_pathmaxrxt = 1;

  rtoinfo.srto_max = 2000;

  sig_handler.sa_handler = handle_signal;
  sig_handler.sa_flags   = 0;

  /*Handle SIGINT in handle_signal Function*/
  if (sigaction(SIGINT, &sig_handler, NULL) == -1) {
    perror("sigaction");
  }

  /*Create the Socket*/
  if (((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP))) < 0) {
    perror("socket");
  }

  /*Configure Heartbeats*/
  if ((setsockopt(sock, SOL_SCTP, SCTP_PEER_ADDR_PARAMS, &heartbeat,
                  sizeof(heartbeat))) != 0) {
    perror("setsockopt");
  }

  /*Set rto_max*/
  if ((setsockopt(sock, SOL_SCTP, SCTP_RTOINFO, &rtoinfo, sizeof(rtoinfo))) !=
      0) {
    perror("setsockopt");
  }

  /*Enable SCTP Events*/
  if ((setsockopt(sock, SOL_SCTP, SCTP_EVENTS, (void *)&events,
                  sizeof(events))) != 0) {
    perror("setsockopt");
  }

  /*Get And Print Heartbeat Interval*/
  i = (sizeof heartbeat);
  getsockopt(sock, SOL_SCTP, SCTP_PEER_ADDR_PARAMS, &heartbeat,
             (socklen_t *)&i);

  printf("Heartbeat interval %d\n", heartbeat.spp_hbinterval);

  /*Connect to Host*/
  if (((connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)))) <
      0) {
    perror("connect");
    close(sock);
    exit(0);
  }

  /*Get Peer Addresses*/
  addr_count = sctp_getpaddrs(sock, 0, (struct sockaddr **)paddrs);
  printf("Peer addresses: %d\n", addr_count);

  /*Print Out Addresses*/
  for (i = 0; i < addr_count; i++) {
    printf("Address %d: %s:%d\n", i + 1, inet_ntoa((*paddrs)[i].sin_addr),
           (*paddrs)[i].sin_port);
  }

  sctp_freepaddrs((struct sockaddr *)*paddrs);

  //-----------------------------------------------------------
  // Authenticate
  char  password[BUFFER_SIZE];
  char  c;
  int   j = 0;
  FILE *file;
  file = fopen("./secretPassword.txt", "r");
  if (file) {
    while ((c = getc(file)) != EOF) {
      password[j++] = c;
      // putchar(c);
    }
    password[j] = '\0';
    fclose(file);
  }
  createMessageToSend("P ", password);

  while ((option = getopt(argc, argv, "hadM:rt:1234")) != -1 && auth_ok) {
    switch (option) {
      case 'h':
        printf("NAME\n");
        printf("        client - proxy configuration\n");
        printf("SYNOPSIS\n");
        printf("        ./client <proxy_address> <proxy_port> [OPTIONS]\n");
        printf("OPTIONS\n");
        printf("        -h\n");
        printf("        -a                  activate-transformations\n");
        printf("        -d                  deactivate-transformations\n");
        printf("        -M [media-types]    set-transformable-media-types\n");
        printf("        -r                  reset-transformable-media-types\n");
        printf("        -t [command]        set command\n");
        printf("        -1                  get-concurrent-connections\n");
        printf("        -2                  get-historical-accesses\n");
        printf("        -3                  get-clients-transferred-bytes\n");
        printf("        -4                  get-servers-transferred-bytes\n");
        printf("\n");
        exit(0);
        break;
      case 'a':
        sendAndReceive("A\r\n");
        break;
      case 'd':
        sendAndReceive("D\r\n");
        break;
      case 'M':
        createMessageToSend("T ", optarg);
        break;
      case 'r':
        sendAndReceive("R\r\n");
        break;
      case 't':
        createMessageToSend("C ", optarg);
      case '1':
        sendAndReceive("1\r\n");
        break;
      case '2':
        sendAndReceive("2\r\n");
        break;
      case '3':
        sendAndReceive("3\r\n");
        break;
      case '4':
        sendAndReceive("4\r\n");
        break;
      default:
        break;
    }
  }

  //------------------------------------------------------------
  sendAndReceive("Q\r\n");
  if (close(sock) != 0) {
    perror("close");
  }
}

static void handle_signal(const int signum) {
  switch (signum) {
    case SIGINT:
      if (close(sock) != 0) {
        perror("close");
      }
      exit(0);
      break;
    default:
      exit(0);
      break;
  }
}

static void createMessageToSend(const char *pref, const char *msg) {
  char buf[strlen(pref) + strlen(msg) + 2];
  strcpy(buf, pref);
  strcpy(buf + strlen(pref), msg);
  strcpy(buf + strlen(pref) + strlen(msg), "\r\n");
  sendAndReceive(buf);
}

static void sendAndReceive(const char *send_buffer) {
  int  msg_received = 0;
  char recv_buffer[BUFFER_SIZE];

  if ((send(sock, send_buffer, strlen(send_buffer), 0)) == -1) {
    perror("write");
  }

  printf("\nSent: %s", send_buffer);

  while (!msg_received) {
    int flags = 0;
    memset(recv_buffer, 0, sizeof(recv_buffer));
    sctp_recvmsg(sock, recv_buffer, BUFFER_SIZE, NULL, NULL, NULL, &flags);

    if (flags & MSG_NOTIFICATION) {
      ;
    } else {
      printf("Received: %s\n", recv_buffer);
      msg_received = 1;
      checkAuthState(recv_buffer);
    }
  }
}

static void checkAuthState(const char *recv_buffer) {
  char *msg_auth = "OK Authorized\r\n";

  if (auth_state == NOT_AUTH) {
    if (strcmp(recv_buffer, msg_auth) == 0) {
      auth_state = AUTH;
      auth_ok    = 1;
    }
  }
}
