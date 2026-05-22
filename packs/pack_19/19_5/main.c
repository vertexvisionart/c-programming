#include <stdio.h>
#include <string.h>

#define MAXN 105

int M, N;
char grid[MAXN][MAXN];
int dist[MAXN][MAXN];

int qr[MAXN * MAXN], qc[MAXN * MAXN];

int dr[] = {-1, 1, 0, 0};
int dc[] = {0, 0, -1, 1};

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    scanf("%d %d", &M, &N);
    for (int i = 0; i < M; i++)
        scanf("%s", grid[i]);

    /* Найдём S и F */
    int sr, sc, fr, fc;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == 'S') { sr = i; sc = j; }
            if (grid[i][j] == 'F') { fr = i; fc = j; }
        }

    /* BFS */
    memset(dist, -1, sizeof(dist));
    int head = 0, tail = 0;

    dist[sr][sc] = 0;
    qr[tail] = sr;
    qc[tail] = sc;
    tail++;

    while (head < tail) {
        int r = qr[head], c = qc[head];
        head++;

        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d];
            int nc = c + dc[d];
            if (nr < 0 || nr >= M || nc < 0 || nc >= N) continue;
            if (grid[nr][nc] == 'X') continue;
            if (dist[nr][nc] != -1) continue;
            dist[nr][nc] = dist[r][c] + 1;
            qr[tail] = nr;
            qc[tail] = nc;
            tail++;
        }
    }

    printf("%d\n", dist[fr][fc]);
    return 0;
}