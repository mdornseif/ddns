/* $Id: ddnsd.c,v 1.6 2000/04/27 09:41:20 drt Exp $
 *
 * server for ddns
 * 
 * $Log: ddnsd.c,v $
 * Revision 1.6  2000/04/27 09:41:20  drt
 * Tiny Bugfix
 *
 * Revision 1.5  2000/04/25 08:34:15  drt
 * Handling of tmp files fixed.
 *
 * Revision 1.4  2000/04/24 16:35:02  drt
 * First basically working version implementing full protocol
 * RENEWENTRY and KILLENTRY finalized
 * logging added
 *
 * Revision 1.3  2000/04/21 06:58:36  drt
 * *** empty log message ***
 *
 * Revision 1.2  2000/04/19 13:36:23  drt
 * Compile fixes
 *
 * Revision 1.1.1.1  2000/04/19 07:01:46  drt
 * initial ddns version
 *
 */

#include <sys/stat.h>
#include <utime.h>
#include <errno.h>

#include "djblib/buffer.h"
#include "djblib/cdb.h"
#include "djblib/stralloc.h"
#include "djblib/env.h"
#include "djblib/fmt.h"
#include "djblib/ip4.h"
#include "djblib/readwrite.h"
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

void ddnsd_log(uint32 uid, char *str)
{
 char strnum[FMT_ULONG];

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
  buffer_puts(buffer_2, str);
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);
}

/* Sends an error packet to the client and logs an error */
void ddnsd_send_err(uint32 uid, uint16 errtype, char *errstr)
{
  struct ddnsreply p = { 0 };
  
  p.type = errtype;
  p.uid = uid;

  /* leasetime is unused when sending error packets so we fill it with random data */
  p.leasetime.x = (uint64) (randomMT() & (uint64)((uint64)randomMT() << 32));

  ddnsd_log(uid, errstr);
 
  ddnsd_send(&p);

  exit(111);
}

void ddnsd_send_err_sys(uint32 uid, char *errstr)
{
  stralloc err = { 0 };

  stralloc_copys(&err, errstr);
  stralloc_cats(&err, ": ");
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
	  ddnsd_send_err_sys(p->uid, "can't read from data.cdb");
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
  stralloc err = { 0 };
  stralloc finname = { 0 };
  char inbuf[BUFFER_INSIZE];
  char outbuf[BUFFER_OUTSIZE];
  buffer ssin;
  char strnum[FMT_ULONG];
  char strip[IP4_FMT];
  buffer ssout;

  
  /* create a temporary name */
  host[0] = 0;
  gethostname(host,sizeof(host));
  for (loop = 0;;++loop)
    {
      stralloc_copys(&tmpname, "tmp/");
      stralloc_catulong0(&tmpname, now(),0); 
      stralloc_cats(&tmpname, "."); 
      stralloc_catulong0(&tmpname, getpid(), 0); 
      stralloc_cats(&tmpname, "."); 
      stralloc_cats(&tmpname, host); 
      stralloc_cats(&tmpname, "-"); 
      stralloc_cat(&tmpname, &username); 
      stralloc_0(&tmpname);
      
      if (stat(tmpname.s,&st) == -1) if (errno == error_noent) break;
      /* really should never get to this point */
      if (loop == 2) _exit(1); /* XXX: logging */
      sleep(1);
    }
	  

  /* create the final name */
  stralloc_copys(&finname, datadir);
  stralloc_cats(&finname, "/");
  stralloc_cat(&finname, &username);
  stralloc_0(&finname);
    

  /* open tmpfile and write to it */
  fd = open_excl(tmpname.s);

  if(fd == -1)
    {
      ddnsd_send_err_sys(p->uid, tmpname.s);
    }
  
  /* XXX: we need checks if the user is comming from the claimed ip */

  /* write data to file */
  buffer_init(&ssout,write, fd, outbuf, sizeof outbuf);
  buffer_puts(&ssout, "=");
  buffer_put(&ssout, strip, ip4_fmt(strip, (char *) &p->ip));
  buffer_puts(&ssout, ":");
  buffer_put(&ssout, strnum, fmt_xlong(strnum, p->uid));
  buffer_puts(&ssout, "\n");
  buffer_flush(&ssout); 

  close(fd);

  /* test if the name we are asked to move to is already there */
  if (stat(finname.s, &st) == -1) 
    {
      if(errno != ENOENT)
	{
	  ddnsd_send_err_sys(p->uid, finname.s);
	}
      /* else: everything is fine, the File doesn't exist */
    }
  else
    {
      ddnsd_send_err(p->uid, DDNS_T_EALLREADYUSED, "allready registered");
    }

  /* move tmp file to final file */

  /* do we have a race condition here? */
  /* Yes, but the only harm this race can cause ist that a user 
     gets DDNS_T_ESERVINT instead of DDNS_T_EALLREADYUSED 
  */

  if(rename(tmpname.s, finname.s) != 0)
    {
      /* try to delete tmpfile */
      unlink(tmpname.s);

      /* return error */
      stralloc_copys(&err, "can't rename ");
      stralloc_cats(&err, tmpname.s);
      stralloc_cats(&err, " to ");
      stralloc_cat(&err, &finname);
      ddnsd_send_err_sys(p->uid, err.s);
    }
  
  ddnsd_log(p->uid, "setting entry");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  /* there should be some more intelligence in setting leasetime */
  r.leasetime.x = 2317;	 
  ddnsd_send(&r);  
}

