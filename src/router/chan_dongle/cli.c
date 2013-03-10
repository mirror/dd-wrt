/* 
   Copyright (C) 2009 - 2010
   
   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org
   
   Dmitry Vagin <dmitry2004@yandex.ru>

   bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asterisk.h>
#include <asterisk/cli.h>			/* struct ast_cli_entry; struct ast_cli_args */
#include <asterisk/callerid.h>			/* ast_describe_caller_presentation() */
#include <asterisk/version.h>			/* ASTERISK_VERSION_NUM */

#include "cli.h"
#include "chan_dongle.h"			/* devices */
#include "helpers.h"				/* ITEMS_OF() send_ccwa_set() send_reset() send_sms() send_ussd() */
#include "pdiscovery.h"				/* pdiscovery_list_begin() pdiscovery_list_next() pdiscovery_list_end() */

static const char * restate2str_msg(restate_time_t when);

static char* complete_device (const char* word, int state)
{
	struct pvt*	pvt;
	char*	res = NULL;
	int	which = 0;
	int	wordlen = strlen (word);

	AST_RWLIST_RDLOCK (&gpublic->devices);
	AST_RWLIST_TRAVERSE (&gpublic->devices, pvt, entry)
	{
		if (!strncasecmp (PVT_ID(pvt), word, wordlen) && ++which > state)
		{
			res = ast_strdup (PVT_ID(pvt));
			break;
		}
	}
	AST_RWLIST_UNLOCK (&gpublic->devices);

	return res;
}

static char* cli_show_devices (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	struct pvt* pvt;

#define FORMAT1 "%-12.12s %-5.5s %-10.10s %-4.4s %-4.4s %-7.7s %-14.14s %-10.10s %-17.17s %-16.16s %-16.16s %-14.14s\n"
#define FORMAT2 "%-12.12s %-5d %-10.10s %-4d %-4d %-7d %-14.14s %-10.10s %-17.17s %-16.16s %-16.16s %-14.14s\n"

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle show devices";
			e->usage   =	"Usage: dongle show devices\n"
					"       Shows the state of Dongle devices.\n";
			return NULL;

		case CLI_GENERATE:
			return NULL;
	}

	if (a->argc != 3)
	{
		return CLI_SHOWUSAGE;
	}

	ast_cli (a->fd, FORMAT1, "ID", "Group", "State", "RSSI", "Mode", "Submode", "Provider Name", "Model", "Firmware", "IMEI", "IMSI", "Number");

	AST_RWLIST_RDLOCK (&gpublic->devices);
	AST_RWLIST_TRAVERSE (&gpublic->devices, pvt, entry)
	{
		ast_mutex_lock (&pvt->lock);
		ast_cli (a->fd, FORMAT2,
			PVT_ID(pvt),
			CONF_SHARED(pvt, group),
			pvt_str_state(pvt),
			pvt->rssi,
			pvt->linkmode,
			pvt->linksubmode,
			pvt->provider_name,
			pvt->model,
			pvt->firmware,
			pvt->imei,
			pvt->imsi,
			pvt->subscriber_number
		);
		ast_mutex_unlock (&pvt->lock);
	}
	AST_RWLIST_UNLOCK (&gpublic->devices);

#undef FORMAT1
#undef FORMAT2

	return CLI_SUCCESS;
}

