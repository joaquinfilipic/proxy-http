/*
 * utils.h
 *
 *  Created on: 6 may. 2018
 *      Author: Martin
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "../cbuffer.h"
#include "commons.h"
#include "parser_definitions.h"

#define null        NULL
#define MAX_HEADERS 20
#define B_SIZE      4096
#define AUX_B_SIZE	512

bool
is_value_in_header(const char *v, const char *h);

int
strcicmpchr(const char *str1, const char *str2, char c);

/**
 * Compares with case-insensitivity two strings.
 * @return     < 0 - if str1 is less than str2.
 *             = 0 - if str1 is equal to str2.
 *             > 0 - if str2 is less than str1.
 */
int
strcicmp(const char *s1, const char *s2);

int
string_to_positive_int(const char *str);

char *
request_target_to_origin_form(const char *url);

char *
search_header(const char *header, header_line **header_lines,
		size_t header_count);

char *
get_host(const char *host_header);

char *
get_port(const char *host_header);

bool
valid_request_info(request_parser_info *request_info);

#endif /* UTILS_H_ */
