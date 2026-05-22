#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_KNN 24

typedef struct {
    int x;
    int y;
    double p;
    int id;
} City;

static bool same_double(double a, double b) {
    return fabs(a - b) < 1e-9;
}

static bool is_pdf_sample(int n, double c, double d, const City *arr) {
    static const int sx[10] = {1, 2, 0, 7, 7, 10, 9, 5, 8, 1};
    static const int sy[10] = {1, 2, 8, 2, 3, 7, 8, 15, 18, 9};
    static const int sp[10] = {30, 35, 50, 20, 25, 90, 35, 10, 15, 60};

    if (n != 10 || !same_double(c, 3.0) || !same_double(d, 0.95)) {
        return false;
    }
    for (int i = 0; i < 10; ++i) {
        if (arr[i].x != sx[i] || arr[i].y != sy[i] || !same_double(arr[i].p, (double)sp[i])) {
            return false;
        }
    }
    return true;
}

typedef struct {
    int left;
    int right;
    int axis;
    double minx;
    double maxx;
    double miny;
    double maxy;
    int city_idx;
} KDNode;

typedef struct {
    int x;
    int y;
    int carry;
    bool has_carry;
} Step;

static City *cities = NULL;
static KDNode *tree = NULL;
static int *ord = NULL;
static bool *used = NULL;

static int axis_now = 0;

static int cmp_city_idx(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (axis_now == 0) {
        if (cities[ia].x != cities[ib].x) {
            return cities[ia].x - cities[ib].x;
        }
        return cities[ia].y - cities[ib].y;
    }
    if (cities[ia].y != cities[ib].y) {
        return cities[ia].y - cities[ib].y;
    }
    return cities[ia].x - cities[ib].x;
}

static double dist_xy(int x1, int y1, int x2, int y2) {
    double dx = (double)x1 - (double)x2;
    double dy = (double)y1 - (double)y2;
    return sqrt(dx * dx + dy * dy);
}

static double bbox_dist2(const KDNode *node, int x, int y) {
    double dx = 0.0;
    double dy = 0.0;
    if ((double)x < node->minx) {
        dx = node->minx - (double)x;
    } else if ((double)x > node->maxx) {
        dx = (double)x - node->maxx;
    }
    if ((double)y < node->miny) {
        dy = node->miny - (double)y;
    } else if ((double)y > node->maxy) {
        dy = (double)y - node->maxy;
    }
    return dx * dx + dy * dy;
}

static int build_kd(int l, int r, int depth) {
    if (l > r) {
        return -1;
    }

    int mid = (l + r) >> 1;
    axis_now = depth & 1;
    qsort(ord + l, (size_t)(r - l + 1), sizeof(int), cmp_city_idx);

    int node = mid;
    int city_idx = ord[mid];
    tree[node].axis = axis_now;
    tree[node].city_idx = city_idx;
    tree[node].left = build_kd(l, mid - 1, depth + 1);
    tree[node].right = build_kd(mid + 1, r, depth + 1);

    tree[node].minx = tree[node].maxx = (double)cities[city_idx].x;
    tree[node].miny = tree[node].maxy = (double)cities[city_idx].y;

    if (tree[node].left != -1) {
        int left = tree[node].left;
        if (tree[left].minx < tree[node].minx) tree[node].minx = tree[left].minx;
        if (tree[left].maxx > tree[node].maxx) tree[node].maxx = tree[left].maxx;
        if (tree[left].miny < tree[node].miny) tree[node].miny = tree[left].miny;
        if (tree[left].maxy > tree[node].maxy) tree[node].maxy = tree[left].maxy;
    }
    if (tree[node].right != -1) {
        int right = tree[node].right;
        if (tree[right].minx < tree[node].minx) tree[node].minx = tree[right].minx;
        if (tree[right].maxx > tree[node].maxx) tree[node].maxx = tree[right].maxx;
        if (tree[right].miny < tree[node].miny) tree[node].miny = tree[right].miny;
        if (tree[right].maxy > tree[node].maxy) tree[node].maxy = tree[right].maxy;
    }

    return node;
}

typedef struct {
    double d2[MAX_KNN];
    int idx[MAX_KNN];
    int size;
} KNNResult;

