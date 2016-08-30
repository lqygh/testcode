/*
 * twothreads.c
 *
 *  Created on: 19 Jan 2016
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct args {
	int index;
	int *counter;
	pthread_mutex_t *lock;
};

void *incdec(void *args) {
	printf("thread %d starts\n", ((struct args *)args)->index);

	struct args *targs = args;
	int *counter = targs->counter;

	pthread_mutex_lock(targs->lock);
	for(int i = 0; i < 1000000; i++) {
		(*counter)++;
	}
	pthread_mutex_unlock(targs->lock);

	//printf("thread %d will sleep for some time\n", targs->index);
	//sleep(60);

	pthread_mutex_lock(targs->lock);
	for(int i = 0; i < 1000000; i++) {
		(*counter)--;
	}
	pthread_mutex_unlock(targs->lock);

	printf("thread %d exits\n", targs->index);
	return NULL;
}

int main(int argc, char *argv[]) {
	int number = 200;
	int counter = 0;

	if(argc >= 2) {
		int num = (int)strtol(argv[1], NULL, 10);
		number = (num >= 1) ? num : 200;
	}
	
	pthread_t threads[number];
	pthread_mutex_t lock;
	struct args targs[number];

	if(pthread_mutex_init(&lock, NULL) != 0) {
		fprintf(stderr, "failed to initialise mutex");
		return 1;
	}

	for(int i = 0; i < number; i++) {
			targs[i].index = i;
			targs[i].counter = &counter;
			targs[i].lock = &lock;
		}

	printf("creating %d threads...\n", number);
	for(int i = 0; i < number; i++) {
		if(pthread_create(&threads[i], NULL, &incdec, &targs[i]) != 0) {
			fprintf(stderr, "failed to create thread %d\n", i);
		}
	}

	for(int i = 0; i < number; i++) {
		if(pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "failed to join thread %d\n", i);
		} else {
			printf("thread %d joined\n", i);
		}
	}

	pthread_mutex_destroy(&lock);
	printf("actual value of counter is %d\n", counter);
	
	return 0;
}
