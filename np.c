#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/timeb.h>
int main(int argc)
{
	void* b = NULL;
	long long unsigned int membyte = 100000000000;

	while(1) {
	b = malloc(membyte);
		if(b == NULL) {
			membyte-=10000000;
		} else {
			printf("maximum bytes: %llu\n", membyte);
			break;
		}
	}
	
	membyte = membyte/2;
	printf("using %llu bytes of memory\n", membyte);
	printf("pointer value: %p\n", b);

	struct timeb before, after;
	printf("writing %llu bytes to memory\n", membyte);
	int i = 0;
	ftime(&before);
	for(i = 0; i < membyte-1; i++) {
	*((char *)b+i) = (char)12;
	}
	ftime(&after);
	double elapsed = (double)after.time + (double)after.millitm/1000.0 - (double)before.time - (double)before.millitm/1000.0;
	//printf("%lu %hu %lu %hu\n", after.time, after.millitm, before.time, before.millitm);
	printf("%f seconds elapsed during writing\n", elapsed);
	double speed = (double)membyte/(double)elapsed;
	printf("\n%f mbytes per second\n", speed/1000000.0);
	
	if(argc > 1) {
		sleep(20);
	}
	
	printf("\nfreeing memory\n");
	free(b);
	return 0;
}
