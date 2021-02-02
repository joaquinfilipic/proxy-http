/*
 * commons.c
 *
 *  Created on: 31 mar. 2018
 *      Author: Martin
 */

#include <stdio.h>
#include <unistd.h>
#include "commons.h"

/*
 * ----------------------------------------------------------------------------
 * Some of the most used characters
 * ----------------------------------------------------------------------------
 */

bool is_alpha(uint8_t c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_bit(uint8_t c) {
	return c == '0' || c == '1';
}

bool is_cr(uint8_t c) {
	return c == CR;
}

bool is_digit(uint8_t c) {
	return c >= '0' && c <= '9';
}

bool is_dquote(uint8_t c) {
	return c == DQUOTE;
}

bool is_dpoint(uint8_t c) {
	return c == DPOINT;
}

bool is_hexdig(uint8_t c) {
	return is_digit(c) || (c >= 0x61 && c <= 0x66)
		|| (c >= 0x41 && c <= 0x46);
}

bool is_htab(uint8_t c) {
	return c == HTAB;
}

bool is_lf(uint8_t c) {
	return c == LF;
}

bool is_ows(uint8_t c) {
	return is_sp(c) || is_htab(c);
}

bool is_sp(uint8_t c) {
	return c == SP;
}

bool is_vchar(uint8_t c) {
	return c >= 0x21 && c <= 0x7E;
}

bool is_wsp(uint8_t c) {
	return is_sp(c) || is_htab(c);
}

/*
 * ----------------------------------------------------------------------------
 * Some more less common types of characters
 * ----------------------------------------------------------------------------
 */


bool is_tchar(uint8_t c) {
	return     c == '!'  || c == '#' || c == '$' || c == '%' || c == '&'
			|| c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' 
			|| c == '^'  || c == '_' || c == '`' || c == '|' || c == '~'
			|| c == '\"' || is_digit(c) || is_alpha(c);
}

bool is_unreserved(uint8_t c) {
	return is_alpha(c) || is_digit(c) || c == '-' || c == '.' || c == '_'
			|| c == '~';
}

bool is_sub_delims(uint8_t c) {
	return c == '!' || c == '$' || c == '&' || c == '\'' || c == '('
			|| c == ')';
}

bool is_gen_delims(uint8_t c) {
	return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']'
			|| c == '@';
}

/*
 * ----------------------------------------------------------------------------
 * Some used arrays of characters
 * ----------------------------------------------------------------------------
 */
