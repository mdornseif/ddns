/* $Id: ddns-cleand.c,v 1.2 2000/05/02 08:03:14 drt Exp $
 *  --drt@ailis.de
 *
 * queue cleaning daemon for ddns
 * 
 * $Log: ddns-cleand.c,v $
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
#include "droproot.h"
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

#include "ddns.h"

static char rcsid[] = "$Id: ddns-cleand.c,v 1.2 2000/05/02 08:03:14 drt Exp $";

/* maximum number of fields in a line */
#define NUMFIELDS 10

static char datadir[] = "dnsdata";
static uint32 linenum = 0;
struct cdb c;

void nomem(void)
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

int dofile(char *file, time_t ctime)
{
  char bspace[1024];
  char cdbkey[4];
  char ch;
  char strnum[FMT_ULONG];
  int fd;
  int match = 1;
  int r, i, j, k;
  uint32 ttl = 17;
  uint32 uid = 0;
  buffer b;
  stralloc data = { 0 };
  stralloc line = { 0 };
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
      j = 0;
      for (i = 0;i < NUMFIELDS;++i) 
	{
	  if (j >= line.len) 
	    {
	      if (!stralloc_copys(&f[i],"")) 
		{
		  nomem();
		}
	    }
	  else 
	    {
	      k = byte_chr(line.s + j,line.len - j,':');
	      if (!stralloc_copyb(&f[i],line.s + j,k)) 
		{
		  nomem();
		}
	      j += k + 1;
	    }
	}

      if(line.s[0] == '=')
	{	 
	  scan_xlong(f[1].s, &uid); 

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
	  
	  /* delete empty files or files which are expired */
	  if((now() - ctime > ttl) || (match == 0 && linenum == 1))
	    {
	      if(unlink(file) != 0)
		{
		  strerr_warn3("error: can't unlink() ", file, " ", &strerr_sys);
		}
	      else
		{
		  strnum[fmt_ulong(strnum, uid)] = 0;
		  buffer_puts(buffer_2, "cleaned ");
		  buffer_puts(buffer_2, file);
		  buffer_puts(buffer_2, " (0x");
		  buffer_puts(buffer_2, strnum);
		  buffer_puts(buffer_2, ")\n");
		  buffer_flush(buffer_2);
		  /* the file is deleted so we don't have to read further from it */
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
	      dofile(name.s, st.st_ctime);
	    }
	  else
	    {
	      strerr_warn2(x->d_name, " no dir and no regular file ", &strerr_sys);
	    }
	}
    }
  closedir(dir);

  return 0;
}

int flagrunasap = 0; void sigalrm() { flagrunasap = 1; }

int main()
{
  int fd;

  /* chroot() to $ROOT and switch to $UID:$GID */
  droproot("ddns-cleand: ");

  sig_alarmcatch(sigalrm);
  
  for (;;)
    {
      /* we reopen data.cdb for every pass over the database */

      fd = open_read("data.cdb");
      if (fd == -1) 
	{
	  strerr_die1sys(111, "can't open data.cdb");
	}
      
      cdb_init(&c, fd);
      
      /* traverse the tree */
      dodir(datadir);
      
      cdb_free(&c);
      close(fd);
      
      sleep(256);
      if(flagrunasap == 1)
	{
	  flagrunasap = 0;
	  
	  buffer_puts(buffer_2, "recived SIGALRM starting to clean\n");
	  buffer_flush(buffer_2);
	  
	}
    }

  sig_alarmdefault();
}
