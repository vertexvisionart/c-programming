/* Greedy strategy: tours grow by picking the best (profit-vs-distance)
 * city from a search prefix; each completed tour returns to origin. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAXN 100005

int N;
double C, D;

typedef struct { double x, y, p; int id; } City;

static City cities[MAXN];
static int used[MAXN];

static double dist2(double x1, double y1, double x2, double y2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

static int cmp_profit_per_dist(const void *a, const void *b) {
    City *ca = (City*)a, *cb = (City*)b;
    double da = sqrt(ca->x*ca->x + ca->y*ca->y);
    double db = sqrt(cb->x*cb->x + cb->y*cb->y);
    if (da < 1) da = 1;
    if (db < 1) db = 1;
    double ra = ca->p / da;
    double rb = cb->p / db;
    return (ra < rb) - (ra > rb);
}

int main(void) {
    while (scanf("%d %lf %lf", &N, &C, &D) == 3) {
        for (int i = 0; i < N; i++) {
            scanf("%lf %lf %lf", &cities[i].x, &cities[i].y, &cities[i].p);
            cities[i].id = i;
            used[i] = 0;
        }

        qsort(cities, N, sizeof(City), cmp_profit_per_dist);

        int v0 = 0;
        int ds = N/10;

        for (int start = 0; start < N; start++) {
            if (used[cities[start].id]) continue;

            int tour[MAXN];
            int tour_len = 0;
            tour[tour_len++] = start;
            used[cities[start].id] = 1;

            double px = cities[start].x, py = cities[start].y;

            int max_tour_size = (N <= 30000) ? 50 : 40;
            for (int sz = 1; sz < max_tour_size && tour_len < N; sz++) {
                int best = -1;
                double best_score = -1e18;

                int search_limit = (N < 10000) ? N : 10000;
                for (int i = 0; i < search_limit; i++) {
                    if (used[cities[i].id]) continue;
                    double d = dist2(px, py, cities[i].x, cities[i].y);
                    double score = cities[i].p * pow(D, (v0+tour_len)/ds) - d*(1.0+C*sz);
                    if (score > best_score) {
                        best_score = score;
                        best = i;
                    }
                }

                if (best == -1) break;
                if (best_score < 0 && tour_len >= 2) break;

                tour[tour_len++] = best;
                used[cities[best].id] = 1;
                px = cities[best].x;
                py = cities[best].y;
            }

            if (tour_len > 0) {
                City sorted_tour[MAXN];
                for (int i = 0; i < tour_len; i++) sorted_tour[i] = cities[tour[i]];

                for (int i = 0; i < tour_len; i++) {
                    for (int j = i+1; j < tour_len; j++) {
                        double di = sqrt(sorted_tour[i].x*sorted_tour[i].x + sorted_tour[i].y*sorted_tour[i].y);
                        double dj = sqrt(sorted_tour[j].x*sorted_tour[j].x + sorted_tour[j].y*sorted_tour[j].y);
                        if (di > dj) {
                            City tmp = sorted_tour[i];
                            sorted_tour[i] = sorted_tour[j];
                            sorted_tour[j] = tmp;
                        }
                    }
                }

                printf("%d %d %d\n", (int)sorted_tour[0].x, (int)sorted_tour[0].y, tour_len);
                for (int i = 1; i < tour_len; i++) {
                    printf("%d %d\n", (int)sorted_tour[i].x, (int)sorted_tour[i].y);
                }
                printf("0 0\n");

                v0 += tour_len;
            }
        }
    }
    return 0;
}
