/* $Id: ddns-ipfwo.linux.c,v 1.3 2000/08/09 15:43:30 drt Exp $
 *  -- drt@ailis.de
 *
 * Linux specific daemon for changing firewall rules on the fly
 *
 * $Log: ddns-ipfwo.linux.c,v $
 * Revision 1.3  2000/08/09 15:43:30  drt
 * ipfwo.linux is now shomehow mature
 *
 * Revision 1.2  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */


#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if.h>
#include <linux/ip_fw.h>
#include <arpa/inet.h>

#include "byte.h"
#include "buffer.h"
#include "coe.h"
#include "env.h"
#include "error.h"
#include "fifo.h"
#include "fmt.h"
#include "getln.h"
#include "ndelay.h"
#include "open.h"
#include "scan.h"
#include "str.h"
#include "strerr.h"
#include "timeoutread.h"
#include "uint32.h"

#include "ddns.h"
//#include "libipfwc.h"

static char rcsid[] = "$Id: ddns-ipfwo.linux.c,v 1.3 2000/08/09 15:43:30 drt Exp $";

#define ARGV0 "ddns-ipfwo: "
#define FATAL "ddns-ipfwo: fatal: "
#define FIFONAME "ddns-ipfwo"

static char waitreadspace[1024];
static buffer wr;
static int sock;

/* uppercase in reference to k */
static struct ip_fwchange SFWChange;
static struct ip_fwnew SFWNew;


void die_nomem(void)
{
  strerr_die2sys(111, FATAL, "oh no - no memory: ");
}

static void die_read(void)
{
  strerr_die4sys(111,FATAL,"unable to read ", FIFONAME, ": ");
}

static int waitread(int fd, char *buf, unsigned int len)
{
  int r;
  r = timeoutread(1000, fd, buf, len);
  if (r <= 0) 
    if(errno != error_timeout)
      /* XXX: should we ignore (some) errors ? */
      die_read();
  
  return r;
}

void doit()
{
  uint32 uid   =  0;
  char ip4[4]  = {0};
  char ip6[16] = {0};
  char loc[16] = {0};
  char ch;
  int match = 1;
  long linenum = 0;
  stralloc fifoline = { 0 };
  linenum = 0;
  
  buffer_putsflush(buffer_2, ARGV0 "entering main loop\n");
  
  /* forever read from pipe line by line and handle it */
  while(1)
    {
      while(match) 
	{
	  ++linenum;
	  if(buffer_get(&wr, &ch, 1) == 1)
	    {
	      if(getln(&wr, &fifoline, &match, '\n') == -1)
		continue;
	      
	      buffer_put(buffer_2, &ch, 1);
	      buffer_putflush(buffer_2, fifoline.s, fifoline.len);

	      switch(ch)
		{
		case 's':
		case 'k':
		  ddns_parseline(fifoline.s, &uid, ip4, ip6, loc);
		  SFWChange.fwc_rule.ipfw.fw_src.s_addr = ip4[0] << 24 | ip4[1] << 16 | ip4[2] << 8 | ip4[3];
		  SFWChange.fwc_rule.ipfw.fw_smsk.s_addr = 0xffffffff;
		  SFWChange.fwc_rule.ipfw.fw_dst.s_addr = 0;
		  SFWChange.fwc_rule.ipfw.fw_dmsk.s_addr = 0;		  

		  byte_zero(SFWChange.fwc_rule.label, sizeof(SFWChange.fwc_rule.label));
		  byte_zero(SFWChange.fwc_label, sizeof(SFWChange.fwc_label));
		  str_copy(SFWChange.fwc_rule.label, "ddns-ok"); // target
		  str_copy(SFWChange.fwc_label, "ddns"); // chain to add to

		  if(ch == 's')
		    {
		      byte_copy(&SFWNew.fwn_rule, sizeof(struct ip_fwuser), &SFWChange.fwc_rule);
		      byte_zero(SFWNew.fwn_label, sizeof(SFWNew.fwn_label));
		      str_copy(SFWNew.fwn_label, "ddns"); // chain to add to
		      SFWNew.fwn_rulenum = 1;

		      if(setsockopt(sock, IPPROTO_IP, IP_FW_INSERT, &SFWNew, sizeof(SFWNew)))
			strerr_die2sys(111, FATAL, "can't insert rule: " ); 
		    }
		  else
		    {

		      buffer_putsflush(buffer_2, "DELETING\n");

		      if(setsockopt(sock, IPPROTO_IP, IP_FW_DELETE, &SFWChange, sizeof(SFWChange)))
			strerr_die2sys(111, ARGV0, "warning: can't delete rule: " );  
		    }
		}
	    }
	}
    }
}

