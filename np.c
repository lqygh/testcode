#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
int main()
{
	void *b = NULL;
	int membyte = 100000000;

	while(1) {
	b = malloc(membyte);
		if(b == NULL) {
			membyte-=50000000;
			break;
		} else {
			free(b);
			membyte+=10000000;
		}
	}
	
	printf("maximum bytes: %d\n", membyte);
	b = malloc(membyte);
	printf("pointer value: %p\n", b);

	int i = 0;
	struct timeb before;
	ftime(&before);
	for(i = 0; i < membyte-1; i++) {
	*((char *)b+i) = (char)12;
	}
	struct timeb after;
	ftime(&after);
	
	double elapsed = (double)after.time + (double)after.millitm/1000.0 - (double)before.time - (double)before.millitm/1000.0;
	printf("%lu %hu %lu %hu\n", after.time, after.millitm, before.time, before.millitm);
	printf("%f seconds elapsed\n", elapsed);

	double speed = (double)membyte/(double)elapsed;
	printf("%f mbytes per second\n", speed/1000000.0);
	sleep(10);
	free(b);
	return 0;
}
