#include <string.h>
#include "tiny-AES128-C/aes.h"
#include <sodium.h>

void print_decimal(unsigned char* data, size_t length) {
	size_t i;
	for(i = 0; i < length; i++) {
		printf("%u ", data[i]);
	}
	printf("\n\n");
}

int main() {
	if(sodium_init() == -1) {
		fprintf(stderr, "sodium_init() failed\n");
		return 1;
	}

	unsigned char plain_text[] = "Hello SECRET SECRET Byebye";
	printf("plain text size: %lu, content:\n%s\n", sizeof(plain_text), plain_text);
	printf("plain text decimal:\n");
	print_decimal(plain_text, sizeof(plain_text));
	
	//uint32_t random = randombytes_random();

	printf("nonce size: %u, key size: %u\n", crypto_stream_chacha20_NONCEBYTES, crypto_stream_chacha20_KEYBYTES);
	
	unsigned char nonce[crypto_stream_chacha20_NONCEBYTES] = {1, 2, 3, 4, 5, 6, 7, 8};
	unsigned char key[crypto_stream_chacha20_KEYBYTES] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	unsigned char cipher_text[sizeof(plain_text)] = {0};
	
	int ret = crypto_stream_chacha20_xor(cipher_text, plain_text, sizeof(plain_text), nonce, key);
	printf("Encrypting, crypto_stream_chacha20_xor() returns %d\n", ret);
	putchar('\n');
	
	printf("cipher text size: %lu, decimal:\n", sizeof(cipher_text));
	print_decimal(cipher_text, sizeof(cipher_text));
	
	ret = crypto_stream_chacha20_xor(cipher_text, cipher_text, sizeof(plain_text), nonce, key);
	printf("Decrypting, crypto_stream_chacha20_xor() returns %d\n", ret);
	putchar('\n');
	
	printf("decrypted cipher text size: %lu, decimal:\n", sizeof(cipher_text));
	print_decimal(cipher_text, sizeof(cipher_text));
	
	unsigned char nonce_encrypted[16];
	memcpy(nonce_encrypted, nonce, 8);
	memcpy(nonce_encrypted+8, nonce, 8);
	printf("16 byte nonce decimal before AES encryption:\n");
	print_decimal(nonce_encrypted, sizeof(nonce_encrypted));
	putchar('\n');

	AES128_ECB_encrypt(nonce_encrypted, key, nonce_encrypted);
	printf("16 byte nonce decimal after AES encryption:\n");
	print_decimal(nonce_encrypted, sizeof(nonce_encrypted));
	putchar('\n');
	
	AES128_ECB_decrypt(nonce_encrypted, key, nonce_encrypted);
	printf("16 byte nonce decimal after AES decryption:\n");
	print_decimal(nonce_encrypted, sizeof(nonce_encrypted));
	putchar('\n');
	
	return 0;
}
