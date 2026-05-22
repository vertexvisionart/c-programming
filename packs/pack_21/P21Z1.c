#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;

typedef struct {
    int to;
    int next;
    int rev;
    ll cap;
} Edge;

enum { MAXN = 105, MAXE = 40010 };

static Edge edges[MAXE];
static int head[MAXN];
static int level_[MAXN];
static int ptr[MAXN];
static int q[MAXN];
static int edge_cnt;
static int n;

static void add_edge(int u, int v, ll cap) {
    edges[edge_cnt] = (Edge){v, head[u], edge_cnt + 1, cap};
    head[u] = edge_cnt++;
    edges[edge_cnt] = (Edge){u, head[v], edge_cnt - 1, 0};
    head[v] = edge_cnt++;
}

static int bfs(int s, int t) {
    int qb = 0, qe = 0;
    memset(level_, -1, sizeof(level_));
    level_[s] = 0;
    q[qe++] = s;
    while (qb < qe) {
        int v = q[qb++];
        for (int e = head[v]; e != -1; e = edges[e].next) {
            if (edges[e].cap > 0 && level_[edges[e].to] == -1) {
                level_[edges[e].to] = level_[v] + 1;
                q[qe++] = edges[e].to;
            }
        }
    }
    return level_[t] != -1;
}

static ll dfs(int v, int t, ll pushed) {
    if (v == t || pushed == 0) {
        return pushed;
    }
    for (int *pe = &ptr[v]; *pe != -1; *pe = edges[*pe].next) {
        int e = *pe;
        int to = edges[e].to;
        if (edges[e].cap <= 0 || level_[to] != level_[v] + 1) {
            continue;
        }
        ll tr = dfs(to, t, pushed < edges[e].cap ? pushed : edges[e].cap);
        if (tr == 0) {
            continue;
        }
        edges[e].cap -= tr;
        edges[edges[e].rev].cap += tr;
        return tr;
    }
    return 0;
}

int main(void) {
    int m;
    if (scanf("%d%d", &n, &m) != 2) {
        return 0;
    }
    memset(head, -1, sizeof(head));
    edge_cnt = 0;

    for (int i = 0; i < m; ++i) {
        int u, v;
        ll c;
        scanf("%d%d%lld", &u, &v, &c);
        add_edge(u, v, c);
        add_edge(v, u, c);
    }

    ll flow = 0;
    while (bfs(1, n)) {
        memcpy(ptr, head, sizeof(head));
        while (1) {
            ll pushed = dfs(1, n, (ll)4e18);
            if (pushed == 0) {
                break;
            }
            flow += pushed;
        }
    }

    printf("%lld\n", flow);
    return 0;
}
