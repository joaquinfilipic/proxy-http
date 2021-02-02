/*
 * commons.h
 *
 *  Created on: 31 mar. 2018
 *      Author: Martin
 */

#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define CR      0x0D
#define DQUOTE  0x22
#define DPOINT  0x3A
#define HTAB    0x09
#define LF      0x0A
#define SP      0x20

/*
 * ----------------------------------------------------------------------------
 * Some of the most used characters
 * ----------------------------------------------------------------------------
 */

// 'A'-'Z' || 'a'-'z'
bool is_alpha(uint8_t c);

// '0' || '1'
bool is_bit(uint8_t c);

// '\r'
bool is_cr(uint8_t c);

// '0'-'9'
bool is_digit(uint8_t c);

// '"'
bool is_dquote(uint8_t c);

// ":"
bool is_dpoint(uint8_t c);

// DIGIT || 'A'-'F'
bool is_hexdig(uint8_t c);

bool is_htab(uint8_t c);

// \n
bool is_lf(uint8_t c);

// SP or HTAB
bool is_ows(uint8_t c);

bool is_sp(uint8_t c);

// Visible printing characters
bool is_vchar(uint8_t c);

bool is_wsp(uint8_t c);

/*
 * ----------------------------------------------------------------------------
 * Some more less common types of characters
 * ----------------------------------------------------------------------------
 */

bool is_tchar(uint8_t c);

bool is_unreserved(uint8_t c);

bool is_sub_delims(uint8_t c);

bool is_gen_delims(uint8_t c);

#endif /* COMMONS_H_ */
