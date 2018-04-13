#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
	if (argc < 1) {
		return 1;
	} else if (argc < 2) {
		printf("Usage: %s <IPv4 Address>\n", argv[0]);
		return 1;
	} else {
		unsigned char addr[4] = { 0 };
		int ret = inet_pton(AF_INET, argv[1], addr);
		if (ret != 1) {
			printf("inet_pton() failed\n");
			return 1;
		}

		char addr6[100] = { 0 };
		snprintf(addr6, sizeof(addr6), "2002:%02x%02x:%02x%02x::1", addr[0],
				addr[1], addr[2], addr[3]);

		if (argc == 4) {
			printf(
					"sudo ip tu ad %s mode sit remote 192.88.99.1 local %s ttl 200\n",
					argv[2], argv[3]);
			printf("sudo ip -6 ad ad %s dev %s\n", addr6, argv[2]);
			printf("sudo ip li se %s up\n", argv[2]);
			printf("sudo ip -6 ro ad default dev %s\n", argv[2]);
		} else {
			printf("%s\n", addr6);
		}

		return 0;
	}
}
