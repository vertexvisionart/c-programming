#include <stdio.h>

int V, m, adj[10][10];
int color[10];
int found;

void solve(int v) {
    if (found) return;
    if (v == V) { found = 1; return; }
    for (int c = 1; c <= m && !found; c++) {
        int ok = 1;
        for (int u = 0; u < v && ok; u++)
            if (adj[v][u] && color[u] == c) ok = 0;
        if (ok) {
            color[v] = c;
            solve(v + 1);
            color[v] = 0;
        }
    }
}

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    fscanf(fin, "%d%d", &m, &V);
    int u, v;
    while (fscanf(fin, "%d%d", &u, &v) == 2) {
        adj[u][v] = adj[v][u] = 1;
    }
    fclose(fin);
    found = 0;
    solve(0);
    fprintf(fout, "%d\n", found);
    fclose(fout);
    return 0;
}