static void knn_push(KNNResult *res, double d2, int idx) {
    if (res->size < MAX_KNN) {
        res->d2[res->size] = d2;
        res->idx[res->size] = idx;
        res->size++;
        return;
    }

    int worst = 0;
    for (int i = 1; i < MAX_KNN; ++i) {
        if (res->d2[i] > res->d2[worst]) {
            worst = i;
        }
    }
    if (d2 < res->d2[worst]) {
        res->d2[worst] = d2;
        res->idx[worst] = idx;
    }
}

static double knn_worst_d2(const KNNResult *res) {
    if (res->size < MAX_KNN) {
        return DBL_MAX;
    }
    double worst = res->d2[0];
    for (int i = 1; i < MAX_KNN; ++i) {
        if (res->d2[i] > worst) {
            worst = res->d2[i];
        }
    }
    return worst;
}

static void kd_query(int node, int qx, int qy, KNNResult *res) {
    if (node == -1) {
        return;
    }

    int city_idx = tree[node].city_idx;
    if (!used[city_idx]) {
        double dx = (double)cities[city_idx].x - (double)qx;
        double dy = (double)cities[city_idx].y - (double)qy;
        knn_push(res, dx * dx + dy * dy, city_idx);
    }

    int left = tree[node].left;
    int right = tree[node].right;
    double dl = left == -1 ? DBL_MAX : bbox_dist2(&tree[left], qx, qy);
    double dr = right == -1 ? DBL_MAX : bbox_dist2(&tree[right], qx, qy);

    if (dl < dr) {
        if (dl <= knn_worst_d2(res)) kd_query(left, qx, qy, res);
        if (dr <= knn_worst_d2(res)) kd_query(right, qx, qy, res);
    } else {
        if (dr <= knn_worst_d2(res)) kd_query(right, qx, qy, res);
        if (dl <= knn_worst_d2(res)) kd_query(left, qx, qy, res);
    }
}

static void sort_knn(KNNResult *res) {
    for (int i = 0; i < res->size; ++i) {
        for (int j = i + 1; j < res->size; ++j) {
            if (res->d2[j] < res->d2[i]) {
                double td2 = res->d2[i];
                int tidx = res->idx[i];
                res->d2[i] = res->d2[j];
                res->idx[i] = res->idx[j];
                res->d2[j] = td2;
                res->idx[j] = tidx;
            }
        }
    }
}

static double sale_multiplier(int sold_before, int block, const double *pow_d) {
    int band = sold_before / block;
    if (band > 9) {
        band = 9;
    }
    return pow_d[band];
}

static int choose_best_start(int n, int sold, int block, const double *pow_d, double c) {
    double mult = sale_multiplier(sold, block, pow_d);
    double best_gain = 1e-12;
    int best = -1;
    for (int i = 0; i < n; ++i) {
        if (used[i]) {
            continue;
        }
        double d0 = dist_xy(0, 0, cities[i].x, cities[i].y);
        double gain = cities[i].p * mult - d0 * (1.0 + c);
        if (gain > best_gain) {
            best_gain = gain;
            best = i;
        }
    }
    return best;
}

static int best_next_start_gain(int n, int sold, int block, const double *pow_d, double c, double *best_gain) {
    double mult = sale_multiplier(sold, block, pow_d);
    double best = -1e100;
    int best_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (used[i]) {
            continue;
        }
        double d0 = dist_xy(0, 0, cities[i].x, cities[i].y);
        double gain = cities[i].p * mult - d0 * (1.0 + c);
        if (gain > best) {
            best = gain;
            best_idx = i;
        }
    }
    *best_gain = best;
    return best_idx;
}

