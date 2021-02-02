#ifndef LOGGER_FILE
#define LOGGER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE_OF_LOG 1024

FILE* init_log_path(char* path);
FILE* init_log(void);
void  close_log(FILE* fptr);
void  write_log(FILE* fptr, char* log);
char* print_time(void);
char* format_log(char* ip, char* verb, char* context, int response_code);

#endif
