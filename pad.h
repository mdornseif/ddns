/* $Id: pad.h,v 1.1 2000/05/01 10:48:07 drt Exp $
 *  --drt@ailis.de
 * 
 * $Log: pad.h,v $
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
