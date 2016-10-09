#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

struct net_receiver_args {
	int sockfd;
};

void print_usage(FILE* fp) {
	fprintf(fp, "Usage:\n\n");
	fprintf(fp, "  -h\t\tshow usage\n");
	fprintf(fp, "  -s\t\tspecify server hostname\n");
	fprintf(fp, "  -p\t\tspecify server port\n");
	fprintf(fp, "  -d\t\tspecify delay in seconds(optional)\n");
	fprintf(fp, "\n");
}

void* net_receiver(void* arg) {	
	struct net_receiver_args* args = (struct net_receiver_args*) arg;
	int sockfd = args->sockfd;
	
	char buffer[100] = {0};
	int read = 0, written = 0;
	while(1) {
		read = recv(sockfd, buffer, sizeof(buffer), 0);
		if(read == -1) {
			fprintf(stderr, "receiver thread: recv() failed: %s\n", strerror(errno));
			break;
		} else if(read == 0) {
			fprintf(stderr, "receiver thread: peer has performed an orderly shutdown\n");
			break;
		} else {
			written = write(1, buffer, read);
			if(written == -1) {
				fprintf(stderr, "receiver thread: write() failed: %s\n", strerror(errno));
				break;
			}
		}
	}
		
	return NULL;
}

int main(int argc, char* argv[]) {
	int sflag = 0, pflag = 0, dflag = 0;
	char* server_address = NULL;
	char* port = NULL;
	int delay = 0;
	
	int opt = 0;
	while((opt = getopt(argc, argv, "hs:p:d:")) != -1) {
		switch(opt) {
			case 'h':
				print_usage(stdout);
				return 0;
			case 's':
				sflag = 1;
				server_address = optarg;
				break;
			case 'p':
				pflag = 1;
				port = optarg;
				break;
			case 'd':
				dflag = 1;
				delay = atoi(optarg);
				break;
			default:
				print_usage(stderr);
				return 1;
		}
	}
	
	if(sflag == 0 || pflag == 0) {
		print_usage(stderr);
		return 1;
	}
	
	if(dflag == 0) {
		delay = 0;
	} else {
		if(delay < 0) {
			fprintf(stderr, "the delay specified by -d must be >= 0\n");
			return 1;
		}
	}
	
	struct addrinfo hints = {0};
	struct addrinfo* res = NULL;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	int ret = getaddrinfo(server_address, port, &hints, &res);
	if(ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		return 1;
	}
	
	int len = 0, buffsize = 1000;
	char* buffer = calloc(buffsize, 1);
	if(buffer == NULL) {
		fprintf(stderr, "calloc() failed\n");
		
		freeaddrinfo(res);
		return 1;
	}
	
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		perror("socket()");
		
		freeaddrinfo(res);
		free(buffer);
		return 1;
	}
	
	{
		int one = 1;
		if(setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)) == -1) {
			perror("setsockopt()");
		}
	}
	
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		perror("connect()");
	
		freeaddrinfo(res);
		free(buffer);
		close(sockfd);
		return 1;
	}
	
	struct net_receiver_args narg;
	narg.sockfd = sockfd;

	pthread_t net_receiver_thread;
	if(pthread_create(&net_receiver_thread, NULL, net_receiver, &narg) != 0) {
		fprintf(stderr, "pthread_create() for receiver thread failed: %s\n", strerror(errno));
		
		freeaddrinfo(res);
		free(buffer);
		close(sockfd);
		return 1;
	}
	
	int i = 0, sent = 0;
	while(1) {
		//read from stdin to buffer
		len = read(0, buffer, buffsize);
		if(len == 0) {
			fprintf(stderr, "read() reaches end of file\n");
			pthread_join(net_receiver_thread, NULL);
			return 0;
		} else if(len < 0) {
			perror("read()");
			return 1;
		} else {
			//send from buffer to socket
			for(i = 0; i < len; i++) {
				//delay
				sleep(delay);
				//send byte by byte
				sent = send(sockfd, &buffer[i], 1, 0);
				if(sent == -1) {
					perror("send()");
					return 1;
				} else if(sent == 0) {
					//resend that byte
					i--;
				}
			}
		}
	}
	
	freeaddrinfo(res);
	free(buffer);
	close(sockfd);
	
	return 0;
}