static char* cli_show_device_settings (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	struct pvt* pvt;

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle show device settings";
			e->usage   =	"Usage: dongle show device settings <device>\n"
					"       Shows the settings of Dongle device.\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 4)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 5)
	{
		return CLI_SHOWUSAGE;
	}

	pvt = find_device (a->argv[4]);
	if (pvt)
	{
		ast_cli (a->fd, "------------- Settings ------------\n");
		ast_cli (a->fd, "  Device                  : %s\n", PVT_ID(pvt));
		ast_cli (a->fd, "  Audio                   : %s\n", CONF_UNIQ(pvt, audio_tty));
		ast_cli (a->fd, "  Data                    : %s\n", CONF_UNIQ(pvt, data_tty));
		ast_cli (a->fd, "  IMEI                    : %s\n", CONF_UNIQ(pvt, imei));
		ast_cli (a->fd, "  IMSI                    : %s\n", CONF_UNIQ(pvt, imsi));
		ast_cli (a->fd, "  Channel Language        : %s\n", CONF_SHARED(pvt, language));
		ast_cli (a->fd, "  Context                 : %s\n", CONF_SHARED(pvt, context));
		ast_cli (a->fd, "  Exten                   : %s\n", CONF_SHARED(pvt, exten));
		ast_cli (a->fd, "  Group                   : %d\n", CONF_SHARED(pvt, group));
		ast_cli (a->fd, "  RX gain                 : %d\n", CONF_SHARED(pvt, rxgain));
		ast_cli (a->fd, "  TX gain                 : %d\n", CONF_SHARED(pvt, txgain));
		ast_cli (a->fd, "  U2Diag                  : %d\n", CONF_SHARED(pvt, u2diag));
		ast_cli (a->fd, "  Use CallingPres         : %s\n", CONF_SHARED(pvt, usecallingpres) ? "Yes" : "No");
		ast_cli (a->fd, "  Default CallingPres     : %s\n", CONF_SHARED(pvt, callingpres) < 0 ? "<Not set>" : ast_describe_caller_presentation (CONF_SHARED(pvt, callingpres)));
		ast_cli (a->fd, "  Auto delete SMS         : %s\n", CONF_SHARED(pvt, autodeletesms) ? "Yes" : "No");
		ast_cli (a->fd, "  Disable SMS             : %s\n", CONF_SHARED(pvt, disablesms) ? "Yes" : "No");
		ast_cli (a->fd, "  Reset Dongle            : %s\n", CONF_SHARED(pvt, resetdongle) ? "Yes" : "No");
		ast_cli (a->fd, "  SMS PDU                 : %s\n", CONF_SHARED(pvt, smsaspdu) ? "Yes" : "No");
		ast_cli (a->fd, "  Call Waiting            : %s\n", dc_cw_setting2str(CONF_SHARED(pvt, callwaiting)));
		ast_cli (a->fd, "  DTMF                    : %s\n", dc_dtmf_setting2str(CONF_SHARED(pvt, dtmf)));
		ast_cli (a->fd, "  Minimal DTMF Gap        : %d\n", CONF_SHARED(pvt, mindtmfgap));
		ast_cli (a->fd, "  Minimal DTMF Duration   : %d\n", CONF_SHARED(pvt, mindtmfduration));
		ast_cli (a->fd, "  Minimal DTMF Interval   : %d\n", CONF_SHARED(pvt, mindtmfinterval));
		ast_cli (a->fd, "  Initial device state    : %s\n\n", dev_state2str(CONF_SHARED(pvt, initstate)));

		ast_mutex_unlock (&pvt->lock);
	}
	else
	{
		ast_cli (a->fd, "Device %s not found\n", a->argv[4]);
	}

	return CLI_SUCCESS;
}

