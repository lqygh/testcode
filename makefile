np:
	gcc -O3 np.c -o np

ztest:
	gcc -O3 ztest.c -lz -o ztest

ipdns1.0:
	gcc -O3 ipdns1.0.c -o ipdns1.0

tt:
	gcc -O3 -std=c99 twothreads.c -lpthread -o twothreads

all:	np	ztest	ipdns1.0	tt

