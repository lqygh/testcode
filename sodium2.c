#include <dlfcn.h>
#include "packet_encoder.h"

#define BUFFERSIZE 500

void print_decimal(unsigned char* data, size_t length) {
	size_t i;
	for(i = 0; i < length; i++) {
		printf("%u ", data[i]);
	}
	printf("\n\n");
}

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("Usage: %s <256-bit key file> <library>\n", argv[0]);
		printf("       %s <256-bit key file> <library> <message>\n", argv[0]);
		return 1;
	}
	
	void* dl_handle = dlopen(argv[2], RTLD_LAZY);
	if(dl_handle == NULL) {
		fprintf(stderr, "dlopen() failed: %s\n", dlerror());
		return 1;
	}
	
	int (*packet_encoder_init)(char* arg, void** state);
	int (*packet_encoder_deinit)(void** state);
	int (*packet_encode)(void* eth_frame, uint16_t eth_frame_len, void* dst, size_t dst_len, int16_t id, void* state);
	int (*packet_decode)(void* src, size_t src_len, void* dst_eth_frame, uint16_t* dst_len, int16_t* id, void* state);
	
	char* dl_err;
	dlerror();
	packet_encoder_init = dlsym(dl_handle, "packet_encoder_init");
	if((dl_err = dlerror()) != NULL) {
		fprintf(stderr, "dlsym() failed: %s\n", dl_err);
		return 1;
	}
	dlerror();
	packet_encoder_deinit = dlsym(dl_handle, "packet_encoder_deinit");
	if((dl_err = dlerror()) != NULL) {
		fprintf(stderr, "dlsym() failed: %s\n", dl_err);
		return 1;
	}
	dlerror();
	packet_encode = dlsym(dl_handle, "packet_encode");
	if((dl_err = dlerror()) != NULL) {
		fprintf(stderr, "dlsym() failed: %s\n", dl_err);
		return 1;
	}
	dlerror();
	packet_decode = dlsym(dl_handle, "packet_decode");
	if((dl_err = dlerror()) != NULL) {
		fprintf(stderr, "dlsym() failed: %s\n", dl_err);
		return 1;
	}
		
	void* state = NULL;
	if(packet_encoder_init(argv[1], &state) < 0) {
		fprintf(stderr, "packet_encoder_init() failed\n");
		dlclose(dl_handle);
		return 1;
	}

	unsigned char plain[] = "Hello SECRET Message SECRET Byebye";
	size_t plain_text_len = sizeof(plain);
	unsigned char* plain_text = plain;
	
	if(argc > 3) {
		plain_text = (unsigned char*)argv[3];
		plain_text_len = strlen((char*)plain_text) + 1;
	}
	
	printf("plain text size: %lu, content:\n%s\n", plain_text_len, plain_text);
	putchar('\n');
	
	printf("plain text decimal:\n");
	print_decimal(plain_text, plain_text_len);
		
	unsigned char packet_data[BUFFERSIZE] = {0};
	unsigned char packet_data2[BUFFERSIZE] = {0};
	int16_t id = 33;
	
	//encryption 1
	int ret = packet_encode(plain_text, plain_text_len, packet_data, BUFFERSIZE, id, state);
	printf("Encrypting, packet_encode() returns %d\n", ret);
	putchar('\n');
	
	if(ret < 0) {
		fprintf(stderr, "packet_encode() failed\n");
		packet_encoder_deinit(&state);
		dlclose(dl_handle);
		return 1;
	}
	int siz = ret;
	
	printf("packet data decimal:\n");
	print_decimal(packet_data, ret);
	
	//encryption 2
	ret = packet_encode(plain_text, plain_text_len, packet_data2, BUFFERSIZE, id, state);
	printf("Encrypting, packet_encode() returns %d\n", ret);
	putchar('\n');
	
	if(ret < 0) {
		fprintf(stderr, "packet_encode() failed\n");
		packet_encoder_deinit(&state);
		dlclose(dl_handle);
		return 1;
	}
	int siz2 = ret;
	
	printf("packet data decimal:\n");
	print_decimal(packet_data2, ret);
	
	//decryption
	unsigned char decrypted[BUFFERSIZE] = {0};
	int16_t id_decrypted = 0;
	uint16_t decrypted_len = BUFFERSIZE;
	
	ret = packet_decode(packet_data, siz, decrypted, &decrypted_len, &id_decrypted, state);
	printf("Decrypting, packet_decode() returns %d\n", ret);
	if(ret < 0) {
		fprintf(stderr, "packet_decode() failed\n");
		packet_encoder_deinit(&state);
		dlclose(dl_handle);
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
		packet_encoder_deinit(&state);
		dlclose(dl_handle);
		return 1;
	} else {
		printf("decrypted id: %d, length: %u\n", id_decrypted, decrypted_len);
	}
	
	printf("decrypted data decimal:\n");
	print_decimal(decrypted, decrypted_len);
	
	printf("decrypted content:\n%s\n", decrypted);
	putchar('\n');
	
	packet_encoder_deinit(&state);
	dlclose(dl_handle);
	
	return 0;
}
