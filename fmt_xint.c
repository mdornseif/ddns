/* $Id: fmt_xint.c,v 1.1 2000/04/30 14:56:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: fmt_xint.c,v $
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#include "fmt.h"

static char rcsid[] = "$Id: fmt_xint.c,v 1.1 2000/04/30 14:56:57 drt Exp $";

unsigned int fmt_xint(char *s,unsigned int u)
{
 unsigned long l; l = u; return fmt_xlong(s,l);
}
