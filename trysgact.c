/* $Id: trysgact.c,v 1.1 2000/05/02 06:23:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: trysgact.c,v $
 * Revision 1.1  2000/05/02 06:23:57  drt
 * ddns-cleand added
 *
 */

#include <signal.h>

static char rcsid[] = "$Id: trysgact.c,v 1.1 2000/05/02 06:23:57 drt Exp $";

main()
{
  struct sigaction sa;
  sa.sa_handler = 0;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(0,&sa,(struct sigaction *) 0);
}
