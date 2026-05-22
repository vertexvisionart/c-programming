#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 100001
#define MAXM 100001

// Прямой граф
int head[MAXN], to[MAXM], nxt_e[MAXM];
// Транспонированный граф
int rhead[MAXN], rto[MAXM], rnxt_e[MAXM];
int edge_cnt;

void add_edge(int u, int v) {
  edge_cnt++;
  to[edge_cnt] = v;
  nxt_e[edge_cnt] = head[u];
  head[u] = edge_cnt;

  rto[edge_cnt] = u;
  rnxt_e[edge_cnt] = rhead[v];
  rhead[v] = edge_cnt;
}

int visited[MAXN];
int order[MAXN], order_top; // порядок выхода из первого DFS
int comp[MAXN];             // номер КСС для каждой вершины

// Стек для итеративного DFS
int stk[MAXN], stk_e[MAXN], stk_top;

// DFS 1 (прямой граф): заполняем order по времени выхода
void dfs1(int start) {
  stk_top = 0;
  stk[stk_top] = start;
  stk_e[stk_top] = head[start];
  visited[start] = 1;
  stk_top++;

  while (stk_top > 0) {
    int v = stk[stk_top - 1];
    int e = stk_e[stk_top - 1];

    if (e == 0) {
      // Выходим из вершины — записываем в order
      order[order_top++] = v;
      stk_top--;
    } else {
      stk_e[stk_top - 1] = nxt_e[e];
      int u = to[e];
      if (!visited[u]) {
        visited[u] = 1;
        stk[stk_top] = u;
        stk_e[stk_top] = head[u];
        stk_top++;
      }
    }
  }
}

// DFS 2 (транспонированный граф): помечаем всю КСС
void dfs2(int start, int c) {
  stk_top = 0;
  stk[stk_top] = start;
  stk_e[stk_top] = rhead[start];
  comp[start] = c;
  stk_top++;

  while (stk_top > 0) {
    int v = stk[stk_top - 1];
    int e = stk_e[stk_top - 1];

    if (e == 0) {
      stk_top--;
    } else {
      stk_e[stk_top - 1] = rnxt_e[e];
      int u = rto[e];
      if (!comp[u]) {
        comp[u] = c;
        stk[stk_top] = u;
        stk_e[stk_top] = rhead[u];
        stk_top++;
      }
    }
  }
}

int main(void) {
  int N, M;
  scanf("%d %d", &N, &M);

  for (int i = 0; i < M; i++) {
    int u, v;
    scanf("%d %d", &u, &v);
    if (u != v) // петли не влияют на КСС
      add_edge(u, v);
  }

  // DFS 1: строим order (порядок по убыванию времени выхода)
  for (int v = 1; v <= N; v++)
    if (!visited[v])
      dfs1(v);

  // DFS 2: обходим в обратном порядке выхода по транспонированному графу
  int num_comp = 0;
  for (int i = order_top - 1; i >= 0; i--) {
    int v = order[i];
    if (!comp[v])
      dfs2(v, ++num_comp);
  }

  printf("%d\n", num_comp);
  for (int v = 1; v <= N; v++)
    printf("%d%c", comp[v], v == N ? '\n' : ' ');

  return 0;
}
