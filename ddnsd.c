/* $Id: ddnsd.c,v 1.12 2000/05/02 21:46:31 drt Exp $
 *
 * server for ddns - this file is to long
 * 
 * $Log: ddnsd.c,v $
 * Revision 1.12  2000/05/02 21:46:31  drt
 * usage of 256 bit keys,
 * huge code cleanup,
 * checking of most possible error conditions
 * like timeout reading, no memory and so on
 *
 * Revision 1.11  2000/05/01 11:47:18  drt
 * ttl/leasetime comes now from data.cdb
 *
 * Revision 1.10  2000/04/30 23:03:18  drt
 * Unknown user is now communicated by using uid == 0
 *
 * Revision 1.9  2000/04/30 15:59:26  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.8  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.7  2000/04/27 12:12:40  drt
 * Changed data packets to 32+512 Bits size, added functionality to
 * transport IPv6 adresses and LOC records.
 *
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

#include <errno.h> 
#include <netdb.h>     /* gethostbyname */
#include <stdio.h>     /* rename */
#include <stdlib.h>    /* random, clock */
#include <sys/stat.h>  /* stat, umask */
#include <sys/types.h> /* utimbuf */
#include <time.h>      /* time */
#include <unistd.h>    /* close, getpid, getppid, sleep, unlink */
#include <utime.h>     /* utimbuf */

#include "buffer.h"
#include "cdb.h"
#include "droproot.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "ip4.h"
#include "now.h"
#include "open.h"
#include "readwrite.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutread.h"

#include "mt19937.h"
#include "rijndael.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddnsd.c,v 1.12 2000/05/02 21:46:31 drt Exp $";

static char *datadir;

static uint32 ttl;

static unsigned char *remotehost, *remoteinfo, *remoteip, *remoteport;
static stralloc username = {0};

void nomem(void)
{
  strerr_die1sys(111, "ddnsd: fatal: help - no memory ");
}

/* fill p with random, timestamp, magic, encrypt it and send it */
void ddnsd_send(struct ddnsreply *p)
{
  struct taia t;
  struct ddnsreply ptmp;

  /* fill our structure */
  /* and get it into network byte order */
  p->magic = DDNS_MAGIC;
  ptmp.random1 = randomMT() & 0xffff;
  taia_now(&t);
  taia_pack((char *) &ptmp.timestamp, &t);
  tai_pack((char *) &ptmp.leasetime, &p->leasetime);
  uint32_pack((char*) &ptmp.uid, p->uid);
  uint16_pack((char*) &ptmp.type, p->type);
  uint32_pack((char*) &ptmp.magic, p->magic);
  ptmp.reserved[0] = randomMT();
  ptmp.reserved[1] = randomMT();
  ptmp.reserved[2] = randomMT();
  ptmp.reserved[3] = randomMT();
  ptmp.reserved[4] = randomMT();
  ptmp.reserved[5] = randomMT();
  ptmp.reserved[6] = randomMT();
  ptmp.reserved[7] = randomMT();
  
  /* encrypt with rijndael, key shedule was already set up when reciving data */
  rijndaelEncrypt((char *) &ptmp.type);
  rijndaelEncrypt((char *) &ptmp.type + 32);

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
  struct ddnsreply p;
  
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
  stralloc err = {0};

  if(!stralloc_copys(&err, errstr)) nomem();
  if(!stralloc_cats(&err, ": ")) nomem();
  if(!stralloc_cats(&err, error_str(errno))) nomem();
  if(!stralloc_0(&err)) nomem();
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
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->ip4));
  buffer_puts(buffer_2, " rnd1=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->random1));
  buffer_puts(buffer_2, " rnd2=0x");
  buffer_put(buffer_2, strnum, fmt_xlong(strnum, p->random2));
  buffer_puts(buffer_2, "\n");
  buffer_flush(buffer_2);
  
}

/* returns 1 when found, 0 when unknown user */
int ddnsd_find_user(uint32 uid, stralloc *sa)
{
  struct cdb c;
  int r;
  int fd;
  char key[4];

  fd = open_read("data.cdb");
  if (fd == -1) 
    {
      ddnsd_send_err_sys(uid, "can't open data.cdb");
    }
  
  cdb_init(&c, fd);

  /* find entry */
  /* uid in cdb is in network byte order */ 
  uint32_pack(key, uid);
  r = cdb_find(&c, key, 4); 
  if (r == 1)
    {
      /* read data */
      if(!stralloc_ready(sa, cdb_datalen(&c))) nomem();
      if (cdb_read(&c, sa->s, cdb_datalen(&c), cdb_datapos(&c)) == -1)
	ddnsd_send_err_sys(uid, "can't read from data.cdb");
      else
	sa->len = cdb_datalen(&c);
    }
  else
    /* can't find id */
    return 0;

  cdb_free(&c);
  close(fd);

  /* did ddnsd-data create crap? */
  if(sa->len < 37)
    return 0;

  return 1;
}


