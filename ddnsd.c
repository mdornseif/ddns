/* $Id: ddnsd.c,v 1.1 2000/04/19 07:01:46 drt Exp $
 *
 * server for ddns
 * 
 * $Log: ddnsd.c,v $
 * Revision 1.1  2000/04/19 07:01:46  drt
 * Initial revision
 *
 */

#include <sys/stat.h>
#include <errno.h>

#include "djblib/buffer.h"
#include "djblib/cdb.h"
#include "djblib/stralloc.h"
#include "djblib/env.h"
#include "djblib/fmt.h"
#include "djblib/ip4.h"
#include "djblib/write.h"
#include "djblib/open.h"
#include "djblib/error.h"
#include "djblib/strerr.h"

#include "lib/mt19937.h"
#include "lib/rijndael.h"

#include "ddns.h"

static char rcsid[] = "$Id";

static char datadir[] = "dnsdata/cx/rc23/walledcity/dyn";

static unsigned char *remotehost, *remoteinfo, *remoteip, *remoteport;
static stralloc username = { 0 };

/* fill p with random, timestamp, magic, encrypt it and send it */
void ddnsd_send(struct ddnsreply *p)
{
  struct taia t;
  struct ddnsreply ptmp;

  /* fill our structure */
  /* and get it into network byte order */
  p->magic = DDNS_MAGIC;
  ptmp.random = randomMT() & 0xffff;
  taia_now(&t);
  taia_pack((char *) &ptmp.timestamp, &t);
  tai_pack((char *) &ptmp.leasetime, &p->leasetime);
  uint32_pack((char*) &ptmp.uid, p->uid);
  uint16_pack((char*) &ptmp.type, p->type);
  uint32_pack((char*) &ptmp.magic, p->magic);
  
  /* encrypt with rijndael, key shedule was already set up when reciving data */
  rijndaelEncrypt((char *) &ptmp.type);

  buffer_put(buffer_1, (char *) &ptmp, sizeof(struct ddnsreply));
  buffer_flush(buffer_1); 
}

/* Sends an error packet to the client and logs an error */
void ddnsd_send_err(uint32 uid, uint16 errtype, char *errstr)
{
  char strnum[FMT_ULONG];
  struct ddnsreply p = { 0 };
  
  p.type = errtype;
  p.uid = uid;

  /* leasetime is unused so we fill it with random data */
  p.leasetime.x = (uint64) (randomMT() & (uint64)((uint64)randomMT() << 32));

  /* Do logging */
  buffer_puts(buffer_2, remotehost);
  buffer_puts(buffer_2, " [");
  buffer_puts(buffer_2, remoteip);  
  buffer_puts(buffer_2, ":");
  buffer_puts(buffer_2, remoteport);
  buffer_puts(buffer_2, "] ");
  buffer_puts(buffer_2, remoteinfo);
  buffer_puts(buffer_2, " ");
  buffer_put(buffer_2, strnum, fmt_ulong(strnum, uid));
  buffer_puts(buffer_2, ": ");
  buffer_puts(buffer_2, errstr);
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);

  ddnsd_send(&p);

  exit(111);
}

void ddnsd_send_err_sys(uint32 uid, char *errstr)
{
  stralloc err = { 0 };

  stralloc_copys(&err, errstr);
  stralloc_cats(&err, " ");
  stralloc_cats(&err, error_str(errno));
  stralloc_0(&err);
  ddnsd_send_err(uid, DDNS_T_ESERVINT, err.s);
}

