#include "r.h"

MAKE_COMPARE_FUNCTION(char, char)

struct __dfa_set {
  int length;
  struct __dfa_set_state *link;
};

struct __dfa_set_state {
  struct generic_set     *states;
  struct dfa_state       *dfa_state;
  struct __dfa_set_state *next;
};

static void destroy_dfa_set(struct __dfa_set *p) {
  struct __dfa_set_state *cur = p->link;
  for (; cur != NULL; ) {
    struct __dfa_set_state *_t = cur;
    cur = cur->next;
    destroy_generic_set(_t->states);
  }
}

static struct __dfa_set_state *create_dfa_set_state() {
  struct __dfa_set_state *p = (struct __dfa_set_state *)malloc(sizeof(*p));
  p->states     = NULL;
  p->next       = NULL;
  return p;
}

static void __dfa_set_add_after(struct generic_set *states, struct __dfa_set_state *set)
{
  struct __dfa_set_state *added = create_dfa_set_state();
  added->next      = set->next;
  set->next        = added;
  added->states    = states;
  added->dfa_state = create_dfa_state(-1);
}

static void __dfa_set_add(struct generic_set *states, struct __dfa_set *set, struct dfa_state *d) {
  struct __dfa_set_state *cur = create_dfa_set_state();
  cur->states    = states;
  cur->dfa_state = d;
  cur->next      = set->link;
  set->link      = cur;
}

static void __collect_dfa_into_set_rec(struct dfa_state *state,
                                       struct dfa_state *b_state[2],
                                        struct generic_set *nonaccept_state,
                                       struct generic_set *accept_state)
{
  state->mark   = state->accept ? 1 : 0;
  state->belong = state->accept ? b_state[0] : b_state[1];
  for (int i = 0; i < state->n_trans; ++i) {
    struct generic_set *which =
      state->trans[i].to->accept? accept_state : nonaccept_state;
    if (generic_set_add(which, &state->trans[i].to, NULL)) {
      __collect_dfa_into_set_rec(state->trans[i].to, b_state, nonaccept_state, accept_state);
    }
  }
}

static void __collect_dfa_into_set(struct dfa_state *state,
                                   struct generic_set *nonaccept_state,
                                   struct generic_set *accept_state,
                                   struct __dfa_set *P) {
  create_generic_set(struct dfa_state *, INIT_BITSIZE, nonaccept_state);
  create_generic_set(struct dfa_state *, INIT_BITSIZE, accept_state);

  state->accept?
    generic_set_add(accept_state, &state, NULL) :
    generic_set_add(nonaccept_state, &state, NULL);

  struct dfa_state *b_state[2];
  b_state[0] = create_dfa_state(0);
  b_state[1] = create_dfa_state(1);
  __collect_dfa_into_set_rec(state, b_state, nonaccept_state, accept_state);

  if (accept_state->length) {
    __dfa_set_add(accept_state, P, b_state[0]);
  } else {
    destroy_generic_set(accept_state);
    free_dfa(b_state[0]);
  }
  if (nonaccept_state->length) {
    __dfa_set_add(nonaccept_state, P, b_state[1]);
  } else {
    destroy_generic_set(nonaccept_state);
    free_dfa(b_state[1]);
  }
}

static void __collect_dfa_char(struct generic_set *states,
                               struct generic_set *new_chars)
{
  struct dfa_state **s = (struct dfa_state **)states->p_dat;
  int n = states->length;
  for (int i = 0; i < n; ++i, ++s) {
    for (int t = 0; t < (*s)->n_trans; ++t) {
      generic_set_add(new_chars, &((*s)->trans[t].trans_char), __cmp_char);
    }
  }
}


/* return mark of dfa which transition to, return -1 if transition to nonthing */
static int __dfa_trans_by(struct dfa_state *d, char ch) {
  for (int t = 0; t < d->n_trans; ++t) {
    if (d->trans[t].trans_char == ch) {
      return d->trans[t].to->mark;
    }
  }
  return -1;
}

static int split_into_set(struct __dfa_set_state *cur,
                          struct generic_set *s1,
                          struct generic_set *s2)
{
  struct generic_set *s0 = cur->states;
  static int global_mark = 2;
  struct generic_set trans_char;
  create_generic_set(char, INIT_BITSIZE, &trans_char);

  create_generic_set(struct dfa_state *, INIT_BITSIZE, s1);
  create_generic_set(struct dfa_state *, INIT_BITSIZE, s2);

  __collect_dfa_char(s0, &trans_char);
  char *c = (char *)trans_char.p_dat;

  for (int i = 0; i < trans_char.length; ++i, ++c) { /* for each character */
    struct dfa_state **s = (struct dfa_state **)s0->p_dat;
    int mark = __dfa_trans_by(*s, *c);
    int split = 0;
    for (int x = 0; x < s0->length; x++, s++) {
      (*s)->belong = cur->dfa_state;
      int mark1 = __dfa_trans_by(*s, *c);
      if (mark1 != mark) {
        split = 1;

        generic_set_add(s2, s, NULL);
      } else {
        generic_set_add(s1, s, NULL);
      }
    }
    if (split) {
      s = s2->p_dat;
      for (int x = 0; x < s2->length; x++, s++) {
        (*s)->mark = global_mark;
      }
      global_mark++;
      destroy_generic_set(&trans_char);
      return 1;
    }
    generic_set_clear(s1);
    generic_set_clear(s2);
  }

  destroy_generic_set(s1); free(s1);
  destroy_generic_set(s2); free(s2);
  destroy_generic_set(&trans_char);
  return 0;
}


static void __dfa_to_minimal_rec(struct __dfa_set *p) {
  struct __dfa_set_state *cur = p->link;

  int rec = 0;
  for (; cur != NULL ; cur = cur->next) {
    struct generic_set *s1 = malloc(sizeof(*s1)), *s2 = malloc(sizeof(*s2));
    if (split_into_set(cur, s1, s2)) {
      rec = 1;
      destroy_generic_set(cur->states);
      cur->states = s1;
      __dfa_set_add_after(s2,cur);
    }
  }
  if (rec) __dfa_to_minimal_rec(p);
}

static void __constuct_dfa_trans(struct dfa_state *s) {
  struct dfa_state *from = s->belong;
  for (int i = 0; i < s->n_trans; ++i)  {
    struct dfa_state *to   = s->trans[i].to->belong;
    dfa_add_trans(from, to, s->trans[i].trans_char);
  }
  s->belong->accept = s->accept;
}

struct dfa_state *dfa_to_minimal(const struct dfa_state *state) {
  struct dfa_state *start = (struct dfa_state *)state;

  /* split into start and accept set */
  struct generic_set nonaccept_state,accept_state;

  struct __dfa_set pset;
  pset.link = NULL;

  __collect_dfa_into_set(start, &nonaccept_state, &accept_state, &pset);
  __dfa_to_minimal_rec(&pset);

  struct __dfa_set_state *cur = pset.link;
  for (; cur != NULL; cur = cur->next) {
    struct dfa_state **s = (struct dfa_state **)cur->states->p_dat; /* only need the first dfa state */
    __constuct_dfa_trans(*s);
  }
  destroy_dfa_set(&pset);

  return pset.link->dfa_state;
}
