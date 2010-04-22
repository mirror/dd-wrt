#ifndef WRR_H
#define WRR_H

/*
 * This describes the information that is written in proxyremap.log and which
 * are used in the communication between proxyremapserver and proxyremapclient.
 * Everything is in network order.
 */

/* First this header is send */
#define PROXY_WELCOME_LINE "ProxyRemap 1.02. This is a binary protocol.\r\n"

/* 
 * Then this block is send every time a connection is opened or closed.
 * Note how it is alligned to use small space usage - arrays of this
 * structure are saved in many places.
 */
typedef struct {
	/* Server endpoint of connection */
	unsigned saddr; 
	unsigned short sport;

	/* IP protocol for this connection (typically udp or tcp) */
	unsigned char proto;

	/* Is the connection opened or closed? */
	unsigned char open;

	/* Client the packets should be accounted to */
	unsigned caddr;
	unsigned char macaddr[6];	/* Might be 0. */

	/* An informal two-charecter code from the proxyserver. Used for debugging. */
	char proxyinfo[2];
} ProxyRemapBlock;


/*
 * This is common code for for handling the tables containing information about 
 * which proxyserver connections are associated with which machines..
 */

/* Returns the number of bytes that should be available in the area
 * maintained by this module given the maximal number of concurrent 
 * connections. */
int proxyGetMemSize(int max_connections);

/* Initializes a memory area to use. There must be as many bytes
   available as returned by getMemSize. */
void proxyInitMem(void *data, int max_connections);

/* Queries */
int proxyGetCurConn(void *data);	/* Returns current number of connections */
int proxyMaxCurConn(void *data);	/* Returns maximal number of connections */

/* This is called to open and close conenctions. Returns -1 if
   a protocol error occores (i.e.: If it is discovered) */
int proxyConsumeBlock(void *data, ProxyRemapBlock *);

/* Returns the RemapBlock associated with this connection or 0: */
ProxyRemapBlock *proxyLookup(void *data, unsigned ipaddr, unsigned short port,
			     char proto);

/* Return the maximum number of connections */
int proxyGetMaxConn(void *data);

#endif
