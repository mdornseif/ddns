/* $Id: iso2txt.h,v 1.1 2000/07/14 05:54:07 drt Exp $
 *  --drt@ailis.de
 * 
 * (K) all rights reversed
 *
 * $Log: iso2txt.h,v $
 * Revision 1.1  2000/07/14 05:54:07  drt
 * *** empty log message ***
 *
 */

/* encode all unprintable characters in \xxx */
void iso2txt(char *s, int len, stralloc *sa);
