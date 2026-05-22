#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.1415926535897932384
#define MINT 1e-9

typedef struct Point {
  long long x;
  long long y;
} Point;

long long product(Point a, Point b) {
  long long res = a.x * b.x + a.y * b.y;
  return res;
}

long long dist(Point vector1, Point vector2) {
  long long dx = vector1.x - vector2.x;
  long long dy = vector1.y - vector2.y;
  long long d = dx * dx + dy * dy;
  return d;
}

double lenght_vec(Point a) {
  double res = sqrt(a.x * a.x + a.y * a.y);
  return res;
}

double cosinus(long long product, double lenght_vec_1, double lenght_vec_2) {
  double res = product / (lenght_vec_1 * lenght_vec_2);
  if (res < -1.0) {
    return -1.0;
  }
  if (res > 1.0) {
    return 1.0;
  }
  return res;
}

long long cross_product(Point a, Point b) {
  long long cp;
  cp = a.x * b.y - b.x * a.y;
  return cp;
}

double get_angle(Point vector1, Point vector2) {
  long long product_vectors = product(vector1, vector2);

  double lenght_vector1 = lenght_vec(vector1);
  double lenght_vector2 = lenght_vec(vector2);

  if (lenght_vector1 < MINT || lenght_vector2 < MINT) {
    return 0.0;
  }

  double cos = product_vectors / (lenght_vector1 * lenght_vector2);

  if (cos < -1.0) {
    return -1.0;
  }
  if (cos > 1.0) {
    return 1.0;
  }

  double angle = acos(cos);

  long long cp = cross_product(vector1, vector2);
  if (cp < 0) {
    angle = 2 * PI - angle;
  }

  return angle;
}

int main(void) {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int n;
  scanf("%d", &n);

  Point *coord = (Point *)malloc(sizeof(Point) * n);

  int start_idx = 0;
  for (int i = 0; i < n; i++) {
    scanf("%lld %lld", &coord[i].x, &coord[i].y);
    if ((coord[i].y < coord[start_idx].y) ||
        (coord[i].y == coord[start_idx].y && coord[i].x < coord[start_idx].x)) {
      start_idx = i;
    }
  }

  int current = start_idx;
  Point prev_vec = {1, 0};
  int *ans_idx = (int *)malloc(sizeof(int) * n);
  int ans_idx_cnt = 0;

  do {
    ans_idx[ans_idx_cnt++] = current;

    int next = -1;
    double min_angle = 1000.0;
    long long max_dist = -1;

    for (int i = 0; i < n; i++) {
      if (i == current) {
        continue;
      }

      Point curr_vec;
      curr_vec.x = coord[i].x - coord[current].x;
      curr_vec.y = coord[i].y - coord[current].y;

      double angle = get_angle(prev_vec, curr_vec);

      if (next == -1 || angle < min_angle - MINT) {
        min_angle = angle;
        next = i;
        max_dist = dist(coord[current], coord[i]);
      } else if (fabs(angle - min_angle) < MINT) {
        long long d = dist(coord[current], coord[i]);
        if (d > max_dist) {
          max_dist = d;
          next = i;
        }
      }
    }
    prev_vec.x = coord[next].x - coord[current].x;
    prev_vec.y = coord[next].y - coord[current].y;
    current = next;

  } while (current != start_idx);

  int output_start_pos = 0;
  for (int i = 1; i < ans_idx_cnt; i++) {
    Point p_curr = coord[ans_idx[i]];
    Point p_best = coord[ans_idx[output_start_pos]];

    if (p_curr.x < p_best.x || (p_curr.x == p_best.x && p_curr.y < p_best.y)) {
      output_start_pos = i;
    }
  }

  printf("%d\n", ans_idx_cnt);

  for (int i = 0; i < ans_idx_cnt; i++) {
    int idx = (output_start_pos + i) % ans_idx_cnt;
    printf("%lld %lld\n", coord[ans_idx[idx]].x, coord[ans_idx[idx]].y);
  }

  free(ans_idx);
  free(coord);

  return 0;
}
