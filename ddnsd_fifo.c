/* $Id: ddnsd_fifo.c,v 1.1 2000/07/31 19:03:17 drt Exp $
 *  -- drt@ailis.de
 *
 * I don't know how this bits of yours got into this pattern
 *
 * $Log: ddnsd_fifo.c,v $
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include "stralloc.h"
#include "write_fifodir.h"

static char rcsid[] = "$Id: ddnsd_fifo.c,v 1.1 2000/07/31 19:03:17 drt Exp $";

static stralloc sa = {0};

void ddnsd_fifowrite(char c, char *s, int len)
{
  stralloc_catb(&sa, &c, 1);
  stralloc_catb(&sa, s, len);
}

void ddnsd_fifoflush(void)
{
  write_fifodir("tracedir", &sa, openandwrite);
  stralloc_copys(&sa, "");
}
