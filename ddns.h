/* $Id: ddns.h,v 1.11 2000/08/02 20:13:22 drt Exp $
 *
 * $Log: ddns.h,v $
 * Revision 1.11  2000/08/02 20:13:22  drt
 * -V
 *
 * Revision 1.10  2000/07/31 19:15:56  drt
 * ddns-file(5) format changed
 * a lot of restructuring
 *
 * Revision 1.9  2000/07/29 21:45:53  drt
 * added stralloc_free()
 * changed uint32 reserved[] to char reserved[]
 *
 * Revision 1.8  2000/07/14 15:32:51  drt
 * The timestamp is checked now in ddnsd and an error
 * is returned if there is more than 4000s fuzz.
 * This needs further checking.
 *
 * Revision 1.7  2000/07/07 13:32:47  drt
 * ddnsd and ddnsc now basically work as they should and are in
 * a usable state. The protocol is changed a little bit to lessen
 * problems with alignmed and so on.
 * Tested on IA32 and PPC.
 * Both are still missing support for LOC.
 *
 * Revision 1.6  2000/05/02 22:50:19  drt
 * Renumbered DDNS_T
 *
 * Revision 1.5  2000/05/01 11:47:18  drt
 * ttl/leasetime coms now from data.cdb
 *
 * Revision 1.4  2000/04/30 14:56:57  drt
 * cleand up usage of djb stuff
 *
 * Revision 1.3  2000/04/27 12:12:40  drt
 * Changed data packets to 32+512 Bits size, added functionality to
 * transport IPv6 adresses and LOC records.
 *
 * Revision 1.2  2000/04/24 16:27:26  drt
 * added ENOENTRYUSED
 *
 * Revision 1.1.1.1  2000/04/19 07:01:48  drt
 * initial ddns version
 *
 */

#ifndef DDNS_H
#define DDNS_H

#include "uint16.h"
#include "uint32.h"
#include "uint64.h"
#include "tai.h"
#include "taia.h"
#include "stralloc.h"

#ifndef NULL
 #define NULL 0
#endif

#define DDNS_MAGIC             0xc0decafe
#define DDNS_T_ACK             1  /* generic acl */
#define DDNS_T_NAK             2  /* generic nak (unused) */
#define DDNS_T_SETENTRY        3               
#define DDNS_T_RENEWENTRY      4
#define DDNS_T_KILLENTRY       5
#define DDNS_T_ESERVINT        6  /* internal server error */
#define DDNS_T_EPROTERROR      7  /* generic protocol error */
#define DDNS_T_EWRONGMAGIC     8  /* wrong magic token */
#define DDNS_T_ECANTDECRYPT    9  /* error while decrypting */
#define DDNS_T_ETIMESWRONG   0xa  /* timestamp in packet is considered to old or in the future */
#define DDNS_T_EUNKNOWNUID   0xb  /* client sent unknown uid */
#define DDNS_T_EALLREADYUSED 0xc  /* client requsted to set something which is already set */
#define DDNS_T_ENOENTRYUSED  0xd  /* client requsted to renew/kill something which is not set */

#define DDNSREQUESTSIZE 68
#define DDNSREPLYSIZE 68

#define NUMFIELDS 10

struct ddnsrequest {
  uint32 uid;
  uint16 type;                             //  16
  uint16 random1;                          //  32
  uint32 magic;                            //  64
  unsigned char ip4[4];                    // 224
  unsigned char ip6[16];                   // 352
  unsigned char reserved1;                 // 360         
  unsigned char loc_size;                  // 368
  unsigned char loc_hpre;                  // 376
  unsigned char loc_vpre;                  // 384
  uint32 loc_lat;                          // 416
  uint32 loc_long;                         // 448
  uint32 loc_alt;                          // 480
  uint16 random2;                          // 496
  uint16 reserved2;                        // 512
   /* taia and alignment of compilers tend to mix up very much, so it is moved to the end */
  struct taia timestamp;                   // 192
};

struct ddnsreply {
  uint32 uid;
  uint16 type;                             //  16
  uint16 random1;                          //  32
  uint32 magic;                            //  64
  uint32 leasetime;                        // 224
  char reserved[36];                     // 512
  /* here we will add a "you must have lost a packet"-field
     for udp as a transport
     uint32 already_set;
  */
  struct taia timestamp;                   // 192
 };



#define VERSIONINFO if(argc > 0 && argv[1][0] == '-' && argv[1][1] == 'V') \
   { \
     buffer_puts(buffer_2, ARGV0 "Version "); \
     buffer_puts(buffer_2, VERSION); \
     buffer_putsflush(buffer_2, " (Build: "); \
     buffer_puts(buffer_2, __DATE__); \
     buffer_putsflush(buffer_2, ")\n"); \
     _exit(0); \
   }

extern void stralloc_free(stralloc *sa);  
extern void stralloc_cleanlineend(stralloc *line);
extern int fieldsep(stralloc f[], int numfields, stralloc *line, char sepchar);
extern void ddns_parseline(char *s, uint32 *uid, char *ip4, char *ip6, char *loc);
extern void openandwrite(char *filename, stralloc *sa);
extern int write_fifodir(char *dirname, stralloc *sa, void (*openandwrite)(char *filename, stralloc *sa));


/* definition foe standard C functions */
/* I dont want to include headers which include 
   headers which include headers which include stuff 
   I don't understans */
extern int close(int);
extern int chdir(const char *);
extern int chroot(const char *);
extern int unlink(const char *);

#endif
