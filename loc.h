/* $Id: loc.h,v 1.1 2000/07/11 19:54:59 drt Exp $
 *
 * header for DNS LOC handling routines
 *  -- drt@ailis.de
 *
 * (K)opyright is Myth
 * 
 * $Log: loc.h,v $
 * Revision 1.1  2000/07/11 19:54:59  drt
 * I have thrown away my own LOC parser and used the one from
 * RfC 1876. Changed it to use strallocs and a structure and
 * work with centimeters instead of meters. Not elegant but it
 * works (somehow) and is compatible to bind.
 *
 */

#include "stralloc.h"
#include "uint32.h"

struct loc_s {
  unsigned char version;
  unsigned char size;
  unsigned char hpre;
  unsigned char vpre;
  uint32 latitude;
  uint32 longitude;
  uint32 altitude;
};

/* converts a zone file representation in a string to an RDATA
 * on-the-wire representation. */
uint32 loc_aton(const char *ascii, struct loc_s *loc);

/* takes an on-the-wire LOC RR and prints it in zone file
 * (human readable) format. */
char *loc_ntoa(struct loc_s *loc, stralloc *sa);
