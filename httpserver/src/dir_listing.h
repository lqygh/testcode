/*
 * dir_listing.h
 *
 *  Created on: 20 Oct 2017
 *      Author: lqy
 */

#ifndef SRC_DIR_LISTING_H_
#define SRC_DIR_LISTING_H_

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/bio.h>

int send_dir_listing_to_socket_chunked(DIR* dir, BIO* socket_bio);

#endif /* SRC_DIR_LISTING_H_ */
