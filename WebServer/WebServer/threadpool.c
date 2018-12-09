/*
 *This part mainly refers following books:
 *1.Linux High-performance Server Programming
 *2.UNIX Network Programming
 *3.TCP/IP Illustrated
 */
#include "threadpool.h"

bool 
grow_connqueue(struct connqueue *queue)
{
	if (queue == NULL)
		return false;
	int multipler;
	multipler = 2;
	if (queue->used >= queue->length) {
		if (safe_realloc(&queue->list, multipler * queue->length, sizeof(struct httpconnection *)) != 0)
			return false;
		queue->length = multipler * queue->length;
	}	
	return true;
}

bool 
init_connqueue(struct connqueue *queue)
{
	if (queue == NULL)
		return false;
	int initialsize;
	initialsize = 10;
	if (safe_realloc(&queue->list, initialsize, sizeof(struct httpconnection *)) != 0)
		return false;
	queue->length = initialsize;
	queue->used = 0;
	return true;
}

bool
push_back(struct connqueue *queue, struct httpconnection * pconn)
{
	if (queue == NULL)
		return false;
	queue->list[queue->used] = pconn;
	++queue->used;
	if (!grow_connqueue(queue))
		return false;
	return true;
}

bool
pop_front(struct connqueue *queue)
{
	if (queue == NULL)
		return false;
	if (queue->used == 0)
		return true;
	if (queue->used == 1) {
		queue->list[queue->used] = NULL;
		--queue->used;
		return true;
	}
	for (int n = 0; n != queue->used - 1; ++n)
		queue->list[n] = queue->list[n + 1];
	--queue->used;
	return true;
}

struct httpconnection * 
get_front(struct connqueue *queue)
{
	if (queue == NULL)
		return NULL;
	if (queue->used == 0)
		return NULL;
	return queue->list[0];
}

void 
destroy_connqueue(struct connqueue *queue)
{ 
	free(queue->list);
}

bool 
init_threadpool(struct threadpool *pool, int threadnumber, int maxconns)
{
	if (pool == NULL)
		return false;
	if (threadnumber <= 0 || maxconns <= 0)
		return false;
	(void)memset(pool, 0, sizeof(struct threadpool));
	if (!init_sem(&pool->queuestat))
		return false;
	if (!init_locker(&pool->queuelocker))
		return false;
	if (!init_connqueue(&pool->workqueue))
		return false;
	if (safe_realloc(&pool->threads, threadnumber, sizeof(pthread_t)) != 0)
		return false;
	for (int n = 0; n != threadnumber; ++n) {
		if (pthread_create(&pool->threads[n], NULL, worker, pool) != 0) {
			free(pool->threads);
			return false;
		}
		if (pthread_detach(pool->threads[n]) != 0) {
			free(pool->threads);
			return false;
		}			
	}
	pool->threadnumber = threadnumber;
	pool->maxconns = maxconns;
	pool->stop = false;
	return true;
}

bool 
append_threadpool(struct threadpool *pool, struct httpconnection *conn)
{
	(void)lock(&pool->queuelocker);
	if (pool->workqueue.used == pool->maxconns) {
		(void)unlock(&pool->queuelocker);
		return false;
	}
	if (!push_back(&pool->workqueue, conn)) {
		(void)unlock(&pool->queuelocker);
		return false;
	}
	(void)unlock(&pool->queuelocker);
	if (post_sem(&pool->queuestat) == 0) {
		return false;
	}
	return true;
}

void * 
worker(void *arg)
{
	struct threadpool* pool = (struct threadpool *)arg;
	while (!pool->stop)	{
		struct httpconnection *conn;
		(void)wait_sem(&pool->queuestat);
		(void)lock(&pool->queuelocker);
		if (pool->workqueue.used == 0) {
			(void)unlock(&pool->queuelocker);
			continue;
		}
		conn = get_front(&pool->workqueue);
		if (!pop_front(&pool->workqueue)) {
			(void)unlock(&pool->queuelocker);
			continue;
		}
		(void)unlock(&pool->queuelocker);
		if (conn != NULL) {
			if(!process(conn))
				close_httpconnection(conn);
		}		
	}
	return pool;
}

void 
destroy_threadpool(struct threadpool *pool)
{
	pool->stop = true;
	destroy_connqueue(&pool->workqueue);
	destroy_sem(&pool->queuestat);
	destroy_locker(&pool->queuelocker);
	free(pool->threads);
}
