#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <wiringPi.h>

struct led_args {
	int* shouldrun;
	pthread_mutex_t* lock;
	pthread_cond_t* cond;
};

struct net_args {
	int fd;
	struct led_args* largs;
};

void* net(void* arg) {
	printf("net thread started\n");
	
	struct net_args* args = (struct net_args*) arg;
	
	char* buffer = calloc(30, 1);
	if(buffer == NULL) {
		close(args->fd);
		return NULL;
	}
	
	int len;
	while(1) {
		len = recv(args->fd, buffer, 30, 0);
		if(len == -1) {
			perror("recv()");
			close(args->fd);
			free(buffer);
			printf("net thread ending\n");
			return NULL;
		}
		
		if(len == 0 || buffer[0] == 'x') {
			close(args->fd);
			free(buffer);
			printf("net thread exiting\n");
			return NULL;
		} else {
			pthread_mutex_lock(args->largs->lock);
			*(args->largs->shouldrun) = atoi(buffer);
			pthread_cond_broadcast(args->largs->cond);
			pthread_mutex_unlock(args->largs->lock);
		}
		
	}
	
	return NULL;
}

void* led(void* arg) {
	printf("led thread started\n");
	
	struct led_args* args = (struct led_args*) arg;
	
	int pin = 18;
	
	wiringPiSetupGpio();
	pinMode(pin, PWM_OUTPUT);
	
	int i = 0;
	while(1) {
		pthread_mutex_lock(args->lock);
		while(!*(args->shouldrun)) {
			printf("led thread waiting\n");
			pwmWrite(pin, 1024);
			usleep(10000);
			pwmWrite(pin, 0);
			pthread_cond_wait(args->cond, args->lock);
			if(*(args->shouldrun)) {
				printf("led thread continuing\n");
			}
		}
		pthread_mutex_unlock(args->lock);
		
		for(i = 0 ; i <= 99; i++) {
			pwmWrite(pin, i);
			usleep(20000);
		}

		//sleep(2);
		
		for(i = 99; i >= 0; i--) {
			pwmWrite(pin, i);
			usleep(20000);
		}
		
		sleep(3);
	}
	
	return NULL;
}

int main() {
	struct addrinfo hints = {0};
	struct addrinfo* res;

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	int ret = getaddrinfo(NULL, "5252", &hints, &res);
	if(ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		return 1;
	}
	
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		perror("socket()");
		freeaddrinfo(res);
		return 1;
	}

	int mode = 0;
	if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &mode, sizeof(int)) == -1) {
		perror("setsockopt()");
		return 1;
	}

	int yes = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    		perror("setsockopt()");
    		return 1;
	}
	
	ret = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if(ret != 0) {
		perror("bind()");
		close(sockfd);
		freeaddrinfo(res);
		return 1;
	}
	
	ret = listen(sockfd, 20);
	if(ret != 0) {
		perror("listen()");
		close(sockfd);
		freeaddrinfo(res);
		return 1;
	}
	
	pthread_t led_thread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
		
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);
	
	int shouldrun = 1;
	
	struct led_args args = {&shouldrun, &lock, &cond};

	pthread_create(&led_thread, NULL, led, &args);

	while(1) {
		struct net_args nargs;
		pthread_t net_thread;
		struct sockaddr_storage client_addr;
		int new_fd;
		socklen_t addr_size = sizeof(client_addr);
		
		new_fd = accept(sockfd, (struct sockaddr*) &client_addr, &addr_size);
		
		nargs.fd = new_fd;
		nargs.largs = &args;
		
		pthread_create(&net_thread, NULL, net, &nargs);
	}
	
	return 0;
}
