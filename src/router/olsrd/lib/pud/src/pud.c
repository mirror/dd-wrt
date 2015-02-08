#include "pud.h"

/* Plugin includes */
#include "dedup.h"
#include "networkInterfaces.h"
#include "configuration.h"
#include "gpsConversion.h"
#include "receiver.h"
#include "state.h"
#include "posFile.h"
#include "compiler.h"

/* OLSRD includes */
#include "olsr.h"
#include "ipcalc.h"
#include "net_olsr.h"
#include "parser.h"
#include "log.h"

/* System includes */

/** The size of the buffer in which the received NMEA string is stored */
#define BUFFER_SIZE_RX_NMEA		2048

/** The size of the buffer in which the received downlink message is stored */
#define BUFFER_SIZE_RX_DOWNLINK	2048

/** The size of the buffer in which the converted NMEA string is assembled for
 * transmission over OSLR */
#define BUFFER_SIZE_TX_OLSR 	512

/** The de-duplication list */
static DeDupList deDupList;

/** When false, use olsr_printf in pudError, otherwise use olsr_syslog */
static bool pudErrorUseSysLog = false;

/**
 Report a plugin error.

 @param useErrno
 when true then errno is used in the error message; the error reason is also
 reported.
 @param format
 a pointer to the format string
 @param ...
 arguments to the format string
 */
void pudError(bool useErrno, const char *format, ...) {
	char strDesc[256];
	const char *colon;
	const char *stringErr;

	if ((format == NULL) || (*format == '\0')) {
		strDesc[0] = '\0';
		colon = "";
		if (!useErrno) {
			stringErr = "Unknown error";
		} else {
			stringErr = strerror(errno);
		}
	} else {
		va_list arglist;

		va_start(arglist, format);
		vsnprintf(strDesc, sizeof(strDesc), format, arglist);
		va_end(arglist);

		if (useErrno) {
			colon = ": ";
			stringErr = strerror(errno);
		} else {
			colon = "";
			stringErr = "";
		}
	}

	if (!pudErrorUseSysLog)
		olsr_printf(0, "%s: %s%s%s\n", PUD_PLUGIN_ABBR, strDesc, colon, stringErr);
	else
		olsr_syslog(OLSR_LOG_ERR, "%s: %s%s%s\n", PUD_PLUGIN_ABBR, strDesc, colon, stringErr);
}

/**
 Sends a buffer out on all transmit interfaces

 @param buffer
 the buffer
 @param bufferLength
 the number of bytes in the buffer
 */
static void sendToAllTxInterfaces(unsigned char *buffer,
		unsigned int bufferLength) {
	union olsr_sockaddr * txAddress = getTxMcAddr();
	void * addr;
	socklen_t addrSize;
	TRxTxNetworkInterface *txNetworkInterfaces = getTxNetworkInterfaces();

	if (txAddress->in.sa_family == AF_INET) {
		addr = &txAddress->in4;
		addrSize = sizeof(struct sockaddr_in);
	} else {
		addr = &txAddress->in6;
		addrSize = sizeof(struct sockaddr_in6);
	}

	while (txNetworkInterfaces != NULL) {
		TRxTxNetworkInterface *networkInterface = txNetworkInterfaces;
		errno = 0;
		if (sendto(networkInterface->socketFd, buffer, bufferLength, 0, addr, addrSize) < 0) {
			pudError(true, "Transmit error on interface %s", &networkInterface->name[0]);
		}
		txNetworkInterfaces = networkInterface->next;
	}
}

/**
 Called by OLSR core when a packet for the plugin is received from the OLSR
 network. It converts the packet into an NMEA string and transmits it over all
 transmit non-OLSR network interfaces.

 @param olsrMessage
 a pointer to the received OLSR message
 @param in_if
 a pointer to the OLSR network interface on which the packet was received
 @param ipaddr
 a pointer to the IP address of the sender

 @return
 - true when the packet was processed
 - false otherwise
 */
