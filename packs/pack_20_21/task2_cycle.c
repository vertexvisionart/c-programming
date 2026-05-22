#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int to;
    int next;
} Edge;

static void reverse_array(int *a, int n) {
    int l = 0;
    int r = n - 1;
    while (l < r) {
        int t = a[l];
        a[l] = a[r];
        a[r] = t;
        ++l;
        --r;
    }
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
    int *color = (int *)calloc(n + 1, sizeof(int));
    int *parent = (int *)malloc((n + 1) * sizeof(int));
    int *iter = (int *)malloc((n + 1) * sizeof(int));
    int *stack = (int *)malloc((n + 1) * sizeof(int));
    int *cycle = (int *)malloc((n + 1) * sizeof(int));
    Edge *edges = (Edge *)malloc((m + 1) * sizeof(Edge));

    for (int i = 1; i <= n; ++i) {
        head[i] = -1;
        parent[i] = -1;
    }

    for (int i = 1; i <= m; ++i) {
        int u, v;
        fscanf(fin, "%d %d", &u, &v);
        edges[i].to = v;
        edges[i].next = head[u];
        head[u] = i;
    }

    int cycle_len = 0;

    for (int start = 1; start <= n && cycle_len == 0; ++start) {
        if (color[start] != 0) {
            continue;
        }

        int top = 0;
        stack[top++] = start;
        color[start] = 1;
        iter[start] = head[start];
        parent[start] = -1;

        while (top > 0 && cycle_len == 0) {
            int v = stack[top - 1];
            int e = iter[v];

            if (e == -1) {
                color[v] = 2;
                --top;
                continue;
            }

            iter[v] = edges[e].next;
            int to = edges[e].to;

            if (color[to] == 0) {
                parent[to] = v;
                color[to] = 1;
                iter[to] = head[to];
                stack[top++] = to;
            } else if (color[to] == 1) {
                cycle[cycle_len++] = to;
                int cur = v;
                while (cur != to) {
                    cycle[cycle_len++] = cur;
                    cur = parent[cur];
                }
                reverse_array(cycle, cycle_len);
            }
        }
    }

    if (cycle_len == 0) {
        fprintf(fout, "-1\n");
    } else {
        fprintf(fout, "%d\n", cycle_len);
        for (int i = 0; i < cycle_len; ++i) {
            if (i) {
                fputc(' ', fout);
            }
            fprintf(fout, "%d", cycle[i]);
        }
        fputc('\n', fout);
    }

    free(head);
    free(color);
    free(parent);
    free(iter);
    free(stack);
    free(cycle);
    free(edges);
    fclose(fin);
    fclose(fout);
    return 0;
}
