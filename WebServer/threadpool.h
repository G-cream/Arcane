/*
 *This part mainly refers following books:
 *1.Linux High-performance Server Programming
 *2.UNIX Network Programming
 *3.TCP/IP Illustrated
 */
#ifndef _POOL_
#define _POOL_

#include <limits.h>
#include "connection.h"
#include "config.h"
#include "threadsyn.h"

struct connqueue {
	struct httpconnection **list;
	int length;
	int used;
};

bool grow_connqueue(struct connqueue *);
bool init_connqueue(struct connqueue *);
bool push_back(struct connqueue *, struct httpconnection *);
bool pop_front(struct connqueue *);
struct httpconnection *get_front(struct connqueue *);
void destroy_connqueue(struct connqueue *);

struct threadpool {
	int threadnumber;
	int maxconns;
	pthread_t *threads;
	struct connqueue workqueue;
	struct locker queuelocker;
	struct sem queuestat;
	bool stop;
};

bool init_threadpool(struct threadpool *, int, int);
bool append_threadpool(struct threadpool *, struct httpconnection *);
void *worker(void *);
void destroy_threadpool(struct threadpool *);

#endif // !_POOL_