/* $id$
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: open_write.c,v $
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */

#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int open_write(char *fn)
{ return open(fn,O_WRONLY | O_NDELAY); }
