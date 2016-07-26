#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
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
int tretval = 0;

void ipbintotext(uint32_t ipbin, char *iptext, int iptextsize) {
	memset(iptext, 0, iptextsize);
	int q1,q2,q3,q4;
	q1 = (ipbin & 0xFF);
	q2 = (ipbin & 0xFF00) >> 8;
	q3 = (ipbin & 0xFF0000) >> 16;
	q4 = (ipbin & 0xFF000000) >> 24;
	sprintf(iptext, "%d.%d.%d.%d", q1, q2, q3, q4);
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
	
	wiringPiSetupGpio();
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
	//int tretval = 0;
	struct addrinfo aihints;
	struct addrinfo *airesult;
	struct sockaddr safrom;
	char ip_text[20];
	char recvbuffer[1500];
	struct passargs *arguments = (struct passargs *)args; 
	
	fprintf(stderr,"\n2nd thread : trying to listen on port %s\n", arguments->port);
	
	aihints.ai_flags = AI_PASSIVE;
    aihints.ai_family = AF_UNSPEC;
    aihints.ai_socktype = SOCK_DGRAM;
    aihints.ai_protocol = 0;
	int gairet = getaddrinfo(NULL, arguments->port, &aihints, &airesult);
    if(gairet != 0) {
		fprintf(stderr, "\n2nd thread : getaddrinfo error %s\n", gai_strerror(gairet));
        tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n2nd thread : getaddrinfo succeeded \n");
    }
	
	int sockfd = socket(airesult->ai_family, airesult->ai_socktype, airesult->ai_protocol);
    if(sockfd == -1) {
		fprintf(stderr, "\n2nd thread : socket error %d\n", sockfd);
		tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n2nd thread : socket returns %d\n", sockfd);
    }
	
	int bindret = bind(sockfd, airesult->ai_addr, airesult->ai_addrlen);
    if(bindret != 0) {
		fprintf(stderr, "\n2nd thread : bind error %d\n", bindret);
		tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n2nd thread : bind returns %d\n", bindret);
    }
	
	int salen = sizeof(struct sockaddr);
	int rfret, stret;
	int ipbin;
	
	sleep(15);
	fprintf(stderr,"\n2nd thread : now begin\n");
	
	while(1) {
		rfret = recvfrom(sockfd, recvbuffer, sizeof(recvbuffer), 0, &safrom, &salen);
		if(rfret == -1) {
			fprintf(stderr, "\n2nd thread : recvfrom error %d\n", rfret);
			perror(" ");
			tretval = 1;
			pthread_exit(&tretval);
		} else {
		ipbin = ((struct sockaddr_in *)&safrom)->sin_addr.s_addr;
		ipbintotext(ipbin, ip_text, sizeof(ip_text));
		fprintf(stderr, "\n2nd thread : recvfrom returns %d, source : %s\n", rfret, ip_text);
		}
		
		pthread_mutex_lock(&lock);
		stret = sendto(sockfd, arguments->data, arguments->datalen, 0, &safrom, salen);
		pthread_mutex_unlock(&lock);
		if(stret == -1) {
			fprintf(stderr, "\n2nd thread : sendto error %d\n", stret);
			tretval = 1;
			pthread_exit(&tretval);
		} else {
		fprintf(stderr, "\n2nd thread : sendto returns %d\n\n", stret);
		}
		fprintf(stderr, "\n\n");
	}	
}

