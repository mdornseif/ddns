/* $Id: write_fifodir.c,v 1.1 2000/07/31 19:03:18 drt Exp $
 * -- drt@ailis.de
 *
 * write a stralloc so all fifos in a directory for ddnsd
 * using a 5 second timeout for each fifo
 *
 * Information wants to be free
 *
 * $Log: write_fifodir.c,v $
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include <dirent.h>    /* opendir, readdir */
#include <sys/types.h> /* opendir, readdir */
#include <sys/stat.h>  /* stat */

#include "buffer.h"
#include "open.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutwrite.h"

#include "ddns.h"

static char rcsid[] = "$Id: write_fifodir.c,v 1.1 2000/07/31 19:03:18 drt Exp $";

void openandwrite(char *filename, stralloc *sa)
{
  int fdfifo;
  
  /* we need this to keep the fifo from beeing closed */
  fdfifo = open_write(filename);
  if (fdfifo == -1)
    strerr_warn3("ddnsd: unable to open ", filename, "for writing: ", &strerr_sys);
  
  // XXX is 5 second timeout a good idea?
  if(timeoutwrite(5, fdfifo, sa->s, sa->len) != sa->len)
    strerr_warn3("can't write to fifo ", filename, ": ", &strerr_sys);
    
  close(fdfifo);
}

int write_fifodir(char *dirname, stralloc *sa, void (*oaw_func)(char *, stralloc *))
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

	  if(S_ISFIFO(st.st_mode))
	    {
	      oaw_func(name.s, sa);
	    }
	  else
	    {
	      buffer_puts(buffer_2, "ddnsd: warning: ");
	      buffer_puts(buffer_2, name.s);
	      buffer_puts(buffer_2, " is no fifo, ignoring\n");
	      buffer_flush(buffer_2);
	    }
	}
    }
  closedir(dir);  

  return 0;
}
