/* $Id: ddnsd.c,v 1.19 2000/07/29 21:41:49 drt Exp $
 *   --drt@ailis.de
 *
 * server for ddns - this file is to long
 * 
 * (K)allisti
 *
 * $Log: ddnsd.c,v $
 * Revision 1.19  2000/07/29 21:41:49  drt
 * creation of network packets is done in ddns_pack()
 * nomem() renamed to die_nomem()
 * first implementation of writing to a fifo
 * fiddeled with stat() which seems not to work on my
 * powerbook - the glibc/kernel interfaces for stat()
 * seem to be a real mess - glibc seems to be a real mess
 * thrown away dump_packet()
 *
 * Revision 1.18  2000/07/17 21:45:24  drt
 * ddnsd and ddns-cleand now refuse to run as root
 *
 * Revision 1.17  2000/07/14 15:32:51  drt
 * The timestamp is checked now in ddnsd and an error
 * is returned if there is more than 4000s fuzz.
 * This needs further checking.
 *
 * Revision 1.16  2000/07/14 14:49:37  drt
 * ddnsd ignires IP-adresses which are set to 0
 *
 * Revision 1.15  2000/07/13 18:20:47  drt
 * everything supports now DNS LOC
 *
 * Revision 1.14  2000/07/12 11:46:01  drt
 * checking of random1 == random2 to keep
 * a attacker from exchanging single blocks
 *
 * Revision 1.13  2000/07/07 13:32:47  drt
 * ddnsd and ddnsc now basically work as they should and are in
 * a usable state. The protocol is changed a little bit to lessen
 * problems with alignmed and so on.
 * Tested on IA32 and PPC.
 * Both are still missing support for LOC.
 *
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
#include "byte.h"
#include "cdb.h"
#include "droprootordie.h"
#include "env.h"
#include "error.h"
#include "fmt.h"
#include "ip4.h"
#include "ip6.h"
#include "now.h"
#include "open.h"
#include "readwrite.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

#include "mt19937.h"
#include "rijndael.h"
#include "iso2txt.h"
#include "ddns_pack.h"

#include "ddns.h"

static char rcsid[] = "$Id: ddnsd.c,v 1.19 2000/07/29 21:41:49 drt Exp $";

static char *datadir;

static uint32 ttl;

static unsigned char *remotehost, *remoteinfo, *remoteip, *remoteport;
static stralloc username = {0};

void die_nomem(void)
{
  strerr_die1sys(111, "ddnsd: fatal: help - no memory ");
}

/* fill p with random, timestamp, magic, encrypt it and send it */
void ddnsd_send(struct ddnsreply *p)
{
  stralloc tmp = { 0 };
  
  /* and get it into network byte order */
  p->magic = DDNS_MAGIC;
  p->random1 = randomMT() & 0xffff;
  taia_now(&p->timestamp);
  /* some unused bytes are filled with random data to make cryptoanalysis harder */
  p->reserved[0] = randomMT();
  p->reserved[4] = randomMT();
  p->reserved[8] = randomMT();
  p->reserved[12] = randomMT();
  p->reserved[16] = randomMT();
  p->reserved[20] = randomMT();
  p->reserved[24] = randomMT();
  p->reserved[28] = randomMT();

  ddnsreply_pack_big(&tmp, p);

  /* encrypt with rijndael, key shedule was already set up when reciving data */
  rijndaelEncrypt(tmp.s + 4);
  rijndaelEncrypt(tmp.s + 36);

  buffer_put(buffer_1, tmp.s, DDNSREPLYSIZE);
  buffer_flush(buffer_1); 
}

void ddnsd_log(uint32 uid, char *str)
{
  char strnum[FMT_ULONG];
  
  /* Do logging */
  buffer_puts(buffer_2, "ddnsd ");
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
  buffer_flush(buffer_2);
}

/* sends an error packet to the client and logs an error */
void ddnsd_send_err(uint32 uid, uint16 errtype, char *errstr)
{
  struct ddnsreply p;
  
  p.type = errtype;
  p.uid = uid;
  
  p.leasetime = 0;
  
  ddnsd_log(uid, errstr);
  buffer_putsflush(buffer_2, "\n");

  ddnsd_send(&p);
  
  exit(111);
}

