/* $Id: ddnsc.c,v 1.5 2000/04/30 14:56:57 drt Exp $
 *
 * client for ddns
 * 
 * $Log: ddnsc.c,v $
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


#include "buffer.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutwrite.h"

#include "lib/mt19937.h"
#include "lib/rijndael.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddnsc.c,v 1.5 2000/04/30 14:56:57 drt Exp $";

#define FATAL "ddnsc: "

char *key = "geheimgeheimgehe";

void die_usage(void)
{
  strerr_die1x(111, "usage: ddnsc uid ip (s|r|k)");
}
void die_parse(void)
{
  strerr_die2sys(111,FATAL,"unable to parse AXFR results: ");
}
void die_netread(void)
{
  strerr_die2sys(111,FATAL,"unable to read from network: ");
}
void die_netwrite(void)
{
  strerr_die2sys(111,FATAL,"unable to write to network: ");
}

void die_write(void)
{
  strerr_die2sys(111,FATAL,"unable to write: ");
}

int saferead(int fd,char *buf,unsigned int len)
{
  int r;
  r = timeoutread(60,fd,buf,len);
  if (r == 0) { errno = error_proto; die_parse(); }
  if (r <= 0) die_netread() ;
  return r;
}

int safewrite(int fd,char *buf,unsigned int len)
{
  int r;
  r = timeoutwrite(60,fd,buf,len);
  if (r <= 0) die_netwrite();
  return r;
}
char netreadspace[1024];
buffer netread = BUFFER_INIT(saferead,6,netreadspace,sizeof netreadspace);
char netwritespace[1024];
buffer netwrite = BUFFER_INIT(safewrite,7,netwritespace,sizeof netwritespace);

void netget(char *buf,unsigned int len)
{
  int r;

  while (len > 0) {
    r = buffer_get(&netread,buf,len);
    buf += r; len -= r;
  }
}

int fd;
buffer b;
char bspace[1024];

void put(char *buf,unsigned int len)
{
  if (buffer_put(&b,buf,len) == -1) die_write();
}

void dump_packet(struct ddnsreply *p, char *info)
{
  char strnum[FMT_ULONG];
  
  buffer_puts(buffer_2, info);
  buffer_puts(buffer_2, ": uid=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->uid));
  buffer_puts(buffer_2, " type=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->type));
  buffer_puts(buffer_2, " magic=0x");
  buffer_put(buffer_2, strnum, fmt_xint(strnum, p->magic));
  buffer_puts(buffer_2, " rnd=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->random1));
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);  
}


/* read, decode and decrypt packet */
int ddnsc_recive(struct ddnsreply *p)
{
  int r;
  struct taia t;
  struct ddnsreply ptmp = { 0 };
  stralloc data = { 0 };

  r = timeoutread(60, 6, &ptmp, sizeof(struct ddnsreply));
  /* XXX: check result */

  uint32_unpack((char*) &ptmp.uid, &p->uid);
  
  /* XXX: check for my own userid */

  /* initialize rijndael with 256 bit blocksize and 128 bit keysize */
  rijndaelKeySched(8, 4, key);
  
  /* decrypt with rijndael */
  rijndaelDecrypt((char *) &ptmp.type);
  rijndaelDecrypt((char *) &ptmp.type + 32);

  uint16_unpack((char*) &ptmp.type, &p->type);
  uint32_unpack((char*) &ptmp.magic, &p->magic);
  taia_unpack((char*) &ptmp.timestamp, &p->timestamp);
  tai_unpack((char*) &ptmp.leasetime, &p->leasetime);


  if(p->magic != DDNS_MAGIC)
    {
      dump_packet(p, "achtung baby");
      strerr_die1x(100,"wrong magic");
    }
  
  return p->type;
}


/* fill p with random, timestamp, magic, encrypt it and send it */
static void ddnsc_send(struct ddnsrequest *p)
{
  struct taia t;
  struct ddnsrequest ptmp = { 0 };

  /* fill our structure */
  /* and get it into network byte order */
  p->magic = DDNS_MAGIC;
  taia_now(&t);
  taia_pack((char *) &ptmp.timestamp, &t);
  uint32_pack((char*) &ptmp.uid, p->uid);
  uint32_pack((char*) &ptmp.magic, p->magic);
  uint32_pack((char*) &ptmp.ip4, p->ip4);
  byte_copy(&ptmp.ip6, 16, p->ip6);
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

  /* initialize rijndael with 256 bit blocksize and 128 bit keysize */
  rijndaelKeySched(8, 4, key);
    
  /* Encrypt with rijndael */
  rijndaelEncrypt((char *) &ptmp.type);
  rijndaelEncrypt((char *) &ptmp.type + 32);

  buffer_put(&netwrite, (char *) &ptmp, sizeof(struct ddnsrequest));
  buffer_flush(&netwrite); 
}

main(int argc, char **argv)
{
  struct ddnsrequest p = { 0 };
  struct ddnsreply r = { 0 };
  unsigned char *remotehost, *remoteinfo, *remoteip, *remoteport;
  unsigned char query[256] = {0};
  unsigned char clean_query[256] = {0};
  unsigned char *qptr = NULL;
  unsigned char *qptr2 = NULL;
  unsigned char ip[4] = { 0 };
  uint32 uid;
  int len, query_len;
  int fd = 0;
  int a;
  stralloc answer = {0};
  char strnum[FMT_ULONG];
  struct tai t;
  uint32 u;

  /* seed some entropy into the MT */
  seedMT((long long) getpid () *
	 (long long) time(0) *
	 (long long) getppid() * 
	 (long long) random() * 
	 (long long) clock());


  if(argc != 4)
    {
      die_usage();
    }

  a = scan_ulong(argv[1], &uid);
  a = ip4_scan(argv[2], ip);
  
  if(*argv[3] == 's')
    {
      p.type = DDNS_T_SETENTRY;
    }
  else if(*argv[3] == 'r')
    {
      p.type = DDNS_T_RENEWENTRY;
    }
  else if(*argv[3] == 'k')
    {
      p.type = DDNS_T_KILLENTRY;
    }
  else 
    {
      die_usage();
    }
  
  p.uid = uid;
  byte_copy(&p.ip4, 4, ip);

  ddnsc_send(&p);
  ddnsc_recive(&r);

  switch(r.type)
    {
    case DDNS_T_ACK: 
      buffer_puts(buffer_1, "ACK: ");
      buffer_put(buffer_1, strnum, fmt_ulong(strnum, r.leasetime.x));
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
    case DDNS_T_EUNSUPPTYPE: 
      buffer_puts(buffer_1, "EUS: \n"); 
      break;  
    default:
      dump_packet(&r, "unknown packet");
    }
  buffer_flush(buffer_1);  
  
  exit(0);
}
