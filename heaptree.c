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

void bubbleDown(int i, int* heap, int* n) {
	if(left(i) > *n) { // no children
		return;
	} else if(right(i) > *n) { // only left child
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
		bubbleDown(i, heap, n);
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
		bubbleDown(1, heap, n);
		return deleted;
	}
}

void printHeapArray(int* heap, int* n) {
	printf("heap array is now ");
	int i;
	for(i = 1; i <= *n; i++) {
		printf("%d ", heap[i]);
	}
	printf("\n\n");
}

void printArray(int* a, int size) {
	int i;
	for(i = 0; i < size; i++) {
		printf("%d ", a[i]);
	}
	printf("\n\n");
}

void heapify(int* a, int size) {
	a = &a[-1];
	int i;
	for(i = size/2; i > 0; i--) {
		bubbleDown(i, a, &size);
	}
}

int main() {
	int heap[MAX+1]; // Stores priority values of nodes of heap tree
	int n = 0; // Largest position that has been filled so far
	
	int newvals[6] = {25, 12, 8, 17, 26, 18};
	int i;
	for(i = 0; i < 6; i++) {
		printf("inserting %d into heap tree\n", newvals[i]);
		insert(newvals[i], heap, &n);
		printHeapArray(heap, &n);
	}
	
	int del1 = deleteRoot(heap, &n);
	printf("deleting root node %d\n", del1);
	printHeapArray(heap, &n);
	
	int del2 = deleteRoot(heap, &n);
	printf("deleting root node %d\n", del2);
	printHeapArray(heap, &n);
	
	printf("inserting %d into heap tree\n", del1);
	insert(del1, heap, &n);
	printHeapArray(heap, &n);
		
	printf("inserting %d into heap tree\n", del2);
	insert(del2, heap, &n);
	printHeapArray(heap, &n);
	
	printf("original array:\n");
	printArray(newvals, 6);
	printf("after heapify:\n");
	heapify(newvals, 6);
	printArray(newvals, 6);
	
	return 0;
}