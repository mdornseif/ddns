/* $Id: ddns-clientd.c,v 1.1 2000/07/12 07:22:46 drt Exp $
 *  -- drt@ailis.de
 * 
 * metaclient for ddns - driver for ddnsdc
 *
 * (K)opyright is Myth
 * 
 * $Log: ddns-clientd.c,v $
 * Revision 1.1  2000/07/12 07:22:46  drt
 * First semi-functional version
 *
 */

#include <unistd.h>    /* fork() */
#include <sys/wait.h>  /* wait() */
#include <errno.h>     /* EINTR */

#include "buffer.h"
#include "env.h"
#include "error.h"
#include "ip4.h"
#include "ip6.h"
#include "pathexec.h"
#include "sig.h"
#include "stralloc.h"
#include "strerr.h"

#include "ddns.h"
#include "loc.h"

static char rcsid[] = "$Id: ddns-clientd.c,v 1.1 2000/07/12 07:22:46 drt Exp $";

#define FATAL "ddns-clientd.c: fatal: "

static uint32 ttl = 30; 
static int flagexit = 0; 
static int flagalarmed = 0;

static char *tcpserver;
static char *ddnsc;
static char *serveraddr;
static char *serverport;
static char *ip4;
static char *ip6;
static char *loc;
stralloc locsa = {0};

static char *margv[8];

void sigterm() 
{
  flagexit = 1;
}

void sigalrm() 
{
  flagalarmed++; 
}

/* log an informational message to stderr */
void log(char *s)
{
  buffer_puts(buffer_2, "dmail-clientd: ");
  buffer_puts(buffer_2, s);
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);
}

/* log the returncode we've got back */
void log_retuncode(int r)
{
  /// XXX
  ;
}

static int call_ddnsc()
{
  // masterplan: fork then exec "tcpclient ddnsc" in the child and wait 
  // for return status in the parent. This is basically our own system()
  // call strongly influenced from DJBs tcpserver.c and the code in the 
  // system(3) manpage of glibc. 

  int pid, status;

  pid = fork();
  
  switch(pid) 
    {
    case 0:
      // this is the children
      // XXX: fix signals
      //      sig_uncatch(sig_child);
      //      sig_unblock(sig_child);
      sig_uncatch(sig_term);
      //      sig_uncatch(sig_pipe);
      // execute child
      pathexec(margv);
      // we sholden't reach this
      strerr_die4sys(111, FATAL, "unable to run ", *margv, ": ");
      
    case -1:
      strerr_warn2(FATAL, "unable to fork: ", &strerr_sys);
    }

  // this is the parent, we have to wait for the children and catch its return status
  for(;;)
    {
      if(wait(&status) == -1)
	{
	  if(errno != EINTR)
	    strerr_die2sys(111, FATAL, "can't wait() for children");
	}
      else 
	return status + 1;
    }  
}

static int set_entry()
{
  margv[4] = "s";
  return call_ddnsc();
}

static int renew_entry()
{
  margv[4] = "r";
  return call_ddnsc();
}

static int kill_entry()
{
  margv[4] = "k";
  return call_ddnsc();
}

// send killentryrequest to server and terminate
static int terminate()
{
  int r;

  log("SIGTERM recived, deregistering");
  r = kill_entry(ip4, ip6, loc);

  if(r != DDNS_T_ACK)
    {
      // we have a problem. Print Error and exit 
      log_retuncode(r);
      strerr_die2x(100, FATAL, "could't kill entry at server, dieing");
    }
  
  // Bye
  log("exiting");
  exit(0);
}

int main(int argc, char *argv[])
{
  int r;
   
  /* read configuration from commandline */
  if(argc < 2)
    {
      strerr_die2x(111, FATAL, "usage: ddns-clientd IPv4 IPv6 LOCATION
       where LOCATION is someting like `50 57 9.7 N 6 54 08.3 E 5700 5000 1500 5000'
       which describes a location at 50\26057'9.7\" north 6\26054'8.3\" east at
       an altitude of 5700cm with 5000cm size. This value is 1500cm acurate in 
       horizontal and 5000cm in vertical direction. The last four values are optional.\n\n");
    }
 
  tcpserver = "/bin/echo";
  serveraddr = "c0re.rc23.cx";
  serverport = "3456";
  ddnsc = "./ddnsc";
  ip4 = argv[1];
  ip6 = argv[2];

  /* put remaining args into a single string */
  for(r=3; r < argc; r++)
    {
      stralloc_cats(&locsa, argv[r]);
      stralloc_append(&locsa, " ");
    }
  
  stralloc_0(&locsa);
  loc = locsa.s;

  margv[0] = tcpserver;
  margv[1] = serveraddr;
  margv[2] = serverport;
  margv[3] = ddnsc;
  margv[4] = "s";
  margv[5] = ip4;
  margv[6] = ip6;
  margv[7] = loc;

  
  /* register at server */
  r = set_entry(ip4, ip6, loc);
  
  if(r == DDNS_T_EALLREADYUSED)
    {
      // deregister and try again
      log("warning there is already an entry. killing and reregistering");
      r = kill_entry(ip4, ip6, loc);
      r = set_entry(ip4, ip6, loc);
    }
    
  if(r != DDNS_T_ACK)
    {
      // we have a problem. Print Error and exit 
      log_retuncode(r);
      strerr_die2x(100, FATAL, "could't register at server");
    }

  // catch SIGTERM
  sig_termcatch(sigterm);
  // ignore SIGALRM
  sig_alarmcatch(sigalrm);

  for(;;)
    {
      // sleep for ttl-17 seconds
      sleep(ttl-17);
      
      if(flagexit)
	{
	  // we've catched a SIGTERM	  
	  terminate(ip4, ip6, loc);
	}
      
      // renew registration
      r = renew_entry(ip4, ip6, loc);
      if(r != DDNS_T_ACK)
	{
	  // we have a problem. Print Error and exit 
	  log_retuncode(r);
	  strerr_die2x(100, FATAL, "could't renew entry at server");
	}
    }
}

