/* $Id: droprootordie.h,v 1.1 2000/07/17 21:45:24 drt Exp $
 *  --drt@ailis.de
 * 
 * based on Dan Bernsteins droproot()
 *
 * $Log: droprootordie.h,v $
 * Revision 1.1  2000/07/17 21:45:24  drt
 * ddnsd and ddns-cleand now refuse to run as root
 *
 */

#ifndef DROPROOT_H
#define DROPROOT_H

extern void droprootordie(char *);

#endif
