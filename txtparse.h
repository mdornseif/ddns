/* $Id: txtparse.h,v 1.1 2000/05/01 11:09:21 drt Exp $
 *  --drt@ailis.de
 * 
 * $Log: txtparse.h,v $
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
