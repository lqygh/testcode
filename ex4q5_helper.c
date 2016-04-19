#include <stdio.h>
#include <stdlib.h>

char convert(char letter) {
	if(letter >= 'a' && letter <= 'z') {
		return letter-97;
	} else if(letter >= 'A' && letter <= 'Z') {
		return letter-65;
	} else {
		return -1;
	}
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "usage: %s <modulo number> <word>\n", argv[0]);
		return 1;
	}
	
	int mod = atoi(argv[1]);
	if(mod == 0) {
		fprintf(stderr, "modulo cannot be %d\n", mod);
		return 1;
	}
	
	int sum = 0;
	
	int i = 0;
	do {
		char tmp = convert(argv[2][i++]);
		if(tmp == -1) {
			fprintf(stderr, "illegal character %c\n", argv[2][i-1]);
			return 1;
		}
		sum += tmp;
	} while(argv[2][i] != '\0');
	
	int res = sum % mod;
	
	printf("sum of %s is %d, %d modulo %d is %d\n", argv[2], sum, sum, mod, res);
	
	return 0;
}