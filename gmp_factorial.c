#include <stdio.h>
#include <gmp.h>
#include <string.h>

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("usage: %s <n> to calculate the factorial of n\n", argv[0]);
		return 1;
	}
	
	mpz_t res;
	mpz_init(res);
	
	unsigned long int n = (unsigned long int)atoi(argv[1]);
	mpz_fac_ui(res, n);
	
	char* resstr = mpz_get_str(NULL, 10, res);
	printf("factorial of %lu is %s\n", n, resstr);
	printf("it has %lu digits\n", strlen(resstr));
	
	mpz_clear(res);
	return 0;
}