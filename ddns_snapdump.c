/* $Id: ddns_snapdump.c,v 1.1 2000/07/31 19:03:17 drt Exp $
 *  -- drt @ailis.de
 *
 * $Log: ddns_snapdump.c,v $
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include "stralloc.h"
#include "strerr.h"
#include "readwrite.h"
#include "buffer.h"
#include "fmt.h"
#include "sig.h"
#include "open.h"
#include "ip6.h"
#include "ip4.h"

#include "ddns.h"
#include "loc.h"
#include "dAVLTree.h"

static char rcsid[] = "$Id: ddns_snapdump.c,v 1.1 2000/07/31 19:03:17 drt Exp $";

#define ARGV0 "ddnsd-snapd: "
#define FATAL "ddnsd-snapd: fatal: "

extern dAVLTree *t;


static char wbspace[1024];
static buffer wb;

void snap_dump(char *filename, stralloc *dummy)
{
  dAVLCursor c;
  dAVLNode *node;
  char strip[IP6_FMT];
  char strnum[FMT_ULONG];
  int fd;
  struct loc_s locs;
  stralloc sa = {0};

  fd = open_trunc("filename");  
  if(fd == -1)
    strerr_warn1(ARGV0 "warning: unable to open for tcp.tmp for writing", &strerr_sys);
  
  buffer_init(&wb, write, fd, wbspace, sizeof wbspace);
  
  /* traverse tree and feed it to fifo 
   * see ddns-pipe(5) for the format, we yust use SETENTRY here 
   *
   * SETENTRY:
   *   s,UID,IP4,IP6,LOC\n
   *   s,123,1.2.3.4,1234:5678::90ab:cdef,50 57 9.7 N 6 54 08.3 E 5700 5000 1500 5000\n
   */

  node = dAVLFirst(&c, t);
  while(node)
    {
      buffer_puts(&wb, "s,");
      buffer_put(&wb, strnum, fmt_ulong(strnum, node->key));
      buffer_puts(&wb, ",");
      buffer_put(&wb, strip, ip4_fmt(strip, node->ip4));
      buffer_puts(&wb, ",");
      buffer_put(&wb, strip, ip6_fmt(strip, node->ip6));
      loc_unpack_big(node->loc, &locs);
      loc_ntoa(&locs, &sa);
      buffer_put(&wb, sa.s, sa.len);
      buffer_puts(&wb, "\n");
   
      node = dAVLNext(&c);
    }
 
  buffer_flush(&wb);
  stralloc_free(&sa);
  close(fd);
}


static void dodump()
{
  stralloc dummy = {0};

  buffer_putsflush(buffer_2, ARGV0 "dumping\n");

  write_fifodir("snapdir", &dummy, snap_dump);

  buffer_putsflush(buffer_2, ARGV0 "dumping ready\n");
}


void dumpcheck(int flagdumpasap, int flagchanged, int flagchildrunning, int flagsighup)
{
  char *z;

  z = "0";
  *z =+ flagchanged;
  buffer_puts(buffer_2, z);
  buffer_puts(buffer_2, " ");
  
  z = "0";
  *z =+ flagchildrunning;
  buffer_puts(buffer_2, z);
  buffer_putsflush(buffer_2, " checking if a dump is needed\n");

  if(flagsighup)
    {
      flagsighup = 0;
      buffer_putsflush(buffer_2, ARGV0 "SIGHUP recived, dumping withouth further asking\n");
    }
  if(flagchanged && !flagchildrunning)
    {
      flagchanged = 0;
      flagchildrunning++;

      buffer_putsflush(buffer_2, ARGV0 "yep, forking\n");

      /* fork of a child to do this */
      switch(fork()) 
	{
	case 0:
	  /* this is the child */
	  /* XXX close fifos? */
	  sig_ignore(sig_alarm);
	  sig_ignore(sig_hangup);
	  buffer_putsflush(buffer_2, ARGV0 "child started\n");
	  dodump();
	  buffer_putsflush(buffer_2, ARGV0 "child exiting\n");
	  _exit(0);
	case -1:
	  strerr_warn2(ARGV0, "unable to fork: ", &strerr_sys);
	  break;
	}
     
      /* this is the parent */
      flagdumpasap = 0;
      buffer_putsflush(buffer_2, ARGV0 "parent\n");
    } 
}
