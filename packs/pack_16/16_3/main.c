#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern void Init(const int *arr, int n);
extern int Query(int l, int64_t sum);

int main(void) {
  int n, m;
  scanf("%d %d", &n, &m);

  int *arr = (int *)malloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) {
    scanf("%d", &arr[i]);
  }
  Init(arr, n);

  int l;
  int64_t s;
  for (int i = 0; i < m; i++) {
    scanf("%d %ld", &l, &s);
    int r = Query(l, s);
    printf("%d\n", r);
  }
  free(arr);
  return 0;
}
