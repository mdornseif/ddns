/* $Id: dd.h,v 1.1 2000/07/17 22:35:32 drt Exp $
 *
 * This is taken from Dan Bernsteins dnscache 1.00
 *  --drt@ailis.de
 *
 * $Log: dd.h,v $
 * Revision 1.1  2000/07/17 22:35:32  drt
 * ddnsd and ddns-cleand don't allow to run with UID0.
 * sources imported from dnscache directly into my tree.
 *
 */

#ifndef DD_H
#define DD_H

extern int dd(char *,char *,char *);

#endif
