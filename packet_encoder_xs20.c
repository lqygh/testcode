#include "packet_encoder_xs20.h"

/*
(unsigned 16 bit Ethernet frame length + Ethernet frame + unsigned 32 bit Ethernet frame checksum + signed 16 bit client ID)streamcipher + (192 bit streamcipher nonce)plain
each packet will have 32 byte overhead*/

//naive checksum function to ensure packet is from correctly implemented server/client
static inline uint32_t getchecksum(void* data, size_t datalen) {
	unsigned char* d = data;
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
		
	*state = calloc(sizeof(struct packet_encoder_state), 1);
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
	
	fclose(fp);
	
	return 0;
}

int packet_encoder_deinit(void** state) {
	if(state == NULL || *state == NULL) {
		return -1;
	}
		
	sodium_memzero(*state, sizeof(struct packet_encoder_state));
	free(*state);
	*state = NULL;
	
	randombytes_close();
	
	return 0;
}

int packet_encode(void* eth_frame, uint16_t eth_frame_len, void* dst, size_t dst_len, int16_t id, void* state) {
	if(eth_frame == NULL || dst == NULL || state == NULL) {
		return -1;
	}
	
	if(eth_frame_len < 1 || dst_len < 1) {
		return -1;
	}
	
	if((size_t)eth_frame_len + 32 > dst_len) {
		return -1;
	}
	
	void* key_256bit = ((struct packet_encoder_state*)(state))->key;
	unsigned char nonce_192bit[24] = {0};
	randombytes_buf(nonce_192bit, 24);
	
	uint16_t eth_frame_len_be = htons(eth_frame_len);
	uint32_t frame_checksum_be = htonl(getchecksum(eth_frame, eth_frame_len));
	int16_t id_be = htons(id);
	
	memcpy(dst, &eth_frame_len_be, 2);
	memcpy(dst + 2, eth_frame, eth_frame_len);
	memcpy(dst + 2 + eth_frame_len, &frame_checksum_be, 4);
	memcpy(dst + 2 + eth_frame_len + 4, &id_be, 2);
	memcpy(dst + 2 + eth_frame_len + 4 + 2, nonce_192bit, 24);
	
	int ret = crypto_stream_xor(dst, dst, 2 + eth_frame_len + 4 + 2, nonce_192bit, key_256bit);
	if(ret < 0) {
		sodium_memzero(dst, dst_len);
		return -1;
	}
	
	return eth_frame_len + 32;
}

int packet_decode(void* src, size_t src_len, void* dst_eth_frame, uint16_t* dst_len, int16_t* id, void* state) {
	if(src == NULL || dst_eth_frame == NULL || dst_len == NULL || id == NULL || state == NULL) {
		return -1;
	}
	
	if(src_len < 32 + 1 || *dst_len < 1) {
		return -1;
	}
	
	if(src_len - 32 > *dst_len) {
		return -1;
	}
	
	void* key_256bit = ((struct packet_encoder_state*)(state))->key;
	void* nonce_192bit = src + (src_len - 24);
	
	int ret = crypto_stream_xor(src, src, src_len - 24, nonce_192bit, key_256bit);
	if(ret < 0) {
		return -1;
	}
	
	uint16_t eth_frame_len_be;
	memcpy(&eth_frame_len_be, src, 2);
	uint16_t eth_frame_len = ntohs(eth_frame_len_be);
	if(2 + (size_t)eth_frame_len + 4 + 2 + 24 != src_len) {
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