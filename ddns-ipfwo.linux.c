/* $Id: ddns-ipfwo.linux.c,v 1.7 2000/11/21 19:43:22 drt Exp $
 *  -- drt@un.bewaff.net
 *
 * Linux specific functions for changing firewall rules on the fly
 *
 * $Log: ddns-ipfwo.linux.c,v $
 * Revision 1.7  2000/11/21 19:43:22  drt
 * Seperated source into a portable and non-portabe part.
 *
 */


#include <sys/socket.h>
#include <netinet/in.h>
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
#include "traversedirhier.h"

static char rcsid[] = "$Id: ddns-ipfwo.linux.c,v 1.7 2000/11/21 19:43:22 drt Exp $";

#define ARGV0 "ddns-ipfwo: "
#define FATAL "ddns-ipfwo: fatal: "
#define FIFONAME "ddns-ipfwo"

/**************************************************************************/

/* linux specific functions for ddns-ipfwo */

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if.h>
#include <linux/ip_fw.h>

#include <linux/capability.h>
#define CAP_TO_MASK(x) (1 << (x))
extern int capset(cap_user_header_t header, cap_user_data_t data);

static char rbspace[1024];
static buffer rb;

/* socket for firewall manipulation */
static int sock;
/* uppercase in reference to k */
static struct ip_fwchange SFWChange;

/* initialize firewallhandling */
void ipfwo_init()
{
  cap_user_header_t cap_head;
  cap_user_data_t cap;
	  
  /* drop capabilities so we are just allowed to change network konfiguration 
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

/* add a rule to a firewall chain - Linux specific function */
void addrule(char *ip4)
{
 uint32 t;

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
 
 if(setsockopt(sock, IPPROTO_IP, IP_FW_APPEND, &SFWChange, sizeof(SFWChange)))
   strerr_die2sys(111, FATAL, "can't insert rule: " ); 
}

/* del a rule to a firewall chain - Linux specific function */
void delrule(char *ip4)
{
 uint32 t;

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
 
 if(setsockopt(sock, IPPROTO_IP, IP_FW_DELETE, &SFWChange, sizeof(SFWChange)))
   strerr_warn2(ARGV0, "warning: can't delete rule: ", &strerr_sys);
}

/* Linux specific functions end here */ 

/**************************************************************************/
