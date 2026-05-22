#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const double TIME_BUDGET = 0.85;

static int vertex_count;
static int edge_count;

static int *adj_head;
static int *adj_next;
static int *adj_to;
static int adj_cnt;

static int *position_in_path;
static int *current_path;
static int current_length;

static int *best_path;
static int best_length;

static unsigned long long rng_state;

static unsigned int rng_next(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return (unsigned int)(rng_state & 0xFFFFFFFFu);
}

static double elapsed_since(clock_t origin)
{
    return (double)(clock() - origin) / (double)CLOCKS_PER_SEC;
}

static void append_edge(int from, int to_vertex)
{
    adj_to[adj_cnt] = to_vertex;
    adj_next[adj_cnt] = adj_head[from];
    adj_head[from] = adj_cnt++;
}

static void remember_if_longer(void)
{
    if (current_length > best_length) {
        best_length = current_length;
        memcpy(best_path, current_path, sizeof(int) * (size_t)current_length);
    }
}

static int try_extend(void)
{
    int tail = current_path[current_length - 1];
    for (int e = adj_head[tail]; e != -1; e = adj_next[e]) {
        int neighbour = adj_to[e];
        if (position_in_path[neighbour] < 0) {
            position_in_path[neighbour] = current_length;
            current_path[current_length++] = neighbour;
            return 1;
        }
    }
    return 0;
}

static void reverse_segment(int lo, int hi)
{
    while (lo < hi) {
        int left = current_path[lo];
        int right = current_path[hi];
        current_path[lo] = right;
        current_path[hi] = left;
        position_in_path[right] = lo;
        position_in_path[left] = hi;
        lo++;
        hi--;
    }
}

static int try_rotate(void)
{
    int tail = current_path[current_length - 1];
    int candidates = 0;
    for (int e = adj_head[tail]; e != -1; e = adj_next[e]) {
        int neighbour = adj_to[e];
        int pos = position_in_path[neighbour];
        if (pos >= 0 && pos < current_length - 2) {
            candidates++;
        }
    }
    if (candidates == 0) {
        return 0;
    }

    unsigned int pick = rng_next() % (unsigned int)candidates;
    int seen = 0;
    int chosen_pos = -1;
    for (int e = adj_head[tail]; e != -1; e = adj_next[e]) {
        int neighbour = adj_to[e];
        int pos = position_in_path[neighbour];
        if (pos >= 0 && pos < current_length - 2) {
            if ((unsigned int)seen == pick) {
                chosen_pos = pos;
                break;
            }
            seen++;
        }
    }

    reverse_segment(chosen_pos + 1, current_length - 1);
    return 1;
}

static void flip_whole_path(void)
{
    reverse_segment(0, current_length - 1);
}

static void reset_to_start(int start_vertex)
{
    for (int i = 0; i < current_length; i++) {
        position_in_path[current_path[i]] = -1;
    }
    current_length = 0;
    current_path[current_length++] = start_vertex;
    position_in_path[start_vertex] = 0;
}

static int read_header(FILE *stream)
{
    return fscanf(stream, "%d %d", &vertex_count, &edge_count) == 2;
}

static int read_edges(FILE *stream)
{
    for (int i = 0; i < edge_count; i++) {
        int u;
        int v;
        if (fscanf(stream, "%d %d", &u, &v) != 2) {
            return 0;
        }
        if (u == v) {
            continue;
        }
        append_edge(u, v);
        append_edge(v, u);
    }
    return 1;
}

static void search_long_path(void)
{
    clock_t origin = clock();
    rng_state = 0x9E3779B97F4A7C15ULL;

    while (elapsed_since(origin) < TIME_BUDGET && best_length < vertex_count) {
        int start = (int)(rng_next() % (unsigned int)vertex_count) + 1;
        reset_to_start(start);
        remember_if_longer();

        int rotations = 0;
        int rotation_cap = 3 * vertex_count + 100;
        int stuck_flips = 0;

        while (elapsed_since(origin) < TIME_BUDGET && current_length < vertex_count) {
            if (try_extend()) {
                remember_if_longer();
                rotations = 0;
                stuck_flips = 0;
                continue;
            }
            if (try_rotate()) {
                rotations++;
                if (rotations > rotation_cap) {
                    flip_whole_path();
                    rotations = 0;
                    if (++stuck_flips > 2) {
                        break;
                    }
                }
                continue;
            }
            flip_whole_path();
            if (!try_extend() && !try_rotate()) {
                break;
            }
            remember_if_longer();
            rotations = 0;
        }
    }
}

static void write_result(FILE *stream)
{
    fprintf(stream, "%d\n", best_length);
    for (int i = 0; i < best_length; i++) {
        fprintf(stream, "%d%c", best_path[i],
                (i + 1 == best_length) ? '\n' : ' ');
    }
}

int main(void)
{
    FILE *input = fopen("input.txt", "r");
    FILE *output = fopen("output.txt", "w");

    if (input == NULL || output == NULL) {
        if (input != NULL) {
            fclose(input);
        }
        if (output != NULL) {
            fclose(output);
        }
        return 1;
    }

    if (!read_header(input)) {
        fclose(input);
        fclose(output);
        return 1;
    }

    int adj_capacity = 2 * edge_count + 4;
    adj_head = malloc(sizeof(int) * (size_t)(vertex_count + 2));
    adj_next = malloc(sizeof(int) * (size_t)adj_capacity);
    adj_to   = malloc(sizeof(int) * (size_t)adj_capacity);
    position_in_path = malloc(sizeof(int) * (size_t)(vertex_count + 2));
    current_path     = malloc(sizeof(int) * (size_t)(vertex_count + 2));
    best_path        = malloc(sizeof(int) * (size_t)(vertex_count + 2));

    if (!adj_head || !adj_next || !adj_to
        || !position_in_path || !current_path || !best_path) {
        fclose(input);
        fclose(output);
        return 1;
    }

    for (int i = 0; i <= vertex_count + 1; i++) {
        adj_head[i] = -1;
        position_in_path[i] = -1;
    }
    adj_cnt = 0;
    current_length = 0;
    best_length = 0;

    read_edges(input);
    fclose(input);

    search_long_path();
    write_result(output);

    fclose(output);
    free(adj_head);
    free(adj_next);
    free(adj_to);
    free(position_in_path);
    free(current_path);
    free(best_path);
    return 0;
}
