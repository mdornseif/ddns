/* $Id: qlog.c,v 1.2 2000/11/21 19:28:23 drt Exp $
 *
 * This is taken from Dan Bernsteins dnscache 1.00
 *  --drt@un.bewaff.net
 *
 * $Log: qlog.c,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/07/17 22:35:32  drt
 * ddnsd and ddns-cleand don't allow to run with UID0.
 * sources imported from dnscache directly into my tree.
 *
 */

#include "buffer.h"
#include "qlog.h"

static char rcsid[] = "$Id: qlog.c,v 1.2 2000/11/21 19:28:23 drt Exp $";

static void put(char c)
{
  buffer_put(buffer_2,&c,1);
}

static void hex(unsigned char c)
{
  put("0123456789abcdef"[(c >> 4) & 15]);
  put("0123456789abcdef"[c & 15]);
}

static void octal(unsigned char c)
{
  put('\\');
  put('0' + ((c >> 6) & 7));
  put('0' + ((c >> 3) & 7));
  put('0' + (c & 7));
}

void qlog(char ip[4],uint16 port,char id[2],char *q,char qtype[2],char *result)
{
  char ch;
  char ch2;

  hex(ip[0]);
  hex(ip[1]);
  hex(ip[2]);
  hex(ip[3]);
  put(':');
  hex(port >> 8);
  hex(port & 255);
  put(':');
  hex(id[0]);
  hex(id[1]);
  buffer_puts(buffer_2,result);
  hex(qtype[0]);
  hex(qtype[1]);
  put(' ');

  if (!*q)
    put('.');
  else
    for (;;) {
      ch = *q++;
      while (ch--) {
        ch2 = *q++;
        if ((ch2 >= 'A') && (ch2 <= 'Z'))
	  ch2 += 32;
        if (((ch2 >= 'a') && (ch2 <= 'z')) || ((ch2 >= '0') && (ch2 <= '9')) || (ch2 == '-') || (ch2 == '_'))
	  put(ch2);
        else
	  octal(ch2);
      }
      if (!*q) break;
      put('.');
    }

  put('\n');
  buffer_flush(buffer_2);
}
