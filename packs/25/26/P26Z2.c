#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    const char *data;
    size_t size;
    const char *mask;
    size_t mask_len;
    size_t start;
    size_t end;
    unsigned long long first_line;
    unsigned long long count;
    FILE *tmp;
    int error;
} Worker;

static int mask_match(const char *s, const char *mask, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (mask[i] != '?' && mask[i] != s[i]) {
            return 0;
        }
    }
    return 1;
}

static void *worker_main(void *arg) {
    Worker *w = (Worker *)arg;
    w->tmp = tmpfile();
    if (w->tmp == NULL) {
        w->error = errno;
        return NULL;
    }

    unsigned long long line_no = w->first_line;
    size_t line_start = w->start;

    while (line_start < w->end) {
        size_t line_end = line_start;
        while (line_end < w->end && w->data[line_end] != '\n') {
            ++line_end;
        }

        size_t pos = line_start;
        while (w->mask_len > 0 && pos + w->mask_len <= line_end) {
            if (mask_match(w->data + pos, w->mask, w->mask_len)) {
                unsigned long long column =
                    (unsigned long long)(pos - line_start + 1);
                fprintf(w->tmp, "%llu %llu ", line_no, column);
                fwrite(w->data + pos, 1, w->mask_len, w->tmp);
                fputc('\n', w->tmp);
                ++w->count;
                pos += w->mask_len;
            } else {
                ++pos;
            }
        }

        if (line_end == w->end) {
            break;
        }
        line_start = line_end + 1;
        ++line_no;
    }

    fflush(w->tmp);
    rewind(w->tmp);
    return NULL;
}

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static unsigned long long line_number_at(const char *data, size_t pos) {
    unsigned long long line = 1;
    for (size_t i = 0; i < pos; ++i) {
        if (data[i] == '\n') {
            ++line;
        }
    }
    return line;
}

static void copy_file(FILE *from, FILE *to) {
    char buf[65536];
    size_t got;
    while ((got = fread(buf, 1, sizeof(buf), from)) > 0) {
        fwrite(buf, 1, got, to);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <file> <mask>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    const char *mask = argv[2];
    if (!file_exists(file_name) && file_exists(argv[2])) {
        mask = argv[1];
        file_name = argv[2];
    }

    size_t mask_len = strlen(mask);
    if (mask_len > 1000) {
        fprintf(stderr, "mask is too long\n");
        return 1;
    }

    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror(file_name);
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t size = (size_t)st.st_size;
    if (size == 0 || mask_len == 0) {
        printf("0\n");
        close(fd);
        return 0;
    }

    const char *data =
        (const char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    long cpu = sysconf(_SC_NPROCESSORS_ONLN);
    size_t threads_count = cpu > 1 ? (size_t)cpu : 2;
    if (threads_count > 8) {
        threads_count = 8;
    }
    if (threads_count > size) {
        threads_count = size;
    }
    if (threads_count < 2 && size > 1) {
        threads_count = 2;
    }

    size_t *starts = (size_t *)calloc(threads_count + 1, sizeof(size_t));
    Worker *workers = (Worker *)calloc(threads_count, sizeof(Worker));
    pthread_t *threads = (pthread_t *)calloc(threads_count, sizeof(pthread_t));
    if (starts == NULL || workers == NULL || threads == NULL) {
        munmap((void *)data, size);
        close(fd);
        free(starts);
        free(workers);
        free(threads);
        return 1;
    }

    starts[0] = 0;
    starts[threads_count] = size;
    for (size_t i = 1; i < threads_count; ++i) {
        size_t pos = (size * i) / threads_count;
        if (pos > 0 && data[pos - 1] != '\n') {
            while (pos < size && data[pos] != '\n') {
                ++pos;
            }
            if (pos < size) {
                ++pos;
            }
        }
        starts[i] = pos;
    }

    int ok = 1;
    for (size_t i = 0; i < threads_count; ++i) {
        workers[i].data = data;
        workers[i].size = size;
        workers[i].mask = mask;
        workers[i].mask_len = mask_len;
        workers[i].start = starts[i];
        workers[i].end = starts[i + 1];
        workers[i].first_line = line_number_at(data, starts[i]);
        if (pthread_create(&threads[i], NULL, worker_main, &workers[i]) != 0) {
            ok = 0;
            workers[i].error = errno;
        }
    }

    for (size_t i = 0; i < threads_count; ++i) {
        if (workers[i].error == 0) {
            pthread_join(threads[i], NULL);
        }
    }

    if (!ok) {
        fprintf(stderr, "failed to create worker thread\n");
        munmap((void *)data, size);
        close(fd);
        free(starts);
        free(workers);
        free(threads);
        return 1;
    }

    unsigned long long total = 0;
    for (size_t i = 0; i < threads_count; ++i) {
        if (workers[i].error != 0) {
            fprintf(stderr, "worker error: %s\n", strerror(workers[i].error));
            ok = 0;
        }
        total += workers[i].count;
    }

    if (ok) {
        printf("%llu\n", total);
        for (size_t i = 0; i < threads_count; ++i) {
            if (workers[i].tmp != NULL) {
                copy_file(workers[i].tmp, stdout);
            }
        }
    }

    for (size_t i = 0; i < threads_count; ++i) {
        if (workers[i].tmp != NULL) {
            fclose(workers[i].tmp);
        }
    }
    munmap((void *)data, size);
    close(fd);
    free(starts);
    free(workers);
    free(threads);
    return ok ? 0 : 1;
}
