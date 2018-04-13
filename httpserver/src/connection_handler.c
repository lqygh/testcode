/*
 * connection_handler.c
 *
 *  Created on: 25 Oct 2017
 *      Author: lqy
 */

#include "connection_handler.h"

static void handle_connection(int id, BIO* socket_bio, char* default_dir,
		struct vhost_table* vhost_table);

void* http_handle_connection(void* args) {
	struct connection_handler_arg* oarg = (struct connection_handler_arg*) args;
	struct connection_handler_arg arg = { 0 };

	//copy arguments onto thread's own stack
	memcpy(&arg, oarg, sizeof(arg));

	//finish copying arguments onto own stack and wait until main thread has called pthread_detach()
	pthread_barrier_wait(arg.barrier);

	//ignore SIGPIPE that can be possibly caused by writes to disconnected clients
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "signal() error for %d\n", arg.socket_fd);

		close(arg.socket_fd);
		pthread_exit(NULL);
	}

	//printf("thread for %d started, default_dir: %s\n", arg.socket_fd,
	//	arg.default_dir);

	//create BIO for new socket
	BIO* socket_bio = BIO_new_socket(arg.socket_fd, BIO_NOCLOSE);
	if (socket_bio == NULL) {
		fprintf(stderr, "BIO_new_socket() error for %d\n", arg.socket_fd);

		close(arg.socket_fd);
		pthread_exit(NULL);
	}

	int identifier = arg.socket_fd;
	handle_connection(identifier, socket_bio, arg.default_dir, arg.vhost_table);

	//printf("thread for %d terminating\n", arg.socket_fd);

	BIO_free(socket_bio);
	shutdown(arg.socket_fd, SHUT_RDWR);
	close(arg.socket_fd);
	pthread_exit(NULL);
}

void* https_handle_connection(void* args) {
	struct https_handler_arg* oarg = (struct https_handler_arg*) args;
	struct https_handler_arg arg = { 0 };

	//copy arguments onto thread's own stack
	memcpy(&arg, oarg, sizeof(arg));

	//finish copying arguments onto own stack and wait until main thread has called pthread_detach()
	pthread_barrier_wait(arg.connection_handler_arg.barrier);

	//ignore SIGPIPE that can be possibly caused by writes to disconnected clients
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "signal() error for %d\n",
				arg.connection_handler_arg.socket_fd);

		//ERR_remove_thread_state(NULL);
		close(arg.connection_handler_arg.socket_fd);
		pthread_exit(NULL);
	}

	//printf("TLS thread for %d started, default_dir: %s\n",
	//arg.connection_handler_arg.socket_fd,
	//arg.connection_handler_arg.default_dir);

	//create BIO for new socket
	BIO* socket_bio = BIO_new_socket(arg.connection_handler_arg.socket_fd,
	BIO_NOCLOSE);
	if (socket_bio == NULL) {
		fprintf(stderr, "BIO_new_socket() error for %d\n",
				arg.connection_handler_arg.socket_fd);

		//ERR_remove_thread_state(NULL);
		close(arg.connection_handler_arg.socket_fd);
		pthread_exit(NULL);
	}

	//create SSL BIO
	BIO* ssl_bio = BIO_new_ssl(arg.ssl_ctx, 0);
	if (ssl_bio == NULL) {
		fprintf(stderr, "BIO_new_ssl() error for %d\n",
				arg.connection_handler_arg.socket_fd);

		BIO_free(socket_bio);
		//ERR_remove_thread_state(NULL);
		close(arg.connection_handler_arg.socket_fd);
		pthread_exit(NULL);
	}

	SSL* ssl = NULL;
	BIO_get_ssl(ssl_bio, &ssl);
	if (ssl == NULL) {
		fprintf(stderr, "BIO_get_ssl() error for %d\n",
				arg.connection_handler_arg.socket_fd);

		BIO_free(ssl_bio);
		BIO_free(socket_bio);
		//ERR_remove_thread_state(NULL);
		close(arg.connection_handler_arg.socket_fd);
		pthread_exit(NULL);
	}
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	BIO_push(ssl_bio, socket_bio);
	if (BIO_do_handshake(ssl_bio) <= 0) {
		fprintf(stderr, "BIO_do_handshake() error for %d\n",
				arg.connection_handler_arg.socket_fd);
		ERR_print_errors_fp(stderr);

		BIO_free(ssl_bio);
		BIO_free(socket_bio);
		//ERR_remove_thread_state(NULL);
		close(arg.connection_handler_arg.socket_fd);
		pthread_exit(NULL);
	}

	int identifier = arg.connection_handler_arg.socket_fd;
	handle_connection(identifier, ssl_bio,
			arg.connection_handler_arg.default_dir,
			arg.connection_handler_arg.vhost_table);

	//printf("TLS thread for %d terminating\n",
	//arg.connection_handler_arg.socket_fd);

	while (SSL_shutdown(ssl) == 0) {
	};
	//BIO_free(ssl_bio) also frees the underlying SSL object
	BIO_free(ssl_bio);
	BIO_free(socket_bio);
	//ERR_remove_thread_state(NULL);
	shutdown(arg.connection_handler_arg.socket_fd, SHUT_RDWR);
	close(arg.connection_handler_arg.socket_fd);
	pthread_exit(NULL);
}

