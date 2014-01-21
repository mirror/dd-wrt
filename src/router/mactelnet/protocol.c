/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define _BSD_SOURCE
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __LINUX__
#include <linux/if_ether.h>
#endif
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <time.h>
#include <endian.h>
#include "protocol.h"
#include "config.h"

#define _(String) String

int init_packet(struct mt_packet *packet, enum mt_ptype ptype, unsigned char *srcmac, unsigned char *dstmac, unsigned short sessionkey, unsigned int counter) {
	unsigned char *data = packet->data;

	/* Packet version */
	data[0] = 1;

	/* Packet type */
	data[1] = ptype;

	/* src ethernet address */
	memcpy(data + 2, srcmac, ETH_ALEN);

	/* dst ethernet address */
	memcpy(data + 8, dstmac, ETH_ALEN);

	if (mt_direction_fromserver) {
		/* Session key */
		sessionkey = htons(sessionkey);
		memcpy(data + 16, &sessionkey, sizeof(sessionkey));

		/* Client type: Mac Telnet */
		memcpy(data + 14, &mt_mactelnet_clienttype, sizeof(mt_mactelnet_clienttype));
	} else {
		/* Session key */
		sessionkey = htons(sessionkey);
		memcpy(data + 14, &sessionkey, sizeof(sessionkey));

		/* Client type: Mac Telnet */
		memcpy(data + 16, &mt_mactelnet_clienttype, sizeof(mt_mactelnet_clienttype));
	}

	/* Received/sent data counter */
	counter = htonl(counter);
	memcpy(data + 18, &counter, sizeof(counter));

	/* 22 bytes header */
	packet->size = 22;
	return 22;
}

int add_control_packet(struct mt_packet *packet, enum mt_cptype cptype, void *cpdata, int data_len) {
	unsigned char *data = packet->data + packet->size;

	/* Something is really wrong. Packets should never become over 1500 bytes */
	if (packet->size + MT_CPHEADER_LEN + data_len > MT_PACKET_LEN) {
		fprintf(stderr, _("add_control_packet: ERROR, too large packet. Exceeds %d bytes\n"), MT_PACKET_LEN);
		return -1;
		//exit(1);
	}

	/* PLAINDATA isn't really a controlpacket, but we handle it here, since
	   parseControlPacket also parses raw data as PLAINDATA */
	if (cptype == MT_CPTYPE_PLAINDATA) {
		memcpy(data, cpdata, data_len);
		packet->size += data_len;
		return data_len;
	}

	/* Control Packet Magic id */
	memcpy(data,  mt_mactelnet_cpmagic, sizeof(mt_mactelnet_cpmagic));

	/* Control packet type */
	data[4] = cptype;

	/* Data length */
#if BYTE_ORDER == LITTLE_ENDIAN
	{
		unsigned int templen;
		templen = htonl(data_len);
		memcpy(data + 5, &templen, sizeof(templen));
	}
#else
	memcpy(data + 5, &data_len, sizeof(data_len));
#endif

	/* Insert data */
	if (data_len > 0) {
		memcpy(data + MT_CPHEADER_LEN, cpdata, data_len);
	}

	packet->size += MT_CPHEADER_LEN + data_len;
	/* Control packet header length + data length */
	return MT_CPHEADER_LEN + data_len;
}

int init_pingpacket(struct mt_packet *packet, unsigned char *srcmac, unsigned char *dstmac) {
	init_packet(packet, MT_PTYPE_PING, srcmac, dstmac, 0, 0);

	/* Zero out sessionkey & counter */
	bzero(packet->data + 14, 4);

	/* Remove data counter field from header */
	packet->size -= 4;
	return packet->size;
}

int init_pongpacket(struct mt_packet *packet, unsigned char *srcmac, unsigned char *dstmac) {
	init_packet(packet, MT_PTYPE_PONG, srcmac, dstmac, 0, 0);

	/* Zero out sessionkey & counter */
	bzero(packet->data + 14, 4);

	/* Remove data counter field from header */
	packet->size -= 4;
	return packet->size;
}

