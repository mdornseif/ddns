/* $Id: traversedirhier.h,v 1.3 2000/11/21 19:28:23 drt Exp $
 *  -- drt@un.bewaff.net
 *
 * You might find more Information at http://rc23.cx/
 * 
 */

#include <unistd.h>    /* time_t */
#include <time.h>    /* time_t */

extern int traversedirhier(char *dirname, int(*dofile)(char *file, time_t ctime));
