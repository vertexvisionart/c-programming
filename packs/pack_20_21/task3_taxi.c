#include <stdio.h>
#include <stdlib.h>

#define INF 4000000000000000000LL

static int build_path(int s, int t, int n, int **next_vertex, int *path) {
    int len = 0;
    int cur = s;
    path[len++] = cur;
    while (cur != t) {
        cur = next_vertex[cur][t];
        if (cur <= 0 || len > n) {
            return 0;
        }
        path[len++] = cur;
    }
    return len;
}

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    if (!fin || !fout) {
        return 0;
    }

    int n, m, p, k;
    if (fscanf(fin, "%d %d %d %d", &n, &m, &p, &k) != 4) {
        fclose(fin);
        fclose(fout);
        return 0;
    }

    long long **dist = (long long **)malloc((n + 1) * sizeof(long long *));
    int **next_vertex = (int **)malloc((n + 1) * sizeof(int *));
    for (int i = 0; i <= n; ++i) {
        dist[i] = (long long *)malloc((n + 1) * sizeof(long long));
        next_vertex[i] = (int *)malloc((n + 1) * sizeof(int));
    }

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            dist[i][j] = (i == j) ? 0 : INF;
            next_vertex[i][j] = (i == j) ? i : -1;
        }
    }

    for (int i = 0; i < m; ++i) {
        int u, v;
        long long w;
        fscanf(fin, "%d %d %lld", &u, &v, &w);
        if (w < dist[u][v]) {
            dist[u][v] = w;
            dist[v][u] = w;
            next_vertex[u][v] = v;
            next_vertex[v][u] = u;
        }
    }

    for (int mid = 1; mid <= n; ++mid) {
        for (int i = 1; i <= n; ++i) {
            if (dist[i][mid] == INF) {
                continue;
            }
            for (int j = 1; j <= n; ++j) {
                if (dist[mid][j] == INF) {
                    continue;
                }
                long long candidate = dist[i][mid] + dist[mid][j];
                if (candidate < dist[i][j]) {
                    dist[i][j] = candidate;
                    next_vertex[i][j] = next_vertex[i][mid];
                }
            }
        }
    }

    int *path = (int *)malloc((n + 1) * sizeof(int));

    for (int i = 0; i < p; ++i) {
        int s, t;
        fscanf(fin, "%d %d", &s, &t);
        int len = build_path(s, t, n, next_vertex, path);
        fprintf(fout, "%lld %d", dist[s][t], len);
        for (int j = 0; j < len; ++j) {
            fprintf(fout, " %d", path[j]);
        }
        fputc('\n', fout);
    }

    for (int i = 0; i < k; ++i) {
        int s, t;
        fscanf(fin, "%d %d", &s, &t);
        fprintf(fout, "%lld\n", dist[s][t]);
    }

    free(path);
    for (int i = 0; i <= n; ++i) {
        free(dist[i]);
        free(next_vertex[i]);
    }
    free(dist);
    free(next_vertex);
    fclose(fin);
    fclose(fout);
    return 0;
}
