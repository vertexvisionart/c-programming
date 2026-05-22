#include <stdio.h>

extern double Function(double x);

int main() {

  FILE *fin = fopen("input.txt", "r");
  FILE *fout = fopen("output.txt", "w");

  if (!fin || !fout) {
    return 1;
  }

  int M;
  if (fscanf(fin, "%d", &M) != 1) {
    return 1;
  }

  double h = 1e-5;

  for (int i = 0; i < M; i++) {
    double x;
    fscanf(fin, "%lf", &x);

    double derivative = 0.0;

    if (x >= h && x <= 1.0 - h) {
      derivative = (Function(x + h) - Function(x - h)) / (2.0 * h);
    }

    else if (x < h) {
      derivative =
          (-3.0 * Function(x) + 4.0 * Function(x + h) - Function(x + 2.0 * h)) /
          (2.0 * h);
    }

    else {
      derivative =
          (3.0 * Function(x) - 4.0 * Function(x - h) + Function(x - 2.0 * h)) /
          (2.0 * h);
    }

    fprintf(fout, "%.15lf\n", derivative);
  }

  fclose(fin);
  fclose(fout);

  return 0;
}
