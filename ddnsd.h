/* $Id: ddnsd.h,v 1.2 2000/11/21 19:38:12 drt Exp $
 *  -- drt@un.bewaff.net
 */

/* ddnsd.c */
extern void die_nomem(void);
void ddnsd_fifowrite(stralloc *sa);
void create_datafilename(stralloc *tmpname, stralloc *username);

/* ddnsd_log.c */
extern void ddnsd_log(uint32 uid, char *str);
extern void ddnsd_send_err(uint32 uid, uint16 errtype, char *errstr);
extern void ddnsd_send_err_sys(uint32 uid, char *errstr);

/* ddnsd_net.c */
extern void ddnsd_send(struct ddnsreply *p);
extern int ddnsd_recive(struct ddnsrequest *p, uint32 *ttl, stralloc *username);

/* ddnsd_setentry.c */
extern void ddnsd_setentry(struct ddnsrequest *, uint32 *ttl, stralloc *username);

/* ddnsd_renewentry.c */
extern void ddnsd_renewentry(struct ddnsrequest *, uint32 *ttl, stralloc *username);

/* ddnsd_killentry.c */
extern void ddnsd_killentry(struct ddnsrequest *, uint32 *ttl, stralloc *username);
