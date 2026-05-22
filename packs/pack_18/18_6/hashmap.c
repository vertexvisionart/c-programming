#define HASHMAP_EXPORTS
#include "hashmap.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
   Linear probing hash table.

   Invariants guaranteed by the problem:
     - At least one cell is always free, so insertion never loops forever.
     - Load factor stays reasonable, so lookups are fast.

   We use a "tombstone" (HM_DELETED) approach so that HM_Get works
   correctly even after future deletions (though HM_Delete isn't required,
   the approach is still correct without it).
   ----------------------------------------------------------------------- */

HashMap HM_Init(EqualFunc ef, HashFunc hf, int size) {
  HashMap hm;
  hm.eq = ef;
  hm.hash = hf;
  hm.size = size;
  hm.cells = (HMCell *)calloc((size_t)size, sizeof(HMCell));
  /* calloc zeroes memory, so state == HM_EMPTY (== 0) for all cells */
  return hm;
}

void HM_Destroy(HashMap *self) {
  free(self->cells);
  self->cells = NULL;
  self->size = 0;
}

/* Find the slot index for [key]:
   - Returns index of the occupied cell with that key, OR
   - Returns index of the first free/deleted slot suitable for insertion
     (tracked via first_deleted), OR
   - Returns -1 if not found (key absent), storing insertion slot in *insert_idx
*/
static int find_slot(const HashMap *self, cpvoid key, int *insert_idx) {
  uint32_t h = self->hash(key);
  int start = (int)(h % (uint32_t)self->size);
  int idx = start;
  int first_deleted = -1; /* first tombstone seen */

  do {
    HMCell *c = &self->cells[idx];

    if (c->state == HM_EMPTY) {
      /* Key definitely not present */
      if (insert_idx)
        *insert_idx = (first_deleted >= 0) ? first_deleted : idx;
      return -1;
    }
    if (c->state == HM_DELETED) {
      if (first_deleted < 0)
        first_deleted = idx;
    } else { /* HM_OCCUPIED */
      if (self->eq(c->key, key)) {
        if (insert_idx)
          *insert_idx = idx;
        return idx; /* found */
      }
    }

    idx = (idx + 1) % self->size;
  } while (idx != start);

  /* Whole table scanned — shouldn't happen given the invariant,
     but handle gracefully */
  if (insert_idx)
    *insert_idx = (first_deleted >= 0) ? first_deleted : -1;
  return -1;
}

cpvoid HM_Get(const HashMap *self, cpvoid key) {
  int found = find_slot(self, key, NULL);
  if (found < 0)
    return NULL;
  return self->cells[found].value;
}

void HM_Set(HashMap *self, cpvoid key, cpvoid value) {
  int insert_idx = -1;
  int found = find_slot(self, key, &insert_idx);

  if (found >= 0) {
    self->cells[found].value = value;
  } else {
    HMCell *c = &self->cells[insert_idx];
    c->key = key;
    c->value = value;
    c->state = HM_OCCUPIED;
  }
}

#ifdef __cplusplus
}
#endif
