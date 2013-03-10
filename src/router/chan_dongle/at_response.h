/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_AT_RESPONSE_H_INCLUDED
#define CHAN_DONGLE_AT_RESPONSE_H_INCLUDED

#include "export.h"			/* EXPORT_DECL EXPORT_DEF */

struct pvt;
struct iovec;

/* magic order!!! keep this enum order same as in at_responses_list */
typedef enum {
	RES_PARSE_ERROR = -1,
	RES_MIN = RES_PARSE_ERROR,
	RES_UNKNOWN = 0,

	RES_BOOT,
	RES_BUSY,
	RES_CEND,

	RES_CMGR,
	RES_CMS_ERROR,
	RES_CMTI,
	RES_CNUM,

	RES_CONF,
	RES_CONN,
	RES_COPS,
	RES_CPIN,

	RES_CREG,
	RES_CSQ,
	RES_CSSI,
	RES_CSSU,

	RES_CUSD,
	RES_ERROR,
	RES_MODE,
	RES_NO_CARRIER,

	RES_NO_DIALTONE,
	RES_OK,
	RES_ORIG,
	RES_RING,

	RES_RSSI,
	RES_SMMEMFULL,
	RES_SMS_PROMPT,
	RES_SRVST,

	RES_CVOICE,
	RES_CMGS,
	RES_CPMS,
	RES_CSCA,
	RES_CLCC,
	RES_CCWA,
	RES_MAX = RES_CCWA,
} at_res_t;

/*! response description */
typedef struct at_response_t
{
	at_res_t		res;
	const char*		name;
	const char*		id;
	unsigned		idlen;
} at_response_t;

/*! responses control */
typedef struct at_responses_t
{
	const at_response_t*	responses;
	unsigned		ids_first;		/*!< index of first id */
	unsigned		ids;			/*!< number of ids */
	int			name_first;		/*!< value of enum for first name */
	int			name_last;		/*!< value of enum for last name */
} at_responses_t;

/*! responses description */
EXPORT_DECL const at_responses_t at_responses;
EXPORT_DECL const char* at_res2str (at_res_t res);
EXPORT_DECL int at_response (struct pvt* pvt, const struct iovec * iov, int iovcnt, at_res_t at_res);

#endif /* CHAN_DONGLE_AT_RESPONSE_H_INCLUDED */
