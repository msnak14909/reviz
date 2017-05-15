#include "r.h"

#define output_to_dot(fn, name, st)                             \
  {                                                             \
    FILE *fp = fopen(_output_ ## name ## _dot_file, "w");       \
    assert(fp);                                                 \
    fn ##_dump_dot(fp, st);                                     \
    fclose(fp);                                                 \
  }

int main(int argc, char **argv) {
  init(argc, argv);

  struct nfa n = re_to_nfa(argv[1]);
  struct dfa_state *dt = nfa_to_dfa(&n);
  struct dfa_state *da = dfa_to_minimal(dt);

  output_to_dot(nfa, nfa, &n);
  output_to_dot(dfa, dfa, dt);
  output_to_dot(dfa, opt, da);

  dispose_nfa(&n);
  dispose_dfa(dt);
  dispose_dfa(da);
  return 0;
}


void main_init(int argc, char **argv) {
  static int inited;
  if (inited) return;
  inited = 1;

  _output_nfa_dot_file = "./nfa_out.dot";
  _output_dfa_dot_file = "./dfa_out.dot";
  _output_opt_dot_file = "./opt_out.dot";
  _line = argv[1];

  if (argc < 2) {
    fprintf(stderr, "illegal arguments");
    exit(1);
  }
}
