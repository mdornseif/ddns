#include "byte.h"

void byte_copyr(register void* To,register unsigned int n,
		register const void* From)
{
  register char *to=To+n;
  register const char *from=From+n;
  for (;;) {
    if (!n) return; *--to = *--from; --n;
    if (!n) return; *--to = *--from; --n;
    if (!n) return; *--to = *--from; --n;
    if (!n) return; *--to = *--from; --n;
  }
}
