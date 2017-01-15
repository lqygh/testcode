#include <sodium.h>

void print_usage(char* argv[]) {
	printf("Usage:\n");
	printf("%s to generate a random number between 0 and 0xffffffff(included)\n", argv[0]);
	printf("%s <number> to generate a random number between 0 and number(excluded)\n", argv[0]);
}

int main(int argc, char* argv[]) {
        if(sodium_init() == -1) {
                fprintf(stderr, "sodium_init() failed\n");
				return 1;
        }

		uint32_t random;
		
		if(argc <= 1) {
			random = randombytes_random();
		} else if(argc == 2) {
			int u = atoi(argv[1]);
			if(u < 0) {
				fprintf(stderr, "upper bound must be >= 0\n");
				return 1;
			}
			random = randombytes_uniform(u);
		} else {
			print_usage(argv);
			return 1;
		}
		
        printf("%u\n%#010x\n", random, random);

        return 0;
}
