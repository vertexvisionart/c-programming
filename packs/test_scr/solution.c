#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_KNN 64
#define MAX_TRIP_BUF 1024

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
  int x;
  int y;
  double p;
  int id;
} City;

typedef struct {
  int idx;
  double score;
} RankedCity;

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
  int city_idx;
} Step;

typedef struct {
  Step *data;
  int size;
  int cap;
} Route;

typedef struct {
  int idx;
  int bucket;
  double dist0;
} SectorCity;

typedef struct {
  int start;
  int len;
  double est_profit;
  double sum_p;
  double max_p;
  double cost_with_return;
  double return_cost;
  double seed_score;
} Chunk;

static City *cities = NULL;
static KDNode *tree = NULL;
static int *ord = NULL;
static bool *used = NULL;
static bool *chosen = NULL;
static bool *is_hq_city = NULL;
static RankedCity *ranked = NULL;
static SectorCity *sector_cities = NULL;
static Chunk *chunks = NULL;

static int axis_now = 0;
static int sector_desc = 0;

static int best_next_start_gain(int n, int sold, int block, const double *pow_d,
                                double c, double *best_gain);
static double elapsed_seconds(clock_t start_clock);
static void reverse_order_segment(int *arr, int l, int r);
static int seq_prev_marker(const int *seq, int pos);
static int seq_next_marker(const int *seq, int len, int pos);
static Route seq_to_route(const int *seq, int len);

static bool city_sellable(int idx) { return !is_hq_city[idx]; }

static void clear_marks(int n) {
  memset(used, 0, (size_t)n * sizeof(bool));
  memset(chosen, 0, (size_t)n * sizeof(bool));
}

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
    if (tree[left].minx < tree[node].minx)
      tree[node].minx = tree[left].minx;
    if (tree[left].maxx > tree[node].maxx)
      tree[node].maxx = tree[left].maxx;
    if (tree[left].miny < tree[node].miny)
      tree[node].miny = tree[left].miny;
    if (tree[left].maxy > tree[node].maxy)
      tree[node].maxy = tree[left].maxy;
  }
  if (tree[node].right != -1) {
    int right = tree[node].right;
    if (tree[right].minx < tree[node].minx)
      tree[node].minx = tree[right].minx;
    if (tree[right].maxx > tree[node].maxx)
      tree[node].maxx = tree[right].maxx;
    if (tree[right].miny < tree[node].miny)
      tree[node].miny = tree[right].miny;
    if (tree[right].maxy > tree[node].maxy)
      tree[node].maxy = tree[right].maxy;
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
    if (dl <= knn_worst_d2(res))
      kd_query(left, qx, qy, res);
    if (dr <= knn_worst_d2(res))
      kd_query(right, qx, qy, res);
  } else {
    if (dr <= knn_worst_d2(res))
      kd_query(right, qx, qy, res);
    if (dl <= knn_worst_d2(res))
      kd_query(left, qx, qy, res);
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

static int choose_best_start(int n, int sold, int block, const double *pow_d,
                             double c) {
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

static int cmp_ranked_desc(const void *a, const void *b) {
  const RankedCity *ra = (const RankedCity *)a;
  const RankedCity *rb = (const RankedCity *)b;
  if (ra->score < rb->score)
    return 1;
  if (ra->score > rb->score)
    return -1;
  return 0;
}

static int cmp_sector_city(const void *a, const void *b) {
  const SectorCity *sa = (const SectorCity *)a;
  const SectorCity *sb = (const SectorCity *)b;
  if (sa->bucket != sb->bucket) {
    return sa->bucket - sb->bucket;
  }
  if (!sector_desc) {
    if (sa->dist0 < sb->dist0)
      return -1;
    if (sa->dist0 > sb->dist0)
      return 1;
  } else {
    if (sa->dist0 < sb->dist0)
      return 1;
    if (sa->dist0 > sb->dist0)
      return -1;
  }
  return 0;
}

static int cmp_chunk_desc(const void *a, const void *b) {
  const Chunk *ca = (const Chunk *)a;
  const Chunk *cb = (const Chunk *)b;
  if (ca->est_profit < cb->est_profit)
    return 1;
  if (ca->est_profit > cb->est_profit)
    return -1;
  return 0;
}

static int cmp_city_p_desc_idx(const void *a, const void *b) {
  int ia = *(const int *)a;
  int ib = *(const int *)b;
  if (cities[ia].p < cities[ib].p) {
    return 1;
  }
  if (cities[ia].p > cities[ib].p) {
    return -1;
  }
  return 0;
}

static void route_push(Route *route, int city_idx, int carry, bool has_carry) {
  if (route->size == route->cap) {
    route->cap = route->cap == 0 ? 16 : route->cap * 2;
    route->data =
        (Step *)realloc(route->data, (size_t)route->cap * sizeof(Step));
  }
  Step *st = &route->data[route->size++];
  st->city_idx = city_idx;
  if (city_idx == -1) {
    st->x = 0;
    st->y = 0;
  } else {
    st->x = cities[city_idx].x;
    st->y = cities[city_idx].y;
  }
  st->carry = carry;
  st->has_carry = has_carry;
}

static bool evaluate_route_exact(const Route *route, int n, double c, double d,
                                 long double *profit_out) {
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  long double decay[10];
  decay[0] = 1.0L;
  for (int i = 1; i < 10; ++i) {
    decay[i] = decay[i - 1] * (long double)d;
  }

  int sold = 0;
  int cargo = 0;
  int curx = 0;
  int cury = 0;
  bool at_hq = true;
  long double revenue = 0.0L;
  long double cost = 0.0L;

  memset(chosen, 0, (size_t)n * sizeof(bool));

  for (int i = 0; i < route->size; ++i) {
    const Step *st = &route->data[i];

    if (st->has_carry) {
      /* Carry can only be set when departing from HQ. */
      if (!at_hq || st->carry <= 0) {
        return false;
      }
      cargo = st->carry;
    }

    if (st->city_idx != -1 && !st->has_carry && at_hq) {
      return false;
    }

    int nx = st->x;
    int ny = st->y;
    double seg = dist_xy(curx, cury, nx, ny);
    cost += (long double)seg * (1.0L + (long double)c * (long double)cargo);
    curx = nx;
    cury = ny;

    if (st->city_idx == -1) {
      /* Explicit return marker must be plain \"0 0\" without carry. */
      if (st->has_carry || at_hq || nx != 0 || ny != 0) {
        return false;
      }
      cargo = 0;
      at_hq = true;
      continue;
    }

    if (st->city_idx < 0 || st->city_idx >= n) {
      return false;
    }
    if (!city_sellable(st->city_idx)) {
      return false;
    }
    if (chosen[st->city_idx] || cargo <= 0) {
      return false;
    }
    chosen[st->city_idx] = true;

    int band = sold / block;
    if (band > 9) {
      band = 9;
    }
    revenue += (long double)cities[st->city_idx].p * decay[band];
    cargo--;
    sold++;
    at_hq = false;
  }

  *profit_out = revenue - cost;
  return true;
}

static double route_profit(const Route *route, int n, double c, double d) {
  long double profit = 0.0L;
  if (!evaluate_route_exact(route, n, c, d, &profit)) {
    return -1e100;
  }
  return (double)profit;
}

static void output_route(const Route *route, FILE *out) {
  for (int i = 0; i < route->size; ++i) {
    const Step *st = &route->data[i];
    if (st->has_carry) {
      fprintf(out, "%d %d %d\n", st->x, st->y, st->carry);
    } else {
      fprintf(out, "%d %d\n", st->x, st->y);
    }
  }
}

typedef struct {
  unsigned char kind;
  signed char city;
  signed char carry;
} ExactChoice;

static int exact_state_idx(int mask, int pos, int load, int n) {
  return ((mask * (n + 1)) + pos) * (n + 1) + load;
}

static double exact_small_dfs(int n, double c, int block, const double *pow_d,
                              const double *dist0, const double *dist_mat,
                              int mask, int pos, int load, double *memo,
                              unsigned char *seen, ExactChoice *choice) {
  int idx = exact_state_idx(mask, pos, load, n);
  if (seen[idx]) {
    return memo[idx];
  }
  seen[idx] = 1;

  int sold = __builtin_popcount((unsigned)mask);
  int band = sold / block;
  if (band > 9) {
    band = 9;
  }

  double best = 0.0;
  ExactChoice best_choice = {0, -1, 0};
  int hq = n;

  if (pos == hq) {
    for (int city = 0; city < n; ++city) {
      if ((mask & (1 << city)) != 0) {
        continue;
      }
      if (!city_sellable(city)) {
        continue;
      }
      double rev = cities[city].p * pow_d[band];
      for (int carry = 1; carry <= n - sold; ++carry) {
        double cand =
            rev - dist0[city] * (1.0 + c * (double)carry) +
            exact_small_dfs(n, c, block, pow_d, dist0, dist_mat,
                            mask | (1 << city), city, carry - 1, memo, seen,
                            choice);
        if (cand > best + 1e-12) {
          best = cand;
          best_choice.kind = 1;
          best_choice.city = (signed char)city;
          best_choice.carry = (signed char)carry;
        }
      }
    }
  } else {
    double ret =
        -dist0[pos] * (1.0 + c * (double)load) +
        exact_small_dfs(n, c, block, pow_d, dist0, dist_mat, mask, hq, 0,
                        memo, seen, choice);
    if (ret > best + 1e-12) {
      best = ret;
      best_choice.kind = 2;
      best_choice.city = -1;
      best_choice.carry = 0;
    }

    if (load > 0) {
      for (int city = 0; city < n; ++city) {
        if ((mask & (1 << city)) != 0) {
          continue;
        }
        if (!city_sellable(city)) {
          continue;
        }
        double rev = cities[city].p * pow_d[band];
        double cand =
            rev - dist_mat[pos * n + city] * (1.0 + c * (double)load) +
            exact_small_dfs(n, c, block, pow_d, dist0, dist_mat,
                            mask | (1 << city), city, load - 1, memo, seen,
                            choice);
        if (cand > best + 1e-12) {
          best = cand;
          best_choice.kind = 3;
          best_choice.city = (signed char)city;
          best_choice.carry = 0;
        }
      }
    }
  }

  memo[idx] = best;
  choice[idx] = best_choice;
  return best;
}

static Route build_small_exact(int n, double c, const double *pow_d) {
  Route route = {0};
  int state_count = (1 << n) * (n + 1) * (n + 1);
  double *memo = (double *)malloc((size_t)state_count * sizeof(double));
  unsigned char *seen = (unsigned char *)calloc((size_t)state_count, 1);
  ExactChoice *choice =
      (ExactChoice *)malloc((size_t)state_count * sizeof(ExactChoice));
  double *dist0 = (double *)malloc((size_t)n * sizeof(double));
  double *dist_mat = (double *)malloc((size_t)n * (size_t)n * sizeof(double));
  if (memo == NULL || seen == NULL || choice == NULL || dist0 == NULL ||
      dist_mat == NULL) {
    free(memo);
    free(seen);
    free(choice);
    free(dist0);
    free(dist_mat);
    return route;
  }

  for (int i = 0; i < n; ++i) {
    dist0[i] = dist_xy(0, 0, cities[i].x, cities[i].y);
    for (int j = 0; j < n; ++j) {
      dist_mat[i * n + j] =
          dist_xy(cities[i].x, cities[i].y, cities[j].x, cities[j].y);
    }
  }

  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  (void)exact_small_dfs(n, c, block, pow_d, dist0, dist_mat, 0, n, 0, memo,
                        seen, choice);

  int mask = 0;
  int pos = n;
  int load = 0;
  while (true) {
    int idx = exact_state_idx(mask, pos, load, n);
    ExactChoice act = choice[idx];
    if (act.kind == 0) {
      break;
    }
    if (act.kind == 1) {
      int city = (int)act.city;
      int carry = (int)act.carry;
      route_push(&route, city, carry, true);
      mask |= 1 << city;
      pos = city;
      load = carry - 1;
    } else if (act.kind == 2) {
      route_push(&route, -1, 0, false);
      pos = n;
      load = 0;
    } else {
      int city = (int)act.city;
      route_push(&route, city, 0, false);
      mask |= 1 << city;
      pos = city;
      load--;
    }
  }

  free(memo);
  free(seen);
  free(choice);
  free(dist0);
  free(dist_mat);
  return route;
}

static Route build_small_greedy(int n, double c, double d, int root,
                                const double *pow_d) {
  Route route = {0};
  clear_marks(n);

  int sold = 0;
  int block = n / 10;

  while (sold < n) {
    int start = choose_best_start(n, sold, block, pow_d, c);
    if (start == -1) {
      break;
    }

    int *trip = (int *)malloc((size_t)n * sizeof(int));
    if (trip == NULL) {
      break;
    }
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
        double seg = dist_xy(cities[last].x, cities[last].y, cities[cand].x,
                             cities[cand].y);
        double gain = cities[cand].p * mult - c * prefix_len - (1.0 + c) * seg;
        if (gain > best_gain) {
          best_gain = gain;
          best_city = cand;
        }
      }

      if (best_city == -1) {
        break;
      }

      prefix_len += dist_xy(cities[last].x, cities[last].y, cities[best_city].x,
                            cities[best_city].y);
      last = best_city;
      used[best_city] = true;
      trip[trip_len++] = best_city;
    }

    route_push(&route, trip[0], trip_len, true);
    for (int i = 1; i < trip_len; ++i) {
      route_push(&route, trip[i], 0, false);
    }

    sold += trip_len;

    double next_gain = -1e100;
    (void)best_next_start_gain(n, sold, block, pow_d, c, &next_gain);
    double return_cost = dist_xy(cities[last].x, cities[last].y, 0, 0);
    if (sold < n && next_gain > return_cost + 1e-12) {
      route_push(&route, -1, 0, false);
    } else {
      free(trip);
      break;
    }

    free(trip);
  }

  return route;
}

