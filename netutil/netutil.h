#ifndef NETUTIL_INCLUDED
#define NETUTIL_INCLUDED

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MACADDRSTRLEN 18

void macaddr_to_text(uint64_t macaddr, char* text, int textlen);

void sockaddr_ip_to_text(struct sockaddr* ai_addr, char* text, int textlen);

in_port_t get_in_port(struct sockaddr* sa);

int is_ip_same(struct sockaddr* a, struct sockaddr* b);

ssize_t tcp_read(int fd, void* buf, size_t count);

ssize_t tcp_write(int fd, void* buf, size_t count);

int create_udp_server_socket(char* port);

#endif
