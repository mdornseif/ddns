/* $Id: ddnsd.c,v 1.22 2000/11/21 19:46:08 drt Exp $
 *   --drt@un.bewaff.net
 *
 * server for ddns - this file is to long
 * 
 * You might find more info at http://rc23.cx/
 *
 * (K)allisti
 *
 * $Log: ddnsd.c,v $
 * Revision 1.22  2000/11/21 19:46:08  drt
 * Updated IP-Address
 *
 * Revision 1.21  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.20  2000/07/31 19:12:01  drt
 * seperated into muptiple source files
 *
 * Revision 1.19  2000/07/29 21:41:49  drt
 * creation of network packets is done in ddns_pack()
 * nomem() renamed to die_nomem()
 * first implementation of writing to a fifo
 * fiddeled with stat() which seems not to work on my
 * powerbook - the glibc/kernel interfaces for stat()
 * seem to be a real mess - glibc seems to be a real mess
 * thrown away dump_packet()
 *
 * Revision 1.18  2000/07/17 21:45:24  drt
 * ddnsd and ddns-cleand now refuse to run as root
 *
 * Revision 1.17  2000/07/14 15:32:51  drt
 * The timestamp is checked now in ddnsd and an error
 * is returned if there is more than 4000s fuzz.
 * This needs further checking.
 *
 * Revision 1.16  2000/07/14 14:49:37  drt
 * ddnsd ignires IP-adresses which are set to 0
 *
 * Revision 1.15  2000/07/13 18:20:47  drt
 * everything supports now DNS LOC
 *
 * Revision 1.14  2000/07/12 11:46:01  drt
 * checking of random1 == random2 to keep
 * a attacker from exchanging single blocks
 *
 * Revision 1.13  2000/07/07 13:32:47  drt
 * ddnsd and ddnsc now basically work as they should and are in
 * a usable state. The protocol is changed a little bit to lessen
 * problems with alignmed and so on.
 * Tested on IA32 and PPC.
 * Both are still missing support for LOC.
 *
 * Revision 1.12  2000/05/02 21:46:31  drt
 * usage of 256 bit keys,
 * huge code cleanup,
 * checking of most possible error conditions
 * like timeout reading, no memory and so on
 *
 * Revision 1.11  2000/05/01 11:47:18  drt
 * ttl/leasetime comes now from data.cdb
 *
 * Revision 1.10  2000/04/30 23:03:18  drt
 * Unknown user is now communicated by using uid == 0
 *
 * Revision 1.9  2000/04/30 15:59:26  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.8  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.7  2000/04/27 12:12:40  drt
 * Changed data packets to 32+512 Bits size, added functionality to
 * transport IPv6 adresses and LOC records.
 *
 * Revision 1.6  2000/04/27 09:41:20  drt
 * Tiny Bugfix
 *
 * Revision 1.5  2000/04/25 08:34:15  drt
 * Handling of tmp files fixed.
 *
 * Revision 1.4  2000/04/24 16:35:02  drt
 * First basically working version implementing full protocol
 * RENEWENTRY and KILLENTRY finalized
 * logging added
 *
 * Revision 1.3  2000/04/21 06:58:36  drt
 * *** empty log message ***
 *
 * Revision 1.2  2000/04/19 13:36:23  drt
 * Compile fixes
 *
 * Revision 1.1.1.1  2000/04/19 07:01:46  drt
 * initial ddns version
 *
 */

#include <stdlib.h>    /* random, clock */
#include <sys/stat.h>  /* umask */
#include <sys/types.h> /* umask */
#include <time.h>      /* time */
#include <unistd.h>    /* close, getpid, getppid, sleep, unlink */

#include "buffer.h"
#include "byte.h"
#include "droprootordie.h"
#include "env.h"
#include "fmt.h"
#include "ip4.h"
#include "ip6.h"
#include "now.h"
#include "open.h"
#include "readwrite.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

#include "mt19937.h"
#include "iso2txt.h"

#include "ddns.h"
#include "ddnsd.h"

static char rcsid[] = "$Id: ddnsd.c,v 1.22 2000/11/21 19:46:08 drt Exp $";

#define ARGV0 "ddnsd: "

static char *datadir;

void die_nomem(void)
{
  strerr_die1sys(111, "ddnsd: fatal: help - no memory ");
}

/* write Information about our processing to every fifo in tracedir/ 
 * 
 * format used:
 *
 * SETENTRY:
 *   s,UID,IP4,IP6,LOC\n
 *   s,123,1.2.3.4,1234:5678::90ab:cdef,50 57 9.7 N 6 54 08.3 E 5700 5000 1500 5000\n
 *
 * KILLENTRY:
 *   k,UID,IP4,IP6,LOC\n
 *   k,123,1.2.3.4,1234:5678::90ab:cdef,50 57 9.7 N 6 54 08.3 E 5700 5000 1500 5000\n
 *
 * RENEWENTRY:
 *   r,UID\n
 *   r,123\n
 *
 * EXPIREENTRY: (used by ddns-cleand)
 *   e,UID\n
 *   e,123\n
 */

void ddnsd_fifowrite(stralloc *sa)
{
  write_fifodir("tracedir", sa, openandwrite);
}

/* take an username and create a filename from it by 
   prepending datadir, return it in \0 terminated tmpname */
void create_datafilename(stralloc *tmpname, stralloc *username)
{
  /* create the filename in our datastructure */
  if(!stralloc_copys(tmpname, datadir)) die_nomem();
  if(!stralloc_cats(tmpname, "/")) die_nomem();
  if(!stralloc_cat(tmpname, username)) die_nomem();
  if(!stralloc_0(tmpname)) die_nomem();
}

void usage(void)
{
  ddnsd_send_err_sys(0, "ddnsd: usage: ddnsd /datadir");
}

int main(int argc, char **argv)
{
  struct ddnsrequest p = { 0 };
  stralloc username = {0};
  uint32 ttl;

  VERSIONINFO;

  /* chroot() to $ROOT and switch to $UID:$GID */
  droprootordie("ddnsd: ");

  // XXX should this be thigtened ?
  umask(024);

  /* seed some entropy into the MT */
  seedMT((long long) getpid () *
	 (long long) time(0) *
	 (long long) getppid() * 
	 (long long) random() * 
	 (long long) clock());

  datadir = argv[1];
  if (!datadir) usage();

  /* read one ddns packet from stdin which should be 
     connected to the tcpstream */
  ddnsd_recive(&p, &ttl, &username);
      
  switch(p.type)
    {
    case DDNS_T_SETENTRY:
      ddnsd_setentry(&p, &ttl, &username);
      break;
    case DDNS_T_RENEWENTRY:
      ddnsd_renewentry(&p, &ttl, &username);
      break;
    case DDNS_T_KILLENTRY:       
      ddnsd_killentry(&p, &ttl, &username);
      break;
    default:
      ddnsd_send_err(p.uid, DDNS_T_EPROTERROR, "unsupported type/command");
    }
  
  return 0;
}
