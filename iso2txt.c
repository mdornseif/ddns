/* $Id: iso2txt.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * (K) all rights reversed
 *
 * $Log: iso2txt.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/14 05:54:07  drt
 * *** empty log message ***
 *
 */

#include "stralloc.h"

static char rcsid[] = "$Id: iso2txt.c,v 1.2 2000/11/21 19:28:23 drt Exp $";

/* encode all unprintable characters and  `,' in \xxx */

void iso2txt(char *s, int len, stralloc *sa)
{
  unsigned char *c;

  for(c = s; c < s + len; c++)
    {
      if(*c > 127 || *c < 32 || *c == '\\' || *c == ',')
        {
          stralloc_append(sa, "\\");
          stralloc_readyplus(sa, 3);
          sa->s[sa->len++] = ((*c & 0xc0) >> 6) + '0';
          sa->s[sa->len++] = ((*c & 0x38) >> 3) + '0';
          sa->s[sa->len++] = (*c & 0x07) + '0';
	}    
      else
	stralloc_append(sa, c);
    }      	
}
