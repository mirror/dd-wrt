// Copyright (C) 2002, 2003, 2008, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#ifndef H_DHCP_FORWARDER_SRC_SPLINT_COMPAT_H
#define H_DHCP_FORWARDER_SRC_SPLINT_COMPAT_H

#ifdef S_SPLINT_S

  /*@-incondefs@*//*@-isoreserved@*//*@-export@*//*@-cppnames@*//*@-protoparamname@*/
  /*@-declundef@*//*@-fcnuse@*//*@-typeuse@*/
  /*@-redef@*//*@-redecl@*//*@-protoparammatch@*/

typedef int		__socklen_t;
typedef __socklen_t	socklen_t;

typedef /*@integraltype@*/	bool;
/*@constant bool false@*/
/*@constant bool true@*/

typedef unsigned short		u_short;
typedef uint8_t			u_int8_t;
typedef uint16_t		u_int16_t;
typedef uint32_t		u_int32_t;

/*@+allmacros@*//*@-macromatchname@*/
#define __arr
/*@=allmacros@*//*@=macromatchname@*/

/*@constant char const PACKAGE_BUGREPORT[]@*/
/*@constant char const PACKAGE_STRING[]@*/
/*@constant char const CFG_FILENAME[]@*/
/*@constant int IP_PKTINFO@*/
/*@constant size_t PATH_MAX=42@*/
/*@constant size_t IFNAMSIZ@*/


/*@constant int SIOCGIFINDEX@*/
/*@constant int SIOCGIFADDR@*/
/*@constant int SIOCGIFHWADDR@*/
/*@constant int SIOCGIFMTU@*/

typedef uint16_t in_port_t; /* An unsigned integral type of exactly 16 bits. */
typedef uint32_t in_addr_t; /* An unsigned integral type of exactly 32 bits. */

/*@constant int RLIMIT_AS@*/
/*@constant int RLIMIT_LOCKS@*/
/*@constant int RLIMIT_RSS@*/
/*@constant int RLIMIT_NPROC@*/
/*@constant int RLIMIT_MEMLOCK@*/

/*@constant int MSG_CTRUNC@*/

/*@constant size_t ETH_ALEN@*/

/*@constant int AF_INET@*/
/*@constant int AF_INET6@*/
/*@constant int AF_PACKET@*/

/*@constant int IPPROTO_IP@*/
/*@constant int IPPROTO_UDP@*/

/*@constant int SOCK_DGRAM@*/
/*@constant int SOCK_RAW@*/

/*@constant int SOL_SOCKET@*/
/*@constant int SOL_IP@*/

/*@constant int SO_BROADCAST@*/
/*@constant int SO_BINDTODEVICE@*/

/*@constant in_addr_t INADDR_NONE@*/
/*@constant in_addr_t INADDR_ANY@*/
/*@constant in_addr_t INADDR_BROADCAST@*/


/*@constant int ARPHRD_ETHER@*/
/*@constant int ARPHRD_EETHER@*/
/*@constant int ARPHRD_IEEE802@*/

typedef /*@abstract@*/ fd_set;

typedef /*@integraltype@*/		sa_family_t;
typedef /*@unsignedintegraltype@*/	rlim_t;

/*@constant rlim_t RLIM_INFINITY@*/

int /*@alt integraltype@*/MAX(int/*@sef@*/a, int/*@sef@*/b) /*@*/;

struct rlimit {
	rlim_t  rlim_cur;
	rlim_t  rlim_max;
};

struct cmsghdr {
  socklen_t cmsg_len;		/* data byte count, including hdr */
  int cmsg_level;		/* originating protocol */
  int cmsg_type;		/* protocol-specific type */
} ;

size_t	CMSG_SPACE(/*@sef@*/size_t) /*@*/;
/*@exposed@*/ unsigned char *CMSG_DATA (/*@sef@*/ struct cmsghdr *) /*@*/ ;
/*@null@*/ /*@exposed@*/ struct cmsghdr *CMSG_NXTHDR (/*@in@*/struct msghdr *,
						      /*@in@*/struct cmsghdr *) /*@*/ ;
/*@null@*/ /*@exposed@*/ struct cmsghdr *CMSG_FIRSTHDR (/*@in@*/struct msghdr *) /*@*/ ;

