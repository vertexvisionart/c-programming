#include <stdio.h>
#include <string.h>

enum { MAXA = 105, MAXB = 105, MAXM = 10005 };

static int head[MAXA];
static int to[MAXM];
static int next_[MAXM];
static int edge_cnt;
static int match_b[MAXB];
static int used[MAXA];
static int a_cnt, b_cnt;

static void add_edge(int u, int v) {
    to[edge_cnt] = v;
    next_[edge_cnt] = head[u];
    head[u] = edge_cnt++;
}

static int kuhn(int v) {
    if (used[v]) {
        return 0;
    }
    used[v] = 1;
    for (int e = head[v]; e != -1; e = next_[e]) {
        int u = to[e];
        if (match_b[u] == 0 || kuhn(match_b[u])) {
            match_b[u] = v;
            return 1;
        }
    }
    return 0;
}

int main(void) {
    int m;
    if (scanf("%d%d%d", &a_cnt, &b_cnt, &m) != 3) {
        return 0;
    }

    memset(head, -1, sizeof(head));
    edge_cnt = 0;
    for (int i = 0; i < m; ++i) {
        int x, y;
        scanf("%d%d", &x, &y);
        add_edge(x, y);
    }

    int ans = 0;
    for (int v = 1; v <= a_cnt; ++v) {
        memset(used, 0, sizeof(used));
        ans += kuhn(v);
    }

    printf("%d\n", ans);
    for (int task = 1; task <= b_cnt; ++task) {
        if (match_b[task] != 0) {
            printf("%d %d\n", match_b[task], task);
        }
    }
    return 0;
}
