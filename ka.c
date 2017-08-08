/*
 * ka.c
 *
 *  Created on: 9 Aug 2017
 *      Author: lqy
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char* argv[]) {
	if (argc < 4) {
		return 2;
	}

	struct addrinfo h = { 0 };
	h.ai_family = AF_UNSPEC;
	h.ai_socktype = SOCK_STREAM;

	struct addrinfo* r;

	int ret = getaddrinfo(argv[1], argv[2], &h, &r);
	if (ret < 0) {
		return 1;
	}

	int fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if (fd < 0) {
		return 1;
	}

	ret = connect(fd, r->ai_addr, r->ai_addrlen);
	if (ret < 0) {
		system(argv[3]);
	}

	close(fd);
	freeaddrinfo(r);

	return 0;
}
