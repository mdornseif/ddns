/* $Id: ddnsd_renewentry.c,v 1.2 2000/10/06 22:03:15 drt Exp $
 *  -- drt@ailis.de - http://rc23.cx/
 *
 * (K)allisti 2000 a.D. - all rights reversed
 *
 * $Log: ddnsd_renewentry.c,v $
 * Revision 1.2  2000/10/06 22:03:15  drt
 * Library reorganisation
 *
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include <sys/stat.h>  /* stat, umask */
#include <sys/types.h> /* utimbuf */
#include <utime.h>     /* utimbuf */

#include "buffer.h"
#include "error.h"
#include "fmt.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutwrite.h"

#include "ddns.h"
#include "ddnsd.h"

static char rcsid[] = "$Id";

/* handle a renewentry request by updating th ctime of the file */
void ddnsd_renewentry( struct ddnsrequest *p, uint32 *ttl, stralloc *username)
{
  char strnum[FMT_ULONG];
  struct stat st = {0};
  struct utimbuf ut;
  struct ddnsreply r;
  stralloc tmpname = {0};
  stralloc fifo = {0};

  /* create the filename in our datastructure */
  create_datafilename(&tmpname, username);

  if(stat(tmpname.s, &st) == -1)
    {
      if(errno == error_noent)
	  // File not found
	  ddnsd_send_err(p->uid, DDNS_T_ENOENTRYUSED, "entry can't be stat()ed");
      else
	ddnsd_send_err_sys(p->uid, tmpname.s);
    }
  
  /* update ctime */
  ut.actime = st.st_atime;
  ut.modtime = st.st_mtime;
  
  if(utime(tmpname.s, &ut) == -1)
    ddnsd_send_err_sys(p->uid, tmpname.s);

  ddnsd_log(p->uid, "renewing entry");
  buffer_puts(buffer_2, " ttl ");
  buffer_put(buffer_2, strnum, fmt_ulong(strnum, *ttl));
  buffer_putsflush(buffer_2, "\n");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime = *ttl;
  ddnsd_send(&r);  

  /* inform others via fifo */
  stralloc_catb(&fifo, "r", 1);
  stralloc_catb(&fifo, strnum, fmt_ulong(strnum, p->uid));
  stralloc_cats(&fifo, "\n");
  ddnsd_fifowrite(&fifo); 
}
