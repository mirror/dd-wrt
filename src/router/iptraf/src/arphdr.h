#ifndef IPTRAF_NG_ARPHDR_H
#define IPTRAF_NG_ARPHDR_H

/*
 * arp header format, stolen from the Linux include files.
 */

struct arp_hdr {
	unsigned short ar_hrd;	/* format of hardware address   */
	unsigned short ar_pro;	/* format of protocol address   */
	unsigned char ar_hln;	/* length of hardware address   */
	unsigned char ar_pln;	/* length of protocol address   */
	unsigned short ar_op;	/* ARP opcode (command)         */

	/*
	 * Ethernet looks like this : This bit is variable sized however...
	 */
	unsigned char ar_sha[ETH_ALEN];	/* sender hardware address      */
	unsigned char ar_sip[4];	/* sender IP address            */
	unsigned char ar_tha[ETH_ALEN];	/* target hardware address      */
	unsigned char ar_tip[4];	/* target IP address            */
};

#endif	/* IPTRAF_NG_ARPHDR_H */
