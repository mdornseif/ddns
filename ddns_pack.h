/* $Id: ddns_pack.h,v 1.1 2000/07/29 21:12:50 drt Exp $
 *  -- drt@ailis.de
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
