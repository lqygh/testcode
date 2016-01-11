#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

int main(int argc, char* argv[])
{
	uLong compsize = 1;
	uLong compbuffsize = compsize;
	uLong uncompsize = 1000;
	uLong cbound = 0;

	char* string = malloc(1000);
	char* compressed = malloc(compsize);
	char* uncompressed = malloc(uncompsize);
	
	int i;
	for(i = 0; i < 10; i++)
	{
		int ret = scanf("%s", string); //get string from stdin
		int len = strlen(string)+1; //get string length + null byte
		
		cbound = compressBound((uLong)len); //get bound of compressed size
		printf("compressBound() returns %lu\n", cbound);
		
		if(cbound > compbuffsize) //allocate more memory to compressed if necessary
		{
			compressed = realloc(compressed, (cbound));
			compbuffsize = cbound;
			printf("buffer enlarged to %lu bytes\n", compbuffsize);
		}
		compsize = compbuffsize;
		
		ret = compress2(compressed, &compsize, string, len, Z_BEST_COMPRESSION);
		if(ret == Z_OK)
		{
			printf("compressed from %d bytes to %lu bytes\n", len, compsize);
		} else {
			printf("compress() error\n");
		}
		
		ret = uncompress(uncompressed, &uncompsize, compressed, compsize);
		if(ret == Z_OK)
		{
			printf("uncompressed from %lu bytes to %lu bytes\n", compsize, uncompsize);
			printf("%s\n", uncompressed);
		} else {
			printf("uncompress() error\n");
		}
		
		uncompsize = 1000;
		
		printf("\n\n");
	}
	
	return 0;
}
