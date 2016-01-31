#include <stdio.h>
#include <stdlib.h>

struct node {
	void* label;
	struct node* left;
	struct node* right;
};

void* emptytree() {
	return NULL;
}

void* maketree(void* label, void* left, void* right) {
	struct node* newnode = malloc(sizeof(struct node));
	if(newnode == NULL) {
		fprintf(stderr, "maketree(): failed to allocate memory\n");
		return newnode;
	}
	
	newnode->label = label;
	newnode->left = left;
	newnode->right = right;
	return newnode;
}

int isempty(void* tree) {
	if(tree == NULL) {
		return 1;
	} else {
		return 0;
	}
}

void* root(void* tree) {
	if(tree == NULL) {
		fprintf(stderr, "root(): empty tree\n");
		return tree;
	}
	
	return ((struct node*)tree)->label;
}

void* left(void* tree) {
	if(tree == NULL) {
		fprintf(stderr, "left(): empty tree\n");
		return tree;
	}
	
	return ((struct node*)tree)->left;
}

void* right(void* tree) {
	if(tree == NULL) {
		fprintf(stderr, "right(): empty tree\n");
		return tree;
	}
	
	return ((struct node*)tree)->right;
}

void* leaf(void* label) {
	return maketree(label, emptytree(), emptytree());
}

int size(void* tree) {
	if(isempty(tree)) {
		return 0;
	} else {
		return (1 + size(left(tree)) + size(right(tree)));
	}
}

int isleaf(void* tree) {
	if(isempty(tree)) {
		return 0;
	} else {
		return isempty(left(tree)) && isempty(right(tree));
	}
}

int numleaves(void* tree) {
	if(isempty(tree)) {
		return 0;
	} else if(isleaf(tree)) {
		return 1;
	} else {
		return numleaves(left(tree)) + numleaves(right(tree));
	}
}

int equalbintree(void* tree1, void* tree2) {
	if(isempty(tree1) && isempty(tree2)) {
		return 1;
	} else if(isempty(tree1) || isempty(tree2)) {
		return 0;
	} else if(root(tree1) != root(tree2)) {
		return 0;
	} else {
		return equalbintree(left(tree1), left(tree2)) && equalbintree(right(tree1), right(tree2));
	}
}

int main(int argc, char* argv[]) {
	int labels[16];
	int i = 0;
	for(i = 0; i < 16; i++) {
		labels[i] = i;
	}
	
	void* t = maketree(&labels[8], maketree(&labels[3], leaf(&labels[1]), maketree(&labels[6], emptytree(), leaf(&labels[7]))), maketree(&labels[11], maketree(&labels[9], emptytree(), leaf(&labels[10])), maketree(&labels[14], leaf(&labels[12]), leaf(&labels[15]))));
	
	printf("struct node size: %lu\n", sizeof(struct node));
	printf("tree pointer: %p\n", t);
	printf("size(): %d\n", size(t));
	printf("numleaves(): %d\n", numleaves(t));
	printf("equalbintree(): %d\n", equalbintree(t, t));
	
	return 0;
}
