/* $Id: ddnsc.c,v 1.2 2000/04/21 06:58:36 drt Exp $
 *
 * client for ddns
 * 
 * $Log: ddnsc.c,v $
 * Revision 1.2  2000/04/21 06:58:36  drt
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2000/04/17 16:18:35  drt
 * initial ddns version
 *
 */


#include "djblib/buffer.h"
#include "djblib/env.h"
#include "djblib/error.h"
#include "djblib/fmt.h"
#include "djblib/stralloc.h"
#include "djblib/strerr.h"
#include "djblib/timeoutwrite.h"

#include "lib/mt19937.h"
#include "lib/rijndael.h"

#include "ddns.h"

#define FATAL "ddnsc: "

char *key = "geheimgeheimgehe";

void die_generate(void)
{
  strerr_die2sys(111,FATAL,"unable to generate AXFR query: ");
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
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->random));
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

  uint16_unpack((char*) &ptmp.type, &p->type);
  uint32_unpack((char*) &ptmp.magic, &p->magic);
  taia_unpack((char*) &ptmp.timestamp, &p->timestamp);
  tai_unpack((char*) &ptmp.leasetime, &p->leasetime);

  dump_packet(p, "unpacked");

  if(p->magic != DDNS_MAGIC)
    {
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
  uint32_pack((char*) &ptmp.ip, p->ip);
  uint16_pack((char*) &ptmp.type, p->type);
  ptmp.random = randomMT() & 0xffff;
  /* fill reserved with random data */
  ptmp.reserved = randomMT();

  /* initialize rijndael with 256 bit blocksize and 128 bit keysize */
  rijndaelKeySched(8, 4, key);
    
  /* Encrypt with rijndael */
  rijndaelEncrypt((char *) &ptmp.type);

  buffer_put(&netwrite, (char *) &ptmp, sizeof(struct ddnsrequest));
  buffer_flush(&netwrite); 
}

main()
{
  struct ddnsrequest p = { 0 };
  struct ddnsreply r = { 0 };
  unsigned char *remotehost, *remoteinfo, *remoteip, *remoteport;
  unsigned char query[256] = {0};
  unsigned char clean_query[256] = {0};
  unsigned char *qptr = NULL;
  unsigned char *qptr2 = NULL;
  int len, query_len;
  int fd = 0;
  stralloc answer = {0};
  
  /* chroot() to $ROOT and switch to $UID:$GID */
  //  droproot("dffingerd: ");

  /* seed some entropy into the MT */
  seedMT((long long) getpid () *
	 (long long) time(0) *
	 (long long) getppid() * 
	 (long long) random() * 
	 (long long) clock());

  p.uid = 16;
  p.type = DDNS_T_SETENTRY;
  p.ip = (127 << 24) | 1;
  ddnsc_send(&p);
  ddnsc_recive(&r);

  switch(r.type)
    {
    case DDNS_T_ACK: dump_packet(&r, "ACK"); break;
    case DDNS_T_NAK: dump_packet(&r, "NAK"); break;
    case DDNS_T_ESERVINT: dump_packet(&r, "ESINT"); break;
    case DDNS_T_EPROTERROR: dump_packet(&r, "ESINT"); break;   
    case DDNS_T_EWRONGMAGIC: dump_packet(&r, "EWMAG"); break;  
    case DDNS_T_ECANTDECRYPT: dump_packet(&r, "ECDEC"); break; 
    case DDNS_T_EALLREADYUSED: dump_packet(&r, "EUSED"); break;
    case DDNS_T_EUNKNOWNUID: dump_packet(&r, "EUNID"); break;  
    case DDNS_T_EUNSUPPTYPE: dump_packet(&r, "EUNSR"); break;  
    default: dump_packet(&r, "unknown packet");
    }

  sleep(1);
  sleep(2);

  p.type = DDNS_T_RENEWENTRY;
  ddnsc_send(&p);
  ddnsc_recive(&r);
  switch(r.type)
    {
    case DDNS_T_ACK: dump_packet(&r, "ACK"); break;
    case DDNS_T_NAK: dump_packet(&r, "NAK"); break;
    case DDNS_T_ESERVINT: dump_packet(&r, "ESINT"); break;
    case DDNS_T_EPROTERROR: dump_packet(&r, "ESINT"); break;   
    case DDNS_T_EWRONGMAGIC: dump_packet(&r, "EWMAG"); break;  
    case DDNS_T_ECANTDECRYPT: dump_packet(&r, "ECDEC"); break; 
    case DDNS_T_EALLREADYUSED: dump_packet(&r, "EUSED"); break;
    case DDNS_T_EUNKNOWNUID: dump_packet(&r, "EUNID"); break;  
    case DDNS_T_EUNSUPPTYPE: dump_packet(&r, "EUNSR"); break;  
    default: dump_packet(&r, "unknown packet");
    }

  exit(1);

  p.type = DDNS_T_KILLENTRY;
  ddnsc_send(&p);
  ddnsc_recive(&r);
  switch(r.type)
    {
    case DDNS_T_ACK: dump_packet(&r, "ACK"); break;
    case DDNS_T_NAK: dump_packet(&r, "NAK"); break;
    case DDNS_T_ESERVINT: dump_packet(&r, "ESINT"); break;
    case DDNS_T_EPROTERROR: dump_packet(&r, "ESINT"); break;   
    case DDNS_T_EWRONGMAGIC: dump_packet(&r, "EWMAG"); break;  
    case DDNS_T_ECANTDECRYPT: dump_packet(&r, "ECDEC"); break; 
    case DDNS_T_EALLREADYUSED: dump_packet(&r, "EUSED"); break;
    case DDNS_T_EUNKNOWNUID: dump_packet(&r, "EUNID"); break;  
    case DDNS_T_EUNSUPPTYPE: dump_packet(&r, "EUNSR"); break;  
    default: dump_packet(&r, "unknown packet");
    }

  p.type = DDNS_T_RENEWENTRY;
  ddnsc_send(&p);
  ddnsc_recive(&r);
  switch(r.type)
    {
    case DDNS_T_ACK: dump_packet(&r, "ACK"); break;
    case DDNS_T_NAK: dump_packet(&r, "NAK"); break;
    case DDNS_T_ESERVINT: dump_packet(&r, "ESINT"); break;
    case DDNS_T_EPROTERROR: dump_packet(&r, "ESINT"); break;   
    case DDNS_T_EWRONGMAGIC: dump_packet(&r, "EWMAG"); break;  
    case DDNS_T_ECANTDECRYPT: dump_packet(&r, "ECDEC"); break; 
    case DDNS_T_EALLREADYUSED: dump_packet(&r, "EUSED"); break;
    case DDNS_T_EUNKNOWNUID: dump_packet(&r, "EUNID"); break;  
    case DDNS_T_EUNSUPPTYPE: dump_packet(&r, "EUNSR"); break;  
    default: dump_packet(&r, "unknown packet");
    }
  
  exit(0);
}
