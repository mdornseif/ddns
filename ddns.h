/* $Id: ddns.h,v 1.1 2000/04/19 07:01:48 drt Exp $
 *
 * $Log: ddns.h,v $
 * Revision 1.1  2000/04/19 07:01:48  drt
 * Initial revision
 *
 */

#include "djblib/uint16.h"
#include "djblib/uint32.h"
#include "djblib/uint64.h"
#include "djblib/tai.h"
#include "djblib/taia.h"

#define NULL 0

#define DDNS_MAGIC             0xc0dedec0
#define DDNS_T_ACK             1               /* generic acl */
#define DDNS_T_NAK             2               /* generic nak */
#define DDNS_T_SETENTRY        3               
#define DDNS_T_RENEWENTRY      4
#define DDNS_T_KILLENTRY       5
#define DDNS_T_EPROTERROR      6               /* generic protocoll error */
#define DDNS_T_EWRONGMAGIC     7               /* wrong magic token */
#define DDNS_T_ECANTDECRYPT    8               /* can't decrypt packet */
#define DDNS_T_EALLREADYUSED   9               /* client requsted to set something which is already set */
#define DDNS_T_ESERVINT        10              /* internal server error */
#define DDNS_T_EUNKNOWNUID     11              /* client sent unknown uid */
#define DDNS_T_EUNSUPPTYPE     12              /* unknown or unsupported type in this situation */

struct ddnsrequest {
  uint32 uid;
  uint16 type;                             //  16
  uint16 random;                           //  32
  uint32 magic;                            //  64
  struct taia timestamp;                   // 192
  uint32 ip;                               // 224  
  uint32 reserved;                         // 256
};

struct ddnsreply {
  uint32 uid;
  uint16 type;                             //  16
  uint16 random;                           //  32
  uint32 magic;                            //  64
  struct taia timestamp;                   // 192
  struct tai leasetime;                    // 256
};