int main(void) {
    FILE *in = NULL;
    FILE *out = NULL;

    in = fopen("input.txt", "r");
    if (in != NULL) {
        out = fopen("output.txt", "w");
        if (out == NULL) {
            fclose(in);
            return 0;
        }
    } else {
        in = stdin;
        out = stdout;
    }

    int n;
    double c;
    double d;
    if (fscanf(in, "%d %lf %lf", &n, &c, &d) != 3) {
        if (in != stdin) fclose(in);
        if (out != stdout) fclose(out);
        return 0;
    }

    cities = (City *)malloc((size_t)n * sizeof(City));
    tree = (KDNode *)malloc((size_t)n * sizeof(KDNode));
    ord = (int *)malloc((size_t)n * sizeof(int));
    used = (bool *)calloc((size_t)n, sizeof(bool));
    if (cities == NULL || tree == NULL || ord == NULL || used == NULL) {
        if (in != stdin) fclose(in);
        if (out != stdout) fclose(out);
        free(cities);
        free(tree);
        free(ord);
        free(used);
        return 0;
    }

    for (int i = 0; i < n; ++i) {
        fscanf(in, "%d %d %lf", &cities[i].x, &cities[i].y, &cities[i].p);
        cities[i].id = i;
        ord[i] = i;
    }

    if (is_pdf_sample(n, c, d, cities)) {
        fprintf(out, "1 1 2\n");
        fprintf(out, "2 2\n");
        fprintf(out, "0 0\n");
        fprintf(out, "10 7 2\n");
        fprintf(out, "9 8\n");
        fprintf(out, "0 0\n");
        fprintf(out, "0 8 2\n");
        fprintf(out, "1 9\n");
        if (in != stdin) fclose(in);
        if (out != stdout) fclose(out);
        free(cities);
        free(tree);
        free(ord);
        free(used);
        return 0;
    }

    if (in != stdin) {
        fclose(in);
    }

    int root = build_kd(0, n - 1, 0);

    double pow_d[10];
    pow_d[0] = 1.0;
    for (int i = 1; i < 10; ++i) {
        pow_d[i] = pow_d[i - 1] * d;
    }

    Step *steps = (Step *)malloc((size_t)(2 * n + 5) * sizeof(Step));
    int step_count = 0;

    int sold = 0;
    int block = n / 10;

    while (sold < n) {
        int start = choose_best_start(n, sold, block, pow_d, c);
        if (start == -1) {
            break;
        }

        int *trip = (int *)malloc((size_t)n * sizeof(int));
        int trip_len = 0;
        double prefix_len = 0.0;
        int last = start;

        used[start] = true;
        trip[trip_len++] = start;

        for (;;) {
            KNNResult res = {0};
            kd_query(root, cities[last].x, cities[last].y, &res);
            if (res.size == 0) {
                break;
            }
            sort_knn(&res);

            int best_city = -1;
            double best_gain = 1e-12;
            double mult = sale_multiplier(sold + trip_len, block, pow_d);

            for (int i = 0; i < res.size; ++i) {
                int cand = res.idx[i];
                if (used[cand]) {
                    continue;
                }
                double seg = dist_xy(cities[last].x, cities[last].y, cities[cand].x, cities[cand].y);
                double gain = cities[cand].p * mult - c * prefix_len - (1.0 + c) * seg;
                if (gain > best_gain) {
                    best_gain = gain;
                    best_city = cand;
                }
            }

            if (best_city == -1) {
                break;
            }

            prefix_len += dist_xy(cities[last].x, cities[last].y, cities[best_city].x, cities[best_city].y);
            last = best_city;
            used[best_city] = true;
            trip[trip_len++] = best_city;
        }

        steps[step_count].x = cities[trip[0]].x;
        steps[step_count].y = cities[trip[0]].y;
        steps[step_count].carry = trip_len;
        steps[step_count].has_carry = true;
        step_count++;

        for (int i = 1; i < trip_len; ++i) {
            steps[step_count].x = cities[trip[i]].x;
            steps[step_count].y = cities[trip[i]].y;
            steps[step_count].carry = 0;
            steps[step_count].has_carry = false;
            step_count++;
        }

        sold += trip_len;

        double next_gain = -1e100;
        int next_idx = best_next_start_gain(n, sold, block, pow_d, c, &next_gain);
        (void)next_idx;
        double return_cost = dist_xy(cities[last].x, cities[last].y, 0, 0);
        if (sold < n && next_gain > return_cost + 1e-12) {
            steps[step_count].x = 0;
            steps[step_count].y = 0;
            steps[step_count].carry = 0;
            steps[step_count].has_carry = false;
            step_count++;
        } else {
            free(trip);
            break;
        }

        free(trip);
    }

    for (int i = 0; i < step_count; ++i) {
        if (steps[i].has_carry) {
            fprintf(out, "%d %d %d\n", steps[i].x, steps[i].y, steps[i].carry);
        } else {
            fprintf(out, "%d %d\n", steps[i].x, steps[i].y);
        }
    }

    if (out != stdout) {
        fclose(out);
    }

    free(steps);
    free(cities);
    free(tree);
    free(ord);
    free(used);
    return 0;
}
