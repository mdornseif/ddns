/* $Id: ddnsd_log.c,v 1.1 2000/07/31 19:03:17 drt Exp $
 *   --drt@ailis.de
 *
 * logging for ddnsd
 * 
 * (K)allisti
 * 
 * you might find more Information at http://rc23.cx/
 * 
 * $Log: ddnsd_log.c,v $
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include "buffer.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "stralloc.h"
#include "strerr.h"

#include "ddns.h"
#include "ddnsd.h"

static char rcsid[] = "$Id: ddnsd_log.c,v 1.1 2000/07/31 19:03:17 drt Exp $";

extern void die_nomem(void);

static unsigned char *remotehost, *remoteinfo, *remoteip, *remoteport;

static void ddnsd_log_init(void)
{
  /* since we run under tcpserver, we can get all info 
     about the remote side from the enviroment */
  remotehost = env_get("TCPREMOTEHOST");
  if (!remotehost) remotehost = "unknown";
  remoteinfo = env_get("TCPREMOTEINFO");
  if (!remoteinfo) remoteinfo = "-";
  remoteip = env_get("TCPREMOTEIP");
  if (!remoteip) remoteip = "unknown";
  remoteport = env_get("TCPREMOTEPORT");
  if (!remoteport) remoteport = "unknown";

  /* now: 
     remotehost is the remote hostname or "unknown" 
     remoteinfo is some ident string or "-"
     remoteip is the remote ipadress or "unknown" (?)
  */
}


void ddnsd_log(uint32 uid, char *str)
{
  char strnum[FMT_ULONG];
  
  ddnsd_log_init();

  /* Do logging */
  buffer_puts(buffer_2, "ddnsd ");
  buffer_puts(buffer_2, remotehost);
  buffer_puts(buffer_2, " [");
  buffer_puts(buffer_2, remoteip);  
  buffer_puts(buffer_2, ":");
  buffer_puts(buffer_2, remoteport);
  buffer_puts(buffer_2, "] ");
  buffer_puts(buffer_2, remoteinfo);
  buffer_puts(buffer_2, " ");
  buffer_put(buffer_2, strnum, fmt_ulong(strnum, uid));
  buffer_puts(buffer_2, ": ");
  buffer_puts(buffer_2, str);
  buffer_flush(buffer_2);

  buffer_putsflush(buffer_2, "\n");
}

/* sends an error packet to the client and logs an error */
void ddnsd_send_err(uint32 uid, uint16 errtype, char *errstr)
{
  struct ddnsreply p;
  
  p.type = errtype;
  p.uid = uid;
  
  p.leasetime = 0;
  
  ddnsd_log(uid, errstr);

  ddnsd_send(&p);
  _exit(111);
}

/* sends an error packet to the client and logs an error 
   including a system error message */ 
void ddnsd_send_err_sys(uint32 uid, char *errstr)
{
  stralloc err = {0};

  if(!stralloc_copys(&err, errstr)) die_nomem();
  if(!stralloc_cats(&err, ": ")) die_nomem();
  if(!stralloc_cats(&err, error_str(errno))) die_nomem();
  if(!stralloc_0(&err)) die_nomem();
  ddnsd_send_err(uid, DDNS_T_ESERVINT, err.s);
}

