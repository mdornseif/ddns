/* $Id: filedns.c,v 1.1 2000/04/27 09:35:33 drt Exp $
 *
 * dnsserver serving from the filesystem
 * 
 * $Log: filedns.c,v $
 * Revision 1.1  2000/04/27 09:35:33  drt
 * Initial Revision
 *
 */

#include "byte.h"
#include "dns.h"
#include "dd.h"
#include "response.h"
#include "stralloc.h"
#include "buffer.h"
#include "readwrite.h"
#include "strerr.h"

static char rcsid[] = "$Id: filedns.c,v 1.1 2000/04/27 09:35:33 drt Exp $";

#define DNS_T_LOC "\0\35"

#define FATAL "fatal: "

char *fatal = "filedns: fatal: ";

void initialize(void)
{
  ;
}

int query2filename(char *q, stralloc *s)
{
  int i;
  char *p;
  stralloc tmp = { 0 };

  p = q;

  while(*p)
    {
      stralloc_copy(&tmp, s);
      stralloc_copyb(s, "/", 1);
      stralloc_catb(s, p+1, (int) *p);
      /* clean up string by Xing all characters not allowed in dnsnames */
      for(i = 1; i < s->len; i++)
	{
	  if(!((s->s[i] >= 'a') && (s->s[i] <= 'z') 
	       || (s->s[i] >= 'A') && (s->s[i] <= 'Z')
	       || (s->s[i] >= '0') && (s->s[i] <= '9')
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

int respond(char *q, char qtype[2])
{
  int flaga;
  int flagloc;
  char ip[] = {0, 0, 0, 0};
  int j;
  stralloc filename = { 0 };
  int fd;
  char key[4];
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
  if (byte_equal(qtype,2,DNS_T_ANY)) flaga = flagloc = 1;
  
  /* find out in wich file we should look */
  query2filename(q, &filename);

  fd = open_read(filename.s);
  if (fd == -1) 
    {
      strerr_die2sys(111, FATAL, "unable to open file: ");
    }

  buffer_init(&b, read, fd, bspace, sizeof bspace);

  while(match) 
    {
      ++linenum;
      if(getln(&b, &line, &match, '\n') == -1)
	{
	  strerr_die2sys(111, FATAL, "unable to read line: ");
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
      
      if(line.s[0] == '=')
	{
	  ip4_scan(&line.s[1], ip);
	  if (flaga) 
	    {
	      data++;
	      /* put type and ttl (60) */
	      if (!response_rstart(q,DNS_T_A,"\0\0\0\74")) return 0;
	      if (!response_addbytes(ip + 3,1)) return 0;
	      if (!response_addbytes(ip + 2,1)) return 0;
	      if (!response_addbytes(ip + 1,1)) return 0;
	      if (!response_addbytes(ip + 0,1)) return 0;
	      response_rfinish(RESPONSE_ANSWER);
	    }
	}
    }
  
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

  /* response 0-1 is transaction id */
  /* set response flags */
  /* clear authority bit */
  response[2] &= ~4;
  /* clear last 3 bits */
  response[3] &= ~15;
  /* flag refused */
  response[3] |= 5;
  
  /* response[4..5]:   nr of questions
   * response[6..7]:   nr of answers rr
   * response[8..9]:   nr of authority rr
   * response[10..11]: nr of additional rr
   */

  return 1;
}
