#include "fmt.h"

unsigned int fmt_uint(register char* s,register unsigned int u)
{
  register unsigned long l; l = u; return fmt_ulong(s,l);
}
