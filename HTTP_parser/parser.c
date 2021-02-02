/*
 * parsers.c
 *
 *  Created on: 2 abr. 2018
 *      Author: Martin
 */

#include "parser.h"

state transition_matrix[HTTP_TYPES][MAX_STATES][MAX_CHARS] = {
	/*              SPACE  , SLASH_R, SLASH_N, D_POINT, OTHER  */
	/* REQUEST  */ {
	/* METHOD   */ {TARGET , ERROR  , ERROR  , ERROR  , METHOD },
	/* TARGET   */ {VERSION, ERROR  , ERROR  , TARGET , TARGET },
	/* VERSION  */ {ERROR  , IDLE_0 , ERROR  , ERROR  , VERSION},
	/* CODE     */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  },
	/* MSG      */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  },
	/* IDLE_0   */ {ERROR  , ERROR  , H_NAME , ERROR  , ERROR  },
	/* H_NAME   */ {H_NAME , ERROR  , ERROR  , H_VALUE, H_NAME },
	/* H_VALUE  */ {H_VALUE, IDLE_1 , ERROR  , H_VALUE, H_VALUE},
	/* IDLE_1   */ {ERROR  , ERROR  , IDLE_2 , ERROR  , ERROR  },
	/* IDLE_2   */ {ERROR  , IDLE_3 , ERROR  , ERROR  , H_NAME },
	/* IDLE_3   */ {ERROR  , ERROR  , BODY   , ERROR  , ERROR  },
	/* BODY     */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  },
	/* ERROR    */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  }
				   },
	/* RESPONSE */ {
	/* METHOD   */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  },
	/* TARGET   */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  },
	/* VERSION  */ {CODE   , ERROR  , ERROR  , ERROR  , VERSION},
	/* CODE     */ {MSG    , IDLE_0 , ERROR  , ERROR  , CODE   },
	/* MSG      */ {MSG    , IDLE_0 , ERROR  , MSG    , MSG    },
	/* IDLE_0   */ {ERROR  , ERROR  , H_NAME , ERROR  , ERROR  },
	/* H_NAME   */ {H_NAME , IDLE_3 , ERROR  , H_VALUE, H_NAME },
	/* H_VALUE  */ {H_VALUE, IDLE_1 , ERROR  , H_VALUE, H_VALUE},
	/* IDLE_1   */ {ERROR  , ERROR  , IDLE_2 , ERROR  , ERROR  },
	/* IDLE_2   */ {ERROR  , IDLE_3 , ERROR  , ERROR  , H_NAME },
	/* IDLE_3   */ {ERROR  , ERROR  , BODY   , ERROR  , ERROR  },
	/* BODY     */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  },
	/* ERROR    */ {ERROR  , ERROR  , ERROR  , ERROR  , ERROR  }
	               }
};



buffer *
initialize_buffer(const int bytes) {

	// Initialize the actual buffer made of bytes
	uint8_t *buff = calloc(bytes, sizeof(uint8_t));

	// Reserve memory for the buffer struct
	buffer *b = malloc(sizeof(buffer));

	// Initialize the struct
	buffer_init(b, bytes, buff);

	return b;
}

http_message *
initialize_http_message() {
	http_message *msg = malloc(sizeof(*msg));

	msg->status       = malloc(sizeof(*(msg->status)));
	msg->header_lines = calloc(MAX_HEADERS, sizeof(*(msg->header_lines)));
	msg->header_count = malloc(sizeof(*(msg->header_count)));

	*(msg->header_count) = 0;

	return msg;
}

request_parser_info *
initialize_request_info(void) {
	request_parser_info *request_info = malloc(sizeof(*request_info));
	
	request_info->aux_buffer       = initialize_buffer(AUX_B_SIZE);
	request_info->headers_buffer   = initialize_buffer(B_SIZE);
	request_info->msg              = initialize_http_message();
	request_info->st               = METHOD; 
	request_info->railing          = false;
	request_info->ready_to_connect = false;

	request_info->length           = -1;

	return request_info;
}

response_parser_info *
initialize_response_info(void) {
	response_parser_info *response_info = malloc(sizeof(*response_info));

	response_info->aux_buffer    = initialize_buffer(AUX_B_SIZE);
	response_info->msg           = initialize_http_message();
	response_info->st            = VERSION;
	response_info->railing       = false;
	response_info->check_headers = false;

	response_info->is_chunked    = false;
	response_info->length        = -1;

	return response_info;
}

