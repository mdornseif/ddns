/* $Id: trysgprm.c,v 1.1 2000/05/02 06:23:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: trysgprm.c,v $
 * Revision 1.1  2000/05/02 06:23:57  drt
 * ddns-cleand added
 *
 */

#include <signal.h>

static char rcsid[] = "$Id";

main()
{
  sigset_t ss;
 
  sigemptyset(&ss);
  sigaddset(&ss,SIGCHLD);
  sigprocmask(SIG_SETMASK,&ss,(sigset_t *) 0);
}
