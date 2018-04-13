/*
 * process_request.h
 *
 *  Created on: 17 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_PROCESS_REQUEST_H_
#define SRC_PROCESS_REQUEST_H_

#define HTTP_REQUEST_MAX_METHOD_SIZE 16
#define HTTP_REQUEST_MAX_URI_SIZE 2048
#define HTTP_REQUEST_MAX_VERSION_SIZE 16
#define HTTP_REQUEST_MAX_HOST_SIZE 512
#define HTTP_REQUEST_MAX_UPGRADE_SIZE 16
#define HTTP_REQUEST_MAX_CONNECTION_SIZE 32
#define HTTP_REQUEST_MAX_SEC_WS_KEY_SIZE 64

#define HTTP_RESPONSE_MAX_VERSION_SIZE 16
#define HTTP_RESPONSE_MAX_STATUS_SIZE 16
#define HTTP_RESPONSE_MAX_REASON_SIZE 32
#define HTTP_RESPONSE_MAX_UPGRADE_SIZE 16
#define HTTP_RESPONSE_MAX_CONNECTION_SIZE 32
#define HTTP_RESPONSE_MAX_SEC_WS_KEY_SIZE 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct http_request {
	char method[HTTP_REQUEST_MAX_METHOD_SIZE]; //method(GET or HEAD)
	char uri[HTTP_REQUEST_MAX_URI_SIZE]; //request URI
	char version[HTTP_REQUEST_MAX_VERSION_SIZE]; //HTTP version

	char host[HTTP_REQUEST_MAX_HOST_SIZE]; //virtual host
	char upgrade[HTTP_REQUEST_MAX_UPGRADE_SIZE];
	char connection[HTTP_REQUEST_MAX_CONNECTION_SIZE];
	char sec_websocket_key[HTTP_REQUEST_MAX_SEC_WS_KEY_SIZE];
};

struct http_response {
	char version[HTTP_RESPONSE_MAX_VERSION_SIZE]; //HTTP version
	char status[HTTP_RESPONSE_MAX_VERSION_SIZE]; //HTTP status
	char reason[HTTP_RESPONSE_MAX_REASON_SIZE]; //HTTP reason phrase

	char upgrade[HTTP_RESPONSE_MAX_UPGRADE_SIZE];
	char connection[HTTP_RESPONSE_MAX_CONNECTION_SIZE];
	char sec_websocket_accept[HTTP_RESPONSE_MAX_SEC_WS_KEY_SIZE];
};

enum http_method {
	HTTP_METHOD_UNKNOWN, HTTP_METHOD_GET, HTTP_METHOD_HEAD
};

enum http_method http_method_to_enum(const struct http_request* request);

int read_http_request_line(FILE* fp_in, struct http_request* request_out);

int read_http_request_headers(FILE* fp_in, struct http_request* request_out);

int read_http_status_line(FILE* fp_in, struct http_response* response_out);

int read_http_response_headers(FILE* fp_in, struct http_response* response_out);

#endif /* SRC_PROCESS_REQUEST_H_ */
