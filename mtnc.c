#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

struct net_args {
	int id;
	struct addrinfo* addrinfo;
	pthread_mutex_t lock;
};

void print_usage(FILE* fp) {
	fprintf(fp, "Usage:\n\n");
	fprintf(fp, "  -h\t\tshow usage\n");
	fprintf(fp, "  -s\t\tspecify server hostname\n");
	fprintf(fp, "  -p\t\tspecify server port\n");
	fprintf(fp, "  -n\t\tspecify number of threads(optional)\n");
	fprintf(fp, "\n");
}

void* net(void* arg) {	
	struct net_args* args = (struct net_args*) arg;
	int id = args->id;
	struct addrinfo* res = args->addrinfo;
	pthread_mutex_t lock = args->lock;
	
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		pthread_mutex_lock(&lock);
		fprintf(stderr, "%d: socket() failed: %s\n", id, strerror(errno));
		pthread_mutex_unlock(&lock);
		
		return NULL;
	}
	
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		pthread_mutex_lock(&lock);
		fprintf(stderr, "%d: connect() failed: %s\n", id, strerror(errno));
		pthread_mutex_unlock(&lock);
		
		close(sockfd);
		return NULL;
	}
	
	/*char* buffer = calloc(30, 1);
	if(buffer == NULL) {
		fprintf(stderr, "%d: calloc() failed\n", id);
		
		close(sockfd);
		return NULL;
	}*/
	
	//printf("%d: ready\n", args->id);
	
	//printf("%d: ptr arg is %p\n", args->id, arg);
	//printf("%d: ptr addrinfo is %p\n", args->id, args->addrinfo);
	
	/*int len;
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
		
	}*/
	
	sleep(20);
	
	//printf("%d: ending\n", args->id);
	close(sockfd);
	//free(buffer);
	
	return NULL;
}

int main(int argc, char* argv[]) {
	int sflag = 0, pflag = 0, nflag = 0;
	char* server_address = NULL;
	char* port = NULL;
	int thread_num = 0;
	
	int opt = 0;
	while((opt = getopt(argc, argv, "hs:p:n:")) != -1) {
		switch(opt) {
			case 'h':
				print_usage(stdout);
				return 0;
			case 's':
				sflag = 1;
				server_address = optarg;
				break;
			case 'p':
				pflag = 1;
				port = optarg;
				break;
			case 'n':
				nflag = 1;
				thread_num = atoi(optarg);
				break;
			default:
				print_usage(stderr);
				return 1;
		}
	}
	
	if(sflag == 0 || pflag == 0) {
		print_usage(stderr);
		return 1;
	}
	
	if(nflag == 0) {
		thread_num = 1;
	} else {
		if(thread_num <= 0) {
			fprintf(stderr, "the number of threads specified by -n must be >= 1\n");
			return 1;
		}
	}
	
	struct addrinfo hints = {0};
	struct addrinfo* res = NULL;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	int ret = getaddrinfo(server_address, port, &hints, &res);
	if(ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		return 1;
	}

	/*if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
    		perror("setsockopt()");
	}
		
	pthread_t led_thread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
		
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);
	
	int shouldrun = 1;*/
	
	pthread_mutex_t lock;
	if(pthread_mutex_init(&lock, NULL) != 0) {
		fprintf(stderr, "pthread_mutex_init() failed\n");
		
		freeaddrinfo(res);
		return 1;
	}
	
	pthread_t* net_threads = calloc(sizeof(pthread_t), thread_num);
	struct net_args* nargs = calloc(sizeof(struct net_args), thread_num);
	if(net_threads == NULL || nargs == NULL) {
		fprintf(stderr, "calloc() failed\n");
		
		freeaddrinfo(res);
		free(net_threads);
		free(nargs);
		return 1;
	}
	
	int i = 0;
	int created_threads = 0;
	for(i = 0; i < thread_num; i++) {
		struct net_args* narg = &nargs[i];
		narg->id = i;
		narg->addrinfo = res;
		nargs->lock = lock;
		
		if(pthread_create(&net_threads[i], NULL, net, narg) != 0) {
			pthread_mutex_lock(&lock);
			fprintf(stderr, "pthread_create() for %d failed: %s\n", i, strerror(errno));
			pthread_mutex_unlock(&lock);
		} else {
			created_threads += 1;
		}
	}
	
	for(i = 0; i < created_threads; i++) {
		pthread_join(net_threads[i], NULL);
	}
	
	freeaddrinfo(res);
	free(net_threads);
	free(nargs);
	pthread_mutex_destroy(&lock);
	
	return 0;
}
