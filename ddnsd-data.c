/* $Id: ddnsd-data.c,v 1.4 2000/05/01 11:47:18 drt Exp $
 *  --drt@ailis.de
 * 
 * $Log: ddnsd-data.c,v $
 * Revision 1.4  2000/05/01 11:47:18  drt
 * ttl/leasetime coms now from data.cdb
 *
 */

#include <sys/stat.h>  /* umask */
#include <unistd.h>    /* fsync, close */
#include <stdio.h>     /* rename */

#include "buffer.h"
#include "byte.h"
#include "cdb_make.h"
#include "exit.h"
#include "fmt.h"
#include "getln.h"
#include "open.h"
#include "readwrite.h"
#include "scan.h"
#include "str.h"
#include "stralloc.h"
#include "strerr.h"
#include "uint16.h"
#include "uint32.h"

#include "pad.h"
#include "txtparse.h"

static char rcsid[] = "$Id: ddnsd-data.c,v 1.4 2000/05/01 11:47:18 drt Exp $";

#define DEFAULT_TTL 3600
#define NUMFIELDS 10

#define FATAL "ddnsd-data: fatal: "

unsigned long linenum = 0;

void die_datatmp(void)
{
  strerr_die2sys(111, FATAL, "unable to create data.tmp: ");
}

void nomem(void)
{
  strerr_die1sys(111, FATAL);
}

void syntaxerror(char *why)
{
  char strnum[FMT_ULONG];
  strnum[fmt_ulong(strnum,linenum)] = 0;
  strerr_die4x(111,FATAL,"unable to parse data line ", strnum, why);
}

int main()
{
  int i, j, k;
  int fdcdb;
  int match = 1;
  uint32 uid = 0;
  uint32 ttl = 0;
  buffer b;
  char bspace[1024];
  char ch;
  static stralloc key;
  static stralloc data;
  static stralloc line;
  struct cdb_make cdb;
  static stralloc f[NUMFIELDS];

  umask(024);

  buffer_init(&b,read,0 ,bspace,sizeof bspace);

  fdcdb = open_trunc("data.tmp");
  if (fdcdb == -1)
    {
      die_datatmp();
    }
  
  if (cdb_make_start(&cdb,fdcdb) == -1) 
    {
      die_datatmp();
    }

  while (match) 
    {
      ++linenum;
      if (getln(buffer_0, &line, &match, '\n') == -1)
	{
	  strerr_die2sys(111, FATAL, "unable to read line: ");
	}
      
      while (line.len) 
	{
	  ch = line.s[line.len - 1];
	  if ((ch != ' ') && (ch != '\t') && (ch != '\n')) break;
	  --line.len;
	}
      if (!line.len) continue;

      /* skip comments */
      if (line.s[0] == '#') continue;
      
      /* split line into seperate fields */
      j = 0;
      for (i = 0;i < NUMFIELDS;++i) 
	{
	  if (j >= line.len) 
	    {
	      if (!stralloc_copys(&f[i],"")) nomem();
	    }
	  else 
	    {
	      k = byte_chr(line.s + j,line.len - j,':');
	      if (!stralloc_copyb(&f[i],line.s + j,k)) nomem();
	      j += k + 1;
	    }
	}
      
      /* example:
	 uid:uname        :key   :ttl                
	 123:joecypherpunk:geheim:3600
      */
      
      stralloc_copys(&key, "");
      stralloc_copys(&data, "");

      /* XXX: error handling is missing */
      scan_ulong(f[0].s, &uid); 
      
      if(f[2].len > 0)
	{
	  txtparse(&f[2]);
	  pad(&f[2], 16);
	}
      else
	{
	  syntaxerror("no shared secret");	  
	}

      if(f[3].len > 0)
	{
	  scan_ulong(f[3].s, &ttl); 
	}
      else
	{
	  ttl = DEFAULT_TTL;
	}

      if(uid == 0) 
	{
	  syntaxerror("uid 0 not allowed");
	}
      
      stralloc_ready(&key, sizeof(uint32));
      uint32_pack(key.s, uid);
      key.len = 4;
      
      stralloc_copy(&data, &f[2]);
      stralloc_readyplus(&data, 4);
      uint32_pack(&data.s[data.len], ttl);
      data.len += 4;
      stralloc_cat(&data, &f[1]);

      if (cdb_make_add(&cdb, key.s, key.len, data.s, data.len) == -1)
	{
	  die_datatmp();
	}
    }
  
  if (cdb_make_finish(&cdb) == -1) die_datatmp();
  if (fsync(fdcdb) == -1) die_datatmp();
  if (close(fdcdb) == -1) die_datatmp(); /* NFS stupidity */
  if (rename("data.tmp", "data.cdb") == -1)
    {
      strerr_die2sys(111, FATAL, "unable to move data.tmp to data.cdb: ");
    }
  
  return 0;
}