void *ondemandresponsetcp(void *args) {
	//int tretval = 0;
	struct addrinfo aihints;
	struct addrinfo *airesult;
	struct sockaddr safrom;
	char ip_text[20];
	char recvbuffer[1500];
	struct passargs *arguments = (struct passargs *)args; 
	
	fprintf(stderr,"\n3rd thread : trying to listen on port %s\n", arguments->port);
	
	aihints.ai_flags = AI_PASSIVE;
    aihints.ai_family = AF_UNSPEC;
    aihints.ai_socktype = SOCK_STREAM;
    aihints.ai_protocol = 0;
	int gairet = getaddrinfo(NULL, arguments->port, &aihints, &airesult);
    if(gairet != 0) {
		fprintf(stderr, "\n3rd thread : getaddrinfo error %s\n", gai_strerror(gairet));
        tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n3rd thread : getaddrinfo succeeded \n");
    }
	
	int sockfd = socket(airesult->ai_family, airesult->ai_socktype, airesult->ai_protocol);
    if(sockfd == -1) {
		fprintf(stderr, "\n3rd thread : socket error %d\n", sockfd);
		tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n3rd thread : socket returns %d\n", sockfd);
    }
	
	int bindret = bind(sockfd, airesult->ai_addr, airesult->ai_addrlen);
    if(bindret != 0) {
		fprintf(stderr, "\n3rd thread : bind error %d\n", bindret);
		tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n3rd thread : bind returns %d\n", bindret);
    }
	
	int listenret = listen(sockfd, 10);
	if(listenret != 0) {
		fprintf(stderr, "\n3rd thread : listen error %d\n", listenret);
		tretval = 1;
		pthread_exit(&tretval);
    } else {
		fprintf(stderr, "\n3rd thread : listen returns %d\n", listenret);
    }
	
	int salen = sizeof(struct sockaddr);
	int newfd;
	int sret;
	int ipbin;
	
	sleep(15);
	fprintf(stderr,"\n3rd thread : now begin\n");
	
	while(1) {
		/*rfret = recvfrom(sockfd, recvbuffer, sizeof(recvbuffer), 0, &safrom, &salen);
		if(rfret == -1) {
			fprintf(stderr, "\n2nd thread : recvfrom error %d\n", rfret);
			perror(" ");
			tretval = 1;
		} else {
		fprintf(stderr, "\n2nd thread : recvfrom returns %d\n", rfret);
		}*/
		
		newfd = accept(sockfd, &safrom, &salen);
		if(newfd == -1) {
			fprintf(stderr, "\n3rd thread : accept error %d\n", newfd);
			tretval = 1;
			pthread_exit(&tretval);
		} else {
			ipbin = ((struct sockaddr_in *)&safrom)->sin_addr.s_addr;
			ipbintotext(ipbin, ip_text, sizeof(ip_text));
			fprintf(stderr, "\n3rd thread : accept returns %d, source: %s\n", newfd, ip_text);
		}
		
		pthread_mutex_lock(&lock);
		sret = send(newfd, arguments->data, arguments->datalen, 0);
		pthread_mutex_unlock(&lock);
		if(sret == -1) {
			fprintf(stderr, "\n3rd thread : send error %d\n", sret);
			tretval = 1;
			pthread_exit(&tretval);
		} else {
		fprintf(stderr, "\n3rd thread : send returns %d\n\n", sret);
		}
		close(newfd);
		fprintf(stderr, "\n\n");
	}	
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int gairet, sockfd, satolen, stret, count;
	int ipbin;
	int pin = 4;
    satolen = sizeof(struct sockaddr); //important 
    char dst[INET_ADDRSTRLEN];
    struct sockaddr sato;
    char resstr[50];
	char response[100];
	char ip_text[20];
	struct sensordata sensorres;
	
	memset(&response, 0, sizeof(response));
    memset(&hints, 0, sizeof(hints));
    memset(&sato, 0, sizeof(sato));
    
	hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;

	if(argc < 5) {
		fprintf(stderr, "usage: %s hostname hostport listenportudp listenporttcp\n", argv[0]);
		exit(1);
	} 
	
    gairet = getaddrinfo(argv[1], argv[2], &hints, &res);
    if(gairet != 0) {
		fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(gairet));
        exit(1);
    } else {
		fprintf(stderr, "getaddrinfo succeeded \n");
    }
	
	sato = *(res->ai_addr); //important
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd == -1) {
		fprintf(stderr, "socket error %d\n", sockfd);
		exit(1);
    } else {
		fprintf(stderr, "socket returns %d\n", sockfd);
    }
	
	int responsesize = 24;

	if(pthread_mutex_init(&lock, NULL)) {
		fprintf(stderr,"error creating mutex\n");
		exit(1);
	}
	
	struct passargs argumentsudp;
	argumentsudp.port = argv[3];
	argumentsudp.data = &response;
	argumentsudp.datalen = responsesize;
	if(pthread_create(&ondemandresponseudp_thread, NULL, ondemandresponseudp, &argumentsudp)) {
		fprintf(stderr, "error creating udp thread\n");
		exit(1);
	}
	/*if(pthread_join(ondemandresponseudp_thread, NULL)) {
		fprintf(stderr, "error joining thread\n");
		exit(1);
	}*/
	
	struct passargs argumentstcp;
	argumentstcp.port = argv[4];
	argumentstcp.data = &response;
	argumentstcp.datalen = responsesize;
	if(pthread_create(&ondemandresponsetcp_thread, NULL, ondemandresponsetcp, &argumentstcp)) {
		fprintf(stderr, "error creating tcp thread\n");
		exit(1);
	}
	
	sleep(5);
	
	while(1) {
    if(readsensor(pin, &sensorres)) {
		if(sensorres.rhint + sensorres.tmpint == sensorres.sum) {
			count = snprintf(resstr, sizeof(resstr), "\ntext %d %d %d %d %d\n", sensorres.rhint, sensorres.rhdec, sensorres.tmpint, sensorres.tmpdec, sensorres.sum);
			memcpy(response, &sensorres, sizeof(sensorres));
			memcpy(response+sizeof(sensorres), resstr, count);
			responsesize = sizeof(sensorres)+count;
			argumentsudp.datalen = responsesize;
			argumentstcp.datalen = responsesize;
			stret = sendto(sockfd, &response, responsesize, 0, &sato, satolen);
			ipbin = ((struct sockaddr_in *)&sato)->sin_addr.s_addr;
			ipbintotext(ipbin, ip_text, sizeof(ip_text));
			fprintf(stderr, "sendto returns %d, destination: %s", stret, ip_text);
			perror(" ");
			fprintf(stderr, "\n\n");
		}
	}
	sleep(8);
    }
    return 0;
}