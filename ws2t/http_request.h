/*
 * http_request.h
 *
 *  Created on: 13 Dec 2017
 *      Author: lqy
 */

#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <stdio.h>

void send_request_ws(FILE* fp, const char* uri, const char* host,
		const char* sec_ws_key);

#endif /* HTTP_REQUEST_H_ */
