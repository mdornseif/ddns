/* $Id: fifo.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@un.bewaff.net
 *
 * You might find more information at http://rc23.cx/
 *
 * $Log: fifo.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "hasmkffo.h"
#include "fifo.h"

static char rcsid[] = "Id";

#ifdef HASMKFIFO
int fifo_make(char *fn,int mode) { return mkfifo(fn,mode); }
#else
int fifo_make(char *fn,int mode) { return mknod(fn,S_IFIFO | mode,0); }
#endif
