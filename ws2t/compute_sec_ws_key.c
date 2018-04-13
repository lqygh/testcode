/*
 * compute_sec_ws_key.c
 *
 *  Created on: 7 Dec 2017
 *      Author: lqy
 */

#include "compute_sec_ws_key.h"

int compute_sec_ws_key(const char* sec_ws_key, char* output,
		const size_t output_buf_len, size_t* output_str_len) {
	if (sec_ws_key == NULL || output == NULL) {
		return -1;
	}

	char guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	size_t guid_len = strlen(guid);
	size_t sec_ws_key_len = strlen(sec_ws_key);

	if (sec_ws_key_len + guid_len + 1 > CONCAT_BUF_LEN) {
		return -1;
	}

	/* concatenate sec_ws_key + guid */
	char concat_buf[CONCAT_BUF_LEN] = { 0 };
	memcpy(concat_buf, sec_ws_key, sec_ws_key_len);
	memcpy(concat_buf + sec_ws_key_len, guid, guid_len);
	concat_buf[sec_ws_key_len + guid_len] = '\0';

	/* calculate SHA1 of concat_buf */
	uint8_t concat_buf_sha1[20];
	{
		SHA1_CTX sha;
		SHA1Init(&sha);
		SHA1Update(&sha, (uint8_t *) concat_buf, sec_ws_key_len + guid_len);
		SHA1Final(concat_buf_sha1, &sha);
	}

	/* calculate base64 of concat_buf_sha1 */
	char* concat_buf_sha1_b64 = b64_encode(concat_buf_sha1,
			sizeof(concat_buf_sha1));
	if (concat_buf_sha1_b64 == NULL) {
		return -1;
	}
	size_t output_len = strlen(concat_buf_sha1_b64);
	if (output_len + 1 > output_buf_len) {
		free(concat_buf_sha1_b64);
		return -1;
	}

	memcpy(output, concat_buf_sha1_b64, output_len + 1);
	if (output_str_len != NULL) {
		*output_str_len = (uint16_t) output_len;
	}

	free(concat_buf_sha1_b64);
	return 0;
}
