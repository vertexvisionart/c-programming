#include "sol.h"
#include <stdlib.h>

#define MAXN 300005
#define MAXM 600005  

static int  N, M;
static Edge raw[MAXN];  
static Edge adj[MAXM];     
static int  head[MAXN];   
static int  cnt[MAXN];     

void init() {
    N = getVerticesCount();


    M = 0;
    Edge e;
    while (readEdge(&e))
        raw[M++] = e;



    for (int i = 0; i < N; i++) cnt[i] = 0;
    for (int i = 0; i < M; i++) {
        cnt[raw[i].from]++;
        cnt[raw[i].to]++;
    }


    head[0] = 0;
    for (int i = 1; i < N; i++)
        head[i] = head[i-1] + cnt[i-1];


    for (int i = 0; i < N; i++) cnt[i] = 0;


    for (int i = 0; i < M; i++) {
        int u = raw[i].from, v = raw[i].to, w = raw[i].weight;
        adj[head[u] + cnt[u]++] = (Edge){u, v, w};
        adj[head[v] + cnt[v]++] = (Edge){v, u, w};

    }
}

int getEdgesCount(int iVertex) {
    return cnt[iVertex];
}

Edge getIncidentEdge(int iVertex, int iIndex) {
    return adj[head[iVertex] + iIndex];
}







