#include "netutil.h"

void macaddr_to_text(uint64_t macaddr, char* text, int textlen) {
	if(textlen >= MACADDRSTRLEN) {
		memset(text, 0, textlen);
		unsigned char* mac = (unsigned char*)&macaddr;
		snprintf(text, textlen, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

void sockaddr_ip_to_text(struct sockaddr* ai_addr, char* text, int textlen) {
	if(textlen > 0) {
		memset(text, 0, textlen);
	
		if(textlen < INET6_ADDRSTRLEN && ai_addr->sa_family == AF_INET6) {
			return;
		}
		
		if(textlen < INET_ADDRSTRLEN) {
			return;
		}
		
		if(ai_addr->sa_family == AF_INET) {
			inet_ntop(ai_addr->sa_family, &(((struct sockaddr_in*)ai_addr)->sin_addr), text, textlen);
		} else if(ai_addr->sa_family == AF_INET6) {
			inet_ntop(ai_addr->sa_family, &(((struct sockaddr_in6*)ai_addr)->sin6_addr), text, textlen);
		}
		
	}
}

in_port_t get_in_port(struct sockaddr* sa) {
    if(sa->sa_family == AF_INET) {
        return ntohs(((struct sockaddr_in*)sa)->sin_port);
    } else if(sa->sa_family == AF_INET6) {
		return ntohs(((struct sockaddr_in6*)sa)->sin6_port);
	} else {
		return 0;
	}
}

int is_ip_same(struct sockaddr* a, struct sockaddr* b) {
	if(a->sa_family == AF_INET && b->sa_family == AF_INET) {
		struct sockaddr_in* aa = (struct sockaddr_in*) a;
		struct sockaddr_in* bb = (struct sockaddr_in*) b;
		if(aa->sin_addr.s_addr == bb->sin_addr.s_addr) {
			return 1;
		} else {
			return 0;
		}
	}
	
	if(a->sa_family == AF_INET6 && b->sa_family == AF_INET6) {
		struct sockaddr_in6* aa = (struct sockaddr_in6*) a;
		struct sockaddr_in6* bb = (struct sockaddr_in6*) b;
		uint64_t* addra = (uint64_t*)&(aa->sin6_addr);
		uint64_t* addrb = (uint64_t*)&(bb->sin6_addr);
		//compare 64 bits at a time
		if(addra[0] == addrb[0] && addra[1] == addrb[1]) {
			return 1;
		} else {
			return 0;
		}
	}

	//return 0 in other cases
	return 0;
}

ssize_t tcp_read(int fd, void* buf, size_t count) {
	if(count <= 0) {
		return count;
	}
	
	size_t bytes_read = 0;
	size_t bytes_left = count;
	
	ssize_t ret;
	while(bytes_left != 0) {
		ret = read(fd, buf + bytes_read, bytes_left);
		
		if(ret < 0) {
			perror("read()");
			return ret;
		} else if(ret == 0) {
			return bytes_read;
		} else {
			bytes_read += ret;
			bytes_left -= ret;
		}
	}
	
	return bytes_read;
}

ssize_t tcp_write(int fd, void* buf, size_t count) {
	if(count <= 0) {
		return count;
	}
	
	size_t bytes_written = 0;
	size_t bytes_left = count;
	
	ssize_t ret;
	while(bytes_left != 0) {
		ret = write(fd, buf + bytes_written, bytes_left);
		
		if(ret < 0) {
			perror("write()");
			return ret;
		} else if(ret == 0) {
			return bytes_written;
		} else {
			bytes_written += ret;
			bytes_left -= ret;
		}
	}
	
	return bytes_written;
}

int create_udp_server_socket(char* port) {
	struct addrinfo hints = {0};
	struct addrinfo* res;
		
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
