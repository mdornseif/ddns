/* $Id: now.h,v 1.2 2000/11/21 19:28:23 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@un.bewaff.net
 * $Log: now.h,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#ifndef NOW_H
#define NOW_H

#include "datetime.h"

extern datetime_sec now();

#endif
