#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "netutil/netutil.h"

#define BUFFLEN 200

struct receiver_args {
	int fd;
	char* buffer;
	size_t bufferlen;
};

ssize_t getaline(FILE* fp, char* message, char** dest, size_t* destlen) {
	if(message != NULL) {
		printf("%s", message);	
	}
	
	ssize_t ret = getline(dest, destlen, fp);
	if(ret < 0) {
		return -1;
	}
	
	if(ret > 0 && (*dest)[ret-1] == '\n') {
		(*dest)[ret-1] = '\0';
	}
	
	return ret;
}

void* receiver(void* arg) {
	struct receiver_args* args = (struct receiver_args*)arg;
	
	ssize_t ret = 0;
	while(1) {
		ret = read(args->fd, args->buffer, args->bufferlen);
		if(ret < 0) {
			fprintf(stderr, "Error: read(): %s\n", strerror(errno));
			break;
		} else if(ret == 0) {
			break;
		}
		
		if(write(1, args->buffer, ret) < 0) {
			break;
		}
	}
	
	return NULL;
}

int main(int argc, char* argv[]) {
	char* s_buffer = calloc(BUFFLEN, 1);
	char* r_buffer = calloc(BUFFLEN, 1);
	char* hostname_a = calloc(BUFFLEN, 1);
	char* port_a = calloc(BUFFLEN, 1);
	if(s_buffer == NULL || r_buffer == NULL || hostname_a == NULL || port_a == NULL) {
		fprintf(stderr, "Error: failed to allocate memory\n");
		
		free(s_buffer);
		free(r_buffer);
		free(hostname_a);
		free(port_a);
		return 1;
	}
	
	char* hostname = NULL;
	char* port = NULL;
	
	if(argc == 3) {
		hostname = argv[1];
		port = argv[2];
	} else if(argc <= 1) {
		size_t hostname_a_len = BUFFLEN;
		ssize_t ret = getaline(stdin, "Hostname: ", &hostname_a, &hostname_a_len);
		if(ret < 0) {
			free(s_buffer);
			free(r_buffer);
			free(hostname_a);
			free(port_a);
			return 1;
		}
		hostname = hostname_a;
		
		size_t port_a_len = BUFFLEN;
		ret = getaline(stdin, "Port: ", &port_a, &port_a_len);
		if(ret < 0) {
			free(s_buffer);
			free(r_buffer);
			free(hostname_a);
			free(port_a);
			return 1;
		}
		port = port_a;
	} else {
		printf("Usage: %s <hostname> <port>\n", argv[0]);
		
		free(s_buffer);
		free(r_buffer);
		free(hostname_a);
		free(port_a);
		return 0;
	}
	
	struct addrinfo hints = {0};
	struct addrinfo* res = NULL;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
		
	int ret = getaddrinfo(hostname, port, &hints, &res);
	if(ret != 0) {
		fprintf(stderr, "Error: getaddrinfo(): %s\n", gai_strerror(ret));
		
		free(s_buffer);
		free(r_buffer);
		free(hostname_a);
		free(port_a);
		return 1;
	}
	
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		fprintf(stderr, "Error: socket(): %s\n", strerror(errno));
		
		freeaddrinfo(res);
		free(s_buffer);
		free(r_buffer);
		free(hostname_a);
		free(port_a);
		return 1;
	}
	
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		fprintf(stderr, "Error: connect(): %s\n", strerror(errno));
		
		close(sockfd);
		freeaddrinfo(res);
		free(s_buffer);
		free(r_buffer);
		free(hostname_a);
		free(port_a);
		return 1;
	}
		
	struct receiver_args rargs = {0};
	rargs.fd = sockfd;
	rargs.buffer = r_buffer;
	rargs.bufferlen = BUFFLEN;
	
	pthread_t receiver_thread;
	ret = pthread_create(&receiver_thread, NULL, receiver, &rargs);
	if(ret != 0) {
		fprintf(stderr, "Error: pthread_create(): %s\n", strerror(ret));
		
		close(sockfd);
		freeaddrinfo(res);
		free(s_buffer);
		free(r_buffer);
		free(hostname_a);
		free(port_a);
		return 1;
	}
	
	while(1) {
		ret = read(0, s_buffer, BUFFLEN);
		if(ret < 0) {
			fprintf(stderr, "Error: read(): %s\n", strerror(errno));
			break;
		} else if(ret == 0) {
			break;
		}
		
		if(tcp_write(sockfd, s_buffer, ret) < 0) {
			break;
		}
	}
	shutdown(sockfd, SHUT_WR);
	
	pthread_join(receiver_thread, NULL);
	shutdown(sockfd, SHUT_RD);
	
	close(sockfd);
	freeaddrinfo(res);
	free(s_buffer);
	free(r_buffer);
	free(hostname_a);
	free(port_a);
	return 0;
}