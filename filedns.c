/* $Id: filedns.c,v 1.2 2000/07/07 23:51:38 drt Exp $
 *
 * dnsserver serving from the filesystem
 * 
 * $Log: filedns.c,v $
 * Revision 1.2  2000/07/07 23:51:38  drt
 * nearly everything works now, ddns is in a somehow usable state now.
 *
 * Revision 1.1  2000/04/27 09:35:33  drt
 * Initial Revision
 *
 */

/* XXX: do we need to handle SOA here ? */
/* XXX: we should put more authority (records) in here */
/* XXX: errorhandling is bad, distinguish not found and internal server error */

#include "open.h"
#include "buffer.h"
#include "byte.h"
#include "dd.h"
#include "dns.h"
#include "getln.h"
#include "ip4.h"
#include "ip6.h"
#include "open.h"
#include "readwrite.h"
#include "response.h"
#include "stralloc.h"
#include "strerr.h"

static char rcsid[] = "$Id: filedns.c,v 1.2 2000/07/07 23:51:38 drt Exp $";

/* This is missing in DJBs headers, fefe should add it to libdjb (and strerr_warnXsys) */
#define DNS_T_LOC "\0\35"

#define FATAL "fatal: "
/* XXX: cleanme */
char *fatal = "filedns: fatal: ";

void initialize(void)
{
  /* we don't need initialisation */
  ;
}

/* convert a query to a filename mainly by adding slashes instead of 
   dots (this is an abstraction) and removing anything that is not
   a valid in DNS, which means A-Z and '-' */
int query2filename(char *q, stralloc *s)
{
  int i;
  char *p;
  stralloc tmp = { 0 };

  p = q;

  /* XXX: handling of uppercase/lowercase is missing */
  while(*p)
    {
      stralloc_copy(&tmp, s);
      stralloc_copyb(s, "/", 1);
      stralloc_catb(s, p+1, (int) *p);
      /* Clean up string by 'X'ing all characters not allowed in dnsnames.
         We need to do tis to keep strange meta characters from slipping 
	 into syscalls when opening files changing directorys */
      for(i = 1; i < s->len; i++)
	{
	  if(!(((s->s[i] >= 'a') && (s->s[i] <= 'z'))
	       || ((s->s[i] >= 'A') && (s->s[i] <= 'Z'))
	       || ((s->s[i] >= '0') && (s->s[i] <= '9'))
	       || s->s[i] == '-'))
	    {
	      s->s[i] = 'X';
	    }
	} 
      stralloc_cat(s, &tmp);
      p += ((int) *p) + 1;
    }

  stralloc_0(s);

  return s->len;
}

/* this is out work routine which is called by DJBs server code */
int respond(char *q, char qtype[2])
{
  int flaga;
  int flagaaaa;
  int flagloc;
  char ip[IP6_FMT];
  stralloc filename = { 0 };
  int fd;
  buffer b;
  char bspace[1024];
  static stralloc line;
  int match = 1;
  unsigned long linenum = 0;
  int data = 0;
  char ch;

  /* check what the client is requesting */
  flaga = byte_equal(qtype,2,DNS_T_A);
  flagloc = byte_equal(qtype,2,DNS_T_LOC);
  flagaaaa = byte_equal(qtype,2,DNS_T_AAAA);
  if (byte_equal(qtype,2,DNS_T_ANY)) flaga = flagloc = flagaaaa = 1;
  
  /* find out in which file we should look */
  query2filename(q, &filename);

  buffer_put(buffer_2, filename.s, filename.len);
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);

  fd = open_read(filename.s);
  if (fd == -1) 
    {
      strerr_warn1("unable to open file: ", &strerr_sys);
      match = 0;
    }
  
  buffer_init(&b, read, fd, bspace, sizeof bspace);

  /* Work through the file and handout the data.
     XXX: At the moment we just don't care what querytype the user 
     asked for assuming an any QUERY */
  while(match) 
    {
      ++linenum;
      if(getln(&b, &line, &match, '\n') == -1)
	{
	  strerr_warn1("unable to read line: ", &strerr_sys);
	  break;
	}
  
      /* clean up line end */
      while(line.len) 
	{
	  ch = line.s[line.len - 1];
	  if ((ch != ' ') && (ch != '\t') && (ch != '\n')) break;
	  --line.len;
	}
      if(!line.len) continue;
      
      /* skip comments */
      if(line.s[0] == '#') continue;
      
      /* IPv4 */
      if(line.s[0] == '=')
	{
	  ip4_scan(&line.s[2], ip);
	  if (flaga) 
	    {
	      data++;
	      /* put type and ttl (60s) */
	      if (!response_rstart(q, DNS_T_A, "\0\0\0\74")) return 0;
	      /* put ip */
	      if (!response_addbytes(ip, 4)) return 0;
	      /* record finished */
	      response_rfinish(RESPONSE_ANSWER);
	    }
	}

      /* IPv6 */
      if(line.s[0] == '6')
	{
	  ip6_scan(&line.s[2], ip);
	  if (flagaaaa) 
	    {
	      data++;
	      /* put type and ttl (60s) */
	      if (!response_rstart(q, DNS_T_AAAA, "\0\0\0\74")) return 0;
	      /* put ip */
	      if (!response_addbytes(ip, 16)) return 0;
	      /* record finished */
	      response_rfinish(RESPONSE_ANSWER);
	    }
	}
      /* XXX: LOC is missing */
    }

  /* Disclaimer ;-) */
  if (!response_rstart(q, DNS_T_TXT, "\0\0\0\74")) return 0;
  if (!response_addbytes("this is a response from an alpha quality dns-server", 51)) return 0;
  response_rfinish(RESPONSE_ADDITIONAL);
  if (!response_rstart(q, DNS_T_TXT, "\0\0\0\74")) return 0;
  if (!response_addbytes("filednes 0.00 - if problems arise contact drt@ailis.de", 54)) return 0;
  response_rfinish(RESPONSE_ADDITIONAL);
  
  //  if (flaga || flagptr) 
  //    {
  //      if (dd(q,"",ip) == 4) 
  //	{
  //	  if (flaga) 
  //	    {
  //	      if (!response_rstart(q,DNS_T_A,"\0\12\0\0")) return 0;
  //	      if (!response_addbytes(ip,4)) return 0;
  //	      response_rfinish(RESPONSE_ANSWER);
  //	    }
  //	  return 1;
  //	}
  //      j = dd(q,"\7in-addr\4arpa",ip);
  //      if (j >= 0) 
  //	{
 
  if(data > 0) return 1;

  /* nothing found */
  buffer_puts(buffer_2, "notfound\n");
  buffer_flush(buffer_2);

  // XXX: this is somehow broken
  /* response 0-1 is transaction id */
  /* set response flags */
  /* clear authority bit */
  response[2] &= ~4;
  /* clear last 4 bits */
  response[3] &= ~15;
  /* flag not found */
  response[3] |= 3;
  
  /* response[4..5]:   nr of questions
   * response[6..7]:   nr of answers rr
   * response[8..9]:   nr of authority rr
   * response[10..11]: nr of additional rr
   */

  return 1;
}
