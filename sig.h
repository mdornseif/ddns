/* $Id: sig.h,v 1.2 2000/07/12 11:51:48 drt Exp $ 
 *   --drt@ailis.de
 *
 * mix from misc DJB software
 */

#ifndef SIG_H
#define SIG_H

extern int sig_alarm;
extern int sig_child;
extern int sig_cont;
extern int sig_hangup;
extern int sig_pipe;
extern int sig_term;

extern void (*sig_defaulthandler)();
extern void (*sig_ignorehandler)();

extern void sig_catch(int,void (*)());
#define sig_ignore(s) (sig_catch((s),sig_ignorehandler))
#define sig_uncatch(s) (sig_catch((s),sig_defaulthandler))

extern void sig_block(int);
extern void sig_unblock(int);
extern void sig_blocknone(void);
extern void sig_pause(void);

extern void sig_dfl(int);

void sig_termcatch(f) void (*f)(); { sig_catch(SIGTERM,f); }
void sig_alarmblock() { sig_block(SIGALRM); }
void sig_alarmunblock() { sig_unblock(SIGALRM); }
void sig_alarmcatch(f) void (*f)(); { sig_catch(SIGALRM,f); }
void sig_alarmdefault() { sig_catch(SIGALRM,SIG_DFL); }
#endif
