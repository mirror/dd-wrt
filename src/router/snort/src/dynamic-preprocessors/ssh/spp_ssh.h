/* $Id */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 * spp_ssh.h: Definitions, structs, function prototype(s) for
 *		the SSH preprocessor.
 * Author: Chris Sherwin
 */

#ifndef SPP_SSH_H
#define SPP_SSH_H

#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"

#define MAX_PORTS 65536

/*
 * Default SSH port
 */
#define SSH_PORT	22

/*
 * Boolean values.
 */
#define SSH_TRUE	(1)
#define SSH_FALSE	(0)

/*
 * Error codes.
 */
#define SSH_SUCCESS	(1)
#define SSH_FAILURE	(0)

/*
 * Default values for configurable parameters.
 */
#define SSH_DEFAULT_MAX_ENC_PKTS	25
#define SSH_DEFAULT_MAX_CLIENT_BYTES	19600
#define SSH_DEFAULT_MAX_SERVER_VERSION_LEN 80

/*
 * Min/Max values for each configurable parameter.
 */
#define MIN_MAX_ENC_PKTS 0
#define MAX_MAX_ENC_PKTS 65535

#define MIN_MAX_CLIENT_BYTES 0
#define MAX_MAX_CLIENT_BYTES 65535

#define MIN_MAX_SERVER_VERSION_LEN 0
#define MAX_MAX_SERVER_VERSION_LEN 255

/*
 * One of these structures is kept for each configured
 * server port.
 */
typedef struct _sshPortlistNode
{
	uint16_t server_port;
	struct _sshPortlistNode* nextp;
} SSHPortNode;

/*
 * Global SSH preprocessor configuration.
 *
 * AutodetectEnabled:	Whether or not to apply auto-detection of SSH
 *				to ports other than those configured.
 * MaxEncryptedPackets: Maximum number of encrypted packets examined per
 *				session.
 * MaxClientBytes:	Maximum bytes of encrypted data that can be
 *				sent by client without a server response.
 * MaxServerVersionLen: Maximum length of a server's version string.
 *              Configurable threshold for Secure CRT-style overflow.
 * DisableRules: 	Disable rule processing for SSH traffic.
 * EnabledAlerts: 	Bit vector describing which alerts are enabled.
 */
typedef struct _sshConfig
{
	uint8_t  AutodetectEnabled;
	uint16_t MaxEncryptedPackets;
	uint16_t MaxClientBytes;
    uint16_t MaxServerVersionLen;
//	uint16_t DisableRules;
	uint16_t EnabledAlerts;
//	SSHPortNode* PortList;
    char      ports[MAX_PORTS/8];

    int ref_count;

} SSHConfig;


/*
 * Per-session data block containing current state
 * of the SSH preprocessor for the session.
 *
 * version:		Version of SSH detected for this session.
 * num_enc_pkts: 	Number of encrypted packets seen on this session.
 * num_client_bytes:    Number of bytes of encrypted data sent by client,
 *				without a server response.
 * state_flags:		Bit vector describing the current state of the
 * 				session.
 */
typedef struct _sshData
{
	uint8_t  version;
	uint16_t num_enc_pkts;
	uint16_t num_client_bytes;
	uint32_t state_flags;

    tSfPolicyId policy_id;
    tSfPolicyUserContextId config;

} SSHData;

/*
 * Session state flags for SSHData::state_flags
 */
#define SSH_FLG_CLEAR			(0x0)
#define SSH_FLG_CLIENT_IDSTRING_SEEN	(0x1)
#define SSH_FLG_SERV_IDSTRING_SEEN	(0x2)
#define SSH_FLG_SERV_PKEY_SEEN		(0x4)
#define SSH_FLG_CLIENT_SKEY_SEEN	(0x8)
#define SSH_FLG_CLIENT_KEXINIT_SEEN	(0x10)
#define SSH_FLG_SERV_KEXINIT_SEEN	(0x20)
#define SSH_FLG_KEXDH_INIT_SEEN		(0x40)
#define SSH_FLG_KEXDH_REPLY_SEEN	(0x80)
#define SSH_FLG_GEX_REQ_SEEN		(0x100)
#define SSH_FLG_GEX_GRP_SEEN		(0x200)
#define SSH_FLG_GEX_INIT_SEEN		(0x400)
#define SSH_FLG_GEX_REPLY_SEEN		(0x800)
#define SSH_FLG_NEWKEYS_SEEN		(0x1000)
#define SSH_FLG_SESS_ENCRYPTED		(0x2000)
#define SSH_FLG_RESPOVERFLOW_ALERTED    (0x4000)
#define SSH_FLG_CRC32_ALERTED		(0x8000)
#define SSH_FLG_MISSED_PACKETS      (0x10000)
#define SSH_FLG_REASSEMBLY_SET      (0x20000)
#define SSH_FLG_AUTODETECTED        (0x40000)

/*
 * Some convenient combinations of state flags.
 */
#define SSH_FLG_BOTH_IDSTRING_SEEN	(SSH_FLG_CLIENT_IDSTRING_SEEN |  \
					 SSH_FLG_SERV_IDSTRING_SEEN )

#define SSH_FLG_V1_KEYEXCH_DONE		(SSH_FLG_SERV_PKEY_SEEN | \
					 SSH_FLG_CLIENT_SKEY_SEEN )

#define SSH_FLG_V2_KEXINIT_DONE		(SSH_FLG_CLIENT_KEXINIT_SEEN | \
					 SSH_FLG_SERV_KEXINIT_SEEN )

