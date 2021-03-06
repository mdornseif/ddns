/* $Id: ddns-cleand.c,v 1.11 2000/11/21 19:39:55 drt Exp $
 *  --drt@un.bewaff.net
 *
 * cleaning daemon for ddns
 * 
 * cleand cleans entrys in the dnstree which are expired,
 * in addition it cleans stale entrys in tmp 
 * it later might check is clients are up by pinging them
 *
 * (K)opyright is myth
 *
 * $Log: ddns-cleand.c,v $
 * Revision 1.11  2000/11/21 19:39:55  drt
 * Now *should* use the ddns-pipe mechanism.
 *
 * Revision 1.10  2000/11/17 02:00:41  drt
 * one shot mode for ddns-clientd
 *
 * Revision 1.9  2000/10/17 21:59:35  drt
 * *** empty log message ***
 *
 * Revision 1.8  2000/10/06 22:01:44  drt
 * Library reorganisation
 *
 * Revision 1.7  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.6  2000/07/31 19:15:56  drt
 * ddns-file(5) format changed
 * a lot of restructuring
 *
 * Revision 1.5  2000/07/29 21:50:37  drt
 * field seperation now is extern
 * line parsing still has to moved put
 *
 * Revision 1.4  2000/07/17 21:45:24  drt
 * ddnsd and ddns-cleand now refuse to run as root
 *
 * Revision 1.3  2000/07/12 11:44:42  drt
 * cleaning of tmp, usage on `,' instead of `:'
 * as a seperator to avoid IPv6 problems.
 *
 * Revision 1.2  2000/05/02 08:03:14  drt
 * dropping root, deleting empty files,
 * code cleanups
 *
 * Revision 1.1  2000/05/02 06:23:57  drt
 * ddns-cleand added
 *
 */

#include <dirent.h>    /* opendir, readdir */
#include <sys/stat.h>  /* stat */
#include <sys/types.h> /* opendir, readdir */
#include <unistd.h>    /* stat */

#include "buffer.h"
#include "byte.h"
#include "cdb.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "getln.h"
#include "now.h"
#include "open.h"
#include "readwrite.h"
#include "scan.h"
#include "sig.h"
#include "stralloc.h"
#include "strerr.h"

#include "droprootordie.h"
#include "traversedirhier.h"
#include "ddnsd_fifo.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddns-cleand.c,v 1.11 2000/11/21 19:39:55 drt Exp $";

#define ARGV0 "ddns-cleand: "

static uint32 linenum = 0;
static int interval = 256;
struct cdb c;
static stralloc tmpname = {0};
stralloc data = { 0 };
stralloc line = { 0 };

static void die_nomem(void)
{
  strerr_die1sys(111, "ddns-cleand fatal: no memory");
}
 
void syntaxerror(char *file, char *why)
{
  char strnum[FMT_ULONG];

  strnum[fmt_ulong(strnum,linenum)] = 0;
  buffer_puts(buffer_2, "error in ");
  buffer_puts(buffer_2, file);
  buffer_puts(buffer_2, ":");
  buffer_puts(buffer_2, strnum);
  buffer_puts(buffer_2, " ");
  buffer_puts(buffer_2, why);
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);
}

/* clean cruft from tmp */
/* cruft is defined as files older than 1.5 days */
/* tis is inspired by Maildir handling */
void tmpdir_clean()
{
 DIR *dir;
 struct dirent *d;
 struct stat st;
 datetime_sec time;

 time = now();

 dir = opendir("tmp");
 if (!dir) return;

 while (d = readdir(dir))
  {
   if (d->d_name[0] == '.') continue;
   if (!stralloc_copys(&tmpname,"tmp/")) break;
   if (!stralloc_cats(&tmpname,d->d_name)) break;
   if (!stralloc_0(&tmpname)) break;
   if (stat(tmpname.s,&st) == 0)
     if (time > st.st_atime + 129600)
       {
	 unlink(tmpname.s);
	 buffer_puts(buffer_2, "cleaned stale tmpfile ");
	 buffer_puts(buffer_2, tmpname.s);
	 buffer_puts(buffer_2, "\n");
	 buffer_flush(buffer_2);
       }
  }
 closedir(dir);
}


/* write Information about our processing to every fifo in tracedir/ 
 * 
 * format used:
 *
 * EXPIREENTRY: (used by ddns-cleand)
 *   e,UID\n
 *   e,123\n
 */

void cleand_fifowrite(stralloc *sa)
{
  write_fifodir("/tracedir", sa, openandwrite);
}


