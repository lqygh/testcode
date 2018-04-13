/*
 * process_request.c
 *
 *  Created on: 17 Oct 2017
 *      Author: lqy
 */

#include "process_request.h"

enum http_method http_method_to_enum(const struct http_request* request) {
	if (strncmp(request->method, "GET", 4) == 0) {
		return HTTP_METHOD_GET;
	} else if (strncmp(request->method, "HEAD", 5) == 0) {
		return HTTP_METHOD_HEAD;
	} else {
		return HTTP_METHOD_UNKNOWN;
	}
}

int my_BIO_gets(BIO* bio, char* buffer, int buffer_size) {
	int i = 0;
	char c = 0;

	while (1) {
		if (i >= buffer_size - 1) {
			break;
		}

		if (BIO_read(bio, &c, 1) != 1) {
			if (i <= 0) {
				return -1;
			} else {
				if (i < buffer_size - 1) {
					buffer[i] = '\0';
				}
				return i;
			}
		}

		buffer[i] = c;
		i += 1;

		if (c == '\n') {
			break;
		}
	}

	if (i < buffer_size - 1) {
		buffer[i] = '\0';
	}
	return i;
}

int read_http_request_line(BIO* bio_in, struct http_request* request_out) {
	char buffer[2 * sizeof(struct http_request)] = { 0 };

	//read Request-Line
	int ret = my_BIO_gets(bio_in, buffer, sizeof(buffer));
	if (ret < 0) {
		//connection broken
		return -2;
	}

	size_t len = strlen(buffer);
	if (len < 1 || len + 1 > sizeof(buffer) || buffer[len - 1] != '\n') {
		//request line too long
		//HTTP 400
		return -1;
	}

	//process Request-Line
	char delim[2] = " ";
	char* saveptr = NULL;
	char* tok = strtok_r(buffer, delim, &saveptr);
	for (int i = 0; i < 3; i++) {
		if (tok == NULL) {
			//HTTP 400
			return -1;
		}

		if (i == 0) {
			if (strlen(tok) > sizeof(request_out->method) - 1) {
				return -1;
			}
			strncpy(request_out->method, tok, sizeof(request_out->method) - 1);
		} else if (i == 1) {
			if (strlen(tok) > sizeof(request_out->uri) - 1) {
				return -1;
			}
			strncpy(request_out->uri, tok, sizeof(request_out->uri) - 1);
		} else {
			if (strlen(tok) > sizeof(request_out->version) - 1) {
				return -1;
			}
			strncpy(request_out->version, tok,
					sizeof(request_out->version) - 1);
		}

		tok = strtok_r(NULL, delim, &saveptr);
	}

	return 0;
}

int read_http_request_headers(BIO* bio_in, struct http_request* request_out) {
	char buffer[2 * sizeof(struct http_request)] = { 0 };

	//read Request Headers
	while (1) {
		int ret = my_BIO_gets(bio_in, buffer, sizeof(buffer));
		if (ret < 0) {
			//connection broken
			return -2;
		}

		size_t len = strlen(buffer);
		if (len < 1 || len + 1 > sizeof(buffer) || buffer[len - 1] != '\n') {
			//request line too long
			//HTTP 432
			return -1;
		}

		//headers end with an empty line
		if ((len == 2 && buffer[0] == '\r' && buffer[1] == '\n')
				|| (len == 1 && buffer[0] == '\n')) {
			return 0;
		}

		//process request header
		char* field_name = NULL;
		char* field_value = NULL;

		char delim[2] = " ";
		char* saveptr = NULL;
		char* tok = strtok_r(buffer, delim, &saveptr);
		if (tok == NULL) {
			//just skip this line
			continue;
		}
		field_name = tok;

		tok = strtok_r(NULL, delim, &saveptr);
		if (tok == NULL) {
			//just skip this line
			continue;
		}
		field_value = tok;

		if (strcmp(field_name, "Host:") == 0) {
			//Host field
			size_t field_value_len = strlen(field_value);
			if (field_value_len > sizeof(request_out->host) - 1) {
				//just skip this line
				continue;
			}
			if (field_value_len >= 1
					&& field_value[field_value_len - 1] == '\n') {
				field_value[field_value_len - 1] = '\0';
			}
			if (field_value_len >= 2
					&& field_value[field_value_len - 2] == '\r') {
				field_value[field_value_len - 2] = '\0';
			}
			strncpy(request_out->host, field_value,
					sizeof(request_out->host) - 1);
			request_out->has_host = 1;
		} else if (strcmp(field_name, "Range:") == 0) {
			//Range field
			if (strncmp(field_value, "bytes=", 6) == 0) {
				size_t field_value_len = strlen(field_value);
				if (field_value_len >= 1
						&& field_value[field_value_len - 1] == '\n') {
					field_value[field_value_len - 1] = '\0';
				}
				if (field_value_len >= 2
						&& field_value[field_value_len - 2] == '\r') {
					field_value[field_value_len - 2] = '\0';
				}

				if (field_value_len >= 6 + 2) {
					char* ns = field_value + 6;
					size_t ns_len = strlen(ns);

					unsigned char minus_count = 0;
					size_t minus_index = 0;

					char has_illegal_character = 0;
					for (size_t i = 0; i < ns_len; i++) {
						if (ns[i] == '-') {
							if (minus_count != 0) {
								minus_count += 1;
								break;
							}
							minus_index = i;
							minus_count += 1;
						} else if (ns[i] != '-' && !isdigit(ns[i])) {
							has_illegal_character = 1;
							break;
						}
					}
					if (minus_count != 1 || has_illegal_character) {
						continue;
					}

					//guaranteed to have exactly one '-' character and all other characters are decimal digits
					if (minus_index == 0) {
						request_out->has_range = 1;
						request_out->range_from = 0;
						request_out->has_range_to = 1;
						request_out->range_to = (size_t) strtol(ns + 1, NULL,
								10);
					} else if (minus_index == ns_len - 1) {
						request_out->has_range = 1;
						request_out->range_from = (size_t) strtol(ns, NULL, 10);
						request_out->has_range_to = 0;
					} else {
						size_t from = (size_t) strtol(ns, NULL, 10);
						size_t to = (size_t) strtol(ns + minus_index + 1, NULL,
								10);
						if (from > to) {
							continue;
						}
						request_out->has_range = 1;
						request_out->range_from = from;
						request_out->has_range_to = 1;
						request_out->range_to = to;
					}

				}
			}
		}

	}

	return 0;
}
