/*
 * httpserver.c
 *
 *  Created on: 8 Oct 2017
 *      Author: lqy
 */

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

#include "connection_handler.h"

#define DEFAULT_HTTP_DIR "."
#define READ_TIMEOUT_SEC 240

//locks for OpenSSL thread safety
static pthread_mutex_t* my_ssl_lock_array = NULL;

//day 1:  empty source file created
//day 2:  basic socket(), bind(), listen(), accept() and thread structure
//day 3:  basic HTTP GET parsing and dummy response
//day 4:  basic file IO for HTTP GET response
//day 5:  HTTP GET URI processing
//day 6:  ???
//day 7:  %xx percent-encoded URI decoding
//day 8:  sanity check for directory traversal
//day 9:  code refactoring
//day 10: code refactoring, range request implemented
//day 11: virtual host implemented
//day 12: ???
//day 13: directory listing
//day 14: ???
//day 15: ???
//day 16: ???
//day 17: switched from FILE* to OpenSSL BIO*
//day 18: TLS support implemented
//day 19: TLS bug fixes

static void show_usage(char* argv0);

SSL_CTX* init_ssl_ctx(char* cert, char* key);

static void my_ssl_lock_callback(int mode, int type, char* file, int line);

static unsigned long my_ssl_get_thread_id(void);

static int my_ssl_init_locks(void);

static void my_ssl_free_locks(void);

static int create_tcp_listening_socket(uint16_t port);

static void* handle_tls_accept(void* args);

