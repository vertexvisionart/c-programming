#include <stdio.h>
#include <stdlib.h>

#define INF 4000000000000000000LL

typedef struct {
    int u;
    int v;
    long long w;
} Edge;

typedef struct {
    int to;
    int next;
} AdjEdge;

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    if (!fin || !fout) {
        return 0;
    }

    int n, m, s;
    if (fscanf(fin, "%d %d %d", &n, &m, &s) != 3) {
        fclose(fin);
        fclose(fout);
        return 0;
    }

    Edge *edges = (Edge *)malloc(m * sizeof(Edge));
    int *head = (int *)malloc((n + 1) * sizeof(int));
    AdjEdge *adj = (AdjEdge *)malloc(m * sizeof(AdjEdge));
    for (int i = 1; i <= n; ++i) {
        head[i] = -1;
    }

    for (int i = 0; i < m; ++i) {
        fscanf(fin, "%d %d %lld", &edges[i].u, &edges[i].v, &edges[i].w);
        adj[i].to = edges[i].v;
        adj[i].next = head[edges[i].u];
        head[edges[i].u] = i;
    }

    long long *dist = (long long *)malloc((n + 1) * sizeof(long long));
    int *bad = (int *)calloc(n + 1, sizeof(int));
    int *queue = (int *)malloc((n + 5) * sizeof(int));

    for (int i = 1; i <= n; ++i) {
        dist[i] = INF;
    }
    dist[s] = 0;

    for (int it = 1; it <= n - 1; ++it) {
        int changed = 0;
        for (int i = 0; i < m; ++i) {
            int u = edges[i].u;
            int v = edges[i].v;
            long long w = edges[i].w;
            if (dist[u] == INF) {
                continue;
            }
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                changed = 1;
            }
        }
        if (!changed) {
            break;
        }
    }

    int qh = 0;
    int qt = 0;
    for (int i = 0; i < m; ++i) {
        int u = edges[i].u;
        int v = edges[i].v;
        long long w = edges[i].w;
        if (dist[u] == INF) {
            continue;
        }
        if (dist[u] + w < dist[v] && !bad[v]) {
            bad[v] = 1;
            queue[qt++] = v;
        }
    }

    while (qh < qt) {
        int v = queue[qh++];
        for (int e = head[v]; e != -1; e = adj[e].next) {
            int to = adj[e].to;
            if (!bad[to]) {
                bad[to] = 1;
                queue[qt++] = to;
            }
        }
    }

    for (int i = 1; i <= n; ++i) {
        if (dist[i] == INF) {
            fprintf(fout, "*\n");
        } else if (bad[i]) {
            fprintf(fout, "-\n");
        } else {
            fprintf(fout, "%lld\n", dist[i]);
        }
    }

    free(edges);
    free(head);
    free(adj);
    free(dist);
    free(bad);
    free(queue);
    fclose(fin);
    fclose(fout);
    return 0;
}
