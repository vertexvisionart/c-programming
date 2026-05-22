#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXN 200005

int head[MAXN], nxt[MAXN], to[MAXN], ecnt;
int dist[MAXN];
int q[MAXN];

void add_edge(int u, int v) {
    ecnt++;
    to[ecnt] = v;
    nxt[ecnt] = head[u];
    head[u] = ecnt;
}

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    int N, M;
    scanf("%d %d", &N, &M);

    for (int i = 0; i < M; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        add_edge(u, v);
    }

    memset(dist, -1, sizeof(dist));
    dist[1] = 0;

    int qhead = 0, qtail = 0;
    q[qtail++] = 1;

    while (qhead < qtail) {
        int u = q[qhead++];
        for (int e = head[u]; e; e = nxt[e]) {
            int v = to[e];
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q[qtail++] = v;
            }
        }
    }

    for (int i = 1; i <= N; i++)
        printf("%d\n", dist[i]);

    return 0;
}