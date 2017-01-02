#include <stdint.h>

struct state {
	uint32_t pc;
	uint32_t reg[32];
	uint32_t* mem;
	uint32_t memsize;
};