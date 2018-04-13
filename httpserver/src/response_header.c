/*
 * response_header.c
 *
 *  Created on: 17 Oct 2017
 *      Author: lqy
 */

#include "response_header.h"

//casting and %llu used because OpenSSL's BIO does not support %zu
void send_response_200(BIO* bio, size_t length) {
	BIO_printf(bio,
			"HTTP/1.1 200 OK\r\n" "Accept-Ranges: bytes\r\n" "Content-Length: %llu\r\n" "\r\n",
			(unsigned long long int) length);
	BIO_flush(bio);
}

void send_response_200_chunked(BIO* bio) {
	BIO_printf(bio,
			"HTTP/1.1 200 OK\r\n" "Accept-Ranges: none\r\n" "Transfer-Encoding: chunked\r\n" "\r\n");
	BIO_flush(bio);
}

//casting and %llu used because OpenSSL's BIO does not support %zu
void send_response_206(BIO* bio, size_t from, size_t to, size_t length,
		size_t total_length) {
	BIO_printf(bio,
			"HTTP/1.1 206 Partial Content\r\n" "Accept-Ranges: bytes\r\n" "Content-Range: bytes %llu-%llu/%llu\r\n" "Content-Length: %llu\r\n" "\r\n",
			(unsigned long long int) from, (unsigned long long int) to,
			(unsigned long long int) total_length,
			(unsigned long long int) length);
	BIO_flush(bio);
}

void send_response_301(BIO* bio, char* uri) {
	BIO_printf(bio,
			"HTTP/1.1 301 Moved Permanently\r\n" "Location: %s\r\n" "Content-Length: 0\r\n" "\r\n",
			uri);
	BIO_flush(bio);
}

void send_response_400(BIO* bio) {
	BIO_printf(bio,
			"HTTP/1.1 400 Bad Request\r\n" "Content-Length: 12\r\n" "\r\n");
	BIO_flush(bio);
}

void send_response_400_body(BIO* bio) {
	BIO_printf(bio, "Bad Request\n");
	BIO_flush(bio);
}

void send_response_403(BIO* bio) {
	BIO_printf(bio,
			"HTTP/1.1 403 Forbidden\r\n" "Content-Length: 10\r\n" "\r\n");
	BIO_flush(bio);
}

void send_response_403_body(BIO* bio) {
	BIO_printf(bio, "Forbidden\n");
	BIO_flush(bio);
}

void send_response_404(BIO* bio) {
	BIO_printf(bio,
			"HTTP/1.1 404 Not Found\r\n" "Content-Length: 10\r\n" "\r\n");
	BIO_flush(bio);
}

void send_response_404_body(BIO* bio) {
	BIO_printf(bio, "Not Found\n");
	BIO_flush(bio);
}

void send_response_416(BIO* bio) {
	BIO_printf(bio,
			"HTTP/1.1 416 Requested Range Not Satisfiable\r\n" "Content-Length: 32\r\n" "\r\n");
	BIO_flush(bio);
}

void send_response_416_body(BIO* bio) {
	BIO_printf(bio, "Requested Range Not Satisfiable\n");
	BIO_flush(bio);
}

void send_response_501(BIO* bio) {
	BIO_printf(bio,
			"HTTP/1.1 501 Not Implemented\r\n" "Content-Length: 16\r\n" "\r\n");
	BIO_flush(bio);
}

void send_response_501_body(BIO* bio) {
	BIO_printf(bio, "Not Implemented\n");
	BIO_flush(bio);
}
