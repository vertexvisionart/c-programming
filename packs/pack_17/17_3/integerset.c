#include "integerset.h"
#include <stdlib.h>

struct IntegerSet {
  int *data;
  int size;
};

static int compare_ints(const void *a, const void *b) {
  int val1 = *(const int *)a;
  int val2 = *(const int *)b;
  if (val1 < val2)
    return -1;
  if (val1 > val2)
    return 1;
  return 0;
}

EXPORT_API IntegerSet *CreateSet(const int *arr, int size) {
  if (size < 0)
    return 0;

  IntegerSet *set = (IntegerSet *)malloc(sizeof(IntegerSet));
  if (!set)
    return 0;

  set->size = size;

  if (size == 0) {
    set->data = 0;
    return set;
  }

  set->data = (int *)malloc(size * sizeof(int));
  if (!set->data) {
    free(set);
    return 0;
  }

  for (int i = 0; i < size; i++) {
    set->data[i] = arr[i];
  }

  qsort(set->data, size, sizeof(int), compare_ints);

  for (int i = 0; i < size - 1; i++) {
    if (set->data[i] == set->data[i + 1]) {
      free(set->data);
      free(set);
      return 0;
    }
  }

  return set;
}

EXPORT_API int IsInSet(IntegerSet *set, int value) {

  if (!set || set->size == 0 || !set->data) {
    return 0;
  }

  if (bsearch(&value, set->data, set->size, sizeof(int), compare_ints)) {
    return 1;
  }
  return 0;
}
