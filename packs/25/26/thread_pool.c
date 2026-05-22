#include "thread_pool.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct {
    task_func_t function;
    void *arg;
    task_func_t completion;
    void *completion_arg;
} pool_task_t;

typedef struct {
    queue_t *queue;
    queue_t *dispatcher_queue;
} worker_t;

struct thread_pool {
    size_t thread_count;
    size_t next_worker;
    pthread_t *threads;
    worker_t *workers;
    queue_t *dispatcher_queue;
};

static void *worker_main(void *arg) {
    worker_t *worker = (worker_t *)arg;
    task_t item;

    while (queue_pop(worker->queue, &item)) {
        pool_task_t *task = (pool_task_t *)item.arg;
        if (task != NULL && task->function != NULL) {
            task->function(task->arg);
        }
        if (task != NULL && task->completion != NULL &&
            worker->dispatcher_queue != NULL) {
            task_t completion = {task->completion, task->completion_arg};
            queue_push(worker->dispatcher_queue, completion);
        }
        free(task);
    }

    return NULL;
}

thread_pool_t *thread_pool_create(size_t thread_count,
                                  size_t per_worker_queue_capacity,
                                  queue_t *dispatcher_queue) {
    if (thread_count == 0) {
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
    if (pool == NULL) {
        return NULL;
    }
    pool->thread_count = thread_count;
    pool->dispatcher_queue = dispatcher_queue;
    pool->threads = (pthread_t *)calloc(thread_count, sizeof(pthread_t));
    pool->workers = (worker_t *)calloc(thread_count, sizeof(worker_t));
    if (pool->threads == NULL || pool->workers == NULL) {
        thread_pool_destroy(pool);
        return NULL;
    }

    for (size_t i = 0; i < thread_count; ++i) {
        pool->workers[i].queue = queue_create(per_worker_queue_capacity);
        pool->workers[i].dispatcher_queue = dispatcher_queue;
        if (pool->workers[i].queue == NULL ||
            pthread_create(&pool->threads[i], NULL, worker_main,
                           &pool->workers[i]) != 0) {
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

int thread_pool_init(thread_pool_t **pool,
                     size_t thread_count,
                     size_t per_worker_queue_capacity,
                     queue_t *dispatcher_queue) {
    if (pool == NULL) {
        return 0;
    }
    *pool = thread_pool_create(thread_count, per_worker_queue_capacity,
                               dispatcher_queue);
    return *pool != NULL;
}

void thread_pool_shutdown(thread_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

    for (size_t i = 0; i < pool->thread_count; ++i) {
        queue_close(pool->workers[i].queue);
    }
    for (size_t i = 0; i < pool->thread_count; ++i) {
        if (pool->threads[i]) {
            pthread_join(pool->threads[i], NULL);
            pool->threads[i] = 0;
        }
    }
}

void thread_pool_destroy(thread_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

    thread_pool_shutdown(pool);
    for (size_t i = 0; i < pool->thread_count; ++i) {
        queue_destroy(pool->workers[i].queue);
    }
    free(pool->threads);
    free(pool->workers);
    free(pool);
}

static int submit_to_queue(queue_t *queue,
                           task_func_t function,
                           void *arg,
                           task_func_t completion,
                           void *completion_arg) {
    pool_task_t *task = (pool_task_t *)calloc(1, sizeof(pool_task_t));
    if (task == NULL) {
        return 0;
    }
    task->function = function;
    task->arg = arg;
    task->completion = completion;
    task->completion_arg = completion_arg;

    task_t item = {NULL, task};
    if (!queue_push(queue, item)) {
        free(task);
        return 0;
    }
    return 1;
}

int thread_pool_submit(thread_pool_t *pool,
                       size_t worker_index,
                       task_func_t function,
                       void *arg,
                       task_func_t completion,
                       void *completion_arg) {
    if (pool == NULL || function == NULL || worker_index >= pool->thread_count) {
        return 0;
    }
    return submit_to_queue(pool->workers[worker_index].queue, function, arg,
                           completion, completion_arg);
}

int thread_pool_submit_rr(thread_pool_t *pool,
                          task_func_t function,
                          void *arg,
                          task_func_t completion,
                          void *completion_arg) {
    if (pool == NULL || pool->thread_count == 0) {
        return 0;
    }
    size_t worker = pool->next_worker++ % pool->thread_count;
    return thread_pool_submit(pool, worker, function, arg, completion,
                              completion_arg);
}

size_t thread_pool_thread_count(const thread_pool_t *pool) {
    return pool == NULL ? 0 : pool->thread_count;
}

queue_t *thread_pool_worker_queue(thread_pool_t *pool, size_t worker_index) {
    if (pool == NULL || worker_index >= pool->thread_count) {
        return NULL;
    }
    return pool->workers[worker_index].queue;
}