static int next_ranked_start(int n, int *ptr, int sold, int block, double d,
                             double c) {
  int band = sold / block;
  if (band > 9) {
    band = 9;
  }
  double mult = 1.0;
  for (int i = 0; i < band; ++i) {
    mult *= d;
  }

  while (*ptr < n) {
    int idx = ranked[*ptr].idx;
    (*ptr)++;
    if (used[idx]) {
      continue;
    }
    double d0 = dist_xy(0, 0, cities[idx].x, cities[idx].y);
    double gain = cities[idx].p * mult - d0 * (1.0 + c);
    if (gain > 1e-12) {
      return idx;
    }
    return -1;
  }
  return -1;
}

static Route build_large_batch(int n, double c, double d, int root,
                               int max_trip_len) {
  Route route = {0};
  clear_marks(n);
  for (int i = 0; i < n; ++i) {
    double d0 = dist_xy(0, 0, cities[i].x, cities[i].y);
    ranked[i].idx = i;
    ranked[i].score = cities[i].p - d0 * (1.0 + c);
  }
  qsort(ranked, (size_t)n, sizeof(RankedCity), cmp_ranked_desc);

  int sold = 0;
  int block = n / 10;
  int ptr = 0;
  bool first_trip = true;

  while (sold < n) {
    int start = next_ranked_start(n, &ptr, sold, block, d, c);
    if (start == -1) {
      break;
    }

    int trip[320];
    int trip_len = 0;
    double prefix_len = 0.0;
    int last = start;

    used[start] = true;
    trip[trip_len++] = start;

    while (trip_len < max_trip_len) {
      KNNResult res = {0};
      kd_query(root, cities[last].x, cities[last].y, &res);
      if (res.size == 0) {
        break;
      }
      sort_knn(&res);

      int best_city = -1;
      double best_gain = 1e-12;
      int band = (sold + trip_len) / block;
      if (band > 9) {
        band = 9;
      }
      double mult = 1.0;
      for (int i = 0; i < band; ++i) {
        mult *= d;
      }

      for (int i = 0; i < res.size; ++i) {
        int cand = res.idx[i];
        if (used[cand]) {
          continue;
        }
        double seg = dist_xy(cities[last].x, cities[last].y, cities[cand].x,
                             cities[cand].y);
        double gain = cities[cand].p * mult - c * prefix_len - (1.0 + c) * seg;
        if (gain > best_gain) {
          best_gain = gain;
          best_city = cand;
        }
      }

      if (best_city == -1) {
        break;
      }

      prefix_len += dist_xy(cities[last].x, cities[last].y, cities[best_city].x,
                            cities[best_city].y);
      last = best_city;
      used[best_city] = true;
      trip[trip_len++] = best_city;
    }

    if (!first_trip) {
      route_push(&route, -1, 0, false);
    }
    first_trip = false;

    route_push(&route, trip[0], trip_len, true);
    for (int i = 1; i < trip_len; ++i) {
      route_push(&route, trip[i], 0, false);
    }

    sold += trip_len;

    int saved_ptr = ptr;
    int next_start = next_ranked_start(n, &saved_ptr, sold, block, d, c);
    if (next_start == -1) {
      break;
    }
    double dback = dist_xy(cities[last].x, cities[last].y, 0, 0);
    int band = sold / block;
    if (band > 9) {
      band = 9;
    }
    double mult = 1.0;
    for (int i = 0; i < band; ++i) {
      mult *= d;
    }
    double d0 = dist_xy(0, 0, cities[next_start].x, cities[next_start].y);
    double next_gain = cities[next_start].p * mult - d0 * (1.0 + c);
    if (next_gain <= dback + 1e-12) {
      break;
    }
  }
  return route;
}

static Route build_large_compact(int n, double c, double d, int root,
                                 int target_cities, int max_trip_len,
                                 int mode) {
  Route route = {0};
  clear_marks(n);
  for (int i = 0; i < n; ++i) {
    double d0 = dist_xy(0, 0, cities[i].x, cities[i].y);
    ranked[i].idx = i;
    if (mode == 0) {
      ranked[i].score = cities[i].p - d0 * (1.0 + c);
    } else {
      ranked[i].score = cities[i].p / (d0 + 25.0);
    }
    chosen[i] = false;
    used[i] = false;
  }
  qsort(ranked, (size_t)n, sizeof(RankedCity), cmp_ranked_desc);

  int selected = 0;
  for (int i = 0; i < n && selected < target_cities; ++i) {
    if (ranked[i].score <= 1e-12) {
      break;
    }
    chosen[ranked[i].idx] = true;
    selected++;
  }

  int sold = 0;
  int block = n / 10;
  bool first_trip = true;

  for (int ptr = 0; ptr < n && sold < selected; ++ptr) {
    int start = ranked[ptr].idx;
    if (!chosen[start] || used[start]) {
      continue;
    }

    int band0 = sold / block;
    if (band0 > 9)
      band0 = 9;
    double mult0 = 1.0;
    for (int i = 0; i < band0; ++i)
      mult0 *= d;
    double start_gain =
        cities[start].p * mult0 -
        dist_xy(0, 0, cities[start].x, cities[start].y) * (1.0 + c);
    if (start_gain <= 1e-12) {
      continue;
    }

    int trip[320];
    int trip_len = 0;
    double prefix_len = 0.0;
    int last = start;
    used[start] = true;
    trip[trip_len++] = start;

    while (trip_len < max_trip_len && sold + trip_len < selected) {
      KNNResult res = {0};
      kd_query(root, cities[last].x, cities[last].y, &res);
      if (res.size == 0) {
        break;
      }
      sort_knn(&res);

      int best_city = -1;
      double best_gain = 1e-12;
      int band = (sold + trip_len) / block;
      if (band > 9)
        band = 9;
      double mult = 1.0;
      for (int i = 0; i < band; ++i)
        mult *= d;

      for (int i = 0; i < res.size; ++i) {
        int cand = res.idx[i];
        if (!chosen[cand] || used[cand]) {
          continue;
        }
        double seg = dist_xy(cities[last].x, cities[last].y, cities[cand].x,
                             cities[cand].y);
        double gain = cities[cand].p * mult - c * prefix_len - (1.0 + c) * seg;
        if (gain > best_gain) {
          best_gain = gain;
          best_city = cand;
        }
      }

      if (best_city == -1) {
        break;
      }

      prefix_len += dist_xy(cities[last].x, cities[last].y, cities[best_city].x,
                            cities[best_city].y);
      last = best_city;
      used[best_city] = true;
      trip[trip_len++] = best_city;
    }

    if (!first_trip) {
      route_push(&route, -1, 0, false);
    }
    first_trip = false;
    route_push(&route, trip[0], trip_len, true);
    for (int i = 1; i < trip_len; ++i) {
      route_push(&route, trip[i], 0, false);
    }
    sold += trip_len;
  }

  return route;
}

static double score_city_for_start(int idx, double c, int score_mode) {
  double d0 = dist_xy(0, 0, cities[idx].x, cities[idx].y);
  if (score_mode == 0) {
    return cities[idx].p - d0 * (1.0 + c);
  }
  if (score_mode == 1) {
    return cities[idx].p / (d0 + 1.0);
  }
  if (score_mode == 2) {
    return cities[idx].p / (d0 + 25.0);
  }
  if (score_mode == 3) {
    return cities[idx].p - d0 * (2.0 + c);
  }
  return cities[idx].p;
}

static int next_ranked_start_with_slack(int n, int *ptr, int sold, int block,
                                        const double *pow_d, double c,
                                        double start_slack,
                                        double *chosen_gain) {
  double mult = sale_multiplier(sold, block, pow_d);
  while (*ptr < n) {
    int idx = ranked[*ptr].idx;
    (*ptr)++;
    if (used[idx]) {
      continue;
    }
    double d0 = dist_xy(0, 0, cities[idx].x, cities[idx].y);
    double gain = cities[idx].p * mult - d0 * (1.0 + c);
    if (gain > -start_slack * d0) {
      *chosen_gain = gain;
      return idx;
    }
  }
  *chosen_gain = -1e100;
  return -1;
}

static Route build_large_knn(int n, double c, int root, const double *pow_d,
                             int max_trip_len, int score_mode,
                             double start_slack, int use_ret_delta,
                             double return_factor) {
  Route route = {0};
  clear_marks(n);

  for (int i = 0; i < n; ++i) {
    ranked[i].idx = i;
    ranked[i].score = score_city_for_start(i, c, score_mode);
  }
  qsort(ranked, (size_t)n, sizeof(RankedCity), cmp_ranked_desc);

  if (max_trip_len < 2) {
    max_trip_len = 2;
  }
  if (max_trip_len > MAX_TRIP_BUF) {
    max_trip_len = MAX_TRIP_BUF;
  }
  if (return_factor < 0.2) {
    return_factor = 0.2;
  }
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  int sold = 0;
  int ptr = 0;
  bool first_trip = true;
  int trip[MAX_TRIP_BUF];

  while (sold < n) {
    double start_gain = -1e100;
    int start = next_ranked_start_with_slack(n, &ptr, sold, block, pow_d, c,
                                             start_slack, &start_gain);
    if (start == -1) {
      break;
    }

    int trip_len = 0;
    int last = start;
    double prefix_len = 0.0;
    used[start] = true;
    trip[trip_len++] = start;

    while (trip_len < max_trip_len) {
      KNNResult res = {0};
      kd_query(root, cities[last].x, cities[last].y, &res);
      if (res.size == 0) {
        break;
      }
      sort_knn(&res);

      int best_city = -1;
      double best_gain = 1e-12;
      double mult = sale_multiplier(sold + trip_len, block, pow_d);
      double back_last = 0.0;
      if (use_ret_delta) {
        back_last = dist_xy(cities[last].x, cities[last].y, 0, 0);
      }

      for (int i = 0; i < res.size; ++i) {
        int cand = res.idx[i];
        if (used[cand]) {
          continue;
        }
        double seg = dist_xy(cities[last].x, cities[last].y, cities[cand].x,
                             cities[cand].y);
        double gain = cities[cand].p * mult - c * prefix_len - (1.0 + c) * seg;
        if (use_ret_delta) {
          gain -= dist_xy(cities[cand].x, cities[cand].y, 0, 0) - back_last;
        }
        if (gain > best_gain) {
          best_gain = gain;
          best_city = cand;
        }
      }

      if (best_city == -1) {
        break;
      }

      prefix_len += dist_xy(cities[last].x, cities[last].y, cities[best_city].x,
                            cities[best_city].y);
      last = best_city;
      used[best_city] = true;
      trip[trip_len++] = best_city;
    }

    if (trip_len == 1 && start_gain <= 1e-12) {
      used[start] = false;
      continue;
    }

    if (!first_trip) {
      route_push(&route, -1, 0, false);
    }
    first_trip = false;
    route_push(&route, trip[0], trip_len, true);
    for (int i = 1; i < trip_len; ++i) {
      route_push(&route, trip[i], 0, false);
    }
    sold += trip_len;

    int saved_ptr = ptr;
    double next_gain = -1e100;
    int next_idx = next_ranked_start_with_slack(
        n, &saved_ptr, sold, block, pow_d, c, start_slack, &next_gain);
    if (next_idx == -1) {
      break;
    }
    double dback = dist_xy(cities[last].x, cities[last].y, 0, 0);
    if (next_gain <= dback * return_factor + 1e-12) {
      break;
    }
  }

  return route;
}

