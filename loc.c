/* $Id: loc.c,v 1.1 2000/07/11 15:27:24 drt Exp $
 * 
 * handling of LOC data 
 * see:
 *
 * Davis, et al
 * RFC 1876           
 * Location Information in the DNS       
 * January 1996
 *
 * $Log: loc.c,v $
 * Revision 1.1  2000/07/11 15:27:24  drt
 * this is boken. gnarf
 *
 */

/* TODO: runden, werte mit fuehrender 0 (vpre) */

static char rcsid[] = "$Id: loc.c,v 1.1 2000/07/11 15:27:24 drt Exp $";

#include "uint32.h"
#include "stralloc.h"
#include "fmt.h"

struct dnsloc {
  unsigned char version;
  unsigned char size;
  unsigned char hpre;
  unsigned char vpre;
  uint32 latitude;
  uint32 longitude;
  uint32 altitude;
};

static char *cp; /* global char pointer modified everywhere arround */

/* convert string to int, stop at `stop'
   and disallow values > max, aton() light */
static int get_num(char stop, int max)
{
  int deg;

  for(deg = 0; *cp != stop; cp++)
    {
      if((*cp == '\0') 
	 || (*cp > '9') 
	 || (*cp < '0'))
	return -1;
      
      deg = deg * 10;
      deg += (int) (*cp - '0');
      
      if(deg >= max) return -1;
    }
  
  return deg;
}

/* convert string to int, stop at `stop', fill to 10**size length
   and disallow values > max, aton() light */
static int get_numl(char stop, int max, unsigned int size)
{
  int degparts;
  int i;
 
  for(degparts = 0, i = 0; *cp != ' '; cp++, i++)
    {
      if((*cp == '\0') 
	 || (*cp > '9') 
	 || (*cp < '0'))
	return -1;
      
      degparts = degparts * 10;
      degparts += (int) (*cp - '0');
      
      if(degparts > max) 
	return -1;
    }

  degparts *= 10*(size-i);
  
  return degparts;
}

/* convert an integer to the DNS LOC exponential format 

From RfC 1871:

             [...] expressed as a pair of four-bit unsigned
             integers, each ranging from zero to nine, with the most
             significant four bits representing the base and the second
             number representing the power of ten by which to multiply
             the base.  This allows sizes from 0e0 (<1cm) to 9e9
             (90,000km) to be expressed.  This representation was chosen
             such that the hexadecimal representation can be read by
             eye; 0x15 = 1e5.  Four-bit values greater than 9 are
             undefined, as are values with a base of zero and a non-zero
             exponent.
*/

/* XXX: we are allways rounding down */
unsigned int ntolocexp(unsigned int val)
{
  int i;
  int exp = 1000000000;

  for(i = 9; i >= 0; i--)
    {
      if((val / exp) > 0)
	break;
      exp = exp / 10;
    }

  return ((val/exp)<<4) | i;
}

/* skip guess what */
int skip_spaces()
{
  if(*cp == '\0')
    return 0;

  while(*cp == ' ')
    {
      if(*cp == '\0')
	return 0;
      cp++;
    }

  return 1;
}

/* convert an ascii representation to struct dnsloc
   the ascii string has to be:

   12.3456896 N 34.567 W [alt [size [hpre [vpre]]]]

   alt, size, hpre and vpre are optional and must be measured
   in centimeters. 

   returns 0 on success, -1 on error
*/ 

