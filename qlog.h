/* $Id: qlog.h,v 1.2 2000/11/21 19:28:23 drt Exp $
 *
 * This is taken from Dan Bernsteins dnscache 1.00
 *  --drt@un.bewaff.net
 *
 * $Log: qlog.h,v $
 * Revision 1.2  2000/11/21 19:28:23  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
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
