/*
 * main.c
 *
 *  Created on: 15 Dec 2017
 *      Author: lqy
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include "ws.h"

static void show_usage(char* argv0);

static int create_tcp_client_socket(char* hostname, char* port);

static int create_tcp_listening_socket(uint16_t port);

static ssize_t fd_write_full(int out_fd, const char* buffer, size_t len);

void* server_thread(void* arg);

void* client_thread(void* arg);

/*int test(int argc, char* argv[]) {
 if (argc <= 1) {
 struct ws* ws = ws_new(0, 1, 1);
 if (ws == NULL) {
 printf("ws_new() failed\n");
 return 1;
 }

 struct http_request http_req = { 0 };
 int ret = ws_do_handshake(ws, &http_req, NULL);
 if (ret < 0) {
 printf("ws_do_handshake() failed\n");

 ws_free(ws, 0);
 return 1;
 }

 printf("ws_do_handshake() returned %d\n", ret);
 printf("Host: %s, URI: %s\n", http_req.host, http_req.uri);

 ws_free(ws, 0);
 return 0;
 } else {
 struct ws* ws = ws_new(0, 1, 0);
 if (ws == NULL) {
 printf("ws_new() failed\n");
 return 1;
 }

 struct http_request http_req = { 0 };
 strcpy(http_req.uri, "/xd");
 strcpy(http_req.host, "xd.co");
 strcpy(http_req.sec_websocket_key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
 int ret = ws_do_handshake(ws, NULL, &http_req);
 if (ret < 0) {
 printf("ws_do_handshake() failed\n");

 ws_free(ws, 0);
 return 1;
 }

 printf("ws_do_handshake() returned %d\n", ret);

 ws_free(ws, 0);
 return 0;
 }
 }*/

struct server_thread_arg {
	int real_client_fd;
	char* server_hostname;
	char* server_port;
	pthread_barrier_t* barrier;
};

struct client_thread_arg {
	int real_client_fd;
	char* server_hostname;
	char* server_port;
	char* http_uri;
	char* http_host;
	pthread_barrier_t* barrier;
};

int main(int argc, char* argv[]) {
	int mode_server = 0;
	char* listening_port = NULL;
	char* dst_hostname = NULL;
	char* dst_port = NULL;
	char* http_uri = NULL;
	char* http_host = NULL;

	if (argc < 1) {
		show_usage(NULL);
		return 1;
	} else if (argc == 5 && strcmp("s", argv[1]) == 0) {
		mode_server = 1;
		listening_port = argv[2];
		dst_hostname = argv[3];
		dst_port = argv[4];
	} else if (argc == 7 && strcmp("c", argv[1]) == 0) {
		mode_server = 0;
		listening_port = argv[2];
		dst_hostname = argv[3];
		dst_port = argv[4];
		http_uri = argv[5];
		http_host = argv[6];
	} else {
		show_usage(argv[0]);
		return 1;
	}

	if (mode_server) {
		struct server_thread_arg server_thread_arg = { 0 };
		pthread_barrier_t* server_thread_barrier = malloc(
				sizeof(pthread_barrier_t));
		if (server_thread_barrier == NULL) {
			fprintf(stderr, "malloc() for barrier failed\n");

			return 1;
		}
		if (pthread_barrier_init(server_thread_barrier, NULL, 2) < 0) {
			fprintf(stderr, "pthread_barrier_init() failed\n");

			free(server_thread_barrier);
			return 1;
		}
		server_thread_arg.server_hostname = dst_hostname;
		server_thread_arg.server_port = dst_port;
		server_thread_arg.barrier = server_thread_barrier;

		uint16_t portnum = strtol(listening_port, NULL, 10);
		int sockfd = create_tcp_listening_socket(portnum);
		//printf("create_tcp_listening_socket() returned %d\n", sockfd);
		if (sockfd < 0) {
			fprintf(stderr, "create_tcp_listening_socket() failed\n");

			free(server_thread_barrier);
			return 1;
		}

		while (1) {
			int newfd = accept(sockfd, NULL, NULL);
			//printf("accept() returned %d\n", newfd);
			if (newfd < 0) {
				fprintf(stderr, "accept() failed\n");
				continue;
			}

			server_thread_arg.real_client_fd = newfd;

			pthread_t ws_forwarder_thread;
			if (pthread_create(&ws_forwarder_thread, NULL, server_thread,
					&server_thread_arg) != 0) {
				fprintf(stderr, "pthread_create() failed\n");

				close(newfd);
				continue;
			}

			/*must be called to avoid memory leaks*/
			pthread_detach(ws_forwarder_thread);
			/*allow pthread_detach() to be called before new thread terminates and
			 wait until the thread finishes copying arguments onto its own stack*/
			pthread_barrier_wait(server_thread_barrier);
			//printf("thread created for %d\n", newfd);
		}
	} else {
		struct client_thread_arg client_thread_arg = { 0 };
		pthread_barrier_t* client_thread_barrier = malloc(
				sizeof(pthread_barrier_t));
		if (client_thread_barrier == NULL) {
			fprintf(stderr, "malloc() for barrier failed\n");

			return 1;
		}
		if (pthread_barrier_init(client_thread_barrier, NULL, 2) < 0) {
			fprintf(stderr, "pthread_barrier_init() failed\n");

			free(client_thread_barrier);
			return 1;
		}
		client_thread_arg.server_hostname = dst_hostname;
		client_thread_arg.server_port = dst_port;
		client_thread_arg.http_uri = http_uri;
		client_thread_arg.http_host = http_host;
		client_thread_arg.barrier = client_thread_barrier;

		uint16_t portnum = strtol(listening_port, NULL, 10);
		int sockfd = create_tcp_listening_socket(portnum);
		//printf("create_tcp_listening_socket() returned %d\n", sockfd);
		if (sockfd < 0) {
			fprintf(stderr, "create_tcp_listening_socket() failed\n");

			free(client_thread_barrier);
			return 1;
		}

		while (1) {
			int newfd = accept(sockfd, NULL, NULL);
			//printf("accept() returned %d\n", newfd);
			if (newfd < 0) {
				fprintf(stderr, "accept() failed\n");
				continue;
			}

			client_thread_arg.real_client_fd = newfd;

			pthread_t ws_forwarder_thread;
			if (pthread_create(&ws_forwarder_thread, NULL, client_thread,
					&client_thread_arg) != 0) {
				fprintf(stderr, "pthread_create() failed\n");

				close(newfd);
				continue;
			}

			/*must be called to avoid memory leaks*/
			pthread_detach(ws_forwarder_thread);
			/*allow pthread_detach() to be called before new thread terminates and
			 wait until the thread finishes copying arguments onto its own stack*/
			pthread_barrier_wait(client_thread_barrier);
			//printf("thread created for %d\n", newfd);
		}
	}

	return 0;
}

