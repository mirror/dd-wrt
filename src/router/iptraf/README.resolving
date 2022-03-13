============================================================================
README DOCUMENT FOR ASYNCHRONOUS REVERSE NAME RESOLVING
----------------------------------------------------------------------------

DESCRIPTION
-----------

IPTraf-ng incorporates a function to resolve IP addresses to host names
in the background, keeping IPTraf-ng from waiting until the lookup is
completed.

If Reverse Lookup is enabled in the Options menu, the IP Traffic Monitor
will fork another process and this process will attempt to resolve
addresses.

When the traffic monitor is done, IPTraf-ng tells this resolving process
to quit.


PROTOCOL
--------

Resolving process and IPTraf-ng communicate with each other with the UNIX
domain socket IPC facility.  They use datagram sockets.

The communication protocol recognizes only 4 types of messages:

RVN_HELLO	the Hello packet.  This simply causes to throw it back,
		telling it is active.

RVN_REQUEST	a reverse lookup request.  This message includes an IP address
		to resolve.  Upon receive of this request, resolving process
		checks its internal cache to see if this IP address is
		already resolved or being resolved.  If it isn't in the cache
		yet, it forks off a copy which resolves in the background,
		while it returns the IP address in the meantime.  Subsequent
		requests will get the IP address until such time that the
		child has completed the resolution, at which time, a request
		will get the host name in reply.

RVN_REPLY	resolving process marks reply packets with this tag. Reply
		packets contain the resolved host name or the ASCII
		representation of the IP address, and an indicator of the
		state of the resolution for this address (NOTRESOLVED,
		RESOLVING or RESOLVED).

RVN_QUIT	Tells resolving process to terminate.

The datagram structure and #define's are found in the rvnamed.h header file.

Important rvnamed messages are written to /var/log/iptraf-ng/rvnamed.log.