void dump_packet(struct ddnsrequest *p, char *info)
{
  char strnum[FMT_ULONG];
  
  buffer_puts(buffer_2, info);
  buffer_puts(buffer_2, ": uid=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->uid));
  buffer_puts(buffer_2, " type=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->type));
  buffer_puts(buffer_2, " magic=0x");
  buffer_put(buffer_2, strnum, fmt_xint(strnum, p->magic));
  buffer_puts(buffer_2, " ip=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->ip));
  buffer_puts(buffer_2, " rnd=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->random));
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);
  
}

int ddnsd_find_user(struct ddnsrequest *p, stralloc *sa)
{
  struct cdb c;
  int r;
  int fd;
  char key[4];

  fd = open_read("data.cdb");
  if (fd == -1) 
    {
      ddnsd_send_err_sys(p->uid, "can't open data.cdb");
    }
  
  cdb_init(&c, fd);

  /* find entry */
  /* uid in cdb is in network byte order */ 
  uint32_pack(key, p->uid);
  r = cdb_find(&c, key, 4); 
  if (r == 1)
    {
      /* read data */
      stralloc_ready(sa, cdb_datalen(&c));
      if (cdb_read(&c, sa->s, cdb_datalen(&c), cdb_datapos(&c)) == -1)
	{
	  ddnsd_send_err(p->uid, DDNS_T_ESERVINT, "can't read from data.cdb");
	}
      else
	{
	  sa->len = cdb_datalen(&c);
	}
    }
  else
    {
      /* can't find id */
      ddnsd_send_err(p->uid, DDNS_T_EUNKNOWNUID, "unknown user");
    }

  cdb_free(&c);
  close(fd);
}


/* read, decode and decrypt packet */
int ddnsd_recive(struct ddnsrequest *p)
{
  int r;
  char *key;
  struct taia t;
  struct ddnsrequest ptmp = { 0 };
  stralloc data = { 0 };

  r = timeoutread(60, 0, &ptmp, sizeof(struct ddnsrequest));
  /* XXX: check result */

  dump_packet(&ptmp, "frisch empfangen");
  uint32_unpack((char*) &ptmp.uid, &p->uid);
  
  ddnsd_find_user(p, &data);

  key = data.s;
  stralloc_copyb(&username, &data.s[16], data.len-16);

  /* initialize rijndael with 256 bit blocksize and 128 bit keysize */
  rijndaelKeySched(8, 4, key);
  
  /* encrypt with rijndael */
  rijndaelDecrypt((char *) &ptmp.type);

  uint16_unpack((char*) &ptmp.type, &p->type);
  uint32_unpack((char*) &ptmp.magic, &p->magic);
  uint32_unpack((char*) &ptmp.ip, &p->ip);
  taia_unpack((char*) &ptmp.timestamp, &p->timestamp);

  dump_packet(p, "entpackt");

  if(p->magic != DDNS_MAGIC)
    {
      ddnsd_send_err(p->uid, DDNS_T_EWRONGMAGIC, "wrong magic");
    }
  
  return p->type;
}

/* handle a setentryrequest */
void ddnsd_setentry( struct ddnsrequest *p)
{
  struct ddnsreply r = { 0 };
  unsigned long pid = 0;
  unsigned long time = 0;
  struct stat st = {0};
  char host[64] = {0};
  char *s = NULL;
  int loop = 0;
  int fd = 0;
  int pos = 0;
  stralloc tmpname = { 0 };
  char inbuf[BUFFER_INSIZE];
  char outbuf[BUFFER_OUTSIZE];
  buffer ssin;
  char strnum[FMT_ULONG];
  char strip[IP4_FMT];
  buffer ssout;

  /* create a temporary name */
  stralloc_copys(&tmpname, datadir);
  stralloc_cats(&tmpname, "/");
  stralloc_cat(&tmpname, &username);
  stralloc_0(&tmpname);

  if (stat(tmpname.s,&st) == -1) 
    {
      if(errno != ENOENT)
	{
	  ddnsd_send_err_sys(p->uid, tmpname.s);
	}
      /* else: everything is fine, the File doesn't exist */
    }
  else
    {
      ddnsd_send_err(p->uid, DDNS_T_EALLREADYUSED, "allready registered");
    }
  
  /* do we have a race condition here? */
  /* Yes, but the only harm this race can cause ist that a user 
     gets DDNS_T_ESERVINT instead of DDNS_T_EALLREADYUSED 
  */
  
  fd = open_excl(tmpname.s);

  if(fd == -1)
    {
      ddnsd_send_err_sys(p->uid, tmpname.s);
    }

  buffer_init(&ssout,write, fd, outbuf, sizeof outbuf);
  buffer_put(&ssout, strip, ip4_fmt(strip, (char *) &p->ip));
  buffer_puts(&ssout, ":");
  buffer_put(&ssout, strnum, fmt_xlong(strnum, p->uid));
  buffer_puts(&ssout, "\n");
  buffer_flush(&ssout); 
  
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime.x = 2317;	 
  ddnsd_send(&r);  
}

main()
{
  struct ddnsrequest p = { 0 };
  struct ddnsreply r = { 0 };
  unsigned char query[256] = {0};
  unsigned char clean_query[256] = {0};
  unsigned char *qptr = NULL;
  unsigned char *qptr2 = NULL;
  int len, query_len;
  int fd = 0;
  stralloc answer = {0};
  int strnum;
  
  /* chroot() to $ROOT and switch to $UID:$GID */
  //  droproot("dffingerd: ");

  /* seed some entropy into the MT */
  seedMT((long long) getpid () *
	 (long long) time(0) *
	 (long long) getppid() * 
	 (long long) random() * 
	 (long long) clock());

  /* since we run under tcpserver, we can get all info 
     about the remote side from the enviroment */
  remotehost = env_get("TCPREMOTEHOST");
  if (!remotehost) remotehost = "unknown";
  remoteinfo = env_get("TCPREMOTEINFO");
  if (!remoteinfo) remoteinfo = "-";
  remoteip = env_get("TCPREMOTEIP");
  if (!remoteip) remoteip = "unknown";
  remoteport = env_get("TCPREMOTEPORT");
  if (!remoteport) remoteport = "unknown";
  
  /* now: 
     remotehost is the remote hostname or "unknown" 
     remoteinfo is some ident string or "-"
     remoteip is the remote ipadress or "unknown" (?)
  */

  ddnsd_recive(&p);
  
  switch(p.type)
    {
    case DDNS_T_SETENTRY:
    case 234545:
      ddnsd_setentry(&p);
      break;
    case DDNS_T_RENEWENTRY:
    case DDNS_T_KILLENTRY:       
      r.type = DDNS_T_NAK;
      r.uid = p.uid;
      r.leasetime.x = 2300;	 
      ddnsd_send(&r);
      break;
    default:
      ddnsd_send_err(p.uid, DDNS_T_EUNSUPPTYPE, "unsupported type/command");
    }
  
  exit(0);
}
