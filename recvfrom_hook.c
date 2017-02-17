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

static unsigned char query_counter = 0;

static inline unsigned char is_packet_queryreply(void* data, size_t len) {
	if(data == NULL || len < 23) {
		return 0;
	}
	
	char* buff = (char*)data;
	if(buff[4] == 'I' && len >= 23 && buff[22] != '\0') {
			if(buff[17] == 'C' && buff[18] == 'o' && buff[19] == '-' && buff[20] == 'o' && buff[21] == 'p') {
				return 1;
			}
	}
	
	return 0;
}

static inline unsigned char should_modify(struct sockaddr_in* sa) {
	unsigned char* ip = (unsigned char*)&(sa->sin_addr.s_addr);
	unsigned char should_modify = 0;
		if(ip[0] == 85 && ip[1] == 188) {
			should_modify = 1;
		} else if(ip[0] == 92 && ip [1] == 232) {
			should_modify = 1;
		} else if(ip[0] == 176 && ip [1] == 56) { //176.56.237.14
			should_modify = 1;
		} 
	return should_modify;
}

ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dst_addr, socklen_t addrlen) {
	size_t (*orig_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = NULL;
	orig_sendto = dlsym(RTLD_NEXT, "sendto");
	//printf("\ncalling sendto(%d, %p, %u, %d, %p, %u)\n", sockfd, buf, len, flags, dst_addr, addrlen);
	if(dst_addr->sa_family == AF_INET) {
		char* buff = (char*)buf;
		struct sockaddr_in* addr = (struct sockaddr_in*)dst_addr;
		
		//char ip[INET_ADDRSTRLEN] = {0};
		//struct sockaddr_in* addr = (struct sockaddr_in*)dst_addr;
		//printf("\nsending to %s:%u\n", inet_ntop(addr->sin_family, &(addr->sin_addr), ip, addrlen), ntohs(addr->sin_port));
		/*if(flags == 16448) {
			putchar('\n');
			size_t i;
			for(i = 0; i < len; i++) {
				printf("%#02x ", (unsigned char)(buff[i]));
			}
			putchar('\n');
			putchar('\n');
		}*/
		
		if(should_modify(addr) && is_packet_queryreply(buff, len)) {
			char orig_22 = buff[22];
			char* server_name = &buff[20];
			snprintf(server_name, 3, "%02x", query_counter);
			buff[22] = orig_22;
			query_counter += 1;
			
			char ip[INET_ADDRSTRLEN] = {0};
			printf("\nsending modified query reply to %s:%u, query_counter: %u (hex: %02x)\n", inet_ntop(addr->sin_family, &(addr->sin_addr), ip, addrlen), ntohs(addr->sin_port), query_counter - 1, query_counter - 1);
			size_t i;
			int nullcount = 0;
			for(i = 6; i < len-1; i++) {
				if(buff[i] == '\0') {
					nullcount += 1;
				}
				if(nullcount == 4) {
					//char oldnum = buff[i+3];
					buff[i+3] = 16;
					buff[i+4] = 32;
					buff[i+5] = 6;
					//printf("\nold number: %d, new number: %d\n", oldnum, buff[i+3]);
					break;
				}
			}
		} else if(buff[4] == 'I') {
			char ip[INET_ADDRSTRLEN] = {0};
			printf("\nsending unmodified query reply to %s:%u, query_counter: %u (hex: %02x)\n", inet_ntop(addr->sin_family, &(addr->sin_addr), ip, addrlen), ntohs(addr->sin_port), query_counter - 1, query_counter - 1);
		}
	}
	
	return orig_sendto(sockfd, buf, len, flags, dst_addr, addrlen);
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
	int (*orig_getaddrinfo)(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) = NULL;
	orig_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
	printf("\ncalling getaddrinfo(%s, %s, %p, %p)\n", node, service, hints, res);
	return orig_getaddrinfo(node, service, hints, res);
}