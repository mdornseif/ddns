/* $Id: ddnsd-data.c,v 1.10 2000/11/21 19:37:49 drt Exp $
 *  --drt@un.bewaff.net
 *
 * There is no such thing like copyright.
 * 
 * You might find more Information at http://rc23.cx/
 *
 * $Log: ddnsd-data.c,v $
 * Revision 1.10  2000/11/21 19:37:49  drt
 * Emailaddress Changed
 *
 * Revision 1.9  2000/08/02 20:45:03  drt
 * out of memory bug removed
 *
 * Revision 1.8  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.7  2000/07/29 21:28:32  drt
 * renamed nomem() to die_nomem(), use fieldsep()
 *
 * Revision 1.6  2000/07/17 13:45:56  drt
 * Dokumentation upgrades
 *
 * Revision 1.5  2000/05/02 19:37:04  drt
 * Handling of error conditions
 * switched to 256 bit keys
 * code cleanups
 *
 * Revision 1.4  2000/05/01 11:47:18  drt
 * ttl/leasetime comes now from data.cdb
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
#include "ddns.h"

static char rcsid[] = "$Id: ddnsd-data.c,v 1.10 2000/11/21 19:37:49 drt Exp $";

#define DEFAULT_TTL 3600
#define NUMFIELDS 10

#define FATAL "ddnsd-data: fatal: "
#define ARGV0 "ddnsd-data: "

unsigned long linenum = 0;

void die_datatmp(void)
{
  strerr_die2sys(111, FATAL, "unable to create data.tmp: ");
}

void die_nomem(void)
{
  strerr_die2sys(111, FATAL, "help - no memory ");
}

void syntaxerror(char *why)
{
  char strnum[FMT_ULONG];
  strnum[fmt_ulong(strnum,linenum)] = 0;
  strerr_die4x(111,FATAL,"unable to parse data line ", strnum, why);
}

int main(int argc, char **argv)
{
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

  VERSIONINFO;

  umask(024);

  buffer_init(&b,read,0 ,bspace,sizeof bspace);

  fdcdb = open_trunc("data.tmp");
  if (fdcdb == -1) 
    die_datatmp();

  if (cdb_make_start(&cdb,fdcdb) == -1) 
    die_datatmp();
 
  while (match) 
    {
      ++linenum;
      
      /* read one line from stdin */
      if (getln(buffer_0, &line, &match, '\n') == -1)
	  strerr_die2sys(111, FATAL, "unable to read line: ");
      
      /* remove cruft from line */
      while (line.len) 
	{
	  ch = line.s[line.len - 1];
	  if ((ch != ' ') && (ch != '\t') && (ch != '\n')) break;
	  --line.len;
	}
      
      if (!line.len) 
	continue;               /* skip empty lines */
      
      if (line.s[0] == '#')  
	continue;       /* skip comments */
      
      /* split line into seperate fields */
      if(fieldsep(f, NUMFIELDS, &line, ',')) die_nomem();
      
      /* Format of datafile 
	 
	 uid,uname        ,key   ,ttl                
	 123,joecypherpunk,geheim,3600
	 
	 
	 Format of cdb:
	 
	 database key: 4 byte uint32 in network byte order -> uid
	 data:         32 byte rijndael passprase/key
	 4 byte uint32 in network byte order -> ttl
	 x byte username
	 
	      0123456789abcdf0123456789abcdef012345678 ...
	 uid  passphrase                     ttl uname ...
      */
      
      stralloc_copys(&key, "");
      stralloc_copys(&data, "");
      
      scan_ulong(f[0].s, &uid); 
      
      /* parse/pad passphrase */
      if(f[2].len > 0)
	{
	  txtparse(&f[2]);
	  pad(&f[2], 32);
	}
      else
	syntaxerror("no shared secret");
      
      if(f[3].len > 0)
	scan_ulong(f[3].s, &ttl);
      else
	ttl = DEFAULT_TTL;
      
      if(uid == 0)
	syntaxerror("uid 0 not allowed");
      
      if(!stralloc_ready(&key, sizeof(uint32))) 
	die_nomem();
      
      uint32_pack(key.s, uid);
      key.len = 4;
      
      /* copy 32 bytes / 256 bits of key */
      if(!stralloc_copy(&data, &f[2])) 
	die_nomem();
      
      /* 4 bytes ttl */
      if(!stralloc_readyplus(&data, 4)) 
	die_nomem();

      uint32_pack(&data.s[data.len], ttl);
      data.len += 4;
      
      /* copy username */
      if(!stralloc_cat(&data, &f[1])) 
	die_nomem();

      if (cdb_make_add(&cdb, key.s, key.len, data.s, data.len) == -1)
	die_datatmp();
    }
  
  if (cdb_make_finish(&cdb) == -1) 
    die_datatmp();
  
  if (fsync(fdcdb) == -1) 
    die_datatmp();
  
  if (close(fdcdb) == -1) 
    die_datatmp(); /* NFS stupidity */

  if (rename("data.tmp", "data.cdb") == -1)
    strerr_die2sys(111, FATAL, "unable to move data.tmp to data.cdb: ");

  return 0;
}
