#include "packet_encoder.h"

static uint32_t getchecksum(void* data, size_t datalen) {
	unsigned char* d = (unsigned char*)data;
	uint32_t sum = 0;
	size_t i = 0;
	for(i = 0; i < datalen; i++) {
		sum += d[i];
	}
	return sum;
}

int packet_encoder_init(char* arg, void** state) {
	if(arg == NULL || state == NULL) {
		return -1;
	}
	
	if(sodium_init() < 0) {
		return -1;
	}
		
	*state = calloc(32 + 8, 1);
	if(*state == NULL) {
		return -1;
	}
	
	FILE* fp = fopen(arg, "r");
	if(fp == NULL) {
		free(*state);
		return -1;
	}
	
	if(fread(*state, 32, 1, fp) != 1) {
		free(*state);
		fclose(fp);
		return -1;
	}
	randombytes_buf((*state) + 32, 8);
	
	fclose(fp);
	randombytes_close();
	
	return 0;
}

int packet_encoder_deinit(void** state) {
	if(state == NULL || *state == NULL) {
		return -1;
	}
	
	sodium_memzero(*state, 32 + 8);
	free(*state);
	*state = NULL;
	
	return 0;
}

int packet_encode(void* eth_frame, uint16_t eth_frame_len, void* dst, size_t dst_len, int16_t id, void* state) {
	if(eth_frame == NULL || dst == NULL || state == NULL) {
		return -1;
	}
	
	if(eth_frame_len < 1 || dst_len < 1) {
		return -1;
	}
	
	if((size_t)eth_frame_len + 24 > dst_len) {
		return -1;
	}
	
	void* key_256bit = state;
	void* nonce_64bit = state + 32;
	
	uint16_t eth_frame_len_be = htons(eth_frame_len);
	uint32_t frame_checksum_be = htonl(getchecksum(eth_frame, eth_frame_len));
	int16_t id_be = htons(id);
	
	uint8_t nonce[16] = {0};
	memcpy(nonce, nonce_64bit, 8);
	memcpy(nonce + 8, nonce_64bit, 8);
	AES128_ECB_encrypt(nonce, key_256bit, nonce);
	
	memcpy(dst, &eth_frame_len_be, 2);
	memcpy(dst + 2, eth_frame, eth_frame_len);
	memcpy(dst + 2 + eth_frame_len, &frame_checksum_be, 4);
	memcpy(dst + 2 + eth_frame_len + 4, &id_be, 2);
	memcpy(dst + 2 + eth_frame_len + 4 + 2, nonce, 16);
	
	int ret = crypto_stream_chacha20_xor(dst, dst, 2 + eth_frame_len + 4 + 2, nonce_64bit, key_256bit);
	if(ret < 0) {
		return -1;
	}
	
	*((uint64_t*)nonce_64bit) += 1;
	
	return eth_frame_len + 24;
}

int packet_decode(void* src, size_t src_len, void* dst_eth_frame, uint16_t* dst_len, int16_t* id, void* state) {
	if(src == NULL || dst_eth_frame == NULL || dst_len == NULL || id == NULL || state == NULL) {
		return -1;
	}
	
	if(src_len < 24 + 1 || *dst_len < 1) {
		return -1;
	}
	
	if(src_len - 24 > *dst_len) {
		return -1;
	}
	
	void* key_256bit = state;
	void* nonce_64bit = src + (src_len - 16);
	AES128_ECB_decrypt(nonce_64bit, key_256bit, nonce_64bit);
	
	int ret = crypto_stream_chacha20_xor(src, src, src_len - 16, nonce_64bit, key_256bit);
	if(ret < 0) {
		return -1;
	}
	
	uint16_t eth_frame_len_be;
	memcpy(&eth_frame_len_be, src, 2);
	uint16_t eth_frame_len = ntohs(eth_frame_len_be);
	if(2 + (size_t)eth_frame_len + 4 + 2 + 16 != src_len) {
		return -1;
	}
	
	uint32_t frame_checksum_be;
	memcpy(&frame_checksum_be, src + 2 + eth_frame_len, 4);
	uint32_t frame_checksum = ntohl(frame_checksum_be);
	if(frame_checksum != getchecksum(src + 2, eth_frame_len)) {
		return -1;
	}
	
	int16_t id_be;
	memcpy(&id_be, src + 2 + eth_frame_len + 4, 2);
	*id = ntohs(id_be);

	memcpy(dst_eth_frame, src + 2, eth_frame_len);
	*dst_len = eth_frame_len;
	
	return eth_frame_len;
}