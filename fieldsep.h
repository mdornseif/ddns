/* $Id: fieldsep.h,v 1.1 2000/07/29 21:24:05 drt Exp $
 *  --drt@ailis.de
 *
 * do what you want ... at least with this
 *
 * You might find more Information at http://rc23.cx/
 */

#include "stralloc.h"

/* split line into seperate fields */
int fieldsep(stralloc f[], int numfields, stralloc *line, char sepchar);
