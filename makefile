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

mmapread:
	gcc -Wall -O3 mmapread.c -o mmapread

mmapheapsort:
	gcc -Wall -O3 mmapheapsort.c -o mmapheapsort

mmapmergesort:
	gcc -Wall -O3 mmapmergesort.c -o mmapmergesort

b:
	gcc -Wall -O3 b.c -lwiringPi -lpthread -o b

c:
	gcc -Wall -O3 c.c -lwiringPi -lpthread -o c

TempSensorSender:
	gcc -Wall -Wextra -O3 TempSensorSender.c -lwiringPi -lpthread -o TempSensorSender

TempSensorSender2302:
	gcc -Wall -Wextra -O3 TempSensorSender2302.c -lwiringPi -lpthread -o TempSensorSender2302

mtnc:
	gcc -Wall -Wextra -Werror -O3 mtnc.c -lpthread -o mtnc

slonc:
	gcc -Wall -Wextra -Werror -O3 slonc.c -lpthread -o slonc

fgv:
	gcc -Wall -Wextra -O3 fgv.c -o fgv

srvka:
	gcc -Wall -Wextra -O3 srvka.c -o srvka

srvqry:
	gcc -Wall -Wextra -O3 srvqry.c -o srvqry
