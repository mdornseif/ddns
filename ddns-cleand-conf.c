/* $Id: ddns-cleand-conf.c,v 1.2 2000/07/31 19:15:56 drt Exp $
 *  --drt@ailis.de
 *
 * create directory structure for using ddns-cleand with svscan
 * based on dan Bernseins *-conf
 *
 * You might find more Info at http://rc23.cx/
 * 
 * I do not belive there is a thing like copyright.
 *
 * $Log: ddns-cleand-conf.c,v $
 * Revision 1.2  2000/07/31 19:15:56  drt
 * ddns-file(5) format changed
 * a lot of restructuring
 *
 * Revision 1.1  2000/07/29 21:12:50  drt
 * initial revision
 *
 */

#include <pwd.h>
#include <unistd.h>

#include "auto_home.h"
#include "exit.h"
#include "generic-conf.h"
#include "stralloc.h"
#include "strerr.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddns-cleand-conf.c,v 1.2 2000/07/31 19:15:56 drt Exp $";

#define FATAL "ddns-cleand-conf: fatal: "

void usage(void)
{
  strerr_die1x(100,"ddns-cleand-conf: usage: ddns-cleand-conf acct logacct /ddns-cleand /ddns/root/dot");
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

  start("run");
  outs("#!/bin/sh\nexec 2>&1\n");
  outs("ROOT="); outs(datadir); outs("; export ROOT\n");
  outs("exec envuidgid "); outs(user);
  outs(" \\\nsoftlimit -d250000");
  outs(" "); outs(auto_home); outs("/bin/ddns-cleand dot\n");
  finish();
  perm(0755);

  return 0;
}
