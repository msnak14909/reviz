#include "r.h"

struct nfa_state *create_nfa_state(void) {
  struct nfa_state *state = (struct nfa_state *)malloc(sizeof(struct nfa_state));
  static int index = 0;
  state->base_state.index    = index++;
  state->mark     = 0;
  state->n_trans  = 0;
  state->to[0]    = state->to[1]    = NULL;
  state->trans[0] = state->trans[1] = (struct nfa_trans){ NONE, 0 };
  return state;
}

int nfa_add_trans(struct nfa_state *state, enum nfa_trans_type type,
                  char ch, struct nfa_state *to) {
  int num = state->n_trans++;
  if (num != 2) {
    state->trans[num].trans_type = type;
    state->trans[num].trans_char = ch;
    state->to[num]               = to;
    return 0;
  }
  return -1;
}

struct nfa nfa_cons(const struct nfa *a, const struct nfa *b) {
  struct nfa c;
  c.start = a->start;
  c.end   = b->end;

  nfa_add_trans(a->end, EPSILON, 0, b->start);
  return c;
}

struct nfa nfa_alt(const struct nfa *a, const struct nfa *b) {
  struct nfa c;
  c.start = create_nfa_state();
  c.end   = create_nfa_state();

  nfa_add_trans(c.start, EPSILON, 0, a->start);
  nfa_add_trans(c.start, EPSILON, 0, b->start);
  nfa_add_trans(a->end,  EPSILON, 0, c.end);
  nfa_add_trans(b->end,  EPSILON, 0, c.end);
  return c;
}

struct nfa nfa_kleen(const struct nfa *a) {
  struct nfa c;
  c.start = create_nfa_state();
  c.end   = create_nfa_state();

  nfa_add_trans(c.start, EPSILON, 0, a->start);
  nfa_add_trans(c.start, EPSILON, 0, c.end);
  nfa_add_trans(a->end,  EPSILON, 0, c.start);
  return c;
}

struct nfa nfa_positive(const struct nfa *a) {
  struct nfa c;
  c.start = create_nfa_state();
  c.end   = create_nfa_state();

  nfa_add_trans(c.start, EPSILON, 0, a->start);
  nfa_add_trans(a->end,  EPSILON, 0, c.start);
  nfa_add_trans(a->end,  EPSILON, 0, c.end);
  return c;
}

struct nfa nfa_optional(const struct nfa *a) {
  struct nfa c;
  c.start = create_nfa_state();
  c.end   = a->end;

  nfa_add_trans(c.start, EPSILON, 0, a->start);
  nfa_add_trans(c.start, EPSILON, 0, a->end);
  return c;
}

struct nfa nfa_node(char c) {
  struct nfa n;
  n.start = create_nfa_state();
  n.end   = create_nfa_state();

  /* assert(c != 0); */
  if (c)
    nfa_add_trans(n.start, CHAR, c, n.end);
  else
    nfa_add_trans(n.start, EPSILON, 0, n.end);
  return n;
}

static void __nfa_walk_through(struct nfa_state *state,
                               struct generic_set *visited) {
  for (int i = 0; i < state->n_trans; ++i) {
    if (generic_set_add(visited, &state->to[i], NULL)) {
      __nfa_walk_through(state->to[i], visited);
    }
  }
}

void dispose_nfa(struct nfa *n) {
  struct generic_set visited;
  create_generic_set(struct nfa_state *, INIT_BITSIZE, &visited);
  state_set_push_back(&visited, &n->start);
  __nfa_walk_through(n->start, &visited);

  struct nfa_state **cur = visited.p_dat;
  for (int i = 0; i < visited.length; i++, cur++) {
    free(*cur);
  }
  destroy_generic_set(&visited);
}

void nfa_dump_dot(FILE *fp,struct nfa *n) {
  struct nfa_state *start = n->start;
  struct nfa_state *end = n->end;
  struct generic_set visited;
  create_generic_set(struct nfa_state *, INIT_BITSIZE, &visited);
  state_set_push_back(&visited, &start);
  __nfa_walk_through(start, &visited);

  struct nfa_state **cur = visited.p_dat;
  fprintf(fp, "digraph nfa {\n");
  fprintf(fp, "  rankdir = LR; \n");
  for (int i = 0; i < visited.length; i++, cur++) {
    fprintf(fp, "  %d[shape=%scircle]\n",
           (*cur)->base_state.index, *cur == end?"double":"");
    for (int t = 0; t < (*cur)->n_trans; ++t) {
      fprintf(fp, "  %d -> %d[label=\"",
             (*cur)->base_state.index,
             (*cur)->to[t]->base_state.index);
      (*cur)->trans[t].trans_type == EPSILON?
        fprintf(fp, "ε\"]\n") :
        fprintf(fp, "%c\"]\n", (*cur)->trans[t].trans_char);
    }
  }
  fprintf(fp, "  node [shape = none label=\"\"] start\n");
  fprintf(fp, "  start -> %d [label = \"start\"]\n", n->start->base_state.index);
  fprintf(fp, "}\n");

  destroy_generic_set(&visited);
}
