/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#ifndef CONTENT_H
#define CONTENT_H

/* 
 * Take the path we want to read, and return the data associated with
 * that.  content_len is set to be the length of the data that is
 * returned.  The caller must free the returned string.
 */
char *content_get(char *path, int *content_len);

#endif
