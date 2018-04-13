/*
 * file_io.h
 *
 *  Created on: 16 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_FILE_IO_H_
#define SRC_FILE_IO_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/bio.h>

int set_fd_position(int fd, size_t offset);

int send_file_to_socket(int file_fd, BIO* socket_bio, size_t length);

#endif /* SRC_FILE_IO_H_ */
