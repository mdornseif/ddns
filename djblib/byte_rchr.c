#include "byte.h"

unsigned int byte_rchr(const char *s,register unsigned int n,int c)
{
  register char ch;
  register const char *t;
  register char *u;

  ch = c;
  t = s;
  u = 0;
  for (;;) {
    if (!n) break; if (*t == ch) u = t; ++t; --n;
    if (!n) break; if (*t == ch) u = t; ++t; --n;
    if (!n) break; if (*t == ch) u = t; ++t; --n;
    if (!n) break; if (*t == ch) u = t; ++t; --n;
  }
  if (!u) u = t;
  return u - s;
}
