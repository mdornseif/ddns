/* $Id: loc_pack.c,v 1.1 2000/07/31 19:03:18 drt Exp $
 *  -- drt @ailis.de
 *
 * $Log: loc_pack.c,v $
 * Revision 1.1  2000/07/31 19:03:18  drt
 * initial revision
 *
 */

#include "strerr.h"
#include "uint32.h"

#include "loc.h"

static char rcsid[] = "$Id: loc_pack.c,v 1.1 2000/07/31 19:03:18 drt Exp $";

/* converts a struct loc_s to a 16 byte v0 RfC 1876 RR  */
void loc_pack_big(char *b, struct loc_s *loc)
{
  char tb[16];

  /* Version 0x */
  tb[0] = 0;
  tb[1] = loc->size;
  /* XXX: something is mixed up here, fixme, use loc_pack */
  tb[3] = loc->hpre;
  tb[2] = loc->vpre;
  uint32_pack_big(&tb[4], loc->latitude); 
  uint32_pack_big(&tb[8], loc->longitude); 
  uint32_pack_big(&tb[12], loc->altitude);  
}


/* converts a RfC 1876 RR to a struct loc_s */
void loc_unpack_big(char *tb, struct loc_s *loc)
{
  /* Version 0 */
  if(tb[0] != 0)
    // arrrgrrr
    strerr_die1x(111, "heyho! new DNS LOC version (v != 0)");

  loc->size = tb[1];
  /* XXX: something is mixed up here, fixme, use loc_pack */
  loc->hpre = tb[3];
  loc->vpre = tb[2];
  uint32_unpack_big(&tb[4], &loc->latitude); 
  uint32_unpack_big(&tb[8], &loc->longitude); 
  uint32_unpack_big(&tb[12], &loc->altitude);  
}