static void show_usage(char* argv0) {
	if (argv0 == NULL) {
		argv0 = "program";
	}
	printf("Usage: %s s <listening port> <dst hostname> <dst port>\n", argv0);
	printf(
			"       %s c <listening port> <dst hostname> <dst port> <HTTP URI> <HTTP Host>\n",
			argv0);
}

static int create_tcp_client_socket(char* hostname, char* port) {
	struct addrinfo hints = { 0 };
	struct addrinfo* res = NULL;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int ret = getaddrinfo(hostname, port, &hints, &res);
	if (ret != 0) {
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(ret));
		return -1;
	}

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		fprintf(stderr, "socket() error\n");

		freeaddrinfo(res);
		return -1;
	}

	if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		fprintf(stderr, "connect() error\n");

		close(sockfd);
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);

	return sockfd;
}

static int create_tcp_listening_socket(uint16_t port) {
	int sockfd = socket(PF_INET6, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("create_tcp_listening_socket(): socket() error");
		return -1;
	}

	//work with both IPv4 and IPv6
	int zero = 0;
	int soret = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &zero,
			sizeof(zero));
	if (soret < 0) {
		perror("create_tcp_listening_socket(): setsockopt() error");
		fprintf(stderr,
				"create_tcp_listening_socket(): Server might not work with IPv4 clients\n");
	}

	//reuse port
	int one = 1;
	soret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	if (soret < 0) {
		perror("create_tcp_listening_socket(): setsockopt() error");
	}

	//bind
	struct sockaddr_in6 sockaddr = { 0 };
	sockaddr.sin6_addr = in6addr_any;
	sockaddr.sin6_family = AF_INET6;
	sockaddr.sin6_port = htons(port);
	int ret = bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr));
	if (ret < 0) {
		perror("create_tcp_listening_socket(): bind() error");
		close(sockfd);
		return -1;
	}

	//listen
	ret = listen(sockfd, 20);
	if (ret < 0) {
		perror("create_tcp_listening_socket(): listen() error");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

void* server_thread(void* arg) {
	struct server_thread_arg* oarg = arg;
	struct server_thread_arg args_cpy = { 0 };

	//copy arguments onto thread's own stack
	memcpy(&args_cpy, oarg, sizeof(args_cpy));

	//finish copying arguments onto own stack and wait until main thread has called pthread_detach()
	if (args_cpy.barrier != NULL) {
		pthread_barrier_wait(args_cpy.barrier);
	}

	//printf("thread ready for %d\n", args_cpy.real_client_fd);

	struct server_thread_arg* args = &args_cpy;

	int ret = -1;

	int real_server_fd = -1;
	struct ws* ws = NULL;

	real_server_fd = create_tcp_client_socket(args->server_hostname,
			args->server_port);
	if (real_server_fd < 0) {
		fprintf(stderr, "create_tcp_client_socket() failed for %d\n",
				args->real_client_fd);

		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		//pthread_exit(NULL);
		return NULL;
	}

	/*ignore SIGPIPE that can be possibly caused by writes to disconnected clients*/
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "signal() failed for %d\n", args->real_client_fd);

		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		shutdown(real_server_fd, SHUT_RDWR);
		close(real_server_fd);
		//pthread_exit(NULL);
		return NULL;
	}

	/*set sockets to non-blocking*/
	/*if (fcntl(args->real_client_fd, F_SETFL, O_NONBLOCK) < 0
	 || fcntl(real_server_fd, F_SETFL, O_NONBLOCK) < 0) {
	 fprintf(stderr, "fcntl() failed\n");

	 close(args->real_client_fd);
	 close(real_server_fd);
	 //pthread_exit(NULL);
	 return NULL;
	 }*/

	/*initialise ws object*/
	ws = ws_new(args->real_client_fd, args->real_client_fd, 1);
	if (ws == NULL) {
		fprintf(stderr, "ws_new() failed for %d\n", args->real_client_fd);

		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		shutdown(real_server_fd, SHUT_RDWR);
		close(real_server_fd);
		//pthread_exit(NULL);
		return NULL;
	}

	/*perform ws handshake*/
	struct http_request http_req = { 0 };
	ret = ws_do_handshake(ws, &http_req, NULL);
	if (ret < 0) {
		fprintf(stderr, "ws_do_handshake() failed for %d\n",
				args->real_client_fd);

		ws_free(ws, 0);
		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		shutdown(real_server_fd, SHUT_RDWR);
		close(real_server_fd);
		return NULL;
	}

	/*poll the two sockets*/
	struct pollfd ufds[2];
	ufds[0].fd = args->real_client_fd;
	ufds[0].events = POLLIN;
	ufds[1].fd = real_server_fd;
	ufds[1].events = POLLIN;

	char buffer[1024 * 64];
	while (1) {
		ret = poll(ufds, 2, -1);

		if (ret < 0) {
			fprintf(stderr, "poll() failed\n");
		} else if (ret == 0) {

		} else {
			//printf("poll() returned %d\n", ret);
			if (ufds[0].revents & POLLIN) {
				//printf("\n------------------\n");
				//printf("Data available from client\n");
				ssize_t ret = read(ufds[0].fd, buffer, sizeof(buffer));
				if (ret < 0) {
					fprintf(stderr, "read() failed for %d\n",
							args->real_client_fd);
					break;
				} else if (ret == 0) {
					//fprintf(stderr, "read() EOL for %d\n",
					//		args->real_client_fd);
					break;
				} else {
					if (fd_write_full(ufds[1].fd, buffer, ret) < 0) {
						fprintf(stderr, "fd_write_full() failed for %d\n",
								args->real_client_fd);
						break;
					}
				}
			}

			if (ufds[1].revents & POLLIN) {
				//printf("\n------------------\n");
				//printf("Data available from server\n");
				ssize_t ret = read(ufds[1].fd, buffer, sizeof(buffer));
				if (ret < 0) {
					fprintf(stderr, "read() failed for %d\n",
							args->real_client_fd);
					break;
				} else if (ret == 0) {
					//fprintf(stderr, "read() EOL for %d\n",
					//		args->real_client_fd);
					break;
				} else {
					if (fd_write_full(ufds[0].fd, buffer, ret) < 0) {
						fprintf(stderr, "fd_write_full() failed for %d\n",
								args->real_client_fd);
						break;
					}
				}
			}
		}
	}

	ws_free(ws, 0);
	shutdown(args->real_client_fd, SHUT_RDWR);
	close(args->real_client_fd);
	shutdown(real_server_fd, SHUT_RDWR);
	close(real_server_fd);
	//pthread_exit(NULL);
	//printf("thread for %d terminating\n", args->real_client_fd);
	return NULL;
}

