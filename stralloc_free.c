/* $Id: stralloc_free.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  -- drt@un.bewaff.net
 *
 * Perhaps better use a garbage collector ?
 *
 * $Log: stralloc_free.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include "alloc.h"
#include "stralloc.h"

static char rcsid[] = "$Id: stralloc_free.c,v 1.2 2000/11/21 19:28:23 drt Exp $";

void stralloc_free(stralloc *sa)
{
  alloc_free(sa->s);
  sa->s = 0;
  sa->len = sa->a = 0;
}