struct ifreq {
  union
  {
    char        ifrn_name[IFNAMSIZ];            /* if name, e.g. "en0" */
  } ifr_ifrn;
  union {
    struct sockaddr ifru_addr;
    struct sockaddr ifru_dstaddr;
    struct sockaddr ifru_broadaddr;
    struct sockaddr ifru_netmask;
    struct  sockaddr ifru_hwaddr;
    short ifru_flags;
    int ifru_ivalue;
    int ifru_mtu;
    struct ifmap ifru_map;
    char ifru_slave[IFNAMSIZ];  /* Just fits the size */
    char ifru_newname[IFNAMSIZ];
    char* ifru_data;
  } ifr_ifru;
};

  /*@notfunction@*/
#define ifr_name        ifr_ifrn.ifrn_name
#define ifr_hwaddr      ifr_ifru.ifru_hwaddr
#define ifr_addr        ifr_ifru.ifru_addr
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr
#define ifr_netmask     ifr_ifru.ifru_netmask
#define ifr_flags       ifr_ifru.ifru_flags
#define ifr_metric      ifr_ifru.ifru_ivalue
#define ifr_mtu         ifr_ifru.ifru_mtu
#define ifr_map         ifr_ifru.ifru_map
#define ifr_slave       ifr_ifru.ifru_slave
#define ifr_data        ifr_ifru.ifru_data
#define ifr_ifindex     ifr_ifru.ifru_ivalue
#define ifr_bandwidth   ifr_ifru.ifru_ivalue
#define ifr_qlen        ifr_ifru.ifru_ivalue
#define ifr_newname     ifr_ifru.ifru_newname

struct iphdr
{
    unsigned int ihl:4;
    unsigned int version:4;
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
};

struct udphdr {
  u_int16_t     source;
  u_int16_t     dest;
  u_int16_t     len;
  u_int16_t     check;
};

  /* Internet address. */
struct in_addr {
    uint32_t		s_addr;		/* address in network byte order */
};

struct in6_addr {
    unsigned char	s6_addr[16];	/* IPv6 address */
};

struct sockaddr {
    sa_family_t		sa_family;	/* address family */
    char		sa_data[];	/* variable length */
};

struct sockaddr_in {
    sa_family_t     sin_family; /* address family: AF_INET */
    uint16_t        sin_port;   /* port in network byte order */
    struct in_addr  sin_addr;  /* internet address */
};


struct sockaddr_ll {
  sa_family_t	  sll_family;
  unsigned short  sll_protocol;
  int             sll_ifindex;
  unsigned short  sll_hatype;
  unsigned char   sll_pkttype;
  unsigned char   sll_halen;
  unsigned char   sll_addr[8];
};

struct sockaddr_in6 {
    u_int16_t		sin6_family;/* AF_INET6 */
    u_int16_t		sin6_port;/* port number */
    u_int32_t		sin6_flowinfo;/* IPv6 flow information */
    struct in6_addr	sin6_addr;/* IPv6 address */
    u_int32_t		sin6_scope_id;	/* Scope id (new in 2.4) */
};


struct in_pktinfo {
  int                   ipi_ifindex;
  struct in_addr        ipi_spec_dst;
  struct in_addr        ipi_addr;
};

struct iovec {
    /*@dependent@*/
    void    *iov_base;
    size_t   iov_len;		/*: maxSet(iov_base) == iov_len@ */
};

struct msghdr {
      /*@dependent@*//*@null@*/
    void         * msg_name;     /* optional address */
    socklen_t    msg_namelen;    /*: maxSet (msg_name) >= msg_namelen */
      /*@dependent@*/
    struct iovec * msg_iov;      /* scatter/gather array */
    size_t       msg_iovlen;     /*: maxSet (msg_iov) >= msg_iovlen */
    /*@dependent@*//*@null@*/
    void         * msg_control;  /* ancillary data, see below */
    socklen_t    msg_controllen; /*: maxSet (msg_control) >= msg_controllen */
    int          msg_flags;      /* flags on received message */
};

long int /*@alt int@*/ TEMP_FAILURE_RETRY(long int /*@alt int@*/) /*@*/;

in_addr_t htonl (in_addr_t) /*@*/ ;
in_port_t htons (in_port_t) /*@*/ ;
in_addr_t ntohl (in_addr_t) /*@*/ ;
in_port_t ntohs (in_port_t) /*@*/ ;

int inet_aton(/*@in@*/const char *, /*@out@*/struct in_addr *inp)
  /*@modifies *inp@*/ ;

/*@observer@*/
char const *inet_ntoa(/*@in@*/struct in_addr) /*@*/;

