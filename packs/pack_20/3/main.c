#include <stdio.h>
#include <stdlib.h>

#define MAXN 305
#define INF 0x3f3f3f3f3f3f3f3fLL

typedef long long ll;

ll dist[MAXN][MAXN];
int nxt[MAXN][MAXN]; // следующая вершина на пути i→j

int main(void) {
  int N, M, P, K;
  scanf("%d %d %d %d", &N, &M, &P, &K);

  // Инициализация
  for (int i = 1; i <= N; i++)
    for (int j = 1; j <= N; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      nxt[i][j] = -1;
    }

  for (int k = 0; k < M; k++) {
    int u, v;
    ll w;
    scanf("%d %d %lld", &u, &v, &w);
    if (w < dist[u][v]) {
      dist[u][v] = w;
      dist[v][u] = w;
      nxt[u][v] = v;
      nxt[v][u] = u;
    }
  }

  // Floyd-Warshall
  for (int mid = 1; mid <= N; mid++)
    for (int i = 1; i <= N; i++) {
      if (dist[i][mid] == INF)
        continue;
      for (int j = 1; j <= N; j++) {
        if (dist[i][mid] + dist[mid][j] < dist[i][j]) {
          dist[i][j] = dist[i][mid] + dist[mid][j];
          nxt[i][j] = nxt[i][mid];
        }
      }
    }

  // P запросов с маршрутом
  for (int q = 0; q < P; q++) {
    int s, t;
    scanf("%d %d", &s, &t);

    printf("%lld ", dist[s][t]);

    // Считаем длину пути
    int len = 1;
    int cur = s;
    while (cur != t) {
      cur = nxt[cur][t];
      len++;
    }

    printf("%d", len);
    cur = s;
    while (1) {
      printf(" %d", cur);
      if (cur == t)
        break;
      cur = nxt[cur][t];
    }
    printf("\n");
  }

  // K запросов только время
  for (int q = 0; q < K; q++) {
    int s, t;
    scanf("%d %d", &s, &t);
    printf("%lld\n", dist[s][t]);
  }

  return 0;
}