int add_packetdata(struct mt_packet *packet, unsigned char *data, unsigned short length) {
	if (packet->size + length > MT_PACKET_LEN) {
		fprintf(stderr, _("add_control_packet: ERROR, too large packet. Exceeds %d bytes\n"), MT_PACKET_LEN);
		return -1;
	}

	memcpy(packet->data + packet->size, data, length);
	packet->size += length;

	return length;
}

void parse_packet(unsigned char *data, struct mt_mactelnet_hdr *pkthdr) {
	/* Packet version */
	pkthdr->ver = data[0];

	/* Packet type */
	pkthdr->ptype = data[1];

	/* src ethernet addr */
	memcpy(pkthdr->srcaddr, data + 2, ETH_ALEN);

	/* dst ethernet addr */
	memcpy(pkthdr->dstaddr, data + 8, ETH_ALEN);

	if (mt_direction_fromserver) {
		/* Session key */
		memcpy(&(pkthdr->seskey), data + 14, sizeof(pkthdr->seskey));
		pkthdr->seskey = ntohs(pkthdr->seskey);

		/* server type */
		memcpy(&(pkthdr->clienttype), data + 16, 2);
	} else {
		/* server type */
		memcpy(&(pkthdr->clienttype), data + 14, 2);

		/* Session key */
		memcpy(&(pkthdr->seskey), data + 16, sizeof(pkthdr->seskey));
		pkthdr->seskey = ntohs(pkthdr->seskey);
	}

	/* Received/sent data counter */
	memcpy(&(pkthdr->counter), data + 18, sizeof(pkthdr->counter));
	pkthdr->counter = ntohl(pkthdr->counter);

	/* Set pointer to actual data */
	pkthdr->data = data + 22;
}


int parse_control_packet(unsigned char *packetdata, int data_len, struct mt_mactelnet_control_hdr *cpkthdr) {
	static unsigned char *int_data;
	static unsigned int int_data_len;
	static unsigned int int_pos;
	unsigned char *data;

	/* Store info so we can call this function once with data,
	   and then several times for each control packets. Letting this function
	   control the data position. */
	if (packetdata != NULL) {
		if (data_len <= 0) {
			return 0;
		}

		int_data = packetdata;
		int_data_len = data_len;
		int_pos = 0;
	}

	/* No more data to parse? */
	if (int_pos >= int_data_len) {
		return 0;
	}

	/* Set current position in data buffer */
	data = int_data + int_pos;

	/* Check for valid minimum packet length & magic header */
	if (int_data_len >= 9 && memcmp(data, &mt_mactelnet_cpmagic, 4) == 0) {

		/* Control packet type */
		cpkthdr->cptype = data[4];

		/* Control packet data length */
		memcpy(&(cpkthdr->length), data + 5, sizeof(cpkthdr->length));
		cpkthdr->length = ntohl(cpkthdr->length);
		
		/* We want no buffer overflows */
		if (cpkthdr->length >= MT_PACKET_LEN - 22 - int_pos) {
			cpkthdr->length = MT_PACKET_LEN - 1 - 22 - int_pos;
		}

		/* Set pointer to actual data */
		cpkthdr->data = data + 9;

		/* Remember old position, for next call */
		int_pos += cpkthdr->length + 9;

		/* Read data successfully */
		return 1;

	} else {
		/* Mark data as raw terminal data */
		cpkthdr->cptype = MT_CPTYPE_PLAINDATA;
		cpkthdr->length = int_data_len - int_pos;
		cpkthdr->data = data;

		/* Consume the whole rest of the packet */
		int_pos = int_data_len;

		/* Read data successfully */
		return 1;
	}
}

