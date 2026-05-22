#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 1005

int N;
char grid[MAXN][MAXN];
int visited[MAXN][MAXN];

int qr[MAXN * MAXN], qc[MAXN * MAXN];
int qhead, qtail;

int dr[] = {-1, 1, 0, 0};
int dc[] = {0, 0, -1, 1};

void bfs(int r, int c) {
  qhead = qtail = 0;
  qr[qtail] = r;
  qc[qtail] = c;
  qtail++;
  visited[r][c] = 1;

  while (qhead < qtail) {
    int cr = qr[qhead];
    int cc = qc[qhead];
    qhead++;

    for (int d = 0; d < 4; d++) {
      int nr = cr + dr[d];
      int nc = cc + dc[d];
      if (nr >= 0 && nr < N && nc >= 0 && nc < N && !visited[nr][nc] &&
          grid[nr][nc] == ' ') {
        visited[nr][nc] = 1;
        qr[qtail] = nr;
        qc[qtail] = nc;
        qtail++;
      }
    }
  }
}

int main() {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  scanf("%d\n", &N);
  for (int i = 0; i < N; i++) {

    fgets(grid[i], MAXN, stdin);

    int len = strlen(grid[i]);
    if (len > 0 && grid[i][len - 1] == '\n')
      grid[i][len - 1] = '\0';

    for (int j = strlen(grid[i]); j < N; j++)
      grid[i][j] = '*';
  }

  int startCol = -1;
  for (int j = 0; j < N; j++) {
    if (grid[0][j] == ' ') {
      startCol = j;
      break;
    }
  }

  memset(visited, 0, sizeof(visited));

  bfs(0, startCol);

  int result = 0;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (grid[i][j] == ' ' && !visited[i][j]) {
        bfs(i, j);
        result++;
      }
    }
  }

  printf("%d\n", result);
  return 0;
}
