/* $Id: traversedirhier.c,v 1.2 2000/10/06 13:48:35 drt Exp $
 *  -- drt@ailis.de
 *
 * Traverse a directory hierachy
 * 
 * I can't own bits on your Harddisk
 *
 * You might find more Information at http://rc23.cx/
 * 
 * $Log: traversedirhier.c,v $
 * Revision 1.2  2000/10/06 13:48:35  drt
 * cleanups
 *
 * Revision 1.1  2000/07/29 21:24:05  drt
 * initial revision
 *
 */

#include <dirent.h>    /* opendir, readdir */
#include <sys/types.h> /* opendir, readdir */
#include <sys/stat.h>  /* stat */

#include "alloc.h"
#include "buffer.h"
#include "fmt.h"
#include "stralloc.h"
#include "strerr.h"

#include "ddns.h"

static char rcsid[] = "$Id: traversedirhier.c,v 1.2 2000/10/06 13:48:35 drt Exp $";

int traversedirhier(char *dirname, int(*dofile)(char *file, time_t ctime))
{
  stralloc name = {0};
  DIR *dir = NULL;
  char strnum[FMT_ULONG];
  struct dirent *x = NULL;
  int r = 0;
  static struct stat st;

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
	  if(name.a) 
	    stralloc_free(&name);
	  return r | -1;
	}

      /* Ignore everything starting with a . */
      if(x->d_name[0] != '.')
	{ 
	  stralloc_copys(&name, dirname);
	  stralloc_cats(&name, "/");
	  stralloc_cats(&name, x->d_name);
	  stralloc_0(&name);

	  if(lstat(name.s, &st) == -1)
	    {
	      strerr_warn2("can't stat ", name.s, &strerr_sys);
	    }

	  if(S_ISDIR(st.st_mode))
	    {
	      traversedirhier(name.s, dofile);
	    }
	  else if(S_ISREG(st.st_mode))
	    {
	      r  |= dofile(name.s, st.st_ctime);
	    }
	  else
	    {
	      buffer_puts(buffer_2, "ddns-cleand: warning: ");
	      buffer_puts(buffer_2, name.s);
	      buffer_puts(buffer_2, " no dir and no regular file (");
	      buffer_put(buffer_2, strnum, fmt_xlong(strnum, st.st_mode));
	      buffer_puts(buffer_2, ")\n");
	      buffer_flush(buffer_2);
	    }
	}
    }
  closedir(dir);

  stralloc_free(&name);
  return r;
}
