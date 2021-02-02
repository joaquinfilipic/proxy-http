#ifndef MAIN_OPTIONS
#define MAIN_OPTIONS

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "settings.h"

int       handle_main_options(int argc, char **argv, settings *st);
in_addr_t to_s_addr(char *str);
int       separateMediaTypes(const char *concatMediaTypes, settings *st);
void      print_help(void);
void      print_version(void);

#endif
