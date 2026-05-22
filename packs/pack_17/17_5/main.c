#include <math.h>
#include <stdio.h>

double get_speed(double dt, double cx1, double cx2, double cx3, double cy1,
                 double cy2, double cy3, double cz1, double cz2, double cz3) {

  double dx = cx1 + 2.0 * cx2 * dt + 3.0 * cx3 * dt * dt;
  double dy = cy1 + 2.0 * cy2 * dt + 3.0 * cy3 * dt * dt;
  double dz = cz1 + 2.0 * cz2 * dt + 3.0 * cz3 * dt * dt;

  return sqrt(dx * dx + dy * dy + dz * dz);
}

int main(void) {
  int n;
  if (scanf("%d", &n) != 1)
    return 0;

  double total_length = 0.0;

  int STEPS = 10000;

  for (int i = 0; i < n; i++) {
    double l, r;
    scanf("%lf %lf", &l, &r);

    double cx0, cx1, cx2, cx3;
    double cy0, cy1, cy2, cy3;
    double cz0, cz1, cz2, cz3;

    scanf("%lf %lf %lf %lf", &cx0, &cx1, &cx2, &cx3);
    scanf("%lf %lf %lf %lf", &cy0, &cy1, &cy2, &cy3);
    scanf("%lf %lf %lf %lf", &cz0, &cz1, &cz2, &cz3);

    double H = r - l;

    double h = H / STEPS;

    double span_integral =
        get_speed(0.0, cx1, cx2, cx3, cy1, cy2, cy3, cz1, cz2, cz3) +
        get_speed(H, cx1, cx2, cx3, cy1, cy2, cy3, cz1, cz2, cz3);

    for (int j = 1; j < STEPS; j++) {
      double dt = j * h;

      double weight = (j % 2 == 0) ? 2.0 : 4.0;
      span_integral +=
          weight * get_speed(dt, cx1, cx2, cx3, cy1, cy2, cy3, cz1, cz2, cz3);
    }

    span_integral *= (h / 3.0);
    total_length += span_integral;
  }

  printf("%.15lf\n", total_length);

  return 0;
}
