CC=gcc
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_POSIX_SOURCE -Wall -Wextra -pedantic -std=c99 -O3 -mtune=native -D_GNU_SOURCE -pthread
SOURCES=main.c ws.c process_request.c compute_sec_ws_key.c decode.c encode.c sha1.c http_request.c http_response.c

sec_ws_key_calc:
	$(CC) $(CFLAGS) ws_key_calc.c compute_sec_ws_key.c decode.c encode.c sha1.c -o sec_ws_key_calc

main:
	$(CC) $(CFLAGS) $(SOURCES) -o main