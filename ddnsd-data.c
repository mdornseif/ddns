#include <sys/types.h>
#include <sys/stat.h>
#include "djblib/uint16.h"
#include "djblib/uint32.h"
#include "djblib/str.h"
#include "djblib/byte.h"
#include "djblib/fmt.h"
#include "djblib/exit.h"
#include "djblib/readwrite.h"
#include "djblib/buffer.h"
#include "djblib/strerr.h"
#include "djblib/getln.h"
#include "djblib/cdb_make.h"
#include "djblib/stralloc.h"
#include "djblib/open.h"

#define TTL_NS 259200
#define TTL_POSITIVE 86400
#define TTL_NEGATIVE 2560

#define FATAL "ddnsd-data: fatal: "

void die_datatmp(void)
{
  strerr_die2sys(111,FATAL,"unable to create data.tmp: ");
}
void nomem(void)
{
  strerr_die1sys(111,FATAL);
}

void txtparse(stralloc *sa)
{
  char ch;
  unsigned int i;
  unsigned int j;

  j = 0;
  i = 0;
  while (i < sa->len) 
    {
      ch = sa->s[i++];
      if (ch == '\\') 
	{
	  if (i >= sa->len) 
	    {
	      break;
	    }
	  ch = sa->s[i++];
	  if ((ch >= '0') && (ch <= '7')) 
	    {
	      ch -= '0';
	      if ((i < sa->len) && (sa->s[i] >= '0') && (sa->s[i] <= '7')) 
		{
		  ch <<= 3;
		  ch += sa->s[i++] - '0';
		  if ((i < sa->len) && (sa->s[i] >= '0') && (sa->s[i] <= '7')) 
		    {
		      ch <<= 3;
		      ch += sa->s[i++] - '0';
		    }
		}
	    }
	}
      sa->s[j++] = ch;
    }
  sa->len = j;
}

int pad(stralloc *sa, int len)
{
  int l;

  while(sa->len < len)
    {
      l = len - sa->len;
      if( l > sa->len) l = sa->len; 
      stralloc_catb(sa, sa->s, l);
    }
  sa->len = len;
}

int fdcdb;
struct cdb_make cdb;
static stralloc key;
static stralloc result;

void rr_finish(char *owner)
{
  if (byte_equal(owner,2,"\1*")) {
    owner += 2;
    result.s[2] = '*';
  }
    //  if (!stralloc_copyb(&key,owner,dns_domain_length(owner))) nomem();
  //case_lowerb(key.s,key.len);
}

buffer b;
char bspace[1024];

static stralloc line;
int match = 1;
unsigned long linenum = 0;

#define NUMFIELDS 10
static stralloc f[NUMFIELDS];

static char *d1;
static char *d2;

char strnum[FMT_ULONG];

void syntaxerror(char *why)
{
  strnum[fmt_ulong(strnum,linenum)] = 0;
  strerr_die4x(111,FATAL,"unable to parse data line ",strnum,why);
}

main()
{
  int fddata;
  int i;
  int j;
  int k;
  char ch;
  unsigned long uid;
  char ttd[8];
  unsigned long u;
  char ip[4];
  char ip6[16];
  char type[2];
  char soa[20];
  char buf[4];
  static stralloc key;
  static stralloc data;

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
	  strerr_die2sys(111,FATAL,"unable to read line: ");
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
	 uid:uname        :key                      
	 123:joecypherpunk:geheim
      */
      
      stralloc_copys(&key, "");
      stralloc_copys(&data, "");

      /* XXX: error handling is missing */
      scan_ulong(f[0].s,&uid); 
      txtparse(&f[2]);
      pad(&f[2], 16);
      
      stralloc_ready(&key, sizeof(uint32));
      uint32_pack(key.s, uid);
      key.len = 4;
      
      stralloc_copy(&data, &f[2]);
      stralloc_cat(&data, &f[1]);

      if (cdb_make_add(&cdb, key.s, key.len, data.s, data.len) == -1)
	{
	  die_datatmp();
	}
    }
  
  if (cdb_make_finish(&cdb) == -1) die_datatmp();
  if (fsync(fdcdb) == -1) die_datatmp();
  if (close(fdcdb) == -1) die_datatmp(); /* NFS stupidity */
  if (rename("data.tmp","data.cdb") == -1)
    {
      strerr_die2sys(111,FATAL,"unable to move data.tmp to data.cdb: ");
    }

  return 0;
}
