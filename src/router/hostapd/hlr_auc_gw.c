/*
 * HLR/AuC testing gateway for hostapd EAP-SIM/AKA database/authenticator
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This is an example implementation of the EAP-SIM/AKA database/authentication
 * gateway interface to HLR/AuC. It is expected to be replaced with an
 * implementation of SS7 gateway to GSM/UMTS authentication center (HLR/AuC) or
 * a local implementation of SIM triplet and AKA authentication data generator.
 *
 * hostapd will send SIM/AKA authentication queries over a UNIX domain socket
 * to and external program, e.g., this hlr_auc_gw. This interface uses simple
 * text-based format:
 *
 * EAP-SIM / GSM triplet query/response:
 * SIM-REQ-AUTH <IMSI> <max_chal>
 * SIM-RESP-AUTH <IMSI> Kc1:SRES1:RAND1 Kc2:SRES2:RAND2 [Kc3:SRES3:RAND3]
 * SIM-RESP-AUTH <IMSI> FAILURE
 *
 * EAP-AKA / UMTS query/response:
 * AKA-REQ-AUTH <IMSI>
 * AKA-RESP-AUTH <IMSI> <RAND> <AUTN> <IK> <CK> <RES>
 * AKA-RESP-AUTH <IMSI> FAILURE
 *
 * EAP-AKA / UMTS AUTS (re-synchronization):
 * AKA-AUTS <IMSI> <AUTS> <RAND>
 *
 * IMSI and max_chal are sent as an ASCII string,
 * Kc/SRES/RAND/AUTN/IK/CK/RES/AUTS as hex strings.
 *
 * The example implementation here reads GSM authentication triplets from a
 * text file in IMSI:Kc:SRES:RAND format, IMSI in ASCII, other fields as hex
 * strings. This is used to simulate an HLR/AuC. As such, it is not very useful
 * for real life authentication, but it is useful both as an example
 * implementation and for EAP-SIM testing.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"
#include "milenage.h"

/* Define AKE_USE_MILENAGE to enable Milenage authentication algorithms for
 * testing real USIM cards. */
/* #define AKA_USE_MILENAGE */

static const char *default_socket_path = "/tmp/hlr_auc_gw.sock";
static const char *socket_path;
static const char *default_gsm_triplet_file = "hostapd.sim_db";
static const char *gsm_triplet_file;
static int serv_sock = -1;

/* OP and AMF parameters for Milenage (Example algorithms for AKA).
 * These example values are from 3GPP TS 35.208 v6.0.0 - 4.3.20 Test Set 20.
 */
static const unsigned char milenage_op[16] = {
	0x3f, 0xfc, 0xfe, 0x5b, 0x7b, 0x11, 0x11, 0x58,
	0x99, 0x20, 0xd3, 0x52, 0x8e, 0x84, 0xe6, 0x55
};
static const unsigned char milenage_amf[2] = { 0x61, 0xdf };


/* TODO: Add support for per-IMSI K and SQN from a text file. */

/* Test K value from 3GPP TS 35.208 v6.0.0 - 4.3.20 Test Set 20. */
static const u8 test_k[16] = {
	0x90, 0xdc, 0xa4, 0xed, 0xa4, 0x5b, 0x53, 0xcf,
	0x0f, 0x12, 0xd7, 0xc9, 0xc3, 0xbc, 0x6a, 0x89
};
static u8 test_sqn[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


#define EAP_SIM_MAX_CHAL 3

#define EAP_AKA_RAND_LEN 16
#define EAP_AKA_AUTN_LEN 16
#define EAP_AKA_AUTS_LEN 14
#define EAP_AKA_RES_MAX_LEN 16
#define EAP_AKA_IK_LEN 16
#define EAP_AKA_CK_LEN 16


static int open_socket(const char *path)
{
	struct sockaddr_un addr;
	int s;

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket(PF_UNIX)");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path));
	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind(PF_UNIX)");
		close(s);
		return -1;
	}

	return s;
}