/* handle a setentryrequest */
void ddnsd_renewentry( struct ddnsrequest *p)
{
  struct ddnsreply r = { 0 };
  stralloc tmpname = { 0 };
  struct stat st = {0};
  struct utimbuf ut;

  /* create the final name */
  stralloc_copys(&tmpname, datadir);
  stralloc_cats(&tmpname, "/");
  stralloc_cat(&tmpname, &username);
  stralloc_0(&tmpname);

  if(stat(tmpname.s, &st) == -1)
    {
      if(errno == ENOENT)
	{
	  // File not found
	  ddnsd_send_err(p->uid, DDNS_T_ENOENTRYUSED, "entry can't be renewed");
	}
      else
	{
	  ddnsd_send_err_sys(p->uid, tmpname.s);
	}
    }

  ut.actime = st.st_atime;
  ut.modtime = st.st_mtime;
  utime(tmpname.s, &ut);

  ddnsd_log(p->uid, "renewing entry");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  /* there should be some more intelligence in setting leasetime */
  r.leasetime.x = 2317;	 
  ddnsd_send(&r);  
}

void ddnsd_killentry( struct ddnsrequest *p)
{
  struct ddnsreply r = { 0 };
  stralloc tmpname = { 0 };

  /* create the final name */
  stralloc_copys(&tmpname, datadir);
  stralloc_cats(&tmpname, "/");
  stralloc_cat(&tmpname, &username);
  stralloc_0(&tmpname);

  if(unlink(tmpname.s) == -1)
    {
      if(errno == ENOENT)
	{
	  // File not found
	  ddnsd_send_err(p->uid, DDNS_T_ENOENTRYUSED, "entry can't be killed");
	}
      else
	{
	  ddnsd_send_err_sys(p->uid, tmpname.s);
	}
    }

  ddnsd_log(p->uid, "killing entry");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime.x = (uint64) (randomMT() & (uint64)((uint64)randomMT() << 32));
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
      ddnsd_log(p.uid, "setentry request");
      ddnsd_setentry(&p);
      break;
    case DDNS_T_RENEWENTRY:
      ddnsd_log(p.uid, "renewentry request");
      ddnsd_renewentry(&p);
      break;
    case DDNS_T_KILLENTRY:       
      ddnsd_log(p.uid, "killentry request");
      ddnsd_killentry(&p);
      break;
    default:
      ddnsd_send_err(p.uid, DDNS_T_EUNSUPPTYPE, "unsupported type/command");
    }
  
  exit(0);
}