/* read, decode and decrypt packet */
int ddnsd_recive(struct ddnsrequest *p)
{
  int r;
  char *key;
  struct ddnsrequest ptmp;
  stralloc data = {0};
  
  r = timeoutread(60, 0, &ptmp, sizeof(struct ddnsrequest));
  if(r != 68) 
    ddnsd_send_err(p->uid, DDNS_T_EPROTERROR, "wrong packetsize/timeout");

  uint32_unpack((char*) &ptmp.uid, &p->uid);
  
  if(ddnsd_find_user(p->uid, &data))
    {
      key = data.s;
      uint32_unpack(&data.s[32], &ttl);
      if(!stralloc_copyb(&username, &data.s[36], data.len-36)) nomem();
      
      /* initialize rijndael with 256 bit blocksize and 256 bit keysize */
      rijndaelKeySched(8, 8, key);
      
      /* decrypt with rijndael */
      rijndaelDecrypt((char *) &ptmp.type);
      rijndaelDecrypt((char *) &ptmp.type + 32);
      
      uint16_unpack((char*) &ptmp.type, &p->type);
      uint32_unpack((char*) &ptmp.magic, &p->magic);
      uint32_unpack((char*) &ptmp.ip4, &p->ip4);
      taia_unpack((char*) &ptmp.timestamp, &p->timestamp);
      uint32_unpack((char*) &ptmp.loc_lat, &p->loc_lat);
      uint32_unpack((char*) &ptmp.loc_long, &p->loc_long);
      uint32_unpack((char*) &ptmp.loc_alt, &p->loc_alt);
      ptmp.loc_size = p->loc_size;
      ptmp.loc_vpre = p->loc_hpre;
      ptmp.loc_hpre = p->loc_vpre;
      
      if(p->magic != DDNS_MAGIC)
	ddnsd_send_err(p->uid, DDNS_T_EWRONGMAGIC, "wrong magic");
      
      return p->type;
    }
  else
    ddnsd_send_err(0, DDNS_T_EUNKNOWNUID, "unknown user");
  
  return 0;
}

/* take an username and create a filename from it by 
   prepending datadir, return it in \0 terminated tmpname */
void create_datafilename(stralloc *tmpname, stralloc *username)
{
  /* create the filename in our datastructure */
  if(!stralloc_copys(tmpname, datadir)) nomem();
  if(!stralloc_cats(tmpname, "/")) nomem();
  if(!stralloc_cat(tmpname, username)) nomem();
  if(!stralloc_0(tmpname)) nomem();
}

/* handle a setentryrequest */
void ddnsd_setentry( struct ddnsrequest *p)
{
  struct ddnsreply r;
  struct stat st;
  char host[64] = {0};
  int loop = 0;
  int fd = 0;
  stralloc tmpname = {0};
  stralloc err = {0};
  stralloc finname = {0};
  char outbuf[BUFFER_OUTSIZE];
  char strnum[FMT_ULONG];
  char strip[IP4_FMT];
  buffer ssout;
  
  /* create a temporary name */
  host[0] = 0;
  gethostname(host,sizeof(host));
  for (loop = 0;;++loop)
    {
      if(!stralloc_copys(&tmpname, "tmp/")) nomem();
      if(!stralloc_catulong0(&tmpname, now(),0)) nomem(); 
      if(!stralloc_cats(&tmpname, ".")) nomem(); 
      if(!stralloc_catulong0(&tmpname, getpid(), 0)) nomem(); 
      if(!stralloc_cats(&tmpname, ".")) nomem(); 
      if(!stralloc_cats(&tmpname, host)) nomem(); 
      if(!stralloc_cats(&tmpname, "-")) nomem(); 
      if(!stralloc_cat(&tmpname, &username)) nomem(); 
      if(!stralloc_0(&tmpname)) nomem();
      
      if (stat(tmpname.s,&st) == -1) 
	if (errno == error_noent) 
	  break;
      /* really should never get to this point */
      if (loop == 2) 
	_exit(1); /* XXX: logging */
      sleep(1);
    }
  
  /* create the final name */
  create_datafilename(&finname, &username);
  
  /* open tmpfile and write to it */
  fd = open_excl(tmpname.s);
  
  if(fd == -1)
    ddnsd_send_err_sys(p->uid, tmpname.s);
  
  /* XXX: we need checks if the user is comming from the claimed ip */
  
  /* write data to file */
  buffer_init(&ssout,write, fd, outbuf, sizeof outbuf);
  buffer_puts(&ssout, "=");
  buffer_put(&ssout, strip, ip4_fmt(strip, (char *) &p->ip4));
  buffer_puts(&ssout, ":");
  buffer_put(&ssout, strnum, fmt_xlong(strnum, p->uid));
  buffer_puts(&ssout, "\n");
  buffer_flush(&ssout); 
  /* XXX: LOC and IPv6 are missing */
  
  close(fd);
  
  /* test if the name we are asked to move to is already there */
  if (stat(finname.s, &st) == -1) 
    {
      if(errno != ENOENT)
	ddnsd_send_err_sys(p->uid, finname.s);
	/* else: everything is fine, the File doesn't exist */
    }
  else
    ddnsd_send_err(p->uid, DDNS_T_EALLREADYUSED, "allready registered");

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
      if(!stralloc_copys(&err, "can't rename ")) nomem();
      if(!stralloc_cats(&err, tmpname.s)) nomem();
      if(!stralloc_cats(&err, " to ")) nomem();
      if(!stralloc_cat(&err, &finname)) nomem();
      ddnsd_send_err_sys(p->uid, err.s);
    }
  
  ddnsd_log(p->uid, "setting entry");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  /* there should be some more intelligence in setting leasetime */
  r.leasetime.x = ttl;	 
  ddnsd_send(&r);  
}