static double chunk_actual_profit(const int *trip, int len, int sold, int n,
                                  double c, double d) {
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }
  double revenue = 0.0;
  double cost = 0.0;
  int cargo = len;
  int cx = 0;
  int cy = 0;

  for (int i = 0; i < len; ++i) {
    int idx = trip[i];
    double seg = dist_xy(cx, cy, cities[idx].x, cities[idx].y);
    cost += seg * (1.0 + c * cargo);
    cx = cities[idx].x;
    cy = cities[idx].y;

    int band = (sold + i) / block;
    if (band > 9) {
      band = 9;
    }
    revenue += cities[idx].p * pow(d, band);
    cargo--;
  }

  cost += dist_xy(cx, cy, 0, 0);
  return revenue - cost;
}

static Route build_large_sector(int n, double c, double d, int max_trip_len,
                                int sector_count, int reverse_dist,
                                double solo_slack, double sector_shift,
                                int sort_mode) {
  Route route = {0};
  int scount = 0;

  for (int i = 0; i < n; ++i) {
    double dist0 = dist_xy(0, 0, cities[i].x, cities[i].y);
    double solo = cities[i].p - dist0 * (1.0 + c);
    if (solo <= -solo_slack * dist0) {
      continue;
    }
    double ang = atan2((double)cities[i].y, (double)cities[i].x);
    double norm = (ang + M_PI) / (2.0 * M_PI) + sector_shift;
    norm -= floor(norm);
    int bucket = (int)(norm * sector_count);
    if (bucket < 0)
      bucket = 0;
    if (bucket >= sector_count)
      bucket = sector_count - 1;
    sector_cities[scount].idx = i;
    sector_cities[scount].bucket = bucket;
    if (sort_mode == 1) {
      sector_cities[scount].dist0 = cities[i].p / (dist0 + 1.0);
    } else {
      sector_cities[scount].dist0 = dist0;
    }
    scount++;
  }

  sector_desc = (sort_mode == 1) ? 1 : reverse_dist;
  qsort(sector_cities, (size_t)scount, sizeof(SectorCity), cmp_sector_city);

  int chunk_count = 0;
  int pos = 0;
  while (pos < scount) {
    int end = pos + 1;
    while (end < scount &&
           sector_cities[end].bucket == sector_cities[pos].bucket) {
      end++;
    }

    int p = pos;
    while (p < end) {
      double best_profit = 1e-12;
      int best_len = 0;
      double path_len = 0.0;
      double route_cost = 0.0;
      double route_rev = 0.0;
      int prevx = 0;
      int prevy = 0;

      int limit = end - p;
      if (limit > max_trip_len) {
        limit = max_trip_len;
      }

      for (int len = 1; len <= limit; ++len) {
        int idx = sector_cities[p + len - 1].idx;
        double seg = dist_xy(prevx, prevy, cities[idx].x, cities[idx].y);
        route_cost += c * path_len + seg * (1.0 + c);
        path_len += seg;
        route_rev += cities[idx].p;
        prevx = cities[idx].x;
        prevy = cities[idx].y;

        double full_profit =
            route_rev - route_cost - dist_xy(prevx, prevy, 0, 0);
        if (full_profit > best_profit) {
          best_profit = full_profit;
          best_len = len;
        }
      }

      if (best_len > 0) {
        chunks[chunk_count].start = p;
        chunks[chunk_count].len = best_len;
        chunks[chunk_count].est_profit = best_profit;
        chunk_count++;
        p += best_len;
      } else {
        p++;
      }
    }

    pos = end;
  }

  qsort(chunks, (size_t)chunk_count, sizeof(Chunk), cmp_chunk_desc);

  int sold = 0;
  bool first = true;
  for (int i = 0; i < chunk_count; ++i) {
    int trip_buf[MAX_TRIP_BUF];
    int max_len = chunks[i].len;
    if (max_len > MAX_TRIP_BUF) {
      max_len = MAX_TRIP_BUF;
    }
    for (int j = 0; j < max_len; ++j) {
      trip_buf[j] = sector_cities[chunks[i].start + j].idx;
    }

    int best_len = max_len;
    double best_profit = chunk_actual_profit(trip_buf, best_len, sold, n, c, d);
    if (best_profit <= 1e-12) {
      int block = n / 10;
      if (block < 1) {
        block = 1;
      }
      best_len = 0;
      best_profit = 1e-12;
      double route_cost = 0.0;
      double route_rev = 0.0;
      double path_len = 0.0;
      int prevx = 0;
      int prevy = 0;
      for (int len = 1; len <= max_len; ++len) {
        int idx = trip_buf[len - 1];
        double seg = dist_xy(prevx, prevy, cities[idx].x, cities[idx].y);
        route_cost += c * path_len + seg * (1.0 + c);
        path_len += seg;
        int band = (sold + len - 1) / block;
        if (band > 9) {
          band = 9;
        }
        route_rev += cities[idx].p * pow(d, band);
        prevx = cities[idx].x;
        prevy = cities[idx].y;
        double prof = route_rev - route_cost - dist_xy(prevx, prevy, 0, 0);
        if (prof > best_profit) {
          best_profit = prof;
          best_len = len;
        }
      }
    }

    if (best_len == 0) {
      continue;
    }
    if (!first) {
      route_push(&route, -1, 0, false);
    }
    first = false;
    route_push(&route, trip_buf[0], best_len, true);
    for (int j = 1; j < best_len; ++j) {
      route_push(&route, trip_buf[j], 0, false);
    }
    sold += best_len;
  }

  return route;
}

static Route build_large_sweep(int n, double c, double d, int max_trip_len,
                               int sector_count, int reverse_dist) {
  Route route = {0};
  int scount = 0;

  for (int i = 0; i < n; ++i) {
    double dist0 = dist_xy(0, 0, cities[i].x, cities[i].y);
    double solo = cities[i].p - dist0 * (1.0 + c);
    if (solo <= 1e-12) {
      continue;
    }
    double ang = atan2((double)cities[i].y, (double)cities[i].x);
    double norm = (ang + M_PI) / (2.0 * M_PI);
    int bucket = (int)(norm * sector_count);
    if (bucket < 0)
      bucket = 0;
    if (bucket >= sector_count)
      bucket = sector_count - 1;
    sector_cities[scount].idx = i;
    sector_cities[scount].bucket = bucket;
    sector_cities[scount].dist0 = dist0;
    scount++;
  }

  sector_desc = reverse_dist;
  qsort(sector_cities, (size_t)scount, sizeof(SectorCity), cmp_sector_city);

  int sold = 0;
  int pos = 0;
  bool first = true;
  while (pos < scount) {
    double best_profit = 1e-12;
    int best_len = 0;
    int trip[MAX_TRIP_BUF];
    int limit = scount - pos;
    if (limit > max_trip_len) {
      limit = max_trip_len;
    }
    if (limit > MAX_TRIP_BUF) {
      limit = MAX_TRIP_BUF;
    }

    double revenue = 0.0;
    double cost = 0.0;
    int cargo = 0;
    int cx = 0;
    int cy = 0;

    for (int len = 1; len <= limit; ++len) {
      int idx = sector_cities[pos + len - 1].idx;
      double seg = dist_xy(cx, cy, cities[idx].x, cities[idx].y);
      cargo = len;
      trip[len - 1] = idx;

      revenue = 0.0;
      cost = 0.0;
      cx = 0;
      cy = 0;
      cargo = len;
      for (int i = 0; i < len; ++i) {
        int id = trip[i];
        double step = dist_xy(cx, cy, cities[id].x, cities[id].y);
        cost += step * (1.0 + c * cargo);
        cx = cities[id].x;
        cy = cities[id].y;
        int band = (sold + i) / (n / 10);
        if (band > 9)
          band = 9;
        revenue += cities[id].p * pow(d, band);
        cargo--;
      }
      cost += dist_xy(cx, cy, 0, 0);

      if (revenue - cost > best_profit) {
        best_profit = revenue - cost;
        best_len = len;
      }
    }

    if (best_len == 0) {
      pos++;
      continue;
    }

    if (!first) {
      route_push(&route, -1, 0, false);
    }
    first = false;
    route_push(&route, sector_cities[pos].idx, best_len, true);
    for (int i = 1; i < best_len; ++i) {
      route_push(&route, sector_cities[pos + i].idx, 0, false);
    }
    sold += best_len;
    pos += best_len;
  }

  return route;
}

