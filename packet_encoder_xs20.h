#ifndef PACKET_ENCODER_XS20_DEFINED
#define PACKET_ENCODER_XS20_DEFINED

#include <string.h>
#include <arpa/inet.h>
#include <sodium.h>

struct packet_encoder_state {
	unsigned char key[32];
};

int packet_encoder_init(char* arg, void** state);

int packet_encoder_deinit(void** state);

int packet_encode(void* eth_frame, uint16_t eth_frame_len, void* dst, size_t dst_len, int16_t id, void* state);

int packet_decode(void* src, size_t src_len, void* dst_eth_frame, uint16_t* dst_len, int16_t* id, void* state);

#endif
