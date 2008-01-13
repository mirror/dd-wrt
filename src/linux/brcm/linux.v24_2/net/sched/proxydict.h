#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
This is common code for for handling the tabels containing information about 
which proxyserver connections are associated with which machines..
*/

// Returns the number of bytes that should be available in the area
// maintained by this module given the maximal number of concurrent 
// connections.
int proxyGetMemSize(int max_connections);

// Initializes a memory area to use. There must be as many bytes
// available as returned by getMemSize.
void proxyInitMem(void* data, int max_connections);

// Queries:
int proxyGetCurConn(void* data); // Returns current number of connections
int proxyMaxCurConn(void* data); // Returns maximal number of connections

// This is called to open and close conenctions. Returns -1 if
// a protocol error occores (i.e.: If it is discovered)
int proxyConsumeBlock(void* data, ProxyRemapBlock*);

// Returns the RemapBlock associated with this connection or 0:
ProxyRemapBlock* proxyLookup(void* data, unsigned ipaddr, unsigned short port, char proto);

#ifdef __cplusplus
}
#endif
