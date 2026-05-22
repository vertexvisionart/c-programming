#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;

typedef struct {
    int to;
    int w;
    int next;
} Edge;

typedef struct {
    ll dist;
    int v;
} HeapNode;

enum { MAXN = 3005, MAXM = 600005, MAXK = 25 };

static Edge edges[MAXM];
static int head[MAXN];
static int edge_cnt;
static ll dist_[MAXN];
static int parent_[MAXN];
static HeapNode heap[4 * MAXM];
static int heap_sz;
static int path[MAXN];

static void add_edge(int u, int v, int w) {
    edges[edge_cnt] = (Edge){v, w, head[u]};
    head[u] = edge_cnt++;
}

static void heap_push(ll dist, int v) {
    int i = ++heap_sz;
    while (i > 1 && heap[i / 2].dist > dist) {
        heap[i] = heap[i / 2];
        i /= 2;
    }
    heap[i].dist = dist;
    heap[i].v = v;
}

static HeapNode heap_pop(void) {
    HeapNode res = heap[1];
    HeapNode last = heap[heap_sz--];
    int i = 1;
    while (1) {
        int l = i * 2;
        int r = l + 1;
        if (l > heap_sz) {
            break;
        }
        int best = l;
        if (r <= heap_sz && heap[r].dist < heap[l].dist) {
            best = r;
        }
        if (heap[best].dist >= last.dist) {
            break;
        }
        heap[i] = heap[best];
        i = best;
    }
    heap[i] = last;
    return res;
}

int main(void) {
    int n, m, k;
    if (scanf("%d%d%d", &n, &m, &k) != 3) {
        return 0;
    }

    int qs[MAXK], qt[MAXK];
    for (int i = 0; i < k; ++i) {
        scanf("%d%d", &qs[i], &qt[i]);
    }

    memset(head, -1, sizeof(head));
    edge_cnt = 0;
    for (int i = 0; i < m; ++i) {
        int u, v, w;
        scanf("%d%d%d", &u, &v, &w);
        add_edge(u, v, w);
        add_edge(v, u, w);
    }

    for (int qi = 0; qi < k; ++qi) {
        int s = qs[qi], t = qt[qi];
        for (int i = 1; i <= n; ++i) {
            dist_[i] = (ll)4e18;
            parent_[i] = -1;
        }
        heap_sz = 0;
        dist_[s] = 0;
        heap_push(0, s);

        while (heap_sz > 0) {
            HeapNode cur = heap_pop();
            if (cur.dist != dist_[cur.v]) {
                continue;
            }
            if (cur.v == t) {
                break;
            }
            for (int e = head[cur.v]; e != -1; e = edges[e].next) {
                int to = edges[e].to;
                ll nd = cur.dist + edges[e].w;
                if (nd < dist_[to]) {
                    dist_[to] = nd;
                    parent_[to] = cur.v;
                    heap_push(nd, to);
                }
            }
        }

        if (dist_[t] == (ll)4e18) {
            printf("NO\n");
            continue;
        }

        int len = 0;
        for (int v = t; v != -1; v = parent_[v]) {
            path[len++] = v;
        }
        printf("YES %lld %d", dist_[t], len);
        for (int i = len - 1; i >= 0; --i) {
            printf(" %d", path[i]);
        }
        printf("\n");
    }

    return 0;
}
