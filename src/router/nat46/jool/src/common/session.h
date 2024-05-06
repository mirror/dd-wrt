#ifndef SRC_COMMON_SESSION_H_
#define SRC_COMMON_SESSION_H_

/** The states from the TCP state machine; RFC 6146 section 3.5.2. */
typedef enum tcp_state {
	/**
	 * The handshake is complete and the sides are exchanging upper-layer
	 * data.
	 *
	 * This is the zero one so UDP and ICMP can unset the state field if
	 * they want without fear of this looking weird.
	 * (UDP/ICMP sessions are always logically established.)
	 */
	ESTABLISHED = 0,
	/**
	 * A SYN packet arrived from the IPv6 side; some IPv4 node is trying to
	 * start a connection.
	 */
	V6_INIT,
	/**
	 * A SYN packet arrived from the IPv4 side; some IPv4 node is trying to
	 * start a connection.
	 */
	V4_INIT,
	/**
	 * The IPv4 node wants to terminate the connection. Data can still flow.
	 * Awaiting a IPv6 FIN...
	 */
	V4_FIN_RCV,
	/**
	 * The IPv6 node wants to terminate the connection. Data can still flow.
	 * Awaiting a IPv4 FIN...
	 */
	V6_FIN_RCV,
	/** Both sides issued a FIN. Packets can still flow for a short time. */
	V4_FIN_V6_FIN_RCV,
	/** The session might die in a short while. */
	TRANS,
} tcp_state;

#endif /* SRC_COMMON_SESSION_H_ */
