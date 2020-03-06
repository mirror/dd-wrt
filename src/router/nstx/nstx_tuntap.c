#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sysexits.h>

#ifdef linux
#include <linux/if_tun.h>
#define TUNDEV "/dev/net/tun"
#else
#	include <net/if_tun.h>
#	if __FreeBSD_version < 500000
#		define TUNDEV "/dev/tun2"
#	else
#		define TUNDEV "/dev/tun"
#	endif
#endif

#include "nstxfun.h"

#define MAXPKT 2000

#define TAPDEV "/dev/tap0"

int tfd = -1, nfd = -1;
static char dev[IFNAMSIZ+1];

static int tun_alloc (const char *path);
#ifdef linux
static int tap_alloc (const char *path);
#endif

void
open_tuntap(const char *device)
{
   int	tunerr;
#ifdef linux
   int	taperr;
#endif
   
   fprintf(stderr, "Opening tun/tap-device... ");
   if ((tunerr = tun_alloc(device ? device : TUNDEV))
#ifdef linux
	&& (taperr = tap_alloc(device ? device : TAPDEV))
#endif
   ) {
      fprintf(stderr, "failed!\n"
	              "Diagnostics:\nTun ("TUNDEV"): ");
      switch (tunerr) {
       case EPERM:
	 fprintf(stderr, "Permission denied. You usually have to "
		         "be root to use nstx.\n");
	 break;
       case ENOENT:
	 fprintf(stderr, TUNDEV " not found. Please create /dev/net/ and\n"
		 "     mknod /dev/net/tun c 10 200 to use the tun-device\n");
	 break;
       case ENODEV:
	 fprintf(stderr, "Device not available. Make sure you have "
		 "kernel-support\n     for the tun-device. Under linux, you "
		 "need tun.o (Universal tun/tap-device)\n");
	 break;
       default:
	 perror("Unexpected error");
	 break;
      }
      fprintf(stderr, "Tap ("TAPDEV"):\n(only available under linux)\n");
#ifdef linux
      switch (taperr) {
       case EPERM:
	 fprintf(stderr, "Permission denied. You generally have to "
		 "be root to use nstx.\n");
	 break;
       case ENOENT:
	 fprintf(stderr, TAPDEV " not found. Please\n"
		 "     mknod /dev/tap0 c 36 16 to use the tap-device\n");
	 break;
       case ENODEV:
	 fprintf(stderr, "Device not available. Make sure you have kernel-support\n"
		 "     for the tap-device. Under linux, you need netlink_dev.o and ethertap.o\n");
	 break;
       default:
	 fprintf(stderr, "Unexpected error: %s\n", strerror(taperr));
	 break;
      }
#endif
      exit(EXIT_FAILURE);
   }
   
   fprintf(stderr, "using device %s\n"
	  "Please configure this device appropriately (IP, routes, etc.)\n", dev);
}

int
tun_alloc (const char *path) 
{
#ifdef linux
   struct ifreq ifr;
#else
   struct stat st;
#endif
 
   if ((tfd = open(path, O_RDWR)) < 0)
     return errno;

#ifdef linux
   memset(&ifr, 0, sizeof(ifr));
   
   ifr.ifr_flags = IFF_TUN|IFF_NO_PI;
   
   if (ioctl(tfd, TUNSETIFF, (void *) &ifr) < 0)
     {
	close(tfd);
	tfd = -1;
	return errno;
     }
   strlcpy(dev, ifr.ifr_name, IFNAMSIZ);
#else
   fstat(tfd, &st);
   strncpy(dev, devname(st.st_rdev, S_IFCHR), IFNAMSIZ+1);
#endif
   
   return 0;
}


#ifdef linux
int
tap_alloc(const char *path)
{
   char *ptr;
   
   if ((tfd = open(path, O_RDWR)) < 0)
     return errno;
   
   if ((ptr = strrchr(path, '/')))
     strlcpy(dev, ptr+1, IFNAMSIZ);
   else
     strlcpy(dev, path, IFNAMSIZ);
   
   return 0;
}
#endif

