#include <stdio.h>
#include <math.h>

#define MAX 100 // Maximum number of nodes allowed

/* Implementation of heap tree from DSA lecture notes (with some slight modifications) */

// Swap two integer values
void swap(int* a, int* b) {
	int c = *a;
	*a = *b;
	*b = c;
}

int isRoot(int i) {
	return i == 1;
}

int level(int i) {
	return log2(i);
}

int parent(int i) {
	return i / 2;
}

int left(int i) {
	return 2 * i;
}

int right(int i) {
	return 2 * i + 1;
}

int heapEmpty(int* heap, int* n) {
	return *n == 0;
}

int root(int* heap, int* n) {
	if(heapEmpty(heap, n)) {
		fprintf(stderr, "Heap is empty\n");
		return -1;
	} else {
		return heap[1];
	}
}

int lastLeaf(int* heap, int* n) {
	if(heapEmpty(heap, n)) {
		fprintf(stderr, "Heap is empty\n");
		return -1;
	} else {
		return heap[*n];
	}
}

void bubbleUp(int i, int* heap, int* n) {
	if(isRoot(i)) {
		return;
	} else if(heap[i] > heap[parent(i)]) {
		swap(&heap[i], &heap[parent(i)]);
		bubbleUp(parent(i), heap, n);
	}
}

void bubbleDown(int i, int* heap, int n) {
	if(left(i) > n) { // no children
		return;
	} else if(right(i) > n) { // only left child
		if(heap[i] < heap[left(i)]) {
			swap(&heap[i], &heap[left(i)]);
		}
	} else { // two children
		if(heap[left(i)] > heap[right(i)] && heap[i] < heap[left(i)]) {
			swap(&heap[i], &heap[left(i)]);
			bubbleDown(left(i), heap, n);
		} else if(heap[i] < heap[right(i)]) {
			swap(&heap[i], &heap[right(i)]);
			bubbleDown(right(i), heap, n);
		}
	}
}

void insert(int p, int* heap, int* n) {
	if(*n == MAX) {
		fprintf(stderr, "Heap is full\n");
	} else {
		*n += 1;
		heap[*n] = p;
		bubbleUp(*n, heap, n);
	}
}

int delete(int i, int* heap, int* n) {
	if(*n < i) {
		fprintf(stderr, "Node does not exist\n");
		return -1;
	} else {
		int deleted = heap[i];
		heap[i] = heap[*n];
		*n -= 1;
		bubbleUp(i, heap, n);
		bubbleDown(i, heap, *n);
		return deleted;
	}
}

int deleteRoot(int* heap, int* n) {
	if(*n < 1) {
		fprintf(stderr, "Node does not exist\n");
		return -1;
	} else {
		int deleted = heap[1];
		heap[1] = heap[*n];
		*n -= 1;
		bubbleDown(1, heap, *n);
		return deleted;
	}
}

void printHeapArray(int* heap, int n) {
	printf("array is ");
	int i;
	for(i = 1; i <= n; i++) {
		printf("%d ", heap[i]);
	}
	printf("\n");
}

void printArray(int* a, int size) {
	int i;
	for(i = 0; i < size; i++) {
		printf("%d ", a[i]);
	}
	printf("\n");
}

void heapify(int* a, int size) {
	//a = &a[-1];
	int i;
	for(i = size/2; i > 0; i--) {
		bubbleDown(i, a, size);
	}
}

void heapSort(int* a, int n) {
	printf("Original array:\n");
	printHeapArray(a, n);
	putchar('\n');
	
	heapify(a, n);
	printf("After heapify:\n");
	printHeapArray(a, n);
	putchar('\n');
	
	//putchar('\n');
	
	int j;
	for(j = n; j > 1; j--) {
		printf("After swapping %d and %d:\n", a[1], a[j]);
		swap(&a[1], &a[j]);
		printHeapArray(a, n);
		putchar('\n');
		
		printf("After bubbleDown %d:\n", a[1]);
		bubbleDown(1, a, j-1);
		printHeapArray(a, n);
		putchar('\n');
		
		printf("Unsorted part:\n");
		printHeapArray(a, j-1);
		putchar('\n');
		
		printf("Sorted part:\n");
		printHeapArray(&a[j-1], n-j+1);
		putchar('\n');
		
		putchar('\n');
	}
	
	printf("Array is now sorted:\n");
	printHeapArray(a, n);
}

int main() {
	
	int arr[] = {-100, 5, 3, 4, 6, 8, 4, 1, 9, 7, 1, 2};
	
	heapSort(arr, 11);
	
	return 0;
}