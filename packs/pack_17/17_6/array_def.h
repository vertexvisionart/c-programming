/* Include guards for standard libraries so they only load once */
#ifndef ARRAY_DEF_INCLUDES
#define ARRAY_DEF_INCLUDES
#include <stdlib.h>
#include <string.h>
#endif

/* Re-define the concatenation macros for this file */
#define ARRAY_CONCAT_IMPL(name, func) name##_##func
#define ARRAY_CONCAT(name, func) ARRAY_CONCAT_IMPL(name, func)
#define AFUNC(func) ARRAY_CONCAT(NAME, func)

void AFUNC(init)(NAME *vec) {
  vec->n = 0;
  vec->capacity = 0;
  vec->arr = NULL;
}

void AFUNC(destroy)(NAME *vec) {
  free(vec->arr);
  vec->arr = NULL;
  vec->n = 0;
  vec->capacity = 0;
}

void AFUNC(reserve)(NAME *vec, int capacity) {
  /* The instructions state we cannot shrink the buffer! */
  if (capacity > vec->capacity) {
    vec->capacity = capacity;
    vec->arr = (TYPE *)realloc(vec->arr, capacity * sizeof(TYPE));
  }
}

int AFUNC(push)(NAME *vec, TYPE value) {
  if (vec->n == vec->capacity) {
    /* Amortized O(1): Double the capacity when full */
    int new_cap = (vec->capacity == 0) ? 1 : (vec->capacity * 2);
    AFUNC(reserve)(vec, new_cap);
  }
  vec->arr[vec->n] = value;
  return vec->n++; /* Returns the index before incrementing */
}

TYPE AFUNC(pop)(NAME *vec) { return vec->arr[--vec->n]; }

void AFUNC(resize)(NAME *vec, int newCnt, TYPE fill) {
  if (newCnt > vec->capacity) {
    int new_cap = (vec->capacity == 0) ? 1 : (vec->capacity * 2);
    while (new_cap < newCnt)
      new_cap *= 2;
    AFUNC(reserve)(vec, new_cap);
  }
  if (newCnt > vec->n) {
    for (int i = vec->n; i < newCnt; i++) {
      vec->arr[i] = fill;
    }
  }
  vec->n = newCnt;
}

void AFUNC(insert)(NAME *vec, int where, TYPE *arr, int num) {
  if (num <= 0)
    return;

  if (vec->n + num > vec->capacity) {
    int new_cap = (vec->capacity == 0) ? 1 : (vec->capacity * 2);
    while (new_cap < vec->n + num)
      new_cap *= 2;
    AFUNC(reserve)(vec, new_cap);
  }

  /* Shift existing elements to the right to make a gap */
  if (vec->n > where) {
    memmove(vec->arr + where + num, vec->arr + where,
            (vec->n - where) * sizeof(TYPE));
  }

  /* Copy the new elements into the gap */
  if (arr != NULL) {
    memcpy(vec->arr + where, arr, num * sizeof(TYPE));
  }
  vec->n += num;
}

void AFUNC(erase)(NAME *vec, int where, int num) {
  if (num <= 0)
    return;

  /* Shift elements to the left to crush the gap */
  if (vec->n > where + num) {
    memmove(vec->arr + where, vec->arr + where + num,
            (vec->n - where - num) * sizeof(TYPE));
  }
  vec->n -= num;
}

/* Final cleanup! DO NOT undef TYPE or NAME. */
#undef AFUNC
#undef ARRAY_CONCAT
#undef ARRAY_CONCAT_IMPL
