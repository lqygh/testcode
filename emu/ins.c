#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int decode(uint32_t);

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("%s <base> <instruction>\n", argv[0]);
		return 1;
	}
	
	uint32_t instruction = (uint32_t)strtol(argv[2], NULL, atoi(argv[1]));
	printf("instruction: %#x\n", instruction);
	
	decode(instruction);
	
	return 0;
}

int decode(uint32_t ins) {
	uint8_t opcode = (ins & 0b11111100000000000000000000000000) >> 26;
	printf("opcode: %u\n", opcode);
	
	switch(opcode) {
		case 0b0:
			printf("add\n");
			break;
		
		case 0b1000:
			printf("addi\n");
			break;
			
		case 0b1001:
			printf("addiu\n");
			break;
			
		case 0b100:
			printf("beq\n");
			break;
			
		case 0b1:
			printf("bgez\n");
			break;
			
		case 0b111:
			printf("bgtz\n");
			break;
			
		default:
			printf("unimplemented\n");
			break;
	}
	
	return 0;
}