#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD 1000000007LL

static int N, M, full_mask;
static int blocked[32];
static long long dp[256][256];
static long long new_dp[256][256];

/* 8 orientations of L-tetromino.
   Each piece is given as 4 cells (dr, dc) relative to its anchor.
   Anchor = topmost-leftmost cell (smallest row, then smallest col). */
static const int piece[8][4][2] = {
    {{0,0},{1,0},{2,0},{2,1}},     /* 1: tall L */
    {{0,0},{1,0},{2,-1},{2,0}},    /* 2: tall J */
    {{0,0},{0,1},{1,0},{2,0}},     /* 3: inverted tall L */
    {{0,0},{0,1},{1,1},{2,1}},     /* 4: inverted tall J */
    {{0,0},{1,0},{1,1},{1,2}},     /* 5: wide L */
    {{0,0},{1,-2},{1,-1},{1,0}},   /* 6: wide J */
    {{0,0},{0,1},{0,2},{1,0}},     /* 7: inverted wide L */
    {{0,0},{0,1},{0,2},{1,2}}      /* 8: inverted wide J */
};
static const int piece_h[8] = {3,3,3,3,2,2,2,2};

static int cur_row;

static void fill_row(int j, int r0, int r1, int r2, long long cnt) {
    while (j < M && (r0 & (1 << j))) j++;
    if (j == M) {
        if (r0 == full_mask)
            new_dp[r1][r2] = (new_dp[r1][r2] + cnt) % MOD;
        return;
    }
    for (int p = 0; p < 8; p++) {
        if (cur_row + piece_h[p] > N) continue;
        int nr0 = r0, nr1 = r1, nr2 = r2;
        int ok = 1;
        for (int k = 0; k < 4; k++) {
            int dr = piece[p][k][0];
            int dc = piece[p][k][1];
            int c = j + dc;
            if (c < 0 || c >= M) { ok = 0; break; }
            int bit = 1 << c;
            if (dr == 0) {
                if (nr0 & bit) { ok = 0; break; }
                nr0 |= bit;
            } else if (dr == 1) {
                if (nr1 & bit) { ok = 0; break; }
                nr1 |= bit;
            } else {
                if (nr2 & bit) { ok = 0; break; }
                nr2 |= bit;
            }
        }
        if (ok) fill_row(j + 1, nr0, nr1, nr2, cnt);
    }
}

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    int T;
    fscanf(fin, "%d", &T);
    while (T--) {
        fscanf(fin, "%d %d", &N, &M);
        full_mask = (1 << M) - 1;
        for (int i = 0; i < N; i++) {
            char s[32];
            fscanf(fin, "%s", s);
            blocked[i] = 0;
            for (int j = 0; j < M; j++)
                if (s[j] == '#') blocked[i] |= (1 << j);
        }

        memset(dp, 0, sizeof(dp));
        int init_b = (N >= 2) ? blocked[1] : 0;
        dp[blocked[0]][init_b] = 1;

        for (int i = 0; i < N; i++) {
            memset(new_dp, 0, sizeof(new_dp));
            cur_row = i;
            int r2_init = (i + 2 < N) ? blocked[i + 2] : 0;
            for (int a = 0; a <= full_mask; a++) {
                for (int b = 0; b <= full_mask; b++) {
                    long long cnt = dp[a][b];
                    if (cnt == 0) continue;
                    fill_row(0, a, b, r2_init, cnt);
                }
            }
            memcpy(dp, new_dp, sizeof(dp));
        }

        fprintf(fout, "%lld\n", dp[0][0]);
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
