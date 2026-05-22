#include <stdio.h>
#include <string.h>

typedef struct {
    int to;
    int next;
    int rev;
    int cap;
    int orig_id;
    int from_v;
    int to_v;
} Edge;

typedef struct {
    int u;
    int v;
} Orig;

typedef struct {
    int to;
    int id;
    int next;
} UsedEdge;

enum {
    MAXN = 3005,
    MAXM = 10005,
    MAXV = 7005,
    MAXE = 120005
};

static Edge edges[MAXE];
static int head[MAXV];
static int level_[MAXV];
static int ptr[MAXV];
static int q[MAXV];
static int edge_cnt;
static Orig orig[MAXM];

static int used_head[MAXN];
static UsedEdge used_edges[2 * MAXM];
static int used_cnt;
static char used_mark[2 * MAXM];
static int path_edges[MAXM];

static int vin(int v) { return v * 2; }
static int vout(int v) { return v * 2 + 1; }

static void add_edge_full(int u, int v, int cap, int orig_id, int from_v, int to_v) {
    edges[edge_cnt] = (Edge){v, head[u], edge_cnt + 1, cap, orig_id, from_v, to_v};
    head[u] = edge_cnt++;
    edges[edge_cnt] = (Edge){u, head[v], edge_cnt - 1, 0, 0, 0, 0};
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

static int dfs(int v, int t, int pushed) {
    if (v == t || pushed == 0) {
        return pushed;
    }
    for (int *pe = &ptr[v]; *pe != -1; *pe = edges[*pe].next) {
        int e = *pe;
        int to = edges[e].to;
        if (edges[e].cap <= 0 || level_[to] != level_[v] + 1) {
            continue;
        }
        int tr = dfs(to, t, pushed < edges[e].cap ? pushed : edges[e].cap);
        if (tr == 0) {
            continue;
        }
        edges[e].cap -= tr;
        edges[edges[e].rev].cap += tr;
        return tr;
    }
    return 0;
}

static void add_used(int u, int v, int id) {
    used_edges[used_cnt] = (UsedEdge){v, id, used_head[u]};
    used_head[u] = used_cnt++;
}

static int extract_path(int v, int t, int len) {
    if (v == t) {
        return len;
    }
    for (int e = used_head[v]; e != -1; e = used_edges[e].next) {
        if (used_mark[e]) {
            continue;
        }
        used_mark[e] = 1;
        path_edges[len] = used_edges[e].id;
        int got = extract_path(used_edges[e].to, t, len + 1);
        if (got != -1) {
            return got;
        }
        used_mark[e] = 0;
    }
    return -1;
}

int main(void) {
    int n, m, oriented, s, t;
    if (scanf("%d%d%d", &n, &m, &oriented) != 3) {
        return 0;
    }
    scanf("%d%d", &s, &t);

    memset(head, -1, sizeof(head));
    edge_cnt = 0;

    for (int v = 1; v <= n; ++v) {
        int cap = (v == s || v == t) ? m + 5 : 1;
        add_edge_full(vin(v), vout(v), cap, 0, 0, 0);
    }

    for (int i = 1; i <= m; ++i) {
        scanf("%d%d", &orig[i].u, &orig[i].v);
        add_edge_full(vout(orig[i].u), vin(orig[i].v), 1, i, orig[i].u, orig[i].v);
        if (!oriented) {
            add_edge_full(vout(orig[i].v), vin(orig[i].u), 1, i, orig[i].v, orig[i].u);
        }
    }

    int source = vout(s);
    int sink = vin(t);
    int flow = 0;
    while (bfs(source, sink)) {
        memcpy(ptr, head, sizeof(head));
        while (1) {
            int pushed = dfs(source, sink, 1);
            if (pushed == 0) {
                break;
            }
            flow += pushed;
        }
    }

    memset(used_head, -1, sizeof(used_head));
    used_cnt = 0;
    for (int e = 0; e < edge_cnt; e += 2) {
        if (edges[e].orig_id != 0 && edges[e].cap == 0) {
            add_used(edges[e].from_v, edges[e].to_v, edges[e].orig_id);
        }
    }

    printf("%d\n", flow);
    for (int k = 0; k < flow; ++k) {
        int len = extract_path(s, t, 0);
        printf("%d\n", len);
        int cur = s;
        for (int i = 0; i < len; ++i) {
            int id = path_edges[i];
            int next_v = (orig[id].u == cur) ? orig[id].v : orig[id].u;
            if (next_v == cur) {
                next_v = orig[id].u + orig[id].v - cur;
            }
            printf("%d %d %d\n", cur, id, next_v);
            cur = next_v;
        }
    }

    return 0;
}
