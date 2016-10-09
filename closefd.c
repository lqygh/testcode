#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Usage: %s <fd>\n", argv[0]);
		return 1;
	} else {
		int fd = atoi(argv[1]);
		if(fd < 0) {
			fprintf(stderr, "fd must be >= 0\n");
			return 1;
		} else {
			printf("closing fd %d\n", fd);
			if(close(fd) == 0) {
				printf("fd %d closed\n", fd);
			} else {
				perror("failed to close fd %d");
				return 1;
			}	
		}
	}
	
	return 0;
}