/* $Id: mt19937.h,v 1.1 2000/04/30 15:59:26 drt Exp $
 *
 * $Log: mt19937.h,v $
 * Revision 1.1  2000/04/30 15:59:26  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.2  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.1.1.1  2000/04/16 22:07:53  drt
 * initial ddns version
 *
 */

#include "uint32.h"

void seedMT(uint32 seed);
void reloadMT(void);
unsigned long randomMT(void);
void blockMT(void *mem, unsigned int len);
void blockMTxor(void *mem, unsigned int len);