bool packetReceivedFromOlsr(union olsr_message *olsrMessage,
		struct interface_olsr *in_if __attribute__ ((unused)), union olsr_ip_addr *ipaddr __attribute__ ((unused))) {
	const union olsr_ip_addr * originator = getOlsrMessageOriginator(
			olsr_cnf->ip_version, olsrMessage);
	unsigned int transmitStringLength;
	unsigned char buffer[BUFFER_SIZE_TX_OLSR];

	/* when we do not loopback then check if the message originated from this
	 * node: back off */
	if (!getUseLoopback() && ipequal(originator, &olsr_cnf->main_addr)) {
		return false;
	}

	/* do deduplication: when we have already seen this message from the same
	 * originator then just back off */
	if (likely(getUseDeDup())) {
		if (isInDeDupList(&deDupList, olsrMessage)) {
			return false;
		}

		addToDeDup(&deDupList, olsrMessage);
	}

	transmitStringLength = gpsFromOlsr(olsrMessage, &buffer[0], sizeof(buffer));
	assert(transmitStringLength <= sizeof(buffer));
	if (unlikely(transmitStringLength == 0)) {
		return false;
	}

	sendToAllTxInterfaces(&buffer[0], transmitStringLength);

	return true;
}

/**
 Called by OLSR core when a packet for the plugin is received from the downlink.
 It unpacks the messages and distributes them into OLSR and on the LAN.

 @param skfd
 the socket file descriptor on which the packet is received
 @param data
 a pointer to the network interface structure on which the packet was received
 @param flags
 unused
 */
static void packetReceivedFromDownlink(int skfd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused))) {
	if (skfd >= 0) {
		unsigned char rxBuffer[BUFFER_SIZE_RX_DOWNLINK];
		ssize_t rxCount = 0;
		ssize_t rxIndex = 0;

		/* Receive the captured Ethernet frame */
		errno = 0;
		rxCount = recvfrom(skfd, &rxBuffer[0], (sizeof(rxBuffer) - 1), 0, NULL, NULL);
		if (rxCount < 0) {
			pudError(true, "Receive error in %s, ignoring message.", __func__);
			return;
		}

		while (rxIndex < rxCount) {
			UplinkMessage * msg = (UplinkMessage *) &rxBuffer[rxIndex];
			uint8_t type;
			uint16_t uplinkMessageLength;
			uint16_t olsrMessageLength;
			bool ipv6;
			union olsr_message * olsrMessage;

			type = getUplinkMessageType(&msg->header);
			olsrMessageLength = getUplinkMessageLength(&msg->header);
			uplinkMessageLength = olsrMessageLength + sizeof(UplinkHeader);

			if (unlikely((rxIndex + uplinkMessageLength) > rxCount)) {
				pudError(false, "Received wrong length (%d) in %s,"
						" ignoring the rest of the messages.", olsrMessageLength,
						__func__);
				return;
			}

			rxIndex += uplinkMessageLength;

			if (type != POSITION) {
				pudError(false, "Received wrong type (%d) in %s,"
						" ignoring message.", type, __func__);
				continue;
			}

			ipv6 = getUplinkMessageIPv6(&msg->header);
			if (unlikely(!ipv6 && (olsr_cnf->ip_version == AF_INET6)) || unlikely(ipv6 && (olsr_cnf->ip_version == AF_INET))) {
				pudError(false, "Received wrong IPv6 status (%s) in %s,"
						" ignoring message.", (ipv6 ? "true" : "false"),
						__func__);
				continue;
			}

			olsrMessage = &msg->msg.olsrMessage;

			/* we now have a position update (olsrMessage) of a certain length
			 * (olsrMessageLength). this needs to be transmitted over OLSR and on the LAN */

			/* send out over OLSR interfaces (only when the smart gateway system is enabled) */
			if (olsr_cnf->smart_gw_active)
			{
				int r;
				struct interface_olsr *ifn;
				for (ifn = ifnet; ifn; ifn = ifn->int_next) {
					/* force the pending buffer out if there's not enough space for our message */
					if ((int)olsrMessageLength > net_outbuffer_bytes_left(ifn)) {
					  net_output(ifn);
					}
					r = net_outbuffer_push(ifn, olsrMessage, olsrMessageLength);
					if (r != (int) olsrMessageLength) {
						pudError(
								false,
								"Could not send to OLSR interface %s: %s"
										" (length=%u, r=%d)",
								ifn->int_name,
								((r == -1) ? "no buffer was found" :
									(r == 0) ? "there was not enough room in the buffer" :
											"unknown reason"), olsrMessageLength, r);
					}
				}
			}

			/* send out over tx interfaces */
			(void) packetReceivedFromOlsr(olsrMessage, NULL, NULL);
		}
	}
}

/**
 Called by OLSR core when a packet for the plugin is received from the non-OLSR
 network. It converts the packet into the internal OLSR wire format for a
 position update and transmits it over all OLSR network interfaces.

 @param skfd
 the socket file descriptor on which the packet is received
 @param data
 a pointer to the network interface structure on which the packet was received
 @param flags
 unused
 */
