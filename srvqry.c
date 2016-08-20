#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("Usage: %s <hostname> <port>\n", argv[0]);
		return 2;
	}
	
	int buffersize = 2000;
	char* buffer = malloc(buffersize);
	if(buffer == NULL) return 1;
	bzero(buffer, 2000);
	
	struct addrinfo hints;
	struct addrinfo* res;
	
	bzero(&hints, sizeof(hints));
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	
	char* hostname = argv[1];
	char* portname = argv[2];
	int ret = getaddrinfo(hostname, portname, &hints, &res);
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
	
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 500000;
	if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt()");
	}
	
	char query[] = {0xff, 0xff, 0xff, 0xff, 0x54, 0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45, 0x6e, 0x67, 0x69, 0x6e, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00};
	ret = sendto(fd, query, sizeof(query), 0, res->ai_addr, res->ai_addrlen);
	printf("sendto() returns %d\n", ret);
	ret = recv(fd, buffer, buffersize, 0);
	printf("recv() returns %d\n", ret);
	if(ret == -1) {
		perror("recv()");
		return 1;
	}
	
	putchar('\n');
	int i = 0;
	for(i = 0; i < ret; i++) {
		printf("%#02x ", (unsigned char)(buffer[i]));
	}
	putchar('\n');
	putchar('\n');
	
	unsigned char* header;
	unsigned char* protocol;
	char* name;
	char* map;
	char* folder;
	char* game;
	short* id;
	unsigned char* players;
	unsigned char* max_players;
	unsigned char* bots;
	char* server_type;
	char* environment;
	unsigned char* visibility;
	unsigned char* vac;
	
	header = (unsigned char*)buffer + 4;
	printf("Header: %u\n", *header);
	
	protocol = header + 1;
	printf("Protocol: %u\n", *protocol);
	
	name = (char*)protocol + 1;
	printf("Name: %s\n", name);
	printf("%lu\n", strlen(name));
	
	map = name + strlen(name) + 1;
	printf("Map: %s\n", map);
	//printf("%lu\n", strlen(map));
	
	folder = map + strlen(map) + 1;
	printf("Folder: %s\n", folder);
	//printf("%lu\n", strlen(folder));
	
	game = folder + strlen(folder) + 1;
	printf("Game: %s\n", game);
	//printf("%lu\n", strlen(game));
	
	id = (short*)(game + strlen(game) + 1);
	//printf("ID: %d\n", *id);
	
	players = (unsigned char*)(id) + 2;
	printf("Players: %u\n", *players);
	
	max_players = players + 1;
	printf("Max Players: %u\n", *max_players);
	
	bots = max_players + 1;
	printf("Bots: %u\n", *bots);

	server_type = (char*)bots + 1;
	printf("Server type: %u(%c)\n", *server_type, *server_type);

	environment = server_type + 1;
	printf("Environment: %u(%c)\n", *environment, *environment);

	visibility = (unsigned char*)environment + 1;
	printf("Visibility: %u\n", *visibility);
	
	vac = visibility + 1;
	printf("VAC: %u\n", *vac);
	
	close(fd);
	freeaddrinfo(res);
	free(buffer);
	return 0;
}