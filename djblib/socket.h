#ifndef SOCKET_H
#define SOCKET_H

#include "uint16.h"

extern int socket_tcp(void);
extern int socket_udp(void);
extern int socket_tcp6(void);
extern int socket_udp6(void);

extern int socket_connect4(int s,const char *ip,uint16);
extern int socket_connect6(int s,const char *ip,uint16);
extern int socket_connected(int s);
extern int socket_bind4(int s,const char *ip,uint16);
extern int socket_bind4_reuse(int s,const char *ip,uint16);
extern int socket_bind6(int s,const char *ip,uint16);
extern int socket_bind6_reuse(int s,const char *ip,uint16);
extern int socket_listen(int s,unsigned int backlog);
extern int socket_accept4(int s,char *ip,uint16 *);
extern int socket_accept6(int s,char *ip,uint16 *);
extern int socket_recv4(int s,char *buf,unsigned int len,char *ip,uint16 *port);
extern int socket_recv6(int s,char *buf,unsigned int len,char *ip,uint16 *port);
extern int socket_send4(int s,const char *buf,unsigned int len,const char *ip,uint16 port);
extern int socket_send6(int s,const char *buf,unsigned int len,const char *ip,uint16 port);
extern int socket_local4(int s,char *ip,uint16 *port);
extern int socket_local6(int s,char *ip,uint16 *port);
extern int socket_remote4(int s,char *ip,uint16 *port);
extern int socket_remote6(int s,char *ip,uint16 *port);

/* enable sending udp packets to the broadcast address */
extern int socket_broadcast(int s);
/* join a multicast group on the given interface */
extern int socket_mcjoin4(int s,const char *groupip,const char *interface);
extern int socket_mcjoin6(int s,const char *groupip,int);
/* leave a multicast group on the given interface */
extern int socket_mcleave4(int s,const char *groupip);
extern int socket_mcleave6(int s,const char *groupip);
/* set multicast TTL/hop count for outgoing packets */
extern int socket_mcttl4(int s,char hops);
extern int socket_mcttl6(int s,char hops);
/* enable multicast loopback */
extern int socket_mcloop4(int s,char hops);
extern int socket_mcloop6(int s,char hops);

extern void socket_tryreservein(int s,int size);

extern int noipv6;

#endif