static void handle_connection(int id, BIO* socket_bio, char* default_dir,
		struct vhost_table* vhost_table) {
	id += 0;

	char should_loop = 1;
	while (should_loop) {
		char buffer[4096] = { 0 };
		struct http_request http_req;
		memset(&http_req, 0, sizeof(http_req));

		//read Request-Line
		int ret = read_http_request_line(socket_bio, &http_req);
		if (ret >= 0) {
			//printf("METHOD for %d: %s\n", id, http_req.method);
			//printf("Request-URI for %d: %s\n", id, http_req.uri);
			//printf("HTTP-Version for %d: %s\n", id, http_req.version);
		} else if (ret == -1) {
			//send HTTP 400
			//printf("Error processing Request-Line for %d\n", id);
			send_response_400(socket_bio);
			send_response_400_body(socket_bio);
			should_loop = 0;
		} else {
			//connection broken
			should_loop = 0;
		}

		if (should_loop == 0) {
			break;
		}

		//read Request Headers
		ret = read_http_request_headers(socket_bio, &http_req);
		if (ret >= 0) {
			//printf("has_host for %d: %d\n", id, http_req.has_host);
			//printf("Host for %d: %s\n", id, http_req.host);

			//printf("has_range for %d: %d\n", id, http_req.has_range);
			//printf("range_from for %d: %zu\n", id, http_req.range_from);
			//printf("has_range_to for %d: %d\n", id, http_req.has_range_to);
			//printf("range_to for %d: %zu\n", id, http_req.range_to);
		} else if (ret == -1) {
			send_response_400(socket_bio);
			send_response_400_body(socket_bio);
			should_loop = 0;
		} else {
			should_loop = 0;
		}

		if (should_loop == 0) {
			//printf("Error processing Request-Headers for %d\n", id);
			break;
		}

		//filter unsupported methods
		enum http_method request_method = http_method_to_enum(&http_req);
		if (request_method != HTTP_METHOD_GET
				&& request_method != HTTP_METHOD_HEAD) {
			//send HTTP 501
			send_response_501(socket_bio);
			send_response_501_body(socket_bio);
			continue;
		}

		//process URI
		const char* vhost_path = NULL;
		if (http_req.has_host) {
			vhost_path = vhost_table_find_ignore_port(vhost_table,
					http_req.host);
		}
		if (vhost_path == NULL) {
			vhost_path = default_dir;
		}

		if (process_uri(vhost_path, http_req.uri, buffer, sizeof(buffer))
				!= 0) {
			//printf("process_uri(%s, %s) failed for %d\n", vhost_path,
			//		http_req.uri, id);

			//send HTTP 404
			send_response_404(socket_bio);
			if (request_method == HTTP_METHOD_GET) {
				send_response_404_body(socket_bio);
			}

			continue;
		} else {
			//if the URI refers to a directory but does not have a trailing slash, send HTTP 301
			size_t old_uri_len = strlen(http_req.uri);
			char old_uri_copy[sizeof(http_req.uri) + 1] = { 0 };
			//strncpy(old_uri_copy, http_req.uri, sizeof(http_req.uri));
			snprintf(old_uri_copy, sizeof(old_uri_copy), "%s", http_req.uri);

			struct stat fst = { 0 };
			if (stat(buffer, &fst) == 0 && S_ISDIR(fst.st_mode)
					&& old_uri_copy[old_uri_len - 1] != '/') {
				//printf("URI refers to a directory without a trailing slash\n");
				//printf("%s | %s\n", http_req.uri, buffer);

				if (old_uri_len + 1 + 1 >= sizeof(old_uri_copy)) {
					//	printf("Buffer is full, sending 404 instead of 301\n");
					send_response_404(socket_bio);
					if (request_method == HTTP_METHOD_GET) {
						send_response_404_body(socket_bio);
					}
				} else {
					old_uri_copy[old_uri_len] = '/';
					old_uri_copy[old_uri_len + 1] = '\0';
					//printf("Redirecting to %s\n", old_uri_copy);

					//send HTTP 301
					send_response_301(socket_bio, old_uri_copy);
				}

				continue;
			} else {
				snprintf(http_req.uri, sizeof(http_req.uri), "%s", buffer);
			}
		}

		//test NULL URI
		{
			struct stat fst = { 0 };
			int ret = stat(http_req.uri, &fst);
			if (ret < 0) {
				//printf("stat(%s) failed for %d\n", http_req.uri, id);
				//send HTTP 404
				send_response_404(socket_bio);
				if (request_method == HTTP_METHOD_GET) {
					send_response_404_body(socket_bio);
				}
				continue;
			}

			if (S_ISDIR(fst.st_mode)) {
				//printf("%s is a directory for %d\n", http_req.uri, id);

				char new_uri[sizeof(http_req.uri)] = { 0 };
				//strcpy(new_uri, http_req.uri);
				snprintf(new_uri, sizeof(new_uri), "%s", http_req.uri);

				ret = append_string(new_uri, sizeof(new_uri), "/" INDEX_FILE);
				if (ret < 0) {
					//printf("append_string(%s) failed for %d\n", new_uri, id);
					//send HTTP 404
					send_response_404(socket_bio);
					if (request_method == HTTP_METHOD_GET) {
						send_response_404_body(socket_bio);
					}
					continue;
				}

				//test if index file exists
				ret = canonicalise_path(new_uri, new_uri, sizeof(new_uri));
				if (ret >= 0 && stat(new_uri, &fst) == 0) {
					//if index file exists, copy new_uri back into http_req.uri
					//printf("%s exists for %d\n", new_uri, id);
					//strncpy(http_req.uri, new_uri,
					//	sizeof(http_req.uri));
					snprintf(http_req.uri, sizeof(http_req.uri), "%s", new_uri);
				} else {
					//printf("canonicalise_path(%s) or stat(%s) failed for %d\n",
					//		new_uri, new_uri, id);
					//printf("listing directory %s for %d\n", http_req.uri,
					//		id);
					//if index file does not exist, perform directory listing
					DIR* dirp = opendir(http_req.uri);
					if (dirp == NULL) {
						int err = errno;
						//printf("opendir(%s) failed for %d\n", http_req.uri,
						//		id);
						if (err == ENOENT) {
							//send HTTP 404
							send_response_404(socket_bio);
							if (request_method == HTTP_METHOD_GET) {
								send_response_404_body(socket_bio);
							}
						} else if (err == EACCES) {
							//send HTTP 403
							send_response_403(socket_bio);
							if (request_method == HTTP_METHOD_GET) {
								send_response_403_body(socket_bio);
							}
						} else {
							//otherwise, send HTTP 404
							send_response_404(socket_bio);
							if (request_method == HTTP_METHOD_GET) {
								send_response_404_body(socket_bio);
							}
						}
						continue;
					}

					send_response_200_chunked(socket_bio);
					if (request_method == HTTP_METHOD_GET
							&& send_dir_listing_to_socket_chunked(dirp,
									socket_bio) < 0) {
						should_loop = 0;
					}
					closedir(dirp);
					if (should_loop == 0) {
						//printf("Directory Listing Response IO error for %d\n",
						//		id);
						break;
					}
					continue;
				}
			}
		}

		//open file
		//printf("Opening file %s for %d\n", http_req.uri, id);
		int file_fd = open(http_req.uri, O_RDONLY);
		if (file_fd < 0) {
			int err = errno;
			//printf("open(%s) failed for %d, errno: %d\n", http_req.uri, id,
			//err
			//);

			if (err == ENOENT) {
				//send HTTP 404
				send_response_404(socket_bio);
				if (request_method == HTTP_METHOD_GET) {
					send_response_404_body(socket_bio);
				}
			} else if (err == EACCES) {
				//send HTTP 403
				send_response_403(socket_bio);
				if (request_method == HTTP_METHOD_GET) {
					send_response_403_body(socket_bio);
				}
			} else {
				//otherwise, send HTTP 404
				send_response_404(socket_bio);
				if (request_method == HTTP_METHOD_GET) {
					send_response_404_body(socket_bio);
				}
			}
			continue;
		}

		//get file mode and length
		struct stat fst = { 0 };
		ret = fstat(file_fd, &fst);
		if (ret < 0) {
			//printf("fstat(%s) failed for %d\n", http_req.uri, id);

			send_response_404(socket_bio);
			if (request_method == HTTP_METHOD_GET) {
				send_response_404_body(socket_bio);
			}

			close(file_fd);
			continue;
		}
		if (S_ISREG(fst.st_mode)) {
			//printf("%s is a regular file for %d\n", http_req.uri, id);
		} else {
			//printf("%s is NOT a regular file for %d\n", http_req.uri, id);

			//write dummy response
			send_response_404(socket_bio);
			if (request_method == HTTP_METHOD_GET) {
				send_response_404_body(socket_bio);
			}

			close(file_fd);
			continue;
		}

		//in case of empty file
		if (fst.st_size == 0) {
			//write response header
			send_response_200(socket_bio, (size_t) fst.st_size);

			close(file_fd);
			continue;
		}

		if (http_req.has_range) {
			//printf("Range request from %d\n", id);
			size_t from = http_req.range_from;
			size_t to;
			size_t file_length = (size_t) fst.st_size;
			if (http_req.has_range_to) {
				to = http_req.range_to;
				if (to > file_length - 1) {
					to = file_length - 1;
				}
			} else {
				to = file_length - 1;
			}

			if (from <= to && from <= file_length - 1 && to <= file_length - 1
					&& set_fd_position(file_fd, from) == 0) {
				size_t bytes_to_send = to - from + 1;
				ret = set_fd_position(file_fd, from);

				//write response header
				send_response_206(socket_bio, from, to, bytes_to_send,
						file_length);

				//write file to client
				if (request_method == HTTP_METHOD_GET
						&& send_file_to_socket(file_fd, socket_bio,
								bytes_to_send) < 0) {
					should_loop = 0;
				}
			} else {
				//send HTTP 416
				send_response_416(socket_bio);
				if (request_method == HTTP_METHOD_GET) {
					send_response_416_body(socket_bio);
				}
			}
			close(file_fd);
			if (should_loop == 0) {
				//printf("Response IO error for %d\n", id);
				break;
			} else {
				continue;
			}
		}

		//printf("Non-Range request from %d\n", id);
		//write response header
		send_response_200(socket_bio, (size_t) fst.st_size);

		//write file to client
		if (request_method == HTTP_METHOD_GET
				&& send_file_to_socket(file_fd, socket_bio,
						(size_t) fst.st_size) < 0) {
			should_loop = 0;
		}
		close(file_fd);
		if (should_loop == 0) {
			//printf("Response IO error for %d\n", id);
			break;
		}
	}
}
