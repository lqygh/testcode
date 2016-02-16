#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int create_udp_server_socket(char* port) {
	struct addrinfo hints;
	struct addrinfo* res;
		
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_PASSIVE; //important for server mode
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	
	int ret = getaddrinfo(NULL, port, &hints, &res);
	if(ret != 0) {
		printf("getaddrinfo(): %s\n", gai_strerror(ret));
		return -1;
	}
	
	int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fd == -1) {
		perror("socket()");
		return -1;
	}
	
	//necessary to ensure dual stack mode is enabled on some platforms
	int mode = 0;
	setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&mode, sizeof(mode));
	
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
	
	int buffersize = 100000;
	char* buffer = malloc(buffersize);
	if(buffer == NULL) {
		printf("failed to allocate memory\n");
		return 1;
	}
	bzero(buffer, buffersize);

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
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = 0;
	
		int ret = getaddrinfo(argv[2], argv[3], &hints, &res);
		if(ret != 0) {
			printf("getaddrinfo(): %s\n", gai_strerror(ret));
			return 1;
		}
	
		char ip[INET6_ADDRSTRLEN];
		printf("IP address string size: %lu\n", sizeof(ip));
		if(res->ai_family == AF_INET) {
			inet_ntop(res->ai_family, &((struct sockaddr_in* )(res->ai_addr))->sin_addr, ip, sizeof(ip));
			
		} else if(res->ai_family == AF_INET6) {
			inet_ntop(res->ai_family, &((struct sockaddr_in6* )(res->ai_addr))->sin6_addr, ip, sizeof(ip));
		
		} else {
			ip[0] = 'e';
			ip[1] = 'r';
			ip[2] = 'r';
			ip[3] = '\0';
		}
		printf("Destination IP address is %s\n", ip);
		
	
		int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(fd == -1) {
			perror("socket()");
			return 1;
		}
	
		int i = 0;
		for(i = 0; i < buffersize+1; i++) {
			ret = sendto(fd, buffer, i, 0, res->ai_addr, res->ai_addrlen);
			printf("sendto() returns %d\n", ret);
			if(ret == -1) {
				perror("sendto()");
				break;
			}
		}
	
		close(fd);
		freeaddrinfo(res);
		free(buffer);
	
		return 0;
		
	} else if(*argv[1] == 's') {
		printf("Server mode\n");
		
		int fd = create_udp_server_socket(argv[2]);
		if(fd == -1) {
			printf("create_udp_server_socket() failed\n");
			return 1;
		}
		
		struct sockaddr_storage from;
		socklen_t fromlen = sizeof(struct sockaddr_storage);
		char fromhost[NI_MAXHOST];
		char fromport[NI_MAXSERV];
		int ret;
		while(1) {
			ret = recvfrom(fd, buffer, buffersize, 0, (struct sockaddr*)&from, &fromlen);
			printf("recvfrom() returns %d ", ret);
			
			ret = getnameinfo((struct sockaddr*)&from, fromlen, fromhost, sizeof(fromhost), fromport, sizeof(fromport), NI_NUMERICHOST | NI_NUMERICSERV);
			if(ret != 0) {
			printf("\ngetnameinfo(): %s\n", gai_strerror(ret));
			} else {
				printf("source: %s:%s\n", fromhost, fromport);
			}
		}
		
	} else {
		printf("unknown argument %s\n", argv[1]);
		return 1;
	}

}