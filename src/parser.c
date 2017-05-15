#include "r.h"

/* -- Grammar --
 * Expr: Term { '|' Expr, Term }
 * Term: Factor [ * ]
 * Factor: ID | '(' Expr ')'
 */
static int is_valild_char(char ch) {
  return isalnum(ch) || ch == '_';
}

static struct nfa __ll_expr(char **s);
static struct nfa __ll_term(char **s);
static struct nfa __ll_fact(char **s);

static struct nfa __ll_expr(char **s) {
  struct nfa lhs = __ll_term(s);
  for (;;) {
    char ch = **s;
    if (is_valild_char(ch) || ch == '(') {
      struct nfa rhs = __ll_term(s);
      lhs = nfa_cons(&lhs, &rhs);
    } else if (ch == '|') {
      *s += 1;
      struct nfa rhs = __ll_expr(s);
      lhs = nfa_alt(&lhs, &rhs);
    } else {
      return lhs;
    }
  }
}


static struct nfa __ll_term(char **s) {
  struct nfa lhs = __ll_fact(s);
  switch (**s) {
  case '*':
    lhs = nfa_kleen(&lhs);
    *s += 1;
    break;
  case '+':
    lhs = nfa_positive(&lhs);
    *s += 1;
    break;
  case '?':
    lhs = nfa_optional(&lhs);
    *s += 1;
    break;
  default:
    break;
  }
  return lhs;
}

static struct nfa __ll_fact(char **s) {
  struct nfa lhs;
  char ch = **s;
  if (is_valild_char(ch)) {
    if (ch == '-')
      lhs = nfa_node(0);
    else
      lhs = nfa_node(ch);
    *s += 1;
  } else if (ch == '(') {
    *s += 1;
    lhs = __ll_expr(s);
    expect(s, ')');
  } else {
    error("syntax error; unrecognized character", *s);
    fprintf(stderr, " `%c'", **s);
    exit(1);
  }
  return lhs;
}

struct nfa re_to_nfa(const char *regexp) {
  char **cur = (char **)(&regexp);
  struct nfa n = __ll_expr(cur);

  expect(cur, 0);
  return n;
}
