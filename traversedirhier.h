/* $Id: traversedirhier.h,v 1.1 2000/07/29 21:24:05 drt Exp $
 *  -- drt@ailis.de
 *
 * You might find more Information at http://rc23.cx/
 * 
 */

#include <unistd.h>    /* time_t */

extern int traversedirhier(char *dirname, int(*dofile)(char *file, time_t ctime));
