/* $Id: ddnsd_fifo.h,v 1.2 2000/11/21 19:28:22 drt Exp $
 *  -- drt@un.bewaff.net
 */

void ddnsd_fifowrite(char c, char *s, int len);
void ddnsd_fifoflush(void);
