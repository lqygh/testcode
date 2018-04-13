/*
 * dir_listing.c
 *
 *  Created on: 19 Oct 2017
 *      Author: lqy
 */

#include "dir_listing.h"

static int is_unreserved_character(char c) {
	if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
		return 1;
	} else {
		return 0;
	}
}

static int encode_percent_encoding(const char* in, char* out,
		const size_t out_buffer_size) {
	if (in == NULL || out == NULL || out_buffer_size < 1) {
		return -1;
	}

	size_t in_len = strlen(in);
	size_t out_char_count = 0;
	for (size_t i = 0; i < in_len; i++) {
		char c = in[i];

		if (is_unreserved_character(c)) {
			if (out_char_count + 1 > out_buffer_size - 1) {
				return -1;
			}

			out[out_char_count] = c;

			out_char_count += 1;
		} else {
			if (out_char_count + 3 > out_buffer_size - 1) {
				return -1;
			}

			char hex[3] = { 0 };
			snprintf(hex, sizeof(hex), "%02x", c);

			out[out_char_count] = '%';
			out[out_char_count + 1] = hex[0];
			out[out_char_count + 2] = hex[1];

			out_char_count += 3;
		}

	}
	out[out_char_count] = '\0';

	return 0;
}

static int my_bio_write_full(BIO* socket_bio, void* buf, size_t count) {
	if (count <= 0) {
		return count;
	}

	int bytes_written = 0;
	int bytes_left = count;

	while (bytes_left != 0) {
		int ret = BIO_write(socket_bio, (char*) (buf) + bytes_written,
				bytes_left);

		if (ret < 0) {
			return ret;
		} else if (ret == 0) {
			return bytes_written;
		} else {
			bytes_written += ret;
			bytes_left -= ret;
		}
	}

	return bytes_written;
}

int send_dir_listing_to_socket_chunked(DIR* dir, BIO* socket_bio) {
	if (dir == NULL) {
		return -1;
	}

	int ret =
			my_bio_write_full(socket_bio,
					"58\r\n<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<title>Directory Listing</title>\n</head>\n<body>\n\r\n",
					94);
	if (ret < 0) {
		fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
		return -1;
	}

	struct dirent *entry = NULL;
	while ((entry = readdir(dir))) {
		char size_string_hex[32] = { 0 };
		char percent_encoded_filename[2048] = { 0 };
		char buffer[2048] = { 0 };

		int ret = encode_percent_encoding(entry->d_name,
				percent_encoded_filename, sizeof(percent_encoded_filename));
		if (ret < 0) {
			fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
			return -1;
		}

		snprintf(buffer, sizeof(buffer), "<a href=\"%s\">%s</a><br>\n",
				percent_encoded_filename, entry->d_name);
		size_t buf_str_len = strlen(buffer);

		snprintf(size_string_hex, sizeof(size_string_hex), "%zx", buf_str_len);

		ret = my_bio_write_full(socket_bio, size_string_hex,
				strlen(size_string_hex));
		if (ret < 0) {
			fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
			return -1;
		}
		ret = my_bio_write_full(socket_bio, "\r\n", 2);
		if (ret < 0) {
			fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
			return -1;
		}
		ret = my_bio_write_full(socket_bio, buffer, buf_str_len);
		if (ret < 0) {
			fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
			return -1;
		}
		ret = my_bio_write_full(socket_bio, "\r\n", 2);
		if (ret < 0) {
			fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
			return -1;
		}
	}

	ret = my_bio_write_full(socket_bio, "f\r\n</body>\n</html>\r\n", 20);
	if (ret < 0) {
		fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
		return -1;
	}

	ret = my_bio_write_full(socket_bio, "0\r\n\r\n", 5);
	if (ret < 0) {
		fprintf(stderr, "send_dir_listing_to_socket_chunked() failed\n");
		return -1;
	}

	return 0;
}
