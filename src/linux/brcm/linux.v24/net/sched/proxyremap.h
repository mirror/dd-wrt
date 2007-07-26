#ifndef PROXYREMAP_H
#define PROXYREMAP_H

// This describes the information that is written in proxyremap.log and which
// are used in the communication between proxyremapserver and proxyremapclient.
// Everything is in network order.

// First this header is send:
#define PROXY_WELCOME_LINE "ProxyRemap 1.02. This is a binary protocol.\r\n"

// Then this block is send every time a connection is opened or closed.
// Note how it is alligned to use small space usage - arrays of this
// structure are saved in many places.
typedef struct {   
  // Server endpoint of connection:
  unsigned saddr;
  unsigned short sport;

  // IP protocol for this connection (typically udp or tcp):
  unsigned char proto;
  
  // Is the connection opened or closed?
  unsigned char open;
  
  // Client the packets should be accounted to:
  unsigned caddr;
  unsigned char macaddr[6]; // Might be 0.
  
  // An informal two-charecter code from the proxyserver. Used for debugging.
  char proxyinfo[2];
} ProxyRemapBlock;

#endif
