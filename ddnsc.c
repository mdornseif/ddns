/* $Id: ddnsc.c,v 1.9 2000/07/07 13:32:47 drt Exp $
 *
 * client for ddns
 * 
 * $Log: ddnsc.c,v $
 * Revision 1.9  2000/07/07 13:32:47  drt
 * ddnsd and ddnsc now basically work as they should and are in
 * a usable state. The protocol is changed a little bit to lessen
 * problems with alignmed and so on.
 * Tested on IA32 and PPC.
 * Both are still missing support for LOC.
 *
 * Revision 1.8  2000/05/02 22:53:49  drt
 * Changed keysize to 256 bits
 * cleand code a bit and removed a lot of cruft
 * some checks for errors
 *
 * Revision 1.7  2000/04/30 23:03:18  drt
 * Unknown user is now communicated by using uid == 0
 *
 * Revision 1.6  2000/04/30 15:59:26  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.5  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.4  2000/04/27 12:12:40  drt
 * Changed data packets to 32+512 Bits size, added functionality to
 * transport IPv6 adresses and LOC records.
 *
 * Revision 1.3  2000/04/24 16:30:56  drt
 * First basically working version implementing full protocol
 *
 * Revision 1.2  2000/04/21 06:58:36  drt
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2000/04/17 16:18:35  drt
 * initial ddns version
 *
 */

#include <stdlib.h>    /* random, clock */
#include <time.h>      /* time */
#include <unistd.h>    /* close, getpid, getppid */

#include "buffer.h"
#include "byte.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "ip4.h"
#include "pad.h"
#include "scan.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "txtparse.h"

#include "mt19937.h"
#include "rijndael.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddnsc.c,v 1.9 2000/07/07 13:32:47 drt Exp $";

#define FATAL "ddnsc: "

stralloc key = {0};

int fd;
buffer b;
char bspace[1024];
char netreadspace[128];

void die_usage(void)
{
  strerr_die1x(111, "usage: ddnsc uid myip (s|r|k)");
}

int saferead(int fd,char *buf,unsigned int len)
{
  int r;

  r = timeoutread(120,fd,buf,len);
  if (r == 0) 
    { 
      errno = error_proto; 
      strerr_die2sys(111, FATAL, "unable to read from server ");
    }
  
  if (r <= 0) 
    strerr_die2sys(111, FATAL, "unable to read from server ");

  return r;
}

int safewrite(int fd,char *buf,unsigned int len)
{
  int r;
  
  r = timeoutwrite(60, fd, buf, len);
  if (r <= 0) 
    strerr_die2sys(111, FATAL, "unable to write to network: ");
  
  return r;
}

buffer netread = BUFFER_INIT(saferead, 6, netreadspace, sizeof netreadspace);
char netwritespace[128];
buffer netwrite = BUFFER_INIT(safewrite, 7, netwritespace, sizeof netwritespace);


/* read, decode and decrypt packet */
int ddnsc_recive(struct ddnsreply *p)
{
  int r;
  struct ddnsreply ptmp = {0};

  r = timeoutread(120, 6, &ptmp, sizeof(struct ddnsreply));
  if(r != 68)
    strerr_die1x(100, "wrong packetsize");
  
  uint32_unpack((char*) &ptmp.uid, &p->uid);
  
  /* XXX: check for my own userid */
  
  /* the key should be set up by ddnsc_send already */
  
  /* decrypt with rijndael */
  rijndaelDecrypt((char *) &ptmp.type);
  rijndaelDecrypt((char *) &ptmp.type + 32);

  uint16_unpack((char*) &ptmp.type, &p->type);
  uint32_unpack((char*) &ptmp.magic, &p->magic);
  taia_unpack((char*) &ptmp.timestamp, &p->timestamp);
  uint32_unpack((char*) &ptmp.leasetime, &p->leasetime);

  if(p->uid == 0)
    strerr_die1x(100, "user unknown to server"); 

  if(p->magic != DDNS_MAGIC)
    strerr_die1x(100, "wrong magic");
  
  return p->type;
}

