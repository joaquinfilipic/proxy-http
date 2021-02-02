/*
 * parser_definitions.h
 *
 *  Created on: 2 jun. 2018
 *      Author: Martin
 */

#ifndef PARSER_DEFINITIONS_H_
#define PARSER_DEFINITIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../cbuffer.h"
#include "commons.h"

typedef enum {
	REQUEST, RESPONSE, HTTP_TYPES
} message_type;

typedef enum {
	METHOD, TARGET, VERSION, CODE, MSG, IDLE_0, H_NAME, H_VALUE,
	IDLE_1, IDLE_2, IDLE_3, BODY, ERROR, MAX_STATES
} state;

typedef enum {
	SPACE, SLASH_R, SLASH_N, D_POINT, OTHER, MAX_CHARS
} transition_char;

typedef enum {
	CHUNK_SIZE, CR_STATE, LF_STATE, CHUNK_DATA
} chunk_state;

// status-line = HTTP-version SP status-code SP reason-phrase CRLF
typedef struct status_line_struct {
	char *method;
	char *request_target;
	char *http_version;
	char *status_code;
	char *reason_phrase;
} status_line;

// header-field = field-name ":" OWS field-value OWS
typedef struct header_line_struct {
	char *field_name;
	char *field_value;
} header_line;

// encapsulates http-request and http-response
typedef struct http_message_struct {
	status_line  *status;
	header_line **header_lines;
	size_t       *header_count;
} http_message;

// neccesary information for the request parser
typedef struct request_parser_info_struct {
	buffer       *aux_buffer;
	buffer       *headers_buffer;
	http_message *msg;
	state         st;
	bool          railing;
	bool          ready_to_connect;
	int           length;

	char         *host;
	int           port;
} request_parser_info;

// neccesary information for the response parser
typedef struct response_parser_info_struct {
	buffer       *aux_buffer;
	http_message *msg;
	state         st;
	chunk_state   chunk_st;
	bool          railing;
	bool          is_chunked;
	bool          check_headers;
	int           length;
	int           chunk_size;
} response_parser_info;

#endif /* PARSER_DEFINITIONS_H_ */