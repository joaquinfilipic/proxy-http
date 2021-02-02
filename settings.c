#include <stdio.h>
#include <stdlib.h>

#include "settings.h"

settings* init_settings(settings* st) {
  st = malloc(sizeof(settings));

  st->error_file                = "/dev/null/error.log";  //-e archivo-de-error
  st->http_url                  = "any";                  //-l dirección-http
  st->management_url            = "loopback";  //-L dirección-de-management
  st->management_port           = 9090;        //-o puerto-de-management
  st->local_port                = 8080;        //-p puerto-local
  st->cmd                       = "cat";       //-t cmd
  st->media_types_array         = NULL;
  st->transformations_activated = 0;
  st->media_types_amount        = 0;
  st->concurrent_connections    = 0;
  st->historical_accesses       = 0;
  st->clients_transfered_bytes  = 0;
  st->servers_transfered_bytes  = 0;
  st->pass = "pass123";  // password for configuration

  return st;
}
