/* $Id: ddns-clientd.c,v 1.5 2000/07/13 18:20:47 drt Exp $
 *  -- drt@ailis.de
 * 
 * client for ddns
 *
 * (K)opyright is Myth
 * 
 * $Log: ddns-clientd.c,v $
 * Revision 1.5  2000/07/13 18:20:47  drt
 * everything supports now DNS LOC
 *
 * Revision 1.4  2000/07/13 14:16:27  drt
 * more logging in ddns-clientd and catching SIGINT
 *
 * Revision 1.3  2000/07/12 12:33:50  drt
 * fixed the buildprocess
 *
 * Revision 1.2  2000/07/12 11:34:25  drt
 * ddns-clientd handels now everything itself.
 * ddnsc is now linked to ddnsd-clientd, do a
 * enduser needs just this single executable
 * and no ucspi-tcp/tcpclient.
 *
 * Revision 1.1  2000/07/12 07:22:46  drt
 * First semi-functional version
 *
 */

#include <errno.h>     /* EINTR */
#include <stdlib.h>    /* random, clock */
#include <sys/wait.h>  /* wait() */
#include <time.h>      /* time */
#include <unistd.h>    /* close, getpid, getppid */

#include "buffer.h"
#include "dns.h"
#include "env.h"
#include "error.h"
#include "fd.h"
#include "ip4.h"
#include "ip6.h"
#include "scan.h"
#include "sig.h"
#include "socket.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutconn.h"
#include "uint16.h"

#include "pad.h"
#include "txtparse.h"
#include "mt19937.h"
#include "loc.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddns-clientd.c,v 1.5 2000/07/13 18:20:47 drt Exp $";

#define FATAL "ddns-clientd: fatal: "

extern int ddnsc(int type, uint32 uid, char *ip4, char *ip6, struct loc_s *loc, char *key, uint32 *ttl);

/* some globals to keep us from passing to manny parameters arround */
char ip4[IP4_FMT];
char ip6[IP6_FMT];
struct loc_s loc;
stralloc key = {0};
uint32 ttl = 30; 
char *serverip;
uint32 port = 0;
uint32 uid = 0;

static int flagexit = 0; 
static int flagalarmed = 0;

void die_nomem(void)
{
  strerr_die1sys(110, "ddnsd: fatal: help - no memory ");
}

void sigterm() { flagexit = 1; }

void sigalrm() { flagalarmed++; }

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
  switch(r)
    {
    case DDNS_T_ACK: 
      buffer_puts(buffer_1, "ACK: leasetime ");
      //  buffer_put(buffer_1, strnum, fmt_ulong(strnum, r.leasetime));
      buffer_puts(buffer_1, "\n");
      break;
    case DDNS_T_NAK: 
      buffer_puts(buffer_1, "NAK: mail drt@ailis.de\n"); 
      break;
    case DDNS_T_ESERVINT: 
      buffer_puts(buffer_1, "EIN: internal server error\n"); 
      break;
    case DDNS_T_EPROTERROR: 
      buffer_puts(buffer_1, "EPR: mail drt@ailis.de\n"); 
      break;   
    case DDNS_T_EWRONGMAGIC: 
      buffer_puts(buffer_1, "EWM: server thinks we send him a message with bad magic\n");
      break;  
    case DDNS_T_ECANTDECRYPT: 
      buffer_puts(buffer_1, "EDC: server can not decrypt our message\n"); 
      break; 
    case DDNS_T_EALLREADYUSED: 
      buffer_puts(buffer_1, "EUD: uid has already registered an ip\n"); 
      break;
    case DDNS_T_EUNKNOWNUID: 
      buffer_puts(buffer_1, "EID: uid is unknown to the server\n"); 
      break;  
    case DDNS_T_ENOENTRYUSED:
	buffer_puts(buffer_1, "ENE: client requsted to renew/kill something which is not set\n");
	break;
    case DDNS_T_EUNSUPPTYPE: 
      buffer_puts(buffer_1, "EUS: \n"); 
      break;  
    default:
      strerr_die1x(100, "unknown packet");
    }
  buffer_flush(buffer_1);  
}

/* basically an emebbed tcpclient */
int buildup_connection(char *ip, uint16 p)
{
  int s;
  
  s = socket_tcp();
  if (s == -1)
    strerr_die2sys(100, FATAL, "unable to create socket: ");
  if (socket_bind4(s, "\0\0\0\0", 0) == -1)
    strerr_die2sys(100, FATAL, "unable to bind socket: ");
  if (timeoutconn(s, ip, p, 120) != 0)
    strerr_die2sys(100, FATAL, "unable to connect: ");

  //  socket_tcpnodelay(s); /* if it fails, bummer */

  if (fd_move(6,s) == -1)
    strerr_die2sys(100, FATAL,"unable to set up descriptor 6: ");
  if (fd_copy(7,6) == -1)
    strerr_die2sys(100, FATAL,"unable to set up descriptor 7: ");
  
  return 1;
}

void teardown_connection()
{
  close(6);
  close(7);
}

static int call_ddnsc(int action)
{
  int r;

  if(!buildup_connection(serverip, port))
    strerr_die2sys(100, FATAL, "could't connect to server");
  r = ddnsc(action, uid, ip4, ip6, &loc, key.s, &ttl);
  teardown_connection();

  return r;
}

