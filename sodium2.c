#include <string.h>
#include <arpa/inet.h>
#include <sodium.h>
#include "tiny-AES128-C/aes.h"

#define BUFFERSIZE 500

void print_decimal(unsigned char* data, size_t length) {
	size_t i;
	for(i = 0; i < length; i++) {
		printf("%u ", data[i]);
	}
	printf("\n\n");
}

uint32_t getchecksum(void* data, size_t datalen) {
	unsigned char* d = (unsigned char*)data;
	uint32_t sum = 0;
	size_t i = 0;
	for(i = 0; i < datalen; i++) {
		sum += d[i];
	}
	return sum;
}

int init(char* arg, void** state) {
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

int deinit(void** state) {
	if(state == NULL || *state == NULL) {
		return -1;
	}
	
	sodium_memzero(*state, 32 + 8);
	free(*state);
	
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

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Usage: %s <256-bit key file>\n", argv[0]);
		printf("       %s <256-bit key file> <message>\n", argv[0]);
		return 1;
	}
	
	void* state = NULL;
	if(init(argv[1], &state) < 0) {
		fprintf(stderr, "init() failed\n");
		return 1;
	}

	unsigned char plain[] = "Hello SECRET Message SECRET Byebye";
	size_t plain_text_len = sizeof(plain);
	unsigned char* plain_text = plain;
	
	if(argc > 2) {
		plain_text = (unsigned char*)argv[2];
		plain_text_len = strlen((char*)plain_text) + 1;
	}
	
	printf("plain text size: %lu, content:\n%s\n", plain_text_len, plain_text);
	putchar('\n');
	
	printf("plain text decimal:\n");
	print_decimal(plain_text, plain_text_len);
		
	unsigned char packet_data[BUFFERSIZE] = {0};
	unsigned char packet_data2[BUFFERSIZE] = {0};
	int16_t id = 33;
	
	//1
	int ret = packet_encode(plain_text, plain_text_len, packet_data, BUFFERSIZE, id, state);
	printf("Encrypting, packet_encode() returns %d\n", ret);
	putchar('\n');
	
	if(ret < 0) {
		fprintf(stderr, "packet_encode() failed\n");
		deinit(&state);
		return 1;
	}
	int siz = ret;
	
	printf("packet data decimal:\n");
	print_decimal(packet_data, ret);
	
	//2
	ret = packet_encode(plain_text, plain_text_len, packet_data2, BUFFERSIZE, id, state);
	printf("Encrypting, packet_encode() returns %d\n", ret);
	putchar('\n');
	
	if(ret < 0) {
		fprintf(stderr, "packet_encode() failed\n");
		deinit(&state);
		return 1;
	}
	int siz2 = ret;
	
	printf("packet data decimal:\n");
	print_decimal(packet_data2, ret);
	
	
	
	unsigned char decrypted[BUFFERSIZE] = {0};
	int16_t id_decrypted = 0;
	uint16_t decrypted_len = BUFFERSIZE;
	
	ret = packet_decode(packet_data, siz, decrypted, &decrypted_len, &id_decrypted, state);
	printf("Decrypting, packet_decode() returns %d\n", ret);
	if(ret < 0) {
		fprintf(stderr, "packet_decode() failed\n");
		deinit(&state);
		return 1;
	} else {
		printf("decrypted id: %d, length: %u\n", id_decrypted, decrypted_len);
	}
	
	printf("decrypted data decimal:\n");
	print_decimal(decrypted, decrypted_len);
	
	printf("decrypted content:\n%s\n", decrypted);
	putchar('\n');
	
	ret = packet_decode(packet_data2, siz2, decrypted, &decrypted_len, &id_decrypted, state);
	printf("Decrypting, packet_decode() returns %d\n", ret);
	if(ret < 0) {
		fprintf(stderr, "packet_decode() failed\n");
		deinit(&state);
		return 1;
	} else {
		printf("decrypted id: %d, length: %u\n", id_decrypted, decrypted_len);
	}
	
	printf("decrypted data decimal:\n");
	print_decimal(decrypted, decrypted_len);
	
	printf("decrypted content:\n%s\n", decrypted);
	putchar('\n');
	
	deinit(&state);
	
	return 0;
}
