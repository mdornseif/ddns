/* $Id: loc.c,v 1.2 2000/07/11 19:54:58 drt Exp $
 *  -- drt@ailis.de
 * 
 * handling of LOC data 
 * see:
 *
 * Davis, et al: RfC 1876 - Location Information in the DNS, January 1996
 *
 * (K)opyright is Myth
 *
 * This needs a major overhaul.
 * stdio shouldn't be used, the code is not really clean and I would prefer
 * using decimal degrees instead of degrees, minutes and 
 * seconds - how baroqe!
 * 
 * $Log: loc.c,v $
 * Revision 1.2  2000/07/11 19:54:58  drt
 * I have thrown away my own LOC parser and used the one from
 * RfC 1876. Changed it to use strallocs and a structure and
 * work with centimeters instead of meters. Not elegant but it
 * works (somehow) and is compatible to bind.
 *
 * Revision 1.1  2000/07/11 15:27:24  drt
 * this is boken. gnarf
 *
 */

static char rcsid[] = "$Id: loc.c,v 1.2 2000/07/11 19:54:58 drt Exp $";

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>

#include "uint32.h"
#include "stralloc.h"
#include "fmt.h"

#include "loc.h"

/* from RfC 1876: Appendix A: Sample Conversion Routines
 *
 * routines to convert between on-the-wire RR format and zone file
 * format.  Does not contain conversion to/from decimal degrees;
 * divide or multiply by 60*60*1000 for that.
 */

static unsigned int poweroften[10] = {1, 10, 100, 1000, 10000, 100000,
                                 1000000,10000000,100000000,1000000000};

/* takes an XeY precision/size value, returns a integer.*/
static u_int32_t precsize_eton(u_int8_t prec)
{
  int mantissa, exponent;
  
  mantissa = (int)((prec >> 4) & 0x0f) % 10;
  exponent = (int)((prec >> 0) & 0x0f) % 10;
  
  return mantissa * poweroften[exponent];
}

/* converts ascii size to 0xXY. moves pointer.*/
static u_int8_t precsize_aton(char **strptr)
{
  unsigned int val = 0;
  u_int8_t retval = 0;
  register char *cp;
  register int exponent;
  register int mantissa;
  
  cp = *strptr;
  
  while (isdigit(*cp))
    val = val * 10 + (*cp++ - '0');
   
  for (exponent = 0; exponent < 9; exponent++)
    if (val < poweroften[exponent+1])
      break;
  
  mantissa = val / poweroften[exponent];
  if (mantissa > 9)
    mantissa = 9;
  
  retval = (mantissa << 4) | exponent;
  
  *strptr = cp;
  
  return (retval);
}

/* converts ascii lat/lon to unsigned encoded 32-bit number.
 *  moves pointer. */
static uint32 latlon2ul(char **latlonstrptr, int* which)
{
  register char *cp;
  int32_t retval;
  int deg = 0, min = 0, secs = 0, secsfrac = 0;
  
  cp = *latlonstrptr;
  
  while (isdigit(*cp))
    deg = deg * 10 + (*cp++ - '0');
  
  while (isspace(*cp))
    cp++;
  
  if (!(isdigit(*cp)))
    goto fndhemi;
  
  while (isdigit(*cp))
    min = min * 10 + (*cp++ - '0');
  
  while (isspace(*cp))
    cp++;
  
  if (!(isdigit(*cp)))
    goto fndhemi;
  
  while (isdigit(*cp))
    secs = secs * 10 + (*cp++ - '0');
  
  if (*cp == '.') {               /* decimal seconds */
    cp++;
    if (isdigit(*cp)) {
      secsfrac = (*cp++ - '0') * 100;
      if (isdigit(*cp)) {
	secsfrac += (*cp++ - '0') * 10;
	if (isdigit(*cp)) {
	  secsfrac += (*cp++ - '0');
	}
      }
    }
  }
  
  while (!isspace(*cp))   /* if any trailing garbage */
    cp++;
  
  while (isspace(*cp))
    cp++;
  
 fndhemi:
  switch (*cp) {
  case 'N': case 'n':
  case 'E': case 'e':
    retval = ((unsigned)1<<31)
      + (((((deg * 60) + min) * 60) + secs) * 1000)
      + secsfrac;
    break;
  case 'S': case 's':
  case 'W': case 'w':
    retval = ((unsigned)1<<31)
      - (((((deg * 60) + min) * 60) + secs) * 1000)
      - secsfrac;
    break;
  default:
    retval = 0;     /* invalid value -- indicates error */
    break;
  }
  
  switch (*cp) {
  case 'N': case 'n':
  case 'S': case 's':
    *which = 1;     /* latitude */
    break;
  case 'E': case 'e':
  case 'W': case 'w':
    *which = 2;     /* longitude */
    break;
  default:
    *which = 0;     /* error */
    break;
  }
  
  cp++;                   /* skip the hemisphere */
  
  while (!isspace(*cp))   /* if any trailing garbage */
    cp++;
  
  while (isspace(*cp))    /* move to next field */
    cp++;
  
  *latlonstrptr = cp;
  
  return (retval);
}

