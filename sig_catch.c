/* $Id: sig_catch.c,v 1.1 2000/05/02 06:23:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: sig_catch.c,v $
 * Revision 1.1  2000/05/02 06:23:57  drt
 * ddns-cleand added
 *
 */

#include <signal.h>
#include "sig.h"
#include "hassgact.h"

static char rcsid[] = "$Id: sig_catch.c,v 1.1 2000/05/02 06:23:57 drt Exp $";

void sig_catch(int sig,void (*f)())
{
#ifdef HASSIGACTION
  struct sigaction sa;
  sa.sa_handler = f;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(sig,&sa,(struct sigaction *) 0);
#else
  signal(sig,f); /* won't work under System V, even nowadays---dorks */
#endif
}
