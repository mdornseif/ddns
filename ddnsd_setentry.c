/* $Id: ddnsd_setentry.c,v 1.1 2000/07/31 19:03:18 drt Exp $
 *  -- drt@ailis.de - http://rc23.cx/
 *
 * (K)allisti 2000 a.D. - all rights reversed
 *
 * $Log: ddnsd_setentry.c,v $
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include <netdb.h>     /* gethostbyname */
#include <stdio.h>     /* rename */
#include <sys/stat.h>  /* stat, umask */
#include <sys/types.h> /* utimbuf */
#include <unistd.h>    /* close, getpid, getppid, sleep, unlink */

#include "buffer.h"
#include "byte.h"
#include "error.h"
#include "fmt.h"
#include "ip4.h"
#include "ip6.h"
#include "now.h"
#include "open.h"
#include "readwrite.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutwrite.h"

#include "iso2txt.h"
#include "loc.h"

#include "ddns.h"
#include "ddnsd.h"

static char rcsid[] = "$Id: ddnsd_setentry.c,v 1.1 2000/07/31 19:03:18 drt Exp $";

/* handle a setentryrequest */
void ddnsd_setentry(struct ddnsrequest *p, uint32 *ttl, stralloc *username)
{
  struct loc_s loc;
  struct ddnsreply r;
  struct stat st;
  char host[64] = {0};
  int loop = 0;
  int fd = 0;
  stralloc out = {0};
  stralloc tmpname = {0};
  stralloc err = {0};
  stralloc finname = {0};
  stralloc fifo = {0};
  char strnum[FMT_ULONG];
  char struid[FMT_ULONG];
  char strip4[IP4_FMT];
  char strip6[IP6_FMT];
  char tb[16];
  
  /* create a temporary name */
  host[0] = 0;
  gethostname(host,sizeof(host));
  for (loop = 0;;++loop)
    {
      if(!stralloc_copys(&tmpname, "tmp/")) die_nomem();
      if(!stralloc_catulong0(&tmpname, now(),0)) die_nomem(); 
      if(!stralloc_cats(&tmpname, ".")) die_nomem(); 
      if(!stralloc_catulong0(&tmpname, getpid(), 0)) die_nomem(); 
      if(!stralloc_cats(&tmpname, ".")) die_nomem(); 
      if(!stralloc_cats(&tmpname, host)) die_nomem(); 
      if(!stralloc_cats(&tmpname, "-")) die_nomem(); 
      if(!stralloc_cat(&tmpname, username)) die_nomem(); 
      if(!stralloc_0(&tmpname)) die_nomem();

      if (stat(tmpname.s,&st) == -1) 
	if (errno == error_noent) 
	  break;
      /* really should never get to this point */
      if (loop == 2) 
	_exit(1); // XXX: logging 
      sleep(1);
    }
  
  /* create the final name */
  create_datafilename(&finname, username);
  
  /* open tmpfile and write to it */
  fd = open_excl(tmpname.s);
  
  if(fd == -1)
    ddnsd_send_err_sys(p->uid, tmpname.s);
  
  /* XXX: we need checks if the user is comming from the claimed ip */

  struid[fmt_ulong(struid, p->uid)] = 0;
  strip4[ip4_fmt(strip4, (char *) &p->ip4)] = 0;
  strip6[ip6_fmt(strip6, (char *) &p->ip6)] = 0;
  
  /* ip4 */
  if(byte_diff(p->ip4, 4, "\0\0\0\0"))
    {
      stralloc_cats(&out, "=,");
      stralloc_cats(&out, struid);
      stralloc_cats(&out, ",");
      stralloc_cats(&out, strip4);
      stralloc_cats(&out, "\n");
    }

  /* ip6 */
  if(byte_diff(p->ip6, 16, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"))
    {
      stralloc_cats(&out, "6,");
      stralloc_cats(&out, struid);
      stralloc_cats(&out, ",");
      stralloc_cats(&out, strip6);
      stralloc_cats(&out, "\n");
    }

  /* LOC */
  loc.size = p->loc_size;
  loc.vpre = p->loc_hpre;
  loc.hpre = p->loc_vpre;
  loc.latitude = p->loc_lat;
  loc.longitude = p->loc_long;
  loc.altitude = p->loc_alt;
 
  // XXX cleanup
  tb[0] = 0;
  tb[1] = p->loc_size;
  /* XXX: something is mixed up here, fixme, use loc_pack */
  tb[3] = p->loc_hpre;
  tb[2] = p->loc_vpre;
  uint32_pack_big(&tb[4], p->loc_lat); 
  uint32_pack_big(&tb[8], p->loc_long); 
  uint32_pack_big(&tb[12], p->loc_alt);  
 
  stralloc_cats(&out, "L,");
  stralloc_cats(&out, struid);
  stralloc_cats(&out, ",");
  iso2txt(tb, 16, &out);
  stralloc_cats(&out, "\n");
  
  stralloc_cats(&fifo, "s");
  stralloc_cats(&fifo, struid);
  stralloc_cats(&fifo, ",");
  stralloc_cats(&fifo, strip4);
  stralloc_cats(&fifo, ",");
  stralloc_cats(&fifo, strip6);
  stralloc_cats(&fifo, ",");
  loc_ntoa(&loc, &fifo);
  stralloc_cats(&fifo, "\n");

  stralloc_catb(&out, fifo.s, fifo.len);

  if(timeoutwrite(60, fd, out.s, out.len) != out.len)
    ddnsd_send_err_sys(p->uid, "couldn't write to disk");
  
  close(fd);
  
  /* test if the name we are asked to move to is already there */
  if (stat(finname.s, &st) == -1) 
    {
      if(errno != error_noent)
	ddnsd_send_err_sys(p->uid, finname.s);
	/* else: everything is fine, the File doesn't exist */
    }
  else
    // XXX: this error can happpen with other conditions too - fixme
    ddnsd_send_err(p->uid, DDNS_T_EALLREADYUSED, "allready registered");

  /* move tmp file to final file */

  /* do we have a race condition here? */
  /* Yes, but the only harm this race can cause ist that a user 
     gets DDNS_T_ESERVINT instead of DDNS_T_EALLREADYUSED 
  */
  
  if(rename(tmpname.s, finname.s) != 0)
    {
      /* try to delete tmpfile */
      unlink(tmpname.s);

      /* return error */
      if(!stralloc_copys(&err, "can't rename ")) die_nomem();
      if(!stralloc_cats(&err, tmpname.s)) die_nomem();
      if(!stralloc_cats(&err, " to ")) die_nomem();
      if(!stralloc_cat(&err, &finname)) die_nomem();
      ddnsd_send_err_sys(p->uid, err.s);
    }
  
  /* log this transaction */
  ddnsd_log(p->uid, "setting entry ");
  buffer_puts(buffer_2, "to ");
  buffer_puts(buffer_2, strip4);
  buffer_puts(buffer_2, "/");
  buffer_puts(buffer_2, strip6);
  buffer_puts(buffer_2, " ttl ");
  buffer_put(buffer_2, strnum, fmt_ulong(strnum, *ttl));
  buffer_putsflush(buffer_2, "\n");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  /* there should be some more intelligence in setting leasetime */
  r.leasetime = *ttl;
  ddnsd_send(&r);  

  ddnsd_fifowrite(&fifo);
}
