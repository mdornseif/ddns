/* $Id: loc.h,v 1.3 2000/07/29 21:35:27 drt Exp $
 *
 * header for DNS LOC handling routines
 *  -- drt@ailis.de
 *
 * (K)opyright is Myth
 * 
 * You might find more information at http://rc23.cx/
 *
 * $Log: loc.h,v $
 * Revision 1.3  2000/07/29 21:35:27  drt
 * removed snprintf()
 *
 * Revision 1.2  2000/07/12 11:34:25  drt
 * ddns-clientd handels now everything itself.
 * ddnsc is now linked to ddnsd-clientd, do a
 * enduser needs just this single executable
 * and no ucspi-tcp/tcpclient.
 *
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

/* converts a zone file representation in a string to an struct loc_s */
int loc_aton(char *ascii, struct loc_s *loc);

/* struct loc_s and emits it in a human readable format. */
char *loc_ntoa(struct loc_s *loc, stralloc *sa);
