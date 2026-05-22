#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int to;
    int next;
    int id;
} Edge;

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    if (!fin || !fout) {
        return 0;
    }

    int n, m;
    if (fscanf(fin, "%d %d", &n, &m) != 2) {
        fclose(fin);
        fclose(fout);
        return 0;
    }

    int *head = (int *)malloc((n + 1) * sizeof(int));
    Edge *edges = (Edge *)malloc((2 * m + 1) * sizeof(Edge));
    int *eu = (int *)malloc((m + 1) * sizeof(int));
    int *ev = (int *)malloc((m + 1) * sizeof(int));
    int *tin = (int *)calloc(n + 1, sizeof(int));
    int *low = (int *)calloc(n + 1, sizeof(int));
    int *parent = (int *)malloc((n + 1) * sizeof(int));
    int *parent_edge = (int *)malloc((n + 1) * sizeof(int));
    int *iter = (int *)malloc((n + 1) * sizeof(int));
    int *stack = (int *)malloc((n + 1) * sizeof(int));
    int *is_bridge = (int *)calloc(m + 1, sizeof(int));

    for (int i = 1; i <= n; ++i) {
        head[i] = -1;
    }

    int ecnt = 0;
    for (int i = 1; i <= m; ++i) {
        int u, v;
        fscanf(fin, "%d %d", &u, &v);
        eu[i] = u;
        ev[i] = v;

        edges[ecnt].to = v;
        edges[ecnt].id = i;
        edges[ecnt].next = head[u];
        head[u] = ecnt++;

        edges[ecnt].to = u;
        edges[ecnt].id = i;
        edges[ecnt].next = head[v];
        head[v] = ecnt++;
    }

    int timer = 0;
    for (int start = 1; start <= n; ++start) {
        if (tin[start] != 0) {
            continue;
        }

        int top = 0;
        stack[top++] = start;
        parent[start] = -1;
        parent_edge[start] = -1;
        tin[start] = low[start] = ++timer;
        iter[start] = head[start];

        while (top > 0) {
            int v = stack[top - 1];
            int e = iter[v];

            if (e == -1) {
                --top;
                if (parent[v] != -1) {
                    int p = parent[v];
                    if (low[v] > tin[p]) {
                        is_bridge[parent_edge[v]] = 1;
                    }
                    if (low[v] < low[p]) {
                        low[p] = low[v];
                    }
                }
                continue;
            }

            iter[v] = edges[e].next;
            int to = edges[e].to;
            int id = edges[e].id;

            if (id == parent_edge[v]) {
                continue;
            }

            if (tin[to] == 0) {
                parent[to] = v;
                parent_edge[to] = id;
                tin[to] = low[to] = ++timer;
                iter[to] = head[to];
                stack[top++] = to;
            } else if (tin[to] < tin[v] && tin[to] < low[v]) {
                low[v] = tin[to];
            }
        }
    }

    int *ans = (int *)malloc(m * sizeof(int));
    int ans_sz = 0;
    for (int i = 1; i <= m; ++i) {
        if (is_bridge[i]) {
            ans[ans_sz++] = i;
        }
    }
    qsort(ans, ans_sz, sizeof(int), cmp_int);

    fprintf(fout, "%d\n", ans_sz);
    for (int i = 0; i < ans_sz; ++i) {
        if (i) {
            fputc(' ', fout);
        }
        fprintf(fout, "%d", ans[i]);
    }
    fputc('\n', fout);

    free(head);
    free(edges);
    free(eu);
    free(ev);
    free(tin);
    free(low);
    free(parent);
    free(parent_edge);
    free(iter);
    free(stack);
    free(is_bridge);
    free(ans);
    fclose(fin);
    fclose(fout);
    return 0;
}
