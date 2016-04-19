#include <stdio.h>
#include <stdlib.h>

void printArray(int* a, int size) {
	int i;
	for(i = 0; i < size; i++) {
		printf("%d ", a[i]);
	}
	printf("\n");
}


int partition(int* a, int left, int right) {
	int* b = calloc(right-left+1, sizeof(int));
	int pivotindex = (left+right)/2;
	int pivot = a[pivotindex];
	int acount = left;
	int bcount = 1;
	
	int i;
	for(i = left; i <= right; i++) {
		if(i == pivotindex) {
			b[0] = a[i];
		} else if(a[i] < pivot || (a[i] == pivot && i < pivotindex)) {
			a[acount++] = a[i];
		} else {
			b[bcount++] = a[i];
		}
	}
	
	for(i = 0; i < bcount; i++) {
		a[acount++] = b[i];
	}
	free(b);
	
	return right-bcount+1;
}

void quicksort(int* a, int left, int right) {
	if(left < right) {
		printf("quicksort from %d to %d\n", left, right);
		printf("before partition from %d to %d with pivot %d at index %d:\n", left, right, a[(left+right)/2], (left+right)/2);
		printArray(a, 11);
		int pivotindex = partition(a, left, right);
		printf("after partition from %d to %d with pivot %d at index %d:\n", left, right, a[pivotindex], pivotindex);
		printArray(a, 11);
		putchar('\n');
		printf("calling quicksort from %d to %d\n", left, pivotindex-1);
		quicksort(a, left, pivotindex-1);
		printf("calling quicksort from %d to %d\n", pivotindex+1, right);
		quicksort(a, pivotindex+1, right);
	} else {
		printf("quicksort from %d to %d does nothing\n", left, right);
	}
	putchar('\n');
	putchar('\n');
}

int main() {
	int a[] = {3, 7, 2, 4, 9, 6, 8, 7, 5, 1, 6};
	
	//int ret = partition(a, 1, 4);
	//printf("partition returns %d\n", ret);
	
	quicksort(a, 0, 10);
	
	printf("sorted array:\n");
	printArray(a, 11);
	
	return 0;
}