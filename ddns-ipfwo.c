/* $Id: ddns-ipfwo.c,v 1.1 2000/11/21 21:29:48 drt Exp $
 *  -- drt@un.bewaff.net
 *
 * generic code for changing firewall rules on the fly
 *
 * $Log: ddns-ipfwo.c,v $
 * Revision 1.1  2000/11/21 21:29:48  drt
 * *** empty log message ***
 *
 * Revision 1.6  2000/10/06 22:15:20  drt
 * fefes djblib added
 *
 * Revision 1.5  2000/08/11 06:41:55  drt
 * Structure of firewallchains changed
 *
 * Revision 1.4  2000/08/10 07:39:03  drt
 * IPs in firewall got mixed up
 *
 * Revision 1.3  2000/08/09 15:43:30  drt
 * ipfwo.linux is now shomehow mature
 *
 * Revision 1.2  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "byte.h"
#include "buffer.h"
#include "coe.h"
#include "env.h"
#include "error.h"
#include "fifo.h"
#include "fmt.h"
#include "getln.h"
#include "ndelay.h"
#include "open.h"
#include "scan.h"
#include "str.h"
#include "strerr.h"
#include "timeoutread.h"
#include "uint32.h"

#include "ddns.h"
#include "traversedirhier.h"

static char rcsid[] = "$Id: ddns-ipfwo.c,v 1.1 2000/11/21 21:29:48 drt Exp $";

#define ARGV0 "ddns-ipfwo: "
#define FATAL "ddns-ipfwo: fatal: "
#define FIFONAME "ddns-ipfwo"

static char waitreadspace[1024];
static buffer wr;

static char rbspace[1024];
static buffer rb;

void ipfwo_init();
void addrule(char *ip4);
void delrule(char *ip4);

void die_nomem(void)
{
  strerr_die2sys(111, FATAL, "oh no - no memory: ");
}

static void die_read(void)
{
  strerr_die4sys(111,FATAL,"unable to read ", FIFONAME, ": ");
}

static int waitread(int fd, char *buf, unsigned int len)
{
  int r;
  r = timeoutread(1000, fd, buf, len);
  if (r <= 0) 
    if(errno != error_timeout)
      /* XXX: should we ignore (some) errors ? */
      die_read();
  
  return r;
}

void doit()
{
  uint32 uid   =  0;
  uint32 t;
  char ip4[4]  = {0};
  char ip6[16] = {0};
  char loc[16] = {0};
  char ch;
  int match = 1;
  long linenum = 0;
  stralloc fifoline = { 0 };
  linenum = 0;
  
  buffer_putsflush(buffer_2, ARGV0 "entering main loop\n");
  
  /* forever read from pipe line by line and handle it */
  while(1)
    {
      while(match) 
	{
	  ++linenum;
	  if(buffer_get(&wr, &ch, 1) == 1)
	    {
	      if(getln(&wr, &fifoline, &match, '\n') == -1)
		continue;
	      
	      buffer_put(buffer_2, &ch, 1);
	      buffer_putflush(buffer_2, fifoline.s, fifoline.len);

	      switch(ch)
		{
		case 's':
		case 'k':
		case 'e':
		  ddns_parseline(fifoline.s, &uid, ip4, ip6, loc);
		  if(ch == 's')
		    {
		      addrule(ip4);
		    }
		  else
		    {
		      delrule(ip4);
		    }
		}
	    }
	}
    }
}

/* filedandler for fill_db() */
int readfileintofirewall(char *file, time_t ctime)
{
  uint32 uid   =  0;
  int fd;
  int match = 1;
  int linenum;
  char ip4[4]  = {0};
  char ip6[16] = {0};
  char loc[16] = {0};
  stralloc line;

  fd = open_read(file);
  if(fd == -1) 
    {
      strerr_warn3("unable to open file: ", file, " ", &strerr_sys);
      return -1;
    }
  
  buffer_init(&rb, read, fd, rbspace, sizeof rbspace);
  
  /* The file might contain references to more than one user id,
     therefore we work through all lines */
  
  linenum = 0;
  while(match) 
    {
      ++linenum;
      if(getln(&rb, &line, &match, '\n') == -1)
	{
	  strerr_warn3("unable to read line: ", file, " ", &strerr_sys);
	  return -1;
	}
      
      /* skip empty lines and comments */
      if(!line.len) continue;

      if(line.s[0] == '=')
	{ 
	  ddns_parseline(line.s, &uid, ip4, ip6, loc);

	  addrule(ip4);
	}
    }
  
  close(fd);
  
  return 0;
}


/* fill database form filesystem */
void fill_db()
{
  // XXX: fixme
  buffer_putsflush(buffer_2, ARGV0 "reading directory database\n");
  traversedirhier("/var/service/ddnsd/root/dot", readfileintofirewall);
  buffer_putsflush(buffer_2, ARGV0 "done\n");
}


int main(int argc, char **argv)
{
  int fdfifo, fdfifowrite;
  uint32 gid, uid;
  char *x;

  VERSIONINFO;

  x = env_get("GID");
  if (!x)
    strerr_die2x(111, FATAL, "$GID not set");
  scan_ulong(x,&gid);
  if (prot_gid((int) gid) == -1)
    strerr_die2sys(111, FATAL, "unable to setgid: ");

  x = env_get("UID");
  if (!x)
    strerr_die2x(111, FATAL, "$UID not set");
  scan_ulong(x,&uid);

  x = env_get("WORKDIR");
  if (!x)
    strerr_die2x(111, FATAL, "$WORKDIR not set");
  if (chdir(x) == -1)
    strerr_die4sys(111, FATAL, "unable to chdir to ", x, ": ");

  if (chdir("tracedir") == -1)
    strerr_die4sys(111, FATAL, "unable to chdir to ", x, ": ");

  /* chroot()ing doesn't help much as long as running as root but at
   * least it puts up an extra line of defense between us and an
   * attacker
   *
   * On Linux we later drop capabilities which might make the whole
   * thing a little bit more secure.
   */
  
  if (chroot(".") == -1)
    strerr_die4sys(111, FATAL,"unable to chroot to ", x, ": ");
  
  buffer_putsflush(buffer_2, ARGV0 "starting\n");

  /* do all linux specific initialisations */
  ipfwo_init();

  if(fifo_make(FIFONAME, 0620) == -1)
    strerr_warn4(ARGV0, "warning: unable to create fifo ", FIFONAME, " ", &strerr_sys);

  fdfifo = open_read(FIFONAME);
  if(fdfifo == -1)
    strerr_die4sys(111, FATAL, "unable to open for read ", FIFONAME, " ");
  coe(fdfifo);
  ndelay_on(fdfifo); /* DJB says: shouldn't be necessary */

  /* we have to chmod since wo don't know what umask did to our mkfifo */
 if(chmod(FIFONAME, 0620) == -1)
    strerr_die4sys(111, FATAL, "unable to chmod() fifo ", FIFONAME, " "); 
 if(chown(FIFONAME, uid, gid) == -1)
    strerr_die4sys(111, FATAL, "unable to chown() fifo ", FIFONAME, " "); 

  /* we need this to keep the fifo from beeing closed */
  fdfifowrite = open_write(FIFONAME);
  if (fdfifowrite == -1)
    strerr_die4sys(111, FATAL, "unable to open for write ", FIFONAME, " ");
  coe(fdfifowrite);

  /* init a buffer for nonblocking reading */
  buffer_init(&wr, waitread, fdfifo, waitreadspace, sizeof waitreadspace);

  /* here we slurp in the whole database to init our chains */
  fill_db();
  
  doit();
  
  /* we never should come here */
  return 1;
}
