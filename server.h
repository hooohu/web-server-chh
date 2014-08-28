/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#ifndef SERVER_H
#define SERVER_H

int server_create(short int port);
int server_accept(int fd);

#endif
