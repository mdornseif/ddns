/* $Id: sig_block.c,v 1.1 2000/05/02 06:23:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: sig_block.c,v $
 * Revision 1.1  2000/05/02 06:23:57  drt
 * ddns-cleand added
 *
 */

#include <signal.h>
#include "sig.h"
#include "hassgprm.h"

static char rcsid[] = "$Id: sig_block.c,v 1.1 2000/05/02 06:23:57 drt Exp $";

void sig_block(int sig)
{
#ifdef HASSIGPROCMASK
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss,sig);
  sigprocmask(SIG_BLOCK,&ss,(sigset_t *) 0);
#else
  sigblock(1 << (sig - 1));
#endif
}

void sig_unblock(int sig)
{
#ifdef HASSIGPROCMASK
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss,sig);
  sigprocmask(SIG_UNBLOCK,&ss,(sigset_t *) 0);
#else
  sigsetmask(sigsetmask(~0) & ~(1 << (sig - 1)));
#endif
}

void sig_blocknone(void)
{
#ifdef HASSIGPROCMASK
  sigset_t ss;
  sigemptyset(&ss);
  sigprocmask(SIG_SETMASK,&ss,(sigset_t *) 0);
#else
  sigsetmask(0);
#endif
}