/* handle a renewentry request by updating th ctime of the file */
void ddnsd_renewentry( struct ddnsrequest *p)
{
  struct stat st = {0};
  struct utimbuf ut;
  struct ddnsreply r;
  stralloc tmpname = {0};
  
  /* create the filename in our datastructure */
  create_datafilename(&tmpname, &username);

  if(stat(tmpname.s, &st) == -1)
    {
      if(errno == ENOENT)
	  // File not found
	  ddnsd_send_err(p->uid, DDNS_T_ENOENTRYUSED, "entry can't be stat()ed");
      else
	ddnsd_send_err_sys(p->uid, tmpname.s);
    }
  
  /* update ctime */
  ut.actime = st.st_atime;
  ut.modtime = st.st_mtime;

  if(utime(tmpname.s, &ut) == -1)
    ddnsd_send_err_sys(p->uid, tmpname.s);
    
  ddnsd_log(p->uid, "renewing entry");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime.x = ttl;
  ddnsd_send(&r);  
}

/* the user requested to delete his entry from the dns */
void ddnsd_killentry( struct ddnsrequest *p)
{
  struct ddnsreply r;
  stralloc tmpname = {0};
  
  /* create the filename in our datastructure */
  create_datafilename(&tmpname, &username);
  
  if(unlink(tmpname.s) == -1)
    {
      if(errno == ENOENT)
	// File not found
	ddnsd_send_err(p->uid, DDNS_T_ENOENTRYUSED, "entry can't be killed");
      else
	ddnsd_send_err_sys(p->uid, tmpname.s);
    }
  
  ddnsd_log(p->uid, "killing entry");
  
  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime.x = (uint64) (randomMT() & (uint64)((uint64)randomMT() << 32));
  ddnsd_send(&r);  
}

void usage(void)
{
  ddnsd_send_err_sys(0, "ddnsd: usage: ddnsd /datadir");
}

int main(int argc, char **argv)
{
  struct ddnsrequest p = { 0 };
  
  /* chroot() to $ROOT and switch to $UID:$GID */
  droproot("ddnsd: ");

  umask(024);

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
  
  datadir = argv[1];
  if (!datadir) usage();

  /* now: 
     remotehost is the remote hostname or "unknown" 
     remoteinfo is some ident string or "-"
     remoteip is the remote ipadress or "unknown" (?)
  */

  /* read one ddns packet from stdin which should be 
     connected to the tcpstream */
  ddnsd_recive(&p);
  
  switch(p.type)
    {
    case DDNS_T_SETENTRY:
      ddnsd_setentry(&p);
      break;
    case DDNS_T_RENEWENTRY:
      ddnsd_renewentry(&p);
      break;
    case DDNS_T_KILLENTRY:       
      ddnsd_killentry(&p);
      break;
    default:
      dump_packet(&p, "unsupported type/command");
      ddnsd_send_err(p.uid, DDNS_T_EUNSUPPTYPE, "unsupported type/command");
    }
  
  return 0;
}
