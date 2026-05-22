#include <stdio.h>
#include <stdlib.h>

typedef struct Point {
  long long x;
  long long y;
} Point;

long long distSq(Point a, Point b) {
  long long dx = a.x - b.x;
  long long dy = a.y - b.y;
  return dx * dx + dy * dy;
}

long long cross_product(Point a, Point b, Point c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

int main(void) {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int n;
  if (scanf("%d", &n) != 1)
    return 0;

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
  int *ans_idx = (int *)malloc(sizeof(int) * n);
  int ans_idx_cnt = 0;

  do {
    ans_idx[ans_idx_cnt++] = current;

    int next = (current + 1) % n;

    for (int i = 0; i < n; i++) {

      long long cp = cross_product(coord[current], coord[next], coord[i]);

      if (cp < 0) {
        next = i;
      }

      else if (cp == 0) {
        if (distSq(coord[current], coord[i]) >
            distSq(coord[current], coord[next])) {
          next = i;
        }
      }
    }

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