int main(int argc, char **argv)
{
  int fdfifo, fdfifowrite;
  uint32 gid, uid;
  char *x;

  VERSIONINFO;

  x = env_get("GID");
  if (!x)
    strerr_die2x(111, FATAL, "$GID not set");
  scan_ulong(x,&gid);
  if (prot_gid((int) gid) == -1)
    strerr_die2sys(111, FATAL, "unable to setgid: ");

  x = env_get("UID");
  if (!x)
    strerr_die2x(111, FATAL, "$UID not set");
  scan_ulong(x,&uid);

  x = env_get("WORKDIR");
  if (!x)
    strerr_die2x(111, FATAL, "$WORKDIR not set");
  if (chdir(x) == -1)
    strerr_die4sys(111, FATAL, "unable to chdir to ", x, ": ");

  if (chdir("tracedir") == -1)
    strerr_die4sys(111, FATAL, "unable to chdir to ", x, ": ");

  /* chroot()ing doesn't help much as long as running as root but at
   * least it puts up an extra line of defense between us and an
   * attacker */
  if (chroot(".") == -1)
    strerr_die4sys(111, FATAL,"unable to chroot to ", x, ": ");
  
  buffer_putsflush(buffer_2, ARGV0 "starting\n");

  if(fifo_make(FIFONAME, 0620) == -1)
    strerr_warn4(ARGV0, "warning: unable to create fifo ", FIFONAME, " ", &strerr_sys);

  fdfifo = open_read(FIFONAME);
  if(fdfifo == -1)
    strerr_die4sys(111, FATAL, "unable to open for read ", FIFONAME, " ");
  coe(fdfifo);
  ndelay_on(fdfifo); /* DJB says: shouldn't be necessary */

  /* we have to chmod since wo don't know what umask did to our mkfifo */
 if(chmod(FIFONAME, 0620) == -1)
    strerr_die4sys(111, FATAL, "unable to chmod() fifo ", FIFONAME, " "); 
 if(chown(FIFONAME, uid, gid) == -1)
    strerr_die4sys(111, FATAL, "unable to chown() fifo ", FIFONAME, " "); 

  /* we need this to keep the fifo from beeing closed */
  fdfifowrite = open_write(FIFONAME);
  if (fdfifowrite == -1)
    strerr_die4sys(111, FATAL, "unable to open for write ", FIFONAME, " ");
  coe(fdfifowrite);

  /* init a buffer for nonblocking reading */
  buffer_init(&wr, waitread, fdfifo, waitreadspace, sizeof waitreadspace);

  /* this socket is used to play with the firewall rules */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1)
    strerr_die2sys(111, FATAL, "can't create socket: "); 

  /* intit structure for firewall modification */
  SFWChange.fwc_rule.ipfw.fw_src.s_addr=0;
  SFWChange.fwc_rule.ipfw.fw_dst.s_addr=0;
  SFWChange.fwc_rule.ipfw.fw_smsk.s_addr=0;
  SFWChange.fwc_rule.ipfw.fw_dmsk.s_addr=0;
  SFWChange.fwc_rule.ipfw.fw_flg=16;
  SFWChange.fwc_rule.ipfw.fw_invflg=0;
  SFWChange.fwc_rule.ipfw.fw_proto=0;
  SFWChange.fwc_rule.ipfw.fw_spts[0]=0;
  SFWChange.fwc_rule.ipfw.fw_spts[1]=65535;
  SFWChange.fwc_rule.ipfw.fw_dpts[0]=0;
  SFWChange.fwc_rule.ipfw.fw_dpts[1]=65535;
  SFWChange.fwc_rule.ipfw.fw_tosand=255;
  SFWChange.fwc_rule.ipfw.fw_tosxor=0;
  SFWChange.fwc_rule.ipfw.fw_redirpt=0;
  SFWChange.fwc_rule.ipfw.fw_mark=0;
  SFWChange.fwc_rule.ipfw.fw_outputsize=0;
  str_copy(SFWChange.fwc_rule.ipfw.fw_vianame,"");

  if(setsockopt(sock, IPPROTO_IP, IP_FW_CREATECHAIN, "ddns-ko\0\0", 9))
    strerr_warn2(ARGV0, "warning: unable to create chain ddns-ko (will leave it alone): ", &strerr_sys);
  else
    {
      str_copy(SFWChange.fwc_rule.label, "REJECT"); // target
      str_copy(SFWChange.fwc_label, "ddns-ko"); // chain to add to
      
      if (setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
	strerr_die2sys(111, FATAL, "can't append default rule to chain ddns-ko: "); 
    }

  if(setsockopt(sock, IPPROTO_IP, IP_FW_CREATECHAIN, "ddns-ok\0\0", 9))
    strerr_warn2(ARGV0, "warning: unable to create chain ddns-ok (will leave it alone): ", &strerr_sys);
  else
    {
      str_copy(SFWChange.fwc_rule.label, "ACCEPT"); // target
      str_copy(SFWChange.fwc_label, "ddns-ok"); // chain to add to

      if(setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
	strerr_die2sys(111, FATAL, "can't append default rule to chain ddns-ok: " ); 
    }

  if(setsockopt(sock, IPPROTO_IP, IP_FW_CREATECHAIN, "ddns\0\0\0\0\0", 9))
    {
      strerr_warn2(ARGV0, "warning: unable to create chain ddns (will try to flush it): ", &strerr_sys);
    
      if(setsockopt(sock, IPPROTO_IP, IP_FW_FLUSH, "ddns\0\0\0\0\0", 9))
	strerr_die2sys(111, FATAL, "couldn't flush chain ddns: "); 
    }
  
  byte_copy(SFWChange.fwc_rule.label, sizeof(SFWChange.fwc_rule.label), "ddns-ko\0\0"); // target
  byte_copy(SFWChange.fwc_label, sizeof(SFWChange.fwc_label), "ddns\0\0\0\0\0"); // chain to add to
  
  if(setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
    strerr_die2sys(111, FATAL, "can't append default rule to chain ddns: " ); 

  doit();

  return 1;
}
