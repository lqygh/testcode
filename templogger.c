#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFERSIZE 1000

int main(int argc, char* argv[]) {
	if(argc < 5) {
		printf("Usage: %s <host name> <port> <log file> <interval>\n", argv[0]);
		return 1;
	}
	
	int interval = atoi(argv[4]);
	if(interval <= 0) {
		fprintf(stderr, "Invalid interval %d\n", interval);
		return 1;
	}
	
	char* buffer = malloc(BUFFERSIZE);
	if(buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory\n");
		return 1;
	}
	bzero(buffer, BUFFERSIZE);
		
	struct addrinfo hints;
	struct addrinfo* res;
	
	bzero(&hints, sizeof(hints));
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	
	int ret = getaddrinfo(argv[1], argv[2], &hints, &res);
	if(ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		freeaddrinfo(res);
		free(buffer);
		return 1;
	}
	
	char ip[INET6_ADDRSTRLEN];
	printf("IP address string maximum size: %lu\n", sizeof(ip));
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
		freeaddrinfo(res);
		free(buffer);
		return 1;
	}

	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt()");
	}
	
	FILE* fp = fopen(argv[3], "a");
	if(fp == NULL) {
		perror("fopen()");
		close(fd);
		freeaddrinfo(res);
		free(buffer);
		return 1;
	}
	
	int writecount = 0;
	struct timeval currenttime;
	while(1) {
		ret = sendto(fd, buffer, 1, 0, res->ai_addr, res->ai_addrlen);
		if(ret == -1) {
			perror("sendto()");
		}
		
		ret = recv(fd, buffer, BUFFERSIZE, 0);
		if(ret == -1) {
			perror("recv()");
		} else if(ret < 5) {
			fprintf(stderr, "Received message too short: %d\n", ret);
		} else {
			gettimeofday(&currenttime, NULL);
			fprintf(fp, "%llu,%d,%d,%d,%d,%d\n", (long long unsigned int)(currenttime.tv_sec), buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
			fflush(fp);
			writecount += 1;
			printf("\rWritten %llu,%d,%d,%d,%d,%d to file, count: %d", (long long unsigned int)(currenttime.tv_sec), buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], writecount);
			fflush(stdout);
		}
		
		sleep(interval);
	}

	close(fd);
	freeaddrinfo(res);
	free(buffer);
	fflush(fp);
	fclose(fp);

	return 0;
}