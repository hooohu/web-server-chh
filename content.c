/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* 10 MB is max size */
#define MAX_CONTENT_SZ (1024*1024*10)

char *
error_resp(char *path, int *len)
{
	const char eresponse[] = "<html><head><title>ERROR</title></head><body><font face=\"sans-serif\"><center><h1>X-P</h1><p>Could not find content at <b>%s</b>.</p></font></center></body>";
	char *resp;
	int sz = strlen(eresponse) + strlen(path);
	
	resp = malloc(sz);
	if (!resp) return NULL;
	*len = sprintf(resp, eresponse, path);
	return resp;
}

int
sanity_check(char *path)
{ return (path[0] == '.' || path[0] == '/'); }

char *
content_get(char *path, int *content_len)
{
	char *resp;
	int content_fd, amnt_read = 0;
	struct stat s;

#ifdef THINK_TIME
	sleep(1);
#endif

	/* Bad path?  No file?  Too large? */
	if (sanity_check(path) || 
	    stat(path, &s)     ||
	    s.st_size > MAX_CONTENT_SZ) goto err;

	content_fd = open(path, O_RDONLY);
	if (content_fd < 0) goto err;

	resp = malloc(s.st_size);
	if (!resp) goto err_close;

	while (amnt_read < s.st_size) {
		int ret = read(content_fd, resp + amnt_read, 
			       s.st_size - amnt_read);

		if (ret < 0) goto err_free;
		amnt_read += ret;
	}
	*content_len = s.st_size;

	return resp;
err_free:
	free(resp);
err_close:
	close(content_fd);
err:
	return error_resp(path, content_len);
}
