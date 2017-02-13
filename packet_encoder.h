#ifndef PACKET_ENCODER_DEFINED
#define PACKET_ENCODER_DEFINED

#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sodium.h>
#include "tiny-AES128-C/aes.h"

int packet_encoder_init(char* arg, void** state);

int packet_encoder_deinit(void** state);

int packet_encode(void* eth_frame, uint16_t eth_frame_len, void* dst, size_t dst_len, int16_t id, void* state);

int packet_decode(void* src, size_t src_len, void* dst_eth_frame, uint16_t* dst_len, int16_t* id, void* state);

#endif
