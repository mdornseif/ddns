/* $Id: sig_alarm.c,v 1.1 2000/05/02 06:23:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: sig_alarm.c,v $
 * Revision 1.1  2000/05/02 06:23:57  drt
 * ddns-cleand added
 *
 */

#include <signal.h>
#include "sig.h"

static char rcsid[] = "$Id: sig_alarm.c,v 1.1 2000/05/02 06:23:57 drt Exp $";

void sig_alarmblock() { sig_block(SIGALRM); }
void sig_alarmunblock() { sig_unblock(SIGALRM); }
void sig_alarmcatch(f) void (*f)(); { sig_catch(SIGALRM,f); }
void sig_alarmdefault() { sig_catch(SIGALRM,SIG_DFL); }
