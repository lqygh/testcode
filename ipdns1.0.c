#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
	int status;
	struct addrinfo hints;
	struct addrinfo *results;
	int sockfd;
	char *recvbuffer;
	char *sendbuffer;
	struct sockaddr dest;
	int destlen;
	int i;
	int j;

	uint32_t hostip;
	uint32_t answerip;
	char hostiptext[20];
	uint16_t hostport;
	uint16_t id;
	uint16_t flags;
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
	char strleng;
	uint16_t qtype;
	uint16_t qclass;

	int headerlen = 12;
	int qlen;
	char *recvbufferbegin;
	char *sendbufferbegin;
	char *header;
	char *headerbegin;
	char *query;
	char *querybegin;
	char *answer;
	char *answerbegin;
	int anslen;
	int totallen;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if (argc <= 1) {
		fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
		fprintf(stderr, "       %s <port number> <IPv4 address>\n", argv[0]);
		exit(1);
	}

	if (argc >= 3) {
		int ret = inet_pton(AF_INET, argv[2], &answerip);
		if (ret != 1) {
			fprintf(stderr, "inet_pton error");
			exit(1);
		}
	}

	status = getaddrinfo(NULL, argv[1], &hints, &results);
	if (status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket error");
		exit(1);
	}

	status = bind(sockfd, results->ai_addr, results->ai_addrlen);
	if (status != 0) {
		perror("bind error");
		exit(1);
	}

	recvbuffer = calloc(1800, 1);
	if (recvbuffer == NULL) {
		perror("calloc error");
		exit(1);
	}
	recvbufferbegin = recvbuffer;

	sendbuffer = calloc(1800, 1);
	if (sendbuffer == NULL) {
		perror("calloc error");
		exit(1);
	}
	sendbufferbegin = sendbuffer;

	header = calloc(1800, 1);
	if (header == NULL) {
		perror("calloc error");
		exit(1);
	}
	headerbegin = header;

	query = calloc(1800, 1);
	if (query == NULL) {
		perror("calloc error");
		exit(1);
	}
	querybegin = query;

	answer = calloc(1800, 1);
	if (answer == NULL) {
		perror("calloc error");
		exit(1);
	}
	answerbegin = answer;

	printf("Now listening on port %s\n\n", argv[1]);

	destlen = sizeof(struct sockaddr);

	while (1) {
		status = recvfrom(sockfd, recvbuffer, 1800, 0, &dest, &destlen);

		if (status == -1) {
			perror("recvfrom error");
			continue;
		} else
			printf("received %d bytes ", status);

		hostip = ((struct sockaddr_in*) &dest)->sin_addr.s_addr;
		inet_ntop(AF_INET, &hostip, hostiptext, sizeof(hostiptext));
		hostport = ntohs(((struct sockaddr_in*) &dest)->sin_port);

		printf("from %s:%d\n\n", hostiptext, hostport);

		id = ntohs(*((uint16_t *) recvbuffer));
		printf("ID is %#04x\n", id);
		recvbuffer += 2;

		flags = ntohs(*((uint16_t *) recvbuffer));
		printf("Flags is %#04x\n", flags);
		recvbuffer += 2;

		qdcount = ntohs(*((uint16_t *) recvbuffer));
		printf("Query count is %u\n", qdcount);
		recvbuffer += 2;

		ancount = ntohs(*((uint16_t *) recvbuffer));
		printf("Answer count is %u\n", ancount);
		recvbuffer += 2;

		nscount = ntohs(*((uint16_t *) recvbuffer));
		printf("Record count is %u\n", nscount);
		recvbuffer += 2;

		arcount = ntohs(*((uint16_t *) recvbuffer));
		printf("Additional record count is %u\n\n", arcount);
		recvbuffer += 2;

		i = 0;
		qlen = 0;
		while (1) {
			i++;
			strleng = *recvbuffer;
			printf("string %d length is %u ", i, strleng);
			recvbuffer++;
			printf("content is ");
			for (j = 0; j < strleng; j++) {
				printf("%c", *recvbuffer);
				recvbuffer++;
			}
			printf("\n\n");
			if (*recvbuffer == 0)
				break;
			if (i > 30)
				break;
		}

		recvbuffer++;

		qtype = ntohs(*((uint16_t *) recvbuffer));
		printf("Query type is %u\n", qtype);
		recvbuffer += 2;

		qclass = ntohs(*((uint16_t *) recvbuffer));
		printf("Query class is %u\n", qclass);
		recvbuffer += 2;

		qlen = recvbuffer - headerlen - recvbufferbegin;
		printf("Query length is %d\n\n", qlen);

		recvbuffer = recvbufferbegin;

		printf("----------------------------------------\n\n");

		id = htons(id);
		flags = htons(0x8180);
		qdcount = htons(qdcount);
		ancount = htons(1);
		nscount = htons(0);
		arcount = htons(0);
		memcpy(header, (char *) &id, 2);
		header += 2;
		memcpy(header, (char *) &flags, 2);
		header += 2;
		memcpy(header, (char *) &qdcount, 2);
		header += 2;
		memcpy(header, (char *) &ancount, 2);
		header += 2;
		memcpy(header, (char *) &nscount, 2);
		header += 2;
		memcpy(header, (char *) &arcount, 2);
		header = headerbegin;

		memcpy(query, (recvbufferbegin + headerlen), qlen);

		*answer = 0xc0;
		*(answer + 1) = 0x0c;
		*(answer + 2) = 0x00;
		*(answer + 3) = 0x01;
		*(answer + 4) = 0x00;
		*(answer + 5) = 0x01;
		*(answer + 6) = 0x00;
		*(answer + 7) = 0x00;
		*(answer + 8) = 0x00;
		*(answer + 9) = 0x05;
		*(answer + 10) = 0x00;
		*(answer + 11) = 0x04;
		if (argc < 3) {
			memcpy(answer + 12, &hostip, 4);
		} else {
			memcpy(answer + 12, &answerip, 4);
		}
		anslen = 16;

		memcpy(sendbuffer, header, headerlen);
		sendbuffer += headerlen;
		memcpy(sendbuffer, query, qlen);
		sendbuffer += qlen;
		memcpy(sendbuffer, answer, anslen);
		sendbuffer = sendbufferbegin;

		totallen = headerlen + qlen + anslen;

		sendto(sockfd, sendbuffer, totallen, 0, &dest, destlen);

	}

	free(recvbuffer);
	free(sendbuffer);
	free(header);
	free(query);
	free(answer);

	return 0;
}
