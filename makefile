nclient:
	gcc -O3 -Wall -Wextra nclient.c netutil/netutil.c -lpthread -o nclient

sdl0:
	gcc -O3 -Wall -Wextra -std=c99 sdl0.c matrix/matrix.c -march=native `sdl2-config --cflags --libs` -lm -o sdl0

sdl0a:
	gcc -O3 -Wall -Wextra -std=c99 sdl0.c matrix/matrix.c `sdl2-config --cflags --libs` -lm -o sdl0a

sodium0:
	gcc -O3 -Wall -Wextra sodium0.c -I/usr/local/include -L/usr/local/lib -lsodium -o sodium0

sodium1:
	gcc -O3 -Wall -Wextra tiny-AES128-C/aes.c sodium1.c -I/usr/local/include -L/usr/local/lib -lsodium -o sodium1

sodium2:
	gcc -O3 -Wall -Wextra -mtune=native sodium2.c -ldl -o sodium2
	
packet_encoder:
	gcc -O3 -Wall -Wextra -shared -fPIC -mtune=native tiny-AES128-C/aes.c packet_encoder.c -I/usr/local/include -L/usr/local/lib -lsodium -lpthread -o packet_encoder.so
	
packet_encoder_xs20:
	gcc -O3 -Wall -Wextra -shared -fPIC -mtune=native packet_encoder_xs20.c -I/usr/local/include -L/usr/local/lib -lsodium -lpthread -o packet_encoder_xs20.so
	
aes0:
	gcc -O3 -Wall -Wextra tiny-AES128-C/aes.c aes0.c -o aes0

montedice:
	gcc -O3 -Wall -Wextra montedice.c -o montedice

monte:
	gcc -O3 -Wall -Wextra monte.c -o monte

hook:
	gcc -shared -fPIC -O3 -Wall -Wextra recvfrom_hook.c -ldl -o recvfrom_hook.so

hook32:
	gcc -shared -fPIC -O3 -Wall -Wextra -m32 recvfrom_hook.c -ldl -o recvfrom_hook.so

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

mmapsizeoffset:
	gcc -Wall -O3 mmapsizeoffset.c -o mmapsizeoffset

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

closefd:
	gcc -Wall -Wextra -Werror -O3 closefd.c -lpthread -o closefd

fgv:
	gcc -Wall -Wextra -O3 fgv.c -o fgv

srvka:
	gcc -Wall -Wextra -O3 srvka.c -o srvka

srvqry:
	gcc -Wall -Wextra -O3 srvqry.c -o srvqry
