#include "packet_encoder_plain.h"

/*
(unsigned 16 bit Ethernet frame length + Ethernet frame + unsigned 32 bit Ethernet frame checksum + signed 16 bit client ID)plain
each packet will have 8 byte overhead*/

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
	if(state == NULL) {
		return -1;
	}
	
	if(arg == NULL) {
	}
	
	return 0;
}

int packet_encoder_deinit(void** state) {
	if(state == NULL) {
		return -1;
	}
	
	*state = NULL;
		
	return 0;
}

int packet_encode(void* eth_frame, uint16_t eth_frame_len, void* dst, size_t dst_len, int16_t id, void* state) {
	if(eth_frame == NULL || dst == NULL) {
		return -1;
	}
	
	if(state == NULL) {
	}
	
	if(eth_frame_len < 1 || dst_len < 1) {
		return -1;
	}
	
	if((size_t)eth_frame_len + 8 > dst_len) {
		return -1;
	}
	
	uint16_t eth_frame_len_be = htons(eth_frame_len);
	uint32_t frame_checksum_be = htonl(getchecksum(eth_frame, eth_frame_len));
	int16_t id_be = htons(id);
	
	memcpy(dst, &eth_frame_len_be, 2);
	memcpy(dst + 2, eth_frame, eth_frame_len);
	memcpy(dst + 2 + eth_frame_len, &frame_checksum_be, 4);
	memcpy(dst + 2 + eth_frame_len + 4, &id_be, 2);
	
	return eth_frame_len + 8;
}

int packet_decode(void* src, size_t src_len, void* dst_eth_frame, uint16_t* dst_len, int16_t* id, void* state) {
	if(src == NULL || dst_eth_frame == NULL || dst_len == NULL || id == NULL) {
		return -1;
	}
	
	if(state == NULL) {
	}
	
	if(src_len < 8 + 1 || *dst_len < 1) {
		return -1;
	}
	
	if(src_len - 8 > *dst_len) {
		return -1;
	}
	
	uint16_t eth_frame_len_be;
	memcpy(&eth_frame_len_be, src, 2);
	uint16_t eth_frame_len = ntohs(eth_frame_len_be);
	if(2 + (size_t)eth_frame_len + 4 + 2 != src_len) {
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