void* client_thread(void* arg) {
	struct client_thread_arg* oarg = arg;
	struct client_thread_arg args_cpy = { 0 };

	//copy arguments onto thread's own stack
	memcpy(&args_cpy, oarg, sizeof(args_cpy));

	//finish copying arguments onto own stack and wait until main thread has called pthread_detach()
	if (args_cpy.barrier != NULL) {
		pthread_barrier_wait(args_cpy.barrier);
	}

	//printf("thread ready for %d\n", args_cpy.real_client_fd);

	struct client_thread_arg* args = &args_cpy;

	int ret = -1;

	int real_server_fd = -1;
	struct ws* ws = NULL;

	real_server_fd = create_tcp_client_socket(args->server_hostname,
			args->server_port);
	if (real_server_fd < 0) {
		fprintf(stderr, "create_tcp_client_socket() failed for %d\n",
				args->real_client_fd);

		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		//pthread_exit(NULL);
		return NULL;
	}

	/*ignore SIGPIPE that can be possibly caused by writes to disconnected clients*/
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "signal() failed for %d\n", args->real_client_fd);

		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		shutdown(real_server_fd, SHUT_RDWR);
		close(real_server_fd);
		//pthread_exit(NULL);
		return NULL;
	}

	/*set sockets to non-blocking*/
	/*if (fcntl(args->real_client_fd, F_SETFL, O_NONBLOCK) < 0
	 || fcntl(real_server_fd, F_SETFL, O_NONBLOCK) < 0) {
	 fprintf(stderr, "fcntl() failed\n");

	 close(args->real_client_fd);
	 close(real_server_fd);
	 //pthread_exit(NULL);
	 return NULL;
	 }*/

	/*initialise ws object*/
	ws = ws_new(real_server_fd, real_server_fd, 0);
	if (ws == NULL) {
		fprintf(stderr, "ws_new() failed for %d\n", args->real_client_fd);

		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		shutdown(real_server_fd, SHUT_RDWR);
		close(real_server_fd);
		//pthread_exit(NULL);
		return NULL;
	}

	/*perform ws handshake*/
	struct http_request http_req = { 0 };
	strncpy(http_req.uri, args->http_uri, sizeof(http_req.uri) - 1);
	strncpy(http_req.host, args->http_host, sizeof(http_req.host) - 1);
	strncpy(http_req.sec_websocket_key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=",
			sizeof(http_req.sec_websocket_key) - 1);
	ret = ws_do_handshake(ws, NULL, &http_req);
	if (ret < 0) {
		fprintf(stderr, "ws_do_handshake() failed for %d\n",
				args->real_client_fd);

		ws_free(ws, 0);
		shutdown(args->real_client_fd, SHUT_RDWR);
		close(args->real_client_fd);
		shutdown(real_server_fd, SHUT_RDWR);
		close(real_server_fd);
		return NULL;
	}

	/*poll the two sockets*/
	struct pollfd ufds[2];
	ufds[0].fd = args->real_client_fd;
	ufds[0].events = POLLIN;
	ufds[1].fd = real_server_fd;
	ufds[1].events = POLLIN;

	char buffer[1024 * 64];
	while (1) {
		ret = poll(ufds, 2, -1);

		if (ret < 0) {
			fprintf(stderr, "poll() failed\n");
		} else if (ret == 0) {

		} else {
			//printf("poll() returned %d\n", ret);
			if (ufds[0].revents & POLLIN) {
				//printf("\n------------------\n");
				//printf("Data available from client\n");
				ssize_t ret = read(ufds[0].fd, buffer, sizeof(buffer));
				if (ret < 0) {
					fprintf(stderr, "read() failed for %d\n",
							args->real_client_fd);
					break;
				} else if (ret == 0) {
					//fprintf(stderr, "read() EOL for %d\n",
					//		args->real_client_fd);
					break;
				} else {
					if (fd_write_full(ufds[1].fd, buffer, ret) < 0) {
						fprintf(stderr, "fd_write_full() failed for %d\n",
								args->real_client_fd);
						break;
					}
				}
			}

			if (ufds[1].revents & POLLIN) {
				//printf("\n------------------\n");
				//printf("Data available from server\n");
				ssize_t ret = read(ufds[1].fd, buffer, sizeof(buffer));
				if (ret < 0) {
					fprintf(stderr, "read() failed for %d\n",
							args->real_client_fd);
					break;
				} else if (ret == 0) {
					//fprintf(stderr, "read() EOL for %d\n",
					//		args->real_client_fd);
					break;
				} else {
					if (fd_write_full(ufds[0].fd, buffer, ret) < 0) {
						fprintf(stderr, "fd_write_full() failed for %d\n",
								args->real_client_fd);
						break;
					}
				}
			}
		}
	}

	ws_free(ws, 0);
	shutdown(args->real_client_fd, SHUT_RDWR);
	close(args->real_client_fd);
	shutdown(real_server_fd, SHUT_RDWR);
	close(real_server_fd);
	//pthread_exit(NULL);
	//printf("thread for %d terminating\n", args->real_client_fd);
	return NULL;
}

ssize_t fd_write_full(int out_fd, const char* buffer, size_t len) {
	size_t bytes_sent = 0;
	while (bytes_sent < len) {
		ssize_t wret = write(out_fd, buffer + bytes_sent, len - bytes_sent);
		int errn = errno;
		if (wret < 0) {
			//printf("fd_write_full: write() error, errno: %d\n", errn);
			if (errn > 0) {
				return -errn;
			} else {
				return -1;
			}
		}
		bytes_sent += wret;
	}

	return bytes_sent;
}
