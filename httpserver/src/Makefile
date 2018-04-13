CC=gcc
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_POSIX_SOURCE -Wall -Wextra -pedantic -std=c99 -O3 -mtune=native -D_GNU_SOURCE -pthread
SOURCES=connection_handler.c file_io.c process_uri.c process_request.c response_header.c vhost_table.c dir_listing.c http_server.c

all:
	$(CC) $(CFLAGS) $(SOURCES) -lssl -lcrypto -pthread -o http_server
