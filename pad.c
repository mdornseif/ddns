/* $Id: pad.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * $Log: pad.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/05/01 10:48:07  drt
 * imported from didentd
 *
 * Revision 1.1  2000/04/30 02:01:58  drt
 * key is now taken from the enviroment
 *
 */

#include "stralloc.h"

static char rcsid[] = "$Id: pad.c,v 1.2 2000/11/21 19:28:23 drt Exp $";

/* pad a string by repeating it or cut it off */

int pad(stralloc *sa, int len)
{
  int l;
  
  while(sa->len < len)
    {
      l = len - sa->len;
      if( l > sa->len) l = sa->len; 
      stralloc_catb(sa, sa->s, l);
    }
  sa->s[len] = '\0';
  sa->len = len;

  return len;
}
