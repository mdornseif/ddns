/* $Id: ddns-snapd.c,v 1.3 2000/11/21 19:45:01 drt Exp $
 *  --drt@un.bewaff.net
 *
 * $Log: ddns-snapd.c,v $
 * Revision 1.3  2000/11/21 19:45:01  drt
 * Tried to bring this up-to-date.
 *
 * Revision 1.2  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include <unistd.h>    /* fork() */
#include <sys/wait.h>

#include "buffer.h"
#include "coe.h"
#include "env.h"
#include "error.h"
#include "fifo.h"
#include "fmt.h"
#include "getln.h"
#include "ip4.h"
#include "ip6.h"
#include "ndelay.h"
#include "open.h"
#include "prot.h"
#include "scan.h"
#include "sig.h" 
#include "strerr.h"
#include "timeoutread.h"

#include "ddns.h"
#include "dAVLTree.h"
#include "traversedirhier.h"

static char rcsid[]="$Id: ddns-snapd.c,v 1.3 2000/11/21 19:45:01 drt Exp $";

#define ARGV0 "ddnsd-snapd: "
#define FATAL "ddnsd-snapd: fatal: "
#define FIFONAME "tracedir/ddns-snapd"

extern void dumpcheck(int flagdumpasap, int flagchanged, int flagchildrunning, int flagsighup);

static char waitreadspace[1024];
static buffer wr;
dAVLTree *t;
static stralloc line = {0};

static char rbspace[1024];
static buffer rb;

static int flagdumpasap = 0; 
static int flagchanged = 0;
static int flagchildrunning = 0;
static int flagsighup = 0;

/* how long to wait until dumping the data */
/* XXX this should be configurable */
static int dumpfreq = 1000;

void die_nomem(void)
{
  strerr_die2sys(111, FATAL, "no memory");
}

void sigalrm() 
{
  flagdumpasap = 1; 
}

void sighup() 
{
  flagsighup++;
  flagdumpasap = 1;
  flagchanged++;
}

void sigchld() 
{
  wait(NULL);
  flagchildrunning = 0;
  /* try duming again in 23 seconds */
  alarm(dumpfreq);
}

// XXX should be unified with ddns-ipfwo ...
static void die_read(void)
{
  strerr_die4sys(111,FATAL,"unable to read ", FIFONAME, ": ");
}

static int waitread(int fd, char *buf, unsigned int len)
{
  int r;
  r = timeoutread(60, fd, buf, len);
  if (r <= 0) 
    if(errno != error_timeout)
      /* XXX: should we ignore (some) errors ? */
      die_read();
  
  return r;
}

void handle_line(stralloc *sa, char action)
{
  uint32 uid   =  0;
  char ip4[4]  = {0};
  char ip6[16] = {0};
  char loc[16] = {0};
  
  /* clean up line end XXX why? */
  stralloc_cleanlineend(sa);
  
  switch(action)
    {
    case 's':
      ddns_parseline(sa->s, &uid, ip4, ip6, loc);
      
      // XXX check for already existing?
      dAVLInsert(t, uid, ip4, ip6, loc);
      flagchanged++;
      break;

    case 'r':
      // ignore
      break;

    case 'e':
    case 'k':
      // XXX check for errors
      dAVLDelete(t, uid);
      flagchanged++;
    }
}

/* filedandler for fill_db() */
int readfileintodb(char *file, time_t ctime)
{
  uint32 uid   =  0;
  uint32 t;
  int fd;
  int match = 1;
  int linenum;
  
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
      if(line.s[0] == '#') continue;
      
      handle_line(&line, 's');
    }
  
  close(fd);
  
  return 0;
}


/* fill database form filesystem */
void fill_db()
{
  // XXX: Fixme
  buffer_putsflush(buffer_2, ARGV0 "reading directory database\n");
  traversedirhier("/var/service/ddnsd/root/dot", readfileintodb);
  flagchanged++;
  flagdumpasap++;
  buffer_putsflush(buffer_2, ARGV0 "done\n");
}


void doit()
{
  char ch;
  int match = 1;
  long linenum = 0;
  stralloc fifoline = { 0 };

  linenum = 0;

  /* try duming data in 23 seconds */
  alarm(dumpfreq);

  buffer_putsflush(buffer_2, ARGV0 "entering main loop\n");

  /* forever read from pipe line by line and handle it */
  while(1)
    {
      while(match) 
	{
	  if(flagdumpasap == 1) 
	    dumpcheck(flagdumpasap, flagchanged, flagchildrunning, flagsighup);

	  ++linenum;
	  if(buffer_get(&wr, &ch, 1) == 1)
	    {
	      if(getln(&wr, &fifoline, &match, '\n') == -1)
		continue;
	      buffer_put(buffer_2, &ch, 1);
	      buffer_putflush(buffer_2, fifoline.s, fifoline.len);
	      handle_line(&fifoline, ch);
	    }
	}
    }
}

int main(int argc, char **argv)
{
  int fdfifo, fdfifowrite;
  char *x;
  unsigned long id;

  VERSIONINFO;

  x = env_get("WORKDIR");
  if (!x)
    strerr_die2x(111, FATAL, "$WORKDIR not set");
  if (chdir(x) == -1)
    strerr_die4sys(111, FATAL, "unable to chdir to ", x, ": ");

  x = env_get("GID");
  if (!x)
    strerr_die2x(111, FATAL, "$GID not set");
  scan_ulong(x,&id);
  if (prot_gid((int) id) == -1)
    strerr_die2sys(111, FATAL, "unable to setgid: ");

  x = env_get("UID");
  if (!x)
    strerr_die2x(111, FATAL, "$UID not set");
  scan_ulong(x,&id);

  /* undocumented feature */
  if(id == 0)
    if(!env_get("IWANTTORUNASROOTANDKNOWWHATIDO"))
      strerr_die2x(111, FATAL, "unable to run under uid 0: please change $UID");

  if (prot_uid((int) id) == -1)
    strerr_die2sys(111, FATAL, "unable to setuid: ");

  buffer_putsflush(buffer_2, ARGV0 "starting\n");

  if(fifo_make(FIFONAME, 0620) == -1)
    strerr_warn4(ARGV0, "unable to create fifo ", FIFONAME, " ", &strerr_sys);

  fdfifo = open_read(FIFONAME);
  if(fdfifo == -1)
    strerr_die4sys(111, FATAL, "unable to open for read ", FIFONAME, " ");
  coe(fdfifo);
  ndelay_on(fdfifo); /* DJB says: shouldn't be necessary */

  /* we need this to keep the fifo from beeing closed */
  fdfifowrite = open_write(FIFONAME);
  if (fdfifowrite == -1)
    strerr_die4sys(111, FATAL, "unable to open for write ", FIFONAME, " ");
  coe(fdfifowrite);

  /* init a buffer for nonblocking reading */
  buffer_init(&wr, waitread, fdfifo, waitreadspace, sizeof waitreadspace);

  t = dAVLAllocTree();

  /* read snapshot of dnsdatatree */ 
  fill_db();

  /* SIGALRM can be used to check if dumping the database is needed */
  sig_catch(sig_alarm, sigalrm);

  /* SIGHUP can be used to force dumping the database */
  sig_catch(sig_hangup, sighup);  

  /* check if out child is done */
  sig_catch(sig_child, sigchld);  

  // XXX SIGINT, SIGTERM,

  /* do our normal workloop */
  doit();

  /* we shouldn't get here */
  return 1;
}