int mndp_init_packet(struct mt_packet *packet, unsigned char version, unsigned char ttl) {
	struct mt_mndp_hdr *header = (struct mt_mndp_hdr *)packet->data;

	header->version = version;
	header->ttl = ttl;
	header->cksum = 0;
	
	packet->size = sizeof(header);
	
	return sizeof(header);
}

int mndp_add_attribute(struct mt_packet *packet, enum mt_mndp_attrtype attrtype, void *attrdata, unsigned short data_len) {
	unsigned char *data = packet->data + packet->size;
	unsigned short type = attrtype;
	unsigned short len = data_len;

	/* Something is really wrong. Packets should never become over 1500 bytes */
	if (packet->size + 4 + data_len > MT_PACKET_LEN) {
		fprintf(stderr, _("mndp_add_attribute: ERROR, too large packet. Exceeds %d bytes\n"), MT_PACKET_LEN);
		return -1;
	}

	type = htons(type);
	memcpy(data, &type, sizeof(type));

	len = htons(len);
	memcpy(data + 2, &len, sizeof(len));

	memcpy(data + 4, attrdata, data_len);

	packet->size += 4 + data_len;

	return 4 + data_len;
}


struct mt_mndp_info *parse_mndp(const unsigned char *data, const int packet_len) {
	const unsigned char *p;
	static struct mt_mndp_info packet;
	struct mt_mndp_hdr *mndp_hdr;

	/* Check for valid packet length */
	if (packet_len < 18) {
		return NULL;
	}

	bzero(&packet, sizeof(packet));

	mndp_hdr = (struct mt_mndp_hdr*)data;

	memcpy(&(packet.header), mndp_hdr, sizeof(packet.header));

	p = data + sizeof(struct mt_mndp_hdr);

	while(p < data + packet_len) {
		unsigned short type, len;

		memcpy(&type, p, 2);
		memcpy(&len, p + 2, 2);

		type = ntohs(type);
		len = ntohs(len);

		p += 4;

		/* Check if len is invalid */
		if (p + len > data + packet_len) {
			break;
		}

		switch (type) {
			case MT_MNDPTYPE_ADDRESS:
				if (len >= ETH_ALEN) {
					memcpy(packet.address, p, ETH_ALEN);
				}
				break;

			case MT_MNDPTYPE_IDENTITY:
				if (len > MT_MNDP_MAX_STRING_LENGTH) {
					len = MT_MNDP_MAX_STRING_LENGTH;
				}

				memcpy(packet.identity, p, len);
				packet.identity[len] = '\0';
				break;

			case MT_MNDPTYPE_PLATFORM:
				if (len > MT_MNDP_MAX_STRING_LENGTH) {
					len = MT_MNDP_MAX_STRING_LENGTH;
				}

				memcpy(packet.platform, p, len);
				packet.platform[len] = '\0';
				break;

			case MT_MNDPTYPE_VERSION:
				if (len > MT_MNDP_MAX_STRING_LENGTH) {
					len = MT_MNDP_MAX_STRING_LENGTH;
				}

				memcpy(packet.version, p, len);
				packet.version[len] = '\0';
				break;

			case MT_MNDPTYPE_TIMESTAMP:
				memcpy(&(packet.uptime), p, 4);
				/* Seems like ping uptime is transmitted as little endian? */
				packet.uptime = le32toh(packet.uptime);
				break;

			case MT_MNDPTYPE_HARDWARE:
				if (len > MT_MNDP_MAX_STRING_LENGTH) {
					len = MT_MNDP_MAX_STRING_LENGTH;
				}

				memcpy(packet.hardware, p, len);
				packet.hardware[len] = '\0';
				break;

			case MT_MNDPTYPE_SOFTID:
				if (len > MT_MNDP_MAX_STRING_LENGTH) {
					len = MT_MNDP_MAX_STRING_LENGTH;
				}
				
				memcpy(packet.softid, p, len);
				packet.softid[len] = '\0';
				break;

			/*default:
				 Unhandled MNDP type
			*/
		}
		
		p += len;
	}
	
