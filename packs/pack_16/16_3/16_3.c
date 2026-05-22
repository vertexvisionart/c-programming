#include <cblas.h>
#include <stdio.h>
int main() {
  int n = 3;
  double x[] = {1.0, 2.0, 3.0};
  double y[] = {4.0, 5.0, 6.0};
  double result;
  // Скалярное произведение: result = x^T * y
  result = cblas_ddot(n, x, 1, y, 1);
  printf("Скалярное произведение: %.2f\n", result); // Ожидаем 32.0
  return 0;
}
