/* $Id: now.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@un.bewaff.net
 * $Log: now.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#include <time.h>
#include "datetime.h"
#include "now.h"

static char rcsid[] = "$Id: now.c,v 1.2 2000/11/21 19:28:23 drt Exp $";

datetime_sec now()
{
  return time((long *) 0);
}
