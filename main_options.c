#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "main_options.h"

int handle_main_options(int argc, char **argv, settings *st) {
  // int value = -1;
  int index, c;

  extern char *optarg;
  extern int   optind, opterr, optopt;

  while ((c = getopt(argc, argv, "e:l:L:o:p:t:M:hv")) != -1) {
    switch (c) {
      case 'h':
        print_help();
        return -1;
      case 'v':
        print_version();
        return -1;
      case 'e':
        st->error_file = optarg;
        break;
      case 'l':
        st->http_url = optarg;
        break;
      case 'L':
        st->management_url = optarg;
        break;
      case 'o':
        st->management_port = atoi(optarg);
        break;
      case 'p':
        st->local_port = atoi(optarg);
        break;
      case 't':
        st->cmd = optarg;
        break;
      case 'M':
        st->media_types_amount = separateMediaTypes(optarg, st);
        if (st->media_types_amount == 0) {
          perror("media types not set");
        } else {
          printf("Media Types:\n");
          int i;
          for (i = 0; i < st->media_types_amount; i++) {
            printf("   ->%s\n", st->media_types_array[i]);
          }
          st->transformations_activated = 1;
        }
        break;

      case '?':
        if (optopt == 'p') {
          fprintf(stderr, "La opción -%c requiere un número.\n", optopt);
          return -1;
        } else if (isprint(optopt)) {
          fprintf(stderr, "Opción desconocida `-%c'.\n", optopt);
          return -1;
        } else {
          fprintf(stderr, "Opción de caracter desconocida `\\x%x'.\n", optopt);
          return -1;
        }

      default:
        abort();
    }
  }

  // for (index = optind; index < argc; index++){
  //   printf ("Argumento no válido %s\n", argv[index]);
  //   return -1;
  // }

  return 1;
}

// check this function! is not working when its used in master_socker...
in_addr_t to_s_addr(char *str) {
  if (strcmp(str, "loopback")) return INADDR_LOOPBACK;

  if (strcmp(str, "any")) return INADDR_ANY;

  if (strcmp(str, "broadcast")) return INADDR_BROADCAST;

  return INADDR_ANY;
}

int separateMediaTypes(const char *concat_media_types, settings *st) {
  int  concat_media_types_len = strlen(concat_media_types);
  int  buf_position_index     = 0;
  int  buf_media_type_index   = 0;
  int  mtypes_amount          = 0;
  int  list_len               = 1;
  char c;

  while (list_len <= concat_media_types_len &&
         (c = concat_media_types[buf_position_index + buf_media_type_index]) &&
         c != '\0') {
    if (c == ',' || list_len == concat_media_types_len) {
      if (c != ',') buf_position_index++;
      mtypes_amount++;
      st->media_types_array =
          realloc(st->media_types_array, mtypes_amount * sizeof(char *));
      (st->media_types_array)[mtypes_amount - 1] =
          malloc(buf_position_index * sizeof(char) + 1);
      memcpy((st->media_types_array)[mtypes_amount - 1],
             concat_media_types + buf_media_type_index, buf_position_index);
      (st->media_types_array)[mtypes_amount - 1][buf_position_index] = '\0';
      buf_media_type_index += buf_position_index + 1;
      buf_position_index = 0;
    } else {
      buf_position_index++;
    }
    list_len++;
  }
  return mtypes_amount;
}

void print_help(void) {
  printf("-e archivo-de-error\n");
  printf("   Especifica el archivo donde se redirecciona stderr de las ejecu-\n");
  printf("   ciones de los filtros. Por defecto el archivo es /dev/null.\n");
  printf("\n");

  printf("-h     Imprime la ayuda y termina.\n");
  printf("\n");

  printf("-l direccion-http\n");
  printf("   Establece la direccion donde servira el proxy HTTP.  Por defecto\n");
  printf("   escucha en todas las interfaces.\n");
  printf("\n");

  printf("-L direccion-de-management\n");
  printf("   Establece  la direccion donde servira el servicio de management.\n");
  printf("   Por defecto escucha unicamente en loopback.\n");
  printf("\n");

  printf("-M media-types-transformables\n");
  printf("   Lista de media types transformables. La  sintaxis  de  la  lista\n");
  printf("   sigue  las  reglas  del header Accept de HTTP (seccion 5.3.2 del\n");
  printf("   RFC7231).\n	 ");
  printf("   Por defecto la lista se encuentra vacia.\n");
  printf("   Por  ejemplo  el	valor text/plain,image/* transforara todas las\n");
  printf("   respuestas declaradas como text/plain o de tipo imagen como  ser\n");
  printf("   image/png.\n");
  printf("\n");

  printf("-o puerto-de-management\n");
  printf("   Puerto  STCP  donde se encuentra el servidor de management.  Por\n");
  printf("   defecto el valor es 9090.\n");
  printf("\n");

  printf("-p puerto-local\n");
  printf("   Puerto TCP donde escuchara por conexiones entrantes  HTTP.   Por\n");
  printf("   defecto el valor es 8080.\n");
  printf("\n");

  printf("-t cmd Comando  utilizado para las transformaciones externas. Compati-\n");
  printf("   ble con system(3).  La  seccion  FILTROS	describe  como	es  la\n");
  printf("   interaccion  entre httpd(8) y el comando filtro.	Por defecto no\n");
  printf("   se aplica ninguna transformacion.\n");
  printf("\n");

  printf("-v     Imprime informacion sobre la version version y termina.\n");
  printf("\n");
}

void print_version(void) { printf("Proxy ITBA - Group 2 - V 1.0.0\n"); }
