#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int n, m;
  scanf("%d %d", &n, &m);

  int *head = calloc(n + 1, sizeof(int));
  int *nxt = malloc((m + 1) * sizeof(int));
  int *to = malloc((m + 1) * sizeof(int));
  int edge_cnt = 0;

  for (int k = 0; k < m; k++) {
    int u, v;
    scanf("%d %d", &u, &v);
    edge_cnt++;
    to[edge_cnt] = v;
    nxt[edge_cnt] = head[u];
    head[u] = edge_cnt;
  }

  // 0=белый, 1=серый (на стеке), 2=чёрный
  int *color = calloc(n + 1, sizeof(int));
  int *par = calloc(n + 1, sizeof(int)); // родитель в DFS-дереве

  // Итеративный DFS: стек (вершина, текущее ребро)
  int *stk_v = malloc((n + 1) * sizeof(int));
  int *stk_e = malloc((n + 1) * sizeof(int));
  int stk_top = 0;

  int cyc_start = -1, cyc_from = -1;

  for (int s = 1; s <= n && cyc_start == -1; s++) {
    if (color[s] != 0)
      continue;

    color[s] = 1;
    stk_v[stk_top] = s;
    stk_e[stk_top] = head[s];
    stk_top++;

    while (stk_top > 0 && cyc_start == -1) {
      int v = stk_v[stk_top - 1];
      int e = stk_e[stk_top - 1];

      if (e == 0) {
        // закончили вершину — красим в чёрный
        color[v] = 2;
        stk_top--;
      } else {
        stk_e[stk_top - 1] = nxt[e]; // переходим к следующему ребру
        int u = to[e];
        if (color[u] == 0) {
          par[u] = v;
          color[u] = 1;
          stk_v[stk_top] = u;
          stk_e[stk_top] = head[u];
          stk_top++;
        } else if (color[u] == 1) {
          // нашли ребро в серую вершину — цикл!
          cyc_start = u; // начало цикла (предок)
          cyc_from = v;  // откуда пришло обратное ребро
        }
      }
    }
  }

  if (cyc_start == -1) {
    printf("-1\n");
  } else {
    // Восстанавливаем цикл: идём по par[] от cyc_from до cyc_start
    // и собираем вершины в обратном порядке
    int len = 0;
    int cur = cyc_from;
    while (cur != cyc_start) {
      len++;
      cur = par[cur];
    }
    len++; // сама cyc_start

    int *cyc = malloc(len * sizeof(int));
    cur = cyc_from;
    for (int i = len - 1; i >= 1; i--) {
      cyc[i] = cur;
      cur = par[cur];
    }
    cyc[0] = cyc_start;

    printf("%d\n", len);
    for (int i = 0; i < len; i++)
      printf("%d%c", cyc[i], i == len - 1 ? '\n' : ' ');

    free(cyc);
  }

  free(head);
  free(nxt);
  free(to);
  free(color);
  free(par);
  free(stk_v);
  free(stk_e);
  return 0;
}
