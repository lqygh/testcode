/*
 * http_response.h
 *
 *  Created on: 12 Dec 2017
 *      Author: lqy
 */

#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <stdio.h>

void send_response_101_ws(FILE* fp, const char* sec_ws_accept);

void send_response_400(FILE* fp);

void send_response_400_body(FILE* fp);

void send_response_404(FILE* fp);

void send_response_404_body(FILE* fp);

void send_response_501(FILE* fp);

void send_response_501_body(FILE* fp);

#endif /* HTTP_RESPONSE_H_ */
