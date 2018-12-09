/*
 *This part mainly refers following books:
 *1.Linux High-performance Server Programming
 *2.UNIX Network Programming
 *3.TCP/IP Illustrated
 */
#include "threadsyn.h"

bool
init_sem(struct sem *s)
{
	if (s == NULL)
		return false;
	if (sem_init(&s->threadsem, 0, 100) != 0)
		return false;
	return true;
}

bool
wait_sem(struct sem *s)
{
	if (s == NULL)
		return false;
	return sem_wait(&s->threadsem) == 0;
}

bool 
post_sem(struct sem *s)
{
	if (s == NULL)
		return false;
	return sem_post(&s->threadsem) == 0;
}

void
destroy_sem(struct sem *s)
{
	if (s == NULL)
		return;
	sem_destroy(&s->threadsem);
}

bool 
init_locker(struct locker *l)
{
	if (l == NULL)
		return false;
	if (pthread_mutex_init(&l->threadmutex, NULL) != 0)
		return false;
	return true;
}

bool 
lock(struct locker *l)
{
	if (l == NULL)
		return false;
	return pthread_mutex_lock(&l->threadmutex) == 0;
}

bool 
unlock(struct locker *l)
{
	if (l == NULL)
		return false;
	return pthread_mutex_unlock(&l->threadmutex) == 0;
}

void 
destroy_locker(struct locker *l)
{
	if (l == NULL)
		return;
	pthread_mutex_destroy(&l->threadmutex);
}

bool 
init_cond(struct cond *c)
{
	if (c == NULL)
		return false;
	if (pthread_mutex_init(&c->threadmutex, NULL) != 0)
		return false;
	if (pthread_cond_init(&c->threadcond, NULL) != 0)
		return false;
	return true;
}

bool 
signal_cond(struct cond *c)
{
	if (c == NULL)
		return false;
	return pthread_cond_signal(&c->threadcond) == 0;
}

bool 
wait_cond(struct cond *c)
{
	if (c == NULL)
		return false;
	int ret = 0;
	pthread_mutex_lock(&c->threadmutex);
	ret = pthread_cond_wait(&c->threadcond, &c->threadmutex);
	pthread_mutex_unlock(&c->threadmutex);
	return ret == 0;
}

void 
destroy_cond(struct cond *c)
{
	if (c == NULL)
		return;
	pthread_mutex_destroy(&c->threadmutex);
	pthread_cond_destroy(&c->threadcond);
}
