/*
 * store_manager.h
 *
 *  Created on: 2 jun. 2018
 *      Author: Martin
 */

#ifndef STORE_MANAGER_H_
#define STORE_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cbuffer.h"
#include "parser_definitions.h"
#include "parser_utils.h"
#include "commons.h"

int
store_information(buffer *b, http_message *message, state st);

int
copy_header_name(buffer *b, char *dest);

int
copy_header_value(buffer *b, char *dest);

void
copy(buffer *b, char *dest);

#endif /* STORE_MANAGER_H_ */