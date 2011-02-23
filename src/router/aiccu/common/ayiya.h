/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/ayiya.c - AYIYA - Anything In Anything
***********************************************************
 $Author: jeroen $
 $Id: ayiya.h,v 1.10 2006-07-13 19:33:39 jeroen Exp $
 $Date: 2006-07-13 19:33:39 $
**********************************************************/

#ifndef AYIYA_H
#define AYIYA_H "H5K7:W3NDY5UU5N1K1N1C0l3"

#include "common.h"
#include "tic.h"

/* Anything In Anything - AYIYA (uses UDP in our case) */
#define AYIYA_PORT	"5072"
/*#define AYIYA_PORT	"8374"*/

/*
 * AYIYA version (which document this should conform to)
 * Per draft-massar-v6ops-ayiya-02 (July 2004)
 */
#define AYIYA_VERSION	"draft-02"

enum ayiya_identities
{
	ayiya_id_none			= 0x0,  /* None */
	ayiya_id_integer		= 0x1,	/* Integer */
	ayiya_id_string			= 0x2	/* ASCII String */
};

enum ayiya_hash
{
	ayiya_hash_none			= 0x0,	/* No hash */
	ayiya_hash_md5			= 0x1,	/* MD5 Signature */
	ayiya_hash_sha1			= 0x2,	/* SHA1 Signature */
	ayiya_hash_umac			= 0x3	/* UMAC Signature (UMAC: Message Authentication Code using Universal Hashing / draft-krovetz-umac-04.txt */
};

enum ayiya_auth
{
	ayiya_auth_none			= 0x0,	/* No authentication */
	ayiya_auth_sharedsecret		= 0x1,	/* Shared Secret */
	ayiya_auth_pgp			= 0x2	/* Public/Private Key */
};

enum ayiya_opcode
{
	ayiya_op_noop			= 0x0,	/* No Operation */
	ayiya_op_forward		= 0x1,	/* Forward */
	ayiya_op_echo_request		= 0x2,	/* Echo Request */
	ayiya_op_echo_request_forward	= 0x3,	/* Echo Request and Forward */
	ayiya_op_echo_response		= 0x4,	/* Echo Response */
	ayiya_op_motd			= 0x5,	/* MOTD */
	ayiya_op_query_request		= 0x6,	/* Query Request */
	ayiya_op_query_response		= 0x7	/* Query Response */
};

struct ayiyahdr
{
#if BYTE_ORDER == BIG_ENDIAN
	uint32_t	ayh_idlen:  4;		/* Identity Length */
	uint32_t	ayh_idtype: 4;		/* Identity Type */
	uint32_t	ayh_siglen: 4;		/* Signature Length */
	uint32_t	ayh_hshmeth:4;		/* Hashing Method */
	uint32_t	ayh_autmeth:4;		/* Authentication Method */
	uint32_t	ayh_opcode: 4;		/* Operation Code */
	uint32_t	ayh_nextheader:8;	/* Next Header (PROTO_*) */
#elif BYTE_ORDER == LITTLE_ENDIAN
	uint32_t	ayh_idtype: 4;		/* Identity Type */
	uint32_t	ayh_idlen:  4;		/* Identity Length */
	uint32_t	ayh_hshmeth:4;		/* Hashing Method */
	uint32_t	ayh_siglen: 4;		/* Signature Length */
	uint32_t	ayh_opcode: 4;		/* Operation Code */
	uint32_t	ayh_autmeth:4;		/* Authentication Method */
	uint32_t	ayh_nextheader:8;	/* Next Header (PROTO_*) */
#else
#error unsupported endianness!
#endif
	uint32_t	ayh_epochtime;		/* Time in seconds since "00:00:00 1970-01-01 UTC" */
};

/* Functions */
bool ayiya(struct TIC_Tunnel *hTunnel);
void ayiya_beat(void);

#endif /* AYIYA_H */