int main(int argc, char* argv[]) {
	if (argc < 2) {
		if (argc > 0) {
			show_usage(argv[0]);
		} else {
			show_usage(NULL);
		}
		return 1;
	}

	//arguments
	int pflag = 0; //port number
	int tflag = 0; //TLS port number
	int cflag = 0; //TLS certificate file
	int kflag = 0; //TLS key file

	//parameters
	uint16_t http_port = -1;
	char* http_dir = DEFAULT_HTTP_DIR;
	struct vhost_table* vhost_table = NULL;

	uint16_t tls_port = -1;
	char* tls_cert_file = NULL;
	char* tls_key_file = NULL;

	//parse command line arguments
	int option = 0;
	while ((option = getopt(argc, argv, "d:p:v:t:c:k:")) != -1) {
		switch (option) {
		case 'd': {
			http_dir = optarg;
			break;
		}

		case 'p': {
			pflag = 1;
			long int http_port_tmp = strtol(optarg, NULL, 10);
			if (http_port_tmp < 0 || http_port_tmp > 65535) {
				fprintf(stderr, "Port number %ld is invalid\n", http_port_tmp);
				return 1;
			}
			http_port = (uint16_t) http_port_tmp;
			break;
		}

		case 'v': {
			vhost_table = vhost_table_new(optarg);
			if (vhost_table == NULL) {
				fprintf(stderr,
						"Failed to process virtual host configuration file %s\n",
						optarg);
				return 1;
			}
			break;
		}

		case 't': {
			tflag = 1;
			long int tls_port_tmp = strtol(optarg, NULL, 10);
			if (tls_port_tmp < 0 || tls_port_tmp > 65535) {
				fprintf(stderr, "TLS port number %ld is invalid\n",
						tls_port_tmp);
				return 1;
			}
			tls_port = (uint16_t) tls_port_tmp;
			break;
		}

		case 'c': {
			cflag = 1;
			tls_cert_file = optarg;
			break;
		}

		case 'k': {
			kflag = 1;
			tls_key_file = optarg;
			break;
		}

		default: {
			show_usage(argv[0]);
			return 1;
		}
		}
	}

	if (pflag != 1 && tflag != 1) {
		fprintf(stderr, "Either -p or -t option is required\n");
		return 1;
	}

	//TLS
	if (tflag == 1) {
		if (cflag != 1 || kflag != 1) {
			fprintf(stderr,
					"TLS certificate or key unspecified, please specify certificate with -c option and key with -k option\n");

			return 1;
		}

		int tls_sockfd = create_tcp_listening_socket(tls_port);
		if (tls_sockfd < 0) {
			fprintf(stderr, "TLS create_tcp_listening_socket() failed\n");

			vhost_table_destroy(vhost_table);
			return 1;
		}

		//initialise OpenSSL
		SSL_library_init();
		if (my_ssl_init_locks() < 0) {
			fprintf(stderr, "TLS my_ssl_init_locks() failed\n");

			vhost_table_destroy(vhost_table);
			close(tls_sockfd);
			return 1;
		}

		//thread
		struct https_handler_arg https_handler_arg = { 0 };
		struct connection_handler_arg* connection_handler_arg =
				&(https_handler_arg.connection_handler_arg);
		pthread_barrier_t* https_thread_barrier = malloc(
				sizeof(pthread_barrier_t));
		if (https_thread_barrier == NULL) {
			fprintf(stderr, "TLS malloc() for barrier failed\n");

			vhost_table_destroy(vhost_table);
			my_ssl_free_locks();
			close(tls_sockfd);
			return 1;
		}
		if (pthread_barrier_init(https_thread_barrier, NULL, 2) < 0) {
			fprintf(stderr, "TLS pthread_barrier_init() failed\n");

			free(https_thread_barrier);
			vhost_table_destroy(vhost_table);
			my_ssl_free_locks();
			close(tls_sockfd);
			return 1;
		}

		//initialise OpenSSL context
		SSL_CTX* ssl_ctx = init_ssl_ctx(tls_cert_file, tls_key_file);
		if (ssl_ctx == NULL) {
			fprintf(stderr, "TLS init_ssl_ctx() failed\n");

			free(https_thread_barrier);
			vhost_table_destroy(vhost_table);
			my_ssl_free_locks();
			pthread_barrier_destroy(https_thread_barrier);
			close(tls_sockfd);
			return 1;
		}
		SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_OFF);

		https_handler_arg.ssl_ctx = ssl_ctx;
		connection_handler_arg->socket_fd = tls_sockfd;
		connection_handler_arg->barrier = https_thread_barrier;
		connection_handler_arg->default_dir = http_dir;
		connection_handler_arg->vhost_table = vhost_table;

		//create accept thread
		pthread_t tls_accept_thread;
		if (pthread_create(&tls_accept_thread, NULL, handle_tls_accept,
				&https_handler_arg) != 0) {
			fprintf(stderr, "TLS pthread_create() failed\n");

			free(https_thread_barrier);
			SSL_CTX_free(ssl_ctx);
			vhost_table_destroy(vhost_table);
			my_ssl_free_locks();
			pthread_barrier_destroy(https_thread_barrier);
			close(tls_sockfd);
			return 1;
		}

		//must be called to avoid memory leaks
		pthread_detach(tls_accept_thread);

		/*allow pthread_detach() to be called before new thread terminates and
		 wait until the thread finishes copying arguments onto its own stack*/
		pthread_barrier_wait(https_thread_barrier);
	}

	//normal HTTP
	if (pflag == 1) {
		//create socket
		int sockfd = create_tcp_listening_socket(http_port);
		if (sockfd < 0) {
			fprintf(stderr, "create_tcp_listening_socket() failed\n");

			return 1;
		}

		//thread
		struct connection_handler_arg http_handler_arg = { 0 };
		pthread_t http_thread;
		pthread_barrier_t* http_thread_barrier = malloc(
				sizeof(pthread_barrier_t));
		if (http_thread_barrier == NULL) {
			fprintf(stderr, "malloc() for barrier failed\n");

			close(sockfd);
			return 1;
		}
		if (pthread_barrier_init(http_thread_barrier, NULL, 2) < 0) {
			fprintf(stderr, "pthread_barrier_init() failed\n");

			free(http_thread_barrier);
			close(sockfd);
			return 1;
		}
		http_handler_arg.barrier = http_thread_barrier;
		http_handler_arg.default_dir = http_dir;
		http_handler_arg.vhost_table = vhost_table;

		printf("Server now accepting HTTP connections\n");
		while (1) {
			int newfd = accept(sockfd, NULL, NULL);
			if (newfd < 0) {
				fprintf(stderr, "accept() failed\n");

				continue;
			}

			{
				struct timeval timeout;
				timeout.tv_sec = READ_TIMEOUT_SEC;
				timeout.tv_usec = 0;

				setsockopt(newfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
						sizeof(timeout));
			}

			http_handler_arg.socket_fd = newfd;
			if (pthread_create(&http_thread, NULL, http_handle_connection,
					&http_handler_arg) != 0) {
				fprintf(stderr, "pthread_create() failed\n");

				close(newfd);
				continue;
			}

			//must be called to avoid memory leaks
			pthread_detach(http_thread);

			/*allow pthread_detach() to be called before new thread terminates and
			 wait until the thread finishes copying arguments onto its own stack*/
			pthread_barrier_wait(http_thread_barrier);
		}
	}

	//sleep
	while (1) {
		pause();
	}

	//the following line should not be reachable
	return 0;
}

