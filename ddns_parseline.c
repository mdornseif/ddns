/* $Id: ddns_parseline.c,v 1.1 2000/07/29 21:12:50 drt Exp $
 *  --drt@ailis.de
 *
 * parse ddnsd-file(5) lines. 
 *
 * Examples: 
 *    =,1.2.3.4,23
 *   6,1234:5678::90ab:cdef,23
 *   L,\\000SS\\023\\212\\356\\352D\\201{'\\254\\000\\230\\254\\304,23
 *
 *
 * Take this, I wouldn't miss a thing.
 * 
 * You might find more Information at http://rc23.cx/
 *
 * $Log: ddns_parseline.c,v $
 * Revision 1.1  2000/07/29 21:12:50  drt
 * initial revision
 *
 */

#include "alloc.h"
#include "buffer.h"
#include "byte.h"
#include "fieldsep.h"
#include "ip4.h"
#include "ip6.h"
#include "scan.h"
#include "stralloc.h"
#include "strerr.h"
#include "uint32.h"

#include "txtparse.h"
#include "ddns.h"

static char rcsid[] = "$Id";

#define NUMFIELDS 10

static void die_nomem(void)
{
  strerr_die1x(111, "fatal: no memory");
}

char ddns_parseline(char *s, uint32 *uid, char *ip4, char *ip6, char *loc)
{
  stralloc sa = {0};
  stralloc f[NUMFIELDS] = {{0}};

  stralloc_copys(&sa, s);
  /* split line into seperate fields */
  if(fieldsep(f, NUMFIELDS, &sa, ',') == -1) 
    die_nomem();
  
  stralloc_free(&sa);
 
  /* get data */
  switch(s[0])
    {
    case '=':
      ip4_scan(f[1].s, ip4);
      break;
    case '6':
      ip6_scan(f[1].s, ip6);
      break;
    case 'L':  
      txtparse(&f[1]);
      if(f[1].len < 16) 
	{
	  buffer_putsflush(buffer_2, "malformed L record\n");	
	  return 0;
	}
      byte_copy(loc, 16, f[1].s);
      break;
      /* XXX
	case default:
	buffer_putsflush(buffer_2, "unknown record\n");	
	return 0;
      */
    } 

  /* get uid */
  scan_ulong(f[2].s, uid); 

  return s[0];	
}
