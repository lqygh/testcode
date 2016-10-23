#include <cuda_runtime.h>

extern "C" {
#include "mf.h"
}

__global__ void frob(char* data, char* key, int datalen, uint16_t offset) {
	int i = threadIdx.x + blockIdx.x * blockDim.x;
	if(i < datalen) {
		data[i] ^= key[i+offset];
	}
}

extern "C"
void memfrob2(void* data, void* key, int datalen, uint16_t offset) {
	char* gpu_d;
	char* gpu_k;
	cudaMalloc((void**)&gpu_d, datalen);
	cudaMalloc((void**)&gpu_k, datalen);
	
	cudaMemcpy(gpu_d, data, datalen, cudaMemcpyHostToDevice);
	cudaMemcpy(gpu_k, key, datalen, cudaMemcpyHostToDevice);
	
	frob<<<(datalen + 255) / 256, 256>>>(gpu_d, gpu_k, datalen, offset);
	
	cudaMemcpy(data, gpu_d, datalen, cudaMemcpyDeviceToHost);
	
	cudaFree(gpu_d);
	cudaFree(gpu_k);
}
