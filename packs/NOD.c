#include <stdio.h>

int NOD(int a, int b) {
  if (b == 0)
    return a;
  return NOD(b, a % b);
}

int main(void) {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int T;
  scanf("%d", &T);
  for (int i = 0; i < T; i++) {
    int a, b;
    scanf("%d %d", &a, &b);
    printf("%d\n", NOD(a, b));
  }

  return 0;
}