/* converts a zone file representation in a string to an RDATA
 * on-the-wire representation. */
uint32 loc_aton(const char *ascii, struct loc_s *loc)
{
  const char *cp, *maxcp;
  
  int32_t latit = 0, longit = 0, alt = 0;
  int32_t lltemp1 = 0, lltemp2 = 0;
  int altmeters = 0, altsign = 1;
  u_int8_t hp = 0x16;    /* default = 1e6 cm = 10000.00m = 10km */
  u_int8_t vp = 0x13;    /* default = 1e3 cm = 10.00m */
  u_int8_t siz = 0x12;   /* default = 1e2 cm = 1.00m */
  int which1 = 0, which2 = 0;
  
  cp = ascii;
  maxcp = cp + strlen(ascii);
  
  lltemp1 = latlon2ul(&cp, &which1);
  lltemp2 = latlon2ul(&cp, &which2);

  switch (which1 + which2) {
  case 3:                 /* 1 + 2, the only valid combination */
    if ((which1 == 1) && (which2 == 2)) { /* normal case */
      latit = lltemp1;
      longit = lltemp2;
    } else if ((which1 == 2) && (which2 == 1)) {/*reversed*/
      longit = lltemp1;
      latit = lltemp2;
    } else {        /* some kind of brokenness */
      return 0;
    }
    break;
  default:                /* we didn't get one of each */
    return 0;
  }
  
  /* altitude */
  if (*cp == '-') {
    altsign = -1;
    cp++;
  }
  
  if (*cp == '+')
    cp++;
  
  while (isdigit(*cp))
    altmeters = altmeters * 10 + (*cp++ - '0');
    
  alt = (10000000 + (altsign * altmeters));
  
  while (!isspace(*cp) && (cp < maxcp))
    /* if trailing garbage or m */
    cp++;
  
  while (isspace(*cp) && (cp < maxcp))
    cp++;
  
  if (cp >= maxcp)
    goto defaults;
  
  siz = precsize_aton(&cp);
  
  while (!isspace(*cp) && (cp < maxcp))/*if trailing garbage or m*/
    cp++;
  
  while (isspace(*cp) && (cp < maxcp))
    cp++;
  
  if (cp >= maxcp)
    goto defaults;
  
  hp = precsize_aton(&cp);
  
  while (!isspace(*cp) && (cp < maxcp))/*if trailing garbage or m*/
    cp++;
  
  while (isspace(*cp) && (cp < maxcp))
    cp++;
  
  if (cp >= maxcp)
    goto defaults;
  
  vp = precsize_aton(&cp);
  
 defaults:
  
  loc->size = siz;
  loc->hpre = hp;
  loc->vpre = vp;
  loc->latitude = latit;
  loc->longitude = longit;
  loc->altitude = alt;
  
  return (1);            /* size of RR in octets */
}

/* takes an on-the-wire LOC RR and prints it in zone file
 * (human readable) format. */
char *loc_ntoa(struct loc_s *loc, stralloc *sa)
{   
  int latdeg, latmin, latsec, latsecfrac;
  int longdeg, longmin, longsec, longsecfrac;
  char northsouth, eastwest;
  int altsign;
  
  const int referencealt = 10000000;
  
  int32_t latval, longval, altval;
      
  latval = (loc->latitude - ((unsigned)1<<31));
  longval = (loc->longitude - ((unsigned)1<<31));
  
  if (loc->altitude < referencealt) { /* below WGS 84 spheroid */
    altval = referencealt - loc->altitude;
    altsign = -1;
  } else {
    altval = loc->altitude - referencealt;
    altsign = 1;
  }
  
  if (latval < 0) {
    northsouth = 'S';
    latval = -latval;
  }
  else
    northsouth = 'N';
  
  latsecfrac = latval % 1000;
  latval = latval / 1000;
  latsec = latval % 60;
  latval = latval / 60;
  latmin = latval % 60;
  latval = latval / 60;
  latdeg = latval;
  
  if (longval < 0) {
    eastwest = 'W';
    longval = -longval;
  }
  else
    eastwest = 'E';
  
  longsecfrac = longval % 1000;
  longval = longval / 1000;
  longsec = longval % 60;
  longval = longval / 60;
  longmin = longval % 60;
  longval = longval / 60;
  longdeg = longval;
  
  altval = altval * altsign;

  stralloc_readyplus(sa, 123);
  snprintf(&sa->s[sa->len], 128,
	  "%d %.2d %.2d.%.3d %c %d %.2d %.2d.%.3d %c %d %d %d %d",
	  latdeg, latmin, latsec, latsecfrac, northsouth,
	  longdeg, longmin, longsec, longsecfrac, eastwest,
	  altval, precsize_eton(loc->size), 
	  precsize_eton(loc->hpre), precsize_eton(loc->vpre));
 
  
  return (sa->s);
}




