#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/*ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
	static size_t (*orig_recvfrom)(int, void*, size_t, int, struct sockaddr*, socklen_t*) = NULL;
	orig_recvfrom = dlsym(RTLD_NEXT, "recvfrom");
	printf("\ncalling recvfrom(%d, %p, %u, %d, %p, %u)\n", sockfd, buf, len, flags, src_addr, *addrlen);
	ssize_t ret = orig_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
	if(src_addr->sa_family == AF_INET) {
		char ip[INET_ADDRSTRLEN] = {0};
		struct sockaddr_in* addr = (struct sockaddr_in*)src_addr;
		printf("\nreceived from %s:%u\n", inet_ntop(addr->sin_family, &(addr->sin_addr), ip, *addrlen), ntohs(addr->sin_port));
	}
	return ret;
}*/

ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dst_addr, socklen_t addrlen) {
	static size_t (*orig_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = NULL;
	orig_sendto = dlsym(RTLD_NEXT, "sendto");
	//printf("\ncalling sendto(%d, %p, %u, %d, %p, %u)\n", sockfd, buf, len, flags, dst_addr, addrlen);
	if(dst_addr->sa_family == AF_INET) {
		//char ip[INET_ADDRSTRLEN] = {0};
		//struct sockaddr_in* addr = (struct sockaddr_in*)dst_addr;
		//printf("\nsending to %s:%u\n", inet_ntop(addr->sin_family, &(addr->sin_addr), ip, addrlen), ntohs(addr->sin_port));
		char* buff = (char*)buf;
		/*if(flags == 16448) {
			putchar('\n');
			size_t i;
			for(i = 0; i < len; i++) {
				printf("%#02x ", (unsigned char)(buff[i]));
			}
			putchar('\n');
			putchar('\n');
		}*/
		if(buff[4] == 'I') {
			char ip[INET_ADDRSTRLEN] = {0};
			struct sockaddr_in* addr = (struct sockaddr_in*)dst_addr;
			printf("\nsending query reply to %s:%u\n", inet_ntop(addr->sin_family, &(addr->sin_addr), ip, addrlen), ntohs(addr->sin_port));
			size_t i;
			int nullcount = 0;
			for(i = 6; i < len-1; i++) {
				if(buff[i] == '\0') {
					++nullcount;
				}
				if(nullcount == 4) {
					//char oldnum = buff[i+3];
					//buff[i+3] = 5;
					//buff[i+4] = 20;
					buff[i+5] = 6;
					//printf("\nold number: %d, new number: %d\n", oldnum, buff[i+3]);
					break;
				}
			}
		}
	}
	return orig_sendto(sockfd, buf, len, flags, dst_addr, addrlen);
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
	static int (*orig_getaddrinfo)(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) = NULL;
	orig_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
	printf("\ncalling getaddrinfo(%s, %s, %p, %p)\n", node, service, hints, res);
	return orig_getaddrinfo(node, service, hints, res);
}