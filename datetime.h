/* $Id: datetime.h,v 1.1 2000/04/30 14:56:57 drt Exp $
 *
 * This is Software based on code of Dan Bernstein,
 * but since I messed with it please do not bother Dan
 * with questions regarding this stuff.
 *                               --- drt@ailis.de
 * $Log: datetime.h,v $
 * Revision 1.1  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 */

#ifndef DATETIME_H
#define DATETIME_H

struct datetime {
  int hour;
  int min;
  int sec;
  int wday;
  int mday;
  int yday;
  int mon;
  int year;
} ;

typedef long datetime_sec;

extern void datetime_tai();
extern datetime_sec datetime_untai();

#endif
