/* $Id: qlog.h,v 1.1 2000/07/17 22:35:32 drt Exp $
 *
 * This is taken from Dan Bernsteins dnscache 1.00
 *  --drt@ailis.de
 *
 * $Log: qlog.h,v $
 * Revision 1.1  2000/07/17 22:35:32  drt
 * ddnsd and ddns-cleand don't allow to run with UID0.
 * sources imported from dnscache directly into my tree.
 *
 */

#ifndef QLOG_H
#define QLOG_H

#include "uint16.h"

extern void qlog(char *,uint16,char *,char *,char *,char *);

#endif
