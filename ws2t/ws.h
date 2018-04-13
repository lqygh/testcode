/*
 * ws.h
 *
 *  Created on: 15 Dec 2017
 *      Author: lqy
 */

#ifndef WS_H_
#define WS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include "http_response.h"
#include "http_request.h"
#include "process_request.h"
#include "compute_sec_ws_key.h"

struct ws {
	int in_fd;
	int out_fd;
	int is_server;
};

int ws_do_handshake(struct ws* ws, struct http_request* server_http_request_out,
		const struct http_request* client_http_request_in);

struct ws* ws_new(int in_fd, int out_fd, int is_server);

int ws_free(struct ws* ws, int should_close_fd);

#endif /* WS_H_ */
