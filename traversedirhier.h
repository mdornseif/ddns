/* $Id: traversedirhier.h,v 1.2 2000/07/31 19:15:56 drt Exp $
 *  -- drt@ailis.de
 *
 * You might find more Information at http://rc23.cx/
 * 
 */

#include <unistd.h>    /* time_t */
#include <time.h>    /* time_t */

extern int traversedirhier(char *dirname, int(*dofile)(char *file, time_t ctime));