static int best_next_start_gain(int n, int sold, int block, const double *pow_d,
                                double c, double *best_gain) {
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

static bool route_is_valid(const Route *route, int n) {
  long double dummy_profit = 0.0L;
  return evaluate_route_exact(route, n, 0.0, 1.0, &dummy_profit);
}

static void consider_route(Route *best_route, double *best_profit, Route cand,
                           int n, double c, double d) {
  if (!route_is_valid(&cand, n)) {
    free(cand.data);
    return;
  }
  double profit = route_profit(&cand, n, c, d);
  if (profit > *best_profit) {
    free(best_route->data);
    *best_route = cand;
    *best_profit = profit;
  } else {
    free(cand.data);
  }
}

typedef struct {
  int trip_len;
  int sector_count;
  double angle_shift;
  int reverse_dir;
  int secondary_mode;
} SweepVariant;

static long double seq_trip_cost_mode(const int *seq, int start, int end,
                                      double c, bool include_return) {
  if (start >= end) {
    return 0.0L;
  }
  int cargo = end - start;
  int cx = 0;
  int cy = 0;
  long double cost = 0.0L;
  for (int i = start; i < end; ++i) {
    int idx = seq[i];
    double seg = dist_xy(cx, cy, cities[idx].x, cities[idx].y);
    cost += (long double)seg * (1.0L + (long double)c * (long double)cargo);
    cargo--;
    cx = cities[idx].x;
    cy = cities[idx].y;
  }
  if (include_return) {
    cost += (long double)dist_xy(cx, cy, 0, 0);
  }
  return cost;
}

static long double seq_trip_profit_segment(const int *seq, int start, int end,
                                           int sold_before, int n, double c,
                                           const double *pow_d,
                                           bool include_return) {
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  long double revenue = 0.0L;
  for (int i = start; i < end; ++i) {
    int band = (sold_before + (i - start)) / block;
    if (band > 9) {
      band = 9;
    }
    revenue += (long double)cities[seq[i]].p * (long double)pow_d[band];
  }
  return revenue - seq_trip_cost_mode(seq, start, end, c, include_return);
}

static int seq_collect_trip_bounds(const int *seq, int len, int *starts,
                                   int *ends) {
  int trip_count = 0;
  int i = 0;
  while (i < len) {
    while (i < len && seq[i] == -1) {
      i++;
    }
    if (i >= len) {
      break;
    }
    int j = i;
    while (j < len && seq[j] != -1) {
      j++;
    }
    starts[trip_count] = i;
    ends[trip_count] = j;
    trip_count++;
    i = j + 1;
  }
  return trip_count;
}

static void seq_trim_best_prefix(int *seq, int *len, int n, double c,
                                 const double *pow_d) {
  if (*len <= 0) {
    return;
  }

  int *starts = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  int *ends = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  if (starts == NULL || ends == NULL) {
    free(starts);
    free(ends);
    return;
  }

  int trip_count = seq_collect_trip_bounds(seq, *len, starts, ends);
  long double pref_return = 0.0L;
  long double best_profit = 0.0L;
  int best_cut = 0;
  int sold = 0;

  for (int t = 0; t < trip_count; ++t) {
    long double no_return = seq_trip_profit_segment(
        seq, starts[t], ends[t], sold, n, c, pow_d, false);
    long double cand = pref_return + no_return;
    if (cand > best_profit) {
      best_profit = cand;
      best_cut = ends[t];
    }

    pref_return += seq_trip_profit_segment(seq, starts[t], ends[t], sold, n, c,
                                           pow_d, true);
    sold += ends[t] - starts[t];
  }

  if (best_cut <= 0) {
    *len = 0;
  } else {
    *len = best_cut;
  }

  free(starts);
  free(ends);
}

static void seq_band_safe_two_opt(int *seq, int len, int n, double c,
                                  const double *pow_d) {
  (void)pow_d;
  if (len <= 1) {
    return;
  }

  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  int *starts = (int *)malloc((size_t)(len + 1) * sizeof(int));
  int *ends = (int *)malloc((size_t)(len + 1) * sizeof(int));
  if (starts == NULL || ends == NULL) {
    free(starts);
    free(ends);
    return;
  }

  int trip_count = seq_collect_trip_bounds(seq, len, starts, ends);
  int sold = 0;
  for (int t = 0; t < trip_count; ++t) {
    int start = starts[t];
    int end = ends[t];
    bool include_return = (t + 1 < trip_count);
    bool improved = true;

    while (improved) {
      improved = false;
      long double best_cost = seq_trip_cost_mode(seq, start, end, c, include_return);
      int best_i = -1;
      int best_j = -1;

      for (int i = start; i < end; ++i) {
        int band_i = (sold + (i - start)) / block;
        if (band_i > 9) {
          band_i = 9;
        }
        for (int j = i + 1; j < end; ++j) {
          int band_j = (sold + (j - start)) / block;
          if (band_j > 9) {
            band_j = 9;
          }
          if (band_i != band_j) {
            break;
          }

          reverse_order_segment(seq, i, j);
          long double new_cost =
              seq_trip_cost_mode(seq, start, end, c, include_return);
          reverse_order_segment(seq, i, j);

          if (new_cost + 1e-12L < best_cost) {
            best_cost = new_cost;
            best_i = i;
            best_j = j;
          }
        }
      }

      if (best_i != -1) {
        reverse_order_segment(seq, best_i, best_j);
        improved = true;
      }
    }

    sold += end - start;
  }

  free(starts);
  free(ends);
}

static void seq_trip_order_optimize(int *seq, int *len, int n, double c,
                                    const double *pow_d) {
  if (*len <= 1) {
    return;
  }

  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  int *starts = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  int *ends = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  int *order = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  bool *picked = (bool *)calloc((size_t)(*len + 1), sizeof(bool));
  int *tmp = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  double *profit_band = (double *)malloc((size_t)(*len + 1) * 10 * sizeof(double));
  if (starts == NULL || ends == NULL || order == NULL || picked == NULL ||
      tmp == NULL || profit_band == NULL) {
    free(starts);
    free(ends);
    free(order);
    free(picked);
    free(tmp);
    free(profit_band);
    return;
  }

  int trip_count = seq_collect_trip_bounds(seq, *len, starts, ends);
  if (trip_count <= 1) {
    free(starts);
    free(ends);
    free(order);
    free(picked);
    free(tmp);
    free(profit_band);
    return;
  }

  for (int t = 0; t < trip_count; ++t) {
    for (int band = 0; band < 10; ++band) {
      profit_band[t * 10 + band] =
          (double)seq_trip_profit_segment(seq, starts[t], ends[t], band * block,
                                          n, c, pow_d, true);
    }
  }

  int sold = 0;
  for (int pos = 0; pos < trip_count; ++pos) {
    int band = sold / block;
    if (band > 9) {
      band = 9;
    }

    int best_trip = -1;
    double best_score = -1e100;
    for (int t = 0; t < trip_count; ++t) {
      if (picked[t]) {
        continue;
      }
      double sc = profit_band[t * 10 + band];
      if (sc > best_score) {
        best_score = sc;
        best_trip = t;
      }
    }

    if (best_trip == -1) {
      break;
    }
    order[pos] = best_trip;
    picked[best_trip] = true;
    sold += ends[best_trip] - starts[best_trip];
  }

  for (int pass = 0; pass < 3; ++pass) {
    bool changed = false;
    sold = 0;
    for (int i = 0; i + 1 < trip_count; ++i) {
      int a = order[i];
      int b = order[i + 1];
      bool b_is_last = (i + 1 == trip_count - 1);

      long double old_profit =
          seq_trip_profit_segment(seq, starts[a], ends[a], sold, n, c, pow_d,
                                  true) +
          seq_trip_profit_segment(seq, starts[b], ends[b],
                                  sold + (ends[a] - starts[a]), n, c, pow_d,
                                  !b_is_last);

      long double new_profit =
          seq_trip_profit_segment(seq, starts[b], ends[b], sold, n, c, pow_d,
                                  true) +
          seq_trip_profit_segment(seq, starts[a], ends[a],
                                  sold + (ends[b] - starts[b]), n, c, pow_d,
                                  !b_is_last);

      if (new_profit > old_profit + 1e-12L) {
        int t = order[i];
        order[i] = order[i + 1];
        order[i + 1] = t;
        changed = true;
      }
      sold += ends[order[i]] - starts[order[i]];
    }
    if (!changed) {
      break;
    }
  }

  int w = 0;
  for (int i = 0; i < trip_count; ++i) {
    int t = order[i];
    if (i > 0) {
      tmp[w++] = -1;
    }
    for (int p = starts[t]; p < ends[t]; ++p) {
      tmp[w++] = seq[p];
    }
  }

  memcpy(seq, tmp, (size_t)w * sizeof(int));
  *len = w;
  seq_trim_best_prefix(seq, len, n, c, pow_d);

  free(starts);
  free(ends);
  free(order);
  free(picked);
  free(tmp);
  free(profit_band);
}

static void seq_local_neighbors(const int *seq, int len, int pos, int *px,
                                int *py, int *nx, int *ny) {
  int start = seq_prev_marker(seq, pos) + 1;
  int end = seq_next_marker(seq, len, pos);

  if (pos > start) {
    int prev = seq[pos - 1];
    *px = cities[prev].x;
    *py = cities[prev].y;
  } else {
    *px = 0;
    *py = 0;
  }

  if (pos + 1 < end) {
    int next = seq[pos + 1];
    *nx = cities[next].x;
    *ny = cities[next].y;
  } else {
    *nx = 0;
    *ny = 0;
  }
}

static bool seq_swap_geometry_ok(const int *seq, int len, int posa, int posb) {
  int a = seq[posa];
  int b = seq[posb];
  int apx, apy, anx, any;
  int bpx, bpy, bnx, bny;
  seq_local_neighbors(seq, len, posa, &apx, &apy, &anx, &any);
  seq_local_neighbors(seq, len, posb, &bpx, &bpy, &bnx, &bny);

  double old_local =
      dist_xy(apx, apy, cities[a].x, cities[a].y) +
      dist_xy(cities[a].x, cities[a].y, anx, any) +
      dist_xy(bpx, bpy, cities[b].x, cities[b].y) +
      dist_xy(cities[b].x, cities[b].y, bnx, bny);

  double new_local =
      dist_xy(apx, apy, cities[b].x, cities[b].y) +
      dist_xy(cities[b].x, cities[b].y, anx, any) +
      dist_xy(bpx, bpy, cities[a].x, cities[a].y) +
      dist_xy(cities[a].x, cities[a].y, bnx, bny);

  return new_local <= old_local * 1.40 + 20.0;
}

static long double seq_swap_gain_exact(int *seq, int len, int n, double c,
                                       const double *pow_d, int posa,
                                       int posb, const int *pref) {
  if (posa == posb) {
    return 0.0L;
  }

  int city_a = seq[posa];
  int city_b = seq[posb];
  int sold_a = pref[posa];
  int sold_b = pref[posb];
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }
  int band_a = sold_a / block;
  int band_b = sold_b / block;
  if (band_a > 9) {
    band_a = 9;
  }
  if (band_b > 9) {
    band_b = 9;
  }

  long double revenue_delta =
      (long double)cities[city_b].p * (long double)pow_d[band_a] +
      (long double)cities[city_a].p * (long double)pow_d[band_b] -
      (long double)cities[city_a].p * (long double)pow_d[band_a] -
      (long double)cities[city_b].p * (long double)pow_d[band_b];

  int s1 = seq_prev_marker(seq, posa) + 1;
  int e1 = seq_next_marker(seq, len, posa);
  int s2 = seq_prev_marker(seq, posb) + 1;
  int e2 = seq_next_marker(seq, len, posb);

  bool ret1 = (e1 < len);
  bool ret2 = (e2 < len);
  long double old_cost = seq_trip_cost_mode(seq, s1, e1, c, ret1);
  if (s2 != s1) {
    old_cost += seq_trip_cost_mode(seq, s2, e2, c, ret2);
  }

  int t = seq[posa];
  seq[posa] = seq[posb];
  seq[posb] = t;

  long double new_cost = seq_trip_cost_mode(seq, s1, e1, c, ret1);
  if (s2 != s1) {
    new_cost += seq_trip_cost_mode(seq, s2, e2, c, ret2);
  }

  t = seq[posa];
  seq[posa] = seq[posb];
  seq[posb] = t;

  return revenue_delta - (new_cost - old_cost);
}

static void seq_band_boundary_swap(int *seq, int *len, int n, double c,
                                   const double *pow_d) {
  if (*len <= 2) {
    return;
  }

  int city_count = 0;
  for (int i = 0; i < *len; ++i) {
    if (seq[i] >= 0) {
      city_count++;
    }
  }
  if (city_count < 4) {
    return;
  }

  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  int *pref = (int *)malloc((size_t)(*len + 1) * sizeof(int));
  int *early = (int *)malloc((size_t)city_count * sizeof(int));
  int *late = (int *)malloc((size_t)city_count * sizeof(int));
  if (pref == NULL || early == NULL || late == NULL) {
    free(pref);
    free(early);
    free(late);
    return;
  }

  int k_cap = city_count > 20000 ? 140 : 80;
  if (k_cap > city_count) {
    k_cap = city_count;
  }

  for (int iter = 0; iter < 12; ++iter) {
    pref[0] = 0;
    for (int i = 0; i < *len; ++i) {
      pref[i + 1] = pref[i] + (seq[i] >= 0 ? 1 : 0);
    }

    int ecount = 0;
    int lcount = 0;
    for (int i = 0; i < *len; ++i) {
      if (seq[i] < 0) {
        continue;
      }
      int sold_before = pref[i];
      int band = sold_before / block;
      if (band > 9) {
        band = 9;
      }
      if (band <= 3) {
        ranked[ecount].idx = i;
        ranked[ecount].score = -cities[seq[i]].p;
        ecount++;
      }
      if (band >= 1) {
        ranked[lcount].idx = i;
        ranked[lcount].score = cities[seq[i]].p;
        lcount++;
      }
    }

    if (ecount == 0 || lcount == 0) {
      break;
    }

    qsort(ranked, (size_t)ecount, sizeof(RankedCity), cmp_ranked_desc);
    if (ecount > k_cap) {
      ecount = k_cap;
    }
    for (int i = 0; i < ecount; ++i) {
      early[i] = ranked[i].idx;
    }

    for (int i = 0; i < lcount; ++i) {
      ranked[i].idx = ranked[i].idx;
    }
    for (int i = 0, w = 0; i < *len; ++i) {
      if (seq[i] < 0) {
        continue;
      }
      int sold_before = pref[i];
      int band = sold_before / block;
      if (band > 9) {
        band = 9;
      }
      if (band >= 1) {
        ranked[w].idx = i;
        ranked[w].score = cities[seq[i]].p;
        w++;
      }
    }
    qsort(ranked, (size_t)lcount, sizeof(RankedCity), cmp_ranked_desc);
    if (lcount > k_cap) {
      lcount = k_cap;
    }
    for (int i = 0; i < lcount; ++i) {
      late[i] = ranked[i].idx;
    }

    long double best_gain = 0.0L;
    int best_e = -1;
    int best_l = -1;
    for (int li = 0; li < lcount; ++li) {
      int posa = late[li];
      int city_a = seq[posa];
      int band_a = pref[posa] / block;
      if (band_a > 9) {
        band_a = 9;
      }
      for (int ei = 0; ei < ecount; ++ei) {
        int posb = early[ei];
        int city_b = seq[posb];
        int band_b = pref[posb] / block;
        if (band_b > 9) {
          band_b = 9;
        }
        if (band_a <= band_b || posa == posb) {
          continue;
        }
        if (cities[city_a].p <= cities[city_b].p + 1e-12) {
          continue;
        }
        if (!seq_swap_geometry_ok(seq, *len, posa, posb)) {
          continue;
        }
        long double gain =
            seq_swap_gain_exact(seq, *len, n, c, pow_d, posa, posb, pref);
        if (gain > best_gain + 1e-12L) {
          best_gain = gain;
          best_e = posb;
          best_l = posa;
        }
      }
    }

    if (best_gain <= 1e-12L) {
      break;
    }

    int t = seq[best_e];
    seq[best_e] = seq[best_l];
    seq[best_l] = t;
  }

  seq_trim_best_prefix(seq, len, n, c, pow_d);
  free(pref);
  free(early);
  free(late);
}

static double sweep_solo_slack(double c, double d) {
  double slack;
  if (c <= 1.0) {
    slack = 1.40;
  } else if (c <= 2.0) {
    slack = 1.10;
  } else {
    slack = 0.85;
  }
  if (d < 0.90) {
    slack += 0.10;
  }
  if (slack > 1.80) {
    slack = 1.80;
  }
  return slack;
}