static void packetReceivedForOlsr(int skfd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused))) {
	if (skfd >= 0) {
		unsigned char rxBuffer[BUFFER_SIZE_RX_NMEA];
		ssize_t rxCount;
		union olsr_sockaddr sender;
		socklen_t senderSize = sizeof(sender);

		assert(data != NULL);

		/* Receive the captured Ethernet frame */
		memset(&sender, 0, senderSize);
		errno = 0;
		rxCount = recvfrom(skfd, &rxBuffer[0], (sizeof(rxBuffer) - 1), 0,
				(struct sockaddr *)&sender, &senderSize);
		if (rxCount < 0) {
			pudError(true, "Receive error in %s, ignoring message.", __func__);
			return;
		}

		/* make sure the string is null-terminated */
		rxBuffer[rxCount] = '\0';

		/* only accept messages from configured IP addresses */
		if (!isRxAllowedSourceIpAddress(&sender)) {
			return;
		}

		/* we have the received string in the rxBuffer now */

		/* hand the NMEA information to the receiver */
		(void) receiverUpdateGpsInformation(&rxBuffer[0], rxCount);
	}
}

/**
 * Timer callback that reads the pud position file
 */
static void pud_read_position_file(void *context __attribute__ ((unused))) {
	updatePositionFromFile();
	return;
}

/** The timer cookie, used to trace back the originator in debug */
static struct olsr_cookie_info *pud_position_file_timer_cookie = NULL;

/** The timer */
static struct timer_entry * pud_position_file_timer = NULL;

/**
 Initialise the plugin: check the configuration, initialise the NMEA parser,
 create network interface sockets, hookup the plugin to OLSR and setup data
 that can be setup in advance.

 @return
 - false upon failure
 - true otherwise
 */
bool initPud(void) {
	unsigned long long positionFilePeriod;

	if (!checkConfig()) {
		pudError(false, "Invalid configuration");
		goto error;
	}

	initState();

	if (!initDeDupList(&deDupList, getDeDupDepth())) {
		pudError(false, "Could not initialise de-duplication list");
		goto error;
	}

	if (!startPositionFile()) {
		goto error;
	}

	if (!startReceiver()) {
		pudError(false, "Could not start receiver");
		goto error;
	}

	/*
	 * Creates receive and transmit sockets and register the receive sockets
	 * with the OLSR stack
	 */
	if (!createNetworkInterfaces(&packetReceivedForOlsr,
			&packetReceivedFromDownlink)) {
		pudError(false, "Could not create require network interfaces");
		goto error;
	}

	if (!checkRunSetup()) {
		pudError(false, "Invalid configuration");
		goto error;
	}

	/*
	 * Tell OLSR to call packetReceivedFromOlsr when the packets for this
	 * plugin arrive from the OLSR network
	 */
	olsr_parser_add_function(&packetReceivedFromOlsr, PUD_OLSR_MSG_TYPE);

	/* switch to syslog logging, load was succesful */
	pudErrorUseSysLog = !olsr_cnf->no_fork;

	positionFilePeriod = getPositionFilePeriod();
	if (getPositionFile() && positionFilePeriod) {
		if (pud_position_file_timer_cookie == NULL) {
			pud_position_file_timer_cookie = olsr_alloc_cookie("pud position file", OLSR_COOKIE_TYPE_TIMER);
			if (pud_position_file_timer_cookie == NULL) {
				pudError(false, "Could not allocate pud position file cookie");
				return false;
			}
		}
		if (pud_position_file_timer == NULL) {
			pud_position_file_timer = olsr_start_timer(positionFilePeriod, 0, OLSR_TIMER_PERIODIC, &pud_read_position_file,
					NULL, pud_position_file_timer_cookie);
			if (pud_position_file_timer == NULL) {
				pudError(false, "Could not start pud position file timer");
				return false;
			}
		}
	}

	return true;

	error: closePud();
	return false;
}

/**
 Stop the plugin: shut down all created network interface sockets and destroy
 the NMEA parser.
 */
void closePud(void) {
	if (pud_position_file_timer != NULL) {
		olsr_stop_timer(pud_position_file_timer);
		pud_position_file_timer = NULL;
	}
	if (pud_position_file_timer_cookie != NULL) {
		olsr_free_cookie(pud_position_file_timer_cookie);
		pud_position_file_timer_cookie = NULL;
	}
	stopPositionFile();
	closeNetworkInterfaces();
	stopReceiver();
	destroyDeDupList(&deDupList);
}
