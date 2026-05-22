/* Macro Magic to concatenate NAME and the function name (e.g., array_long +
 * _init) */
#define ARRAY_CONCAT_IMPL(name, func) name##_##func
#define ARRAY_CONCAT(name, func) ARRAY_CONCAT_IMPL(name, func)
#define AFUNC(func) ARRAY_CONCAT(NAME, func)

/* The Main Structure */
typedef struct NAME {
  int n;        /* Number of actual elements */
  TYPE *arr;    /* Pointer to the memory buffer */
  int capacity; /* Total allocated slots in the buffer */
} NAME;

/* Function Prototypes */
void AFUNC(init)(NAME *vec);
void AFUNC(destroy)(NAME *vec);
int AFUNC(push)(NAME *vec, TYPE value);
TYPE AFUNC(pop)(NAME *vec);
void AFUNC(reserve)(NAME *vec, int capacity);
void AFUNC(resize)(NAME *vec, int newCnt, TYPE fill);
void AFUNC(insert)(NAME *vec, int where, TYPE *arr, int num);
void AFUNC(erase)(NAME *vec, int where, int num);

/* Clean up ONLY our custom macros! DO NOT undef TYPE or NAME. */
#undef AFUNC
#undef ARRAY_CONCAT
#undef ARRAY_CONCAT_IMPL
