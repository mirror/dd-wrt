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

#include <signal.h>				/* SIGURG */

#include <asterisk.h>
#include <asterisk/callerid.h>			/*  AST_PRES_* */

#include "helpers.h"
#include "chan_dongle.h"			/* devices */
#include "at_command.h"
#include "pdu.h"				/* pdu_digit2code() */

static int is_valid_ussd_string(const char* number)
{
	for(; *number; number++)
		if(pdu_digit2code(*number) == 0)
			return 0;

	return 1;
}

#/* */
EXPORT_DEF int is_valid_phone_number(const char* number)
{
	if(number[0] == '+')
		number++;
	return is_valid_ussd_string(number);
}


#/* */
EXPORT_DEF int get_at_clir_value (struct pvt* pvt, int clir)
{
	int res = 0;

	switch (clir)
	{
		case AST_PRES_ALLOWED_NETWORK_NUMBER:
		case AST_PRES_ALLOWED_USER_NUMBER_FAILED_SCREEN:
		case AST_PRES_ALLOWED_USER_NUMBER_NOT_SCREENED:
		case AST_PRES_ALLOWED_USER_NUMBER_PASSED_SCREEN:
		case AST_PRES_NUMBER_NOT_AVAILABLE:
			ast_debug (2, "[%s] callingpres: %s\n", PVT_ID(pvt), ast_describe_caller_presentation (clir));
			res = 2;
			break;

		case AST_PRES_PROHIB_NETWORK_NUMBER:
		case AST_PRES_PROHIB_USER_NUMBER_FAILED_SCREEN:
		case AST_PRES_PROHIB_USER_NUMBER_NOT_SCREENED:
		case AST_PRES_PROHIB_USER_NUMBER_PASSED_SCREEN:
			ast_debug (2, "[%s] callingpres: %s\n", PVT_ID(pvt), ast_describe_caller_presentation (clir));
			res = 1;
			break;

		default:
			ast_log (LOG_WARNING, "[%s] Unsupported callingpres: %d\n", PVT_ID(pvt), clir);
			if ((clir & AST_PRES_RESTRICTION) != AST_PRES_ALLOWED)
			{
				res = 0;
			}
			else
			{
				res = 2;
			}
			break;
	}

	return res;
}

typedef int (*at_cmd_f)(struct cpvt*, const char*, const char*, unsigned, int, void **);

#/* */
static const char* send2(const char* dev_name, int * status, int online, const char* emsg, const char* okmsg, at_cmd_f func, const char* arg1, const char * arg2, unsigned arg3, int arg4, void ** arg5)
{
	struct pvt* pvt;
	const char* msg;

	if(status)
		*status = 0;
	pvt = find_device_ext(dev_name, &msg);
	if(pvt)
	{
		if(pvt->connected && (!online || (pvt->initialized && pvt->gsm_registered)))
		{
			if((*func) (&pvt->sys_chan, arg1, arg2, arg3, arg4, arg5))
			{
				msg = emsg;
				ast_log (LOG_ERROR, "[%s] %s\n", PVT_ID(pvt), emsg);
			}
			else
			{
				msg = okmsg;
				if(status)
					*status = 1;
			}
		}
		else
			msg = "Device not connected / initialized / registered";
		ast_mutex_unlock (&pvt->lock);
	}
	return msg;
}

#/* */
EXPORT_DEF const char* send_ussd(const char* dev_name, const char* ussd, int * status, void ** id)
{
	if(is_valid_ussd_string(ussd))
		return send2(dev_name, status, 1, "Error adding USSD command to queue", "USSD queued for send", (at_cmd_f)at_enque_ussd, ussd, 0, 0, 0, id);
	if(status)
		*status = 0;
	return "Invalid USSD";
}

#/* */
EXPORT_DEF const char * send_sms(const char * dev_name, const char * number, const char * message, const char * validity, const char * report, int * status, void ** id)
{
	if(is_valid_phone_number(number))
	{
		int val = 0;
		int srr = 0;

		if(validity)
		{
			val = strtol (validity, NULL, 10);
			if(val <= 0)
				val = 0;
		}

		if(report)
			srr = ast_true (report);

		return send2(dev_name, status, 1, "Error adding SMS commands to queue", "SMS queued for send", at_enque_sms, number, message, val, srr, id);
	}
	if(status)
		*status = 0;
	return "Invalid destination number";
}

#/* */
EXPORT_DEF const char * send_pdu(const char * dev_name, const char * pdu, int * status, void ** id)
{
	return send2(dev_name, status, 1, "Error adding SMS commands to queue", "SMS queued for send", at_enque_pdu, pdu, NULL, 0, 0, id);
}

#/* */
EXPORT_DEF const char* send_reset(const char* dev_name, int * status)
{
	return send2(dev_name, status, 0, "Error adding reset command to queue", "Reset command queued for execute", (at_cmd_f)at_enque_reset, 0, 0, 0, 0, NULL);
}

#/* */
EXPORT_DEF const char* send_ccwa_set(const char* dev_name, call_waiting_t enable, int * status)
{
	return send2(dev_name, status, 1, "Error adding CCWA commands to queue", "Call-Waiting commands queued for execute", (at_cmd_f)at_enque_set_ccwa, 0, 0, enable, 0, NULL);
}

#/* */
EXPORT_DEF const char* send_at_command(const char* dev_name, const char* command)
{
	return send2(dev_name, NULL, 0, "Error adding command", "Command queued for execute", (at_cmd_f)at_enque_user_cmd, command, NULL, 0, 0, NULL);
}

EXPORT_DEF const char* schedule_restart_event(dev_state_t event, restate_time_t when, const char* dev_name, int * status)
{
	const char * msg;
	struct pvt * pvt = find_device(dev_name);

	if (pvt)
	{
		pvt->desired_state = event;
		pvt->restart_time = when;

		pvt_try_restate(pvt);
		ast_mutex_unlock (&pvt->lock);

		msg = dev_state2str_msg(event);

		if(status)
			*status = 1;
	}
	else
	{
		msg = "Device not found";
		if(status)
			*status = 0;
	}

	return msg;
}
