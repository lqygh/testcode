/*
 * response_header.h
 *
 *  Created on: 17 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_RESPONSE_HEADER_H_
#define SRC_RESPONSE_HEADER_H_

#include <openssl/bio.h>

void send_response_200(BIO* bio, size_t length);

void send_response_200_chunked(BIO* bio);

void send_response_206(BIO* bio, size_t from, size_t to, size_t length,
		size_t total_length);

void send_response_301(BIO* bio, char* uri);

void send_response_400(BIO* bio);

void send_response_400_body(BIO* bio);

void send_response_403(BIO* bio);

void send_response_403_body(BIO* bio);

void send_response_404(BIO* bio);

void send_response_404_body(BIO* bio);

void send_response_416(BIO* bio);

void send_response_416_body(BIO* bio);

void send_response_501(BIO* bio);

void send_response_501_body(BIO* bio);

#endif /* SRC_RESPONSE_HEADER_H_ */
