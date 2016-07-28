#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>

typedef struct passargs {
	char *port;
	void *data;
	int datalen;
} passargs;

typedef struct sensordata {
	char rhint;
	char rhdec;
	char tmpint;
	char tmpdec;
	char sum;
} sensordata;

pthread_t ondemandresponseudp_thread;
pthread_t ondemandresponsetcp_thread;
pthread_mutex_t lock;

void addrinfoiptotext(struct addrinfo *ai, char *text, int textsize) {
	if(textsize > 0) {
		
		memset(text, 0, textsize);
	
		if(textsize < INET6_ADDRSTRLEN && ai->ai_family == AF_INET6) {
			return;
		}
		if(textsize < INET_ADDRSTRLEN) {
			return;
		}
		
		if(ai->ai_family == AF_INET) {
			inet_ntop(ai->ai_family, &((struct sockaddr_in*)(ai->ai_addr))->sin_addr, text, textsize);
		} else if(ai->ai_family == AF_INET6) {
			inet_ntop(ai->ai_family, &((struct sockaddr_in6*)(ai->ai_addr))->sin6_addr, text, textsize);
		}
		
	}
}

void sockaddriptotext(struct sockaddr *ai_addr, char *text, int textsize) {
	if(textsize > 0) {
		
		memset(text, 0, textsize);
	
		if(textsize < INET6_ADDRSTRLEN && ai_addr->sa_family == AF_INET6) {
			return;
		}
		if(textsize < INET_ADDRSTRLEN) {
			return;
		}
		
		if(ai_addr->sa_family == AF_INET) {
			inet_ntop(ai_addr->sa_family, &(((struct sockaddr_in*)ai_addr)->sin_addr), text, textsize);
		} else if(ai_addr->sa_family == AF_INET6) {
			inet_ntop(ai_addr->sa_family, &(((struct sockaddr_in6*)ai_addr)->sin6_addr), text, textsize);
		}
		
	}
}

in_port_t get_in_port(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    }
    return (((struct sockaddr_in6*)sa)->sin6_port);
}

int readsensor(int pin, struct sensordata* sensorresult) {
	int bitcount = 0;
	int status;
	int laststatus;
	int arr[50];
	int zerocount = 0;
	int onecount = 0;
	int totalcount = 0;
	int i = 0;
	int j = 0;
	unsigned char result[5] = {0, 0, 0, 0, 0};
	
	unsigned int starttime;
	unsigned int totaltime;
	unsigned int lasttime;
	unsigned int interval;
	
	if(pin <= 0) {
		printf("Invalid pin number %d\n", pin);
		return -1;
	}
	
	//wiringPiSetupGpio();
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	delay(200);
	digitalWrite(pin, LOW);
	delay(5);
	digitalWrite(pin, HIGH);
	//delayMicroseconds(30);
	pinMode(pin, INPUT);
	pullUpDnControl(pin, PUD_UP);
	
	laststatus = digitalRead(pin);
	lasttime = micros();
	starttime = lasttime;
	
	/*while(1) {
		if(digitalRead(pin) == 0) {
			lasttime = micros();
			laststatus = 0;
			starttime = lasttime;
			break;
		}
	}*/
	
	printf("\nNow begin\n");
	
	while(1) {
		status = digitalRead(pin);
		if(status != laststatus) {
			interval = micros() - lasttime;
			printf("%u:%d ", interval, laststatus);
			if(laststatus) {
				bitcount++;
				if(bitcount >= 3 && interval <= 22) {
					arr[i] = 0;
					i++;
					zerocount++;
				} else if (bitcount >= 3 && interval <= 68) {
					arr[i] = 1;
					i++;
					onecount++;
				} else if (bitcount >= 3) {
					printf("abnormal interval:%d ", interval);
				}
			} 
			lasttime = micros();
			laststatus = status;
		}
		totaltime = micros() - starttime;
		if(bitcount >= 42) break;
		if(totaltime > 2500000) break;
	}
	
	printf("\n%d microseconds elapsed\n", micros() - starttime);
	totalcount = zerocount + onecount;
	printf("Now end\n");
	printf("Current pin status: %d\n\n", digitalRead(pin));
	
	for(i = 0; i < totalcount; i++) {
		printf("%d ", arr[i]);
		if((i+1) % 8 == 0) printf("\n");
	}
	
	printf("\nRead %d bits, %d bits valid\n", bitcount, totalcount);
	
	if(totalcount < 40) {
		printf("Less than 40 valid bits, now return error code :( \n");
		return -1;
	}
	
	i = 0;
	while(i < 5) {
		result[i] += arr[j];
		if((j+1) % 8 != 0) {
			result[i] <<= 1;
		} else {
			i++;
		}
	j++;
	}
	
	for(i = 0; i < 5; i++) {
		printf("%u ", result[i]);
	}
	
	printf("\n");
	
	pthread_mutex_lock(&lock);
	sensorresult->rhint = result[0];
	sensorresult->rhdec = result[1];
	sensorresult->tmpint = result[2];
	sensorresult->tmpdec = result[3];
	sensorresult->sum = result[4];
	pthread_mutex_unlock(&lock);
	
	if(result[0] + result[2] != result[4]) printf("Warning: %d + %d != %d\n", result[0], result[2], result[4]);
	
	return 1;
}

