/*
 * ws.c
 *
 *  Created on: 9 Dec 2017
 *      Author: lqy
 */

#include "ws.h"

int ws_do_handshake(struct ws* ws, struct http_request* server_http_request_out,
		const struct http_request* client_http_request_in) {
	if (ws == NULL) {
		return -1;
	}

	int in_fd2 = dup(ws->in_fd);
	if (in_fd2 < 0) {
		return -1;
	}
	FILE* in_fp = fdopen(in_fd2, "r");
	if (in_fp == NULL) {
		close(in_fd2);
		return -1;
	}
	/* very important to prevent C FILE I/O from reading past HTTP headers */
	if (setvbuf(in_fp, NULL, _IONBF, 0) != 0) {
		fclose(in_fp);
		return -1;
	}

	int out_fd2 = dup(ws->out_fd);
	if (out_fd2 < 0) {
		fclose(in_fp);
		return -1;
	}
	FILE* out_fp = fdopen(out_fd2, "w");
	if (out_fp == NULL) {
		fclose(in_fp);
		close(out_fd2);
		return -1;
	}

	if (ws->is_server) {
		/* server */

		//char buffer[4096] = { 0 };
		struct http_request http_req;
		memset(&http_req, 0, sizeof(http_req));

		//read Request-Line
		int ret = read_http_request_line(in_fp, &http_req);
		if (ret >= 0) {
			//printf("METHOD for %d: %s\n", in_fd2, http_req.method);
			//printf("Request-URI for %d: %s\n", in_fd2, http_req.uri);
			//printf("HTTP-Version for %d: %s\n", in_fd2, http_req.version);
		} else if (ret == -1) {
			//send HTTP 400
			//printf("Error processing Request-Line for %d\n", in_fd2);

			send_response_400(out_fp);
			send_response_400_body(out_fp);

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		} else {
			//connection broken
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		//read Request Headers
		ret = read_http_request_headers(in_fp, &http_req);
		if (ret >= 0) {
			//printf("Host for %d: %s\n", in_fd2, http_req.host);
			//printf("Upgrade for %d: %s\n", in_fd2, http_req.upgrade);
			//printf("Connection for %d: %s\n", in_fd2, http_req.connection);
			//printf("Sec-WebSocket-Key for %d: %s\n", in_fd2,
			//http_req.sec_websocket_key
			//);
		} else if (ret == -1) {
			send_response_400(out_fp);
			send_response_400_body(out_fp);

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		} else {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		//filter unsupported methods
		enum http_method request_method = http_method_to_enum(&http_req);
		if (request_method != HTTP_METHOD_GET
				&& request_method != HTTP_METHOD_HEAD) {
			//send HTTP 501
			send_response_501(out_fp);
			send_response_501_body(out_fp);

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		//check request headers
		if (strcasecmp(http_req.upgrade, "websocket") != 0) {
			send_response_404(out_fp);
			if (request_method != HTTP_METHOD_HEAD) {
				send_response_404_body(out_fp);
			}

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		if (strcasecmp(http_req.connection, "Upgrade") != 0) {
			send_response_404(out_fp);
			if (request_method != HTTP_METHOD_HEAD) {
				send_response_404_body(out_fp);
			}

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		if (strlen(http_req.sec_websocket_key) < 1) {
			send_response_404(out_fp);
			if (request_method != HTTP_METHOD_HEAD) {
				send_response_404_body(out_fp);
			}

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		//send response
		char sec_ws_accept[64] = { 0 };
		compute_sec_ws_key(http_req.sec_websocket_key, sec_ws_accept,
				sizeof(sec_ws_accept), NULL);
		if (ret < 0) {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}
		//printf("compute_sec_ws_key() returned %d, output: %s\n", ret,
		//		sec_ws_accept);
		send_response_101_ws(out_fp, sec_ws_accept);

		fclose(in_fp);
		fclose(out_fp);
		if (request_method == HTTP_METHOD_HEAD) {
			return -1;
		} else {
			if (server_http_request_out != NULL) {
				memcpy(server_http_request_out, &http_req,
						sizeof(struct http_request));
			}
			return 0;
		}
	} else {
		/* client */

		//send request
		if (client_http_request_in == NULL) {
			send_request_ws(out_fp, NULL, NULL, NULL);
		} else {
			send_request_ws(out_fp, client_http_request_in->uri,
					client_http_request_in->host,
					client_http_request_in->sec_websocket_key);
		}

		//receive response
		struct http_response http_res;
		memset(&http_res, 0, sizeof(http_res));

		//read Status-Line
		int ret = read_http_status_line(in_fp, &http_res);
		if (ret >= 0) {
			//printf("Version for %d: %s\n", in_fd2, http_res.version);
			//printf("Status for %d: %s\n", in_fd2, http_res.status);
			//printf("Reason for %d: %s\n", in_fd2, http_res.reason);
		} else if (ret == -1) {
			//printf("Error processing Status-Line for %d\n", in_fd2);

			fclose(in_fp);
			fclose(out_fp);
			return -1;
		} else {
			//connection broken
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		//read Response Headers
		ret = read_http_response_headers(in_fp, &http_res);
		if (ret >= 0) {
			//printf("Upgrade for %d: %s\n", in_fd2, http_res.upgrade);
			//printf("Connection for %d: %s\n", in_fd2, http_res.connection);
			//printf("Sec-WebSocket-Accept for %d: %s\n", in_fd2,
			//http_res.sec_websocket_accept
			//);
		} else if (ret == -1) {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		} else {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		//check response
		if (strcmp(http_res.status, "101") != 0) {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		if (strcasecmp(http_res.upgrade, "websocket") != 0) {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		if (strcasecmp(http_res.connection, "Upgrade") != 0) {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		if (strlen(http_res.sec_websocket_accept) < 1) {
			fclose(in_fp);
			fclose(out_fp);
			return -1;
		}

		fclose(in_fp);
		fclose(out_fp);
		return 0;
	}

	return -1;
}

struct ws* ws_new(int in_fd, int out_fd, int is_server) {
	struct ws* ws = malloc(sizeof(struct ws));
	if (ws == NULL) {
		return NULL;
	}

	ws->in_fd = in_fd;
	ws->out_fd = out_fd;
	ws->is_server = is_server;

	return ws;
}

int ws_free(struct ws* ws, int should_close_fd) {
	if (ws == NULL) {
		return -1;
	}

	if (should_close_fd) {
		close(ws->in_fd);
		if (ws->in_fd != ws->out_fd) {
			close(ws->out_fd);
		}
	}
	free(ws);

	return 0;
}

/*ssize_t ws_read(struct ws* ws, void *buf, size_t count) {
 return -1;
 }

 ssize_t ws_write(struct ws* ws, const void *buf, size_t count) {
 return -1;
 }*/