int atoloc(char *locstr, struct dnsloc *loc)
{
  int dir = 0;
  int i, t;

  /* set defaults */
  int tmp_hpre = 1000000;   // default 10km = 1000000cm
  int tmp_vpre = 1000000;   // default 10km = 1000000cm
  int tmp_size = 100;           // default 1m   = 100cm
  loc->latitude = 1 << 31;
  loc->longitude = 1 << 31;
  loc->altitude = 10000000; // default sealevel = 1000000cm

  cp = locstr;
  if(!skip_spaces()) 
    return -1;
  
  /* latitude */
  if(!skip_spaces()) 
    return -1;
  
  if((i = get_num('.', 90)) == -1)
    return -1;
  t = i * 60 * 60 * 1000;
  cp++;
  
  if((i = get_numl(' ', 9999999, 7)) == -1)
    return -1;
  t += i * 60 * 60 * 1000 / 1000000; 
 
  if(!skip_spaces()) 
    return -1;
 
  if(*cp++ == 'N') 
    loc->latitude += t;
  else 
    loc->latitude -= t;
  
  /* longitude */
  if(!skip_spaces()) 
    return -1;


  if((i = get_num('.', 180)) == -1)
    return -1;
  t = i * 60 * 60 * 1000;
  cp++;

  if((i = get_numl(' ', 9999999, 7)) == -1)
    return -1;
  t += i * 60 * 60 * 1000 / 1000000; 
 
  if(!skip_spaces()) 
    return -1;
  
  if(*cp++ == 'W') 
    loc->longitude -= t;
  else 
    loc->longitude += t;

  /* altitude */
  if(!skip_spaces()) 
    return 0;

  /* get sign */
  if(*cp == '-') 
    {
      dir = -1; 
      cp++;
      if(*cp == '\0') 
	return 0;
    }
  else 
    dir = 1;

  if((i = get_num(' ', 10000000)) == -1)
    return 0;
  loc->altitude += i * dir;
  
  
  /* size */
  if(!skip_spaces()) 
    return 0;
  if((tmp_size = get_num(' ', 2000000000)) == -1)
    return 0;
  loc->size = ntolocexp(tmp_size);
  
  
  /* hpre */
  if(!skip_spaces())
    return 0;
  if((tmp_hpre = get_num(' ', 2000000000)) == -1)
    return 0;
  loc->hpre = ntolocexp(tmp_hpre);
  
  
  /* vpre */
  if(!skip_spaces())
    return 0;
  if((tmp_vpre = get_num(' ', 2000000000)) == -1)
    return 0;
  loc->vpre = ntolocexp(tmp_vpre);
  
  return 0;
}

int loctoa(struct dnsloc *loc, stralloc *sa)
{
  int i, t;
  char c, dir;
  char str[FMT_ULONG];

  /* loc */
  if(loc->latitude > 1<<31)
    {
      loc->latitude -= 1<<31;
      dir = 'N';
    }
  else
    {
      loc->latitude = 1<<31 - loc->latitude;
      dir = 'S';
    }

  t = loc->latitude / (1000 * 60 * 60);
  stralloc_catb(sa,str, fmt_ulong(str, t));

  stralloc_append(sa, " ");
  stralloc_append(sa, &dir);
  stralloc_append(sa, " ");

  /* longitude */
  t = loc->longitude / (1000 * 60 * 60);
  stralloc_catb(sa,str, fmt_ulong(str, t));
  

  /* size */
  c = (char) ((loc->size & 0xf0) >> 4) + '0';
  stralloc_append(sa, &c);
  c = (char) (loc->size & 0x0f);
  for(i = 0; i < c ; i++)
    stralloc_append(sa, "0");
  
  stralloc_append(sa, " ");

  /* hpre */
  c = (char) ((loc->hpre & 0xf0) >> 4) + '0';
  stralloc_append(sa, &c);
  c = (char) (loc->hpre & 0x0f);
  for(i = 0; i < c ; i++)
    stralloc_append(sa, "0");

  stralloc_append(sa, " ");

  /* vpre */
  c = (char) ((loc->vpre & 0xf0) >> 4) + '0';
  stralloc_append(sa, &c);
  c = (char) (loc->vpre & 0x0f);
  for(i = 0; i < c ; i++)
    stralloc_append(sa, "0");
}

int main()
{
  struct dnsloc loc;
  stralloc s = {0};

  atoloc("42.123952 N 71.3456 W -2400 512 21334 1245", &loc);
  loctoa(&loc, &s);

  return 0;
}







