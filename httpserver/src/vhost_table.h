/*
 * vhost_table.h
 *
 *  Created on: 18 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_VHOST_TABLE_H_
#define SRC_VHOST_TABLE_H_

#include "uthash/uthash.h"
#include "process_request.h"

struct vhost_table {
	struct vhost_table_item* vti;
};

struct vhost_table_item {
	char host[HTTP_REQUEST_MAX_HOST_SIZE];
	char path[HTTP_REQUEST_MAX_URI_SIZE];
	UT_hash_handle hh;
};

struct vhost_table* vhost_table_new(const char* vhost_file_path);

const char* vhost_table_find(struct vhost_table* vhost_table, char* host);

const char* vhost_table_find_ignore_port(struct vhost_table* vhost_table,
		char* host);

void vhost_table_destroy(struct vhost_table* vhost_table);

#endif /* SRC_VHOST_TABLE_H_ */
