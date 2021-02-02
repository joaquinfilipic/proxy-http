#ifndef SETTINGS
#define SETTINGS

#include <stdio.h>
#include <stdlib.h>

typedef struct settings {
  char * error_file;         //-e archivo-de-error
  char * http_url;           //-l dirección-http
  char * management_url;     //-L dirección-de-management
  int    management_port;    //-o puerto-de-management
  int    local_port;         //-p puerto-local
  char * cmd;                //-t cmd
  char **media_types_array;  //-M media-types-transformables
  int    transformations_activated;
  int    media_types_amount;
  int    concurrent_connections;
  int    historical_accesses;
  int    clients_transfered_bytes;
  int    servers_transfered_bytes;
  char * pass;

} settings;

settings *init_settings(settings *st);

#endif
