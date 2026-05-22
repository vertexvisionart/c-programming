#include <stdio.h>
#include <string.h>

typedef long long ll;

typedef struct {
    int to;
    int next;
    int rev;
    ll cap;
} Edge;

typedef struct {
    int u;
    int v;
    ll c;
} Orig;

enum { MAXN = 105, MAXM = 5005, MAXE = 40010 };

static Edge edges[MAXE];
static int head[MAXN];
static int level_[MAXN];
static int ptr[MAXN];
static int q[MAXN];
static int vis[MAXN];
static Orig orig[MAXM];
static int edge_cnt;
static int n, m, s, t;

static void add_edge(int u, int v, ll cap) {
    edges[edge_cnt] = (Edge){v, head[u], edge_cnt + 1, cap};
    head[u] = edge_cnt++;
    edges[edge_cnt] = (Edge){u, head[v], edge_cnt - 1, 0};
    head[v] = edge_cnt++;
}

static int bfs_level(void) {
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

static ll dfs_flow(int v, ll pushed) {
    if (v == t || pushed == 0) {
        return pushed;
    }
    for (int *pe = &ptr[v]; *pe != -1; *pe = edges[*pe].next) {
        int e = *pe;
        int to = edges[e].to;
        if (edges[e].cap <= 0 || level_[to] != level_[v] + 1) {
            continue;
        }
        ll tr = dfs_flow(to, pushed < edges[e].cap ? pushed : edges[e].cap);
        if (tr == 0) {
            continue;
        }
        edges[e].cap -= tr;
        edges[edges[e].rev].cap += tr;
        return tr;
    }
    return 0;
}

static void bfs_residual(void) {
    int qb = 0, qe = 0;
    memset(vis, 0, sizeof(vis));
    vis[s] = 1;
    q[qe++] = s;
    while (qb < qe) {
        int v = q[qb++];
        for (int e = head[v]; e != -1; e = edges[e].next) {
            int to = edges[e].to;
            if (edges[e].cap > 0 && !vis[to]) {
                vis[to] = 1;
                q[qe++] = to;
            }
        }
    }
}

int main(void) {
    if (scanf("%d%d", &n, &m) != 2) {
        return 0;
    }
    scanf("%d%d", &s, &t);

    memset(head, -1, sizeof(head));
    edge_cnt = 0;
    for (int i = 0; i < m; ++i) {
        scanf("%d%d%lld", &orig[i].u, &orig[i].v, &orig[i].c);
        add_edge(orig[i].u, orig[i].v, orig[i].c);
        add_edge(orig[i].v, orig[i].u, orig[i].c);
    }

    ll flow = 0;
    while (bfs_level()) {
        memcpy(ptr, head, sizeof(head));
        while (1) {
            ll pushed = dfs_flow(s, (ll)4e18);
            if (pushed == 0) {
                break;
            }
            flow += pushed;
        }
    }

    bfs_residual();

    int cnt = 0;
    for (int i = 0; i < m; ++i) {
        if (vis[orig[i].u] != vis[orig[i].v]) {
            ++cnt;
        }
    }

    printf("%lld %d\n", flow, cnt);
    for (int i = 0; i < m; ++i) {
        if (vis[orig[i].u] != vis[orig[i].v]) {
            printf("%d %d\n", orig[i].u, orig[i].v);
        }
    }
    return 0;
}
