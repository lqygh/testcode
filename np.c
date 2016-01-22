#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/timeb.h>
int main(int argc)
{
	void* b = NULL;
	size_t membyte;
	printf("size_t size is %d\n", sizeof(membyte));
	if(sizeof(membyte) < 8) {
		membyte = 4000000000;
	} else {
		membyte = 100000000000;
	}
	
	while(1) {
	b = malloc(membyte);
		if(b == NULL) {
			membyte -= 10000000;
		} else {
			printf("maximum bytes: %zu\n", membyte);
			break;
		}
	}
	
	membyte = membyte*0.9;
	printf("using %zu bytes of memory\n", membyte);
	printf("pointer value: %p\n", b);

	struct timeb before, after;
	printf("writing %zu bytes to memory\n", membyte);
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