void free_request_parser(request_parser_info *parser_info) {

	free(parser_info->aux_buffer);
	free(parser_info->headers_buffer);

	free(parser_info->msg->status);

	size_t i;
	for (i = 0; i < *(parser_info->msg->header_count); i++) {
		free(parser_info->msg->header_lines[i]->field_name);
		free(parser_info->msg->header_lines[i]->field_value);
		free(parser_info->msg->header_lines[i]);
	}

	free(parser_info->msg->header_lines);
	free(parser_info->msg->header_count);
	free(parser_info->msg);

	free(parser_info);
}

void free_response_parser(response_parser_info *parser_info) {

	free(parser_info->aux_buffer);

	free(parser_info->msg->status);
	free(parser_info->msg->header_lines);
	free(parser_info->msg->header_count);
	free(parser_info->msg);

	free(parser_info);
}

int
parse_request(buffer *bin, buffer *bout, request_parser_info *request_info) {

	int result;

	if(request_info->railing) {

		// only when content-length header was found
		if(request_info->length != -1) {
			parse_length(bin, bout, &(request_info->length));
		}

		else {
			while(buffer_can_read(bin) && buffer_can_write(bout)) {
				buffer_write(bout, buffer_read(bin));
			}
		}

		return 0;
	}

	if(request_info->st != BODY) {

		result = parse_message(
			bin,
			request_info->headers_buffer,
			request_info->aux_buffer,
			request_info->msg,
			&(request_info->st),
			REQUEST
		);

		// parser finished parsing the headers or found an error
		if(result == 1 || result == -1) {

			// the needed info to establish connection was already found
			if(valid_request_info(request_info)) {
				request_info->railing = true;
				request_info->ready_to_connect = true;
			}
			else {
				return -1;
			}
		}

	}

	if(request_info->railing) {
		while(buffer_can_read(request_info->headers_buffer) 
			&& buffer_can_write(bout)) {
			buffer_write(bout, buffer_read(request_info->headers_buffer));
		}

		// only when content-length header was found
		if(request_info->length != -1) {
			parse_length(bin, bout, &(request_info->length));
		}

		else {
			while(buffer_can_read(bin) && buffer_can_write(bout)) {
				buffer_write(bout, buffer_read(bin));
			}
		}

	}

	return 0;
}

int
parse_response(buffer *bin, buffer *bout, response_parser_info *response_info) {

	int result;

	if(response_info->railing) {
		while(buffer_can_read(bin) && buffer_can_write(bout)) {
			buffer_write(bout, buffer_read(bin));
		}
		return 1;
	}

	if(response_info->st != BODY) {

		result = parse_message(
			bin,
			bout,
			response_info->aux_buffer,
			response_info->msg,
			&(response_info->st),
			RESPONSE
		);

		// error while parsing response headers
		if(result == -1) {
			response_info->railing = true;
		}

		// parser got to body state with no errors
		else if(result == 1) {

			response_info->check_headers = true;

			char *header;

			header = search_header(
				"Content-Length",
				response_info->msg->header_lines,
				*(response_info->msg->header_count)
			);

			// content-length found
			if(header != null) {
				response_info->length = string_to_positive_int(header);
			}

			// content-length not found
			else {

				header = search_header(
					"Transfer-Encoding",
					response_info->msg->header_lines,
					*(response_info->msg->header_count)
				);

				// transfer-encoding: chunked found
				if(header != null && strcicmp("chunked", header) == 0) {

					response_info->is_chunked = true;
					response_info->chunk_size = 0;

					// needed for storing chunk size numbers while dechunking
					buffer_reset(response_info->aux_buffer);
				}

				else {
					response_info->railing = true;
				}

			}

			return 1;
		}

	}

	if(response_info->st == BODY) {

		// only when content-length header was found
		if(response_info->length != -1) {
			parse_length(bin, bout, &(response_info->length));
		}

		// only when transfer-encoding: chunked was found
		else if(response_info->is_chunked) {
			
			result = parse_chunked(
				bin,
				bout,
				response_info->aux_buffer,
				&(response_info->chunk_size),
				&(response_info->chunk_st)
			);

			if(result == -1) {
				response_info->railing = true;
			}

		}

	}

	// for when the railing is set and bin has still chars to send
	if(response_info->railing) {
		while(buffer_can_read(bin) && buffer_can_write(bout)) {
			buffer_write(bout, buffer_read(bin));
		}
		return 1;			
	}

	return 1;
}

