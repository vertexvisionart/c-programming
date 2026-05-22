

void dgemv(int m, int n, double alpha, const double *A, const double *X,
           double beta, double *Y) {
  for (int i = 0; i < m; i++) {
    double dot_product = 0.0;
    for (int j = 0; j < n; j++) {
      dot_product += A[i * n + j] * X[j];
    }
    Y[i] = dot_product * alpha + beta * Y[i];
  }
}

void dger(int m, int n, double alpha, const double *X, const double *Y,
          double *A) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      A[i * n + j] += alpha * X[i] * Y[j];
    }
  }
}
