/* $Id: ddns_parseline.c,v 1.2 2000/07/31 19:15:56 drt Exp $
 *  --drt@ailis.de
 *
 * parse ddnsd-pipe(5) lines obtained by creating a fifo in tracedir/. 
 *
 * Take this, I wouldn't miss a thing.
 * 
 * You might find more Information at http://rc23.cx/
 *
 * $Log: ddns_parseline.c,v $
 * Revision 1.2  2000/07/31 19:15:56  drt
 * ddns-file(5) format changed
 * a lot of restructuring
 *
 * Revision 1.1  2000/07/29 21:12:50  drt
 * initial revision
 */

#include "alloc.h"
#include "buffer.h"
#include "byte.h"
#include "ip4.h"
#include "ip6.h"
#include "scan.h"
#include "stralloc.h"
#include "strerr.h"
#include "uint32.h"

#include "txtparse.h"
#include "ddns.h"
#include "loc.h"

static char rcsid[] = "$Id: ddns_parseline.c,v 1.2 2000/07/31 19:15:56 drt Exp $";
 
#define NUMFIELDS 10

void ddns_parseline(char *s, uint32 *uid, char *ip4, char *ip6, char *loc)
{
  stralloc sa = {0};
  stralloc f[NUMFIELDS] = {{0}};
  struct loc_s locs;

  stralloc_copys(&sa, s);
  /* split line into seperate fields */
  if(fieldsep(f, NUMFIELDS, &sa, ',') == -1) 
    die_nomem();
  
  stralloc_free(&sa);
 
  /* get data */
  scan_ulong(f[0].s, uid); 
  ip4_scan(f[1].s, ip4);
  ip6_scan(f[2].s, ip6);
  stralloc_0(&f[3]);
  loc_aton(f[3].s, &locs);
  loc_pack_big(loc, &locs);
}
