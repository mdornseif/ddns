#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "ndelay.h"
#include "socket.h"
#include "haveip6.h"
#include "error.h"
#include "readwrite.h"

int socket_udp6(void)
{
#ifdef LIBC_HAS_IP6
  int s;

  s = socket(PF_INET6,SOCK_DGRAM,0);
  if (s == -1) {
    if (errno == EINVAL) {
      s=socket(AF_INET,SOCK_DGRAM,0);
      noipv6=1;
      if (s==-1) return -1;
    } else
    return -1;
  }
  if (ndelay_on(s) == -1) { close(s); return -1; }
  return s;
#else
  return socket_udp();
#endif
}
