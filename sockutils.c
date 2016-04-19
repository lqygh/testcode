#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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
		//printf("getaddrinfo(): %s\n", gai_strerror(ret));
		return -1;
	}
	
	int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fd == -1) {
		//perror("socket()");
		return -1;
	}
	
	//necessary to ensure dual stack mode is enabled on some platforms
	int mode = 0;
	setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&mode, sizeof(mode));
	
	ret = bind(fd, res->ai_addr, res->ai_addrlen);
	if(ret != 0) {
		//perror("bind()");
		return -1;
	}
	
	return fd;
}