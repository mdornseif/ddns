/* $Id: ddnsd-conf.c,v 1.1 2000/07/29 21:12:50 drt Exp $
 *  --drt@ailis.de
 *
 * create directory structure for using ddnsd with svscan
 * based on dffingerd conf
 * based on dan Bernseins code
 *
 * You might find more Info at http://rc23.cx/
 * 
 * I do not belive there is a thing like copyright.
 *
 * $Log: ddnsd-conf.c,v $
 * Revision 1.1  2000/07/29 21:12:50  drt
 * initial revision
 *
 */

#include <pwd.h>
#include <unistd.h>
#include "stralloc.h"
#include "strerr.h"
#include "exit.h"
#include "auto_home.h"
#include "generic-conf.h"
#include "ddns.h"

static char rcsid[]="$Id: ddnsd-conf.c,v 1.1 2000/07/29 21:12:50 drt Exp $";

#define FATAL "ddnsd-conf: fatal: "

void usage(void)
{
  strerr_die1x(100,"ddnsd-conf: usage: ddnsd-conf acct logacct /ddns myip myport myzone");
}

char *dir;
char *user;
char *loguser;
struct passwd *pw;
char *myip;
char *myport;
char *myzone;

void outdnszonedirhier(char *dir)
{
  char *p;

  p = dir;

  while(*p)
    p++;
  
  /* I'm ugly and fat */
  while(p > dir)
    {
      while(*p != '.' && p >= dir)
	p--;
      if(p > dir)
	*p = '\0';
      p++;
      outs(p);
      outs("/");      
    }  
}

void mkdnsdirhier(char *base, char *dir)
{
  char wd[1024];
  char *p;

  if(getcwd(wd, sizeof(wd)) == NULL)
    strerr_die1sys(100,"ddnsd-conf: fatal, can't getcwd()");

  if(chdir(base) == -1)
    strerr_die2sys(100,"ddnsd-conf: fatal, can't chdir() to ", base);
    
  p = dir;

  while(*p)
    p++;
  
  /* I'm ugly and fat */
  while(p > dir)
    {
      while(*p != '.' && p >= dir)
	p--;
      if(p > dir)
	*p = '\0';
      p++;
      makedir(p); 
      perm(02750);
      owner(pw->pw_uid,pw->pw_gid);
     if(chdir(p) == -1)
	strerr_die2sys(100,"ddnsd-conf: fatal, can't chdir() to ", p);
    }  

  if(chdir(wd) == -1)
    strerr_die2sys(100,"ddnsd-conf: fatal, can't chdir() to ", wd);
  
}

int main(int argc, char **argv)
{
  stralloc sa = { 0 };

  user = argv[1];
  if (!user) usage();
  loguser = argv[2];
  if (!loguser) usage();
  dir = argv[3];
  if (!dir) usage();
  if (dir[0] != '/') usage();
  myip = argv[4];
  if (!myip) usage();
  myport = argv[5];
  if (!myport) usage();
  myzone = argv[6];
  if (!myzone) usage();

  pw = getpwnam(loguser);
  if (!pw)
    strerr_die3x(111,FATAL,"unknown account ",loguser);

  stralloc_copys(&sa, myzone);
  stralloc_0(&sa);

  init(dir,FATAL);
  makelog(loguser,pw->pw_uid,pw->pw_gid);

  pw = getpwnam(user);
  if (!pw)
    strerr_die3x(111,FATAL,"unknown account ",user);

  start("run");
  outs("#!/bin/sh\nexec 2>&1\n");
  outs("umask 026\n");
  outs("ROOT="); outs(dir); outs("/root; export ROOT\n");
  outs("IP="); outs(myip); outs("; export IP\n");
  outs("exec envuidgid "); outs(user);
  outs(" \\\nsoftlimit -d250000");
  outs(" \\\ntcpserver -Odhpr -c 10 $IP "); outs(myport);
  outs(" "); outs(auto_home); outs("/bin/ddnsd");
  outs(" "); outs("dot/");
  outdnszonedirhier(sa.s);
  outs("\n");
  finish();
  perm(0750);

  makedir("root");
  perm(02770);
  owner(0, pw->pw_gid);

  makedir("root/tmp");
  perm(02750);
  owner(pw->pw_uid,pw->pw_gid);

  makedir("root/dot");
  perm(02750);
  owner(pw->pw_uid,pw->pw_gid);

  mkdnsdirhier("root/dot", myzone);

  start("root/Makefile");
  outs("data.cdb: data\n");
  outs("\t"); outs(auto_home); outs("/bin/ddnsd-data < data\n");
  finish();
  perm(0640);
  owner(0,pw->pw_gid);

  start("root/data");
  outs("# Format: uid,uname,secret\n");
  outs("#\n");
  outs("# example entry\n");
  outs("1,testuser,supersecret\n");
  finish();
  perm(0600);

  return 0;
}
