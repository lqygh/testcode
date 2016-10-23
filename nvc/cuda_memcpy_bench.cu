#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <cuda_runtime.h>

#define N 1500000000

int cudaCheck(cudaError_t code) {
	if(code == cudaSuccess) {
		//printf("cudaSuccess\n");
		return 0;
	} else {
		printf("cudaCheck(): %s\n", cudaGetErrorString(cudaGetLastError()));
		return -1;
	}
}

int main() {
	void* host = malloc(N);
	if(host == NULL) {
		perror("malloc()");
		return 1;
	}
	
	void* device = NULL;
	cudaError_t ret = cudaMalloc(&device, N);
	if(cudaCheck(ret) < 0) {
		return 1;
	}
	
	unsigned long int diff = 0;
	struct timeb t_begin = {0}, t_end = {0};
	
	ftime(&t_begin);
	cudaMemcpy(host, device, N, cudaMemcpyDeviceToHost);
	ftime(&t_end);
	diff = (t_end.time * 1000 + t_end.millitm) - (t_begin.time * 1000 + t_begin.millitm);
	printf("bytes transferred from device to host: %lu\n", (unsigned long int)N);
	printf("time elapsed: %lu ms\n", diff);
	printf("speed: %f MB/s\n", ((unsigned long int)N) / (1000.0 * diff));
	
	putchar('\n');
	
	ftime(&t_begin);
	cudaMemcpy(device, host, N, cudaMemcpyHostToDevice);
	ftime(&t_end);
	diff = (t_end.time * 1000 + t_end.millitm) - (t_begin.time * 1000 + t_begin.millitm);
	printf("bytes transferred from host to device: %lu\n", (unsigned long int)N);
	printf("time elapsed: %lu ms\n", diff);
	printf("speed: %f MB/s\n", ((unsigned long int)N) / (1000.0 * diff));
	
	free(host);
	cudaFree(device);
	return 0;
}