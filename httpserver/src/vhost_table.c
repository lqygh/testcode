/*
 * vhost_table.c
 *
 *  Created on: 18 Oct 2017
 *      Author: lqy
 */

#include "vhost_table.h"

struct vhost_table* vhost_table_new(const char* vhost_file_path) {
	struct vhost_table* vt = calloc(1, sizeof(struct vhost_table));
	if (vt == NULL) {
		return NULL;
	}

	vt->vti = NULL;

	FILE* fp = fopen(vhost_file_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "vhost_table_new(): failed to open %s\n",
				vhost_file_path);

		free(vt);
		return NULL;
	}

	char buffer[HTTP_REQUEST_MAX_HOST_SIZE + 1 + HTTP_REQUEST_MAX_URI_SIZE + 1] =
			{ 0 };

	int error = 0;
	while (1) {
		char* line = fgets(buffer, sizeof(buffer), fp);
		if (line == NULL) {
			break;
		}

		size_t line_len = strlen(line);
		if (line_len < 4) {
			//skip this line if too short
			fprintf(stderr, "vhost_table_new(): line too short in %s\n",
					vhost_file_path);
			continue;
		}

		if (buffer[line_len - 1] != '\n') {
			fprintf(stderr, "vhost_table_new(): line too long in %s\n",
					vhost_file_path);
			error = 1;
			break;
		}

		//remove /r and /n from the line
		if (line_len >= 1 && line[line_len - 1] == '\n') {
			line[line_len - 1] = '\0';
		}
		if (line_len >= 2 && line[line_len - 2] == '\r') {
			line[line_len - 2] = '\0';
		}

		line_len = strlen(line);
		if (line_len < 3) {
			//skip this line if too short
			fprintf(stderr, "vhost_table_new(): line too short in %s\n",
					vhost_file_path);
			continue;
		}

		//process line
		char* host = NULL;
		char* path = NULL;

		{
			char state = 0;
			size_t host_i = 0;
			size_t path_i = 0;
			size_t space_i = 0;
			for (size_t i = 0; i < line_len; i++) {
				char c = line[i];
				if (state == 0) {
					if (c == ' ') {
						state = 0;
					} else {
						host_i = i;
						state = 1;
					}
				} else if (state == 1) {
					if (c == ' ') {
						space_i = i;
						state = 2;
					} else {
						state = 1;
					}
				} else if (state == 2) {
					if (c == ' ') {
						state = 2;
					} else {
						path_i = i;
						state = 3;
					}
				} else if (state == 3) {
					break;
				}
			}

			if (state != 3) {
				fprintf(stderr,
						"vhost_table_new(): line rejected due to invalid format in %s\n",
						vhost_file_path);
				continue;
			}

			line[space_i] = '\0';
			host = line + host_i;
			path = line + path_i;
		}

		size_t host_len = strlen(host);
		if (host_len + 1 > HTTP_REQUEST_MAX_HOST_SIZE) {
			continue;
		}

		size_t path_len = strlen(path);
		if (path_len + 1 > HTTP_REQUEST_MAX_URI_SIZE) {
			continue;
		}

		//fprintf(stderr, "vhost_table_new(): Host: \"%s\" ->  Path: \"%s\"\n",
		//		host, path);

		//add to table
		{
			struct vhost_table_item* item = NULL;

			HASH_FIND_STR(vt->vti, host, item);
			if (item != NULL) {
				//if key exists, update value
				//fprintf(stderr,
				//		"vhost_table_new(): Key exists, updating value\n");
				strncpy(item->path, path, path_len);
			} else {
				//if key does not exist, add to table
				//fprintf(stderr,
				//		"vhost_table_new(): Key does not exist, adding item\n");
				struct vhost_table_item* new_item = calloc(1,
						sizeof(struct vhost_table_item));
				if (new_item == NULL) {
					continue;
				}
				strncpy(new_item->host, host, host_len);
				strncpy(new_item->path, path, path_len);
				HASH_ADD_STR(vt->vti, host, new_item);
			}
		}
	}

	if (error) {
		vhost_table_destroy(vt);
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	return vt;
}

const char* vhost_table_find(struct vhost_table* vhost_table, char* host) {
	if (vhost_table == NULL || host == NULL) {
		return NULL;
	}

	struct vhost_table_item* item = NULL;

	HASH_FIND_STR(vhost_table->vti, host, item);
	if (item != NULL) {
		//fprintf(stderr, "vhost_table_find(): Key %s found, value: %s\n",
		//		item->host, item->path);
		return item->path;
	} else {
		//fprintf(stderr, "vhost_table_find(): Key %s not found\n", host);
	}

	return NULL;
}

const char* vhost_table_find_ignore_port(struct vhost_table* vhost_table,
		char* host) {
	if (vhost_table == NULL || host == NULL) {
		return NULL;
	}

	const char* path = vhost_table_find(vhost_table, host);
	if (path != NULL) {
		return path;
	}

	char* r = strstr(host, ":");
	if (r == NULL) {
		return NULL;
	}

	char new_host[HTTP_REQUEST_MAX_HOST_SIZE] = { 0 };
	strncpy(new_host, host, sizeof(new_host));
	r = strstr(new_host, ":");
	if (r == NULL) {
		return NULL;
	}
	*r = '\0';

	path = vhost_table_find(vhost_table, new_host);

	return path;
}

void vhost_table_destroy(struct vhost_table* vhost_table) {
	if (vhost_table == NULL) {
		return;
	}

	struct vhost_table_item* item = NULL;
	struct vhost_table_item* tmp = NULL;
	HASH_ITER(hh, vhost_table->vti, item, tmp)
	{
		//fprintf(stderr, "\nDeleting item %s : %s\n", item->host, item->path);
		HASH_DEL(vhost_table->vti, item);
		free(item);
	}

	free(vhost_table);
}
