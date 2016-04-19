#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("usage: %s <m> <n> to calculate the percentage probability of collisions when inserting n items into a table of size m\n", argv[0]);
		return 1;
	}
	
	unsigned long int m = atoi(argv[1]);
	unsigned long int n = atoi(argv[2]);
	
	mpz_t a, b, c, d;
	mpz_init(a);
	mpz_init(b);
	mpz_init(c);
	mpz_init(d);
	
	mpf_t x, y, z, z2;
	mpf_init(x);
	mpf_init(y);
	mpf_init(z);
	mpf_init(z2);
	
	mpz_fac_ui(a, m-1);
	mpz_ui_pow_ui(b, m, n-1);
	mpz_fac_ui(c, m-n);
	
	mpz_mul(d, b, c);
	
	mpf_set_z(x, a);
	mpf_set_z(y, d);
	
	mpf_div(z, x, y);
	
	mp_exp_t exp = 0;
	printf("probability of no collisions is %s\n", mpf_get_str(NULL, &exp, 10, 30, z));
	printf("exp %ld\n", exp);
	
	putchar('\n');
	
	mpf_ui_sub(z2, 1, z);
	printf("probability of collisions is %s\n", mpf_get_str(NULL, &exp, 10, 30, z2));
	printf("exp %ld\n", exp);
	
	mpz_clear(a);
	mpz_clear(b);
	mpz_clear(c);
	mpz_clear(d);
	
	mpf_clear(x);
	mpf_clear(y);
	mpf_clear(z);
	mpf_clear(z2);
	
	return 0;
}