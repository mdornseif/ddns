/* $Id: socket_delay.c,v 1.2 2000/07/31 19:15:56 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 *
 * You might find more information at http://rc23.cx/
 *
 * $Log: socket_delay.c,v $
 * Revision 1.2  2000/07/31 19:15:56  drt
 * ddns-file(5) format changed
 * a lot of restructuring
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket.h"

static char rcsid[]="$Id: socket_delay.c,v 1.2 2000/07/31 19:15:56 drt Exp $";

int socket_tcpnodelay(int s)
{
  int opt = 1;
  return setsockopt(s,IPPROTO_TCP,1,&opt,sizeof opt); /* 1 == TCP_NODELAY */
}
