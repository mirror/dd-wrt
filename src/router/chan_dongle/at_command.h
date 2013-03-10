/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_AT_SEND_H_INCLUDED
#define CHAN_DONGLE_AT_SEND_H_INCLUDED

#include "export.h"		/* EXPORT_DECL EXPORT_DEF */
#include "dc_config.h"		/* call_waiting_t */
#include "mutils.h"		/* enum2str_def() ITEMS_OF() */

#define CCWA_CLASS_VOICE	1

/* magic order !!! keep order of this values like in at_cmd2str()
*/
typedef enum {
	CMD_USER = 0,

	CMD_AT,
	CMD_AT_A,
	CMD_AT_CCWA_STATUS,
	CMD_AT_CCWA_SET,
	CMD_AT_CFUN,

	CMD_AT_CGMI,
	CMD_AT_CGMM,
	CMD_AT_CGMR,
	CMD_AT_CGSN,

	CMD_AT_CHUP,
	CMD_AT_CIMI,
//	CMD_AT_CLIP,
	CMD_AT_CLIR,

	CMD_AT_CLVL,
	CMD_AT_CMGD,
	CMD_AT_CMGF,
	CMD_AT_CMGR,

	CMD_AT_CMGS,
	CMD_AT_SMSTEXT,
	CMD_AT_CNMI,
	CMD_AT_CNUM,

	CMD_AT_COPS,
	CMD_AT_COPS_INIT,
	CMD_AT_CPIN,
	CMD_AT_CPMS,

	CMD_AT_CREG,
	CMD_AT_CREG_INIT,
	CMD_AT_CSCS,
	CMD_AT_CSQ,

	CMD_AT_CSSN,
	CMD_AT_CUSD,
	CMD_AT_CVOICE,
	CMD_AT_D,

	CMD_AT_DDSETEX,
	CMD_AT_DTMF,
	CMD_AT_E,

	CMD_AT_U2DIAG,
	CMD_AT_Z,
	CMD_AT_CMEE,
	CMD_AT_CSCA,

	CMD_AT_CHLD_1x,
	CMD_AT_CHLD_2x,
	CMD_AT_CHLD_2,
	CMD_AT_CHLD_3,
	CMD_AT_CLCC
} at_cmd_t;

/*!
 * \brief Get the string representation of the given AT command
 * \param cmd -- the command to process
 * \return a string describing the given command
 */

INLINE_DECL const char* at_cmd2str (at_cmd_t cmd)
{
	/* magic!!! must be in same order as elements of enums in at_cmd_t */
	static const char * const cmds[] = {
		"USER'S",

		"AT",
		"ATA",
		"AT+CCWA?",
		"AT+CCWA=",
		"AT+CFUN",

		"AT+CGMI",
		"AT+CGMM",
		"AT+CGMR",
		"AT+CGSN",

		"AT+CHUP",
		"AT+CIMI",
//		"AT+CLIP",
		"AT+CLIR",

		"AT+CLVL",
		"AT+CMGD",
		"AT+CMGF",
		"AT+CMGR",

		"AT+CMGS",
		"SMSTEXT",
		"AT+CNMI",
		"AT+CNUM",

		"AT+COPS?",
		"AT+COPS=",
		"AT+CPIN?",
		"AT+CPMS",

		"AT+CREG?",
		"AT+CREG=",
		"AT+CSCS",
		"AT+CSQ",

		"AT+CSSN",
		"AT+CUSD",
		"AT^CVOICE",
		"ATD",

		"AT^DDSETEX",
		"AT^DTMF",
		"ATE",

		"AT^U2DIAG",
		"ATZ",
		"AT+CMEE",
		"AT+CSCA",

		"AT+CHLD=1x",
		"AT+CHLD=2x",
		"AT+CHLD=2",
		"AT+CHLD=3",
		"AT+CLCC"
	};
	return enum2str_def(cmd, cmds, ITEMS_OF(cmds), "UNDEFINED");
}


struct cpvt;

EXPORT_DECL const char* at_cmd2str (at_cmd_t cmd);
EXPORT_DECL int at_enque_initialization(struct cpvt * cpvt, at_cmd_t from_command);
EXPORT_DECL int at_enque_ping (struct cpvt * cpvt);
EXPORT_DECL int at_enque_cops (struct cpvt * cpvt);
EXPORT_DECL int at_enque_sms (struct cpvt * cpvt, const char * number, const char * msg, unsigned validity_min, int report_req, void ** id);
EXPORT_DECL int at_enque_pdu (struct cpvt * cpvt, const char * pdu, attribute_unused const char *, attribute_unused unsigned, attribute_unused int, void ** id);
EXPORT_DECL int at_enque_ussd (struct cpvt * cpvt, const char * code, attribute_unused const char *, attribute_unused unsigned, attribute_unused int, void ** id);
EXPORT_DECL int at_enque_dtmf (struct cpvt * cpvt, char digit);
EXPORT_DECL int at_enque_set_ccwa (struct cpvt * cpvt, attribute_unused const char * unused1, attribute_unused const char * unused2, unsigned call_waiting);
EXPORT_DECL int at_enque_reset (struct cpvt * cpvt);
EXPORT_DECL int at_enque_dial(struct cpvt * cpvt, const char * number, int clir);
EXPORT_DECL int at_enque_answer(struct cpvt * cpvt);
EXPORT_DECL int at_enque_user_cmd(struct cpvt * cpvt, const char * input);
EXPORT_DECL int at_enque_retrive_sms(struct cpvt * cpvt, int index, int delete);
EXPORT_DECL int at_enque_hangup (struct cpvt * cpvt, int call_idx);
EXPORT_DECL int at_enque_volsync (struct cpvt * cpvt);
EXPORT_DECL int at_enque_clcc (struct cpvt * cpvt);
EXPORT_DECL int at_enque_activate (struct cpvt * cpvt);
EXPORT_DECL int at_enque_flip_hold (struct cpvt * cpvt);
EXPORT_DECL int at_enque_conference (struct cpvt * cpvt);
EXPORT_DECL void at_hangup_immediality(struct cpvt * cpvt);

#endif /* CHAN_DONGLE_AT_SEND_H_INCLUDED */
