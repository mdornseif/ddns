/* $Id: iso2txt.h,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * (K) all rights reversed
 *
 * $Log: iso2txt.h,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/14 05:54:07  drt
 * *** empty log message ***
 *
 */

/* encode all unprintable characters in \xxx */
void iso2txt(char *s, int len, stralloc *sa);
