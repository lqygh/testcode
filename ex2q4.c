#include <stdio.h>
#include <stdlib.h>

int sortcounter = 0;

void printArray(int* a, size_t size) {
	int i;
	for(i = 0; i < size; i++) {
		printf("%d ", a[i]);
	}
	printf("\n");
}

// Integer comparison function
int compare(const void* a, const void* b) {
	sortcounter += 1;
	
	int x = *(const int*)a;
	int y = *(const int*)b;
	
	if(x < y) {
		return -1;
	} else if(x > y) {
		return 1;
	} else {
		return 0;
	}
}

int* Xsort(int* a, size_t n) {
	int* b = malloc(n*sizeof(int));
	int i;
	for(i = 0; i < n; i++) {
		b[i] = a[i];
	}
	qsort(b, n, sizeof(int), compare);
	return b;
}

/*int duplicates0(int* a, size_t n) {
	printArray(a, n);
	
	int counter = 0;
	int i,j;
	for(i = 0; i < n; i++) {
		for(j = i+1; j < n; j++) {
			counter += 1;
			if(a[j] == a[i]) {
				printf("duplicates0(): %d comparisons, found duplicate\n", counter);
				return 1;
			}
		}
	}
	printf("duplicates0(): %d comparisons, not found duplicate\n", counter);
	return 0;
}*/

int duplicates1(int* a, size_t n) {
	printArray(a, n);
	
	int counter = 0;
	int i, max, min;
	max = a[0];
	for(i = 1; i < n; i++) {
		if(a[i] > max) {
			max = a[i];
		}
	}
	
	min = a[0];
	for(i = 1; i < n; i++) {
		if(a[i] < min) {
			min = a[i];
		}
	}
	
	printf("duplicates1(): max value is %d, min value is %d\n", max, min);
	
	int* countarr = (int*)calloc(sizeof(int), max-min+1);
	
	for(i = 0; i < n; i++) {
		counter += 1;
		if(countarr[a[i]-min] == 1) {
			printf("duplicates1(): %d comparisons, found duplicate %d\n", counter, a[i]);
			return 1;
		} else {
			countarr[a[i]-min] += 1;
		}
	}
	printf("duplicates1(): %d comparisons, not found duplicate\n", counter);
	return 0;
}

int duplicates2(int* a, size_t n) {
	printArray(a, n);
	printf("sorting... ");
	int* b = Xsort(a, n);
	printf("%d comparisons in sorting\n", sortcounter);
	printArray(b, n);
	
	int counter = 0;
	int i;
	for(i = 1; i < n; i++) {
		counter += 1;
		if(b[i] == b[i-1]) {
			printf("duplicates2(): %d comparisons, found duplicate %d\n", counter, b[i]);
			return 1;
		}
	}
	printf("duplicates2(): %d comparisons, not found duplicate\n", counter);
	return 0;
}

int main() {
	int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	
	duplicates1(arr, 10);
	putchar('\n');
	duplicates2(arr, 10);
	putchar('\n');
	
	sortcounter = 0;
	int arr2[10] = {7, 1, 3, 2, 5, 4, 6, 0, 8, 9};
	
	duplicates1(arr2, 10);
	putchar('\n');
	duplicates2(arr2, 10);
	putchar('\n');
	
	sortcounter = 0;
	int arr3[10] = {7, 1, 3, 2, 5, 4, 6, 0, 5, 9};
	
	duplicates1(arr3, 10);
	putchar('\n');
	duplicates2(arr3, 10);
	putchar('\n');
	
	sortcounter = 0;
	int arr4[10] = {9, 8, 7, 6, 5, 4, 3, 7, 1, 0};
	
	duplicates1(arr4, 10);
	putchar('\n');
	duplicates2(arr4, 10);
	putchar('\n');
	
	return 0;
}