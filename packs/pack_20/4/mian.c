#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 5001
#define MAXM 50001
#define INF 0x3f3f3f3f3f3f3f3fLL

typedef long long ll;

// Список смежности
int head[MAXN], to[MAXM], nxt_e[MAXM], edge_cnt;
ll wt[MAXM];

void add_edge(int u, int v, ll w) {
  edge_cnt++;
  to[edge_cnt] = v;
  wt[edge_cnt] = w;
  nxt_e[edge_cnt] = head[u];
  head[u] = edge_cnt;
}

ll dist[MAXN];
int par[MAXN];  // предыдущая вершина на пути
int in_q[MAXN]; // в очереди?
int que[MAXN * 2];

void spfa(int src, int N) {
  for (int i = 1; i <= N; i++) {
    dist[i] = INF;
    par[i] = -1;
    in_q[i] = 0;
  }
  dist[src] = 0;

  int qh = 0, qt = 0;
  que[qt++] = src;
  in_q[src] = 1;

  while (qh != qt) {
    int v = que[qh++];
    if (qh == MAXN * 2)
      qh = 0;
    in_q[v] = 0;

    for (int e = head[v]; e; e = nxt_e[e]) {
      int u = to[e];
      ll w = wt[e];
      if (dist[v] + w < dist[u]) {
        dist[u] = dist[v] + w;
        par[u] = v;
        if (!in_q[u]) {
          que[qt++] = u;
          if (qt == MAXN * 2)
            qt = 0;
          in_q[u] = 1;
        }
      }
    }
  }
}

// Восстановление пути (рекурсивно для порядка)
int path[MAXN], path_len;

void build_path(int v) {
  if (par[v] == -1) {
    path[path_len++] = v;
    return;
  }
  build_path(par[v]);
  path[path_len++] = v;
}

int main(void) {
  int N, M, K;
  scanf("%d %d %d", &N, &M, &K);

  int friends[51];
  for (int i = 0; i < K; i++)
    scanf("%d", &friends[i]);

  for (int i = 0; i < M; i++) {
    int a, b;
    ll w;
    scanf("%d %d %lld", &a, &b, &w);
    add_edge(a, b, w);
  }

  spfa(1, N);

  for (int i = 0; i < K; i++) {
    int f = friends[i];
    printf("%lld ", dist[f]);

    path_len = 0;
    build_path(f);

    printf("%d", path_len);
    for (int j = 0; j < path_len; j++)
      printf(" %d", path[j]);
    printf("\n");
  }

  return 0;
}
