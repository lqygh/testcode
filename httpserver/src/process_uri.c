/*
 * process_uri.c
 *
 *  Created on: 15 Oct 2017
 *      Author: lqy
 */

#include "process_uri.h"

int canonicalise_path(const char* uri_in, char* uri_out,
		const size_t uri_out_size) {
	if (uri_in == NULL || uri_out == NULL || uri_out_size < 1) {
		return -1;
	}

	char* rp = realpath(uri_in, NULL);
	if (rp == NULL) {
		return -1;
	}
	size_t rp_len = strlen(rp);
	if (rp_len > uri_out_size - 1) {
		free(rp);
		return -1;
	}

	strncpy(uri_out, rp, rp_len + 1);

	uri_out[rp_len] = '\0';
	uri_out[uri_out_size - 1] = '\0';

	free(rp);

	return 0;
}

int append_string(char* to, const size_t to_buffer_size, const char* suffix) {
	if (to == NULL || to_buffer_size < 1 || suffix == NULL) {
		return -1;
	}

	size_t to_orig_len = strlen(to);
	size_t suffix_len = strlen(suffix);

	if (to_orig_len + suffix_len + 1 > to_buffer_size) {
		return -1;
	}

	char* to_orig_copy = strdup(to);
	if (to_orig_copy == NULL) {
		return -1;
	}

	snprintf(to, to_orig_len + suffix_len + 1, "%s%s", to_orig_copy, suffix);

	free(to_orig_copy);

	return 0;
}

int prepend_string(char* to, const size_t to_buffer_size, const char* prefix) {
	if (to == NULL || to_buffer_size < 1 || prefix == NULL) {
		return -1;
	}

	size_t to_orig_len = strlen(to);
	size_t prefix_len = strlen(prefix);

	if (to_orig_len + prefix_len + 1 > to_buffer_size) {
		return -1;
	}

	char* to_orig_copy = strdup(to);
	if (to_orig_copy == NULL) {
		return -1;
	}

	snprintf(to, to_orig_len + prefix_len + 1, "%s%s", prefix, to_orig_copy);

	free(to_orig_copy);

	return 0;
}

int decode_percent_encoding(const char* uri_in, char* uri_out,
		const size_t uri_out_size) {
	if (uri_in == NULL || uri_out == NULL || uri_out_size < 1) {
		return -1;
	}

	size_t uri_in_len = strlen(uri_in);
	char* uri_decoded_tmp = calloc(uri_in_len + 1, 1);
	if (uri_decoded_tmp == NULL) {
		return -1;
	}
	size_t uri_decoded_len = 0;
	for (size_t i = 0; i < uri_in_len;) {
		if (i + 2 < uri_in_len && (uri_in[i] == '%')
				&& (isxdigit(uri_in[i + 1])) && (isxdigit(uri_in[i + 2]))) {
			char hex[3] = { uri_in[i + 1], uri_in[i + 2], '\0' };
			char character = (char) strtol(hex, NULL, 16);
			uri_decoded_tmp[uri_decoded_len] = character;
			i += 3;
		} else {
			uri_decoded_tmp[uri_decoded_len] = uri_in[i];
			i += 1;
		}
		uri_decoded_len += 1;
	}
	uri_decoded_tmp[uri_decoded_len] = '\0';

	if (uri_decoded_len + 1 > uri_out_size) {
		free(uri_decoded_tmp);
		return -1;
	}

	strncpy(uri_out, uri_decoded_tmp, uri_decoded_len + 1);
	free(uri_decoded_tmp);

	return 0;
}

int string_has_prefix(const char* string, const char* prefix) {
	if (string == NULL || prefix == NULL) {
		return 0;
	}

	size_t string_len = strlen(string);
	size_t prefix_len = strlen(prefix);

	if (string_len < prefix_len) {
		return 0;
	}

	char* pos = strstr(string, prefix);
	if (pos != string) {
		return 0;
	}

	return 1;
}

int process_uri(const char* vhost_path, const char* uri_in, char* path_out,
		const size_t path_out_size) {
	if (vhost_path == NULL || uri_in == NULL || path_out == NULL
			|| path_out_size < 1) {
		return -1;
	}

	//canonicalise vhost path
	char* vhost_path_canonical = realpath(vhost_path, NULL);
	if (vhost_path_canonical == NULL) {
		return -1;
	}
	//printf("vhost_path_canonical: %s\n", vhost_path_canonical);

	//prepend vhost_path_canonical to URI
	int len = snprintf(NULL, 0, "%s/%s", vhost_path_canonical, uri_in);
	char* uri = calloc(len + 1, 1);
	if (uri == NULL) {
		free(vhost_path_canonical);
		return -1;
	}
	snprintf(uri, len + 1, "%s/%s", vhost_path_canonical, uri_in);
	//printf("uri: %s\n", uri);

	//decode percent encoding
	char* uri_decoded = calloc(len + 1, 1);
	if (uri_decoded == NULL) {
		free(vhost_path_canonical);
		free(uri);
		return -1;
	}
	int ret = decode_percent_encoding(uri, uri_decoded, len + 1);
	if (ret < 0) {
		free(vhost_path_canonical);
		free(uri);
		free(uri_decoded);
		return -1;
	}
	//printf("uri_decoded: %s\n", uri_decoded);

	//canonicalise URI
	ret = canonicalise_path(uri_decoded, path_out, path_out_size);
	if (ret < 0) {
		free(vhost_path_canonical);
		free(uri);
		free(uri_decoded);
		return -1;
	}
	//printf("path_out: %s\n", path_out);

	//check URI prefix
	ret = string_has_prefix(path_out, vhost_path_canonical);
	if (!ret) {
		free(vhost_path_canonical);
		free(uri);
		free(uri_decoded);
		//printf("prefix check failed\n");
		return -1;
	}

	free(vhost_path_canonical);
	free(uri);
	free(uri_decoded);
	return 0;
}
