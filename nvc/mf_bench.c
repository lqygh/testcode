#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <string.h>
#include "mf.h"

#define NUM_LOOPS 1
#define BUFFER_SIZE 1000000000

int main(int argc, char* argv[]) {
	int m_loops = 1;
	if(argc > 1) {
		int n = atoi(argv[1]);
		if(n > 0) {
			m_loops = n;
		}
	}
	
	unsigned char* k = malloc(2 * BUFFER_SIZE);
	unsigned long int i = 0;
	for(i = 0; i < 2 * BUFFER_SIZE; i++) {
		k[i] = i % 256;
	}
	
	//unsigned char* src = malloc(BUFFER_SIZE);
	unsigned char* tmp = malloc(BUFFER_SIZE);
	//unsigned char* dst = malloc(BUFFER_SIZE);
	
	struct timeb t_begin = {0}, t_end = {0};
	printf("bench starts\n");
	ftime(&t_begin);
	for(i = 0; i < (unsigned long int)NUM_LOOPS * m_loops; i++) {
		//memcpy(tmp, src, BUFFER_SIZE);
		memfrob2(tmp, k, BUFFER_SIZE, 0);
		//memcpy(dst, tmp, BUFFER_SIZE);
	}
	ftime(&t_end);
	printf("bench ends\n");
	
	printf("before: %ld s + %u ms\n", t_begin.time, t_begin.millitm);
	printf("after: %ld s + %u ms\n", t_end.time, t_end.millitm);
	unsigned long int diff = (t_end.time * 1000 + t_end.millitm) - (t_begin.time * 1000 + t_begin.millitm);
	printf("bytes processed: %lu\n", (unsigned long int)NUM_LOOPS * m_loops * BUFFER_SIZE);
	printf("time elapsed: %lu ms\n", diff);
	printf("speed: %f MB/s\n", ((unsigned long int)NUM_LOOPS * m_loops * BUFFER_SIZE) / (1000.0 * diff));
	
	return 0;
}