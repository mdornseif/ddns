/* $Id: sig_int.c,v 1.1 2000/07/13 14:16:27 drt Exp $
 *  --drt@ailis.de
 * 
 * clone of sig_term.c from ucspi-tcp-0.88
 *
 * $Log: sig_int.c,v $
 * Revision 1.1  2000/07/13 14:16:27  drt
 * more logging in ddns-clientd and catching SIGINT
 *
 */

#include <signal.h>
#include "sig.h"

static char rcsid[] = "$Id: sig_int.c,v 1.1 2000/07/13 14:16:27 drt Exp $";

void sig_intblock() { sig_block(SIGINT); }
void sig_intunblock() { sig_unblock(SIGINT); }
void sig_intcatch(f) void (*f)(); { sig_catch(SIGINT,f); } 
void sig_intdefault() { sig_catch(SIGINT,SIG_DFL); }
