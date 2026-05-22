#include "queue.h"
#include "thread_pool.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 1024
#define PER_WORKER_LIMIT 10

typedef struct {
    unsigned char *src;
    unsigned char *dst;
    size_t offset;
    size_t len;
    size_t worker;
    int *ok;
} BlockTask;

typedef struct {
    thread_pool_t *pool;
    queue_t *done_queue;
    unsigned char *src;
    unsigned char *dst;
    size_t size;
    size_t block_count;
    size_t next_block;
    size_t completed;
    size_t threads;
    size_t *inflight;
    int *ok;
    int verify;
} Stage;

static unsigned char random_byte(uint32_t *state) {
    *state = *state * 1664525u + 1013904223u;
    return (unsigned char)(*state >> 24);
}

static unsigned char stream_byte(size_t absolute_pos) {
    uint32_t x = (uint32_t)absolute_pos ^ 0x9E3779B9u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return (unsigned char)x;
}

static void crypt_block(void *arg) {
    BlockTask *task = (BlockTask *)arg;
    for (size_t i = 0; i < task->len; ++i) {
        task->dst[task->offset + i] =
            task->src[task->offset + i] ^ stream_byte(task->offset + i);
    }
}

static void verify_block(void *arg) {
    BlockTask *task = (BlockTask *)arg;
    for (size_t i = 0; i < task->len; ++i) {
        unsigned char decoded =
            task->src[task->offset + i] ^ stream_byte(task->offset + i);
        if (decoded != task->dst[task->offset + i]) {
            *task->ok = 0;
            break;
        }
    }
}

static void completion_task(void *arg) {
    (void)arg;
}

static BlockTask *make_task(Stage *stage, size_t block, size_t worker) {
    BlockTask *task = (BlockTask *)calloc(1, sizeof(BlockTask));
    if (task == NULL) {
        return NULL;
    }
    task->src = stage->src;
    task->dst = stage->dst;
    task->offset = block * BLOCK_SIZE;
    task->len = BLOCK_SIZE;
    task->worker = worker;
    task->ok = stage->ok;
    return task;
}

static int submit_next_to_worker(Stage *stage, size_t worker) {
    if (stage->next_block >= stage->block_count ||
        stage->inflight[worker] >= PER_WORKER_LIMIT) {
        return 1;
    }

    size_t block = stage->next_block++;
    BlockTask *task = make_task(stage, block, worker);
    if (task == NULL) {
        return 0;
    }
    task_func_t run = stage->verify ? verify_block : crypt_block;
    if (!thread_pool_submit(stage->pool, worker, run, task, completion_task, task)) {
        free(task);
        return 0;
    }
    ++stage->inflight[worker];
    return 1;
}

static int fill_worker(Stage *stage, size_t worker) {
    while (stage->next_block < stage->block_count &&
           stage->inflight[worker] < PER_WORKER_LIMIT) {
        if (!submit_next_to_worker(stage, worker)) {
            return 0;
        }
    }
    return 1;
}

static int run_stage(Stage *stage) {
    for (size_t i = 0; i < stage->threads; ++i) {
        if (!fill_worker(stage, i)) {
            return 0;
        }
    }

    while (stage->completed < stage->block_count) {
        task_t done;
        if (!queue_pop(stage->done_queue, &done)) {
            return 0;
        }
        BlockTask *task = (BlockTask *)done.arg;
        size_t worker = task->worker;
        if (done.function != NULL) {
            done.function(done.arg);
        }
        free(task);
        --stage->inflight[worker];
        ++stage->completed;
        if (!fill_worker(stage, worker)) {
            return 0;
        }
    }
    return 1;
}

static long parse_arg(int argc, char **argv, const char *name, long fallback) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (strcmp(argv[i], name) == 0) {
            return strtol(argv[i + 1], NULL, 10);
        }
    }
    return fallback;
}

int main(int argc, char **argv) {
    long size_arg = parse_arg(argc, argv, "-size", 1048576);
    long threads_arg = parse_arg(argc, argv, "-threads", 4);
    if (size_arg <= 0 || threads_arg <= 0 || size_arg % BLOCK_SIZE != 0) {
        puts("FAIL");
        return 1;
    }

    size_t size = (size_t)size_arg;
    size_t threads = (size_t)threads_arg;
    unsigned char *buffer1 = (unsigned char *)malloc(size);
    unsigned char *buffer2 = (unsigned char *)malloc(size);
    if (buffer1 == NULL || buffer2 == NULL) {
        free(buffer1);
        free(buffer2);
        puts("FAIL");
        return 1;
    }

    uint32_t state = 1;
    for (size_t i = 0; i < size; ++i) {
        buffer1[i] = random_byte(&state);
    }
    memset(buffer2, 0, size);

    queue_t *done_queue = queue_create(threads * PER_WORKER_LIMIT + 1);
    thread_pool_t *pool = thread_pool_create(threads, PER_WORKER_LIMIT, done_queue);
    size_t *inflight = (size_t *)calloc(threads, sizeof(size_t));
    int ok = 1;

    if (done_queue == NULL || pool == NULL || inflight == NULL) {
        ok = 0;
    }

    if (ok) {
        Stage encrypt = {pool, done_queue, buffer1, buffer2, size,
                         size / BLOCK_SIZE, 0, 0, threads, inflight, &ok, 0};
        ok = run_stage(&encrypt);
    }

    if (ok) {
        memset(inflight, 0, threads * sizeof(size_t));
        Stage verify = {pool, done_queue, buffer2, buffer1, size,
                        size / BLOCK_SIZE, 0, 0, threads, inflight, &ok, 1};
        ok = run_stage(&verify);
    }

    thread_pool_destroy(pool);
    queue_destroy(done_queue);
    free(inflight);
    free(buffer1);
    free(buffer2);

    puts(ok ? "OK" : "FAIL");
    return ok ? 0 : 1;
}