void *ondemandresponseudp(void *args) {
	struct addrinfo aihints;
	struct addrinfo *airesult;
	struct sockaddr_storage safrom;
	socklen_t salen = sizeof(safrom);
	char ip_text[INET6_ADDRSTRLEN];
	char recvbuffer[1500];
	struct passargs *arguments = (struct passargs *)args;
	
	printf("\nUDP listen thread: trying to listen on port %s\n", arguments->port);
	
	memset(&aihints, 0, sizeof(aihints));
	aihints.ai_flags = AI_PASSIVE;
    aihints.ai_family = AF_INET6;
    aihints.ai_socktype = SOCK_DGRAM;
    aihints.ai_protocol = 0;
	
	int gairet = getaddrinfo(NULL, arguments->port, &aihints, &airesult);
    if(gairet != 0) {
		fprintf(stderr, "\nUDP listen thread: getaddrinfo(): %s\n", gai_strerror(gairet));
        return NULL;
    } else {
		printf("\nUDP listen thread: getaddrinfo() succeeded \n");
    }
	
	int sockfd = socket(airesult->ai_family, airesult->ai_socktype, airesult->ai_protocol);
    if(sockfd == -1) {
		perror("\nUDP listen thread: socket()");
		return NULL;
    } else {
		printf("\nUDP listen thread: socket() succeeded, %d returned\n", sockfd);
    }
	
	int mode = 0;
	setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&mode, sizeof(mode));
	
	int bindret = bind(sockfd, airesult->ai_addr, airesult->ai_addrlen);
    if(bindret != 0) {
		perror("\nUDP listen thread: bind()");
		return NULL;
    } else {
		printf("\nUDP listen thread: bind() succeeded, %d returned\n", bindret);
    }
		
	//printf("\nUDP listen thread: sleep for 15 seconds\n");
	//sleep(15);
	printf("\nUDP listen thread: now begin\n");
	
	int rfret, stret;
	while(1) {
		rfret = recvfrom(sockfd, recvbuffer, sizeof(recvbuffer), 0, (struct sockaddr *)&safrom, &salen);
		if(rfret == -1) {
			perror("\nUDP listen thread: recvfrom()");
			sleep(1);
		} else {
		sockaddriptotext((struct sockaddr *)&safrom, ip_text, sizeof(ip_text));
		printf("\nUDP listen thread: recvfrom() returns %d, source: %s:%d\n", rfret, ip_text, ntohs(get_in_port((struct sockaddr *)&safrom)));
		}
		
		pthread_mutex_lock(&lock);
		stret = sendto(sockfd, arguments->data, arguments->datalen, 0, (struct sockaddr *)&safrom, salen);
		pthread_mutex_unlock(&lock);
		if(stret == -1) {
			perror("\nUDP listen thread: sendto()");
			sleep(1);
		} else {
		printf("\nUDP listen thread: recvfrom() returns %d\n", stret);
		}
		
		printf("\n\n");
	}	
}

