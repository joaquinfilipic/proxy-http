#ifndef OTHER
#define OTHER

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"

int max(int n1, int n2);

char *response_503(void);

int isMediaTypeInList(char *responseMType, struct settings *st);

int compareMediaType(char *responseMType, char *setMType);

#endif
