#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct cell {
	void* value;
	struct cell* next;
};

struct queue {
	struct cell* first;
	struct cell* last;
};

void* newqueue() {
	struct queue* q = malloc(sizeof(struct queue));
	if(q == NULL) {
		printf("newqueue() error: cannot allocate memory\n");
		return q;
	}
	q->first = NULL;
	q->last = NULL;
	return q;
}

void* getfirst(struct queue* q) {
	if(q->first == NULL) { //when queue is empty
		printf("getfirst() error: empty queue\n");
		return NULL;
	} else {
		return q->first->value;
	}
}

void* getlast(struct queue* q) {
	if(q->first == NULL && q->last == NULL) { //when queue is empty
		printf("getlast() error: empty queue\n");
		return NULL;
	} else if(q->first != NULL && q->last == NULL) { //when queue has only 1 item
		return q->first->value;
	} else if(q->first != NULL && q->last != NULL) { //when queue has at least 2 items
		return q->last->value;
	} else { //when first is NULL while last is not NULL
		printf("getlast() error: queue first item should not be NULL when last item is not NULL\n");
		return NULL;
	}
}

int push(void* value, struct queue* q) {
	struct cell* newcell = malloc(sizeof(struct cell));
	if(newcell == NULL) {
		printf("push() error: cannot allocate memory\n");
		return -1;
	}
	if(q->first == NULL && q->last == NULL) { //when queue is empty
		newcell->value = value;
		newcell->next = NULL;
		q->first = newcell;
		return 1;
	} else if(q->first != NULL && q->last == NULL) { //when queue has only 1 item
		newcell->value = value;
		newcell->next = NULL;
		q->last = newcell;
		q->first->next = newcell;
		return 1;
	} else if(q->first != NULL && q->last != NULL) { //when queue has at least 2 items
		newcell->value = value;
		newcell->next = NULL;
		q->last->next = newcell;
		q->last = newcell;
		return 1;
	} else { //when first is NULL while last is not NULL
		printf("push() error: queue first item should not be NULL when last item is not NULL\n");
		free(newcell);
		return -1;
	}
}

int pop(void** value, struct queue* q) {
	struct cell* newfirst = NULL;
	if(q->first == NULL && q->last == NULL) { //when queue is empty
		printf("pop() error: cannot pop empty queue\n");
		return -1;
	} else if(q->first != NULL && q->last == NULL) { //when queue has only 1 item
		*value = q->first->value;
		free(q->first);
		q->first = NULL;;
		return 1;
	} else if(q->first != NULL && q->last != NULL) { //when queue has at least 2 items
		if(q->first->next == q->last) { //when there are only 2 items
			*value = q->first->value;
			free(q->first);
			q->first = q->last;
			q->last = NULL;
		} else { //when there are more than 2 items
			*value = q->first->value;
			newfirst = q->first->next;
			free(q->first);
			q->first = newfirst;
		}
		return 1;
	} else { //when first is NULL while last is not NULL
		printf("pop() error: queue first item should not be NULL when last item is not NULL\n");
		return -1;
	}
}

int main(int argc, char* argv[]) {
	struct timeval before;
	struct timeval after;
	struct queue* myqueue = newqueue();
	if(myqueue == NULL) {
		printf("failed to create empty queue\n");
		return 1;
	}
	int number = 0;
	if(argc > 1) {
		number = atoi(argv[1]);
	} else {
		number = 10000000;
	}
	int* vals = malloc(sizeof(int)*number);
	if(vals == NULL) {
		printf("failed to allocate memory for array");
		return 1;
	}
	int i = 0;
	int retval = 0;
	
	printf("number of items: %d\n", number);
	gettimeofday(&before, NULL);
	for(i = 0; i < number; i++) {
		//printf("adding %d into array\n", vals[i]);
		vals[i] = i;
	}
	gettimeofday(&after, NULL);
	printf("array operation time: %f seconds\n", (after.tv_sec-before.tv_sec) + (after.tv_usec-before.tv_usec)/1000000.0);
	
	gettimeofday(&before, NULL);
	for(i = 0; i < number; i++) {
		//printf("pushing %d into queue\n", vals[i]);
		retval = push(&vals[i], myqueue);
		if(retval != 1) { //when push() fails, go to pop() loop directly
			printf("can only push %d items at most\n", i);
			number = i;
			break;
		}
	}
	gettimeofday(&after, NULL);
	printf("push() operation time: %f seconds\n", (after.tv_sec-before.tv_sec) + (after.tv_usec-before.tv_usec)/1000000.0);
	
	gettimeofday(&before, NULL);
	for(i = 0; i < number; i++) {
		int* value = NULL;
		//printf("popping queue, ");
		retval = pop((void *)(&value), myqueue);
		/*if(retval != 1) {
			//failed
		} else {
			//printf("got %d\n", *value);
		}*/
	}
	gettimeofday(&after, NULL);
	printf("pop() operation time: %f seconds\n", (after.tv_sec-before.tv_sec) + (after.tv_usec-before.tv_usec)/1000000.0);

	free(vals);
	
	return 0;
}