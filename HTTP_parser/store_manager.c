/*
 * store_manager.c
 *
 *  Created on: 2 jun. 2018
 *      Author: Martin
 */

#include "store_manager.h"

int
store_information(buffer *b, http_message *message, state st) {

	if(b == null) {
		return -1;
	}
	if(message == null) {
		return -1;
	}

	size_t bytes_to_read;
	buffer_read_ptr(b, &bytes_to_read);

	switch(st) {
		case METHOD: {
			message->status->method = malloc(bytes_to_read + 1);
			copy(b, message->status->method);

			break;
		}
		case TARGET: {
			message->status->request_target = malloc(bytes_to_read + 1);
			copy(b, message->status->request_target);
			break;
		}
		case VERSION: {
			break;
		}
		case CODE: {
			break;
		}
		case MSG: {
			break;
		}
		case IDLE_0: {
			break;
		}
		case H_NAME: {
			int idx = *(message->header_count);
			header_line *header = message->header_lines[idx];

			header = malloc(sizeof(*header));
			header->field_name = malloc(bytes_to_read + 1);

			if(copy_header_name(b, header->field_name) == -1) {
				return -1;
			}
			message->header_lines[idx] = header;
			break;
		}
		case H_VALUE: {
			int idx = *(message->header_count);
			header_line *header = message->header_lines[idx];

			header->field_value = malloc(bytes_to_read + 1);

			if(copy_header_value(b, header->field_value) == -1) {
				return -1;
			}
			message->header_lines[idx] = header;
			*(message->header_count) = idx + 1;
			break;
		}
		case IDLE_1: {
			break;
		}
		case IDLE_2: {
			break;
		}
		case IDLE_3: {
			break;
		}
		case BODY: {
			break;
		}
		case ERROR: {
			break;
		}
		case MAX_STATES: {
			break;
		}
		default: {
			break;
		}
	}

	return 0;
}

int
copy_header_name(buffer *b, char *dest) {

	int  idx         = 0;
	bool found_tchar = false;

	while(buffer_can_read(b) && !found_tchar) {
		uint8_t c = buffer_read(b);
		if(!is_ows(c)) {
			if(!is_tchar(c)) {
				return -1;
			}
			idx = 0;
			dest[idx] = c;
			found_tchar = true;
		}
		idx ++;
	}

	if(!found_tchar) {
		return -1;
	}

	bool found_ows = false;

	while(buffer_can_read(b)) {
		uint8_t c = buffer_read(b);
		if(is_ows(c)) {
			found_ows = true;
		}
		else {
			if(found_ows) {
				return -1;
			}
			if(!is_tchar(c)) {
				return -1;
			}
			dest[idx] = c;
			idx ++;
		}
	}
	dest[idx] = '\0';

	return 0;
}

int
copy_header_value(buffer *b, char *dest) {

	int  idx          = 0;
	bool found_tchar = false;

	uint8_t c;
	while(buffer_can_read(b) && !found_tchar) {
		c = buffer_read(b);
		if(!is_ows(c)) {
			if(!is_tchar(c)) {
				return -1;
			}
			idx = 0;
			dest[idx] = c;
			found_tchar = true;
		}
		idx ++;
	}

	if(!found_tchar) {
		return -1;
	}

	while(buffer_can_read(b)) {
		c = buffer_read(b);
		dest[idx] = c;
		idx ++;
	}

	dest[idx] = '\0';

	return 0;
}

void
copy(buffer *b, char *dest) {
	int idx = 0;
	while(buffer_can_read(b)) {
		dest[idx] = buffer_read(b);
		idx ++;
	}
	dest[idx] = '\0';
}