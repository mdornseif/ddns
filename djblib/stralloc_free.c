#include "stralloc.h"
#include "alloc.h"

void stralloc_free(stralloc *sa)
{
  alloc_free(sa->s);
  sa->s = 0;
  sa->len = sa->a = 0;
}

