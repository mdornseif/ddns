/* $Id: sig_term.c,v 1.2 2000/07/12 12:33:50 drt Exp $
 *  --drt@ailis.de
 * 
 * from ucspi-tcp-0.88
 *
 * $Log: sig_term.c,v $
 * Revision 1.2  2000/07/12 12:33:50  drt
 * fixed the buildprocess
 *
 * Revision 1.1  2000/07/12 11:51:48  drt
 * ddns-clientd
 *
 */

#include <signal.h>
#include "sig.h"

static char rcsid[] = "$Id: sig_term.c,v 1.2 2000/07/12 12:33:50 drt Exp $";

void sig_termblock() { sig_block(SIGTERM); }
void sig_termunblock() { sig_unblock(SIGTERM); }
void sig_termcatch(f) void (*f)(); { sig_catch(SIGTERM,f); } 
void sig_termdefault() { sig_catch(SIGTERM,SIG_DFL); }
