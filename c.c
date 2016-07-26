#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

#define DELAY 500

int main(int argc, char* argv[]) {
	if(argc < 3) {
		return 1;
	}

	int pin = 47;
	
	wiringPiSetupGpio();
	pinMode(pin, OUTPUT);
	
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);

	while(1) {

		digitalWrite(pin, HIGH);

		usleep(a*DELAY);
		
		digitalWrite(pin, LOW);
		
		usleep(b*DELAY);

	}
	
	return 0;
}