static void show_usage(char* argv0) {
	if (argv0 == NULL) {
		argv0 = "program";
	}
	printf("Usage: %s\n"
			"\t-d <default directory>\t(optional)\n\n"
			"\t-p <HTTP listening port>\t(required if TLS not enabled)\n\n"
			"\t-v <virtual host configuration file>\t(optional)\n\n"
			"\t-t <TLS listening port>\t(required if HTTP not enabled)\n\n"
			"\t-c <TLS certificate file>\t(required if TLS enabled)\n\n"
			"\t-k <TLS key file>\t(required if TLS enabled)\n\n", argv0);
}

// https://aticleworld.com/ssl-server-client-using-openssl-in-c/
SSL_CTX* init_ssl_ctx(char* cert, char* key) {
	SSL_CTX *ctx;

	OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
	SSL_load_error_strings(); /* load all error messages */
	ctx = SSL_CTX_new(TLSv1_2_server_method()); /* create new context from method */
	if (ctx == NULL) {
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	/* set the local certificate from CertFile */
	if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}
	/* verify private key */
	if (!SSL_CTX_check_private_key(ctx)) {
		fprintf(stderr,
				"Private key %s does not match the public certificate %s\n",
				key, cert);
		SSL_CTX_free(ctx);
		return NULL;
	}

	return ctx;
}

// https://curl.haxx.se/libcurl/c/threaded-ssl.html
static void my_ssl_lock_callback(int mode, int type, char* file, int line) {
	(void) file;
	(void) line;
	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&(my_ssl_lock_array[type]));
	} else {
		pthread_mutex_unlock(&(my_ssl_lock_array[type]));
	}
}

static unsigned long my_ssl_get_thread_id(void) {
	unsigned long ret;

	ret = (unsigned long) pthread_self();
	return ret;
}

static int my_ssl_init_locks(void) {
	my_ssl_lock_array = (pthread_mutex_t *) OPENSSL_malloc(
			CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	if (my_ssl_lock_array == NULL) {
		return -1;
	}

	for (int i = 0; i < CRYPTO_num_locks(); i++) {
		if (pthread_mutex_init(&(my_ssl_lock_array[i]), NULL) != 0) {
			for (int j = 0; j < i; j++) {
				pthread_mutex_destroy(&(my_ssl_lock_array[j]));
			}
			OPENSSL_free(my_ssl_lock_array);
			return -1;
		}
	}

	CRYPTO_set_id_callback((unsigned long (*)()) my_ssl_get_thread_id);
	CRYPTO_set_locking_callback((void (*)()) my_ssl_lock_callback);

	return 0;
}

static void my_ssl_free_locks(void) {
	CRYPTO_set_locking_callback(NULL);
	for (int i = 0; i < CRYPTO_num_locks(); i++) {
		pthread_mutex_destroy(&(my_ssl_lock_array[i]));
	}

	OPENSSL_free(my_ssl_lock_array);

	my_ssl_lock_array = NULL;
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

static void* handle_tls_accept(void* args) {
	struct https_handler_arg* oarg = (struct https_handler_arg*) args;
	struct https_handler_arg arg = { 0 };

	//copy arguments onto thread's own stack
	memcpy(&arg, oarg, sizeof(arg));

	//finish copying arguments onto own stack and wait until main thread has called pthread_detach()
	pthread_barrier_wait(arg.connection_handler_arg.barrier);

	//loop for accept()
	pthread_t https_thread;
	int tls_sockfd = arg.connection_handler_arg.socket_fd;
	printf("Server now accepting TLS connections\n");
	while (1) {
		int newfd = accept(tls_sockfd, NULL, NULL);
		if (newfd < 0) {
			fprintf(stderr, "TLS accept() failed\n");

			continue;
		}

		{
			struct timeval timeout;
			timeout.tv_sec = READ_TIMEOUT_SEC;
			timeout.tv_usec = 0;

			setsockopt(newfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
					sizeof(timeout));
		}

		arg.connection_handler_arg.socket_fd = newfd;
		if (pthread_create(&https_thread, NULL, https_handle_connection, &arg)
				!= 0) {
			fprintf(stderr, "TLS pthread_create() failed\n");

			close(newfd);
			continue;
		}

		//must be called to avoid memory leaks
		pthread_detach(https_thread);

		/*allow pthread_detach() to be called before new thread terminates and
		 wait until the thread finishes copying arguments onto its own stack*/
		pthread_barrier_wait(arg.connection_handler_arg.barrier);
	}

	//the following line should not be reachable
	pthread_exit(NULL);
}
