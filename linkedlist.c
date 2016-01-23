#include <stdio.h>
#include <stdlib.h>

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
		printf("getlast() error: first item should not be NULL when last item is not NULL\n");
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
		newcell->next = q->first;
		q->last = newcell;
		return 1;
	} else if(q->first != NULL && q->last != NULL) { //when queue has at least 2 items
		newcell->value = value;
		newcell->next = q->last;
		q->last = newcell;
		return 1;
	} else { //when first is NULL while last is not NULL
		printf("push() error: queue first item should not be NULL when last item is not NULL\n");
		free(newcell);
		return -1;
	}
}

int pop(void** value, struct queue* q) {
	if(q->first == NULL && q->last == NULL) { //when queue is empty
		printf("pop() error: cannot pop empty queue\n");
		return -1;
	} else if(q->first != NULL && q->last == NULL) { //when queue has only 1 item
		*value = q->first->value;
		free(q->first);
		q->first = NULL;
		return 1;
	} else if(q->first != NULL && q->last != NULL) { //when queue has at least 2 items
		if(q->last->next == q->first) { //when there are only 2 items
			*value = q->first->value;
			free(q->first);
			q->first = q->last;
			q->first->next = NULL;
			q->last = NULL;
		} else { //when there are more than 2 items
			*value = q->first->value;
			//seek the second cell that will be the new first cell
			q->first = q->last->next;
			while(q->first->next->next != NULL) {
				q->first = q->first->next;
			}
			free(q->first->next); //free the old first cell
			q->first->next = NULL;
		}
		return 1;
	} else { //when first is NULL while last is not NULL
		printf("pop() error: queue first item should not be NULL when last item is not NULL\n");
		return -1;
	}
}

int main(int argc, char* argv[]) {
	struct queue* myqueue = newqueue();
	if(myqueue == NULL) {
		return 1;
	}
	int number = 100000;
	int* vals = malloc(sizeof(int)*number);
	int i = 0;
	int retval = 0;
	for(i = 0; i < number; i++) {
		vals[i] = i;
		printf("pushing %d into queue\n", vals[i]);
		retval = push(&vals[i], myqueue);
		if(retval != 1) {
			return 1;
		}
	}
	for(i = 0; i < number; i++) {
		int* value = NULL;
		printf("popping queue, ");
		retval = pop((void *)(&value), myqueue);
		if(retval != 1) {
			//failed
		} else {
			printf("got %d\n", *value);
		}
	}	
	return 0;
}