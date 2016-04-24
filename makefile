np:
	gcc -O3 np.c -o np

ztest:
	gcc -O3 ztest.c -lz -o ztest

ipdns1.0:
	gcc -O3 ipdns1.0.c -o ipdns1.0

twothreads:
	gcc -O3 -std=c99 twothreads.c -lpthread -o twothreads

linkedlist:
	gcc -O3 linkedlist.c -o linkedlist

bintree:
	gcc -O3 bintree.c -o bintree

BST:
	gcc -O3 BST.c -o BST

gmp_factorial:
	gcc -O3 -o gmp_factorial gmp_factorial.c -lgmp

ex4q7:
	gcc -O3 -o ex4q7 ex4q7.c -lgmp

templogger:
	gcc -O3 -Wall templogger.c -o templogger

all:	np	ztest	ipdns1.0	twothreads	linkedlist	bintree	BST

rm:
	rm np ztest ipdns1.0 twothreads linkedlist bintree BST
