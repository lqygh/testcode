/*
 * ws_key_calc.c
 *
 *  Created on: 12 Dec 2017
 *      Author: lqy
 */

#include "compute_sec_ws_key.h"

int main() {
	char input[1024];
	char output[1024];

	while (1) {
		char* cret = fgets(input, sizeof(input), stdin);
		if (cret == NULL) {
			break;
		}
		{
			size_t len = strlen(cret);
			if (cret[len - 1] == '\n') {
				cret[len - 1] = '\0';
			}
		}

		size_t len = 0;
		int ret = compute_sec_ws_key(input, output, sizeof(output), &len);
		printf("Return value: %d, length: %zu, output: %s\n", ret, len, output);
	}

	return 0;
}
