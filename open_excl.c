/* $Id: open_excl.c,v 1.3 2000/11/21 19:28:23 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@un.bewaff.net
 * $Log: open_excl.c,v $
 * Revision 1.3  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.2  2000/10/06 22:03:15  drt
 * Library reorganisation
 *
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

static char rcsid[] = "$Id: open_excl.c,v 1.3 2000/11/21 19:28:23 drt Exp $";

int open_excl(const char *fn)
{ return open(fn,O_WRONLY | O_EXCL | O_CREAT,0644); }
