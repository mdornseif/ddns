/* $Id: ddns-ipfwo.linux.c,v 1.6 2000/10/06 22:15:20 drt Exp $
 *  -- drt@ailis.de
 *
 * Linux specific daemon for changing firewall rules on the fly
 *
 * $Log: ddns-ipfwo.linux.c,v $
 * Revision 1.6  2000/10/06 22:15:20  drt
 * fefes djblib added
 *
 * Revision 1.5  2000/08/11 06:41:55  drt
 * Structure of firewallchains changed
 *
 * Revision 1.4  2000/08/10 07:39:03  drt
 * IPs in firewall got mixed up
 *
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

static char rcsid[] = "$Id: ddns-ipfwo.linux.c,v 1.6 2000/10/06 22:15:20 drt Exp $";

#define ARGV0 "ddns-ipfwo: "
#define FATAL "ddns-ipfwo: fatal: "
#define FIFONAME "ddns-ipfwo"

static char waitreadspace[1024];
static buffer wr;


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

/* linux specific functions for ddns-ipfwo */

#include <linux/capability.h>
#define CAP_TO_MASK(x) (1 << (x))
extern int capset(cap_user_header_t header, cap_user_data_t data);

/* socket for firewall manipulation */
static int sock;
/* uppercase in reference to k */
static struct ip_fwchange SFWChange;

/* initialize firewallhandling */
void ipfwo_init()
{
  cap_user_header_t cap_head;
  cap_user_data_t cap;
	  
  /* drop capabilities so we are yust allowed to change network konfiguration 
     - nothing more */

  /* capabilities seem to be seriously broken. Why should I dynamicaly
     allocate memory? */
  cap_head = (cap_user_header_t) alloc(sizeof(cap_user_header_t));
  cap = (cap_user_data_t) alloc(sizeof(cap_user_data_t));
  
  if((!cap_head) || (!cap)) 
    strerr_die2sys(111, FATAL, "can't alloc memory for capabilities: "); 

  cap_head->version = _LINUX_CAPABILITY_VERSION;
  cap_head->pid = 0;
  
  cap->inheritable = cap->permitted = cap->effective = CAP_NET_ADMIN;
  
  if(capset(cap_head, cap) != 0) 
    strerr_die2sys(111, FATAL, "can't drop capabilities: "); 

  alloc_free(cap_head);
  alloc_free(cap);
 
  /* this socket is used to play with the firewall rules */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1)
    strerr_die2sys(111, FATAL, "can't create socket: "); 

  /* init structure for firewall modification */
  SFWChange.fwc_rule.ipfw.fw_src.s_addr=0; SFWChange.fwc_rule.ipfw.fw_smsk.s_addr=0;
  SFWChange.fwc_rule.ipfw.fw_dst.s_addr=0; SFWChange.fwc_rule.ipfw.fw_dmsk.s_addr=0;
  SFWChange.fwc_rule.ipfw.fw_spts[0]=0; SFWChange.fwc_rule.ipfw.fw_spts[1]=65535;
  SFWChange.fwc_rule.ipfw.fw_dpts[0]=0; SFWChange.fwc_rule.ipfw.fw_dpts[1]=65535;
  SFWChange.fwc_rule.ipfw.fw_flg=16;
  SFWChange.fwc_rule.ipfw.fw_invflg=0;
  SFWChange.fwc_rule.ipfw.fw_proto=0;
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
      byte_zero(SFWChange.fwc_rule.label, sizeof(SFWChange.fwc_rule.label));
      str_copy(SFWChange.fwc_rule.label, "REJECT"); // target
      byte_zero(SFWChange.fwc_label, sizeof(SFWChange.fwc_label));
      str_copy(SFWChange.fwc_label, "ddns-ko"); // chain to add to
      
      if (setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
	strerr_die2sys(111, FATAL, "can't append default rule to chain ddns-ko: "); 
    }

  if(setsockopt(sock, IPPROTO_IP, IP_FW_CREATECHAIN, "ddns-ok\0\0", 9))
    strerr_warn2(ARGV0, "warning: unable to create chain ddns-ok (will leave it alone): ", &strerr_sys);
  else
    {
      byte_zero(SFWChange.fwc_rule.label, sizeof(SFWChange.fwc_rule.label));
      str_copy(SFWChange.fwc_rule.label, "ACCEPT"); // target
      byte_zero(SFWChange.fwc_label, sizeof(SFWChange.fwc_label));
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
  
  byte_zero(SFWChange.fwc_rule.label, sizeof(SFWChange.fwc_rule.label));
  str_copy(SFWChange.fwc_rule.label, "ddns-ko"); // target
  byte_zero(SFWChange.fwc_label, sizeof(SFWChange.fwc_label));
  str_copy(SFWChange.fwc_label, "ddns"); // chain to add to
  
  if(setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
    strerr_die2sys(111, FATAL, "can't append default rule to chain ddns: " ); 
}

/* add a rule to the firewallchain */
void ipfwo_add(char *ip4)
{
}

/* delete a rule from the firewallchain*/ 
void ipfwo_del(char *ip4)
{
}


/* generic functions for ddns-ipfwo */

void doit()
{
  uint32 uid   =  0;
  uint32 t;
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
		case 'e':
		  ddns_parseline(fifoline.s, &uid, ip4, ip6, loc);
		  // XXX works this on all circunstances?
		  uint32_unpack(ip4, &t);
		  SFWChange.fwc_rule.ipfw.fw_src.s_addr = t;
		  SFWChange.fwc_rule.ipfw.fw_smsk.s_addr = 0xffffffff;
		  SFWChange.fwc_rule.ipfw.fw_dst.s_addr = 0;
		  SFWChange.fwc_rule.ipfw.fw_dmsk.s_addr = 0;		  

		  byte_zero(SFWChange.fwc_rule.label, sizeof(SFWChange.fwc_rule.label));
		  byte_zero(SFWChange.fwc_label, sizeof(SFWChange.fwc_label));
		  str_copy(SFWChange.fwc_rule.label, "ddns-ok"); // target
		  str_copy(SFWChange.fwc_label, "ddns"); // chain to add to

		  if(ch == 's')
		    {
  		      if(setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
			strerr_die2sys(111, FATAL, "can't insert rule: " ); 
		    }
		  else
		    {
		      if(setsockopt(sock, IPPROTO_IP, IP_FW_DELETE, &SFWChange, sizeof(SFWChange)))
			//	strerr_die2sys(111, ARGV0, "warning: can't delete rule: " );  
			strerr_warn2(ARGV0, "warning: can't delete rule: ", &strerr_sys);
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

  //  VERSIONINFO;
if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'V') 
   { 
     buffer_puts(buffer_2, ARGV0 "Version "); 
     buffer_puts(buffer_2, VERSION); 
     buffer_putsflush(buffer_2, " (Build: "); 
     buffer_puts(buffer_2, __DATE__); 
     buffer_putsflush(buffer_2, ")\n");
     _exit(0); 
   }

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

  ipfwo_init();

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

  doit();

  return 1;
}
