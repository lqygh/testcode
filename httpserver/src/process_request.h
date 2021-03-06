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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/bio.h>

struct http_request {
	char method[HTTP_REQUEST_MAX_METHOD_SIZE]; //method(GET or HEAD)
	char uri[HTTP_REQUEST_MAX_URI_SIZE]; //request URI
	char version[HTTP_REQUEST_MAX_VERSION_SIZE]; //HTTP version

	char has_host;
	char host[HTTP_REQUEST_MAX_HOST_SIZE]; //virtual host

	char has_range;
	size_t range_from;
	char has_range_to;
	size_t range_to;
};

enum http_method {
	HTTP_METHOD_UNKNOWN, HTTP_METHOD_GET, HTTP_METHOD_HEAD
};

enum http_method http_method_to_enum(const struct http_request* request);

int read_http_request_line(BIO* bio_in, struct http_request* request_out);

int read_http_request_headers(BIO* bio_in, struct http_request* request_out);

#endif /* SRC_PROCESS_REQUEST_H_ */
