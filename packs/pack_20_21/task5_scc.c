#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int to;
    int next;
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
    int *rhead = (int *)malloc((n + 1) * sizeof(int));
    Edge *edges = (Edge *)malloc((m + 1) * sizeof(Edge));
    Edge *redges = (Edge *)malloc((m + 1) * sizeof(Edge));
    int *state = (int *)calloc(n + 1, sizeof(int));
    int *iter = (int *)malloc((n + 1) * sizeof(int));
    int *stack = (int *)malloc((n + 1) * sizeof(int));
    int *order = (int *)malloc(n * sizeof(int));
    int *comp = (int *)calloc(n + 1, sizeof(int));

    for (int i = 1; i <= n; ++i) {
        head[i] = -1;
        rhead[i] = -1;
    }

    for (int i = 1; i <= m; ++i) {
        int u, v;
        fscanf(fin, "%d %d", &u, &v);
        edges[i].to = v;
        edges[i].next = head[u];
        head[u] = i;
        redges[i].to = u;
        redges[i].next = rhead[v];
        rhead[v] = i;
    }

    int order_sz = 0;
    for (int start = 1; start <= n; ++start) {
        if (state[start] != 0) {
            continue;
        }
        int top = 0;
        stack[top++] = start;
        state[start] = 1;
        iter[start] = head[start];

        while (top > 0) {
            int v = stack[top - 1];
            int e = iter[v];

            if (e == -1) {
                state[v] = 2;
                order[order_sz++] = v;
                --top;
                continue;
            }

            iter[v] = edges[e].next;
            int to = edges[e].to;
            if (state[to] == 0) {
                state[to] = 1;
                iter[to] = head[to];
                stack[top++] = to;
            }
        }
    }

    int comps = 0;
    for (int i = order_sz - 1; i >= 0; --i) {
        int start = order[i];
        if (comp[start] != 0) {
            continue;
        }
        ++comps;
        int top = 0;
        stack[top++] = start;
        comp[start] = comps;

        while (top > 0) {
            int v = stack[--top];
            for (int e = rhead[v]; e != -1; e = redges[e].next) {
                int to = redges[e].to;
                if (comp[to] == 0) {
                    comp[to] = comps;
                    stack[top++] = to;
                }
            }
        }
    }

    fprintf(fout, "%d\n", comps);
    for (int i = 1; i <= n; ++i) {
        if (i > 1) {
            fputc(' ', fout);
        }
        fprintf(fout, "%d", comp[i]);
    }
    fputc('\n', fout);

    free(head);
    free(rhead);
    free(edges);
    free(redges);
    free(state);
    free(iter);
    free(stack);
    free(order);
    free(comp);
    fclose(fin);
    fclose(fout);
    return 0;
}
