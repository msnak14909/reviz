#include "r.h"

void error(const char *fmt, const char *cur) {
  fprintf(stderr, "%s : [%ld]", fmt, cur - _line);
}

void expect(char **cur, int tok) {
  if (**cur == tok)
    (*cur)++;
  else {
    error("syntax error; found", *cur);
    fprintf(stderr, " `%c'", **cur);
    exit(1);
  }
}
