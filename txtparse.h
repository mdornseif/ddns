/* $Id: txtparse.h,v 1.2 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * $Log: txtparse.h,v $
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

/* change encoded octets (\012) to their 'real' values (\n) */

extern void txtparse(stralloc *sa);
