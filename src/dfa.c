#include "r.h"
struct dfa_state *create_dfa_state(int over) {
  struct dfa_state *state = (struct dfa_state *)malloc(sizeof(struct dfa_state));
  static int index = 0;
  if (over != -1) index = over;
  state->base_state.index = index++;
  state->mark             = 0;
  state->accept           = 0;
  state->n_trans          = 0;
  state->capacity         = 4;
  state->belong           = NULL;
  state->trans = (struct dfa_trans *)malloc(state->capacity * sizeof(struct dfa_trans));
  return state;
}

void dfa_add_trans(struct dfa_state *from, struct dfa_state *to, char ch) {
  if (from->n_trans == from->capacity) {
    from->capacity *= 2;
    from->trans = (struct dfa_trans *)
      realloc(from->trans, from->capacity * sizeof(struct dfa_trans));
  }
  from->trans[from->n_trans].to         = to;
  from->trans[from->n_trans].trans_char = ch;
  from->n_trans += 1;
}

struct dfa_state *dfa_trans_to(struct dfa_state *state, char ch) {
  for (int i = 0, n = state->n_trans; i < n; ++i) {
    if (state->trans[i].trans_char == ch)
      return state->trans[i].to;
  }
  return NULL;
}

static void __dfa_walk_through(struct dfa_state *state,
                               struct generic_set *visited) {
  for (int i = 0; i < state->n_trans; ++i) {
    if (generic_set_add(visited, &state->trans[i].to, NULL)) {
      __dfa_walk_through(state->trans[i].to, visited);
    }
  }
}

void free_dfa(struct dfa_state *d) {
  free(d->trans);
  free(d);
}

void dispose_dfa(struct dfa_state *d) {
  struct generic_set visited;
  create_generic_set(struct dfa_state *, INIT_BITSIZE, &visited);
  state_set_push_back(&visited, &d);
  __dfa_walk_through(d, &visited);

  struct dfa_state **cur = visited.p_dat;
  for (int i = 0; i < visited.length; i++, cur++) {
    free_dfa(*cur);
  }
  destroy_generic_set(&visited);
}


void dfa_dump_dot(FILE *fp,struct dfa_state *start) {
  struct generic_set visited;
  create_generic_set(struct dfa_state *, INIT_BITSIZE, &visited);
  state_set_push_back(&visited, &start);
  __dfa_walk_through(start, &visited);

  struct dfa_state **cur = visited.p_dat;
  fprintf(fp, "digraph dfa {\n");
  fprintf(fp, "  rankdir = LR; \n");
  for (int i = 0; i < visited.length; i++, cur++) {
    fprintf(fp, "  %d[shape=%scircle]\n",
            (*cur)->base_state.index,
            (*cur)->accept?"double":"");

    for (int t = 0; t < (*cur)->n_trans; ++t) {
      fprintf(fp, "  %d -> %d[label=\"%c\"]\n",
              (*cur)->base_state.index,
              (*cur)->trans[t].to->base_state.index, (*cur)->trans[t].trans_char);
    }
  }
  fprintf(fp, "  node [shape = none label=\"\"] start\n");
  fprintf(fp, "  start -> %d [label = \"start\"]\n", start->base_state.index);
  fprintf(fp, "}\n");

  destroy_generic_set(&visited);
}
