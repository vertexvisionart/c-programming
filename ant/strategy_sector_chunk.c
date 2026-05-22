/* Sector-based chunking heuristic: partitions cities by polar angle,
 * then greedily extracts contiguous profitable trips per sector. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAXN 100005
#define MAX_KNN 64

int N;
double C, D;

typedef struct { int x, y; double p; int id; } City;
typedef struct { int idx, bucket; double dist0; } SectorCity;
typedef struct { int start, len; double profit; } Chunk;

static City cities[MAXN];
static SectorCity sector[MAXN];
static Chunk chunks[MAXN];
static int used[MAXN];

static double dist2(int x1, int y1, int x2, int y2) {
    double dx = x1 - x2, dy = y1 - y2;
    return sqrt(dx*dx + dy*dy);
}

static int cmp_sector(const void *a, const void *b) {
    SectorCity *sa = (SectorCity*)a, *sb = (SectorCity*)b;
    if (sa->bucket != sb->bucket) return sa->bucket - sb->bucket;
    return (sa->dist0 > sb->dist0) - (sa->dist0 < sb->dist0);
}

static int cmp_chunk(const void *a, const void *b) {
    double pa = ((Chunk*)a)->profit, pb = ((Chunk*)b)->profit;
    return (pa < pb) - (pa > pb);
}

static double chunk_profit(int *trip, int len, int sold) {
    int block = N/10;
    if (block < 1) block = 1;
    double rev = 0, cost = 0;
    int cx = 0, cy = 0, cargo = len;

    for (int i = 0; i < len; i++) {
        int idx = trip[i];
        double seg = dist2(cx, cy, cities[idx].x, cities[idx].y);
        cost += seg * (1.0 + C * cargo);
        cx = cities[idx].x; cy = cities[idx].y;

        int band = (sold + i) / block;
        if (band > 9) band = 9;
        rev += cities[idx].p * pow(D, band);
        cargo--;
    }
    cost += dist2(cx, cy, 0, 0);
    return rev - cost;
}

int main(void) {
    while (scanf("%d %lf %lf", &N, &C, &D) == 3) {
        for (int i = 0; i < N; i++) {
            scanf("%d %d %lf", &cities[i].x, &cities[i].y, &cities[i].p);
            cities[i].id = i;
            used[i] = 0;
        }

        int max_trip = (C > 3.0) ? 8 : (C > 2.0) ? 12 : (C > 1.0) ? 20 : 35;
        int sectors = (N < 5000) ? 64 : (N < 20000) ? 256 : 384;

        int scount = 0;
        for (int i = 0; i < N; i++) {
            double d0 = dist2(0, 0, cities[i].x, cities[i].y);
            double solo = cities[i].p - d0 * (1.0 + C);
            if (solo <= -0.5 * d0) continue;

            double ang = atan2(cities[i].y, cities[i].x);
            double norm = (ang + M_PI) / (2.0 * M_PI);
            int bucket = (int)(norm * sectors);
            if (bucket < 0) bucket = 0;
            if (bucket >= sectors) bucket = sectors - 1;

            sector[scount].idx = i;
            sector[scount].bucket = bucket;
            sector[scount].dist0 = d0;
            scount++;
        }

        qsort(sector, scount, sizeof(SectorCity), cmp_sector);

        int chunk_count = 0, pos = 0;
        while (pos < scount) {
            int end = pos + 1;
            while (end < scount && sector[end].bucket == sector[pos].bucket) end++;

            int p = pos;
            while (p < end) {
                double best_profit = 1e-12;
                int best_len = 0;
                double path_len = 0, route_cost = 0, route_rev = 0;
                int prevx = 0, prevy = 0;

                int limit = end - p;
                if (limit > max_trip) limit = max_trip;

                for (int len = 1; len <= limit; len++) {
                    int idx = sector[p + len - 1].idx;
                    double seg = dist2(prevx, prevy, cities[idx].x, cities[idx].y);
                    route_cost += C * path_len + seg * (1.0 + C);
                    path_len += seg;
                    route_rev += cities[idx].p;
                    prevx = cities[idx].x;
                    prevy = cities[idx].y;

                    double full = route_rev - route_cost - dist2(prevx, prevy, 0, 0);
                    if (full > best_profit) {
                        best_profit = full;
                        best_len = len;
                    }
                }

                if (best_len > 0) {
                    chunks[chunk_count].start = p;
                    chunks[chunk_count].len = best_len;
                    chunks[chunk_count].profit = best_profit;
                    chunk_count++;
                    p += best_len;
                } else {
                    p++;
                }
            }
            pos = end;
        }

        qsort(chunks, chunk_count, sizeof(Chunk), cmp_chunk);

        int sold = 0;
        for (int i = 0; i < chunk_count; i++) {
            int trip[1024];
            int max_len = chunks[i].len;
            if (max_len > 1024) max_len = 1024;

            for (int j = 0; j < max_len; j++) {
                trip[j] = sector[chunks[i].start + j].idx;
            }

            double profit = chunk_profit(trip, max_len, sold);
            if (profit <= 1e-12) continue;

            printf("%d %d %d\n", cities[trip[0]].x, cities[trip[0]].y, max_len);
            for (int j = 1; j < max_len; j++) {
                printf("%d %d\n", cities[trip[j]].x, cities[trip[j]].y);
            }
            if (i != chunk_count - 1) printf("0 0\n");

            sold += max_len;
        }
    }
    return 0;
}
