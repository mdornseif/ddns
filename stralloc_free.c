/* $Id: stralloc_free.c,v 1.1 2000/07/31 19:03:18 drt Exp $
 *  -- drt@ailis.de
 *
 * Perhaps better use a garbage collector ?
 *
 * $Log: stralloc_free.c,v $
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include "alloc.h"
#include "stralloc.h"

static char rcsid[] = "$Id: stralloc_free.c,v 1.1 2000/07/31 19:03:18 drt Exp $";

void stralloc_free(stralloc *sa)
{
  alloc_free(sa->s);
  sa->s = 0;
  sa->len = sa->a = 0;
}
