#include "mf.h"

void memfrob2(void* data, void* key, int datalen, uint16_t offset) {
	char* d = (char*)data;
	char* k = (char*)key;
	int i;
	for(i = 0; i < datalen; i++) {
		d[i] ^= k[i+offset];
	}
}
