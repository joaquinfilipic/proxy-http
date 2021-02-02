/*
 * utils.c
 *
 *  Created on: 6 may. 2018
 *      Author: Martin
 */

#include "parser_utils.h"

bool
is_value_in_header(const char *v, const char *h) {

	bool found = false;
	char *coma_ptr;

	while(!found) {
		coma_ptr = strchr(h, ',');
		if(coma_ptr == null) {
			return strcicmp(v, h) == 0;
		}
		else {
			if(strcicmpchr(v, h, ',') == 0) {
				return true;
			}

			h = coma_ptr + 1;
			while(*h != '\0' && *h == ' ') {
				h ++;
			}
		}
	}

	return false;
}

int
strcicmpchr(const char *str1, const char *str2, char c) {

	int idx_1 = 0, idx_2 = 0;
	while(str1[idx_1] != '\0' && str2[idx_2] != '\0'
		&& str1[idx_1] != c && str2[idx_2] != c) {
		int d = tolower(str1[idx_1]) - tolower(str2[idx_2]);
		if(d != 0) {
			return d;
		}
		idx_1 ++;
		idx_2 ++;
	}
	if(str1[idx_1] == c || str2[idx_2] == c) {
		return 0;
	}
	if(str1[idx_1] == '\0' && str2[idx_2] == '\0') {
		return 0;
	}
	if(str1[idx_1] == '\0') {
		return -1;
	}
	if(str2[idx_2] == '\0') {
		return 1;
	}
	return 0;
}

int
strcicmp(const char *str1, const char *str2) {

	int idx_1 = 0, idx_2 = 0;
	while(str1[idx_1] != '\0' && str2[idx_2] != '\0') {
		int d = tolower(str1[idx_1]) - tolower(str2[idx_2]);
		if(d != 0) {
			return d;
		}
		idx_1 ++;
		idx_2 ++;
	}
	if(str1[idx_1] == '\0' && str2[idx_2] == '\0') {
		return 0;
	}
	if(str1[idx_1] == '\0') {
		return -1;
	}
	if(str2[idx_2] == '\0') {
		return 1;
	}
	return 0;
}

int
string_to_positive_int(const char *str) {
	
	int resp = 0;
	int idx  = 0;

	char c;
	while(str[idx] != '\0') {
		c = str[idx];
		if(!is_digit(c)) {
			return -1;
		}
		resp = resp*10 + (c - '0');
		idx ++;
	}

	return resp;
}

char *
request_target_to_origin_form(const char *url) {
	int   i = 0, start = 0, end = 0;
	int   schemeSeen = 0;
	char *resp;

	while(url[i]) {
		if(url[i] == ':') {
			i++;
			if(url[i] == '/') {
				i++;
				if(url[i] == '/') {
					i++;
					if(!schemeSeen) {
						schemeSeen = 1;
					} else {
						return null;
					}
				}
			}
		}
		if(url[i] == '/' && !start) {
			start = i;
		}
		if(url[i] == '#') {
			end = i;
		}

		i++;
	}
	if(start == -1) {
		return null;
	}
	if(end == 0) {
		end = i;
	}

	resp = calloc(i, sizeof(char*));
	strncpy(resp, url+start, end-start);
	resp[end-start] = 0;

	return resp;
}

char *
search_header(const char *header, header_line **header_lines,
		size_t header_count) {
	size_t i;
	for(i = 0; i < header_count; i++) {
		if(strcicmp(header, header_lines[i]->field_name) == 0) {
			return header_lines[i]->field_value;
		}
	}

	return null;
}

char *
get_host(const char *host_header) {
	char *resp = malloc(strlen(host_header) + 1);

	int i = 0;
	while(host_header[i] != '\0' && host_header[i] != ':') {
		resp[i] = host_header[i];
		i++;
	}

	resp[i] = '\0';

	return resp;
}

char *
get_port(const char *host_header) {
	char *resp;

	int i = 0;
	while(host_header[i] != '\0' && host_header[i] != ':') {
		i++;
	}

	if(host_header[i] == '\0') {
		resp = malloc(strlen("80") + 1);
		resp[0] = '8';
		resp[1] = '0';
		resp[2] = '\0';
		return resp;
	}
	
	// max port number: 5 digits
	resp = malloc(5 + 1);

	i++;

	int j = 0;
	while(host_header[i] != '\0') {
		resp[j] = host_header[i];
		i++;
		j++;
	}

	resp[j] = '\0';

	return resp;
}

bool
valid_request_info(request_parser_info *request_info) {
	
	if(request_info->msg->status->request_target == null) {
		return false;
	}

	char *host_header = search_header(
		"Host",
		request_info->msg->header_lines,
		*(request_info->msg->header_count)
	);
	if(host_header == null) {
		return false;
	}

	printf("Host: %s\n", host_header);
//	printf("Host: %s\n", get_host(host_header));

	request_info->host = get_host(host_header);
	request_info->port = string_to_positive_int(get_port(host_header)); 

	char *length_header = search_header(
		"Content-Length",
		request_info->msg->header_lines,
		*(request_info->msg->header_count)
	);

	// content-length found
	if(length_header != null) {
		request_info->length = string_to_positive_int(length_header);
	}

	return true;
}