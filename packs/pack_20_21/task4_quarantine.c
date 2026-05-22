#include <stdio.h>
#include <stdlib.h>

#define INF 4000000000000000000LL

typedef struct {
    int to;
    int next;
    int w;
} Edge;

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    if (!fin || !fout) {
        return 0;
    }

    int n, m, k;
    if (fscanf(fin, "%d %d %d", &n, &m, &k) != 3) {
        fclose(fin);
        fclose(fout);
        return 0;
    }

    int *friends = (int *)malloc(k * sizeof(int));
    for (int i = 0; i < k; ++i) {
        fscanf(fin, "%d", &friends[i]);
    }

    int *head = (int *)malloc((n + 1) * sizeof(int));
    Edge *edges = (Edge *)malloc((m + 1) * sizeof(Edge));
    for (int i = 1; i <= n; ++i) {
        head[i] = -1;
    }

    for (int i = 1; i <= m; ++i) {
        int a, b, w;
        fscanf(fin, "%d %d %d", &a, &b, &w);
        edges[i].to = b;
        edges[i].w = w;
        edges[i].next = head[a];
        head[a] = i;
    }

    long long *dist = (long long *)malloc((n + 1) * sizeof(long long));
    int *parent = (int *)malloc((n + 1) * sizeof(int));
    int *in_queue = (int *)calloc(n + 1, sizeof(int));
    int *queue = (int *)malloc((m + n + 5) * sizeof(int));
    int *path = (int *)malloc((n + 1) * sizeof(int));

    for (int i = 1; i <= n; ++i) {
        dist[i] = INF;
        parent[i] = -1;
    }

    int qh = 0;
    int qt = 0;
    dist[1] = 0;
    queue[qt++] = 1;
    in_queue[1] = 1;

    while (qh != qt) {
        int v = queue[qh++];
        if (qh == m + n + 5) {
            qh = 0;
        }
        in_queue[v] = 0;

        for (int e = head[v]; e != -1; e = edges[e].next) {
            int to = edges[e].to;
            long long nd = dist[v] + edges[e].w;
            if (nd < dist[to]) {
                dist[to] = nd;
                parent[to] = v;
                if (!in_queue[to]) {
                    queue[qt++] = to;
                    if (qt == m + n + 5) {
                        qt = 0;
                    }
                    in_queue[to] = 1;
                }
            }
        }
    }

    for (int i = 0; i < k; ++i) {
        int v = friends[i];
        int len = 0;
        while (v != -1) {
            path[len++] = v;
            v = parent[v];
        }

        fprintf(fout, "%lld %d", dist[friends[i]], len);
        for (int j = len - 1; j >= 0; --j) {
            fprintf(fout, " %d", path[j]);
        }
        fputc('\n', fout);
    }

    free(friends);
    free(head);
    free(edges);
    free(dist);
    free(parent);
    free(in_queue);
    free(queue);
    free(path);
    fclose(fin);
    fclose(fout);
    return 0;
}
