/* $Id: droprootordie.c,v 1.3 2000/07/29 21:32:53 drt Exp $
 *  --drt@ailis.de
 * 
 * based on Dan Bernsteins droproot()
 * chdir() to ROOT
 * change gid to GID
 * change uid to UID if UID != 0
 *
 * I have no right to keep you from copying this.
 *
 * You might find more Information at http://rc23.cx/
 *
 * $Log: droprootordie.c,v $
 * Revision 1.3  2000/07/29 21:32:53  drt
 * added a backdoor
 *
 * Revision 1.2  2000/07/17 22:35:32  drt
 * ddnsd and ddns-cleand don't allow to run with UID0.
 * sources imported from dnscache directly into my tree.
 *
 * Revision 1.1  2000/07/17 21:45:24  drt
 * ddnsd and ddns-cleand now refuse to run as root
 *
 */


#include "env.h"
#include "scan.h"
#include "prot.h"
#include "strerr.h"

static char rcsid[] = "$Id: droprootordie.c,v 1.3 2000/07/29 21:32:53 drt Exp $";

void droprootordie(char *fatal)
{
  char *x;
  unsigned long id;

  x = env_get("ROOT");
  if (!x)
    strerr_die2x(111,fatal,"$ROOT not set");
  if (chdir(x) == -1)
    strerr_die4sys(111,fatal,"unable to chdir to ",x,": ");
  if (chroot(".") == -1)
    strerr_die4sys(111,fatal,"unable to chroot to ",x,": ");

  x = env_get("GID");
  if (!x)
    strerr_die2x(111,fatal,"$GID not set");
  scan_ulong(x,&id);
  if (prot_gid((int) id) == -1)
    strerr_die2sys(111,fatal,"unable to setgid: ");

  x = env_get("UID");
  if (!x)
    strerr_die2x(111,fatal,"$UID not set");
  scan_ulong(x,&id);

  if(id == 0)
    if(!env_get("IWANTTORUNASROOTANDKNOWWHATIDO"))
      strerr_die2x(111, fatal, "unable to run under uid 0: please change $UID");

  if (prot_uid((int) id) == -1)
    strerr_die2sys(111,fatal,"unable to setuid: ");
}
