/* $Id: ddnsd_fifo.h,v 1.1 2000/07/31 19:15:56 drt Exp $
 *  -- drt@ailis.de
 */

void ddnsd_fifowrite(char c, char *s, int len);
void ddnsd_fifoflush(void);
