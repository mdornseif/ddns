/* $Id: txtparse.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * $Log: txtparse.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/05/01 11:09:21  drt
 * converted from didentd
 *
 * Revision 1.1  2000/04/30 02:01:58  drt
 * key is now taken from the enviroment
 *
 */

#include "stralloc.h"

static char rcsid[] = "$Id: txtparse.c,v 1.2 2000/11/21 19:28:23 drt Exp $";

/* change encoded octets (\012) to their 'real' values (\n) */

void txtparse(stralloc *sa)
{
  char ch;
  unsigned int i;
  unsigned int j;

  j = 0;
  i = 0;
  while (i < sa->len) 
    {
      ch = sa->s[i++];
      if (ch == '\\') 
	{
	  if (i >= sa->len) 
	    {
	      break;
	    }
	  ch = sa->s[i++];
	  if ((ch >= '0') && (ch <= '7')) 
	    {
	      ch -= '0';
	      if ((i < sa->len) && (sa->s[i] >= '0') && (sa->s[i] <= '7')) 
		{
		  ch <<= 3;
		  ch += sa->s[i++] - '0';
		  if ((i < sa->len) && (sa->s[i] >= '0') && (sa->s[i] <= '7')) 
		    {
		      ch <<= 3;
		      ch += sa->s[i++] - '0';
		    }
		}
	    }
	}
      sa->s[j++] = ch;
    }
  sa->len = j;
}
