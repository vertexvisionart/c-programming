#include <stdlib.h>

double Function(double x) {
  // The trap! It crashes if we calculate outside the [0, 1] range
  if (x < 0.0 || x > 1.0) {
    exit(666);
  }

  // The mathematical function from the example: f(x) = x^2 - x
  return x * x - x;
}