void *ondemandresponsetcp(void *args) {
	struct addrinfo aihints;
	struct addrinfo *airesult;
	struct sockaddr_storage safrom;
	socklen_t salen = sizeof(safrom);
	char ip_text[INET6_ADDRSTRLEN];
	//char recvbuffer[1500];
	struct passargs *arguments = (struct passargs *)args; 
	
	printf("\nTCP listen thread: trying to listen on port %s\n", arguments->port);

	aihints.ai_flags = AI_PASSIVE;
    aihints.ai_family = AF_INET6;
    aihints.ai_socktype = SOCK_STREAM;
    aihints.ai_protocol = 0;
	
	int gairet = getaddrinfo(NULL, arguments->port, &aihints, &airesult);
    if(gairet != 0) {
		fprintf(stderr, "\nTCP listen thread: getaddrinfo(): %s\n", gai_strerror(gairet));
        return NULL;
    } else {
		printf("\nTCP listen thread: getaddrinfo() succeeded \n");
    }
	
	int sockfd = socket(airesult->ai_family, airesult->ai_socktype, airesult->ai_protocol);
    if(sockfd == -1) {
		perror("\nTCP listen thread: socket()");
		return NULL;
    } else {
		printf("\nTCP listen thread: socket() succeeded, %d returned\n", sockfd);
    }
	
	int mode = 0;
	setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&mode, sizeof(mode));
	
	int bindret = bind(sockfd, airesult->ai_addr, airesult->ai_addrlen);
    if(bindret != 0) {
		perror("\nTCP listen thread: bind()");
		return NULL;
    } else {
		printf("\nTCP listen thread: bind() succeeded, %d returned\n", bindret);
    }
		
	//printf("\nTCP listen thread: sleep for 15 seconds\n");
	//sleep(15);
	printf("\nTCP listen thread: now begin\n");
	
	int listenret = listen(sockfd, 20);
	if(listenret != 0) {
		perror("\nTCP listen thread: listen()");
		return NULL;
    } else {
		printf("\nTCP listen thread: listen() succeeded, %d returned\n", listenret);
    }
	
	int newfd, sret;
	while(1) {
		newfd = accept(sockfd, (struct sockaddr *)&safrom, &salen);
		if(newfd == -1) {
			perror("\nTCP listen thread: accept()");
			return NULL;
		} else {
			sockaddriptotext((struct sockaddr *)&safrom, ip_text, sizeof(ip_text));
			printf("\nTCP listen thread: accept() returns %d, source: %s:%d\n", newfd, ip_text, ntohs(get_in_port((struct sockaddr *)&safrom)));
		}
		
		pthread_mutex_lock(&lock);
		sret = send(newfd, arguments->data, arguments->datalen, 0);
		pthread_mutex_unlock(&lock);
		if(sret == -1) {
			perror("\nTCP listen thread: send()");
			sleep(1);
		} else {
		printf("\nTCP listen thread: send() returns %d\n", sret);
		close(newfd);
		}
		
		printf("\n\n");
	}	
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int gairet, sockfd, stret, resstrsize;
	int pin = 4;
	int responsesize = 24;
    char resstr[50];
	char response[100];
	char dstiptext[INET6_ADDRSTRLEN];
	struct sensordata sensorres;
	
	memset(&hints, 0, sizeof(hints));
	memset(&response, 0, sizeof(response));
    
	hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;

	if(argc < 6) {
		printf("Usage: %s <sensor pin> <udp send hostname> <udp send port> <udp listen port> <tcp listen port>\n", argv[0]);
		printf("Sensor pin might be 4\n");
		exit(1);
	} 
	
	pin = atoi(argv[1]);
	printf("Using sensor pin %d\n", pin);
	
    gairet = getaddrinfo(argv[2], argv[3], &hints, &res);
    if(gairet != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(gairet));
		exit(1);
	} else {
		printf("getaddrinfo() succeeded\n");
	}
	    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd == -1) {
		perror("socket()");
		exit(1);
    } else {
		printf("socket() succeeded, %d returned\n", sockfd);
    }

	if(pthread_mutex_init(&lock, NULL) != 0) {
		perror("pthread_mutex_init()");
		exit(1);
	} else {
		printf("pthread_mutex_init() succeeded\n");
	}
	
	struct passargs argumentsudp;
	argumentsudp.port = argv[4];
	argumentsudp.data = &response;
	argumentsudp.datalen = responsesize;
	if(pthread_create(&ondemandresponseudp_thread, NULL, ondemandresponseudp, &argumentsudp) != 0) {
		fprintf(stderr, "Warning: failed to create UDP listen thread\n");
	}
	
	struct passargs argumentstcp;
	argumentstcp.port = argv[5];
	argumentstcp.data = &response;
	argumentstcp.datalen = responsesize;
	if(pthread_create(&ondemandresponsetcp_thread, NULL, ondemandresponsetcp, &argumentstcp)) {
		fprintf(stderr, "Warning: failed to create TCP listen thread\n");
	}
	
	sleep(5);
	
	wiringPiSetupGpio();
	while(1) {
		if(readsensor(pin, &sensorres)) {
			if(sensorres.rhint + sensorres.tmpint == sensorres.sum || sensorres.rhint + sensorres.rhdec + sensorres.tmpint + sensorres.tmpdec == sensorres.sum) {
				resstrsize = snprintf(resstr, sizeof(resstr), "\ntext %d %d %d %d %d\n", sensorres.rhint, sensorres.rhdec, sensorres.tmpint, sensorres.tmpdec, sensorres.sum);
				
				memcpy(response, &sensorres, sizeof(sensorres));
				memcpy(response + sizeof(sensorres), resstr, resstrsize);
				
				responsesize = sizeof(sensorres) + resstrsize;
				
				argumentsudp.datalen = responsesize;
				argumentstcp.datalen = responsesize;
				
				stret = sendto(sockfd, &response, responsesize, 0, res->ai_addr, res->ai_addrlen);
				addrinfoiptotext(res, dstiptext, sizeof(dstiptext));
				printf("sendto() returns %d, destination: %s:%d\n", stret, dstiptext, ntohs(get_in_port(res->ai_addr)));
				if(stret == -1) {
					perror("sendto()");
				}
				
				printf("\n\n");
			}
		}
		
		sleep(10);
	}
	
    return 0;
}