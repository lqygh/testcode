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
	pthread_mutex_t* lock;
	pthread_cond_t* cond;
	pthread_barrier_t* barrier;
	int* len;
	char* data;
	int* ready;
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
	pthread_mutex_t* lock = args->lock;
	pthread_cond_t* cond = args->cond;
	pthread_barrier_t* barrier = args->barrier;
	int* len = args->len;
	char* data = args->data;
	int* ready = args->ready;
	
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		pthread_mutex_lock(lock);
		fprintf(stderr, "%d: socket() failed: %s\n", id, strerror(errno));
		pthread_mutex_unlock(lock);
		
		return NULL;
	}
	
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		pthread_mutex_lock(lock);
		fprintf(stderr, "%d: connect() failed: %s\n", id, strerror(errno));
		pthread_mutex_unlock(lock);
		
		close(sockfd);
		return NULL;
	}
	
	while(1) {
		//lock, wait, unlock
		pthread_mutex_lock(lock);
		while(!(*ready)){
			pthread_cond_wait(cond, lock);
		}
		pthread_mutex_unlock(lock);
		
		//write to socket from buffer
		send(sockfd, data, *len, 0);
		
		//barrier
		pthread_barrier_wait(barrier);
	}
	
	close(sockfd);
	
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
	
	pthread_mutex_t lock;
	if(pthread_mutex_init(&lock, NULL) != 0) {
		fprintf(stderr, "pthread_mutex_init() failed\n");
		
		freeaddrinfo(res);
		return 1;
	}
	
	pthread_cond_t cond;
	if(pthread_cond_init(&cond, NULL) != 0) {
		fprintf(stderr, "pthread_cond_init() failed\n");
		
		freeaddrinfo(res);
		pthread_mutex_destroy(&lock);
		return 1;
	}
	
	pthread_barrier_t barrier;
	
	pthread_t* net_threads = calloc(sizeof(pthread_t), thread_num);
	struct net_args* nargs = calloc(sizeof(struct net_args), thread_num);
	if(net_threads == NULL || nargs == NULL) {
		fprintf(stderr, "calloc() failed\n");
		
		freeaddrinfo(res);
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(&cond);
		free(net_threads);
		free(nargs);
		return 1;
	}
	
	int len = 0, buffsize = 1000;
	char* buffer = calloc(buffsize, 1);
	if(buffer == NULL) {
		fprintf(stderr, "calloc() failed\n");
		
		freeaddrinfo(res);
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(&cond);
		free(net_threads);
		free(nargs);
		return 1;
	}
	
	int i = 0, ready = 0;
	int created_threads = 0;
	for(i = 0; i < thread_num; i++) {
		struct net_args* narg = &nargs[i];
		narg->id = i;
		narg->addrinfo = res;
		narg->lock = &lock;
		narg->cond = &cond;
		narg->barrier = &barrier;
		narg->len = &len;
		narg->data = buffer;
		narg->ready = &ready;

		if(pthread_create(&net_threads[i], NULL, net, &(nargs[i])) != 0) {
			pthread_mutex_lock(&lock);
			fprintf(stderr, "pthread_create() for %d failed: %s\n", i, strerror(errno));
			pthread_mutex_unlock(&lock);
		} else {
			created_threads += 1;
		}
	}
	
	if(pthread_barrier_init(&barrier, NULL, created_threads+1) != 0) {
		fprintf(stderr, "pthread_barrier_init() failed\n");
		
		freeaddrinfo(res);
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(&cond);
		free(net_threads);
		free(nargs);
		free(buffer);
		return 1;
	}
	
	while(1) {
		//read from stdin to buffer
		len = read(0, buffer, buffsize);
		if(len == 0) {
			fprintf(stderr, "read() reaches end of file\n");
			return 0;
		} else if(len < 0) {
			perror("read()");
			return 1;
		}
		
		//lock, broadcast, unlock
		pthread_mutex_lock(&lock);
		ready = 1;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&lock);
		
		//barrier
		pthread_barrier_wait(&barrier);
		
		ready = 0;
	}
	
	freeaddrinfo(res);
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
	free(net_threads);
	free(nargs);
	free(buffer);
	pthread_barrier_destroy(&barrier);
	
	return 0;
}
