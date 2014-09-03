/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */
/* 
 * This is a HTTP server.  It accepts connections on port 8080, and
 * serves a local static document.
 *
 * The clients you can use are 
 * - httperf (e.g., httperf --port=8080),
 * - wget (e.g. wget localhost:8080 /), 
 * - or even your browser.  
 *
 * To measure the efficiency and concurrency of your server, use
 * httperf and explore its options using the manual pages (man
 * httperf) to see the maximum number of connections per second you
 * can maintain over, for example, a 10 second period.
 *
 * Example usage:
 * # make test1
 * # make test2
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>

#include <util.h> 		/* client_process */
#include <server.h>		/* server_accept and server_create */

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 16

/* 
 * This is the function for handling a _single_ request.  Understand
 * what each of the steps in this function do, so that you can handle
 * _multiple_ requests.  Use this function as an _example_ of the
 * basic functionality.  As you increase the server in functionality,
 * you will want to probably keep all of the functions called in this
 * function, but define different code to use them.
 */
void server_single_request(int accept_fd) {
  int fd;
  struct last_data temp;

  temp.last_path = NULL;
  temp.last_response = NULL;
  temp.resp_len = 0;
  /* 
   * The server thread will always want to be doing the accept.
   * That main thread will want to hand off the new fd to the
   * new threads/processes/thread pool.
   */
  fd = server_accept(accept_fd);
  client_process(fd, &temp);

  /* 
   * A loop around these two lines will result in multiple
   * documents being served.
   */

  return;
}

/*
 * This is a data struct used for para in simple thread part
 * It contains a fd flag
 * And a last_data cache file pointer
 */
struct simple_data {
  int simple_fd;
  struct last_data *simple_cache;
};

/* 
 * Simple Thread Used to handle each request
 * Added in 8/28/2014 by chh 
 */
void simple_thread_handle(void *ptr) {
  struct simple_data *data = (struct simple_data *)ptr;

  //printf("new thread forked! id: %d\n", new_fd);
  client_process(data->simple_fd, data->simple_cache);
  free(ptr);
  return;
}

/* 
 * This implementation uses a single master thread which then spawns a
 * new thread to handle each incoming requst.  Each of these worker
 * threads should process a single request and then terminate.
 */
void server_simple_thread(int accept_fd) {
  int fd;
  struct last_data cache_data;

  pthread_mutex_init(&(cache_data.last_lock), NULL);
  cache_data.last_response = NULL;
  cache_data.last_path = NULL;
  cache_data.resp_len = 0;

  while (1) {
    fd = server_accept(accept_fd);
    if (fd == 0) return;
    pthread_t new_thread;
    struct simple_data *para = (struct simple_data *)malloc(sizeof(struct simple_data));
    para->simple_fd = fd;
    para->simple_cache = &(cache_data);
    int iret = pthread_create(&new_thread, NULL, (void *)&simple_thread_handle, (void *)para);
    if (iret) return;
  }
  return;
}

/*
 * Struct for the list.
 */
struct pool_node {
  int pool_fd;
  struct pool_node *next;
};

/*
 *This data structure is used in pthread_creat for multiply parameters of thread function.
 */
struct pool_data {
  struct pool_node *head;
  pthread_mutex_t share;
  struct last_data pool_cache;
};

/*
 * This is the thread that handle the List Data Structure
 * The main fork handle all the requests
 * This fork read fd from the List and manage and initial mutex array
 * Then it will generate handle threads with mutex
 */
void pool_thread_handle(void *ptr) {
  struct pool_data *pass = (struct pool_data *)ptr;  
  int the_fd;

  while(1) {
    pthread_mutex_lock(&(pass->share));

    if (pass->head != NULL) {
      the_fd = pass->head->pool_fd;
      struct pool_node *temp = pass->head;
      pass->head = pass->head->next;
      free(temp);
    }
    else the_fd = -1;

    pthread_mutex_unlock(&(pass->share));
    
    if (the_fd != -1) client_process(the_fd, &(pass->pool_cache));

    //sleep(1);
  }
}

/* 
 * The following implementation uses a thread pool.  This collection
 * of threads is of maximum size MAX_CONCURRENCY, and is created by
 * pthread_create.  These threads retrieve data from a shared
 * data-structure with the main thread.  The synchronization around
 * this shared data-structure is done using mutexes + condition
 * variables (for a bounded structure).
 */
void server_thread_pool_bounded(int accept_fd) {
  pthread_t threads[MAX_CONCURRENCY];
  int fd;
  struct pool_node *tail = NULL;
  struct pool_data pool_pass;
  int i;
  
  pool_pass.head = NULL;
//  pool_pass.share = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_init(&(pool_pass.share), NULL);
  pthread_mutex_init(&(pool_pass.pool_cache.last_lock), NULL);  
  pool_pass.pool_cache.last_path = NULL;
  pool_pass.pool_cache.last_response = NULL;
  pool_pass.pool_cache.resp_len = 0;

  pthread_mutex_lock(&(pool_pass.share));

  for (i = 0; i < MAX_CONCURRENCY; i++) {
    pthread_create(&(threads[i]), NULL, (void *)&pool_thread_handle, (void *)&pool_pass);
  }

  pthread_mutex_unlock(&(pool_pass.share));

  while(1) {
    fd = server_accept(accept_fd);
    if (fd == 0) return;
    struct pool_node *new_node = (struct pool_node *)malloc(sizeof(struct pool_node));
    new_node->pool_fd = fd;
    new_node->next = NULL;
    if (pool_pass.head == NULL) pool_pass.head = tail = new_node;
    else {
      tail->next = new_node;
      tail = new_node;
    }
  }
 
  return;
}

typedef enum {
  SERVER_TYPE_ONE = 0,
  SERVER_TYPE_SIMPLE_THREAD,
  SERVER_TYPE_THREAD_POOL_BOUND,
} server_type_t;

int main(int argc, char *argv[]) {
  server_type_t server_type;
  short int port;
  int accept_fd;

  if (argc != 3) {
    printf("Proper usage of http server is:\n%s <port> <#>\n"
           "port is the port to serve on, # is either\n"
           "0: server only a single request\n"
           "1: use a master thread that spawns new threads for "
           "each request\n"
           "2: use a thread pool and a _bounded_ buffer with "
           "mutexes + condition variables\n",argv[0]);
    return -1;
  }

  port = atoi(argv[1]);
  accept_fd = server_create(port);
  if (accept_fd < 0) return -1;
	
  server_type = atoi(argv[2]);

  switch(server_type) {
    case SERVER_TYPE_ONE:
      server_single_request(accept_fd);
      break;
    case SERVER_TYPE_THREAD_POOL_BOUND:
      server_thread_pool_bounded(accept_fd);
      break;
    case SERVER_TYPE_SIMPLE_THREAD:
      server_simple_thread(accept_fd);
      break;
  }
  close(accept_fd);

  return 0;
}