static int build_giant_tour_sweep(int *order, int n, double c, double d,
                                  int sector_count, double angle_shift,
                                  int reverse_dir, int secondary_mode) {
  int scount = 0;
  double slack = sweep_solo_slack(c, d);

  for (int i = 0; i < n; ++i) {
    if (!city_sellable(i)) {
      continue;
    }
    double dist0 = dist_xy(0, 0, cities[i].x, cities[i].y);
    double solo = cities[i].p - dist0 * (1.0 + c);
    if (solo <= -slack * dist0) {
      continue;
    }

    double ang = atan2((double)cities[i].y, (double)cities[i].x);
    double norm = (ang + M_PI) / (2.0 * M_PI) + angle_shift;
    norm -= floor(norm);
    int bucket = (int)(norm * sector_count);
    if (bucket < 0) {
      bucket = 0;
    }
    if (bucket >= sector_count) {
      bucket = sector_count - 1;
    }
    if (reverse_dir) {
      bucket = sector_count - 1 - bucket;
    }

    sector_cities[scount].idx = i;
    sector_cities[scount].bucket = bucket;
    if (secondary_mode == 0 || secondary_mode == 1) {
      sector_cities[scount].dist0 = dist0;
    } else if (secondary_mode == 2) {
      sector_cities[scount].dist0 = cities[i].p / (dist0 + 1.0);
    } else {
      sector_cities[scount].dist0 = cities[i].p - 0.35 * dist0 * (1.0 + c);
    }
    scount++;
  }

  if (scount == 0) {
    return 0;
  }

  sector_desc = (secondary_mode == 1 || secondary_mode >= 2) ? 1 : 0;
  qsort(sector_cities, (size_t)scount, sizeof(SectorCity), cmp_sector_city);
  for (int i = 0; i < scount; ++i) {
    order[i] = sector_cities[i].idx;
  }
  return scount;
}

static int split_giant_tour_to_seq(const int *order, int m, int n, double c,
                                   const double *pow_d, int max_trip_len,
                                   int *seq_out) {
  if (m <= 0) {
    return 0;
  }
  if (max_trip_len < 1) {
    max_trip_len = 1;
  }
  if (max_trip_len > MAX_TRIP_BUF) {
    max_trip_len = MAX_TRIP_BUF;
  }

  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  long double *rev_pref =
      (long double *)malloc((size_t)(m + 1) * sizeof(long double));
  long double *dp_ret =
      (long double *)malloc((size_t)(m + 1) * sizeof(long double));
  int *prev_ret = (int *)malloc((size_t)(m + 1) * sizeof(int));
  int *trip_starts = (int *)malloc((size_t)(m + 1) * sizeof(int));
  int *trip_ends = (int *)malloc((size_t)(m + 1) * sizeof(int));
  if (rev_pref == NULL || dp_ret == NULL || prev_ret == NULL ||
      trip_starts == NULL || trip_ends == NULL) {
    free(rev_pref);
    free(dp_ret);
    free(prev_ret);
    free(trip_starts);
    free(trip_ends);
    return 0;
  }

  rev_pref[0] = 0.0L;
  for (int i = 0; i < m; ++i) {
    int band = i / block;
    if (band > 9) {
      band = 9;
    }
    rev_pref[i + 1] =
        rev_pref[i] + (long double)cities[order[i]].p * (long double)pow_d[band];
  }

  const long double inf = 1e300L;
  for (int i = 0; i <= m; ++i) {
    dp_ret[i] = inf;
    prev_ret[i] = -1;
  }
  dp_ret[0] = 0.0L;

  long double best_profit = 0.0L;
  int best_last_start = -1;
  int best_end = 0;

  for (int i = 0; i < m; ++i) {
    if (dp_ret[i] >= inf / 4.0L) {
      continue;
    }

    long double path_len = 0.0L;
    long double sum_arrivals = 0.0L;
    int px = 0;
    int py = 0;

    for (int len = 1; len <= max_trip_len && i + len <= m; ++len) {
      int idx = order[i + len - 1];
      long double seg =
          (long double)dist_xy(px, py, cities[idx].x, cities[idx].y);
      path_len += seg;
      sum_arrivals += path_len;
      long double trip_no_return = path_len + (long double)c * sum_arrivals;
      long double trip_return =
          trip_no_return +
          (long double)dist_xy(cities[idx].x, cities[idx].y, 0, 0);
      int j = i + len;

      long double cand_ret = dp_ret[i] + trip_return;
      if (cand_ret + 1e-12L < dp_ret[j]) {
        dp_ret[j] = cand_ret;
        prev_ret[j] = i;
      }

      long double profit = rev_pref[j] - (dp_ret[i] + trip_no_return);
      if (profit > best_profit + 1e-12L) {
        best_profit = profit;
        best_last_start = i;
        best_end = j;
      }

      px = cities[idx].x;
      py = cities[idx].y;
    }
  }

  int seq_len = 0;
  if (best_end > 0 && best_last_start >= 0) {
    int trip_count = 0;
    int cur = best_last_start;
    while (cur > 0) {
      int prev = prev_ret[cur];
      if (prev < 0) {
        break;
      }
      trip_starts[trip_count] = prev;
      trip_ends[trip_count] = cur;
      trip_count++;
      cur = prev;
    }

    for (int t = trip_count - 1; t >= 0; --t) {
      if (seq_len > 0) {
        seq_out[seq_len++] = -1;
      }
      for (int i = trip_starts[t]; i < trip_ends[t]; ++i) {
        seq_out[seq_len++] = order[i];
      }
    }

    if (seq_len > 0) {
      seq_out[seq_len++] = -1;
    }
    for (int i = best_last_start; i < best_end; ++i) {
      seq_out[seq_len++] = order[i];
    }
  }

  free(rev_pref);
  free(dp_ret);
  free(prev_ret);
  free(trip_starts);
  free(trip_ends);
  return seq_len;
}

static Route build_multistart_sweep_split(int n, double c, double d,
                                          const double *pow_d,
                                          clock_t start_clock,
                                          double deadline_sec) {
  Route best_route = {0};
  double best_profit = -1e100;
  Route empty = {0};
  consider_route(&best_route, &best_profit, empty, n, c, d);

  int *order = (int *)malloc((size_t)n * sizeof(int));
  int *seq = (int *)malloc((size_t)(2 * n + 4) * sizeof(int));
  if (order == NULL || seq == NULL) {
    free(order);
    free(seq);
    return best_route;
  }

  int sectors_base;
  if (n <= 200) {
    sectors_base = 32;
  } else if (n <= 2000) {
    sectors_base = 96;
  } else if (n <= 20000) {
    sectors_base = 256;
  } else {
    sectors_base = 512;
  }

  int trip_base;
  if (n <= 200) {
    trip_base = 12;
  } else if (n <= 2000) {
    trip_base = 24;
  } else if (d < 0.90) {
    trip_base = 40;
  } else {
    trip_base = 56;
  }

  static const SweepVariant variants[] = {
      {0, 0, 0.00, 0, 0}, {0, 0, 0.125, 0, 0}, {0, 0, 0.00, 1, 0},
      {0, 0, 0.125, 0, 2}, {0, 0, 0.00, 0, 1}, {0, 0, 0.0625, 0, 3},
      {0, 0, 0.1875, 1, 2}, {0, 0, 0.25, 0, 0},
  };

  for (size_t vi = 0; vi < sizeof(variants) / sizeof(variants[0]); ++vi) {
    if (elapsed_seconds(start_clock) > deadline_sec - 0.05) {
      break;
    }

    SweepVariant v = variants[vi];
    v.trip_len = trip_base + (int)((vi % 3) * 8);
    v.sector_count = sectors_base + (int)((vi % 4) * (sectors_base / 4));
    if (v.sector_count < 16) {
      v.sector_count = 16;
    }

    int m = build_giant_tour_sweep(order, n, c, d, v.sector_count,
                                   v.angle_shift, v.reverse_dir,
                                   v.secondary_mode);
    if (m <= 0) {
      continue;
    }

    int seq_len =
        split_giant_tour_to_seq(order, m, n, c, pow_d, v.trip_len, seq);
    if (seq_len <= 0) {
      continue;
    }

    seq_trim_best_prefix(seq, &seq_len, n, c, pow_d);
    if (seq_len <= 0) {
      continue;
    }

    if (elapsed_seconds(start_clock) < deadline_sec - 0.10) {
      seq_band_safe_two_opt(seq, seq_len, n, c, pow_d);
    }
    if (elapsed_seconds(start_clock) < deadline_sec - 0.08) {
      seq_trip_order_optimize(seq, &seq_len, n, c, pow_d);
    }
    if (elapsed_seconds(start_clock) < deadline_sec - 0.05) {
      seq_band_boundary_swap(seq, &seq_len, n, c, pow_d);
    }
    if (elapsed_seconds(start_clock) < deadline_sec - 0.03) {
      seq_band_safe_two_opt(seq, seq_len, n, c, pow_d);
    }

    Route cand = seq_to_route(seq, seq_len);
    consider_route(&best_route, &best_profit, cand, n, c, d);
  }

  free(order);
  free(seq);
  return best_route;
}

static double elapsed_seconds(clock_t start_clock) {
  return (double)(clock() - start_clock) / (double)CLOCKS_PER_SEC;
}

