#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
 
/* A binary tree node has data, pointer to left child
   and a pointer to right child */
struct node
{
    int data;
    struct node* left;
    struct node* right;
};
 
int isBSTUtil(struct node* node, int min, int max);
 
/* Returns true if the given tree is a binary search tree 
 (efficient version). */
int isBST(struct node* node) 
{ 
  return(isBSTUtil(node, INT_MIN, INT_MAX)); 
} 
 
/* Returns true if the given tree is a BST and its 
   values are >= min and <= max. */
int isBSTUtil(struct node* node, int min, int max) 
{ 
 
  /* an empty tree is BST */
  if (node==NULL) 
     return 1;
       
  /* false if this node violates the min/max constraint */ 
  if (node->data < min || node->data > max) 
     return 0; 
 
  /* otherwise check the subtrees recursively, 
   tightening the min or max constraint */
  return
    isBSTUtil(node->left, min, node->data-1) &&  // Allow only distinct values
    isBSTUtil(node->right, node->data+1, max);  // Allow only distinct values
} 
 
/* Helper function that allocates a new node with the
   given data and NULL left and right pointers. */
struct node* newNode(int data)
{
  struct node* node = (struct node*)
                       malloc(sizeof(struct node));
  node->data = data;
  node->left = NULL;
  node->right = NULL;
 
  return(node);
}

// Traverse the tree in-order and print the values of each node
void printInOrder(struct node* node) {
	if(node != NULL) {
		printInOrder(node->left);
		printf("%d, ", node->data);
		printInOrder(node->right);
	}
}

/* Node for a linked list */
struct listnode
{
	struct node* data;
	struct listnode* next;
};

struct queue
{
	struct listnode* first;
	struct listnode* last;
};

struct queue* newQueue() {
	struct queue* newq = malloc(sizeof(struct queue));
	newq->first = NULL;
	newq->last = NULL;

	return newq;
}

void queue_push(struct queue* queue, struct node* data) {
	if(queue->first == NULL) {
		queue->first = malloc(sizeof(struct listnode));
		queue->last = queue->first;
		queue->first->data = data;
		queue->first->next = NULL;
	} else {
		struct listnode* oldlast = queue->last;
		
		queue->last = malloc(sizeof(struct listnode));
		queue->last->data = data;
		queue->last->next = NULL;

		oldlast->next = queue->last;
	}
}

void bst2list(struct queue* queue, struct node* node) {
	if(node != NULL) {
		bst2list(queue, node->left);
		queue_push(queue, node);
		bst2list(queue, node->right);
	}
}

// ascending order
int isSorted(struct listnode* ln) {
	if(ln == NULL || ln->next == NULL) {
		return 1;
	}

	if(ln->data->data >= ln->next->data->data) {
		return 0;
	}

	//printf("comparison made: %d and %d\n", ln->data->data, ln->next->data->data);
	return isSorted(ln->next);
}

// search the tree for a given value
int isIn(int value, struct node* tree, int* counter) {
	if(tree == NULL) {
		return 0;
	} else if(value == tree->data) {
		*counter +=1;
		return 1;
	} else if(value < tree->data) {
		*counter +=1;
		return isIn(value, tree->left, counter);
	} else {
		*counter +=1;
		return isIn(value, tree->right, counter);
	}
}

/* Convert a binary search tree into a linked list of nodes in ascending order
struct listnode* bst2list(struct 

/* Driver program to test above functions*/
int main()
{
  /*struct node *root = newNode(4);
  root->left        = newNode(2);
  root->right       = newNode(5);
  root->left->left  = newNode(1);
  root->left->right = newNode(3);*/

	struct node *root = newNode(8);

	root->left = newNode(4);
	root->right = newNode(12);

	root->left->left = newNode(2);
	root->left->right = newNode(6);

	root->left->left->left = newNode(1);
	root->left->left->right = newNode(3);

	root->left->right->left = newNode(5);
	root->left->right->right = newNode(7);

	root->right->left = newNode(10);
	root->right->right = newNode(14);

	root->right->left->left = newNode(9);
	root->right->left->right = newNode(11);

	root->right->right->left = newNode(13);
	root->right->right->right = newNode(15);

 
  if(isBST(root))
    printf("Is BST");
  else
    printf("Not a BST");

	printf("\n");
	
	//printInOrder(root);
	//printf("\n");    
 
  //getchar();
  
	printf("Converting tree to linked list\n");
	struct queue* myqueue = newQueue();
	bst2list(myqueue, root);

	struct listnode* ln = myqueue->first;
	
	while(ln != NULL) {
		printf("%d ", ln->data->data);
		ln = ln->next;
	}
	printf("\n");

	if(isSorted(myqueue->first)) {
		printf("List is sorted\n");
	}
	else {
		printf("List is not sorted\n");
	}

	int i = 0;
	int counter = 0;
	int tcounter = 0;
	for(i = 1; i <= 15; i++) {
		printf("Searching %d in the tree: ", i);
		
		int retval = isIn(i, root, &counter);
		if(retval) {
			printf("found ");
		} else {
			printf("not found ");
		}
		
		printf("Number of comparisons: %d\n", counter);
		tcounter += counter;
		counter = 0;
	}
	printf("Total number of comparisons: %d\n", tcounter);
	printf("Average number of comparisons for one number: %f\n", tcounter/15.0);

  return 0;
}  
