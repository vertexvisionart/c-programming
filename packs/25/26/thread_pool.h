#ifndef P26_THREAD_POOL_H
#define P26_THREAD_POOL_H

#include <stddef.h>

#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread_pool thread_pool_t;

thread_pool_t *thread_pool_create(size_t thread_count,
                                  size_t per_worker_queue_capacity,
                                  queue_t *dispatcher_queue);
int thread_pool_init(thread_pool_t **pool,
                     size_t thread_count,
                     size_t per_worker_queue_capacity,
                     queue_t *dispatcher_queue);
void thread_pool_shutdown(thread_pool_t *pool);
void thread_pool_destroy(thread_pool_t *pool);

int thread_pool_submit(thread_pool_t *pool,
                       size_t worker_index,
                       task_func_t function,
                       void *arg,
                       task_func_t completion,
                       void *completion_arg);
int thread_pool_submit_rr(thread_pool_t *pool,
                          task_func_t function,
                          void *arg,
                          task_func_t completion,
                          void *completion_arg);

size_t thread_pool_thread_count(const thread_pool_t *pool);
queue_t *thread_pool_worker_queue(thread_pool_t *pool, size_t worker_index);

#ifdef __cplusplus
}
#endif

#endif
