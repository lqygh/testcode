#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int loopnum = 1000000;
	int i = 0, j = 0, sum = 0, div6 = 0;
	int diceroll = 0;
	int dicevalue[6] = {0};
	int dice = 0;
	unsigned int randnum = 0;
	
	FILE* randsrc;
	if(argc <= 1) {
		randsrc = fopen("/dev/urandom", "r");
	} else {
		randsrc = fopen(argv[1], "r");
	}
	
	if(argc >= 3) {
		int numloop = atoi(argv[2]);
		if(numloop > 0) {
			loopnum = numloop;
		}
	}
	
	if(randsrc == NULL) {
		perror("fopen()");
		return 1;
	}
	
	for(i = 0; i < loopnum; i++) {
		sum = 0;
		for(j = 0; j < 13; j++) {
			if(fread(&randnum, sizeof(randnum), 1, randsrc) <= 0) {
			perror("fread()");
			fclose(randsrc);
			printf("i: %d, j: %d\n", i, j);
			return 1;
			}
			
			dice = (randnum % 6) + 1;
			
			diceroll += 1;
			dicevalue[dice-1] += 1;
			sum += dice;
		}
		
		if(sum % 6 == 0) {
			div6 += 1;
		}
	}
	
	fclose(randsrc);
	
	printf("number of simulations: %d\n", loopnum);
	printf("sum divisible by 6: %d, probability: %f\n", div6, div6/(double)loopnum);
	
	putchar('\n');
	printf("number of dice rolls: %d\n", diceroll);
	for(i = 0; i < 6; i++) {
		printf("number of %d: %d\n", i + 1, dicevalue[i]);
	}
	
	return 0;
}