static unsigned sa_rand_u32(unsigned *state) {
  unsigned x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

static void reverse_order_segment(int *arr, int l, int r) {
  while (l < r) {
    int t = arr[l];
    arr[l] = arr[r];
    arr[r] = t;
    l++;
    r--;
  }
}

static double eval_chunk_order(const int *order, int m, int n,
                               const double *pow_d, int *best_keep) {
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  int sold = 0;
  int keep = 0;
  double pref_profit_with_return = 0.0;
  double best_profit = 0.0;

  for (int pos = 0; pos < m; ++pos) {
    const Chunk *ch = &chunks[order[pos]];
    double revenue = 0.0;
    for (int j = 0; j < ch->len; ++j) {
      int idx = sector_cities[ch->start + j].idx;
      int band = (sold + j) / block;
      if (band > 9) {
        band = 9;
      }
      revenue += cities[idx].p * pow_d[band];
    }
    sold += ch->len;
    pref_profit_with_return += revenue - ch->cost_with_return;
    double candidate = pref_profit_with_return + ch->return_cost;
    if (candidate > best_profit) {
      best_profit = candidate;
      keep = pos + 1;
    }
  }

  *best_keep = keep;
  return best_profit;
}

static Route build_large_sa(int n, double c, double d, int max_trip_len,
                            int sector_count, int reverse_dist,
                            double sector_shift, const double *pow_d,
                            clock_t start_clock, double deadline_sec) {
  Route route = {0};
  int scount = 0;

  if (max_trip_len < 2) {
    max_trip_len = 2;
  }
  if (max_trip_len > MAX_TRIP_BUF) {
    max_trip_len = MAX_TRIP_BUF;
  }
  if (sector_count < 8) {
    sector_count = 8;
  }

  int sort_mode = d < 0.92 ? 1 : 0;
  double solo_slack;
  if (c <= 1.0) {
    solo_slack = 1.50;
  } else if (c <= 2.0) {
    solo_slack = 1.20;
  } else {
    solo_slack = 0.90;
  }
  if (d < 0.90) {
    solo_slack += 0.10;
  }
  if (solo_slack > 2.00) {
    solo_slack = 2.00;
  }

  for (int i = 0; i < n; ++i) {
    double dist0 = dist_xy(0, 0, cities[i].x, cities[i].y);
    double solo = cities[i].p - dist0 * (1.0 + c);
    if (solo <= -solo_slack * dist0) {
      continue;
    }
    double ang = atan2((double)cities[i].y, (double)cities[i].x);
    double norm = (ang + M_PI) / (2.0 * M_PI) + sector_shift;
    norm -= floor(norm);
    int bucket = (int)(norm * sector_count);
    if (bucket < 0) {
      bucket = 0;
    }
    if (bucket >= sector_count) {
      bucket = sector_count - 1;
    }
    sector_cities[scount].idx = i;
    sector_cities[scount].bucket = bucket;
    if (sort_mode == 1) {
      sector_cities[scount].dist0 = cities[i].p / (dist0 + 1.0);
    } else {
      sector_cities[scount].dist0 = dist0;
    }
    scount++;
  }

  if (scount == 0) {
    return route;
  }

  sector_desc = (sort_mode == 1) ? 1 : reverse_dist;
  qsort(sector_cities, (size_t)scount, sizeof(SectorCity), cmp_sector_city);

  int chunk_count = 0;
  int pos = 0;
  while (pos < scount) {
    int end = pos + 1;
    while (end < scount &&
           sector_cities[end].bucket == sector_cities[pos].bucket) {
      end++;
    }

    int p = pos;
    while (p < end) {
      double best_profit = 1e-12;
      int best_len = 0;
      double path_len = 0.0;
      double route_cost = 0.0;
      double route_rev = 0.0;
      int prevx = 0;
      int prevy = 0;

      int limit = end - p;
      if (limit > max_trip_len) {
        limit = max_trip_len;
      }

      for (int len = 1; len <= limit; ++len) {
        int idx = sector_cities[p + len - 1].idx;
        double seg = dist_xy(prevx, prevy, cities[idx].x, cities[idx].y);
        route_cost += c * path_len + seg * (1.0 + c);
        path_len += seg;
        route_rev += cities[idx].p;
        prevx = cities[idx].x;
        prevy = cities[idx].y;

        double full_profit =
            route_rev - route_cost - dist_xy(prevx, prevy, 0, 0);
        if (full_profit > best_profit) {
          best_profit = full_profit;
          best_len = len;
        }
      }

      if (best_len > 0) {
        chunks[chunk_count].start = p;
        chunks[chunk_count].len = best_len;
        chunks[chunk_count].est_profit = best_profit;
        chunks[chunk_count].sum_p = 0.0;
        chunks[chunk_count].max_p = 0.0;
        chunks[chunk_count].cost_with_return = 0.0;
        chunks[chunk_count].return_cost = 0.0;
        chunks[chunk_count].seed_score = 0.0;
        chunk_count++;
        p += best_len;
      } else {
        p++;
      }
    }

    pos = end;
  }

  if (chunk_count == 0) {
    return route;
  }

  double global_pmax = 0.0;
  for (int ci = 0; ci < chunk_count; ++ci) {
    Chunk *ch = &chunks[ci];
    double sum_p = 0.0;
    double max_p = 0.0;
    double cost = 0.0;
    int cargo = ch->len;
    int cx = 0;
    int cy = 0;

    for (int j = 0; j < ch->len; ++j) {
      int idx = sector_cities[ch->start + j].idx;
      double pval = cities[idx].p;
      if (pval > max_p) {
        max_p = pval;
      }
      if (pval > global_pmax) {
        global_pmax = pval;
      }
      sum_p += pval;
      double seg = dist_xy(cx, cy, cities[idx].x, cities[idx].y);
      cost += seg * (1.0 + c * cargo);
      cx = cities[idx].x;
      cy = cities[idx].y;
      cargo--;
    }

    double ret = dist_xy(cx, cy, 0, 0);
    ch->sum_p = sum_p;
    ch->max_p = max_p;
    ch->return_cost = ret;
    ch->cost_with_return = cost + ret;
  }

  double vip_thr = global_pmax * 0.90;
  for (int ci = 0; ci < chunk_count; ++ci) {
    Chunk *ch = &chunks[ci];
    double vip_mass = 0.0;
    for (int j = 0; j < ch->len; ++j) {
      int idx = sector_cities[ch->start + j].idx;
      double pval = cities[idx].p;
      if (pval > vip_thr) {
        vip_mass += pval - vip_thr;
      }
    }
    double density = ch->sum_p / (ch->cost_with_return + 1.0);
    ch->seed_score =
        ch->est_profit + 0.15 * vip_mass + 8.0 * density + 0.01 * ch->max_p;
  }

  int m = chunk_count;
  int *order_curr = (int *)malloc((size_t)m * sizeof(int));
  int *order_best = (int *)malloc((size_t)m * sizeof(int));
  int *order_seed = (int *)malloc((size_t)m * sizeof(int));
  bool *picked = (bool *)calloc((size_t)m, sizeof(bool));
  if (order_curr == NULL || order_best == NULL || order_seed == NULL ||
      picked == NULL) {
    free(order_curr);
    free(order_best);
    free(order_seed);
    free(picked);
    return route;
  }

  for (int i = 0; i < m; ++i) {
    ranked[i].idx = i;
    ranked[i].score = chunks[i].max_p;
  }
  qsort(ranked, (size_t)m, sizeof(RankedCity), cmp_ranked_desc);
  int vip_target = m < 6 ? m : 6;
  int head_len = 0;
  for (int i = 0; i < m && head_len < vip_target; ++i) {
    int ci = ranked[i].idx;
    if (chunks[ci].max_p < vip_thr) {
      break;
    }
    order_seed[head_len++] = ci;
    picked[ci] = true;
  }

  for (int i = 0; i < m; ++i) {
    ranked[i].idx = i;
    ranked[i].score = chunks[i].seed_score;
  }
  qsort(ranked, (size_t)m, sizeof(RankedCity), cmp_ranked_desc);
  for (int i = 0; i < m && head_len < m; ++i) {
    int ci = ranked[i].idx;
    if (picked[ci]) {
      continue;
    }
    order_seed[head_len++] = ci;
    picked[ci] = true;
  }
  for (int i = 0; i < m; ++i) {
    order_curr[i] = order_seed[i];
  }

  int keep_curr = 0;
  double cur_profit = eval_chunk_order(order_curr, m, n, pow_d, &keep_curr);
  int keep_best = keep_curr;
  double best_profit = cur_profit;
  memcpy(order_best, order_curr, (size_t)m * sizeof(int));

  for (int i = 0; i < m; ++i) {
    ranked[i].idx = i;
    ranked[i].score = chunks[i].est_profit;
  }
  qsort(ranked, (size_t)m, sizeof(RankedCity), cmp_ranked_desc);
  for (int i = 0; i < m; ++i) {
    order_curr[i] = ranked[i].idx;
  }
  int keep_alt = 0;
  double alt_profit = eval_chunk_order(order_curr, m, n, pow_d, &keep_alt);
  if (alt_profit > cur_profit) {
    cur_profit = alt_profit;
    keep_curr = keep_alt;
  } else {
    for (int i = 0; i < m; ++i) {
      order_curr[i] = order_seed[i];
    }
  }
  if (cur_profit > best_profit) {
    best_profit = cur_profit;
    keep_best = keep_curr;
    memcpy(order_best, order_curr, (size_t)m * sizeof(int));
  }

  int locked = 0;
  if (m >= 30 && d < 0.92) {
    locked = 2;
  }

  double mean_scale = 0.0;
  for (int i = 0; i < m; ++i) {
    mean_scale += fabs(chunks[i].est_profit);
  }
  mean_scale = mean_scale / (double)m;
  if (mean_scale < 10.0) {
    mean_scale = 10.0;
  }
  double temp = mean_scale * 0.25;
  double temp_min = mean_scale * 1e-4 + 1e-6;
  unsigned rng = (unsigned)(n * 1103515245u + (unsigned)(c * 1000.0) +
                            (unsigned)(d * 100000.0) + 1234567u);

  while (elapsed_seconds(start_clock) < deadline_sec) {
    int movable = m - locked;
    if (movable < 2) {
      break;
    }

    bool do_reverse = (sa_rand_u32(&rng) & 7u) == 0u;
    int i = locked + (int)(sa_rand_u32(&rng) % (unsigned)movable);
    int j = locked + (int)(sa_rand_u32(&rng) % (unsigned)movable);
    if (i == j) {
      continue;
    }
    if (i > j) {
      int t = i;
      i = j;
      j = t;
    }

    if (do_reverse) {
      reverse_order_segment(order_curr, i, j);
    } else {
      int t = order_curr[i];
      order_curr[i] = order_curr[j];
      order_curr[j] = t;
    }

    int keep_new = 0;
    double new_profit = eval_chunk_order(order_curr, m, n, pow_d, &keep_new);
    double delta = new_profit - cur_profit;

    bool accept = false;
    if (delta >= 0.0) {
      accept = true;
    } else {
      double x = delta / temp;
      if (x > -20.0) {
        double r = (double)(sa_rand_u32(&rng) & 0xFFFFFFu) / 16777216.0;
        if (exp(x) > r) {
          accept = true;
        }
      }
    }

    if (accept) {
      cur_profit = new_profit;
      keep_curr = keep_new;
      if (new_profit > best_profit) {
        best_profit = new_profit;
        keep_best = keep_new;
        memcpy(order_best, order_curr, (size_t)m * sizeof(int));
      }
    } else {
      if (do_reverse) {
        reverse_order_segment(order_curr, i, j);
      } else {
        int t = order_curr[i];
        order_curr[i] = order_curr[j];
        order_curr[j] = t;
      }
    }

    temp *= 0.9994;
    if (temp < temp_min) {
      temp = temp_min;
    }
  }

  if (keep_best > 0) {
    bool first = true;
    for (int pos2 = 0; pos2 < keep_best; ++pos2) {
      Chunk *ch = &chunks[order_best[pos2]];
      if (!first) {
        route_push(&route, -1, 0, false);
      }
      first = false;
      int idx0 = sector_cities[ch->start].idx;
      route_push(&route, idx0, ch->len, true);
      for (int j = 1; j < ch->len; ++j) {
        int idx = sector_cities[ch->start + j].idx;
        route_push(&route, idx, 0, false);
      }
    }
  }

  free(order_curr);
  free(order_best);
  free(order_seed);
  free(picked);
  return route;
}

static int seq_cleanup(int *seq, int len) {
  int w = 0;
  bool prev_marker = true;
  for (int i = 0; i < len; ++i) {
    int v = seq[i];
    if (v == -1) {
      if (prev_marker) {
        continue;
      }
      seq[w++] = -1;
      prev_marker = true;
    } else {
      seq[w++] = v;
      prev_marker = false;
    }
  }
  if (w > 0 && seq[w - 1] == -1) {
    w--;
  }
  return w;
}

static int seq_prev_marker(const int *seq, int pos) {
  for (int i = pos - 1; i >= 0; --i) {
    if (seq[i] == -1) {
      return i;
    }
  }
  return -1;
}

static int seq_next_marker(const int *seq, int len, int pos) {
  for (int i = pos + 1; i < len; ++i) {
    if (seq[i] == -1) {
      return i;
    }
  }
  return len;
}

static void seq_insert(int *seq, int *len, int pos, int value) {
  if (pos < 0) {
    pos = 0;
  }
  if (pos > *len) {
    pos = *len;
  }
  memmove(seq + pos + 1, seq + pos, (size_t)(*len - pos) * sizeof(int));
  seq[pos] = value;
  (*len)++;
}

static int seq_delete(int *seq, int *len, int pos) {
  int v = seq[pos];
  memmove(seq + pos, seq + pos + 1, (size_t)(*len - pos - 1) * sizeof(int));
  (*len)--;
  return v;
}

static long double seq_trip_cost(const int *seq, int start, int end, double c) {
  if (start >= end) {
    return 0.0L;
  }
  int cargo = end - start;
  int cx = 0;
  int cy = 0;
  long double cost = 0.0L;
  for (int i = start; i < end; ++i) {
    int idx = seq[i];
    double seg = dist_xy(cx, cy, cities[idx].x, cities[idx].y);
    cost += (long double)seg * (1.0L + (long double)c * (long double)cargo);
    cargo--;
    cx = cities[idx].x;
    cy = cities[idx].y;
  }
  cost += (long double)dist_xy(cx, cy, 0, 0);
  return cost;
}

static bool seq_mutate_split(int *seq, int *len, int cap, unsigned *rng,
                             int *marker_pos_out) {
  if (*len < 2 || *len + 1 >= cap) {
    return false;
  }
  for (int att = 0; att < 24; ++att) {
    int pos = (int)(sa_rand_u32(rng) % (unsigned)(*len - 1));
    if (seq[pos] >= 0 && seq[pos + 1] >= 0) {
      int ins = pos + 1;
      seq_insert(seq, len, ins, -1);
      if (marker_pos_out != NULL) {
        *marker_pos_out = ins;
      }
      return true;
    }
  }
  return false;
}

static bool seq_mutate_merge(int *seq, int *len, unsigned *rng,
                             int *marker_pos_out) {
  if (*len < 3) {
    return false;
  }
  for (int att = 0; att < 24; ++att) {
    int pos = (int)(sa_rand_u32(rng) % (unsigned)*len);
    if (seq[pos] == -1 && pos > 0 && pos < *len - 1 && seq[pos - 1] >= 0 &&
        seq[pos + 1] >= 0) {
      if (marker_pos_out != NULL) {
        *marker_pos_out = pos;
      }
      (void)seq_delete(seq, len, pos);
      return true;
    }
  }
  return false;
}

static bool seq_mutate_relocate(int *seq, int *len, int cap, unsigned *rng) {
  (void)cap;
  if (*len < 3) {
    return false;
  }
  for (int att = 0; att < 40; ++att) {
    int mpos = (int)(sa_rand_u32(rng) % (unsigned)*len);
    if (seq[mpos] != -1 || mpos == 0 || mpos == *len - 1 || seq[mpos - 1] < 0 ||
        seq[mpos + 1] < 0) {
      continue;
    }

    int left_start = seq_prev_marker(seq, mpos) + 1;
    int left_end = mpos - 1;
    int right_start = mpos + 1;
    int right_end = seq_next_marker(seq, *len, mpos) - 1;
    if (left_start > left_end || right_start > right_end) {
      continue;
    }

    bool left_to_right = (sa_rand_u32(rng) & 1u) != 0u;
    if (left_to_right) {
      int src =
          left_start + (int)(sa_rand_u32(rng) % (unsigned)(left_end - left_start + 1));
      int value = seq_delete(seq, len, src);
      if (src < mpos) {
        mpos--;
      }
      right_start = mpos + 1;
      right_end = seq_next_marker(seq, *len, mpos) - 1;
      int ins_range = right_end - right_start + 2;
      int ins = right_start + (int)(sa_rand_u32(rng) % (unsigned)ins_range);
      seq_insert(seq, len, ins, value);
      *len = seq_cleanup(seq, *len);
      return true;
    }

    int src = right_start +
              (int)(sa_rand_u32(rng) % (unsigned)(right_end - right_start + 1));
    int value = seq_delete(seq, len, src);
    left_start = seq_prev_marker(seq, mpos) + 1;
    left_end = mpos - 1;
    int ins_range = left_end - left_start + 2;
    int ins = left_start + (int)(sa_rand_u32(rng) % (unsigned)ins_range);
    seq_insert(seq, len, ins, value);
    *len = seq_cleanup(seq, *len);
    return true;
  }
  return false;
}

static bool seq_mutate_vip_pull(int *seq, int *len, int cap, unsigned *rng) {
  (void)cap;
  if (*len < 32) {
    return false;
  }

  int src = -1;
  double src_p = -1.0;
  int lo = *len / 3;
  if (lo < 1) {
    lo = 1;
  }
  for (int att = 0; att < 64; ++att) {
    int pos = lo + (int)(sa_rand_u32(rng) % (unsigned)(*len - lo));
    if (seq[pos] < 0) {
      continue;
    }
    double p = cities[seq[pos]].p;
    if (p > src_p) {
      src_p = p;
      src = pos;
    }
  }
  if (src < 0) {
    return false;
  }

  int head_lim = *len / 8;
  if (head_lim < 1) {
    head_lim = 1;
  }
  if (head_lim > 700) {
    head_lim = 700;
  }

  int dst = 0;
  for (int att = 0; att < 32; ++att) {
    int pos = (int)(sa_rand_u32(rng) % (unsigned)(head_lim + 1));
    if (pos == 0 || pos == *len || seq[pos - 1] < 0 || seq[pos] < 0) {
      dst = pos;
      break;
    }
    if ((sa_rand_u32(rng) & 1u) == 0u) {
      dst = pos;
      break;
    }
  }

  int v = seq_delete(seq, len, src);
  if (dst > src) {
    dst--;
  }
  if (dst < 0) {
    dst = 0;
  }
  if (dst > *len) {
    dst = *len;
  }
  seq_insert(seq, len, dst, v);
  *len = seq_cleanup(seq, *len);
  return true;
}

static bool seq_mutate_cross_exchange(int *seq, int *len, int cap,
                                      unsigned *rng) {
  (void)len;
  int n = *len;
  if (n < 10) {
    return false;
  }
  int *tmp = (int *)malloc((size_t)cap * sizeof(int));
  if (tmp == NULL) {
    return false;
  }

  bool ok = false;
  for (int att = 0; att < 48 && !ok; ++att) {
    int p1 = (int)(sa_rand_u32(rng) % (unsigned)n);
    int p2 = (int)(sa_rand_u32(rng) % (unsigned)n);
    if (seq[p1] < 0 || seq[p2] < 0) {
      continue;
    }
    int s1 = seq_prev_marker(seq, p1) + 1;
    int e1 = seq_next_marker(seq, n, p1);
    int s2 = seq_prev_marker(seq, p2) + 1;
    int e2 = seq_next_marker(seq, n, p2);
    if (s1 == s2 || e1 <= s1 || e2 <= s2) {
      continue;
    }

    if (s2 < s1) {
      int ts = s1;
      s1 = s2;
      s2 = ts;
      int te = e1;
      e1 = e2;
      e2 = te;
    }
    if (s2 < e1) {
      continue;
    }

    int len1 = e1 - s1;
    int len2 = e2 - s2;
    if (len1 <= 0 || len2 <= 0) {
      continue;
    }

    int max_take1 = len1 < 10 ? len1 : 10;
    int max_take2 = len2 < 10 ? len2 : 10;
    int take1 = 1 + (int)(sa_rand_u32(rng) % (unsigned)max_take1);
    int take2 = 1 + (int)(sa_rand_u32(rng) % (unsigned)max_take2);
    int a1 = s1 + (int)(sa_rand_u32(rng) % (unsigned)(len1 - take1 + 1));
    int a2 = s2 + (int)(sa_rand_u32(rng) % (unsigned)(len2 - take2 + 1));
    int b1 = a1 + take1;
    int b2 = a2 + take2;

    int w = 0;
    memcpy(tmp + w, seq, (size_t)a1 * sizeof(int));
    w += a1;
    memcpy(tmp + w, seq + a2, (size_t)take2 * sizeof(int));
    w += take2;
    memcpy(tmp + w, seq + b1, (size_t)(a2 - b1) * sizeof(int));
    w += a2 - b1;
    memcpy(tmp + w, seq + a1, (size_t)take1 * sizeof(int));
    w += take1;
    memcpy(tmp + w, seq + b2, (size_t)(n - b2) * sizeof(int));
    w += n - b2;

    if (w <= 0 || w >= cap) {
      continue;
    }
    memcpy(seq, tmp, (size_t)w * sizeof(int));
    *len = seq_cleanup(seq, w);
    ok = true;
  }

  free(tmp);
  return ok;
}

static double seq_insert_score(const int *seq, int len, int pos, int city_idx,
                               const int *pref_city, int n, double c,
                               const double *pow_d) {
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }
  int sold_before = pref_city[pos];
  int band = sold_before / block;
  if (band > 9) {
    band = 9;
  }
  double rev = cities[city_idx].p * pow_d[band];

  int px = 0;
  int py = 0;
  if (pos > 0 && seq[pos - 1] >= 0) {
    px = cities[seq[pos - 1]].x;
    py = cities[seq[pos - 1]].y;
  }
  int nx = 0;
  int ny = 0;
  if (pos < len && seq[pos] >= 0) {
    nx = cities[seq[pos]].x;
    ny = cities[seq[pos]].y;
  }

  double removed = dist_xy(px, py, nx, ny);
  double added =
      dist_xy(px, py, cities[city_idx].x, cities[city_idx].y) +
      dist_xy(cities[city_idx].x, cities[city_idx].y, nx, ny);

  int pm = seq_prev_marker(seq, pos);
  int nm = seq_next_marker(seq, len, pos - 1);
  int trip_len = pref_city[nm] - pref_city[pm + 1];
  int before_in_trip = pref_city[pos] - pref_city[pm + 1];
  int cargo_guess = trip_len - before_in_trip + 1;
  if (cargo_guess < 1) {
    cargo_guess = 1;
  }

  double delta_cost = (added - removed) * (1.0 + c * (double)cargo_guess);
  return rev - delta_cost;
}

