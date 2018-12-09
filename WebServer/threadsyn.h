/*
 *This part mainly refers following books:
 *1.Linux High-performance Server Programming
 *2.UNIX Network Programming
 *3.TCP/IP Illustrated
 */
#ifndef _SYN_
#define _SYN_

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>

struct sem {
	sem_t threadsem;
};

bool init_sem(struct sem *);
bool wait_sem(struct sem *);
bool post_sem(struct sem *);
void destroy_sem(struct sem *);

struct locker {
	pthread_mutex_t threadmutex;
};

bool init_locker(struct locker *);
bool lock(struct locker *);
bool unlock(struct locker *);
void destroy_locker(struct locker *);

struct cond {
	pthread_mutex_t threadmutex;
	pthread_cond_t threadcond;
};

bool init_cond(struct cond *);
bool signal_cond(struct cond *);
bool wait_cond(struct cond *);
void destroy_cond(struct cond *);

#endif // !_SYN_
