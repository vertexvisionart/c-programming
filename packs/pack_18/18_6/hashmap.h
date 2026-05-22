#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>

/* Pointer to key or value (untyped) */
typedef const void *cpvoid;

/* Returns 1 iff keys pointed by [a] and [b] are equal, 0 otherwise */
typedef int (*EqualFunc)(cpvoid a, cpvoid b);

/* Returns 32-bit hash of a key pointed by [key] */
typedef uint32_t (*HashFunc)(cpvoid key);

/* Export macros for Windows/Linux */
#ifdef _WIN32
#ifdef HASHMAP_EXPORTS
#define HASHMAP_API __declspec(dllexport)
#else
#define HASHMAP_API __declspec(dllimport)
#endif
#else
#ifdef HASHMAP_EXPORTS
#define HASHMAP_API __attribute__((visibility("default")))
#else
#define HASHMAP_API
#endif
#endif

/* Internal cell state */
typedef enum {
  HM_EMPTY = 0,
  HM_OCCUPIED,
  HM_DELETED /* tombstone for linear probing */
} CellState;

/* A single cell in the hash table */
typedef struct {
  cpvoid key;
  cpvoid value;
  CellState state;
} HMCell;

/* The hash map structure */
typedef struct {
  HMCell *cells;
  int size; /* total number of cells */
  EqualFunc eq;
  HashFunc hash;
} HashMap;

#ifdef __cplusplus
extern "C" {
#endif

/* Creates and returns a new hash table */
HASHMAP_API HashMap HM_Init(EqualFunc ef, HashFunc hf, int size);

/* Frees memory of hash map [self] */
HASHMAP_API void HM_Destroy(HashMap *self);

/* Returns value for [key], or NULL if not present */
HASHMAP_API cpvoid HM_Get(const HashMap *self, cpvoid key);

/* Sets [value] for [key], overwriting if key already exists */
HASHMAP_API void HM_Set(HashMap *self, cpvoid key, cpvoid value);

#ifdef __cplusplus
}
#endif

#endif /* HASHMAP_H */
