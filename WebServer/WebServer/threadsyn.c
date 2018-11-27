/*
 *This part mainly references following books:
 *1.Linux High-performance Server Programming
 *2.UNIX Network Programming
 *3.TCP/IP Illustrated
 */
#include "threadsyn.h"

void
init_sem(struct sem *s)
{
	if (sem_init(&s->threadsem, 0, 0) != 0)
		return;
}

bool
wait_sem(struct sem *s)
{
	return sem_wait(&s->threadsem) == 0;
}

bool 
post_sem(struct sem *s)
{
	return sem_post(&s->threadsem) == 0;
}

void
destroy_sem(struct sem *s)
{
	sem_destroy(&s->threadsem);
}

void 
init_locker(struct locker *l)
{
	if (pthread_mutex_init(&l->threadmutex, NULL) != 0)
		return;
}

bool 
lock(struct locker *l)
{
	return pthread_mutex_lock(&l->threadmutex) == 0;
}

bool 
unlock(struct locker *l)
{
	return pthread_mutex_unlock(&l->threadmutex) == 0;
}

void 
destroy_locker(struct locker *l)
{
	pthread_mutex_destroy(&l->threadmutex);
}

void 
init_cond(struct cond *c)
{
	if (pthread_mutex_init(&c->threadmutex, NULL) != 0)
		return;
	if (pthread_cond_init(&c->threadcond, NULL) != 0)
		return;
}

bool 
signal_cond(struct cond *c)
{
	return pthread_cond_signal(&c->threadcond) == 0;
}

bool 
wait_cond(struct cond *c)
{
	int ret = 0;
	pthread_mutex_lock(&c->threadmutex);
	ret = pthread_cond_wait(&c->threadcond, &c->threadmutex);
	pthread_mutex_unlock(&c->threadmutex);
	return ret == 0;
}

void 
destroy_cond(struct cond *c)
{
	pthread_mutex_destroy(&c->threadmutex);
	pthread_cond_destroy(&c->threadcond);
}