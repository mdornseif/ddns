/* $Id: sig.c,v 1.1 2000/07/12 11:51:48 drt Exp $
 *  --drt@ailis.de
 * 
 * From misc DJB software
 *
 * $Log: sig.c,v $
 * Revision 1.1  2000/07/12 11:51:48  drt
 * ddns-clientd
 *
 */

#include <signal.h>
#include "sig.h"

int sig_alarm = SIGALRM;
int sig_child = SIGCHLD;
int sig_cont = SIGCONT;
int sig_hangup = SIGHUP;
int sig_pipe = SIGPIPE;
int sig_term = SIGTERM;

void (*sig_defaulthandler)() = SIG_DFL;
void (*sig_ignorehandler)() = SIG_IGN;
