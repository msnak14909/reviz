#include "r.h"
void bits_set(const int x, int *bits) { bits[x>>5] |= 1<<(x&31); }
int  bits_get(const int x, int *bits) { return bits[x>>5]>>(x&31) & 1; }
/* return 0 if disjoint, otherwise 1 */
int bits_disjoint(const int *a, const int *b, const int bit_size) {
  for (int i = 0; i < bit_size; ++i)
    if (a[i] & b[i]) return 1;
  return 0;
}
/* return 0 if equivalent, otherwise 0 */
int bits_equivalent(const int *a, const int *b, const int bit_size) {
  for (int i = 0; i < bit_size; ++i)
    if (a[i] ^ b[i]) return 1;
  return 0;
}


void __create_generic_set(int elem_size, int init_capacity, int bit_size,
                          struct generic_set *gset)
{
  assert(init_capacity != 0);
  gset->elem_size = elem_size;
  gset->capacity  = init_capacity;
  gset->length    = 0;
  gset->bit_size  = bit_size;
  gset->bitset    = (int *)(malloc(bit_size * sizeof(int)));
  gset->p_dat     = (void *)(malloc(elem_size * init_capacity));
  memset(gset->bitset, 0, bit_size * sizeof(int));
}

void generic_set_duplicate(struct generic_set *dest, const struct generic_set *src) {
  __create_generic_set(src->elem_size, src->capacity, src->bit_size, dest);
  dest->length = src->length;
  memcpy(dest->bitset, src->bitset, src->bit_size * sizeof(int));
  memcpy(dest->p_dat,  src->p_dat,  src->length * src->elem_size);
}

void state_set_push_back(struct generic_set *gset, const void *elem) {
  /* if running out of space */
  if (gset->capacity == gset->length) {
    gset->capacity *= 2;
    gset->p_dat = (void*)realloc(gset->p_dat, gset->elem_size * gset->capacity);
  }

  struct base_state *x = *((struct base_state **)elem);
  int elem_bit_size = (x->index >> 5) + 1;
  /* expand the bitset if running out of bitset space,
   * that olny happen in dfa_state_entry created,
   * bucause the index of subset of states is not determined
   */
  if (elem_bit_size >= gset->bit_size) {
    gset->bit_size = elem_bit_size * 2;
    gset->bitset = (int *)realloc(gset->bitset, gset->bit_size * sizeof(int));
  }

  /* push the element */
  bits_set(x->index, gset->bitset);
  memcpy(gset->p_dat + gset->elem_size * gset->length++, elem, gset->elem_size);
}

void generic_set_push_back(struct generic_set *gset, const void *elem) {
  /* if running out of space */
  if (gset->capacity == gset->length) {
    gset->capacity *= 2;
    gset->p_dat = (void *)realloc(gset->p_dat, gset->elem_size * gset->capacity);
  }
  memcpy(gset->p_dat + gset->elem_size * gset->length++, elem, gset->elem_size);
}

void *generic_set_back(struct generic_set *gset) {
  assert(gset->length != 0);
  return gset->p_dat + gset->elem_size * (gset->length - 1);
}

void *generic_set_front(struct generic_set *gset) {
  assert(gset->length != 0);
  return gset->p_dat;
}

void generic_set_clear(struct generic_set *gset) {
  memset(gset->bitset, 0, gset->bit_size * sizeof(int));
  /* for (int i = 0; i < gset->bit_size; ++i) { */
  /*   gset->bitset[i] = 0; */
  /* } */
  gset->bit_size = 2;
  gset->length  = 0;

}

void *generic_set_find(struct generic_set *gset, const void *elem,
                       int(*cmp)(const void*, const void*)) {
  void *cur = gset->p_dat;
  for (int i = 0; i < gset->length; ++i, cur += gset->elem_size) {
    if (cmp(cur, elem) == 0) return cur;
  }

  return NULL;
}

int generic_set_add(struct generic_set *gset, const void *elem,
                    int(*cmp)(const void*, const void*)) {
  if (cmp) {
    void *same = generic_set_find(gset, elem, cmp);
    if (same == NULL) {
      generic_set_push_back(gset, elem);
      return 1;
    }
    return 0;
  }
  struct base_state *e = *((struct base_state **)elem);

  int bit = bits_get(e->index, gset->bitset);
  if (bit == 0) {
    state_set_push_back(gset, elem);
    return 1;
  }
  return 0;
}

void destroy_generic_set(struct generic_set *gset) {
  free(gset->bitset);
  free(gset->p_dat);
}