static bool seq_mutate_destroy_repair(int *seq, int *len, int cap, int n,
                                      double c, const double *pow_d,
                                      unsigned *rng) {
  int city_total = 0;
  for (int i = 0; i < *len; ++i) {
    if (seq[i] >= 0) {
      city_total++;
    }
  }
  if (city_total < 20) {
    return false;
  }

  int target = (int)((double)city_total *
                     (0.10 + 0.05 * (double)(sa_rand_u32(rng) & 0xFFFFu) /
                                 65535.0));
  if (target < 10) {
    target = 10;
  }
  /* Runtime cap: still performs large-neighborhood perturbation on big tests. */
  if (target > 900) {
    target = 900;
  }
  if (target > city_total - 2) {
    target = city_total - 2;
  }
  if (target <= 0) {
    return false;
  }

  int *removed = (int *)malloc((size_t)target * sizeof(int));
  int *pref = (int *)malloc((size_t)(cap + 1) * sizeof(int));
  if (removed == NULL || pref == NULL) {
    free(removed);
    free(pref);
    return false;
  }

  double prob = (double)target / (double)city_total;
  int rc = 0;
  int w = 0;
  for (int i = 0; i < *len; ++i) {
    int v = seq[i];
    if (v >= 0 && rc < target) {
      double u = (double)(sa_rand_u32(rng) & 0xFFFFFFu) / 16777216.0;
      if (u < prob) {
        removed[rc++] = v;
        continue;
      }
    }
    seq[w++] = v;
  }
  *len = seq_cleanup(seq, w);

  if (rc == 0) {
    free(removed);
    free(pref);
    return false;
  }

  qsort(removed, (size_t)rc, sizeof(int), cmp_city_p_desc_idx);

  for (int ri = 0; ri < rc; ++ri) {
    int city = removed[ri];
    pref[0] = 0;
    for (int i = 0; i < *len; ++i) {
      pref[i + 1] = pref[i] + (seq[i] >= 0 ? 1 : 0);
    }

    int best_pos = 0;
    double best_score = -1e300;
    int sample_cnt = *len < 48 ? *len + 1 : 48;

    for (int si = 0; si < sample_cnt; ++si) {
      int pos;
      if (si == 0) {
        pos = 0;
      } else if (si == 1) {
        pos = *len;
      } else if (si == 2) {
        int early = *len / 10;
        if (early < 1) {
          early = 1;
        }
        pos = (int)(sa_rand_u32(rng) % (unsigned)(early + 1));
      } else if (si == 3) {
        pos = (int)(sa_rand_u32(rng) % (unsigned)(*len + 1));
      } else {
        int pivot = (int)(sa_rand_u32(rng) % (unsigned)(*len + 1));
        int pm = seq_prev_marker(seq, pivot);
        int nm = seq_next_marker(seq, *len, pivot - 1);
        if ((sa_rand_u32(rng) & 1u) && pm + 1 <= *len) {
          pos = pm + 1;
        } else {
          pos = nm;
        }
      }
      if (pos < 0) {
        pos = 0;
      }
      if (pos > *len) {
        pos = *len;
      }
      double sc = seq_insert_score(seq, *len, pos, city, pref, n, c, pow_d);
      if (sc > best_score) {
        best_score = sc;
        best_pos = pos;
      }
    }

    /* Allow dropping weak cities so ALNS can change visited-city set. */
    double drop_margin = -0.03 * cities[city].p;
    if (best_score < drop_margin) {
      continue;
    }

    if (*len + 1 >= cap) {
      break;
    }
    seq_insert(seq, len, best_pos, city);
  }

  *len = seq_cleanup(seq, *len);
  free(removed);
  free(pref);
  return true;
}

static int route_to_seq(const Route *route, int *seq, int cap) {
  int len = 0;
  for (int i = 0; i < route->size; ++i) {
    if (len >= cap) {
      break;
    }
    const Step *st = &route->data[i];
    if (st->city_idx == -1) {
      seq[len++] = -1;
    } else {
      seq[len++] = st->city_idx;
    }
  }
  return seq_cleanup(seq, len);
}

static long double seq_profit_exact(const int *seq, int len, int n, double c,
                                    const double *pow_d) {
  int block = n / 10;
  if (block < 1) {
    block = 1;
  }

  int sold = 0;
  long double revenue = 0.0L;
  long double cost = 0.0L;

  int i = 0;
  while (i < len) {
    while (i < len && seq[i] == -1) {
      i++;
    }
    if (i >= len) {
      break;
    }

    int j = i;
    while (j < len && seq[j] != -1) {
      j++;
    }

    int cargo = j - i;
    int cx = 0;
    int cy = 0;
    for (int k = i; k < j; ++k) {
      int idx = seq[k];
      int nx = cities[idx].x;
      int ny = cities[idx].y;
      double seg = dist_xy(cx, cy, nx, ny);
      cost += (long double)seg * (1.0L + (long double)c * (long double)cargo);
      int band = sold / block;
      if (band > 9) {
        band = 9;
      }
      revenue += (long double)cities[idx].p * (long double)pow_d[band];
      sold++;
      cargo--;
      cx = nx;
      cy = ny;
    }

    if (j < len) {
      cost += (long double)dist_xy(cx, cy, 0, 0);
    }
    i = j + 1;
  }

  return revenue - cost;
}

