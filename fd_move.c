/* $Id: fd_move.c,v 1.1 2000/07/12 11:51:48 drt Exp $
 *  --drt@ailis.de
 * 
 * form ucspi-tcp-0.88 by DJB
 *
 * $Log: fd_move.c,v $
 * Revision 1.1  2000/07/12 11:51:48  drt
 * ddns-clientd
 *
 */

#include "fd.h"

int fd_move(int to,int from)
{
  if (to == from) return 0;
  if (fd_copy(to,from) == -1) return -1;
  close(from);
  return 0;
}
