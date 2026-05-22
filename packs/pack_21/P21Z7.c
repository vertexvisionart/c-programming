#include <stdio.h>
#include <string.h>

typedef struct {
    int to;
    int next;
    int rev;
    int cap;
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
    MAXE = 50005
};

static Edge edges[MAXE];
static int head[MAXN];
static int level_[MAXN];
static int ptr[MAXN];
static int q[MAXN];
static int edge_cnt;

static Orig orig[MAXM];
static int dir_arc[MAXM][2];
static int used_head[MAXN];
static UsedEdge used_edges[2 * MAXM];
static int used_cnt;
static char used_mark[2 * MAXM];
static int path_edges[MAXM];
static int path_dir[MAXM];

static void add_edge(int u, int v, int cap, int *idx_out) {
    int idx = edge_cnt;
    edges[edge_cnt] = (Edge){v, head[u], edge_cnt + 1, cap};
    head[u] = edge_cnt++;
    edges[edge_cnt] = (Edge){u, head[v], edge_cnt - 1, 0};
    head[v] = edge_cnt++;
    if (idx_out) {
        *idx_out = idx;
    }
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
        int id = used_edges[e].id;
        path_edges[len] = id;
        path_dir[len] = (orig[id].u == v) ? 0 : 1;
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

    for (int i = 1; i <= m; ++i) {
        scanf("%d%d", &orig[i].u, &orig[i].v);
        if (oriented) {
            add_edge(orig[i].u, orig[i].v, 1, &dir_arc[i][0]);
            dir_arc[i][1] = -1;
        } else {
            add_edge(orig[i].u, orig[i].v, 1, &dir_arc[i][0]);
            add_edge(orig[i].v, orig[i].u, 1, &dir_arc[i][1]);
        }
    }

    int flow = 0;
    while (bfs(s, t)) {
        memcpy(ptr, head, sizeof(head));
        while (1) {
            int pushed = dfs(s, t, 1);
            if (pushed == 0) {
                break;
            }
            flow += pushed;
        }
    }

    memset(used_head, -1, sizeof(used_head));
    used_cnt = 0;
    for (int i = 1; i <= m; ++i) {
        int u = orig[i].u;
        int v = orig[i].v;
        int used_uv = (edges[dir_arc[i][0]].cap == 0);
        int used_vu = (dir_arc[i][1] != -1 && edges[dir_arc[i][1]].cap == 0);
        if (used_uv && !used_vu) {
            add_used(u, v, i);
        } else if (used_vu && !used_uv) {
            add_used(v, u, i);
        }
    }

    printf("%d\n", flow);
    for (int k = 0; k < flow; ++k) {
        int len = extract_path(s, t, 0);
        printf("%d\n", len);
        for (int i = 0; i < len; ++i) {
            int id = path_edges[i];
            int from = path_dir[i] == 0 ? orig[id].u : orig[id].v;
            int to = path_dir[i] == 0 ? orig[id].v : orig[id].u;
            printf("%d %d %d\n", from, id, to);
        }
    }

    return 0;
}