/* sends an error packet to the client and logs an error 
   including a system error message */ 
void ddnsd_send_err_sys(uint32 uid, char *errstr)
{
  stralloc err = {0};

  if(!stralloc_copys(&err, errstr)) die_nomem();
  if(!stralloc_cats(&err, ": ")) die_nomem();
  if(!stralloc_cats(&err, error_str(errno))) die_nomem();
  if(!stralloc_0(&err)) die_nomem();
  ddnsd_send_err(uid, DDNS_T_ESERVINT, err.s);
}


/* find a user in the cdb */
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
  /* uid in cdb is in intel byte order */ 
  uint32_pack(key, uid);
  r = cdb_find(&c, key, 4); 
  if (r > 0)
    {
      /* read data */
      if(!stralloc_ready(sa, cdb_datalen(&c))) die_nomem();
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


/* read, decrypt and decode packet */
int ddnsd_recive(struct ddnsrequest *p)
{
  int r;
  char *key;
  char tmp[DDNSREQUESTSIZE];
  struct taia now;
  struct taia deadline;
  stralloc data = {0};
  
  /* read using a 120 second timeout */
  r = timeoutread(120, 0, tmp, DDNSREQUESTSIZE);
  if(r != DDNSREQUESTSIZE) 
    ddnsd_send_err(p->uid, DDNS_T_EPROTERROR, "wrong packetsize or timeout");
  
  /* bring the uid in host byteorder */
  uint32_unpack_big(tmp, &p->uid);

  if(ddnsd_find_user(p->uid, &data) == 1)
    {
      key = data.s;
      // TTL and username from cdb
      uint32_unpack(&data.s[32], &ttl);
      if(!stralloc_copyb(&username, &data.s[36], data.len-36)) die_nomem();
      
      /* initialize rijndael with 256 bit blocksize and 256 bit keysize */
      rijndaelKeySched(8, 8, key);
      
      /* decrypt two blocks ( = 512 bits = one ddnsd request) with rijndael */
      rijndaelDecrypt(tmp + 4);
      rijndaelDecrypt(tmp + 36);

      ddnsrequest_unpack_big(tmp + 4, p);
 
      /* check for the right magic */
      if(p->magic != DDNS_MAGIC)
	/* propably decryption didn't suceed */
	ddnsd_send_err(p->uid, DDNS_T_EWRONGMAGIC, "wrong magic");

      /* check for random1 and random2 beeing equal */
      if(p->random1 != p->random2)
	/* propably decryption didn't suceed */
 	ddnsd_send_err(p->uid, DDNS_T_ECANTDECRYPT, "alert: someone tampered with the request");
      
      /* check for timestamp beeing sane */
      /* we use a huge window of legal timestams here
         because clients clocks are screwed up to often */
      taia_now(&now);
      taia_uint(&deadline, 4000);
      taia_sub(&deadline, &now, &deadline);     
      if (taia_less(&p->timestamp, &deadline))
	ddnsd_send_err(p->uid, DDNS_T_ETIMESWRONG, "warning: timestamp wrong (to old)");
      taia_uint(&deadline, 4000);
      taia_add(&deadline, &now, &deadline);     
      if (taia_less(&deadline, &p->timestamp))
	ddnsd_send_err(p->uid, DDNS_T_ETIMESWRONG, "warning: timestamp wrong (to new)");
      // XXX: this needs checking

      /* everything is fine */
      return p->type;
    }
  else
    ddnsd_send_err(p->uid, DDNS_T_EUNKNOWNUID, "unknown user");
  /* error condition */
  return 0;
}


void ddnsd_fifowrite(char m, char *line, int len)
{
  int fdfifo;

  // XXX we should do this *after* we have finished client communication - should we?
  // XXX opening and closing for every line isn't strictly effective

  /* we need this to keep the fifo from beeing closed */
  fdfifo = open_write(ACLWFIFONAME);
  if (fdfifo == -1)
    strerr_warn3("ddnsd: unable to open for write ", ACLWFIFONAME, " ", &strerr_sys);

  if((timeoutwrite(5, fdfifo, &m, 1) != 1) 
     || (timeoutwrite(5, fdfifo, line, len) != len))
    strerr_warn3("can't write to fifo ", ACLWFIFONAME, " ", &strerr_sys);
  
  /* add newline if needed */
  if(line[len] != '\n' && line[len-1] != '\n')
    if(timeoutwrite(5, fdfifo, "\n", 1) != 1)
      strerr_warn3("can't write to fifo ", ACLWFIFONAME, " ", &strerr_sys);
  
  close(fdfifo);
}

/* take an username and create a filename from it by 
   prepending datadir, return it in \0 terminated tmpname */
void create_datafilename(stralloc *tmpname, stralloc *username)
{
  /* create the filename in our datastructure */
  if(!stralloc_copys(tmpname, datadir)) die_nomem();
  if(!stralloc_cats(tmpname, "/")) die_nomem();
  if(!stralloc_cat(tmpname, username)) die_nomem();
  if(!stralloc_0(tmpname)) die_nomem();
}

/* handle a setentryrequest */
void ddnsd_setentry( struct ddnsrequest *p)
{
  struct ddnsreply r;
  struct stat st;
  char host[64] = {0};
  int loop = 0;
  int fd = 0;
  stralloc out = {0};
  stralloc tmpname = {0};
  stralloc err = {0};
  stralloc finname = {0};
  char strnum[FMT_ULONG];
  char strip[IP6_FMT];
  char tb[16];
  
  /* create a temporary name */
  host[0] = 0;
  gethostname(host,sizeof(host));
  for (loop = 0;;++loop)
    {
      if(!stralloc_copys(&tmpname, "tmp/")) die_nomem();
      if(!stralloc_catulong0(&tmpname, now(),0)) die_nomem(); 
      if(!stralloc_cats(&tmpname, ".")) die_nomem(); 
      if(!stralloc_catulong0(&tmpname, getpid(), 0)) die_nomem(); 
      if(!stralloc_cats(&tmpname, ".")) die_nomem(); 
      if(!stralloc_cats(&tmpname, host)) die_nomem(); 
      if(!stralloc_cats(&tmpname, "-")) die_nomem(); 
      if(!stralloc_cat(&tmpname, &username)) die_nomem(); 
      if(!stralloc_0(&tmpname)) die_nomem();

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
  
  /* ip4 */
  if(byte_diff(p->ip4, 4, "\0\0\0\0"))
    {
      stralloc_cats(&out, "=,");
      stralloc_catb(&out, strip, ip4_fmt(strip, (char *) &p->ip4));
      stralloc_cats(&out, ",");
      stralloc_catb(&out, strnum, fmt_ulong(strnum, p->uid));
      stralloc_cats(&out, "\n");

      if(timeoutwrite(60, fd, out.s, out.len) != out.len)
	ddnsd_send_err_sys(p->uid, "couldn't write to disk");
      /* inform others via fifo */
      ddnsd_fifowrite('s', out.s, out.len);
    }

  /* ip6 */
  if(byte_diff(p->ip6, 16, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"))
    {
      stralloc_copys(&out, "6,");
      stralloc_catb(&out, strip, ip6_fmt(strip, (char *) &p->ip6));
      stralloc_cats(&out, ",");
      stralloc_catb(&out, strnum, fmt_ulong(strnum, p->uid));
      stralloc_cats(&out, "\n");
     
      if(timeoutwrite(60, fd, out.s, out.len) != out.len)
	ddnsd_send_err_sys(p->uid, "couldn't write to disk");
      /* inform others via fifo */
      ddnsd_fifowrite('s', out.s, out.len);
    }

  /* LOC */
  stralloc_copys(&out, "L,");
  tb[0] = 0;
  tb[1] = p->loc_size;
  /* XXX: something is mixed up here, fixme */
  tb[3] = p->loc_hpre;
  tb[2] = p->loc_vpre;
  uint32_pack_big(&tb[4], p->loc_lat); 
  uint32_pack_big(&tb[8], p->loc_long); 
  uint32_pack_big(&tb[12], p->loc_alt);  
  iso2txt(tb, 16, &out);
  stralloc_cats(&out, ",");
  stralloc_catb(&out, strnum, fmt_ulong(strnum, p->uid));
  stralloc_cats(&out, "\n");
  
  if(timeoutwrite(60, fd, out.s, out.len) != out.len)
    ddnsd_send_err_sys(p->uid, "couldn't write to disk");
  /* inform others via fifo */
  ddnsd_fifowrite('s', out.s, out.len);
  
  close(fd);
  
  /* test if the name we are asked to move to is already there */
  if (stat(finname.s, &st) == -1) 
    {
      if(errno != ENOENT)
	ddnsd_send_err_sys(p->uid, finname.s);
	/* else: everything is fine, the File doesn't exist */
    }
  else
    // XXX: this error can happpen with other conditions too - fixme
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
      if(!stralloc_copys(&err, "can't rename ")) die_nomem();
      if(!stralloc_cats(&err, tmpname.s)) die_nomem();
      if(!stralloc_cats(&err, " to ")) die_nomem();
      if(!stralloc_cat(&err, &finname)) die_nomem();
      ddnsd_send_err_sys(p->uid, err.s);
    }
  
  /* log this transaction */
  ddnsd_log(p->uid, "setting entry ");
  buffer_puts(buffer_2, "to ");
  buffer_put(buffer_2, strip, ip4_fmt(strip, (char *) &p->ip4));
  buffer_puts(buffer_2, "/");
  buffer_put(buffer_2, strip, ip6_fmt(strip, (char *) &p->ip6));
  buffer_puts(buffer_2, " ttl ");
  buffer_put(buffer_2, strnum, fmt_ulong(strnum, ttl));
  buffer_putsflush(buffer_2, "\n");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  /* there should be some more intelligence in setting leasetime */
  r.leasetime = ttl;
  ddnsd_send(&r);  
}

/* handle a renewentry request by updating th ctime of the file */
void ddnsd_renewentry( struct ddnsrequest *p)
{
  char strnum[FMT_ULONG];
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
  /*  XXX: for some reason this doesn't work
  ut.actime = st.st_atime;
  ut.modtime = st.st_mtime;
  
  if(utime(tmpname.s, &ut) == -1)
    ddnsd_send_err_sys(p->uid, tmpname.s);
  */

  if(utime(tmpname.s, NULL) == -1)
    ddnsd_send_err_sys(p->uid, tmpname.s);
    
  ddnsd_log(p->uid, "renewing entry");
  buffer_puts(buffer_2, " ttl ");
  buffer_put(buffer_2, strnum, fmt_ulong(strnum, ttl));
  buffer_putsflush(buffer_2, "\n");

  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime = ttl;
  ddnsd_send(&r);  

  ddnsd_fifowrite('r', strnum, fmt_ulong(strnum, p->uid)); 
}

/* the user requested to delete his entry from the dns */
void ddnsd_killentry( struct ddnsrequest *p)
{
  char strnum[FMT_ULONG];
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
  
  ddnsd_log(p->uid, "killing entry\n");
  
  /* construct the answer Packet */
  r.type = DDNS_T_ACK;
  r.uid = p->uid;
  r.leasetime = 0;
  ddnsd_send(&r);  
 
  ddnsd_fifowrite('k', strnum, fmt_ulong(strnum, p->uid)); 
}

void usage(void)
{
  ddnsd_send_err_sys(0, "ddnsd: usage: ddnsd /datadir");
}

int main(int argc, char **argv)
{
  struct ddnsrequest p = { 0 };

  /* chroot() to $ROOT and switch to $UID:$GID */
  droprootordie("ddnsd: ");

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
      ddnsd_send_err(p.uid, DDNS_T_EPROTERROR, "unsupported type/command");
    }
  
  return 0;
}
