/* $Id: ddns.h,v 1.6 2000/05/02 22:50:19 drt Exp $
 *
 * $Log: ddns.h,v $
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

#include "uint16.h"
#include "uint32.h"
#include "uint64.h"
#include "tai.h"
#include "taia.h"

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
#define DDNS_T_ECANTDECRYPT    9  /* can't decrypt packet */
#define DDNS_T_EUNSUPPTYPE   0xa  /* unknown or unsupported type in this situation */
#define DDNS_T_EUNKNOWNUID   0xb  /* client sent unknown uid */
#define DDNS_T_EALLREADYUSED 0xc  /* client requsted to set something which is already set */
#define DDNS_T_ENOENTRYUSED  0xd  /* client requsted to renew/kill something which is not set */

struct ddnsrequest {
  uint32 uid;
  uint16 type;                             //  16
  uint16 random1;                          //  32
  uint32 magic;                            //  64
  struct taia timestamp;                   // 192
  uint32 ip4;                              // 224
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
};

struct ddnsreply {
  uint32 uid;
  uint16 type;                             //  16
  uint16 random1;                          //  32
  uint32 magic;                            //  64
  struct taia timestamp;                   // 192
  struct tai leasetime;                    // 256
  /* here we will add a "you must have lost a packet"-field
     for udp as a transport
     uint32 already_set;
  */
  uint32 reserved[8];                      // 512
};

