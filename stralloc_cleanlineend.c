/* $Id: stralloc_cleanlineend.c,v 1.1 2000/07/29 21:24:05 drt Exp $
 *  -- drt@ailis.de
 *
 * Clean up cruft at a line end
 * 
 * I can't own bits on your Harddisk
 *
 * You might find more Information at http://rc23.cx/
 * 
 * $Log: stralloc_cleanlineend.c,v $
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */


#include "stralloc.h"

static char rcsid[] = "$Id: stralloc_cleanlineend.c,v 1.1 2000/07/29 21:24:05 drt Exp $";

void stralloc_cleanlineend(stralloc *line)
{
  char ch;

  while(line->len) 
    {
      ch = line->s[line->len - 1];
      if ((ch != ' ') && (ch != '\t') && (ch != '\n')) break;
      --line->len;
    }
}
