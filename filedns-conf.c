/* $Id: filedns-conf.c,v 1.1 2000/07/29 21:12:50 drt Exp $
 *  --drt@ailis.de
 *
 * create directory structure for using filedns with svscan
 *
 * Based on Dan Bernsteins *-conf
 *
 * You might find more Info at http://rc23.cx/
 * 
 * I do not belive there is a thing like copyright.
 *
 * $Log: filedns-conf.c,v $
 * Revision 1.1  2000/07/29 21:12:50  drt
 * initial revision
 *
 */

#include <pwd.h>
#include <unistd.h>
#include "strerr.h"
#include "exit.h"
#include "auto_home.h"
#include "generic-conf.h"
#include "ddns.h"

static char rcsid[] = "$Id: filedns-conf.c,v 1.1 2000/07/29 21:12:50 drt Exp $";

#define FATAL "filedns-conf: fatal: "

void usage(void)
{
  strerr_die1x(100, "filedns-conf: usage: filedns-conf acct logacct /filedns /ddns/root/dot myip");
}

char *dir;
char *datadir;
char *user;
char *loguser;
struct passwd *pw;
char *myip;

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
  myip = argv[5];
  if (!myip) usage();

  pw = getpwnam(loguser);
  if (!pw)
    strerr_die3x(111, FATAL, "unknown account ", loguser);

  init(dir, FATAL);
  makelog(loguser, pw->pw_uid, pw->pw_gid);

  start("run");
  outs("#!/bin/sh\nexec 2>&1\n");
  outs("ROOT="); outs(datadir); outs("; export ROOT\n");
  outs("IP="); outs(myip); outs("; export IP\n");
  outs("exec envuidgid "); outs(user);
  outs(" \\\nsoftlimit -d250000");
  outs(" "); outs(auto_home); outs("/bin/filedns");
  outs("\n");
  finish();
  perm(0755);

  return 0;
}
