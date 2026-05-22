#include <stdio.h>
#include <stdlib.h>

int n, m;
char board[20][20];
int col[20];
int found = 0;

int is_safe(int row, int c) {
    if (board[row][c] == '.') return 0;

    for (int i = 0; i < row; i++) {
        if (col[i] == c || abs(col[i] - c) == abs(i - row)) {
            return 0;
        }
    }
    return 1;
}

void solve(int row) {
    if (found) return;

    if (row == n) {
        found = 1;
        return;
    }

    for (int c = 0; c < m; c++) {
        if (is_safe(row, c)) {
            col[row] = c;
            solve(row + 1);
            if (found) return;
        }
    }
}

int main() {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");

    fscanf(fin, "%d %d", &n, &m);

    for (int i = 0; i < n; i++) {
        fscanf(fin, "%s", board[i]);
    }

    solve(0);

    if (found) {
        fprintf(fout, "YES\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (col[i] == j) {
                    fprintf(fout, "X");
                } else {
                    fprintf(fout, ".");
                }
            }
            fprintf(fout, "\n");
        }
    } else {
        fprintf(fout, "NO\n");
    }

    fclose(fin);
    fclose(fout);

    return 0;
}