static Route seq_to_route(const int *seq, int len) {
  Route route = {0};

  int i = 0;
  while (i < len) {
    while (i < len && seq[i] == -1) {
      i++;
    }
    if (i >= len) {
      break;
    }

    int j = i;
    while (j < len && seq[j] != -1) {
      j++;
    }
    int trip_len = j - i;
    if (trip_len > 0) {
      route_push(&route, seq[i], trip_len, true);
      for (int k = i + 1; k < j; ++k) {
        route_push(&route, seq[k], 0, false);
      }
      if (j < len) {
        route_push(&route, -1, 0, false);
      }
    }
    i = j + 1;
  }

  return route;
}

static Route optimize_route_dynamic_sa(const Route *base, int n, double c,
                                       const double *pow_d,
                                       clock_t start_clock,
                                       double deadline_sec) {
  Route empty = {0};
  if (base->size <= 1 || elapsed_seconds(start_clock) >= deadline_sec) {
    return empty;
  }

  int cap = base->size * 2 + 8;
  if (cap < 32) {
    cap = 32;
  }

  int *seq_cur = (int *)malloc((size_t)cap * sizeof(int));
  int *seq_cand = (int *)malloc((size_t)cap * sizeof(int));
  int *seq_best = (int *)malloc((size_t)cap * sizeof(int));
  if (seq_cur == NULL || seq_cand == NULL || seq_best == NULL) {
    free(seq_cur);
    free(seq_cand);
    free(seq_best);
    return empty;
  }

  int cur_len = route_to_seq(base, seq_cur, cap);
  if (cur_len <= 0) {
    free(seq_cur);
    free(seq_cand);
    free(seq_best);
    return empty;
  }

  long double cur_profit = seq_profit_exact(seq_cur, cur_len, n, c, pow_d);
  long double best_profit = cur_profit;
  int best_len = cur_len;
  memcpy(seq_best, seq_cur, (size_t)cur_len * sizeof(int));

  long double sum_p = 0.0L;
  int city_cnt = 0;
  for (int i = 0; i < cur_len; ++i) {
    if (seq_cur[i] >= 0) {
      sum_p += (long double)cities[seq_cur[i]].p;
      city_cnt++;
    }
  }
  long double avg_p = city_cnt > 0 ? sum_p / (long double)city_cnt : 1000.0L;
  double t0 = (double)(avg_p * 5.0L + fabsl(cur_profit) * 1e-6L);
  if (t0 < 50.0) {
    t0 = 50.0;
  }
  double temp = t0;
  double temp_min = t0 * 1e-4 + 1e-6;
  int stagnation = 0;
  int iters = 0;

  unsigned rng = (unsigned)(n * 2654435761u + (unsigned)(c * 1000.0) + 17u);
  enum {
    OP_SPLIT = 0,
    OP_MERGE = 1,
    OP_RELOCATE = 2,
    OP_XCHG = 3,
    OP_DR = 4,
    OP_VIP = 5
  };
  double op_w[6] = {2.4, 2.0, 1.2, 0.9, 0.8, 1.1};

  while (elapsed_seconds(start_clock) < deadline_sec) {
    iters++;
    int cand_len = cur_len;
    memcpy(seq_cand, seq_cur, (size_t)cur_len * sizeof(int));

    double wsum = op_w[0] + op_w[1] + op_w[2] + op_w[3] + op_w[4] + op_w[5];
    double pick = ((double)(sa_rand_u32(&rng) & 0xFFFFFFu) / 16777216.0) * wsum;
    int op = OP_VIP;
    double acc = op_w[0];
    if (pick < acc) {
      op = OP_SPLIT;
    } else {
      acc += op_w[1];
      if (pick < acc) {
        op = OP_MERGE;
      } else {
        acc += op_w[2];
        if (pick < acc) {
          op = OP_RELOCATE;
        } else {
          acc += op_w[3];
          if (pick < acc) {
            op = OP_XCHG;
          } else {
            acc += op_w[4];
            if (pick < acc) {
              op = OP_DR;
            }
          }
        }
      }
    }

    bool ok = false;
    int marker_pos = -1;
    if (op == OP_SPLIT) {
      ok = seq_mutate_split(seq_cand, &cand_len, cap, &rng, &marker_pos);
    } else if (op == OP_MERGE) {
      ok = seq_mutate_merge(seq_cand, &cand_len, &rng, &marker_pos);
    } else if (op == OP_RELOCATE) {
      ok = seq_mutate_relocate(seq_cand, &cand_len, cap, &rng);
    } else if (op == OP_XCHG) {
      ok = seq_mutate_cross_exchange(seq_cand, &cand_len, cap, &rng);
    } else if (op == OP_DR) {
      ok = seq_mutate_destroy_repair(seq_cand, &cand_len, cap, n, c, pow_d, &rng);
    } else {
      ok = seq_mutate_vip_pull(seq_cand, &cand_len, cap, &rng);
    }
    if (!ok || cand_len <= 0) {
      continue;
    }

    /* Fast proxy for D&R: skip obviously overfragmented candidates. */
    if (op == OP_DR) {
      int marker_cnt = 0;
      for (int i = 0; i < cand_len; ++i) {
        if (seq_cand[i] == -1) {
          marker_cnt++;
        }
      }
      int cur_markers = 0;
      for (int i = 0; i < cur_len; ++i) {
        if (seq_cur[i] == -1) {
          cur_markers++;
        }
      }
      if (marker_cnt > cur_markers + 300) {
        continue;
      }
    }

    long double cand_profit;
    bool used_fast_delta = false;
    if (op == OP_SPLIT && marker_pos > 0 && marker_pos < cand_len - 1) {
      int old_start = seq_prev_marker(seq_cur, marker_pos) + 1;
      int old_end = seq_next_marker(seq_cur, cur_len, marker_pos - 1);
      if (old_start < marker_pos && marker_pos < old_end) {
        long double old_cost = seq_trip_cost(seq_cur, old_start, old_end, c);
        int new_start = seq_prev_marker(seq_cand, marker_pos) + 1;
        int new_end = seq_next_marker(seq_cand, cand_len, marker_pos);
        long double left_cost = seq_trip_cost(seq_cand, new_start, marker_pos, c);
        long double right_cost =
            seq_trip_cost(seq_cand, marker_pos + 1, new_end, c);
        cand_profit = cur_profit + old_cost - (left_cost + right_cost);
        used_fast_delta = true;
      }
    } else if (op == OP_MERGE && marker_pos > 0 && marker_pos < cur_len - 1) {
      int left_start = seq_prev_marker(seq_cur, marker_pos) + 1;
      int left_end = marker_pos;
      int right_start = marker_pos + 1;
      int right_end = seq_next_marker(seq_cur, cur_len, marker_pos);
      if (left_start < left_end && right_start < right_end) {
        long double old_cost =
            seq_trip_cost(seq_cur, left_start, left_end, c) +
            seq_trip_cost(seq_cur, right_start, right_end, c);
        int merged_end = right_end - 1;
        long double new_cost =
            seq_trip_cost(seq_cand, left_start, merged_end, c);
        cand_profit = cur_profit + old_cost - new_cost;
        used_fast_delta = true;
      }
    }
    if (!used_fast_delta) {
      cand_profit = seq_profit_exact(seq_cand, cand_len, n, c, pow_d);
    }
    long double delta = cand_profit - cur_profit;
    bool accept = false;
    if (delta >= 0.0L) {
      accept = true;
    } else {
      double x = (double)(delta / (long double)temp);
      if (x > -20.0) {
        double u = (double)(sa_rand_u32(&rng) & 0xFFFFFFu) / 16777216.0;
        if (exp(x) > u) {
          accept = true;
        }
      }
    }

    if (accept) {
      int *tmp_ptr = seq_cur;
      seq_cur = seq_cand;
      seq_cand = tmp_ptr;
      cur_len = cand_len;
      cur_profit = cand_profit;
      op_w[op] += 0.8;
      if (cand_profit > best_profit) {
        best_profit = cand_profit;
        best_len = cand_len;
        memcpy(seq_best, seq_cur, (size_t)cand_len * sizeof(int));
        op_w[op] += 2.2;
        stagnation = 0;
      } else {
        stagnation++;
      }
    } else {
      op_w[op] *= 0.998;
      if (op_w[op] < 0.20) {
        op_w[op] = 0.20;
      }
      stagnation++;
    }

    temp *= 0.9994;
    if (temp < temp_min) {
      temp = temp_min;
    }

    if ((iters & 255) == 0) {
      for (int i = 0; i < 6; ++i) {
        if (op_w[i] > 8.0) {
          op_w[i] = 8.0;
        }
      }
    }

    if (stagnation > 1400) {
      temp *= 1.35;
      if (temp > t0) {
        temp = t0;
      }
      memcpy(seq_cur, seq_best, (size_t)best_len * sizeof(int));
      cur_len = best_len;
      cur_profit = best_profit;
      stagnation = 0;
    }
  }

  Route out = seq_to_route(seq_best, best_len);
  free(seq_cur);
  free(seq_cand);
  free(seq_best);
  return out;
}

int main(void) {
  FILE *in = stdin;
  FILE *out = stdout;
  int n;
  double c;
  double d;

  if (fscanf(in, "%d %lf %lf", &n, &c, &d) != 3) {
    in = fopen("input.txt", "r");
    if (in == NULL) {
      return 0;
    }
    out = fopen("output.txt", "w");
    if (out == NULL) {
      fclose(in);
      return 0;
    }
    if (fscanf(in, "%d %lf %lf", &n, &c, &d) != 3) {
      fclose(in);
      fclose(out);
      return 0;
    }
  }

  cities = (City *)malloc((size_t)n * sizeof(City));
  tree = (KDNode *)malloc((size_t)n * sizeof(KDNode));
  ord = (int *)malloc((size_t)n * sizeof(int));
  used = (bool *)calloc((size_t)n, sizeof(bool));
  chosen = (bool *)calloc((size_t)n, sizeof(bool));
  is_hq_city = (bool *)calloc((size_t)n, sizeof(bool));
  ranked = (RankedCity *)malloc((size_t)n * sizeof(RankedCity));
  sector_cities = (SectorCity *)malloc((size_t)n * sizeof(SectorCity));
  chunks = (Chunk *)malloc((size_t)n * sizeof(Chunk));
  if (cities == NULL || tree == NULL || ord == NULL || used == NULL ||
      chosen == NULL || is_hq_city == NULL || ranked == NULL ||
      sector_cities == NULL || chunks == NULL) {
    if (in != stdin)
      fclose(in);
    if (out != stdout)
      fclose(out);
    free(cities);
    free(tree);
    free(ord);
    free(used);
    free(chosen);
    free(is_hq_city);
    free(ranked);
    free(sector_cities);
    free(chunks);
    return 0;
  }

  for (int i = 0; i < n; ++i) {
    fscanf(in, "%d %d %lf", &cities[i].x, &cities[i].y, &cities[i].p);
    cities[i].id = i;
    ord[i] = i;
    is_hq_city[i] = (cities[i].x == 0 && cities[i].y == 0);
  }

  if (in != stdin) {
    fclose(in);
  }

  int root = build_kd(0, n - 1, 0);
  (void)root;

  double pow_d[10];
  pow_d[0] = 1.0;
  for (int i = 1; i < 10; ++i) {
    pow_d[i] = pow_d[i - 1] * d;
  }

  Route best_route = {0};
  double best_profit = -1e100;
  clock_t start_clock = clock();
  Route empty = {0};
  consider_route(&best_route, &best_profit, empty, n, c, d);

  if (n <= 12) {
    Route exact = build_small_exact(n, c, pow_d);
    consider_route(&best_route, &best_profit, exact, n, c, d);
  } else {
    const char *deadline_env = getenv("SA_DEADLINE");
    double deadline = deadline_env != NULL ? atof(deadline_env) : 1.85;
    if (deadline < 0.40) {
      deadline = 0.40;
    }
    if (deadline > 1.95) {
      deadline = 1.95;
    }

    Route sweep_split =
        build_multistart_sweep_split(n, c, d, pow_d, start_clock, deadline);
    consider_route(&best_route, &best_profit, sweep_split, n, c, d);
  }

  output_route(&best_route, out);

  if (out != stdout) {
    fclose(out);
  }

  free(best_route.data);
  free(cities);
  free(tree);
  free(ord);
  free(used);
  free(chosen);
  free(is_hq_city);
  free(ranked);
  free(sector_cities);
  free(chunks);
  return 0;
}
