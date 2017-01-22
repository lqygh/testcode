#include "tiny-AES128-C/aes.h"
#include <stdio.h>

int main() {
	uint8_t key[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	uint8_t iv[16] = {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
	uint8_t plain_text[128];
	uint8_t i;
	for(i = 0; i < 128; i++) {
		plain_text[i] = i;
	}
	
	printf("plain_text:\n");
	for(i = 0; i < 128; i++) {
		printf("%u ", plain_text[i]);
	}
	printf("\n\n");
	
	uint8_t cipher_text[128] = {0};
	AES128_CBC_encrypt_buffer(cipher_text, plain_text, 128, key, iv);
	printf("cipher_text:\n");
	for(i = 0; i < 128; i++) {
		printf("%u ", cipher_text[i]);
	}
	printf("\n\n");
	
	uint8_t decrypted_text[128] = {0};
	AES128_CBC_decrypt_buffer(decrypted_text, cipher_text, 128, key, iv);
	printf("decrypted_text:\n");
	for(i = 0; i < 128; i++) {
		printf("%u ", decrypted_text[i]);
	}
	printf("\n\n");
	
	return 0;
}