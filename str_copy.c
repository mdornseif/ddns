/* $Id: str_copy.c,v 1.1 2000/07/31 19:03:18 drt Exp $
 *  --drt@ailis.de
 * 
 * Hacked by drt to emulate DJBs str_copy()
 *
 * $Log: str_copy.c,v $
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include "str.h"

static char rcsid[] = "$Id: str_copy.c,v 1.1 2000/07/31 19:03:18 drt Exp $";

extern unsigned int str_copy(char *to, char *from)
{
  int n;

  for (n = 0;;) {
    if (!*from) return n; *to++ = *from++; n++;
    if (!*from) return n; *to++ = *from++; n++;
    if (!*from) return n; *to++ = *from++; n++;
    if (!*from) return n; *to++ = *from++; n++;
  }
}
