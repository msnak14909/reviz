#ifndef R_H
#define R_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

extern void init(int argc, char **argv);
extern void main_init(int argc, char **argv);

// error.c
extern void error(const char *fmt, const char *cur);
extern void expect(char **cur, int tok);

char *_line;
char *_output_nfa_dot_file;
char *_output_dfa_dot_file;
char *_output_opt_dot_file;



enum nfa_trans_type { NONE, CHAR, EPSILON };

// gset structure
struct generic_set {
  int elem_size;                 /* size of each element */
  int length;                    /* num of elemets in the memory */
  int capacity;                  /* capacity of this array */
  int bit_size;                  /* num of state */
  int *bitset;                   /* for comparsion */
  void *p_dat;                   /* record the data */
};

#define INIT_CAPACITY 8
#define INIT_BITSIZE 2
// structure of state
// base type
struct base_state {
  int index;
};

// nfa

struct nfa_trans {
  enum nfa_trans_type trans_type;
  char trans_char;
};

struct nfa_state {
  struct base_state base_state;
  int mark;
  int n_trans;
  struct nfa_state *to[2];
  struct nfa_trans trans[2];
};

struct nfa {
  struct nfa_state *start;
  struct nfa_state *end;
};

// dfa
struct dfa_state;
struct dfa_trans {
  struct dfa_state *to;
  char trans_char;
};
struct dfa_state {
  struct base_state base_state;
  int mark;
  int accept;
  int n_trans;
  int capacity;
  struct dfa_trans *trans;
  struct dfa_state *belong;      // for optimize dfa
};

/*  dfa etnry structure
 *
 * /---entry------\
 * |  nfa_states--|---> { n0, n1, ... }
 * |  dfa_state --|---> qn
 * \--------------/
 */

struct __dfa_state_entry {
  // struct base_state  base_state;
  struct generic_set nfa_states;
  struct dfa_state   *dfa_state;
};

// gset.c
void bits_set(const int x, int *bits);
int  bits_get(const int x, int *bits);
extern int bits_disjoint(const int *bits_a, const int *bits_b, const int n_state);
extern int bits_equivalent(const int *bits_a, const int *bits_b, const int n_state);
extern void __create_generic_set(int elem_size, int init_capacity, int bit_size, struct generic_set *gset);
#define create_generic_set(type, bit_size, gset)                        \
  __create_generic_set(sizeof(type), INIT_CAPACITY, bit_size, gset);
extern void generic_set_duplicate(struct generic_set *dest,
                                   const struct generic_set *src);
extern void destroy_generic_set(struct generic_set *gset);
extern void generic_set_push_back(struct generic_set *gset, const void *elem);
extern void generic_set_push_back_char(struct generic_set *gset, const void *elem);
extern void generic_set_pop_back(struct generic_set *gset);
extern void *generic_set_back(struct generic_set *gset);
extern void *generic_set_front(struct generic_set *gset);
extern void generic_set_clear(struct generic_set *gset);
extern void state_set_push_back(struct generic_set *gset, const void *elem);

#define MAKE_COMPARE_FUNCTION(postfix, type)                    \
  static int __cmp_##postfix (const void *a_, const void *b_)   \
  {                                                             \
    type a = *((type*) a_);                                     \
    type b = *((type*) b_);                                     \
    if (a < b) return -1;                                       \
    else if (a > b) return 1;                                   \
    else return 0;                                              \
  }
extern void *generic_set_find(struct generic_set *gset, const void *elemm,
                        int(*cmp)(const void*, const void*));
extern int generic_set_add(struct generic_set *gset, const void *elem,
                     int(*cmp)(const void*, const void*));

// nfa.c function
extern int nfa_add_trans(struct nfa_state *stat, enum nfa_trans_type type,
                         char ch, struct nfa_state *to);
extern struct nfa nfa_cons(const struct nfa *a, const struct nfa *b);
extern struct nfa nfa_alt(const struct nfa *a, const struct nfa *b);
extern struct nfa nfa_kleen(const struct nfa *a);
extern struct nfa nfa_positive(const struct nfa *a);
extern struct nfa nfa_optional(const struct nfa *a);
extern struct nfa nfa_node(char c);
extern void dispose_nfa(struct nfa *n) ;


// dfa.c function
extern struct dfa_state *create_dfa_state(int over);
extern void dfa_add_trans(struct dfa_state *from, struct dfa_state *to, char ch);
extern struct dfa_state *dfa_trans_to(struct dfa_state *state, char ch);
extern void dispose_dfa(struct dfa_state *d);
extern void free_dfa(struct dfa_state *d);

// process
extern struct nfa re_to_nfa(const char *regexp);
extern struct dfa_state *nfa_to_dfa(const struct nfa *n);
extern struct dfa_state *dfa_to_minimal(const struct dfa_state *d);


extern void nfa_dump_dot(FILE *fp, struct nfa *n);
extern void dfa_dump_dot(FILE *fp, struct dfa_state *start);

#endif
