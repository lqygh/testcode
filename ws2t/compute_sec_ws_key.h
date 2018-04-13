/*
 * compute_sec_ws_key.h
 *
 *  Created on: 12 Dec 2017
 *      Author: lqy
 */

#ifndef COMPUTE_SEC_WS_KEY_H_
#define COMPUTE_SEC_WS_KEY_H_

#define CONCAT_BUF_LEN 1024

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sha1.h"
#include "b64.h"

int compute_sec_ws_key(const char* sec_ws_key, char* output,
		const size_t output_buf_len, size_t* output_str_len);

#endif /* COMPUTE_SEC_WS_KEY_H_ */
