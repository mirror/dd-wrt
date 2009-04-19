#if !defined TL_WS2TCPIP_INCLUDED

#define TL_WS2TCPIP_INCLUDED

#define AF_INET6 23

struct in6_addr {
  unsigned char s6_addr[16];
};

struct sockaddr_in6 {
  short sin6_family;
  unsigned short sin6_port;
  unsigned long sin6_flowinfo;
  struct in6_addr sin6_addr;
  unsigned long sin6_scope_id;
};

typedef int socklen_t;

struct sockaddr_storage {
  unsigned char dummy[128];
};

#define IPPROTO_IPV6 41

#define IPV6_MULTICAST_IF 9
#define IPV6_ADD_MEMBERSHIP 12

struct ipv6_mreq {
  struct in6_addr ipv6mr_multiaddr;
  unsigned int ipv6mr_interface;
};

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
