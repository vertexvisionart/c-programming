#include <stdio.h>
#include <string.h>

char grid[10][12];
char words[10][12];
int nwords, used[10], found;
char result[10][12];

typedef struct { int r, c, dr, dc, len; } Slot;
Slot slots[20];
int nslots;

void findSlots(void) {
    nslots = 0;
    for (int r = 0; r < 10; r++) {
        int c = 0;
        while (c < 10) {
            if (grid[r][c] == '-') {
                int s = c;
                while (c < 10 && grid[r][c] == '-') c++;
                if (c - s >= 2) {
                    slots[nslots++] = (Slot){r, s, 0, 1, c - s};
                }
            } else c++;
        }
    }
    for (int c = 0; c < 10; c++) {
        int r = 0;
        while (r < 10) {
            if (grid[r][c] == '-') {
                int s = r;
                while (r < 10 && grid[r][c] == '-') r++;
                if (r - s >= 2) {
                    slots[nslots++] = (Slot){s, c, 1, 0, r - s};
                }
            } else r++;
        }
    }
}

void solve(int si) {
    if (found) return;
    if (si == nslots) {
        found = 1;
        memcpy(result, grid, sizeof(grid));
        return;
    }
    Slot *s = &slots[si];
    for (int w = 0; w < nwords && !found; w++) {
        if (used[w] || (int)strlen(words[w]) != s->len) continue;
        int ok = 1;
        for (int k = 0; k < s->len && ok; k++) {
            char ch = grid[s->r + k * s->dr][s->c + k * s->dc];
            if (ch != '-' && ch != words[w][k]) ok = 0;
        }
        if (!ok) continue;
        char saved[11];
        for (int k = 0; k < s->len; k++) {
            int rr = s->r + k * s->dr, cc = s->c + k * s->dc;
            saved[k] = grid[rr][cc];
            grid[rr][cc] = words[w][k];
        }
        used[w] = 1;
        solve(si + 1);
        used[w] = 0;
        for (int k = 0; k < s->len; k++)
            grid[s->r + k * s->dr][s->c + k * s->dc] = saved[k];
    }
}

int main(void) {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");
    for (int i = 0; i < 10; i++) fscanf(fin, "%s", grid[i]);
    char line[256];
    fscanf(fin, "%s", line);
    fclose(fin);
    nwords = 0;
    char *tok = strtok(line, ";");
    while (tok) { strcpy(words[nwords++], tok); tok = strtok(NULL, ";"); }
    findSlots();
    found = 0;
    solve(0);
    for (int i = 0; i < 10; i++) fprintf(fout, "%s\n", result[i]);
    fclose(fout);
    return 0;
}
