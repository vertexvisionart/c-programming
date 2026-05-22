# algorithms — classic textbook examples

Small, self-contained demos of common algorithms.

- `gcd_euclidean.c` — greatest common divisor via the Euclidean algorithm.
- `balanced_brackets.c` — stack-based validator for `()`, `[]`, `{}`.
- `mergesort.c` — top-down merge sort on a fixed integer array.

## Build

```sh
cc -O2 -Wall -o gcd gcd_euclidean.c
cc -O2 -Wall -o brackets balanced_brackets.c
cc -O2 -Wall -o mergesort mergesort.c
```
