#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int to;
    int next;
    int id;
} Edge;

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
    int *tin = (int *)calloc(n + 1, sizeof(int));
    int *low = (int *)calloc(n + 1, sizeof(int));
    int *parent = (int *)malloc((n + 1) * sizeof(int));
    int *parent_edge = (int *)malloc((n + 1) * sizeof(int));
    int *iter = (int *)malloc((n + 1) * sizeof(int));
    int *children = (int *)calloc(n + 1, sizeof(int));
    int *stack = (int *)malloc((n + 1) * sizeof(int));
    int *is_cut = (int *)calloc(n + 1, sizeof(int));

    for (int i = 1; i <= n; ++i) {
        head[i] = -1;
    }

    int ecnt = 0;
    for (int i = 1; i <= m; ++i) {
        int u, v;
        fscanf(fin, "%d %d", &u, &v);

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
                if (parent[v] == -1) {
                    if (children[v] > 1) {
                        is_cut[v] = 1;
                    }
                } else {
                    int p = parent[v];
                    if (low[v] >= tin[p]) {
                        is_cut[p] = 1;
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
                ++children[v];
                tin[to] = low[to] = ++timer;
                iter[to] = head[to];
                stack[top++] = to;
            } else if (tin[to] < tin[v] && tin[to] < low[v]) {
                low[v] = tin[to];
            }
        }

        is_cut[start] = (children[start] > 1);
    }

    int count = 0;
    for (int i = 1; i <= n; ++i) {
        if (is_cut[i]) {
            ++count;
        }
    }

    fprintf(fout, "%d\n", count);
    int first = 1;
    for (int i = 1; i <= n; ++i) {
        if (is_cut[i]) {
            if (!first) {
                fputc(' ', fout);
            }
            first = 0;
            fprintf(fout, "%d", i);
        }
    }
    fputc('\n', fout);

    free(head);
    free(edges);
    free(tin);
    free(low);
    free(parent);
    free(parent_edge);
    free(iter);
    free(children);
    free(stack);
    free(is_cut);
    fclose(fin);
    fclose(fout);
    return 0;
}
