#include <stdio.h>

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");

    int n, m;
    fscanf(fin, "%d %d", &n, &m);

    static int a[1000][1000];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            fscanf(fin, "%d", &a[i][j]);

    static int row_max[1000], row_max_col[1000], row_max_cnt[1000];
    for (int i = 0; i < n; i++) {
        row_max[i] = a[i][0];
        row_max_col[i] = 0;
        row_max_cnt[i] = 1;
        for (int j = 1; j < m; j++) {
            if (a[i][j] > row_max[i]) {
                row_max[i] = a[i][j];
                row_max_col[i] = j;
                row_max_cnt[i] = 1;
            } else if (a[i][j] == row_max[i]) {
                row_max_cnt[i]++;
            }
        }
    }

    static int col_min[1000], col_min_row[1000], col_min_cnt[1000];
    for (int j = 0; j < m; j++) {
        col_min[j] = a[0][j];
        col_min_row[j] = 0;
        col_min_cnt[j] = 1;
        for (int i = 1; i < n; i++) {
            if (a[i][j] < col_min[j]) {
                col_min[j] = a[i][j];
                col_min_row[j] = i;
                col_min_cnt[j] = 1;
            } else if (a[i][j] == col_min[j]) {
                col_min_cnt[j]++;
            }
        }
    }

    int rx = 0, ry = 0;
    for (int i = 0; i < n; i++) {
        if (row_max_cnt[i] != 1)
            continue;
        int j = row_max_col[i];
        if (col_min_cnt[j] == 1 && col_min_row[j] == i) {
            rx = i + 1;
            ry = j + 1;
            break;
        }
    }

    fprintf(fout, "%d %d\n", rx, ry);

    fclose(fin);
    fclose(fout);
    return 0;
}
