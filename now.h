/* $Id: now.h,v 1.1 2000/04/30 14:56:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: now.h,v $
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#ifndef NOW_H
#define NOW_H

#include "datetime.h"

extern datetime_sec now();

#endif