int dofile(char *file, time_t ctime)
{
  char bspace[1024];
  char cdbkey[4];
  char ch;
  char strnum[FMT_ULONG];
  int fd;
  int match = 1;
  int r;
  uint32 ttl = 17;
  uint32 uid = 0;
  buffer b;
  stralloc f[NUMFIELDS] = {{0}};

  fd = open_read(file);
  if (fd == -1) 
    {
      strerr_warn3("unable to open file: ", file, " ", &strerr_sys);
      return -1;
    }
  
  buffer_init(&b, read, fd, bspace, sizeof bspace);

  /* The file might contain references to more than one user id,
     therefore we work through all lines */

  linenum = 0;
  while(match) 
    {
      ++linenum;
      if(getln(&b, &line, &match, '\n') == -1)
	{
	  strerr_warn3("unable to read line: ", file, " ", &strerr_sys);
	  return -1;
	}
      
      /* clean up line end */
      while(line.len) 
	{
	  ch = line.s[line.len - 1];
	  if ((ch != ' ') && (ch != '\t') && (ch != '\n')) break;
		  --line.len;
	}
      
      /* skip empty lines */
      if(!line.len) 
	{
	  continue;
	}
	  
      /* skip comments */
      if(line.s[0] == '#')
	{
	  continue;
	}
      
      /* split line into seperate fields */
      if(!fieldsep(f, NUMFIELDS, &line, ',')) die_nomem();
	  
      if(line.s[0] == '=')
	{	 
	  scan_ulong(&f[2].s[0], &uid); 
	  
	  if(uid == 0)
	    {
	      syntaxerror(file, "can't parse uid");
	    }
	  else
	    {
	      /* get info from cdb */
	      uint32_pack(cdbkey, uid);
	      r = cdb_find(&c, cdbkey, 4); 
	      if (r == 1)
		{
		  /* read data */
		  stralloc_ready(&data, cdb_datalen(&c));
		  if (cdb_read(&c, data.s, cdb_datalen(&c), cdb_datapos(&c)) == -1)
		    {
		      strerr_warn1("error: can't read from data.cdb ", &strerr_sys);
		    }
		  else
		    {
		      data.len = cdb_datalen(&c);
		    }
		  
		  uint32_unpack(&data.s[16], &ttl);
		}
	      else
		{
		  /* can't find id */
		  syntaxerror(file, "can't find user in data.cdb");
		  uid = 0;
		}
	    }
	  
	  /* delete files which are expired and empty files */
	  if((now() - ctime > ttl) || (match == 0 && linenum == 1))
	    {
	      if(unlink(file) != 0)
		{
		  strerr_warn3("error: can't unlink() ", file, " ", &strerr_sys);
		}
	      else
		{
		  /* inform others via ddns-pipe */ 
		  line.s[0] = 'e';
		  stralloc_append(&line, "\n");
		  cleand_fifowrite(&line);
		  
		  /* do loging */
		  strnum[fmt_ulong(strnum, uid)] = 0;
		  buffer_puts(buffer_2, "cleaned ");
		  buffer_puts(buffer_2, file);
		  buffer_puts(buffer_2, " (0x");
		  buffer_puts(buffer_2, strnum);
		  buffer_puts(buffer_2, ")\n");
		  buffer_flush(buffer_2);
		  /* the file is deleted so we don't have to read further from it */
		  // XXX leaking a filediscriptor
		  return 0;
		}
	    }
	}
    }

  close(fd);

  return 0;
}

int dodir(char *dirname)
{
  stralloc name = {0};
  DIR *dir = NULL;
  char strnum[FMT_ULONG];
  struct dirent *x = NULL;
  struct stat st = {0};

  dir = opendir(dirname);
  if(dir == NULL)
    {
      strerr_warn3("can't opendir() ", dirname, " ", &strerr_sys);
      return -1;
    }

  while (x = readdir(dir))
    {
      if(x == NULL)
	{
	  strerr_warn3("can't readdir() ", dirname, " ", &strerr_sys);
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
	  
	  if(S_ISDIR(st.st_mode))
	    {
	      dodir(name.s);
	    }
	  else if(S_ISREG(st.st_mode))
	    {
	      /* this is a file!
		 if it's got the sticky bit ignore it, else check it out */
	      if(S_ISVTX & st.st_mode == 0)
		{
		  dofile(name.s, st.st_ctime);
		}
	    }
	  else
	    {
	      buffer_puts(buffer_2, "ddns-cleand: warning: ");
	      buffer_puts(buffer_2, name.s);
	      buffer_puts(buffer_2, " no dir and no regular file (");
	      buffer_put(buffer_2, strnum, fmt_ulong(strnum, st.st_mode));
	      buffer_puts(buffer_2, ")\n");
	      buffer_flush(buffer_2);
	    }
	}
    }
  closedir(dir);
  
  return 0;
}

int flagrunasap = 0; void sigalrm() { flagrunasap = 1; }

int main(int argc, char *argv[])
{
  int fd;
  char *x;
  
  VERSIONINFO;
  
  /* chroot() to $ROOT and switch to $UID:$GID */
  droprootordie("ddns-cleand: ");
  
  if (!argv[1]) 
    strerr_die1x(100, "fatal: usage: ddns-cleand dnsdir");
  
  buffer_puts(buffer_2, "ddns-cleand: starting\n");
  buffer_flush(buffer_2);
  
  /* SIGALRM can be used to force cleaning now */
  sig_catch(sig_alarm, sigalrm);
  
  for (;;)
    {
      /* we reopen data.cdb for every pass over the database */
      
      fd = open_read("data.cdb");
      if (fd == -1) 
	strerr_die1sys(111, "can't open data.cdb ");
      
      cdb_init(&c, fd);
      
      /* traverse the tree */
      dodir(argv[1]);
      
      cdb_free(&c);
      close(fd);
      
      /* clean up tmp */
      x = env_get("DDNSDONTCLEANTMP"); /* undocumented feature */ 
      if (!x) 
	tmpdir_clean();
      
      sleep(interval);
	  
      /* check for SIGALRM */
      if(flagrunasap == 1)
	{
	  flagrunasap = 0;
	  
	  buffer_puts(buffer_2, "recived SIGALRM starting to clean\n");
	  buffer_flush(buffer_2);
	}
    }
}
