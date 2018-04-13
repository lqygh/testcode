/*
 * file.c
 *
 *  Created on: 16 Oct 2017
 *      Author: lqy
 */

#include "file_io.h"

int set_fd_position(int fd, size_t offset) {
	off_t ret = lseek(fd, (off_t) offset, SEEK_SET);
	if (ret < 0 || offset != (size_t) ret) {
		return -1;
	}

	return 0;
}

int send_file_to_socket(int file_fd, BIO* socket_bio, size_t length) {
	ssize_t slength = (ssize_t) length;
	ssize_t bytes_written = 0;
	char buffer[1024 * 8] = { 0 };
	while (bytes_written < slength) {
		ssize_t rret = read(file_fd, buffer, sizeof(buffer));
		if (rret < 0) {
			//read error
			//printf("read() error\n");
			return -1;
		} else if (rret == 0) {
			//EOF
			if (bytes_written < slength) {
				//error
				//printf("early file EOF\n");
				return -1;
			} else {
				//normal EOF
				return 0;
			}
		}

		if (rret + bytes_written > slength) {
			rret = slength - bytes_written;
		}

		ssize_t bytes_sent = 0;
		while (bytes_sent < rret) {
			ssize_t sret = BIO_write(socket_bio, buffer + bytes_sent,
					rret - bytes_sent);
			if (sret < 0) {
				fprintf(stderr, "send_file_to_socket() failed\n");
				return -1;
			}
			bytes_sent += sret;
		}
		bytes_written += rret;
	}
	return 0;
}
