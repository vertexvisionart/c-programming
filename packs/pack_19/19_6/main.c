#include <stdio.h>
#include <string.h>

#define MAXN 5005
#define MAXM 100005

int head[MAXN], nxt[MAXM], to[MAXM], ecnt;
int indeg[MAXN];
int result[MAXN];
int N, M;

void addEdge(int u, int v) {
  ecnt++;
  to[ecnt] = v;
  nxt[ecnt] = head[u];
  head[u] = ecnt;
}

int heap[MAXN], hsz;

void pushHeap(int x) {
  heap[++hsz] = x;
  int i = hsz;
  while (i > 1 && heap[i] < heap[i / 2]) {
    int t = heap[i];
    heap[i] = heap[i / 2];
    heap[i / 2] = t;
    i /= 2;
  }
}

int popHeap() {
  int res = heap[1];
  heap[1] = heap[hsz--];
  int i = 1;
  while (1) {
    int smallest = i;
    if (2 * i <= hsz && heap[2 * i] < heap[smallest])
      smallest = 2 * i;
    if (2 * i + 1 <= hsz && heap[2 * i + 1] < heap[smallest])
      smallest = 2 * i + 1;
    if (smallest == i)
      break;
    int t = heap[i];
    heap[i] = heap[smallest];
    heap[smallest] = t;
    i = smallest;
  }
  return res;
}

int main() {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  scanf("%d %d", &N, &M);
  memset(head, 0, sizeof(head));
  memset(indeg, 0, sizeof(indeg));
  ecnt = 0;
  hsz = 0;

  for (int i = 0; i < M; i++) {
    int b, a;
    scanf("%d %d", &b, &a);
    addEdge(b, a);
    indeg[a]++;
  }

  for (int i = 1; i <= N; i++)
    if (indeg[i] == 0)
      pushHeap(i);

  int cnt = 0;
  while (hsz > 0) {
    int v = popHeap();
    result[cnt++] = v;

    for (int e = head[v]; e; e = nxt[e]) {
      indeg[to[e]]--;
      if (indeg[to[e]] == 0)
        pushHeap(to[e]);
    }
  }

  if (cnt < N) {
    printf("bad course\n");
  } else {
    for (int i = 0; i < N; i++) {
      if (i > 0)
        putchar(' ');
      printf("%d", result[i]);
    }
    putchar('\n');
  }

  return 0;
}
