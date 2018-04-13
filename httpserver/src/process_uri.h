/*
 * process_uri.h
 *
 *  Created on: 15 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_PROCESS_URI_H_
#define SRC_PROCESS_URI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

int canonicalise_path(const char* uri_in, char* uri_out,
		const size_t uri_out_size);

int append_string(char* to, const size_t to_buffer_size, const char* suffix);

int prepend_string(char* to, const size_t to_buffer_size, const char* prefix);

int decode_percent_encoding(const char* uri_in, char* uri_out,
		const size_t uri_out_size);

int string_has_prefix(const char* string, const char* prefix);

int process_uri(const char* vhost_path, const char* uri_in, char* path_out,
		const size_t path_out_size);

#endif /* SRC_PROCESS_URI_H_ */
