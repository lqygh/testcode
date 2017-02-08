#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	char* str = NULL;
	asprintf(&str, "argc is %d, argv is %p\n", argc, argv);
	if(str == NULL) {
		printf("asprintf() failed\n");
		return 1;
	} else {
		printf("%s", str);
	}
	
	free(str);
	return 0;
}