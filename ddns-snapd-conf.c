/* $Id: ddns-snapd-conf.c,v 1.2 2000/11/21 19:37:02 drt Exp $
 *  --drt@un.bewaff.net
 *
 * create directory structure for using ddnsd-aclwriter with svscan
 *
 * You might find more Info at http://rc23.cx/
 * 
 * I do not belive there is a thing like copyright.
 *
 * $Log: ddns-snapd-conf.c,v $
 * Revision 1.2  2000/11/21 19:37:02  drt
 * Emailaddress Changed
 *
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include <pwd.h>
#include <unistd.h>
#include "strerr.h"
#include "exit.h"
#include "auto_home.h"
#include "generic-conf.h"

static char rcsid[] = "$Id: ddns-snapd-conf.c,v 1.2 2000/11/21 19:37:02 drt Exp $";

#define FATAL "ddns-snap-conf: fatal: "

void usage(void)
{
  strerr_die1x(100,"ddns-snap-conf: usage: ddns-snap-conf acct logacct /ddns-snapd /ddns/root/");
}

char *dir;
char *datadir;
char *user;
char *loguser;
struct passwd *pw;

int main(int argc, char **argv)
{
  user = argv[1];
  if (!user) usage();
  loguser = argv[2];
  if (!loguser) usage();
  dir = argv[3];
  if (!dir) usage();
  if (dir[0] != '/') usage();
  datadir = argv[4];
  if (!datadir) usage();
  if (datadir[0] != '/') usage();

  pw = getpwnam(loguser);
  if (!pw)
    strerr_die3x(111, FATAL, "unknown account ", loguser);

  init(dir, FATAL);
  makelog(loguser, pw->pw_uid, pw->pw_gid);

  pw = getpwnam(user);
  if (!pw)
    strerr_die3x(111, FATAL, "unknown account ", user);

  start("run");
  outs("#!/bin/sh\nexec 2>&1\n");
  outs("WORKDIR="); outs(datadir); outs("; export WORKDIR\n");
  outs("exec envuidgid "); outs(user);
  outs(" \\\nsoftlimit -d10000000");
  outs(" "); outs(auto_home); outs("/bin/ddns-snapd\n");
  finish();
  perm(0755);

  if(chdir(datadir) == -1)
    strerr_die4sys(111, FATAL, "can't chdir() to ", datadir, ": ");

  makedir("snapdir");
  perm(02730);
  owner(pw->pw_uid, pw->pw_gid);

  return 0;
}
