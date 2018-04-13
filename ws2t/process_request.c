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

int read_http_request_line(FILE* fp_in, struct http_request* request_out) {
	char buffer[2 * sizeof(struct http_request)] = { 0 };

	//read Request-Line
	char* cret = fgets(buffer, sizeof(buffer), fp_in);
	if (cret == NULL) {
		//connection broken
		return -2;
	}

	size_t len = strlen(buffer);
	if (len < 1 || buffer[len - 1] != '\n') {
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
			if (strlen(tok) + 1 > sizeof(request_out->method)) {
				return -1;
			}
			strncpy(request_out->method, tok, sizeof(request_out->method) - 1);
		} else if (i == 1) {
			if (strlen(tok) + 1 > sizeof(request_out->uri)) {
				return -1;
			}
			strncpy(request_out->uri, tok, sizeof(request_out->uri) - 1);
		} else {
			if (strlen(tok) + 1 > sizeof(request_out->version)) {
				return -1;
			}
			strncpy(request_out->version, tok,
					sizeof(request_out->version) - 1);
		}

		tok = strtok_r(NULL, delim, &saveptr);
	}

	return 0;
}

int read_http_request_headers(FILE* fp_in, struct http_request* request_out) {
	char buffer[2 * sizeof(struct http_request)] = { 0 };

	//read Request Headers
	while (1) {
		char* cret = fgets(buffer, sizeof(buffer), fp_in);
		if (cret == NULL) {
			//connection broken
			return -2;
		}

		size_t len = strlen(buffer);
		if (len < 1 || buffer[len - 1] != '\n') {
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

		//set null after field name
		for (size_t i = field_value - field_name; i > 0; i--) {
			if (field_name[i] == ' ') {
				field_name[i] = '\0';
			} else {
				break;
			}
		}

		//strip \r and \n after field value
		size_t field_value_len = strlen(field_value);
		if (field_value_len >= 1 && field_value[field_value_len - 1] == '\n') {
			field_value[field_value_len - 1] = '\0';
		}
		if (field_value_len >= 2 && field_value[field_value_len - 2] == '\r') {
			field_value[field_value_len - 2] = '\0';
		}
		field_value_len = strlen(field_value);

		if (strcmp(field_name, "Host:") == 0) {
			//Host field
			if (field_value_len + 1 > sizeof(request_out->host)) {
				//just skip this line if too long
				continue;
			}
			strncpy(request_out->host, field_value,
					sizeof(request_out->host) - 1);
		} else if (strcmp(field_name, "Upgrade:") == 0) {
			//Upgrade field
			if (field_value_len + 1 > sizeof(request_out->upgrade)) {
				//just skip this line if too long
				continue;
			}
			strncpy(request_out->upgrade, field_value,
					sizeof(request_out->upgrade) - 1);
		} else if (strcmp(field_name, "Connection:") == 0) {
			//Connection field
			if (field_value_len + 1 > sizeof(request_out->connection)) {
				//just skip this line if too long
				continue;
			}
			strncpy(request_out->connection, field_value,
					sizeof(request_out->connection) - 1);
		} else if (strcmp(field_name, "Sec-WebSocket-Key:") == 0) {
			//Sec-WebSocket-Key field
			if (field_value_len + 1 > sizeof(request_out->sec_websocket_key)) {
				//just skip this line if too long
				continue;
			}
			strncpy(request_out->sec_websocket_key, field_value,
					sizeof(request_out->sec_websocket_key) - 1);
		}

	}

	return 0;
}

int read_http_status_line(FILE* fp_in, struct http_response* response_out) {
	char buffer[2 * sizeof(struct http_response)] = { 0 };

	//read Status-Line
	char* cret = fgets(buffer, sizeof(buffer), fp_in);
	if (cret == NULL) {
		//connection broken
		return -2;
	}

	size_t len = strlen(buffer);
	if (len < 1 || buffer[len - 1] != '\n') {
		//status line too long
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
			if (strlen(tok) + 1 > sizeof(response_out->version)) {
				return -1;
			}
			strncpy(response_out->version, tok,
					sizeof(response_out->version) - 1);
		} else if (i == 1) {
			if (strlen(tok) + 1 > sizeof(response_out->status)) {
				return -1;
			}
			strncpy(response_out->status, tok,
					sizeof(response_out->status) - 1);
		} else {
			if (strlen(tok) + 1 > sizeof(response_out->reason)) {
				return -1;
			}
			strncpy(response_out->reason, tok,
					sizeof(response_out->reason) - 1);
		}

		tok = strtok_r(NULL, delim, &saveptr);
	}

	return 0;
}

int read_http_response_headers(FILE* fp_in, struct http_response* response_out) {
	char buffer[2 * sizeof(struct http_response)] = { 0 };

	//read Response Headers
	while (1) {
		char* cret = fgets(buffer, sizeof(buffer), fp_in);
		if (cret == NULL) {
			//connection broken
			return -2;
		}

		size_t len = strlen(buffer);
		if (len < 1 || buffer[len - 1] != '\n') {
			//response line too long
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

		//set null after field name
		for (size_t i = field_value - field_name; i > 0; i--) {
			if (field_name[i] == ' ') {
				field_name[i] = '\0';
			} else {
				break;
			}
		}

		//strip \r and \n after field value
		size_t field_value_len = strlen(field_value);
		if (field_value_len >= 1 && field_value[field_value_len - 1] == '\n') {
			field_value[field_value_len - 1] = '\0';
		}
		if (field_value_len >= 2 && field_value[field_value_len - 2] == '\r') {
			field_value[field_value_len - 2] = '\0';
		}
		field_value_len = strlen(field_value);

		if (strcmp(field_name, "Upgrade:") == 0) {
			//Upgrade field
			if (field_value_len + 1 > sizeof(response_out->upgrade)) {
				//just skip this line if too long
				continue;
			}
			strncpy(response_out->upgrade, field_value,
					sizeof(response_out->upgrade) - 1);
		} else if (strcmp(field_name, "Connection:") == 0) {
			//Connection field
			if (field_value_len + 1 > sizeof(response_out->connection)) {
				//just skip this line if too long
				continue;
			}
			strncpy(response_out->connection, field_value,
					sizeof(response_out->connection) - 1);
		} else if (strcmp(field_name, "Sec-WebSocket-Accept:") == 0) {
			//Sec-WebSocket-Accept field
			if (field_value_len + 1
					> sizeof(response_out->sec_websocket_accept)) {
				//just skip this line if too long
				continue;
			}
			strncpy(response_out->sec_websocket_accept, field_value,
					sizeof(response_out->sec_websocket_accept) - 1);
		}

	}

	return 0;
}
