#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("usage: %s <ifr_name>\n", argv[0]);
		printf("usage: %s <ifr_name> <new mtu>\n", argv[0]);
		return 1;
	}
	
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		perror("");
		return 1;
	}
	
	struct ifreq req = {0};
	strncpy(req.ifr_name, argv[1], IFNAMSIZ);
	
	int ret = ioctl(fd, SIOCGIFMTU, &req);
	if(ret == -1) {
		perror("");
		return 1;
	}
	
	int mtu = req.ifr_mtu;
	printf("mtu is %d\n", mtu);
	
	
	if(argc == 3) {
		int newmtu = atoi(argv[2]);
		req.ifr_mtu = newmtu;
		
		ret = ioctl(fd, SIOCSIFMTU, &req);
		if(ret == -1) {
			perror("failed to change mtu");
			return 1;
		}
		
		printf("new mtu is %d\n", newmtu);
	}
	
	
	close(fd);
	return 0;
}