	return &packet;
}

int query_mndp(const char *identity, unsigned char *mac) {
	int fastlookup = 0;
	int sock, length;
	int optval = 1;
	struct sockaddr_in si_me, si_remote;
	unsigned char buff[MT_PACKET_LEN];
	unsigned int message = 0;
	struct timeval timeout;
	time_t start_time;
	fd_set read_fds;
	struct mt_mndp_info *packet;

	start_time = time(0);

	/* Open a UDP socket handle */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/* Allow to share socket */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	/* Set initialize address/port */
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(MT_MNDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Bind to specified address/port */
	if (bind(sock, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
		fprintf(stderr, _("Error binding to %s:%d\n"), inet_ntoa(si_me.sin_addr), MT_MNDP_PORT);
		close(sock);
		return 0;
	}

	/* Set the socket to allow sending broadcast packets */
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval));

	/* Request routers identify themselves */
	memset((char *) &si_remote, 0, sizeof(si_remote));
	si_remote.sin_family = AF_INET;
	si_remote.sin_port = htons(MT_MNDP_PORT);
	si_remote.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	if (sendto(sock, &message, sizeof (message), 0, (struct sockaddr *)&si_remote, sizeof(si_remote)) == -1) {
		fprintf(stderr, _("Unable to send broadcast packet: Router lookup will be slow\n"));
		fastlookup = 0;
	} else {
		fastlookup = 1;
	}

	while (1) {
		/* Timeout, in case we receive a lot of packets, but from the wrong routers */
		if (time(0) - start_time > (fastlookup ? MT_MNDP_TIMEOUT : MT_MNDP_LONGTIMEOUT)) {
			goto done;
		}

		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);

		timeout.tv_sec = fastlookup ? MT_MNDP_TIMEOUT : MT_MNDP_LONGTIMEOUT;
		timeout.tv_usec = 0;
	
		select(sock + 1, &read_fds, NULL, NULL, &timeout);
		if (!FD_ISSET(sock, &read_fds)) {
			goto done;
		}

		/* Read UDP packet */
		length = recvfrom(sock, buff, MT_PACKET_LEN, 0, 0, 0);
		if (length < 0) {
			goto done;
		}

		/* Parse MNDP packet */
		packet = parse_mndp(buff, length);

		if (packet != NULL) {
			if (strcasecmp(identity, packet->identity) == 0) {
				memcpy(mac, packet->address, ETH_ALEN);
				close(sock);
				return 1;
			}
		}
	}
done:
	close(sock);
	return 0;
}

/*
 * This function accepts either a full MAC address using : or - as seperators.
 * Or a router hostname. The hostname will be searched for via MNDP broadcast packets.
 */
int query_mndp_or_mac(char *address, unsigned char *dstmac, int verbose) {
	char *p = address;
	int colons = 0;
	int dashs = 0;

	while (*p++) {
		if (*p == ':') {
			colons++;
		}
		else if (*p == '-') {
			dashs++;
		}
	}

	/* 
	* Windows users often enter macs with dash instead
	* of colon.
	*/
	if (colons == 0 && dashs == 5) {
		p = address;
		while (*p++) {
			if (*p == '-') {
				*p = ':';
			}
		}
		colons = dashs;
	}

	if (colons != 5) {
		/* 
		 * Not a valid mac-address.
		 * Search for Router by identity name, using MNDP
		 */
		if (verbose) {
			fprintf(stderr, _("Searching for '%s'..."), address);
		}
		if (!query_mndp(address, dstmac)) {
			if (verbose) {
				fprintf(stderr, _("not found\n"));
			}
			return 0;
		}

		/* Router found, display mac and continue */
		if (verbose) {
			fprintf(stderr, _("found\n"));
		}
	} else {
		/* Convert mac address string to ether_addr struct */
		ether_aton_r(address, (struct ether_addr *)dstmac);
	}

	return 1;
}
