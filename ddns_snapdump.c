/* $Id: ddns_snapdump.c,v 1.2 2000/11/21 19:45:44 drt Exp $
 *  --drt@un.bewaff.net
 *
 * this fork(2)s a child. 
 *
 * $Log: ddns_snapdump.c,v $
 * Revision 1.2  2000/11/21 19:45:44  drt
 * Tried to bring this up-to-date.
 *
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 */

#include <dirent.h>    /* opendir, readdir */
#include <sys/types.h> /* opendir, readdir */
#include <sys/stat.h>  /* stat */

#include "stralloc.h"
#include "strerr.h"
#include "readwrite.h"
#include "buffer.h"
#include "fmt.h"
#include "sig.h"
#include "open.h"
#include "ip6.h"
#include "ip4.h"
#include "pathexec.h"

#include "ddns.h"
#include "loc.h"
#include "dAVLTree.h"

static char rcsid[] = "$Id: ddns_snapdump.c,v 1.2 2000/11/21 19:45:44 drt Exp $";

#define ARGV0 "ddnsd-snapd: "

char *dirname = "/dumpdir";

extern char **environ;
extern dAVLTree *t;

static char wbspace[1024];
static buffer wb;

/* masterplan:
   1. find every file in /dumpdir
   2. if it is executable spawn a child, execute the process
   3. feed all the data to the started process
*/


/* feedtochild */
void feedtochild(int fd)
{
  dAVLCursor c;
  dAVLNode *node;
  char strip[IP6_FMT];
  char strnum[FMT_ULONG];
  int fd;
  struct loc_s locs;
  stralloc sa = {0};
  stralloc dummy = {0};

  buffer_init(&wb, write, fd, wbspace, sizeof wbspace);

  /* hey ho, we exec everything in /dumpdir and feed the whole
   * datastructure via stdin this processes */
  
  /* traverse tree and feed it to fifo 
   * see ddns-pipe(5) for the format, we just use SETENTRY here 
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


/* execute progname and feed it all the date */
static void dodump(char *fn)
{
  int pi[2];
  char *args[2];
  
  /* create a pipe to feed our child */
  if (pipe(pi) == -1)
    {
      strerr_warn4(ARGV0, "warning: unable to create pipe for ", fn, ": ", &strerr_sys);
      return;
    }
  else
    {
      /* fork a child */
      switch(fork())
	{
	case -1:
	  strerr_warn4(ARGV0, "warning: unable to fork for ", fn, ": ", &strerr_sys);
	  return;
	case 0:
	  /* this is the child */
	  close(1);
	  if (fd_move(1, pi[0]) == -1)
	    strerr_die4sys(111, ARGV0, "fatal: unable to set up descriptors for ", fn, ": ");
	  
	  /* XXX what's about signal handling? */
	  args[0] = fn;
	  args[1] = 0;
	  pathexec_run(*args, args, environ);
	  /* we never should go here */
	  strerr_die4sys(111, ARGV0, "fatal: unable to start supervise ", fn, ": ");
	default:
	  /* this is the parent, feed it to the child .. */ 
	  feedtochild(pi[1]);
	  /* ... and wait it until it's ready */
	  wait(NULL);
	  close(pi[0]);
	  close(pi[1]);
	}
    }
}

void handlechilddir()
{
  DIR *dir = NULL;
  stralloc name = {0};
  struct dirent *x = NULL;
  static struct stat st;

  /* read directory */
  dir = opendir(dirname);
  if(dir == NULL)
    {
      strerr_warn3("can't opendir() ", dirname, ": ", &strerr_sys);
      return -1;
    }

  while (x = readdir(dir))
    {
      if(x == NULL)
	{
	  strerr_warn3("can't readdir() ", dirname, ": ", &strerr_sys);
	  if(name.a) 
	    stralloc_free(&name);
	  return -1;
	}

      /* Ignore everything starting with a . */
      if(x->d_name[0] != '.')
	{ 
	  stralloc_copys(&name, dirname);
	  stralloc_cats(&name, "/");
	  stralloc_cats(&name, x->d_name);
	  stralloc_0(&name);

	  if(stat(name.s, &st) == -1)
	    {
	      strerr_warn2("can't stat ", name.s, &strerr_sys);
	    }

	  if(S_ISREG(st.st_mode))
	    {
	      dodump(name.s);
	    }
	  else
	    {
	      buffer_puts(buffer_2, "ddnsd: warning: ");
	      buffer_puts(buffer_2, name.s);
	      buffer_puts(buffer_2, " is no regular file, ignoring\n");
	      buffer_flush(buffer_2);
	    }
	}
    }
  closedir(dir);  

  return 0;

}

/* check if we have to dump our data */
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

      /* fork of a child to do this
       * If we would't fork we wouldn't be able to read from our
       * fifo and the processes writing to it would block.
       *
       * On the other hand forking consumes a lot of resources.
       */
      switch(fork()) 
	{
	case 0:
	  /* this is the child */
	  /* XXX close fifos? */
	  sig_ignore(sig_alarm);
	  sig_ignore(sig_hangup);
	  handlechilddir();
	  _exit(0);
	case -1:
	  strerr_warn2(ARGV0, "unable to fork: ", &strerr_sys);
	  break;
	}
     
      /* this is the parent, ust go on */
      flagdumpasap = 0;
      buffer_putsflush(buffer_2, ARGV0 "parent\n");
    } 
}
