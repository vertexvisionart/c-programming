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

typedef struct {
    int r;
    int c;
} Cell;

enum { MAXHW = 160005, MAXM = 640005, MAXE = 4500000 };

static Edge *edges;
static int *head;
static int *level_;
static int *ptr;
static int *q;
static char *vis;
static int edge_cnt;
static int node_cnt;

static void add_edge(int u, int v, ll cap) {
    edges[edge_cnt] = (Edge){v, head[u], edge_cnt + 1, cap};
    head[u] = edge_cnt++;
    edges[edge_cnt] = (Edge){u, head[v], edge_cnt - 1, 0};
    head[v] = edge_cnt++;
}

static void add_undirected(int u, int v, ll cap) {
    add_edge(u, v, cap);
    add_edge(v, u, cap);
}

static int bfs(int s, int t) {
    int qb = 0, qe = 0;
    for (int i = 0; i < node_cnt; ++i) {
        level_[i] = -1;
    }
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
        ll take = pushed < edges[e].cap ? pushed : edges[e].cap;
        ll tr = dfs(to, t, take);
        if (tr == 0) {
            continue;
        }
        edges[e].cap -= tr;
        edges[edges[e].rev].cap += tr;
        return tr;
    }
    return 0;
}

static void bfs_reachable(int s) {
    int qb = 0, qe = 0;
    memset(vis, 0, node_cnt);
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
    int h, w;
    if (scanf("%d%d", &h, &w) != 2) {
        return 0;
    }

    int cells = h * w;
    int s = 0;
    int t = cells + 1;
    node_cnt = cells + 2;

    head = (int *)malloc((size_t)node_cnt * sizeof(int));
    level_ = (int *)malloc((size_t)node_cnt * sizeof(int));
    ptr = (int *)malloc((size_t)node_cnt * sizeof(int));
    q = (int *)malloc((size_t)node_cnt * sizeof(int));
    vis = (char *)malloc((size_t)node_cnt);
    edges = (Edge *)malloc((size_t)MAXE * sizeof(Edge));
    if (!head || !level_ || !ptr || !q || !vis || !edges) {
        return 0;
    }

    for (int i = 0; i < node_cnt; ++i) {
        head[i] = -1;
    }
    edge_cnt = 0;

    ll *a = (ll *)malloc((size_t)cells * sizeof(ll));
    ll *b = (ll *)malloc((size_t)cells * sizeof(ll));
    if (!a || !b) {
        return 0;
    }

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            int id = r * w + c;
            scanf("%lld%lld", &a[id], &b[id]);
        }
    }

    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            int id = r * w + c + 1;
            ll d0, d1;
            if (((r + c) & 1) == 0) {
                d0 = a[r * w + c];
                d1 = b[r * w + c];
            } else {
                d0 = b[r * w + c];
                d1 = a[r * w + c];
            }
            add_edge(s, id, d1);
            add_edge(id, t, d0);
        }
    }

    int m;
    scanf("%d", &m);
    ll bonus_sum = 0;
    for (int i = 0; i < m; ++i) {
        int r1, c1, r2, c2;
        ll p;
        scanf("%d%d%d%d%lld", &r1, &c1, &r2, &c2, &p);
        --r1; --c1; --r2; --c2;
        int u = r1 * w + c1 + 1;
        int v = r2 * w + c2 + 1;
        add_undirected(u, v, p);
        bonus_sum += p;
    }

    ll flow = 0;
    while (bfs(s, t)) {
        memcpy(ptr, head, (size_t)node_cnt * sizeof(int));
        while (1) {
            ll pushed = dfs(s, t, (ll)4e18);
            if (pushed == 0) {
                break;
            }
            flow += pushed;
        }
    }

    bfs_reachable(s);
    printf("%lld\n", flow - bonus_sum);
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            int id = r * w + c + 1;
            int y = vis[id] ? 0 : 1;
            int x;
            if (((r + c) & 1) == 0) {
                x = y;
            } else {
                x = 1 - y;
            }
            putchar(x ? '1' : '0');
        }
        putchar('\n');
    }

    free(a);
    free(b);
    free(edges);
    free(head);
    free(level_);
    free(ptr);
    free(q);
    free(vis);
    return 0;
}
