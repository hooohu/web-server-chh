/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#ifndef UTIL_H
#define UTIL_H

struct last_data {
  char *last_path;
  char *last_response;
  int resp_len;
  pthread_mutex_t last_lock;
};

void client_process(int fd, struct last_data *para);

#endif
