#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int loopnum = 1000000;
	int i = 0;
	char coin;
	int win[3] = {0};
	
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
		//player 1
		if(fread(&coin, 1, 1, randsrc) <= 0) {
			perror("fread()");
			fclose(randsrc);
			return 1;
		}
		if(coin >= 0) {
			win[0] += 1;
			continue;
		}
		
		//player 2
		if(fread(&coin, 1, 1, randsrc) <= 0) {
			perror("fread()");
			fclose(randsrc);
			return 1;
		}
		if(coin >= 0) {
			win[1] += 1;
			continue;
		}
		
		//player 3
		if(fread(&coin, 1, 1, randsrc) <= 0) {
			perror("fread()");
			fclose(randsrc);
			return 1;
		}
		if(coin >= 0) {
			win[2] += 1;
		} else {
			//repeat
			i -= 1;
		}
		
	}
	
	fclose(randsrc);
	
	printf("number of simulations: %d\n", loopnum);
	printf("player 1 wins: %d, probability: %f\n", win[0], win[0]/(double)loopnum);
	printf("player 2 wins: %d, probability: %f\n", win[1], win[1]/(double)loopnum);
	printf("player 3 wins: %d, probability: %f\n", win[2], win[2]/(double)loopnum);
	
	return 0;
}