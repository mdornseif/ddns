#include "byte.h"

void byte_copy(register void* To,
	       register unsigned int n,
	       register const void* From)
{
  register char *to=To;
  register const char *from=From;
  for (;;) {
    if (!n) return; *to++ = *from++; --n;
    if (!n) return; *to++ = *from++; --n;
    if (!n) return; *to++ = *from++; --n;
    if (!n) return; *to++ = *from++; --n;
  }
}
