#include "config_utilities.h"

void handle_config_socket(int conf_sock, settings *st) {
  int                conf_flags, assoc_id = 0;
  char               conf_recv_buffer[CONF_BUFFER_SIZE];
  struct sockaddr_in conf_source_addr;
  socklen_t          conf_source_addr_len = 16;

  conf_flags = 0;

  memset(conf_recv_buffer, '\0', CONF_BUFFER_SIZE);

  sctp_recvmsg(conf_sock, conf_recv_buffer, CONF_BUFFER_SIZE,
               (struct sockaddr *)&conf_source_addr, &conf_source_addr_len,
               NULL, &conf_flags);

  if (conf_flags & MSG_NOTIFICATION) {
    union sctp_notification *notification;
    notification = (union sctp_notification *)conf_recv_buffer;
    switch (notification->sn_header.sn_type) {
      case SCTP_SN_TYPE_BASE:
        // printf("SCTP_SN_TYPE_BASE\n");
        break;
      case SCTP_ASSOC_CHANGE:
        printf("---CONF---SCTP_ASSOC_CHANGE\n");
        printf("---CONF---Conf Client Address: %s\n",
               inet_ntoa(conf_source_addr.sin_addr));
               
        if (notification->sn_assoc_change.sac_state == SCTP_COMM_UP) {
          assoc_id = notification->sn_assoc_change.sac_assoc_id;
          printf("---CONF---New association. ID = %d\n", assoc_id);
          auth_state = NOT_AUTH;
        } else if (notification->sn_assoc_change.sac_state ==
                   SCTP_SHUTDOWN_COMP) {
          printf("---CONF---Association closed. ID = %d\n\n", assoc_id);
          auth_state = QUIT;
        }
        break;
      case SCTP_PEER_ADDR_CHANGE:
        // printf("SCTP_PEER_ADDR_CHANGE\n");
        break;
      case SCTP_SEND_FAILED:
        // printf("SCTP_SEND_FAILED\n");
        break;
      case SCTP_REMOTE_ERROR:
        // printf("SCTP_REMOTE_ERROR\n");
        break;
      case SCTP_SHUTDOWN_EVENT:
        // printf("SCTP_SHUTDOWN_EVENT\n");
        break;
      case SCTP_PARTIAL_DELIVERY_EVENT:
        // printf("SCTP_PARTIAL_DELIVERY_EVENT\n");
        break;
      case SCTP_ADAPTATION_INDICATION:
        // printf("SCTP_ADAPTATION_INDICATION\n");
        break;
      case SCTP_AUTHENTICATION_INDICATION:
        // printf("SCTP_AUTHENTICATION_INDICATION\n");
        break;
      case SCTP_SENDER_DRY_EVENT:
        // printf("SCTP_SENDER_DRY_EVENT\n");
        break;
      default:
        printf("%d\n", notification->sn_header.sn_type);
    }
  } else {
    parseAndRespond(conf_sock, conf_recv_buffer, &conf_source_addr,
                    conf_source_addr_len, st);
  }
}

