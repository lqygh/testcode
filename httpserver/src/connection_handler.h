/*
 * connection_handler.h
 *
 *  Created on: 25 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_CONNECTION_HANDLER_H_
#define SRC_CONNECTION_HANDLER_H_

#define INDEX_FILE "index.html"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include "process_uri.h"
#include "file_io.h"
#include "process_request.h"
#include "response_header.h"
#include "vhost_table.h"
#include "dir_listing.h"

struct connection_handler_arg {
	int socket_fd;
	pthread_barrier_t* barrier;
	char* default_dir;
	struct vhost_table* vhost_table;
};

struct https_handler_arg {
	SSL_CTX* ssl_ctx;
	struct connection_handler_arg connection_handler_arg;
};

void* http_handle_connection(void* args);

void* https_handle_connection(void* args);

#endif /* SRC_CONNECTION_HANDLER_H_ */
