#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 100001
#define MAXM 200001

int head[MAXN], to[MAXM], nxt_e[MAXM], eid[MAXM];
int edge_cnt;

void add_edge(int u, int v, int id) {
    edge_cnt++;
    to[edge_cnt]    = v;
    eid[edge_cnt]   = id;
    nxt_e[edge_cnt] = head[u];
    head[u]         = edge_cnt;
}

int tin[MAXN], low[MAXN], timer_val;
int visited[MAXN];
int is_cut[MAXN];  // точка сочленения?
int children[MAXN]; // кол-во детей в DFS-дереве (для корня)

int stk_v[MAXN], stk_e[MAXN], stk_par_eid[MAXN], stk_is_root[MAXN];
int stk_top;

void dfs(int start) {
    stk_top = 0;
    stk_v[stk_top]       = start;
    stk_e[stk_top]       = head[start];
    stk_par_eid[stk_top] = -1;
    stk_is_root[stk_top] = 1;
    visited[start]        = 1;
    tin[start] = low[start] = timer_val++;
    children[start] = 0;
    stk_top++;

    while (stk_top > 0) {
        int v      = stk_v[stk_top - 1];
        int e      = stk_e[stk_top - 1];
        int pe     = stk_par_eid[stk_top - 1];
        int is_root = stk_is_root[stk_top - 1];

        if (e == 0) {
            stk_top--;
            if (stk_top > 0) {
                int par = stk_v[stk_top - 1];
                // Обновляем low родителя
                if (low[v] < low[par]) low[par] = low[v];
                // Проверяем: par — точка сочленения?
                // (для не-корня: если low[v] >= tin[par])
                if (!stk_is_root[stk_top - 1] && low[v] >= tin[par])
                    is_cut[par] = 1;
                // Для корня считаем детей
                children[par]++;
            }
        } else {
            stk_e[stk_top - 1] = nxt_e[e];
            int u  = to[e];
            int ei = eid[e];

            if (ei == pe) continue; // ребро, по которому пришли — пропускаем

            if (!visited[u]) {
                visited[u] = 1;
                tin[u] = low[u] = timer_val++;
                children[u] = 0;
                stk_v[stk_top]       = u;
                stk_e[stk_top]       = head[u];
                stk_par_eid[stk_top] = ei;
                stk_is_root[stk_top] = 0;
                stk_top++;
            } else {
                // back edge
                if (tin[u] < low[v]) low[v] = tin[u];
            }
        }
    }

    // Корень — точка сочленения если ≥ 2 детей
    if (children[start] >= 2)
        is_cut[start] = 1;
}

int main(void) {
    int N, M;
    scanf("%d %d", &N, &M);

    for (int i = 1; i <= M; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        if (u == v) continue; // петля не влияет
        add_edge(u, v, i);
        add_edge(v, u, i);
    }

    for (int v = 1; v <= N; v++)
        if (!visited[v])
            dfs(v);

    int cnt = 0;
    for (int v = 1; v <= N; v++)
        if (is_cut[v]) cnt++;

    printf("%d\n", cnt);
    int first = 1;
    for (int v = 1; v <= N; v++) {
        if (is_cut[v]) {
            if (!first) printf(" ");
            printf("%d", v);
            first = 0;
        }
    }
    if (cnt > 0) printf("\n");

    return 0;
}