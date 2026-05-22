#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 100005
#define MAXM 200005


int head[MAXN], nxt[MAXM], to[MAXM], ecnt;
int indeg[MAXN];

void addEdge(int u, int v) {
    ecnt++;
    to[ecnt] = v;
    nxt[ecnt] = head[u];
    head[u] = ecnt;
    indeg[v]++;
}


int heap[MAXN], hsz;

void heapPush(int x) {
    heap[++hsz] = x;
    int i = hsz;
    while (i > 1 && heap[i] < heap[i/2]) {
        int t = heap[i]; heap[i] = heap[i/2]; heap[i/2] = t;
        i /= 2;
    }
}

int heapPop() {
    int ret = heap[1];
    heap[1] = heap[hsz--];
    int i = 1;
    while (1) {
        int s = i, l = 2*i, r = 2*i+1;
        if (l <= hsz && heap[l] < heap[s]) s = l;
        if (r <= hsz && heap[r] < heap[s]) s = r;
        if (s == i) break;
        int t = heap[i]; heap[i] = heap[s]; heap[s] = t;
        i = s;
    }
    return ret;
}

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    int N, M;
    scanf("%d %d", &N, &M);

    memset(head, 0, sizeof(head));
    memset(indeg, 0, sizeof(indeg));
    ecnt = 0;

    for (int i = 0; i < M; i++) {
        int b, a;
        scanf("%d %d", &b, &a);  
        addEdge(b, a);
    }


    hsz = 0;
    for (int i = 1; i <= N; i++)
        if (indeg[i] == 0)
            heapPush(i);

    int result[MAXN], rcnt = 0;

    while (hsz > 0) {
        int u = heapPop();
        result[rcnt++] = u;

        for (int e = head[u]; e; e = nxt[e]) {
            int v = to[e];
            indeg[v]--;
            if (indeg[v] == 0)
                heapPush(v);
        }
    }

    if (rcnt != N) {
        printf("bad course\n");
    } else {
        for (int i = 0; i < N; i++) {
            if (i) putchar(' ');
            printf("%d", result[i]);
        }
        putchar('\n');
    }

    return 0;
}

