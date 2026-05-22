#ifndef INTEGERSET_H
#define INTEGERSET_H

#ifdef _WIN32

#else
#define EXPORT_API
#endif

typedef struct IntegerSet IntegerSet;

EXPORT_API IntegerSet *CreateSet(const int *arr, int size);
EXPORT_API int IsInSet(IntegerSet *set, int value);

#endif
