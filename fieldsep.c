/* $Id: fieldsep.c,v 1.1 2000/07/29 21:24:05 drt Exp $
 *  --drt@ailis.de
 *
 * do what you want ... at least with this
 *
 * You might find more Information at http://rc23.cx/
 * 
 * $Log: fieldsep.c,v $
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */

#include "byte.h"
#include "stralloc.h"

static char rcsid[] = "$Id: fieldsep.c,v 1.1 2000/07/29 21:24:05 drt Exp $"; 

/* split line into seperate fields */
int fieldsep(stralloc f[], int numfields, stralloc *line, char sepchar)
{
  int i, j, k;
   
  j = 0;
  for (i = 0; i < numfields; ++i) 
    {
      if (j >= line->len) 
	{
	  if (!stralloc_copys(&f[i],"")) return -1;
	}  
      else 
	{
	  k = byte_chr(line->s + j, line->len - j, sepchar);
	  if (!stralloc_copyb(&f[i], line->s + j, k)) return -1;
	  j += k + 1;
	}
    }
  return 0;
}