static char* cli_show_device_state (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	struct pvt* pvt;
	struct ast_str * statebuf;
	char buf[40];

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle show device state";
			e->usage   =	"Usage: dongle show device state <device>\n"
					"       Shows the state of Dongle device.\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 4)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 5)
	{
		return CLI_SHOWUSAGE;
	}

	pvt = find_device (a->argv[4]);
	if (pvt)
	{
		statebuf = pvt_str_state_ex(pvt);

		ast_cli (a->fd, "-------------- Status -------------\n");
		ast_cli (a->fd, "  Device                  : %s\n", PVT_ID(pvt));
		ast_cli (a->fd, "  State                   : %s\n", ast_str_buffer(statebuf));
		ast_cli (a->fd, "  Audio                   : %s\n", PVT_STATE(pvt, audio_tty));
		ast_cli (a->fd, "  Data                    : %s\n", PVT_STATE(pvt, data_tty));
		ast_cli (a->fd, "  Voice                   : %s\n", (pvt->has_voice) ? "Yes" : "No");
		ast_cli (a->fd, "  SMS                     : %s\n", (pvt->has_sms) ? "Yes" : "No");
		ast_cli (a->fd, "  Manufacturer            : %s\n", pvt->manufacturer);
		ast_cli (a->fd, "  Model                   : %s\n", pvt->model);
		ast_cli (a->fd, "  Firmware                : %s\n", pvt->firmware);
		ast_cli (a->fd, "  IMEI                    : %s\n", pvt->imei);
		ast_cli (a->fd, "  IMSI                    : %s\n", pvt->imsi);
		ast_cli (a->fd, "  GSM Registration Status : %s\n", GSM_regstate2str(pvt->gsm_reg_status));
		ast_cli (a->fd, "  RSSI                    : %d, %s\n", pvt->rssi, rssi2dBm(pvt->rssi, buf, sizeof(buf)));
		ast_cli (a->fd, "  Mode                    : %s\n", sys_mode2str(pvt->linkmode));
		ast_cli (a->fd, "  Submode                 : %s\n", sys_submode2str(pvt->linksubmode));
		ast_cli (a->fd, "  Provider Name           : %s\n", pvt->provider_name);
		ast_cli (a->fd, "  Location area code      : %s\n", pvt->location_area_code);
		ast_cli (a->fd, "  Cell ID                 : %s\n", pvt->cell_id);
		ast_cli (a->fd, "  Subscriber Number       : %s\n", pvt->subscriber_number);
		ast_cli (a->fd, "  SMS Service Center      : %s\n", pvt->sms_scenter);
		ast_cli (a->fd, "  Use UCS-2 encoding      : %s\n", pvt->use_ucs2_encoding ? "Yes" : "No");
		ast_cli (a->fd, "  USSD use 7 bit encoding : %s\n", pvt->cusd_use_7bit_encoding ? "Yes" : "No");
		ast_cli (a->fd, "  USSD use UCS-2 decoding : %s\n", pvt->cusd_use_ucs2_decoding ? "Yes" : "No");
		ast_cli (a->fd, "  Tasks in queue          : %u\n", PVT_STATE(pvt, at_tasks));
		ast_cli (a->fd, "  Commands in queue       : %u\n", PVT_STATE(pvt, at_cmds));
		ast_cli (a->fd, "  Call Waiting            : %s\n", pvt->has_call_waiting ? "Enabled" : "Disabled" );
		ast_cli (a->fd, "  Current device state    : %s\n", dev_state2str(pvt->current_state) );
		ast_cli (a->fd, "  Desired device state    : %s\n", dev_state2str(pvt->desired_state) );
		ast_cli (a->fd, "  When change state       : %s\n", restate2str_msg(pvt->restart_time) );
		
		ast_cli (a->fd, "  Calls/Channels          : %u\n", PVT_STATE(pvt, chansno));
		ast_cli (a->fd, "    Active                : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_ACTIVE]));
		ast_cli (a->fd, "    Held                  : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_ONHOLD]));
		ast_cli (a->fd, "    Dialing               : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_DIALING]));
		ast_cli (a->fd, "    Alerting              : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_ALERTING]));
		ast_cli (a->fd, "    Incoming              : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_INCOMING]));
		ast_cli (a->fd, "    Waiting               : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_WAITING]));
		ast_cli (a->fd, "    Releasing             : %u\n", PVT_STATE(pvt, chan_count[CALL_STATE_RELEASED]));
		ast_cli (a->fd, "    Initializing          : %u\n\n", PVT_STATE(pvt, chan_count[CALL_STATE_INIT]));
/* TODO: show call waiting  network setting and local config value */
		ast_mutex_unlock (&pvt->lock);

		ast_free(statebuf);
	}
	else
	{
		ast_cli (a->fd, "Device %s not found\n", a->argv[4]);
	}

	return CLI_SUCCESS;
}

#/* */
static int32_t getACD(uint32_t calls, uint32_t duration)
{
	int32_t acd;

	if(calls) {
		acd = duration / calls;
	} else {
		acd = -1;
	}

	return acd;
}

#/* */
static int32_t getASR(uint32_t total, uint32_t handled)
{
	int32_t asr;
	if(total) {
		asr = handled * 100 / total;
	} else {
		asr = -1;
	}

	return asr;
}

static char* cli_show_device_statistics (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	struct pvt * pvt;

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle show device statistics";
			e->usage   =	"Usage: dongle show device statistics <device>\n"
					"       Shows the statistics of Dongle device.\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 4)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 5)
	{
		return CLI_SHOWUSAGE;
	}

	pvt = find_device (a->argv[4]);
	if (pvt)
	{
		ast_cli (a->fd, "-------------- Statistics -------------\n");
		ast_cli (a->fd, "  Device                      : %s\n", PVT_ID(pvt));
		ast_cli (a->fd, "  Queue tasks                 : %u\n", PVT_STAT(pvt, at_tasks));
		ast_cli (a->fd, "  Queue commands              : %u\n", PVT_STAT(pvt, at_cmds));
		ast_cli (a->fd, "  Responses                   : %u\n", PVT_STAT(pvt, at_responces));
		ast_cli (a->fd, "  Bytes of read responses     : %u\n", PVT_STAT(pvt, d_read_bytes));
		ast_cli (a->fd, "  Bytes of written commands   : %u\n", PVT_STAT(pvt, d_write_bytes));
		ast_cli (a->fd, "  Bytes of read audio         : %llu\n", (unsigned long long int)PVT_STAT(pvt, a_read_bytes));
		ast_cli (a->fd, "  Bytes of written audio      : %llu\n", (unsigned long long int)PVT_STAT(pvt, a_write_bytes));
		ast_cli (a->fd, "  Readed frames               : %u\n", PVT_STAT(pvt, read_frames));
		ast_cli (a->fd, "  Readed short frames         : %u\n", PVT_STAT(pvt, read_sframes));
		ast_cli (a->fd, "  Wrote frames                : %u\n", PVT_STAT(pvt, write_frames));
		ast_cli (a->fd, "  Wrote short frames          : %u\n", PVT_STAT(pvt, write_tframes));
		ast_cli (a->fd, "  Wrote silence frames        : %u\n", PVT_STAT(pvt, write_sframes));
		ast_cli (a->fd, "  Write buffer overflow bytes : %llu\n", (unsigned long long int)PVT_STAT(pvt, write_rb_overflow_bytes));
		ast_cli (a->fd, "  Write buffer overflow count : %u\n", PVT_STAT(pvt, write_rb_overflow));
		ast_cli (a->fd, "  Incoming calls              : %u\n", PVT_STAT(pvt, in_calls));
		ast_cli (a->fd, "  Waiting calls               : %u\n", PVT_STAT(pvt, cw_calls));
		ast_cli (a->fd, "  Handled input calls         : %u\n", PVT_STAT(pvt, in_calls_handled));
		ast_cli (a->fd, "  Fails to PBX run            : %u\n", PVT_STAT(pvt, in_pbx_fails));
		ast_cli (a->fd, "  Attempts to outgoing calls  : %u\n", PVT_STAT(pvt, out_calls));
		ast_cli (a->fd, "  Answered outgoing calls     : %u\n", PVT_STAT(pvt, calls_answered[CALL_DIR_OUTGOING]));
		ast_cli (a->fd, "  Answered incoming calls     : %u\n", PVT_STAT(pvt, calls_answered[CALL_DIR_INCOMING]));
		ast_cli (a->fd, "  Seconds of outgoing calls   : %u\n", PVT_STAT(pvt, calls_duration[CALL_DIR_OUTGOING]));
		ast_cli (a->fd, "  Seconds of incoming calls   : %u\n", PVT_STAT(pvt, calls_duration[CALL_DIR_INCOMING]));
		ast_cli (a->fd, "  ACD for incoming calls      : %d\n", getACD(PVT_STAT(pvt, calls_answered[CALL_DIR_INCOMING]), PVT_STAT(pvt, calls_duration[CALL_DIR_INCOMING])));
		ast_cli (a->fd, "  ACD for outgoing calls      : %d\n", getACD(PVT_STAT(pvt, calls_answered[CALL_DIR_OUTGOING]), PVT_STAT(pvt, calls_duration[CALL_DIR_OUTGOING])));
/*
		ast_cli (a->fd, "  ACD                         : %d\n",
			getACD(
				PVT_STAT(pvt, calls_answered[CALL_DIR_OUTGOING]) 
				+ PVT_STAT(pvt, calls_answered[CALL_DIR_INCOMING]), 

				PVT_STAT(pvt, calls_duration[CALL_DIR_OUTGOING]) 
				+ PVT_STAT(pvt, calls_duration[CALL_DIR_INCOMING])
				)
			);
*/
		ast_cli (a->fd, "  ASR for incoming calls      : %d\n", getASR(PVT_STAT(pvt, in_calls) + PVT_STAT(pvt, cw_calls), PVT_STAT(pvt, calls_answered[CALL_DIR_INCOMING])) );
		ast_cli (a->fd, "  ASR for outgoing calls      : %d\n\n", getASR(PVT_STAT(pvt, out_calls), PVT_STAT(pvt, calls_answered[CALL_DIR_OUTGOING])));
/*
		ast_cli (a->fd, "  ASR                         : %d\n\n",
			getASR(
				PVT_STAT(pvt, out_calls)
				+ PVT_STAT(pvt, in_calls)
				+ PVT_STAT(pvt, cw_calls),

				PVT_STAT(pvt, calls_answered[CALL_DIR_OUTGOING])
				+ PVT_STAT(pvt, calls_answered[CALL_DIR_INCOMING])
				)
			);
*/
		ast_mutex_unlock (&pvt->lock);
	}
	else
	{
		ast_cli (a->fd, "Device %s not found\n", a->argv[4]);
	}

	return CLI_SUCCESS;
}


static char* cli_show_version (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle show version";
			e->usage   =	"Usage: dongle show version\n"
					"       Shows the version of module.\n";
			return NULL;

		case CLI_GENERATE:
			return NULL;
	}

	if (a->argc != 3)
	{
		return CLI_SHOWUSAGE;
	}

	ast_cli (a->fd, "\n%s: %s, Version %s, Revision %s\nProject Home: %s\nBug Reporting: %s\n\n", AST_MODULE, MODULE_DESCRIPTION, MODULE_VERSION, PACKAGE_REVISION, MODULE_URL, MODULE_BUGREPORT);

	return CLI_SUCCESS;
}

static char* cli_cmd (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	const char * msg;

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle cmd";
			e->usage   =	"Usage: dongle cmd <device> <command>\n"
					"       Send <command> to the rfcomm port on the device\n"
					"       with the specified <device>.\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 2)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 4)
	{
		return CLI_SHOWUSAGE;
	}

	msg = send_at_command(a->argv[2], a->argv[3]);
	ast_cli (a->fd, "[%s] '%s' %s\n", a->argv[2], a->argv[3], msg);

	return CLI_SUCCESS;
}

static char* cli_ussd (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	const char * msg;
	int status;
	void * msgid;

	switch (cmd)
	{
		case CLI_INIT:
			e->command = "dongle ussd";
			e->usage =
				"Usage: dongle ussd <device> <command>\n"
				"       Send ussd <command> to the dongle\n"
				"       with the specified <device>.\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 2)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 4)
	{
		return CLI_SHOWUSAGE;
	}

	msg = send_ussd(a->argv[2], a->argv[3], &status, &msgid);
	if(status)
		ast_cli (a->fd, "[%s] %s with id %p\n", a->argv[2], msg, msgid);
	else
		ast_cli (a->fd, "[%s] %s\n", a->argv[2], msg);

	return CLI_SUCCESS;
}

static char* cli_sms (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	const char * msg;
	struct ast_str * buf;
	int i;
	int status;
	void * msgid;

	switch (cmd)
	{
		case CLI_INIT:
			e->command = "dongle sms";
			e->usage =
				"Usage: dongle sms <device> <number> <message>\n"
				"       Send a SMS to <number> with the <message> from <device>\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 2)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc < 5)
	{
		return CLI_SHOWUSAGE;
	}

	buf = ast_str_create (256);
	for (i = 4; i < a->argc; i++)
	{
		if (i < (a->argc - 1))
		{
			ast_str_append (&buf, 0, "%s ", a->argv[i]);
		}
		else
		{
			ast_str_append (&buf, 0, "%s", a->argv[i]);
		}
	}

	msg = send_sms(a->argv[2], a->argv[3], ast_str_buffer(buf), 0, 0, &status, &msgid);
	ast_free (buf);

	if(status)
		ast_cli(a->fd, "[%s] %s with id %p\n", a->argv[2], msg, msgid);
	else
		ast_cli(a->fd, "[%s] %s\n", a->argv[2], msg);

	return CLI_SUCCESS;
}

static char * cli_pdu(struct ast_cli_entry * e, int cmd, struct ast_cli_args * a)
{
	const char * msg;
	int status;
	void * msgid;

	switch (cmd)
	{
		case CLI_INIT:
			e->command = "dongle pdu";
			e->usage =
				"Usage: dongle pdu <device> <PDU>\n"
				"       Send a <PDU> of sms from <device>\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 2)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 4)
	{
		return CLI_SHOWUSAGE;
	}

	msg = send_pdu(a->argv[2], a->argv[3], &status, &msgid);

	if(status)
		ast_cli(a->fd, "[%s] %s with id %p\n", a->argv[2], msg, msgid);
	else
		ast_cli(a->fd, "[%s] %s\n", a->argv[2], msg);

	return CLI_SUCCESS;
}

#if ASTERISK_VERSION_NUM >= 10800
typedef const char * const * ast_cli_complete2_t;
#else
typedef char * const * ast_cli_complete2_t;
#endif

static char* cli_ccwa_set (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	static const char * const choices[] = { "enable", "disable", NULL };
	const char * msg;
	call_waiting_t enable;

	switch (cmd)
	{
		case CLI_INIT:
			e->command = "dongle callwaiting";
			e->usage =
				"Usage: dongle callwaiting disable|enable <device>\n"
				"       Disable/Enable Call-Waiting on <device>\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 2)
			{
				return ast_cli_complete(a->word, (ast_cli_complete2_t)choices, a->n);
			}
			if (a->pos == 3)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc < 4)
	{
		return CLI_SHOWUSAGE;
	}
	if (strcasecmp("disable", a->argv[2]) == 0)
		enable = CALL_WAITING_DISALLOWED;
	else if (strcasecmp("enable", a->argv[2]) == 0)
		enable = CALL_WAITING_ALLOWED;
	else
		return CLI_SHOWUSAGE;

	msg = send_ccwa_set(a->argv[3], enable, NULL);
	ast_cli (a->fd, "[%s] %s\n", a->argv[3], msg);

	return CLI_SUCCESS;
}

static char* cli_reset (struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	const char * msg;

	switch (cmd)
	{
		case CLI_INIT:
			e->command = "dongle reset";
			e->usage =
				"Usage: dongle reset <device>\n"
				"       Reset dongle <device>\n";
			return NULL;

		case CLI_GENERATE:
			if (a->pos == 2)
			{
				return complete_device (a->word, a->n);
			}
			return NULL;
	}

	if (a->argc != 3)
	{
		return CLI_SHOWUSAGE;
	}

	msg = send_reset(a->argv[2], NULL);
	ast_cli (a->fd, "[%s] %s\n", a->argv[2], msg);

	return CLI_SUCCESS;
}

static const char * const a_choices[] = { "now", "gracefully", "when", NULL };
static const char * const a_choices2[] = { "convenient", NULL };

static const char * restate2str_msg(restate_time_t when)
{
	static const char * const choices[] = { "now", "gracefully", "when convenient" };
	return enum2str(when, choices, ITEMS_OF(choices));
}

#/* */
static char* cli_restart_event(struct ast_cli_entry* e, int cmd, struct ast_cli_args* a, dev_state_t event)
{

	static char * const cmds[] = {
		"dongle stop",
		"dongle restart",
		"dongle remove",
		};
	static const char * const usage[] = {
		"Usage: dongle stop < now | gracefully | when convenient > <device>\n"
		"       Stop dongle <device>\n",

		"Usage: dongle restart < now | gracefully | when convenient > <device>\n"
		"       Restart dongle <device>\n",

		"Usage: dongle remove < now | gracefully | when convenient > <device>\n"
		"       Remove dongle <device>\n",
		};

	const char * device = NULL;
	const char * msg;
	int i;

	switch (cmd)
	{
		case CLI_INIT:
			e->command = cmds[event];
			e->usage = usage[event];
			return NULL;

		case CLI_GENERATE:
			switch(a->pos)
			{
				case 2:
					return ast_cli_complete(a->word, (ast_cli_complete2_t)a_choices, a->n);
				case 3:
					if(strcasecmp(a->argv[2], "when") == 0)
						return ast_cli_complete(a->word, (ast_cli_complete2_t)a_choices2, a->n);
					return complete_device(a->word, a->n);
					break;
				case 4:
					if(strcasecmp(a->argv[2], "when") == 0 && strcasecmp(a->argv[3], "convenient") == 0)
						return complete_device(a->word, a->n);
			}
			return NULL;
	}

	if(a->argc != 4 && a->argc != 5)
	{
		return CLI_SHOWUSAGE;
	}

	for(i = 0; a_choices[i]; i++)
	{
		if(strcasecmp(a->argv[2], a_choices[i]) == 0)
		{
			if(i == RESTATE_TIME_CONVENIENT)
			{
				if(a->argc == 5 && strcasecmp(a->argv[3], a_choices2[0]) == 0)
				{
					device = a->argv[4];
				}
			}
			else if(a->argc == 4)
			{
				device = a->argv[3];
			}

			if(device)
			{
				msg = schedule_restart_event(event, i, device, NULL);
				ast_cli(a->fd, "[%s] %s\n", device, msg);
				return CLI_SUCCESS;
			}
			break;
		}
	}
	return CLI_SHOWUSAGE;
}

#/* */
static char* cli_stop(struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	return cli_restart_event(e, cmd, a, DEV_STATE_STOPPED);
}

#/* */
static char* cli_restart(struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	return cli_restart_event(e, cmd, a, DEV_STATE_RESTARTED);
}

#/* */
static char * cli_remove(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
{
	return cli_restart_event(e, cmd, a, DEV_STATE_REMOVED);
}

#/* */
static char* cli_start(struct ast_cli_entry* e, int cmd, struct ast_cli_args* a)
{
	const char * msg;

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle start";
			e->usage   =	"Usage: dongle start <device>\n"
					"       Start dongle <device>\n";
			return NULL;

		case CLI_GENERATE:
			if(a->pos == 2)
				return complete_device(a->word, a->n);
			return NULL;
	}

	if (a->argc != 3)
	{
		return CLI_SHOWUSAGE;
	}

	msg = schedule_restart_event(DEV_STATE_STARTED, RESTATE_TIME_NOW, a->argv[2], NULL);
	ast_cli(a->fd, "[%s] %s\n", a->argv[2], msg);

	return CLI_SUCCESS;
}

static char * cli_reload(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
{
	int ok = 0;
	int i;

	switch (cmd)
	{
		case CLI_INIT:
			e->command =	"dongle reload";
			e->usage   =	"Usage: dongle reload < now | gracefully | when convenient >\n"
					"       Reloads the chan_dongle configuration\n";
			return NULL;

		case CLI_GENERATE:
			switch(a->pos)
			{
				case 2:
					return ast_cli_complete(a->word, (ast_cli_complete2_t)a_choices, a->n);
				case 3:
					if(strcasecmp(a->argv[2], "when") == 0)
						return ast_cli_complete(a->word, (ast_cli_complete2_t)a_choices2, a->n);
			}
			return NULL;
	}

	if (a->argc != 3 && a->argc != 4)
	{
		return CLI_SHOWUSAGE;
	}

	for(i = 0; a_choices[i]; i++)
	{
		if(strcasecmp(a->argv[2], a_choices[i]) == 0)
		{
			if(i == RESTATE_TIME_CONVENIENT)
			{
				if(a->argc == 4 && strcasecmp(a->argv[3], a_choices2[0]) == 0)
				{
					ok = 1;
				}
			}
			else if(a->argc == 3)
			{
				ok = 1;
			}

			if(ok)
			{
				pvt_reload(i);
				return CLI_SUCCESS;
			}
			break;
		}
	}
	return CLI_SHOWUSAGE;
}

#/* */
static char * cli_discovery(struct ast_cli_entry * e, int cmd, struct ast_cli_args * a)
{
	const struct pdiscovery_cache_item * item;
	const struct pdiscovery_result * res;
	struct pvt * pvt;
	const char * imei;
	const char * imsi;
	int imeilen;
	int imsilen;
	
	switch (cmd) {
		case CLI_INIT:
			e->command =	"dongle discovery";
			e->usage   =	"Usage: dongle discovery\n"
					"       Discovery devices and create config\n";
			return NULL;

		case CLI_GENERATE:
			return NULL;
	}

	if (a->argc != 2) {
		return CLI_SHOWUSAGE;
	}

	AST_RWLIST_RDLOCK(&gpublic->devices);
	for(res = pdiscovery_list_begin(&item); res; res = pdiscovery_list_next(&item)) {
		AST_RWLIST_TRAVERSE (&gpublic->devices, pvt, entry) {
			if(strcmp(PVT_STATE(pvt, data_tty), res->ports.ports[INTERFACE_TYPE_DATA]) == 0) {
				break;
			}
		}
		if(pvt) {
/*
			ast_cli(a->fd, "; existing device\n");
			ast_cli(a->fd, "[%s](defaults)\n", PVT_ID(pvt));

			if(CONF_UNIQ(pvt, audio_tty)[0])
				ast_cli(a->fd, "audio=%s\n", CONF_UNIQ(pvt, audio_tty));
			else
				ast_cli(a->fd, ";audio=%s\n", PVT_STATE(pvt, audio_tty));

			if(CONF_UNIQ(pvt, data_tty)[0])
				ast_cli(a->fd, "data=%s\n", CONF_UNIQ(pvt, data_tty));
			else
				ast_cli(a->fd, ";data=%s\n", PVT_STATE(pvt, data_tty));

			if(CONF_UNIQ(pvt, imei)[0])
				ast_cli(a->fd, "imei=%s\n", CONF_UNIQ(pvt, imei));
			else
				ast_cli(a->fd, ";imei=%s\n", pvt->imei);

			if(CONF_UNIQ(pvt, imsi)[0])
				ast_cli(a->fd, "imsi=%s\n\n", CONF_UNIQ(pvt, imsi));
			else
				ast_cli(a->fd, ";imsi=%s\n\n", pvt->imsi);
*/
		} else {
			imei = S_OR(res->imei, "");
			imsi = S_OR(res->imsi, "");

			imeilen = strlen(imei);
			imsilen = strlen(imsi);

			ast_cli(a->fd, "; discovered device\n");
			ast_cli(a->fd, "[dc_%s_%s](defaults)\n", imei + imeilen - MIN(imeilen,4), imsi + imsilen - MIN(imsilen,4));
			ast_cli(a->fd, ";audio=%s\n", res->ports.ports[INTERFACE_TYPE_VOICE]);
			ast_cli(a->fd, ";data=%s\n", res->ports.ports[INTERFACE_TYPE_DATA]);
			ast_cli(a->fd, "imei=%s\n", imei);
			ast_cli(a->fd, "imsi=%s\n\n", imsi);
		}
	}
	pdiscovery_list_end();
	AST_RWLIST_UNLOCK(&gpublic->devices);
	
	return CLI_SUCCESS;
}



static struct ast_cli_entry cli[] = {
	AST_CLI_DEFINE (cli_show_devices,	"Show Dongle devices state"),
	AST_CLI_DEFINE (cli_show_device_settings,"Show Dongle device settings"),
	AST_CLI_DEFINE (cli_show_device_state,	 "Show Dongle device state"),
	AST_CLI_DEFINE (cli_show_device_statistics,"Show Dongle device statistics"),
	AST_CLI_DEFINE (cli_show_version,	"Show module version"),
	AST_CLI_DEFINE (cli_cmd,		"Send commands to port for debugging"),
	AST_CLI_DEFINE (cli_ussd,		"Send USSD commands to the dongle"),
	AST_CLI_DEFINE (cli_sms,		"Send SMS from the dongle"),
	AST_CLI_DEFINE (cli_pdu,		"Send PDU of SMS from the dongle"),
	AST_CLI_DEFINE (cli_ccwa_set,		"Enable/Disable Call-Waiting on the dongle"),
	AST_CLI_DEFINE (cli_reset,		"Reset dongle now"),

	AST_CLI_DEFINE (cli_stop,		"Stop dongle"),
	AST_CLI_DEFINE (cli_restart,		"Restart dongle"),
	AST_CLI_DEFINE (cli_remove,		"Remove dongle"),
	AST_CLI_DEFINE (cli_reload,		"Reload dongle"),

	AST_CLI_DEFINE (cli_start,		"Start dongle"),
	AST_CLI_DEFINE (cli_discovery,		"Discovery devices and create config"),
};

#/* */
EXPORT_DEF void cli_register()
{
	ast_cli_register_multiple (cli, ITEMS_OF(cli));
}

#/* */
EXPORT_DEF void cli_unregister()
{
	ast_cli_unregister_multiple (cli, ITEMS_OF(cli));
}

/*
static char * ast_str_truncate2(struct ast_str *buf, ssize_t len)
{
	if (len < 0) {
		buf->__AST_STR_USED += ((ssize_t) abs(len)) > ((ssize_t) buf->__AST_STR_USED) ? (ssize_t)-buf->__AST_STR_USED : (ssize_t)len;
	} else {
		buf->__AST_STR_USED = len;
	}
	buf->__AST_STR_STR[buf->__AST_STR_USED] = '\0';
	return buf->__AST_STR_STR;
}
*/