void parseAndRespond(int conf_sock, const char *conf_recv_buffer,
                     struct sockaddr_in *conf_source_addr,
                     const socklen_t conf_source_addr_len, settings *st) {
  char *ok_quit         = "OK Quit\r\n";
  char *msg_not_auth    = "ERROR Not Authorized\r\n";
  char *msg_auth        = "OK Authorized\r\n";
  char *inc_pass        = "ERROR Incorrect Password\r\n";
  char *transf_act      = "OK Transformations Activated\r\n";
  char *transf_deact    = "OK Transformations Deactivated\r\n";
  char *transf_set      = "OK Transformations Set\r\n";
  char *transf_res      = "OK Transformations Reset\r\n";
  char *comm_set        = "OK Command Set\r\n";
  char *unk_com         = "ERROR Unknown Command\r\n";
  char *transf_not_set  = "ERROR Transformations Not Set\r\n";
  char *metrics_not_obt = "ERROR Metrics Not Obtained\r\n";

  switch (auth_state) {
    case NOT_AUTH:
      if (memcmp(conf_recv_buffer, "Q\r\n", 3) == 0) {
        auth_state = QUIT;
        sctp_sendmsg(conf_sock, ok_quit, strlen(ok_quit),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Quit\n");
      } else if (memcmp(conf_recv_buffer, "P ", 2) == 0) {
        int pass_len = strlen(st->pass);
        if (memcmp(st->pass, conf_recv_buffer + 2, pass_len) == 0 &&
            conf_recv_buffer[pass_len + 2] == '\r' &&
            conf_recv_buffer[pass_len + 3] == '\n') {
          auth_state = AUTH;
          sctp_sendmsg(conf_sock, msg_auth, strlen(msg_auth),
                       (struct sockaddr *)conf_source_addr,
                       conf_source_addr_len, 0, 0, 1, 0, 0);
          printf("---CONF---Authorized\n");
        } else {
          sctp_sendmsg(conf_sock, inc_pass, strlen(inc_pass),
                       (struct sockaddr *)conf_source_addr,
                       conf_source_addr_len, 0, 0, 1, 0, 0);
          printf("---CONF---Incorrect Password\n");
        }
      } else {
        sctp_sendmsg(conf_sock, msg_not_auth, strlen(msg_not_auth),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Not Authorized\n");
      }
      break;
    case AUTH:
      if (memcmp(conf_recv_buffer, "Q\r\n", 3) == 0) {
        auth_state = QUIT;
        sctp_sendmsg(conf_sock, ok_quit, strlen(ok_quit),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Quit\n");
      } else if (memcmp(conf_recv_buffer, "A\r\n", 3) == 0) {
        st->transformations_activated = 1;
        sctp_sendmsg(conf_sock, transf_act, strlen(transf_act),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Transformations Activated\n");
      } else if (memcmp(conf_recv_buffer, "D\r\n", 3) == 0) {
        st->transformations_activated = 0;
        sctp_sendmsg(conf_sock, transf_deact, strlen(transf_deact),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Transformations Deactivated\n");
      } else if (memcmp(conf_recv_buffer, "T ", 2) == 0) {
        freeMediaTypesArray(st);
        st->media_types_amount = parseMediaTypes(conf_recv_buffer + 2, st);
        if (st->media_types_amount == 0) {
          sctp_sendmsg(conf_sock, transf_not_set, strlen(transf_not_set),
                       (struct sockaddr *)conf_source_addr,
                       conf_source_addr_len, 0, 0, 1, 0, 0);
          printf("---CONF---Transformations Not Set\n");
        } else {
          printf("   Media Types:\n");
          int i;
          for (i = 0; i < st->media_types_amount; i++) {
            printf("   ->%s\n", st->media_types_array[i]);
          }
          sctp_sendmsg(conf_sock, transf_set, strlen(transf_set),
                       (struct sockaddr *)conf_source_addr,
                       conf_source_addr_len, 0, 0, 1, 0, 0);
          printf("---CONF---Transformations Set\n");
        }
      } else if (memcmp(conf_recv_buffer, "R\r\n", 3) == 0) {
        freeMediaTypesArray(st);
        st->media_types_amount = 0;
        sctp_sendmsg(conf_sock, transf_res, strlen(transf_res),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Transformations Reset\n");
      } else if (memcmp(conf_recv_buffer, "C ", 2) == 0) {
        // substract 'C ' and '\r\n', and add final '\0'
        int   arg_len = strlen(conf_recv_buffer) - 3;
        char *command = malloc(arg_len);
        memcpy(command, conf_recv_buffer + 2, strlen(conf_recv_buffer) - 4);
        command[arg_len] = '\0';
        st->cmd          = command;
        sctp_sendmsg(conf_sock, comm_set, strlen(comm_set),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Command Set\n");
        printf("   ->%s\n", st->cmd);
      } else if (memcmp(conf_recv_buffer, "1\r\n", 3) == 0) {
        char *msg_cc = formStringFromMetric(st->concurrent_connections);
        sctp_sendmsg(conf_sock, msg_cc, strlen(msg_cc),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Concurrent Connections\n");
      } else if (memcmp(conf_recv_buffer, "2\r\n", 3) == 0) {
        char *msg_ha = formStringFromMetric(st->historical_accesses);
        sctp_sendmsg(conf_sock, msg_ha, strlen(msg_ha),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Historical Accesses\n");
      } else if (memcmp(conf_recv_buffer, "3\r\n", 3) == 0) {
        char *msg_ctb = formStringFromMetric(st->clients_transfered_bytes);
        sctp_sendmsg(conf_sock, msg_ctb, strlen(msg_ctb),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Transferred Clients Bytes\n");
      } else if (memcmp(conf_recv_buffer, "4\r\n", 3) == 0) {
        char *msg_stb = formStringFromMetric(st->servers_transfered_bytes);
        sctp_sendmsg(conf_sock, msg_stb, strlen(msg_stb),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Transferred Servers Bytes\n");
      } else {
        sctp_sendmsg(conf_sock, unk_com, strlen(unk_com),
                     (struct sockaddr *)conf_source_addr, conf_source_addr_len,
                     0, 0, 1, 0, 0);
        printf("---CONF---Unknown Command\n");
      }
      break;
    case QUIT:
      break;
    default:;
  }
}

int parseMediaTypes(const char *conf_recv_buffer, settings *st) {
  int  buf_position_index   = 0;
  int  buf_media_type_index = 0;
  int  mtypes_amount        = 0;
  int  list_len             = 0;
  char c;

  while ((c = conf_recv_buffer[buf_position_index + buf_media_type_index]) &&
         list_len <= MTYPES_MAX_LEN && c != EOF) {
    if (c == ',' ||
        (c == '\r' &&
         conf_recv_buffer[buf_position_index + buf_media_type_index + 1] ==
             '\n')) {
      mtypes_amount++;
      st->media_types_array =
          realloc(st->media_types_array, mtypes_amount * sizeof(char *));
      (st->media_types_array)[mtypes_amount - 1] =
          malloc(buf_position_index * sizeof(char) + 1);
      memcpy((st->media_types_array)[mtypes_amount - 1],
             conf_recv_buffer + buf_media_type_index, buf_position_index);
      (st->media_types_array)[mtypes_amount - 1][buf_position_index] = '\0';
      buf_media_type_index += buf_position_index + 1;
      buf_position_index = 0;
    } else {
      buf_position_index++;
    }
    list_len++;
  }
  if (list_len > MTYPES_MAX_LEN || c == EOF) {
    freeMediaTypesArray(st);
    return 0;
  }
  return mtypes_amount;
}

void freeMediaTypesArray(settings *st) {
  int i;
  for (i = 0; i < st->media_types_amount; i++) {
    free((st->media_types_array)[i]);
  }
  free(st->media_types_array);
  st->media_types_array = NULL;
}

char *formStringFromMetric(int metric) {
  char aux_buf[15];
  snprintf(aux_buf, 15, "%d", metric);
  //+6: "OK" + space + '\r\n' + '\0'
  char *msg = malloc(strlen(aux_buf) * sizeof(char) + 6);
  strcpy(msg, "OK ");
  strcat(msg, aux_buf);
  strcat(msg, "\r\n");
  msg[strlen(msg)] = '\0';
  return msg;
}
