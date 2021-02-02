/*
 * parser.h
 *
 *  Created on: 2 abr. 2018
 *      Author: Martin
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../cbuffer.h"
#include "commons.h"
#include "parser_definitions.h"
#include "parser_utils.h"
#include "store_manager.h"

buffer *
initialize_buffer(const int bytes);

http_message *
initialize_http_message(void);

request_parser_info *
initialize_request_info(void);

response_parser_info *
initialize_response_info(void);

void
free_request_parser(request_parser_info *parser_info);

void
free_response_parser(response_parser_info *parser_info);

int
parse_request(buffer *bin, buffer *bout, request_parser_info *request_info);

int
parse_response(buffer *bin, buffer *bout, response_parser_info *response_info);

void
parse_length(buffer *bin, buffer *bout, int *length);

int
parse_chunked(buffer *bin, buffer *bout, buffer *baux, int *chunk_size, 
	chunk_state *st);

/**
 * This method parses a http message, storing in the http_message type, the
 * following information:
 *     method.
 *     request_target in origin-form.
 *     important headers (host, content-length, transfer-encoding).
 * 
 * Receives:
 *     bin      - buffer with the characters to parse.
 *     bout     - buffer with the characters parsed.
 *     baux     - buffer to store the chars to later storage.
 *     message  - struct in which the information will be stored.
 *     st       - parser current state.
 *     msg_type - REQUEST or RESPONSE.
 *
 * Returns: 
 *     1 - could read correctly the information until BODY.
 *     0 - still reading and requests another buffer of characters.
 *    -1 - error occurred.
 */
int
parse_message(buffer *bin, buffer *bout, buffer *baux, http_message *message, 
	state *st, message_type msg_type);

transition_char
get_transition_char(char c);

bool
write_after_transition(state current_state, transition_char t_char);

#endif /* PARSER_H_ */
