/* $Id: response.h,v 1.1 2000/07/17 22:35:32 drt Exp $
 *
 * This is taken from Dan Bernsteins dnscache 1.00
 *  --drt@ailis.de
 *
 * $Log: response.h,v $
 * Revision 1.1  2000/07/17 22:35:32  drt
 * ddnsd and ddns-cleand don't allow to run with UID0.
 * sources imported from dnscache directly into my tree.
 *
 */

#ifndef RESPONSE_H
#define RESPONSE_H

extern char response[];
extern unsigned int response_len;

extern int response_query(char *,char *);
extern void response_nxdomain(void);
extern void response_servfail(void);
extern void response_id(char *);
extern void response_tc(void);

extern int response_addbytes(char *,unsigned int);
extern int response_addname(char *);
extern int response_rstart(char *,char *,char *);
extern void response_rfinish(int);

#define RESPONSE_ANSWER 6
#define RESPONSE_AUTHORITY 8
#define RESPONSE_ADDITIONAL 10

extern int response_cname(char *,char *);

#endif
