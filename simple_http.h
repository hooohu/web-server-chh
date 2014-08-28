/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#ifndef SIMPLE_HTTP_H
#define SIMPLE_HTTP_H

struct http_req {
	int   fd;

	/* Request information */
	char *request;
	int   req_len;
	char *path; 		/* points to string inside of request */

	/* Response information */
	char *resp_head, *response;
	int   resp_hd_len, resp_len;
};


/* 
 * Allocate a new http_req for the file descriptor, and with the
 * specific request.
 */
struct http_req *shttp_alloc_req(int fd, char *request);

/* 
 * Will free the memory for the request, response, and will close the
 * file descriptor.
 */
void shttp_free_req(struct http_req *r);

/* populate the ->path filed in http_req */
int shttp_get_path(struct http_req *r);

/* 
 * Take the answer, which is the response (of length len) to the
 * request with the given path, and formulate the response to be
 * written out to the client in ->response.  Note that answer will not
 * be freed by this function, and must be freed by the caller.
 */
int shttp_alloc_response_head(struct http_req *r, char *resp, int rlen);

#endif
