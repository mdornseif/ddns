/* $Id: coe.c,v 1.2 2000/11/21 19:28:22 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@un.bewaff.net
 *
 * You might find more information at http://rc23.cx/
 *
 * $Log: coe.c,v $
 * Revision 1.2  2000/11/21 19:28:22  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */

#include <fcntl.h>
#include "coe.h"

static char rcsid[] = "Id";

int coe(int fd)
{
  return fcntl(fd,F_SETFD,1);
}
