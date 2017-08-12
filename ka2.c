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
	close(fd);
	freeaddrinfo(r);

	if (ret < 0) {
		system(argv[3]);
	} else {
		ret = system("ping -c 4 10.1.5.1");
		if (ret == 0) {
			ret = system("curl -v --connect-timeout 12 https://g.co");
			if (ret > 0) {
				system(argv[3]);
			}
		}
	}

	return 0;
}
