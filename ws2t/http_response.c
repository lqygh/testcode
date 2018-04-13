/*
 * http_response.c
 *
 *  Created on: 15 Dec 2017
 *      Author: lqy
 */

#include "http_response.h"

void send_response_101_ws(FILE* fp, const char* sec_ws_accept) {
	fprintf(fp,
			"HTTP/1.1 101 Switching Protocols\r\n" "Upgrade: websocket\r\n" "Connection: Upgrade\r\n" "Sec-WebSocket-Accept: %s\r\n" "\r\n",
			sec_ws_accept);
	fflush(fp);
}

void send_response_400(FILE* fp) {
	fprintf(fp, "HTTP/1.1 400 Bad Request\r\n" "Content-Length: 12\r\n" "\r\n");
	fflush(fp);
}

void send_response_400_body(FILE* fp) {
	fprintf(fp, "Bad Request\n");
	fflush(fp);
}

void send_response_404(FILE* fp) {
	fprintf(fp, "HTTP/1.1 404 Not Found\r\n" "Content-Length: 10\r\n" "\r\n");
	fflush(fp);
}

void send_response_404_body(FILE* fp) {
	fprintf(fp, "Not Found\n");
	fflush(fp);
}

void send_response_501(FILE* fp) {
	fprintf(fp,
			"HTTP/1.1 501 Not Implemented\r\n" "Content-Length: 16\r\n" "\r\n");
	fflush(fp);
}

void send_response_501_body(FILE* fp) {
	fprintf(fp, "Not Implemented\n");
	fflush(fp);
}
