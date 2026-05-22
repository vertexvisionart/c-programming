#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int n, m;
  scanf("%d %d", &n, &m);

  // Список смежности через массивы
  int *head = calloc(n + 1, sizeof(int));
  int *nxt = malloc((m + 1) * sizeof(int));
  int *to = malloc((m + 1) * sizeof(int));
  int *indeg = calloc(n + 1, sizeof(int));
  int edge_cnt = 0;

  for (int k = 0; k < m; k++) {
    int i, j;
    scanf("%d %d", &i, &j);
    // ребро i -> j (Xi < Xj)
    edge_cnt++;
    to[edge_cnt] = j;
    nxt[edge_cnt] = head[i];
    head[i] = edge_cnt;
    indeg[j]++;
  }

  // Очередь для алгоритма Кана
  int *queue = malloc((n + 1) * sizeof(int));
  int qhead = 0, qtail = 0;

  for (int v = 1; v <= n; v++)
    if (indeg[v] == 0)
      queue[qtail++] = v;

  int *order = malloc((n + 1) * sizeof(int)); // порядок обхода
  int cnt = 0;

  while (qhead < qtail) {
    int v = queue[qhead++];
    order[cnt++] = v;
    for (int e = head[v]; e; e = nxt[e]) {
      int u = to[e];
      if (--indeg[u] == 0)
        queue[qtail++] = u;
    }
  }

  if (cnt != n) {
    printf("NO\n");
  } else {
    // order[0] получает значение 1, order[1] — 2, ...
    int *ans = malloc((n + 1) * sizeof(int));
    for (int i = 0; i < n; i++)
      ans[order[i]] = i + 1;

    printf("YES\n");
    for (int v = 1; v <= n; v++)
      printf("%d%c", ans[v], v == n ? '\n' : ' ');

    free(ans);
  }

  free(head);
  free(nxt);
  free(to);
  free(indeg);
  free(queue);
  free(order);
  return 0;
}