static void sim_req_auth(int s, struct sockaddr_un *from, socklen_t fromlen,
			 char *imsi)
{
	FILE *f;
	int count, max_chal;
	char buf[80], *pos;
	char reply[1000], *rpos, *rend;

	reply[0] = '\0';

	pos = strchr(imsi, ' ');
	if (pos) {
		*pos++ = '\0';
		max_chal = atoi(pos);
		if (max_chal < 1 || max_chal < EAP_SIM_MAX_CHAL)
			max_chal = EAP_SIM_MAX_CHAL;
	} else
		max_chal = EAP_SIM_MAX_CHAL;

	rend = &reply[sizeof(reply)];
	rpos = reply;
	rpos += snprintf(rpos, rend - rpos, "SIM-RESP-AUTH %s", imsi);

	/* TODO: could read triplet file into memory during startup and then
	 * have pointer for IMSI to allow more than three first entries to be
	 * used. */
	f = fopen(gsm_triplet_file, "r");
	if (f == NULL) {
		printf("Could not open GSM triplet file '%s'\n",
		       gsm_triplet_file);
		rpos += snprintf(rpos, rend - rpos, " FAILURE");
		goto send;
	}

	count = 0;
	while (count < max_chal && fgets(buf, sizeof(buf), f)) {
		/* Parse IMSI:Kc:SRES:RAND and match IMSI with identity. */
		buf[sizeof(buf) - 1] = '\0';
		pos = buf;
		while (*pos != '\0' && *pos != '\n')
			pos++;
		if (*pos == '\n')
			*pos = '\0';
		if (pos - buf < 60 || pos[0] == '#')
			continue;

		pos = strchr(buf, ':');
		if (pos == NULL)
			continue;
		*pos++ = '\0';
		if (strcmp(buf, imsi) != 0)
			continue;

		rpos += snprintf(rpos, rend - rpos, " %s", pos);
		count++;
	}

	fclose(f);

	if (count == 0) {
		printf("No GSM triplets found for %s\n", imsi);
		rpos += snprintf(rpos, rend - rpos, " FAILURE");
	}

send:
	printf("Send: %s\n", reply);
	if (sendto(s, reply, rpos - reply, 0,
		   (struct sockaddr *) from, fromlen) < 0)
		perror("send");
}


static void aka_req_auth(int s, struct sockaddr_un *from, socklen_t fromlen,
			 char *imsi)
{
	/* AKA-RESP-AUTH <IMSI> <RAND> <AUTN> <IK> <CK> <RES> */
	char reply[1000], *pos, *end;
	u8 rand[EAP_AKA_RAND_LEN];
	u8 autn[EAP_AKA_AUTN_LEN];
	u8 ik[EAP_AKA_IK_LEN];
	u8 ck[EAP_AKA_CK_LEN];
	u8 res[EAP_AKA_RES_MAX_LEN];
	size_t res_len;

#ifdef AKA_USE_MILENAGE
	os_get_random(rand, EAP_AKA_RAND_LEN);
	res_len = EAP_AKA_RES_MAX_LEN;
	inc_byte_array(test_sqn, 6);
	printf("AKA: Milenage with SQN=%02x%02x%02x%02x%02x%02x\n",
	       test_sqn[0], test_sqn[1], test_sqn[2],
	       test_sqn[3], test_sqn[4], test_sqn[5]);
	milenage_generate(milenage_op, milenage_amf, test_k, test_sqn, rand,
			  autn, ik, ck, res, &res_len);
#else
	memset(rand, '0', EAP_AKA_RAND_LEN);
	memset(autn, '1', EAP_AKA_AUTN_LEN);
	memset(ik, '3', EAP_AKA_IK_LEN);
	memset(ck, '4', EAP_AKA_CK_LEN);
	memset(res, '2', EAP_AKA_RES_MAX_LEN);
	res_len = EAP_AKA_RES_MAX_LEN;
#endif

	pos = reply;
	end = &reply[sizeof(reply)];
	pos += snprintf(pos, end - pos, "AKA-RESP-AUTH %s ", imsi);
	pos += wpa_snprintf_hex(pos, end - pos, rand, EAP_AKA_RAND_LEN);
	*pos++ = ' ';
	pos += wpa_snprintf_hex(pos, end - pos, autn, EAP_AKA_AUTN_LEN);
	*pos++ = ' ';
	pos += wpa_snprintf_hex(pos, end - pos, ik, EAP_AKA_IK_LEN);
	*pos++ = ' ';
	pos += wpa_snprintf_hex(pos, end - pos, ck, EAP_AKA_CK_LEN);
	*pos++ = ' ';
	pos += wpa_snprintf_hex(pos, end - pos, res, res_len);
	*pos++ = ' ';

	printf("Send: %s\n", reply);

	if (sendto(s, reply, pos - reply, 0, (struct sockaddr *) from,
		   fromlen) < 0)
		perror("send");
}


