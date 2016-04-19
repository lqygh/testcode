#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int tun_alloc(char* dev) {
	struct ifreq ifr;
	int fd, err;

	if((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));

	/* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
	*        IFF_TAP   - TAP device  
	*
	*        IFF_NO_PI - Do not provide packet information  
	*/ 
	ifr.ifr_flags = IFF_TAP; //create tap device
	
	ifr.ifr_mtu = 1234;
	
	if(dev != NULL) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	if((err = ioctl(fd, TUNSETIFF, (void*) &ifr)) < 0 ){
		close(fd);
		return err;
	}
	
	if((err = ioctl(fd, TUNSETPERSIST, 0)) < 0 ){
		close(fd);
		return err;
	}
	
	strcpy(dev, ifr.ifr_name);
	
	return fd;
}

int main(int argc, char* argv[]) {
	char name[IFNAMSIZ] = {0};
	
	memset(name, 0, IFNAMSIZ);
	int fd = tun_alloc(name);
	printf("%d: ", fd);
	printf("%s\n", name);
	//putchar('\n');
	
	int fd2 = socket(AF_INET, SOCK_STREAM, 0);
	if(fd2 == -1) {
		perror("");
		return -1;
	}
	
	struct ifreq req = {0};
	strncpy(req.ifr_name, name, IFNAMSIZ);
	
	int ret = ioctl(fd2, SIOCGIFMTU, &req);
	if(ret == -1) {
		perror("");
		return -1;
	}
	
	int mtu = req.ifr_mtu;
	printf("mtu is %d\n", mtu);
	
	if(argc > 1) {
		int newmtu = atoi(argv[1]);
		req.ifr_mtu = newmtu;
		
		ret = ioctl(fd2, SIOCSIFMTU, &req);
		if(ret == -1) {
			perror("failed to change mtu");
			return -1;
		}
		
		printf("new mtu is %d\n", newmtu);
	}
	
	close(fd2);
	
	sleep(10);
	close(fd);
	sleep(10);
	
	return 0;
}