const char *inet_ntop(int, /*@in@*/const void *,
		      /*@out@*//*@returned@*/char *inp, size_t)
  /*@modifies *inp@*/ ;


int gettimeofday(/*@out@*//*@null@*/struct timeval *,
		 /*@out@*//*@null@*/struct timezone *);

struct tm *localtime_r(/*@in@*/const time_t *,
		       /*@out@*//*@returned@*/struct tm *tmval)
  /*@modifies *tmval@*/ ;

int chroot (/*@notnull@*/ /*@nullterminated@*/ const char *)
    /*@globals internalState, errno@*/
    /*@modifies internalState, errno@*/
    /*:errorcode -1:*/
  /*@warn superuser "Only super-user processes may call chroot."@*/ ;

int		socket(int, int, int)
    /*@globals errno@*/
  /*@modifies errno@*/ ;

int		setsockopt(int, int, int, void const *optval, socklen_t optlen)
    /*@globals internalState, errno@*/
  /*@modifies internalState, errno@*/
  /*@requires maxRead(optval) >= optlen@*/ ;

int		select(int n, /*@null@*/fd_set *r, /*@null@*/fd_set *w,
		       /*@null@*/fd_set *e, /*@null@*/struct timeval *t)
    /*@globals errno@*/
  /*@modifies *r, *w, *e, *t, errno@*/ ;

int		recv(int s, /*@out@*/void *buf, size_t len, int flags)
  /*@globals errno@*/
  /*@modifies *buf, errno@*/
  /*@requires maxSet(buf) >= len@*/ ;


int		recvfrom(int s, /*@out@*/void *buf, size_t len,
			 int flags,
			 /*@out@*//*@null@*/struct sockaddr *from, socklen_t *fromlen)
  /*@globals errno@*/
  /*@modifies *buf, *from, *fromlen, errno@*/
  /*@requires maxSet(buf) >= len@*/ ;

int		bind(int sockfd, /*@in@*/struct sockaddr *my_addr, int addrlen)
  /*@globals errno, fileSystem@*/
  /*@modifies errno, fileSystem@*/ ;

ssize_t
sendto (int s, const void *msg, size_t len, int flags, const struct sockaddr *to, int to_len)
  /*@globals errno, fileSystem@*/
  /*@requires maxRead(msg) >= len@*/
  /*@modifies errno, fileSystem@*/ ;

ssize_t
sendmsg (int s, const struct msghdr *msg, int flags)
  /*@globals errno@*/
  /*@modifies errno@*/;


#undef FD_ZERO
#undef FD_CLR
#undef FD_SET
#undef FD_ISSET
#undef FD_COPY

extern void
FD_CLR (int n, fd_set *p)
  /*@modifies *p@*/;

extern void
FD_COPY (fd_set *f, /*@out@*/ fd_set *t)
  /*@modifies *t@*/;

extern int
FD_ISSET (int n, fd_set *p)
  /*@*/;

extern void
FD_SET (int n, fd_set *p)
  /*@modifies *p@*/;

extern void
FD_ZERO (fd_set /*@out@*/ *p)
  /*@modifies *p@*/;

int vsnprintf(/*@out@*/char *str, size_t size,
	      /*@in@*/const char *format, va_list ap)
  /*@requires (maxSet(str)+1) >= size@*/ ;


extern ssize_t
recvmsg(int s, /*@special@*/struct msghdr *msg, int flags)
    /*:errorcode -1:*/
    /*@globals fileSystem, errno@*/
    /*@requires (maxSet(msg->msg_iov)+1) >= msg->msg_iovlen
	     /\ (maxSet(msg->msg_iov->iov_base)+1) == msg->msg_iov->iov_len@*/
    /*@requires dependent msg->msg_iov->iov_base@*/
    /*@sets msg->msg_namelen, msg->msg_iovlen, msg->msg_controllen@*/
    /*@modifies *msg->msg_iov->iov_base, *msg->msg_control,
		msg->msg_controllen, msg->msg_flags, fileSystem, errno@*/
  ;

extern /*@null@*//*@temp@*//*@only@*/ void *
alloca(size_t size) /*@*/ ;


  /*@=fcnuse@*//*@=typeuse@*/
  /*@=redef@*//*@=redecl@*//*@=protoparammatch@*//*@=declundef@*/
  /*@=incondefs@*//*@=isoreserved@*//*@=export@*//*@=cppnames@*//*@=protoparamname@*/

#endif
#endif	/* H_DHCP_FORWARDER_SRC_SPLINT_COMPAT_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
