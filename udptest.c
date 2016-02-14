#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int create_server_socket(char* port) {
	struct addrinfo hints;
	struct addrinfo* res;
	
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_PASSIVE; //important for server mode
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	
	int ret = getaddrinfo(NULL, port, &hints, &res);
	if(ret != 0) {
		printf("getaddrinfo(): %s\n", gai_strerror(ret));
		return -1;
	}
	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1) {
		perror("socket()");
		return -1;
	}
	
	ret = bind(fd, res->ai_addr, res->ai_addrlen);
	if(ret != 0) {
		perror("bind()");
		return -1;
	}
	
	return fd;
}

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("%s c <hostname> <port>\n", argv[0]);
		printf("%s s <port>\n", argv[0]);
		return 1;
	}
	
	int buffersize = 2000;
	char* buffer = malloc(buffersize);
	if(buffer == NULL) {
		printf("failed to allocate memory\n");
		return 1;
	}
	bzero(buffer, 2000);

	if(*argv[1] == 'c') {
		printf("Client mode\n");
		if(argc < 4) {
			printf("%s c <hostname> <port>\n", argv[0]);
			return 1;
		}
		
		struct addrinfo hints;
		struct addrinfo* res;
	
		bzero(&hints, sizeof(hints));
		hints.ai_flags = 0;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = 0;
	
		int ret = getaddrinfo(argv[2], argv[3], &hints, &res);
		if(ret != 0) {
			printf("getaddrinfo(): %s\n", gai_strerror(ret));
			return 1;
		}
	
		char ip[INET_ADDRSTRLEN];
		printf("IP address string size: %lu\n", sizeof(ip));
		inet_ntop(AF_INET, &((struct sockaddr_in* )(res->ai_addr))->sin_addr, ip, sizeof(ip));
		printf("Destination IP address is %s\n", ip);
	
		int fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(fd == -1) {
			perror("socket()");
			return 1;
		}
	
		int i = 0;
		for(i = 0; i < buffersize; i++) {
			ret = sendto(fd, buffer, i, 0, res->ai_addr, res->ai_addrlen);
			printf("sendto() returns %d\n", ret);
		}
	
		close(fd);
		freeaddrinfo(res);
		free(buffer);
	
		return 0;
		
	} else if(*argv[1] == 's') {
		printf("Server mode\n");
		
		int fd = create_server_socket(argv[2]);
		if(fd == -1) {
			printf("create_server_socket() failed\n");
			return 1;
		}
		
		while(1) {
			int ret = recv(fd, buffer, buffersize, 0);
			printf("recv() returns %d\n", ret);
		}
		
	} else {
		printf("unknown argument %s\n", argv[1]);
		return 1;
	}

}