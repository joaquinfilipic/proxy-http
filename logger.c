#include "logger.h"

FILE* init_log_path(char* path) {
  FILE* fptr_aux;
  fptr_aux = fopen(path, "a+");

  if (fptr_aux == NULL) {
    printf("Error!");
    exit(1);
  }

  return fptr_aux;
}

FILE* init_log(void) { return init_log_path("./http.log"); }

void close_log(FILE* fptr) { fclose(fptr); }

void write_log(FILE* fptr, char* log) { fprintf(fptr, "%s\n", log); }

char* print_time(void) {
  int    size = 0;
  time_t t;
  char*  buf;

  t = time(NULL);  // get current calendar time

  char* timestr                = asctime(localtime(&t));
  timestr[strlen(timestr) - 1] = 0;  // Getting rid of \n

  size = strlen(timestr) + 1 + 2;  // Additional +2 for square braces
  buf  = (char*)malloc(size);

  memset(buf, 0x0, size);
  snprintf(buf, size, "%s", timestr);

  return buf;
}

char* format_log(char* ip, char* verb, char* context, int response_code) {
  char* to_ret = malloc(sizeof(char) * SIZE_OF_LOG);
  sprintf(to_ret, "%s - - [%s] \"%s %s\" %d", ip, print_time(), verb, context,
          response_code);
  return to_ret;
}
