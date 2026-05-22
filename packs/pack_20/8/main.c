#include <stdio.h>
#include <string.h>

#define MAXN 2001
#define MAXM 6001

typedef long long ll;

#define INF 0x3f3f3f3f3f3f3f3fLL

/* Рёбра для Bellman-Ford */
int ef[MAXM], et[MAXM];
ll   ew[MAXM];

/* Список смежности для BFS-распространения «плохих» вершин */
int head[MAXN], nxt[MAXM], adj[MAXM], ecnt;

ll  dist[MAXN];
int bad[MAXN];          /* 1 = достижима через отрицательный цикл */
int bfs_q[MAXN];

void add(int u, int v) {
    adj[ecnt] = v;
    nxt[ecnt] = head[u];
    head[u]   = ecnt++;
}

int main(void) {
    int N, M, s;
    scanf("%d %d %d", &N, &M, &s);

    memset(head, -1, sizeof(head));
    ecnt = 0;

    for (int i = 0; i < M; i++) {
        scanf("%d %d %lld", &ef[i], &et[i], &ew[i]);
        add(ef[i], et[i]);
    }

    /* Инициализация */
    for (int i = 1; i <= N; i++) dist[i] = INF;
    dist[s] = 0;

    /* Bellman-Ford: N-1 итерация */
    for (int iter = 0; iter < N - 1; iter++) {
        int updated = 0;
        for (int i = 0; i < M; i++) {
            int u = ef[i], v = et[i];
            if (dist[u] == INF) continue;
            if (dist[u] + ew[i] < dist[v]) {
                dist[v] = dist[u] + ew[i];
                updated = 1;
            }
        }
        if (!updated) break;  /* оптимизация */
    }

    /* N-я итерация: найдём вершины, которые всё ещё уменьшаются */
    int qh = 0, qt = 0;
    for (int i = 0; i < M; i++) {
        int u = ef[i], v = et[i];
        if (dist[u] == INF) continue;
        if (dist[u] + ew[i] < dist[v]) {
            /* v достижима через отрицательный цикл */
            if (!bad[v]) {
                bad[v] = 1;
                bfs_q[qt++] = v;
            }
        }
    }

    /* BFS: распространяем «плохой» статус по направленным рёбрам */
    while (qh < qt) {
        int u = bfs_q[qh++];
        for (int e = head[u]; e != -1; e = nxt[e]) {
            int v = adj[e];
            if (!bad[v]) {
                bad[v] = 1;
                bfs_q[qt++] = v;
            }
        }
    }

    /* Вывод */
    for (int i = 1; i <= N; i++) {
        if (bad[i])          puts("-");
        else if (dist[i] == INF) puts("*");
        else                 printf("%lld\n", dist[i]);
    }

    return 0;
}