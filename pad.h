/* $Id: pad.h,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * $Log: pad.h,v $
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

/* adjust sa to len by cutting it off at the end or by 
   repeating the string until we are at len
*/

extern int pad(stralloc *sa, int len);
