/* $Id: ddnsd_killentry.c,v 1.3 2000/10/17 21:59:35 drt Exp $
 *  -- drt@ailis.de - http://rc23.cx/
 *
 * (K)allisti 2000 a.D. - all rights reversed
 *
 * $Log: ddnsd_killentry.c,v $
 * Revision 1.3  2000/10/17 21:59:35  drt
 * *** empty log message ***
 *
 * Revision 1.2  2000/08/09 15:43:30  drt
 * ipfwo.linux is now shomehow mature
 *
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include "buffer.h"
#include "error.h"
#include "fmt.h"
#include "getln.h"
#include "open.h"
#include "readwrite.h"
#include "stralloc.h"
#include "strerr.h"

#include "ddns.h"
#include "ddnsd.h"

static char rcsid[] = "$Id: ddnsd_killentry.c,v 1.3 2000/10/17 21:59:35 drt Exp $";


/* the user requested to delete his entry from the dns */
void ddnsd_killentry( struct ddnsrequest *p, uint32 *ttl, stralloc *username)
{
  struct ddnsreply r;
  stralloc tmpname = {0};
  stralloc line = {0};
  int match = 1;
  buffer b;
  char bspace[1024];
  int fd;

  /* create the filename in our datastructure */
  create_datafilename(&tmpname, username);
  
  /* we first have to read the file to inform the lurkers in tracedir/ */
  fd = open_read(tmpname.s);
  if (fd == -1) 
    {
      strerr_warn3("unable to open file ", tmpname.s, ": ", &strerr_sys);
      match = 0;
    }
  
  buffer_init(&b, read, fd, bspace, sizeof bspace);
  
  /* Work through the file */
  while(match) 
    {
      if(getln(&b, &line, &match, '\n') == -1)
		{
		  strerr_warn1("unable to read line: ", &strerr_sys);
		  break;
		}
      
      /* clean up line end */
      stralloc_cleanlineend(&line); 
      
      /* skip comments  & empty lines */
      if(line.s[0] == '#') continue;
      if(line.s[0] == 0) continue;
      if(line.s[0] == 's') break;
    }
  
  if(unlink(tmpname.s) == -1)
    {
      if(errno == error_noent)
		// File not found
		ddnsd_send_err(p->uid, DDNS_T_ENOENTRYUSED, "entry can't be killed");
      else
		ddnsd_send_err_sys(p->uid, tmpname.s);
    }
  ddnsd_log(p->uid, "killing entry\n");
  
  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime = 0;
  ddnsd_send(&r);  
  
  /* inform others via fifo */
  line.s[0] = 'k';
  stralloc_append(&line, "\n");
  ddnsd_fifowrite(&line); 
}

