/* $Id: buffer_0.c,v 1.2 2000/11/21 19:28:22 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@un.bewaff.net
 * $Log: buffer_0.c,v $
 * Revision 1.2  2000/11/21 19:28:22  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#include "readwrite.h"
#include "buffer.h"

static char rcsid[] = "$Id: buffer_0.c,v 1.2 2000/11/21 19:28:22 drt Exp $";

int buffer_0_read(fd,buf,len) int fd; char *buf; int len;
{
  if (buffer_flush(buffer_1) == -1) return -1;
  return read(fd,buf,len);
}

char buffer_0_space[BUFFER_INSIZE];
static buffer it = BUFFER_INIT(buffer_0_read,0,buffer_0_space,sizeof buffer_0_space);
buffer *buffer_0 = &it;