int
parse_message(buffer *bin, buffer *bout, buffer *baux, http_message *message, 
	state *st, message_type msg_type) {

	//buffer_reset(bout);

	state current_state = *st;
	state next_state;

	// start parsing
	while(buffer_can_read(bin) && buffer_can_write(bout) && buffer_can_write(baux)) {
		
		// next char to read
		char c = buffer_read(bin);

		if(c == EOF) {
			// no body found
			return 1;
		}

		// redirect char to output buffer
		buffer_write(bout, c);

		transition_char t_char = get_transition_char(c);

		next_state = transition_matrix[msg_type][current_state][t_char];

		if(next_state == BODY) {
			*st = next_state;
			return 1;
		}

		if(next_state == ERROR) {
			return -1;
		}

		if(next_state != current_state) {

			if(next_state != IDLE_3
				&& store_information(baux, message, current_state) == -1) {
				return -1;
			}

			// always reset on state change
			buffer_reset(baux);

			// check if the current char needs to be stored
			if(write_after_transition(current_state, t_char)) {
				buffer_write(baux, c);
			}

			current_state = next_state;
			*st = current_state;
		}
		else {
			buffer_write(baux, c);
		}

	}

	// when an element has more than AUX_B_SIZE bytes.
	if(!buffer_can_write(baux)) {
		return -1;
	}

	return 0;
}

void
parse_length(buffer *bin, buffer *bout, int *length) {

	while(*length > 0 && buffer_can_read(bin)
		&& buffer_can_write(bout)) {

		buffer_write(bout, buffer_read(bin));
		*length = *length - 1;
	}
}

int
parse_chunked(buffer *bin, buffer *bout, buffer *baux, int *chunk_size, 
	chunk_state *st) {

	chunk_state current_state = *st;
	chunk_state next_state;

	char c;

	// start parsing
	while(buffer_can_read(bin) && buffer_can_write(bout) 
		&& buffer_can_write(baux)) {

		c = buffer_read(bin);

		buffer_write(bout, c);

		if(c == EOF) {
			return 1;
		}

		if(*chunk_size < 0) {
			return -1;
		}

		switch(current_state) {
			case CHUNK_SIZE: {
				if(is_hexdig(c)) {
					next_state = CHUNK_SIZE;
					buffer_write(baux, c);
				}
				else if(is_cr(c)) {
					next_state = CR_STATE;

					// So that strtol can read correctly the hex number.
					buffer_write(baux, '\0');

					*chunk_size = strtol((const char *)baux->read, null, 16);

					if(*chunk_size == 0) {

						while(buffer_can_read(bin) && buffer_can_write(bout)) {
							buffer_write(bout, buffer_read(bin));
						}
						return 1;
					}

					buffer_reset(baux);
				}
				else {
					return -1;
				}
				break;
			}
			case CR_STATE: {
				if(is_lf(c)) {
					next_state = LF_STATE;
				}
				else {
					return -1;
				}
				break;
			}
			case LF_STATE: {
				if(*chunk_size > 0) {
					next_state = CHUNK_DATA;
					*chunk_size = *chunk_size - 1;
				}
				else {
					next_state = CHUNK_SIZE;
					buffer_write(baux, c);
				}
				break;
			}
			case CHUNK_DATA: {
				if(is_cr(c) && *chunk_size == 0) {
					next_state = CR_STATE;
				}
				else {
					next_state = CHUNK_DATA;
					*chunk_size = *chunk_size - 1;
				}
				break;
			}
			default : {
				break;
			}
		}
		if(next_state != current_state) {
			current_state = next_state;
			*st = current_state;
		}

	}

	return 0;
}

transition_char
get_transition_char(char c) {
	if(is_sp(c)) {
		return SPACE;
	}
	if(is_cr(c)) {
		return SLASH_R;
	}
	if(is_lf(c)) {
		return SLASH_N;
	}
	if(is_dpoint(c)) {
		return D_POINT;
	}
	return OTHER;
}

bool
write_after_transition(state current_state, transition_char t_char) {
	if(current_state == IDLE_2 && t_char == OTHER) {
		return true;
	}
	return false;
}