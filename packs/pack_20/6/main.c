#include <stdio.h>
#include <stdlib.h>

#define MAXN 100001
#define MAXM 200001 // каждое ребро добавляем дважды (неориентированный)

int head[MAXN], to[MAXM], nxt_e[MAXM], eid[MAXM]; // eid = номер исходного ребра
int edge_cnt;

void add_edge(int u, int v, int id) {
  edge_cnt++;
  to[edge_cnt] = v;
  eid[edge_cnt] = id;
  nxt_e[edge_cnt] = head[u];
  head[u] = edge_cnt;
}

int tin[MAXN], low[MAXN], timer_val;
int visited[MAXN];
int is_bridge[MAXN]; // по номеру исходного ребра

// Итеративный DFS
int stk_v[MAXN], stk_e[MAXN], stk_par_eid[MAXN];
int stk_top;

void dfs(int start) {
  stk_top = 0;
  stk_v[stk_top] = start;
  stk_e[stk_top] = head[start];
  stk_par_eid[stk_top] = -1;
  visited[start] = 1;
  tin[start] = low[start] = timer_val++;
  stk_top++;

  while (stk_top > 0) {
    int v = stk_v[stk_top - 1];
    int e = stk_e[stk_top - 1];
    int pe = stk_par_eid[stk_top - 1]; // id ребра, по которому пришли в v

    if (e == 0) {
      // Выходим из v — обновляем low родителя
      stk_top--;
      if (stk_top > 0) {
        int par = stk_v[stk_top - 1];
        if (low[v] < low[par])
          low[par] = low[v];
        // Проверяем: ребро par→v — мост?
        if (low[v] > tin[par])
          is_bridge[pe] = 1;
      }
    } else {
      stk_e[stk_top - 1] = nxt_e[e]; // переходим к следующему ребру
      int u = to[e];
      int ei = eid[e];

      if (ei == pe)
        continue; // то же ребро (но не кратное!) — пропускаем

      if (!visited[u]) {
        visited[u] = 1;
        tin[u] = low[u] = timer_val++;
        stk_v[stk_top] = u;
        stk_e[stk_top] = head[u];
        stk_par_eid[stk_top] = ei;
        stk_top++;
      } else {
        // back edge: обновляем low[v]
        if (tin[u] < low[v])
          low[v] = tin[u];
      }
    }
  }
}

int main(void) {
  int N, M;
  scanf("%d %d", &N, &M);

  for (int i = 1; i <= M; i++) {
    int u, v;
    scanf("%d %d", &u, &v);
    if (u == v)
      continue; // петля — не мост
    add_edge(u, v, i);
    add_edge(v, u, i);
  }

  for (int v = 1; v <= N; v++)
    if (!visited[v])
      dfs(v);

  // Собираем ответ
  int cnt = 0;
  for (int i = 1; i <= M; i++)
    if (is_bridge[i])
      cnt++;

  printf("%d\n", cnt);
  int first = 1;
  for (int i = 1; i <= M; i++)
    if (is_bridge[i]) {
      if (!first)
        printf(" ");
      printf("%d", i);
      first = 0;
    }
  if (cnt > 0)
    printf("\n");

  return 0;
}
