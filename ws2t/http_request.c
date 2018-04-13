/*
 * http_request.c
 *
 *  Created on: 15 Dec 2017
 *      Author: lqy
 */

#include "http_request.h"

void send_request_ws(FILE* fp, const char* uri, const char* host,
		const char* sec_ws_key) {
	if (uri != NULL) {
		fprintf(fp, "GET %s HTTP/1.1\r\n", uri);
	} else {
		fprintf(fp, "GET / HTTP/1.1\r\n");
	}

	if (host != NULL) {
		fprintf(fp, "Host: %s\r\n", host);
	}

	fprintf(fp, "Upgrade: websocket\r\n" "Connection: Upgrade\r\n");

	if (sec_ws_key != NULL) {
		fprintf(fp, "Sec-WebSocket-Key: %s\r\n", sec_ws_key);
	} else {
		fprintf(fp, "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n");
	}

	fprintf(fp, "\r\n");

	fflush(fp);
}
