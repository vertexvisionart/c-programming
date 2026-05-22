#include <stdio.h>

char maze[1000][1002];
int visited[1000][1000];

void search(int x, int y, int N) {
    visited[x][y] = 1;
    if(x-1 >= 0 && maze[x-1][y] == ' ' && !visited[x-1][y]) {
        search(x-1, y, N);
    }
    if(x+1 < N && maze[x+1][y] == ' ' && !visited[x+1][y]) {
        search(x+1, y, N);
    }
    if(y-1 >= 0 && maze[x][y-1] == ' ' && !visited[x][y-1]) {
        search(x, y-1, N);
    }
    if(y+1 < N && maze[x][y+1] == ' ' && !visited[x][y+1]) {
        search(x, y+1, N);
    }
}

int main(void) {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    
    int N;
    scanf("%d", &N);
    getc(stdin);
    
    for(int i = 0; i < N; ++i) {
        fgets(maze[i], 1002, stdin);
    }

    int entry = 0;
    for(int i = 0; i < N; ++i) {
        if (maze[0][i] == ' ') entry = i;
    }

    search(0, entry, N);

    int answer = 0;
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            if(maze[i][j] == ' ' && !visited[i][j]) {
                answer++;
                search(i, j, N);
            }
        }
    }
    printf("%d", answer);
    return 0;
}