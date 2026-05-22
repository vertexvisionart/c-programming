#include <stdio.h>
#include <string.h>

#define MAXN 10
#define MAXK 10
#define MAXM 100

int N, K, M, L;
int eff[MAXN][MAXK][MAXM];
int volt[MAXM];
int choice[MAXN];
int found;
int minSuf[MAXN + 1][MAXM];
int maxSuf[MAXN + 1][MAXM];

void solve(int c) {
    if (found) return;
    if (c == N) {
        found = 1;
        return;
    }
    for (int j = 0; j < K && !found; j++) {
        for (int t = 0; t < M; t++)
            volt[t] += eff[c][j][t];

        int ok = 1;
        for (int t = 0; t < M && ok; t++) {
            int lo = volt[t] + minSuf[c + 1][t];
            int hi = volt[t] + maxSuf[c + 1][t];
            if (lo > L || hi < L) ok = 0;
        }

        if (ok) {
            choice[c] = j;
            solve(c + 1);
        }

        for (int t = 0; t < M; t++)
            volt[t] -= eff[c][j][t];
    }
}

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");

    fscanf(fin, "%d%d%d%d", &N, &K, &M, &L);

    char line[200];
    for (int i = 0; i < N; i++)
        for (int j = 0; j < K; j++) {
            fscanf(fin, "%s", line);
            for (int t = 0; t < M; t++)
                eff[i][j][t] = (line[t] == 'X');
        }
    fclose(fin);

    memset(minSuf[N], 0, sizeof(int) * M);
    memset(maxSuf[N], 0, sizeof(int) * M);
    for (int i = N - 1; i >= 0; i--)
        for (int t = 0; t < M; t++) {
            int mn = 1, mx = 0;
            for (int j = 0; j < K; j++) {
                if (eff[i][j][t] < mn) mn = eff[i][j][t];
                if (eff[i][j][t] > mx) mx = eff[i][j][t];
            }
            minSuf[i][t] = minSuf[i + 1][t] + mn;
            maxSuf[i][t] = maxSuf[i + 1][t] + mx;
        }

    memset(volt, 0, sizeof(volt));
    found = 0;
    solve(0);

    if (found) {
        fprintf(fout, "YES\n");
        for (int i = 0; i < N; i++)
            fprintf(fout, "%d\n", choice[i] + 1);
    } else {
        fprintf(fout, "NO\n");
    }
    fclose(fout);
    return 0;
}