/* fill p with random, timestamp, magic, encrypt it and send it */
static void ddnsc_send(struct ddnsrequest *p)
{
  struct taia t;
  struct ddnsrequest ptmp = {0};

  /* fill our structure */
  /* and get it into network byte order */
  p->magic = DDNS_MAGIC;
  taia_now(&t);
  taia_pack((char *) &ptmp.timestamp, &t);
  uint32_pack((char*) &ptmp.uid, p->uid);
  uint32_pack((char*) &ptmp.magic, p->magic);
  byte_copy(&ptmp.ip4[0], 4, p->ip4);
  byte_copy(&ptmp.ip6[0], 16, p->ip6);
  uint16_pack((char*) &ptmp.type, p->type);
  uint32_pack((char*) &ptmp.loc_lat, p->loc_lat);
  uint32_pack((char*) &ptmp.loc_long, p->loc_long);
  uint32_pack((char*) &ptmp.loc_alt, p->loc_alt);
  ptmp.loc_size = p->loc_size;
  ptmp.loc_hpre = p->loc_hpre;
  ptmp.loc_vpre = p->loc_vpre;
  ptmp.random1 = randomMT() & 0xffff;
  ptmp.random2 = randomMT() & 0xffff;
  /* fill reserved with random data */
  ptmp.reserved1 = randomMT() & 0xff;;
  ptmp.reserved2 = randomMT() & 0xffff;


  /* initialize rijndael with 256 bit blocksize and 256 bit keysize */
  rijndaelKeySched(8, 8, key.s);
    
  /* Encrypt with rijndael */
  rijndaelEncrypt((char *) &ptmp.type);
  rijndaelEncrypt((char *) &ptmp.type + 32);  

  buffer_put(&netwrite, (char *) &ptmp, sizeof(struct ddnsrequest));
  buffer_flush(&netwrite); 
}

int main(int argc, char **argv)
{
  struct ddnsrequest p = {0};
  struct ddnsreply r = {0};
  unsigned char ip[4] = {0};
  uint32 uid = 0;
  int a;
  char *x;
  char strnum[FMT_ULONG];

  /* seed some entropy into the MT */
  seedMT((long long) getpid () *
	 (long long) time(0) *
	 (long long) getppid() * 
	 (long long) random() * 
	 (long long) clock());

  if(argc != 4)
    die_usage();
  
  scan_ulong(argv[1], &uid);
  if(!uid) die_usage();

  a = ip4_scan(argv[2], ip);
  if((uint32 *)ip == 0) die_usage();
  
  if(*argv[3] == 's')
    p.type = DDNS_T_SETENTRY;
  else if(*argv[3] == 'r')
    p.type = DDNS_T_RENEWENTRY;
  else if(*argv[3] == 'k')
    p.type = DDNS_T_KILLENTRY;
  else 
    die_usage();
  
  /* get key from enviroment */
  x = env_get("KEY");
  if (!x)
    {
      strerr_die2x(111, FATAL, "$KEY not set");
    }

  stralloc_copys(&key, x);
  txtparse(&key);
  pad(&key, 32);

  p.uid = uid;
  byte_copy(&p.ip4[0], 4, ip);

  // XXX: put something useful here
  byte_copy(&p.ip6[0], 16, "1234567890abcdef");
  p.loc_lat = 70;
  p.loc_long = 60;
  p.loc_alt = 70;
  p.loc_size = 60;
  p.loc_hpre = 70;
  p.loc_vpre = 60;
  
  /* send request */
  ddnsc_send(&p);

  /* get answer */
  ddnsc_recive(&r);

  switch(r.type)
    {
    case DDNS_T_ACK: 
      buffer_puts(buffer_1, "ACK: leasetime ");
      buffer_put(buffer_1, strnum, fmt_ulong(strnum, r.leasetime));
      buffer_puts(buffer_1, "\n");
      break;
    case DDNS_T_NAK: 
      buffer_puts(buffer_1, "NAK: mail drt@ailis.de\n"); 
      break;
    case DDNS_T_ESERVINT: 
      buffer_puts(buffer_1, "EIN: internal server error\n"); 
      break;
    case DDNS_T_EPROTERROR: 
      buffer_puts(buffer_1, "EPR: mail drt@ailis.de\n"); 
      break;   
    case DDNS_T_EWRONGMAGIC: 
      buffer_puts(buffer_1, "EWM: server thinks we send him a message with bad magic\n");
      break;  
    case DDNS_T_ECANTDECRYPT: 
      buffer_puts(buffer_1, "EDC: server can not decrypt our message\n"); 
      break; 
    case DDNS_T_EALLREADYUSED: 
      buffer_puts(buffer_1, "EUD: uid has already registered an ip\n"); 
      break;
    case DDNS_T_EUNKNOWNUID: 
      buffer_puts(buffer_1, "EID: uid is unknown to the server\n"); 
      break;  
    case DDNS_T_ENOENTRYUSED:
	buffer_puts(buffer_1, "ENE: client requsted to renew/kill something which is not set\n");
	break;
    case DDNS_T_EUNSUPPTYPE: 
      buffer_puts(buffer_1, "EUS: \n"); 
      break;  
    default:
      strerr_die1x(100, "unknown packet");
    }
  buffer_flush(buffer_1);  
  
  return 0;
}
