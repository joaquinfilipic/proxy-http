#include "utilities.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"

int max(int n1, int n2) { return n1 > n2 ? n1 : n2; }

char *response_503(void) {
  return "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 129\r\nConnection: Closed\r\nContent-Type: text/html; charset=iso-8859-1\r\n\r\n <html>\r\n <head>\r\n <title>503 Service Unavailable</title>\r\n </head>\r\n <body>\r\n <h1>Service Unavailable</h1>\r\n </body>\r\n </html>\r\n";
}

int isMediaTypeInList(char *responseMType, settings *st) {
  if (st->media_types_array == NULL || st->media_types_amount == 0) {
    return 0;
  }
  int i;
  for (i = 0; i < st->media_types_amount; i++) {
    if (compareMediaType(responseMType, (st->media_types_array)[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int compareMediaType(char *responseMType, char *setMType) {
  if (strcmp(setMType, "/") == 0) return 0;
  const unsigned char *s1 = (const unsigned char *)responseMType;
  const unsigned char *s2 = (const unsigned char *)setMType;
  unsigned char        c1, c2;
  typedef enum { TYPE, SUBTYPE, OWS, PARAM, QUOT1, QUOT2, END } ParseState_t;
  ParseState_t parse_state = TYPE;
  do {
    c1 = tolower((unsigned char)*s1++);
    c2 = tolower((unsigned char)*s2++);
    switch (parse_state) {
      case TYPE:
        if (c2 == '/') parse_state = SUBTYPE;
        break;
      case SUBTYPE:
        if (c2 == '*') return 0;
        if (c1 == ' ' && c1 != c2) {
          c1 = tolower((unsigned char)*s1++);
          if (c1 == ';' && c2 == '\0') return 0;
          parse_state = OWS;
        } else if (c2 == ' ' && c1 != c2) {
          c2          = tolower((unsigned char)*s2++);
          parse_state = OWS;
        } else if (c1 == ';') {
          if (c2 == '\0') return 0;
          parse_state = OWS;
        }
        break;
      case OWS:
        if (c1 == ' ' && c1 != c2)
          c1 = tolower((unsigned char)*s1++);
        else if (c2 == ' ' && c1 != c2)
          c2 = tolower((unsigned char)*s2++);
        parse_state = PARAM;
        break;
      case PARAM:
        if (c1 == '"' && c1 != c2) {
          c1          = tolower((unsigned char)*s1++);
          parse_state = QUOT1;
        } else if (c2 == '"' && c1 != c2) {
          c2          = tolower((unsigned char)*s2++);
          parse_state = QUOT2;
        }
        break;
      case QUOT1:
        if (c1 == '"' && c1 != c2) {
          c1          = tolower((unsigned char)*s1++);
          parse_state = END;
        }
        break;
      case QUOT2:
        if (c2 == '"' && c1 != c2) {
          c2          = tolower((unsigned char)*s2++);
          parse_state = END;
        }
        break;
      case END:;
      default:;
    }
    if (c1 == '\0') return c1 - c2;
  } while (c1 == c2);
  return c1 - c2;
}