static int set_entry()
{
  return call_ddnsc(DDNS_T_SETENTRY);
}

static int renew_entry()
{
  return call_ddnsc(DDNS_T_RENEWENTRY);
}

static int kill_entry()
{
  return call_ddnsc(DDNS_T_KILLENTRY);
}

// send killentryrequest to server and terminate
static int terminate()
{
  int r;

  log("Signal recived, deregistering");
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

void doit(void)
{
  int r;
  char strip[IP6_FMT];
  stralloc sa = {0};

  stralloc_copys(&sa, "registering at server from ");
  loc_ntoa(&loc, &sa);
  stralloc_cats(&sa, " with ");
  stralloc_catb(&sa, strip, ip4_fmt(strip, ip4));
  stralloc_cats(&sa, "/");
  stralloc_catb(&sa, strip, ip6_fmt(strip, ip6));
  stralloc_cats(&sa, "\n");  
  stralloc_0(&sa);
  log(sa.s);

  /* register at server */
  r = set_entry();
  
  if(r == DDNS_T_EALLREADYUSED)
    {
      // deregister and try again
      log("warning there is already an entry. killing and reregistering");
      r = kill_entry();
      r = set_entry();
    }
    
  if(r != DDNS_T_ACK)
    {
      // we have a problem. Print Error and exit 
      log_retuncode(r);
      strerr_die2x(100, FATAL, "could't register at server");
    }

  // catch SIGTERM & SIGINT
  sig_termcatch(sigterm);
  sig_intcatch(sigterm);
  // ignore SIGALRM
  sig_alarmcatch(sigalrm);

  for(;;)
    {
      // sleep for ttl-17 seconds
      sleep(ttl-17);
      
      if(flagexit)
	{
	  // we've catched a SIGTERM	  
	  terminate();
	}
      
      // renew registration
      log("renewing entry");
      r = renew_entry();
      if(r != DDNS_T_ACK)
	{
	  // we have a problem. Print Error and exit 
	  log_retuncode(r);
	  strerr_die2x(100, FATAL, "could't renew entry at server");
	}
    }
}

int main(int argc, char *argv[])
{
  int r;
  char *x;
  char *sva = NULL;
  char *serverport = NULL;
  stralloc serversa = {0};
  stralloc fqdn = {0};
  stralloc svasa = {0};
  stralloc locsa = {0};

  log("starting");

  /* seed some entropy into the MT */
  seedMT((long long) getpid () *
	 (long long) time(0) *
	 (long long) getppid() * 
	 (long long) random() * 
	 (long long) clock());
   
  /* read configuration from commandline */
  if(argc < 3)
    {
      strerr_die2x(111, FATAL, "usage: ddns-clientd IPv4 IPv6 LOCATION
       where LOCATION is someting like `50 57 9.7 N 6 54 08.3 E 5700 5000 1500 5000'
       which describes a location at 50\26057'9.7\" north 6\26054'8.3\" east at
       an altitude of 5700cm with 5000cm size. This value is 1500cm acurate in 
       horizontal and 5000cm in vertical direction. The last four values are optional.\n\n");
    }
   
  // XXX: try to read from configfile

  /* get key from enviroment */
  x = env_get("KEY");
  if (x)
    stralloc_copys(&key, x);

  /* get server addr from enviroment */
  x = env_get("DDNS_SERVER_ADDR");
  if (x)
    sva = x;

  /* get server port from enviroment */
  x = env_get("DDNS_SERVER_PORT");
  if (x)
    serverport = x;

  /* get UID from enviroment */
  x = env_get("DDNS_UID");
  if (x)
    scan_ulong(x, &uid);
 
  /* get ddns uid */
  if(uid == 0) 
    strerr_die2x(111, FATAL, "uid not set (try $DDNS_UID)");

  /* get key */
  if(key.len == 0)       
    strerr_die2x(111, FATAL, "key not set (try $KEY)");
  
  /* mangle keymaterial */
  txtparse(&key);
  pad(&key, 32);
  
  // check/convert ip-adresses
  if(!sva)       
    strerr_die2x(111, FATAL, "server address not set (try $DDNS_SERVER_ADDR)");
  
  if (!stralloc_copys(&svasa, sva)) 
    die_nomem();
  if (dns_ip4_qualify(&serversa, &fqdn, &svasa) == -1)
    strerr_die4sys(111, FATAL, "temporarily unable to figure out IP address for ", sva, ": ");
  if (serversa.len < 4)
    strerr_die3x(111, FATAL, "no IP address for ", sva);
  serverip = serversa.s;

  if(!serverport)       
    strerr_die2x(111, FATAL, "server port not set (try $DDNS_SERVER_PORT)");
  scan_ulong(serverport, &port);
  
  ip4_scan(argv[1], ip4);
  ip6_scan(argv[2], ip6); 
   
  /* create log_s */
  /* put remaining args (loc) into a single string */
  for(r=3; r < argc; r++)
    {
      stralloc_cats(&locsa, argv[r]);
      stralloc_append(&locsa, " ");
    }
  
  stralloc_0(&locsa);
  
  if(!loc_aton(locsa.s, &loc))
    strerr_die3x(111, FATAL, "can't find a LOCation in ", locsa.s);
    
  /* all there now, here we go */ 
  doit();

  return 0;
}

