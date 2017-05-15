#include "r.h"

MAKE_COMPARE_FUNCTION(char, char)

static void create_dfa_state_entry(const struct generic_set *states,
                                   struct __dfa_state_entry *entry)
{
  generic_set_duplicate(&entry->nfa_states, states);
  entry->dfa_state = create_dfa_state(-1);
}

static void __destroy_dfa_state_entry(struct __dfa_state_entry *entry) {
  destroy_generic_set(&entry->nfa_states);
}

static void __state_epsilon_closure(const struct nfa_state *state,
                                    struct generic_set *visited)
{
  for (int i = 0; i < state->n_trans; ++i) {
    if (state->trans[i].trans_type == EPSILON &&
        generic_set_add(visited, &state->to[i], NULL))
      {
      __state_epsilon_closure(state->to[i], visited);
      }
  }
}

static void __epsilon_closure(struct generic_set *states) {
  int n = states->length;
  for (int i = 0; i < n; ++i) {
    const struct nfa_state *state = *(((const struct nfa_state **)states->p_dat) + i);
    __state_epsilon_closure(state, states);
  }
}

static void __collect_nfa_char(struct generic_set *states,
                               struct generic_set *new_chars)
{
  struct nfa_state **s = (struct nfa_state **)states->p_dat;
  int n = states->length;
  for (int i = 0; i < n; ++i, ++s) {
    for (int t = 0; t < (*s)->n_trans; ++t) {
      if ((*s)->trans[t].trans_type == CHAR) {
        generic_set_add(new_chars, &((*s)->trans[t].trans_char), __cmp_char);
      }
    }
  }
}

static void __collect_nfa_to(struct generic_set *states, char c,
                             struct generic_set *new_states)
{
  struct nfa_state **s = (struct nfa_state **)states->p_dat;
  int n = states->length;
  for (int i = 0; i < n; ++i, ++s) {
    for (int t = 0; t < (*s)->n_trans; ++t) {
      if ((*s)->trans[t].trans_type == CHAR &&
          (*s)->trans[t].trans_char == c)
        {
          generic_set_add(new_states, &((*s)->to[t]), NULL);
        }
    }
  }
}


static int __cmp_dfa_state_entry(const void *a_, const void *b_) {
  struct __dfa_state_entry *a = (struct __dfa_state_entry *)a_;
  struct __dfa_state_entry *b = (struct __dfa_state_entry *)b_;

  struct generic_set *label_a = &a->nfa_states;
  struct generic_set *label_b = &b->nfa_states;

  if (label_a->length   != label_b->length ||
      label_a->bit_size != label_b->bit_size) return 1;

  return label_a->length   != label_b->length    ||
         label_a->bit_size != label_b->bit_size  ||
         bits_equivalent(label_a->bitset, label_b->bitset, label_a->bit_size);
}



static void *__get_dfa_state_addr(struct generic_set *entry_set,
                                  const struct generic_set *states,
                                  int *is_new)
{
  struct __dfa_state_entry entry, *addr;
  create_dfa_state_entry(states, &entry);

  addr = (struct __dfa_state_entry *)
    generic_set_find(entry_set, &entry, __cmp_dfa_state_entry);

  if(addr) {
    *is_new = 0;
    __destroy_dfa_state_entry(&entry);
    free_dfa(entry.dfa_state);
    return addr->dfa_state;
  }
  *is_new = 1;

  generic_set_push_back(entry_set, &entry);
  return entry.dfa_state;
}

static void __nfa_to_dfa_rec(struct generic_set *states, struct generic_set *entry_set) {
  struct generic_set trans_char, new_states;
  struct dfa_state *from, *to;
  int dummy = 0, rec = 0;
  char *c;

  create_generic_set(char, INIT_BITSIZE, &trans_char);
  create_generic_set(struct nfa_state *, INIT_BITSIZE, &new_states);

  /* get all trans char of the states */
  __collect_nfa_char(states, &trans_char);

  c = (char *)trans_char.p_dat;
  for (int i = 0; i < trans_char.length; ++i, ++c) {
    __collect_nfa_to(states, *c, &new_states);
    __epsilon_closure(&new_states);

    from = __get_dfa_state_addr(entry_set, states, &dummy);
    to   = __get_dfa_state_addr(entry_set, &new_states, &rec);

    dfa_add_trans(from, to, *c);
    if (rec) {
      __nfa_to_dfa_rec(&new_states, entry_set);
    }
    generic_set_clear(&new_states);
  }
  destroy_generic_set(&trans_char);
  destroy_generic_set(&new_states);
}


struct dfa_state *nfa_to_dfa(const struct nfa *n) {
  struct generic_set start_set;
  struct generic_set entry_set;
  struct dfa_state  *dfa_start_state; /* return the start state of dfa */
  create_generic_set(struct nfa_state *, INIT_BITSIZE, &start_set);
  create_generic_set(struct __dfa_state_entry, INIT_BITSIZE, &entry_set);

  state_set_push_back(&start_set, &n->start);
  __epsilon_closure(&start_set);
  __nfa_to_dfa_rec(&start_set, &entry_set);

  dfa_start_state = ((struct __dfa_state_entry *)entry_set.p_dat)[0].dfa_state;

    /* marking acceptable and cleaning up */
  for (int i = 0; i < entry_set.length;  ++i) {
    struct __dfa_state_entry *x =
      (((struct __dfa_state_entry *)entry_set.p_dat) + i);
    x->dfa_state->accept = bits_get(n->end->base_state.index, x->nfa_states.bitset);
    __destroy_dfa_state_entry(x);
  }

  destroy_generic_set(&start_set);
  destroy_generic_set(&entry_set);
  return dfa_start_state;
}
