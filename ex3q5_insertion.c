#include <stdio.h>

void printArray(int* a, size_t size) {
	int i;
	for(i = 0; i < size; i++) {
		printf("%d ", a[i]);
	}
	printf("\n");
}

int main() {
	int a[6] = {6, 4, 8, 5, 2, 7};
	
	int i, j, t;
	for(i = 1; i != 6; i++) {
		j = i;
		t = a[j];
		while(j > 0 && t < a[j-1]) {a[j] = a[j-1]; j = j-1;};
		a[j] = t;
		
		printf("i: %d, j: %d, t: %d\n", i, j, t);
		printf("array: ");
		printArray(a, 6);
		printf("\n");
	}
	
	return 0;
}