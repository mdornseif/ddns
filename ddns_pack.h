/* $Id: ddns_pack.h,v 1.2 2000/11/21 19:28:22 drt Exp $
 *  -- drt@un.bewaff.net
 *
 * converts ddnsrequest and ddnsreply to machine 
 * independent bytestreams and back
 *
 * There is no such thing like interlectual property
 * 
 * You might find more Information at http://rc23.cx/
 */

#include "ddns.h"

int ddnsrequest_pack_big(stralloc *sa, struct ddnsrequest *r);
int ddnsrequest_unpack_big(char *buf, struct ddnsrequest *r);
int ddnsreply_pack_big(stralloc *sa, struct ddnsreply *r);
int ddnsreply_unpack_big(char *buf, struct ddnsreply *r);