#define SSH_FLG_V2_DHOLD_DONE		(SSH_FLG_KEXDH_INIT_SEEN | \
					 SSH_FLG_KEXDH_REPLY_SEEN | \
					 SSH_FLG_NEWKEYS_SEEN )

#define SSH_FLG_V2_DHNEW_DONE		(SSH_FLG_GEX_REQ_SEEN | \
					 SSH_FLG_GEX_GRP_SEEN | \
					 SSH_FLG_GEX_INIT_SEEN | \
					 SSH_FLG_GEX_REPLY_SEEN | \
					 SSH_FLG_NEWKEYS_SEEN )

/*
 * SSH version values for SSHData::version
 */
#define SSH_VERSION_UNKNOWN	(0x0)
#define SSH_VERSION_1		(0x1)
#define SSH_VERSION_2		(0x2)

/*
 * Length of SSH2 header, in bytes.
 */
#define SSH2_HEADERLEN		(5)
#define SSH2_PACKET_MAX_SIZE    (256 * 1024)

/*
 * SSH2 binary packet struct.
 *
 * packet_length: Length of packet in bytes not including
 *		  this field or the mesg auth code (mac)
 * padding_length: Length of padding section.
 * packet_data:    Variable length packet payload + padding + MAC.
 */
typedef struct _ssh2Packet
{
	uint32_t packet_length;
	uint8_t  padding_length;
	char 	  packet_data[1];
} SSH2Packet;


/*
 * SSH v1 message types (of interest)
 */
#define SSH_MSG_V1_SMSG_PUBLIC_KEY 	2
#define SSH_MSG_V1_CMSG_SESSION_KEY	3

/*
 * SSH v2 message types (of interest)
 */
#define SSH_MSG_KEXINIT		20
#define SSH_MSG_NEWKEYS		21
#define SSH_MSG_KEXDH_INIT	30
#define SSH_MSG_KEXDH_REPLY	31

#define SSH_MSG_KEXDH_GEX_REQ	34
#define SSH_MSG_KEXDH_GEX_GRP	33
#define SSH_MSG_KEXDH_GEX_INIT  32
#define SSH_MSG_KEXDH_GEX_REPLY 31



/* Direction of sent message. */
#define SSH_DIR_FROM_SERVER	(0x1)
#define SSH_DIR_FROM_CLIENT	(0x2)

/*
 * Keyword strings for parsing configuration options.
 */
#define SSH_SERVERPORTS_KEYWORD			"server_ports"
#define SSH_MAX_ENC_PKTS_KEYWORD		"max_encrypted_packets"
#define SSH_MAX_CLIENT_BYTES_KEYWORD		"max_client_bytes"
#define SSH_MAX_SERVER_VERSION_KEYWORD  "max_server_version_len"
#define SSH_AUTODETECT_KEYWORD			"autodetect"
#define SSH_ENABLE_RESPOVERFLOW_KEYWORD		"enable_respoverflow"
#define SSH_ENABLE_CRC32_KEYWORD		"enable_ssh1crc32"
#define SSH_ENABLE_SECURECRT_KEYWORD		"enable_srvoverflow"
#define SSH_ENABLE_PROTOMISMATCH_KEYWORD	"enable_protomismatch"
#define SSH_ENABLE_WRONGDIR_KEYWORD 		"enable_badmsgdir"
#define SSH_DISABLE_RULES_KEYWORD       "disable_rules"
#define SSH_ENABLE_PAYLOAD_SIZE         "enable_paysize"
#define SSH_ENABLE_UNRECOGNIZED_VER     "enable_recognition"

/*
 * SSH preprocessor alert types.
 */
#define SSH_EVENT_RESPOVERFLOW  1
#define SSH_EVENT_CRC32			2
#define SSH_EVENT_SECURECRT		3
#define SSH_EVENT_PROTOMISMATCH	4
#define SSH_EVENT_WRONGDIR		5
#define SSH_EVENT_PAYLOAD_SIZE  6
#define SSH_EVENT_VERSION       7

/*
 * SSH alert flags
 */
#define SSH_ALERT_NONE			(0x0)
#define SSH_ALERT_RESPOVERFLOW	(0x1)
#define SSH_ALERT_CRC32			(0x2)
#define SSH_ALERT_SECURECRT		(0x4)
#define SSH_ALERT_PROTOMISMATCH	(0x8)
#define SSH_ALERT_WRONGDIR		(0x10)
#define SSH_ALERT_PAYSIZE       (0x20)
#define SSH_ALERT_UNRECOGNIZED  (0x40)
#define SSH_ALERT_ALL			(0xFFFF)

/*
 * SSH preprocessor alert strings.
 */
#define SSH_EVENT_RESPOVERFLOW_STR		"(spp_ssh) Challenge-Response Overflow exploit"
#define SSH_EVENT_CRC32_STR		"(spp_ssh) SSH1 CRC32 exploit"
#define	SSH_EVENT_SECURECRT_STR		"(spp_ssh) Server version string overflow"
#define SSH_EVENT_PROTOMISMATCH_STR	"(spp_ssh) Protocol mismatch"
#define SSH_EVENT_WRONGDIR_STR 		"(spp_ssh) Bad message direction"
#define SSH_PAYLOAD_SIZE_STR        "(spp_ssh) Payload size incorrect for the given payload"
#define SSH_VERSION_STR             "(spp_ssh) Failed to detect SSH version string"

/* Prototypes for public interface */
extern void SetupSSH(void);

#endif /* SPP_SSH_H */
