/* $Id: ddnsd_net.c,v 1.1 2000/07/31 19:03:17 drt Exp $
 *   --drt@ailis.de
 *
 * network communications for ddnsd
 * 
 * (K)allisti
 * 
 * you might find more Information at http://rc23.cx/
 * 
 * $Log: ddnsd_net.c,v $
 * Revision 1.1  2000/07/31 19:03:17  drt
 * initial revision
 *
 */

#include "buffer.h"
#include "byte.h"
#include "cdb.h"
#include "error.h"
#include "fmt.h"
#include "ip4.h"
#include "ip6.h"
#include "now.h"
#include "open.h"
//#include "readwrite.h"
#include "stralloc.h"
#include "strerr.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

#include "mt19937.h"
#include "rijndael.h"
#include "ddns_pack.h"

#include "ddns.h"
#include "ddnsd.h"

static char rcsid[] = "$Id: ddnsd_net.c,v 1.1 2000/07/31 19:03:17 drt Exp $";

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


/* find a user in the cdb */
/* returns 1 when found, 0 when unknown user */
static int ddnsd_find_user(uint32 uid, stralloc *sa)
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
int ddnsd_recive(struct ddnsrequest *p, uint32 *ttl, stralloc *username)
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
      uint32_unpack(&data.s[32], ttl);
      if(!stralloc_copyb(username, &data.s[36], data.len-36)) die_nomem();
      
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
