/* $Id: ddns_pack.c,v 1.2 2000/11/21 19:28:22 drt Exp $
 *  -- drt@un.bewaff.net
 *
 * converts ddnsrequest and ddnsreply to machine 
 * independent bytestreams and back
 *
 * There is no such thing like interlectual property
 *
 * You might find more Information at http://rc23.cx/
 *
 * $Log: ddns_pack.c,v $
 * Revision 1.2  2000/11/21 19:28:22  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/29 21:12:50  drt
 * initial revision
 *
 */

#include "byte.h"
#include "stralloc.h"
#include "strerr.h"
#include "uint16.h"
#include "uint32.h"

#include "buffer.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddns_pack.c,v 1.2 2000/11/21 19:28:22 drt Exp $";

static void die_nomem()
{
  strerr_die1x(100, "$File$ not enough memory!\n");
}

int ddnsrequest_pack_big(stralloc *sa, struct ddnsrequest *r)
{
  char tmp[16];
  unsigned int oldlen;
  
  oldlen = sa->len;

  uint16_pack_big(tmp, r->type); if(!stralloc_catb(sa, tmp, 2)) die_nomem();
  uint16_pack_big(tmp, r->random1); if(!stralloc_catb(sa, tmp, 2)) die_nomem();
  uint32_pack_big(tmp, r->magic); if(!stralloc_catb(sa, tmp, 4)) die_nomem();
  if(!stralloc_catb(sa, r->ip4, 4)) die_nomem();
  if(!stralloc_catb(sa, r->ip6, 16)) die_nomem();
  if(!stralloc_catb(sa, &r->reserved1, 1)) die_nomem();
  if(!stralloc_catb(sa, &r->loc_size, 1)) die_nomem();
  if(!stralloc_catb(sa, &r->loc_hpre, 1)) die_nomem();
  if(!stralloc_catb(sa, &r->loc_vpre, 1)) die_nomem();
  uint32_pack_big(tmp, r->loc_lat); if(!stralloc_catb(sa, tmp, 4)) die_nomem();
  uint32_pack_big(tmp, r->loc_long); if(!stralloc_catb(sa, tmp, 4)) die_nomem();
  uint32_pack_big(tmp, r->loc_alt); if(!stralloc_catb(sa, tmp, 4)) die_nomem();
  uint16_pack_big(tmp, r->random2); if(!stralloc_catb(sa, tmp, 2)) die_nomem();
  uint16_pack_big(tmp, r->reserved2); if(!stralloc_catb(sa, tmp, 2)) die_nomem();
  taia_pack(tmp, &r->timestamp); if(!stralloc_catb(sa, tmp, 16)) die_nomem();
  
  return sa->len - oldlen;
}

int ddnsrequest_unpack_big(char *buf, struct ddnsrequest *r)
{
  char *oldstart;
  
  oldstart = buf;

  uint16_unpack_big(buf, &r->type); buf += 2;
  uint16_unpack_big(buf, &r->random1); buf += 2;
  uint32_unpack_big(buf, &r->magic); buf += 4;
  byte_copy(r->ip4, 4, buf); buf += 4;
  byte_copy(r->ip6, 16, buf); buf += 16;
  byte_copy(&r->reserved1, 1, buf); buf += 1;
  byte_copy(&r->loc_size, 1, buf); buf += 1;
  byte_copy(&r->loc_hpre, 1, buf); buf += 1;
  byte_copy(&r->loc_vpre, 1, buf); buf += 1;
  uint32_unpack_big(buf, &r->loc_lat); buf += 4;
  uint32_unpack_big(buf, &r->loc_long); buf += 4;
  uint32_unpack_big(buf, &r->loc_alt); buf += 4;
  uint16_unpack_big(buf, &r->random2); buf += 2;
  uint16_unpack_big(buf, &r->reserved2); buf += 2;
  taia_unpack(buf, &r->timestamp); buf += 16;
  
  return oldstart - buf;
}


int ddnsreply_pack_big(stralloc *sa, struct ddnsreply *r)
{
  char tmp[16];
  unsigned int oldlen;
  
  oldlen = sa->len;
  
  uint16_pack_big(tmp, r->type); if(!stralloc_catb(sa, tmp, 2)) die_nomem();
  uint16_pack_big(tmp, r->random1); if(!stralloc_catb(sa, tmp, 2)) die_nomem();
  uint32_pack_big(tmp, r->magic); if(!stralloc_catb(sa, tmp, 4)) die_nomem();
  uint32_pack_big(tmp, r->leasetime); if(!stralloc_catb(sa, tmp, 4)) die_nomem();
  if(!stralloc_catb(sa, r->reserved, 36)) die_nomem();
  taia_pack(tmp, &r->timestamp); if(!stralloc_catb(sa, tmp, 16)) die_nomem();
  
  return sa->len - oldlen;
}

int ddnsreply_unpack_big(char *buf, struct ddnsreply *r)
{
  char *oldstart;
  
  oldstart = buf;
  
  uint16_unpack_big(buf, &r->type); buf += 2;
  uint16_unpack_big(buf, &r->random1); buf += 2;
  uint32_unpack_big(buf, &r->magic); buf += 4;
  uint32_unpack_big(buf, &r->leasetime); buf += 4;
  byte_copy(&r->reserved, 36, buf); buf += 36;
  taia_unpack(buf, &r->timestamp); buf += 16;
  
  return oldstart - buf;
}

