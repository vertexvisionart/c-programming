/* DP-over-windows strategy: tries angle-sorted vs distance-sorted city
 * orderings, runs an O(N*W) DP per ordering, then keeps the better plan. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAXN 100005

int N, W;
double C, D;

typedef struct { double x, y, p; } City;

static City cities[MAXN];
static City sorted1[MAXN];
static City sorted2[MAXN];

static double dp[MAXN];
static int par_j[MAXN];
static int par_ret[MAXN];

typedef struct { int start, len, ret; } TourRef;
static TourRef trefs1[MAXN], trefs2[MAXN], kept[MAXN];

static double dist2(double x1, double y1, double x2, double y2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}
static double d0c(const City *c) { return sqrt(c->x*c->x + c->y*c->y); }

static int cmp_angle(const void *a, const void *b) {
    double da = atan2(((City*)a)->y, ((City*)a)->x);
    double db = atan2(((City*)b)->y, ((City*)b)->x);
    return (da > db) - (da < db);
}
static int cmp_dist(const void *a, const void *b) {
    double da = d0c((City*)a), db = d0c((City*)b);
    return (da > db) - (da < db);
}

/* Tour assumed already sorted by distance ascending. */
static double tour_cost_sorted(City *t, int m, int v0, int ret) {
    int ds = N/10;
    double tr = 0, px = 0, py = 0;
    for (int i = 0; i < m; i++) {
        double d = dist2(px, py, t[i].x, t[i].y);
        tr += d * (1.0 + C * (m - i));
        px = t[i].x; py = t[i].y;
    }
    if (ret) tr += dist2(px, py, 0, 0);
    double s = 0;
    for (int i = 0; i < m; i++) s += t[i].p * pow(D, (v0+i)/ds);
    return s - tr;
}

/* Insertion sort by distance — good for small windows. */
static City win_buf[MAXN];

static void isort_dist(City *a, int n) {
    for (int i = 1; i < n; i++) {
        City key = a[i]; double dk = d0c(&key);
        int j = i - 1;
        while (j >= 0 && d0c(&a[j]) > dk) { a[j+1] = a[j]; j--; }
        a[j+1] = key;
    }
}

static double run_dp(City *cs, int n, TourRef *trefs, int *ntours, int presorted) {
    int ww = (n < W) ? n : W;
    for (int i = 0; i <= n; i++) { dp[i] = -1e18; par_j[i] = -1; par_ret[i] = -1; }
    dp[0] = 0;

    for (int i = 0; i <= n; i++) {
        if (dp[i] < -1e17) continue;
        if (i < n && dp[i] > dp[i+1]) { dp[i+1] = dp[i]; par_j[i+1] = i; par_ret[i+1] = -1; }
        int jmax = (i+ww < n) ? i+ww : n;
        for (int j = i+1; j <= jmax; j++) {
            int m = j - i;
            City *win;
            if (presorted) {
                win = cs + i;
            } else {
                memcpy(win_buf, cs+i, m*sizeof(City));
                isort_dist(win_buf, m);
                win = win_buf;
            }
            double p = tour_cost_sorted(win, m, i, 0);
            if (dp[i] + p > dp[j]) { dp[j] = dp[i] + p; par_j[j] = i; par_ret[j] = 0; }
            double p2 = tour_cost_sorted(win, m, i, 1);
            if (dp[i] + p2 > dp[j]) { dp[j] = dp[i] + p2; par_j[j] = i; par_ret[j] = 1; }
        }
    }

    int best = 0;
    for (int i = 1; i <= n; i++) if (dp[i] > dp[best]) best = i;

    *ntours = 0;
    int i = best;
    while (i > 0) {
        int j = par_j[i], ret = par_ret[i];
        if (ret == -1) { i = j; continue; }
        trefs[*ntours].start = j; trefs[*ntours].len = i - j; trefs[*ntours].ret = ret;
        (*ntours)++;
        i = j;
    }
    for (int a = 0, b = *ntours-1; a < b; a++, b--) { TourRef tmp = trefs[a]; trefs[a] = trefs[b]; trefs[b] = tmp; }
    return dp[best];
}

static City *g_cs;
static double tour_sum_ref(TourRef *t) {
    double s = 0;
    for (int i = 0; i < t->len; i++) s += g_cs[t->start + i].p;
    return s;
}
static int cmp_tref_val(const void *a, const void *b) {
    double da = tour_sum_ref((TourRef*)a);
    double db = tour_sum_ref((TourRef*)b);
    return (da < db) - (da > db);
}

static void output_tours(City *best_cs, TourRef *best, int nb, int presorted) {
    int nk = 0, v0 = 0;
    for (int t = 0; t < nb; t++) {
        int m = best[t].len;
        City *win;
        if (presorted) {
            win = best_cs + best[t].start;
        } else {
            memcpy(win_buf, best_cs + best[t].start, m*sizeof(City));
            isort_dist(win_buf, m);
            win = win_buf;
        }
        int is_last = (t == nb - 1);
        double p = tour_cost_sorted(win, m, v0, !is_last);
        if (p > 0) { kept[nk++] = best[t]; v0 += m; }
    }
    if (nk > 0) { best = kept; nb = nk; }

    for (int t = 0; t < nb; t++) {
        int m = best[t].len;
        City *win;
        if (presorted) {
            win = best_cs + best[t].start;
        } else {
            memcpy(win_buf, best_cs + best[t].start, m*sizeof(City));
            isort_dist(win_buf, m);
            win = win_buf;
        }
        printf("%d %d %d\n", (int)win[0].x, (int)win[0].y, m);
        for (int k = 1; k < m; k++) printf("%d %d\n", (int)win[k].x, (int)win[k].y);
        if (t != nb - 1) printf("0 0\n");
    }
}

int main(void) {
    while (scanf("%d %lf %lf", &N, &C, &D) == 3) {
        for (int i = 0; i < N; i++) scanf("%lf %lf %lf", &cities[i].x, &cities[i].y, &cities[i].p);

        /* W chosen so O(N*W^2) ~ 5e7 */
        W = (int)sqrt(5e7 / N);
        if (W < 15) W = 15;
        if (W > N) W = N;

        memcpy(sorted1, cities, N*sizeof(City));
        qsort(sorted1, N, sizeof(City), cmp_angle);

        memcpy(sorted2, cities, N*sizeof(City));
        qsort(sorted2, N, sizeof(City), cmp_dist);

        int ntours1, ntours2;
        double p1 = run_dp(sorted1, N, trefs1, &ntours1, 0);
        double p2 = run_dp(sorted2, N, trefs2, &ntours2, 1);

        City *best_cs; TourRef *best; int nb; int presorted;
        if (p1 >= p2) { best_cs = sorted1; best = trefs1; nb = ntours1; presorted = 0; }
        else          { best_cs = sorted2; best = trefs2; nb = ntours2; presorted = 1; }

        g_cs = best_cs;
        qsort(best, nb, sizeof(TourRef), cmp_tref_val);

        output_tours(best_cs, best, nb, presorted);
    }
    return 0;
}
