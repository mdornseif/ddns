/* $Id: fd_copy.c,v 1.3 2000/11/21 19:28:23 drt Exp $
 *  --drt@un.bewaff.net
 * 
 * form ucspi-tcp-0.88 by DJB
 *
 * $Log: fd_copy.c,v $
 * Revision 1.3  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.2  2000/07/31 19:03:18  drt
 * initial revision
 *
 * Revision 1.1  2000/07/12 11:51:48  drt
 * ddns-clientd
 *
 */

#include <fcntl.h>
#include "fd.h"

extern int close(int);

int fd_copy(int to,int from)
{
  if (to == from) return 0;
  if (fcntl(from,F_GETFL,0) == -1) return -1;
  close(to);
  if (fcntl(from,F_DUPFD,to) == -1) return -1;
  return 0;
}
