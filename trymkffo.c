/* $Id: trymkffo.c,v 1.1 2000/07/29 21:24:05 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 *
 * You might find more information at http://rc23.cx/
 *
 * $Log: trymkffo.c,v $
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */

#include <sys/types.h>
#include <sys/stat.h>

void main()
{
  mkfifo("temp-trymkffo",0);
}