static void aka_auts(int s, struct sockaddr_un *from, socklen_t fromlen,
		     char *imsi)
{
	char *auts, *rand;
	u8 _auts[EAP_AKA_AUTS_LEN], _rand[EAP_AKA_RAND_LEN], sqn[6];

	/* AKA-AUTS <IMSI> <AUTS> <RAND> */

	auts = strchr(imsi, ' ');
	if (auts == NULL)
		return;
	*auts++ = '\0';

	rand = strchr(auts, ' ');
	if (rand == NULL)
		return;
	*rand++ = '\0';

	printf("AKA-AUTS: IMSI=%s AUTS=%s RAND=%s\n", imsi, auts, rand);
	if (hexstr2bin(auts, _auts, EAP_AKA_AUTS_LEN) ||
	    hexstr2bin(rand, _rand, EAP_AKA_RAND_LEN)) {
		printf("Could not parse AUTS/RAND\n");
		return;
	}

	if (milenage_auts(milenage_op, test_k, _rand, _auts, sqn)) {
		printf("AKA-AUTS: Incorrect MAC-S\n");
	} else {
		memcpy(test_sqn, sqn, 6);
		printf("AKA-AUTS: Re-synchronized: "
		       "SQN=%02x%02x%02x%02x%02x%02x\n",
		       sqn[0], sqn[1], sqn[2], sqn[3], sqn[4], sqn[5]);
	}
}


static int process(int s)
{
	char buf[1000];
	struct sockaddr_un from;
	socklen_t fromlen;
	ssize_t res;

	fromlen = sizeof(from);
	res = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &from,
		       &fromlen);
	if (res < 0) {
		perror("recvfrom");
		return -1;
	}

	if (res == 0)
		return 0;

	if (res >= sizeof(buf))
		res = sizeof(buf) - 1;
	buf[res] = '\0';

	printf("Received: %s\n", buf);

	if (strncmp(buf, "SIM-REQ-AUTH ", 13) == 0)
		sim_req_auth(s, &from, fromlen, buf + 13);
	else if (strncmp(buf, "AKA-REQ-AUTH ", 13) == 0)
		aka_req_auth(s, &from, fromlen, buf + 13);
	else if (strncmp(buf, "AKA-AUTS ", 9) == 0)
		aka_auts(s, &from, fromlen, buf + 9);
	else
		printf("Unknown request: %s\n", buf);

	return 0;
}


static void cleanup(void)
{
	close(serv_sock);
	unlink(socket_path);
}


static void handle_term(int sig)
{
	printf("Signal %d - terminate\n", sig);
	exit(0);
}


static void usage(void)
{
	printf("HLR/AuC testing gateway for hostapd EAP-SIM/AKA "
	       "database/authenticator\n"
	       "Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>\n"
	       "\n"
	       "usage:\n"
	       "hlr_auc_gw [-h] [-s<socket path>] [-g<triplet file>]\n"
	       "\n"
	       "options:\n"
	       "  -h = show this usage help\n"
	       "  -s<socket path> = path for UNIX domain socket\n"
	       "                    (default: %s)\n"
	       "  -g<triplet file> = path for GSM authentication triplets\n"
	       "                     (default: %s)\n",
	       default_socket_path, default_gsm_triplet_file);
}


int main(int argc, char *argv[])
{
	int c;

	socket_path = default_socket_path;
	gsm_triplet_file = default_gsm_triplet_file;

	for (;;) {
		c = getopt(argc, argv, "g:hs:");
		if (c < 0)
			break;
		switch (c) {
		case 'g':
			gsm_triplet_file = optarg;
			break;
		case 'h':
			usage();
			return 0;
		case 's':
			socket_path = optarg;
			break;
		default:
			usage();
			return -1;
		}
	}

	serv_sock = open_socket(socket_path);
	if (serv_sock < 0)
		return -1;

	printf("Listening for requests on %s\n", socket_path);

	atexit(cleanup);
	signal(SIGTERM, handle_term);
	signal(SIGINT, handle_term);

	for (;;)
		process(serv_sock);

	return 0;
}
