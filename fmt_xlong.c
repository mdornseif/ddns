/* $Id: fmt_xlong.c,v 1.1 2000/04/30 14:56:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: fmt_xlong.c,v $
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#include "fmt.h"

static char rcsid[] = "$Id: fmt_xlong.c,v 1.1 2000/04/30 14:56:57 drt Exp $";

unsigned int fmt_xlong(register char *s,register unsigned long u)
{
  register unsigned int len; register unsigned long q; register char c;
  len = 1; q = u;
  while (q > 15) { ++len; q /= 16; }
  if (s) {
    s += len;
    do { c = '0' + (u & 15); if (c > '0' + 9) c += 'a' - '0' - 10;
    *--s = c; u /= 16; } while(u);
  }
  return len;
}