void
open_ns(const char *ip)
{
	struct sockaddr_in sock = { 0 };
   
	fprintf(stderr, "Opening nameserver-socket... ");
	if ((nfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("failed!\nUnexpected error creating socket");
		exit(EX_OSERR);
	}
	sock.sin_family = AF_INET;
	sock.sin_port = htons(53);
	sock.sin_addr.s_addr = inet_addr(ip);
	if (connect(nfd, (struct sockaddr *)&sock,
	    sizeof(struct sockaddr_in))) {
		perror("connect");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Using nameserver %s\n", ip);
}

void
open_ns_bind(in_addr_t bindip)
{
	struct sockaddr_in sock = { 0 };
   
	fprintf(stderr, "Opening nameserver-socket... ");
	if ((nfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("failed!\nUnexpected error creating socket");
		exit(EX_OSERR);
	}
	sock.sin_family = AF_INET;
	sock.sin_port = htons(53);
 
	sock.sin_addr.s_addr = bindip;
	if (bind (nfd, (struct sockaddr *) &sock, sizeof(struct sockaddr_in))) {
	   fprintf(stderr, "failed!\n");
	   switch (errno) {
	    case EADDRINUSE:
	      fprintf(stderr, "Address is in use, please kill other processes "
		      "listening on UDP-Port 53 on %s\n",
		      bindip == INADDR_ANY ?
			"all local IPs" : "the specified IP");
	      break;
	    case EACCES:
	    case EPERM:
	      fprintf(stderr, "Permission denied binding port 53. You generally "
		      "have to be root to bind privileged ports.\n");
	      break;
	    default:
	      fprintf(stderr, "Unexpected error: bind: %s\n", strerror(errno));
	      break;
	   }
	   exit(EXIT_FAILURE);
	}
	fprintf(stderr, "listening on 53/UDP\n");
}

struct nstxmsg *nstx_select (int timeout)
{
   unsigned peerlen;
   fd_set set;
   struct timeval tv;
   static struct nstxmsg *ret = NULL;
   
   FD_ZERO(&set);
   if (nfd > 0)
     FD_SET(nfd, &set);
   if (tfd > 0)
     FD_SET(tfd, &set);
   
   tv.tv_sec = timeout;
   tv.tv_usec = 0;
   
   if (timeout < 0)
     select(((tfd>nfd)?tfd:nfd)+1, &set, NULL, NULL, NULL);
   else
     select(((tfd>nfd)?tfd:nfd)+1, &set, NULL, NULL, &tv);
   
   if (!ret)
     ret = malloc(sizeof(struct nstxmsg));
   if (FD_ISSET(tfd, &set)) {
      ret->len = read(tfd, ret->data, MAXPKT);
      if (ret->len > 0) {
	 ret->src = FROMTUN;
	 return ret;
      }
   }
   if (FD_ISSET(nfd, &set)) {
      peerlen = sizeof(struct sockaddr_in);
      ret->len = recvfrom(nfd, ret->data, MAXPKT, 0,
			  (struct sockaddr *) &ret->peer, &peerlen);
      if (ret->len > 0) {
#ifdef WITH_PKTDUMP
	 pktdump("/tmp/nstx/pkt.", *((unsigned short *)ret->data),
		 ret->data, ret->len, 0);
#endif
	 ret->src = FROMNS;
	 return ret;
      }
   }

   return NULL;
}

void
sendtun(const char *data, size_t len)
{
//   printf("Sent len %d, csum %d\n", len, checksum(data, len));
   write(tfd, data, len);
}

void
sendns (const char *data, size_t len, const struct sockaddr *peer)
{
   if (peer)
     sendto(nfd, data, len, 0, peer,
	    sizeof(struct sockaddr_in));
   else
     send(nfd, data, len, 0);
#ifdef WITH_PKTDUMP
   pktdump("/tmp/nstx/pkt.", *((unsigned short *)data), data, len, 1);
#endif
}
