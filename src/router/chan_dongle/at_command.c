/* 
   Copyright (C) 2009 - 2010
   
   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org
   
   Dmitry Vagin <dmitry2004@yandex.ru>

   bg <bg_one@mail.ru>
*/

/*
   Copyright (C) 2009 - 2010 Artem Makhutov
   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asterisk.h>
#include <asterisk/utils.h>

#include "at_command.h"
//#include ".h"
#include "at_queue.h"
#include "char_conv.h"			/* char_to_hexstr_7bit() */
#include "chan_dongle.h"		/* struct pvt */
#include "pdu.h"			/* build_pdu() */

static const char cmd_at[] 	 = "AT\r";
static const char cmd_chld1x[]   = "AT+CHLD=1%d\r";
static const char cmd_chld2[]    = "AT+CHLD=2\r";
static const char cmd_clcc[]     = "AT+CLCC\r";
static const char cmd_ddsetex2[] = "AT^DDSETEX=2\r";

/*!
 * \brief Format and fill generic command
 * \param cmd -- the command structure
 * \param format -- printf format string
 * \param ap -- list of arguments
 * \return 0 on success
 */

static int at_fill_generic_cmd_va (at_queue_cmd_t * cmd, const char * format, va_list ap)
{
	char buf[4096];
	
	cmd->length = vsnprintf (buf, sizeof(buf)-1, format, ap);

	buf[cmd->length] = 0;
	cmd->data = ast_strdup(buf);
	if(!cmd->data)
		return -1;

	cmd->flags &= ~ATQ_CMD_FLAG_STATIC;
	return 0;

}

/*!
 * \brief Format and fill generic command
 * \param cmd -- the command structure
 * \param format -- printf format string
 * \return 0 on success
 */

static int __attribute__ ((format(printf, 2, 3))) at_fill_generic_cmd (at_queue_cmd_t * cmd, const char * format, ...)
{
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = at_fill_generic_cmd_va(cmd, format, ap);
	va_end(ap);

	return rv;
}

/*!
 * \brief Enque generic command
 * \param pvt -- pvt structure
 * \param cmd -- at command
 * \param prio -- queue priority of this command
 * \param format -- printf format string including AT command text
 * \return 0 on success
 */

static int __attribute__ ((format(printf, 4, 5))) at_enque_generic (struct cpvt* cpvt, at_cmd_t cmd, int prio, const char * format, ...)
{
	va_list ap;
	int rv;
	at_queue_cmd_t at_cmd = ATQ_CMD_DECLARE_DYN(cmd);

	va_start(ap, format);
	rv = at_fill_generic_cmd_va(&at_cmd, format, ap);
	va_end(ap);

	if(!rv)
		rv = at_queue_insert(cpvt, &at_cmd, 1, prio);
	return rv;
}

/*!
 * \brief Enque initialization commands
 * \param cpvt -- cpvt structure
 * \param from_command -- begin initialization from this command in list
 * \return 0 on success
 */
EXPORT_DEF int at_enque_initialization(struct cpvt* cpvt, at_cmd_t from_command)
{
	static const char cmd2[] = "ATZ\r";
	static const char cmd3[] = "ATE0\r";

	static const char cmd5[] = "AT+CGMI\r";
	static const char cmd6[] = "AT+CSCA?\r";
	static const char cmd7[] = "AT+CGMM\r";
	static const char cmd8[] = "AT+CGMR\r";

	static const char cmd9[] = "AT+CMEE=0\r";
	static const char cmd10[] = "AT+CGSN\r";
	static const char cmd11[] = "AT+CIMI\r";
	static const char cmd12[] = "AT+CPIN?\r";

	static const char cmd13[] = "AT+COPS=0,0\r";
	static const char cmd14[] = "AT+CREG=2\r";
	static const char cmd15[] = "AT+CREG?\r";
	static const char cmd16[] = "AT+CNUM\r";

	static const char cmd17[] = "AT^CVOICE?\r";
//	static const char cmd18[] = "AT+CLIP=0\r";
	static const char cmd19[] = "AT+CSSN=1,1\r";
	static const char cmd21[] = "AT+CSCS=\"UCS2\"\r";

	static const char cmd22[] = "AT+CPMS=\"ME\",\"ME\",\"ME\"\r";
	static const char cmd23[] = "AT+CNMI=2,1,0,0,0\r";
	static const char cmd24[] = "AT+CSQ\r";

	static const at_queue_cmd_t st_cmds[] = {
		ATQ_CMD_DECLARE_ST(CMD_AT, cmd_at),
		ATQ_CMD_DECLARE_ST(CMD_AT_Z, cmd2),		/* optional,  reload configuration */
		ATQ_CMD_DECLARE_ST(CMD_AT_E, cmd3),		/* disable echo */
		ATQ_CMD_DECLARE_DYN(CMD_AT_U2DIAG),		/* optional, Enable or disable some devices */
		ATQ_CMD_DECLARE_ST(CMD_AT_CGMI, cmd5),		/* Getting manufacturer info */

		ATQ_CMD_DECLARE_ST(CMD_AT_CGMM, cmd7),		/* Get Product name */
		ATQ_CMD_DECLARE_ST(CMD_AT_CGMR, cmd8),		/* Get software version */
		ATQ_CMD_DECLARE_ST(CMD_AT_CMEE, cmd9),		/* set MS Error Report to 'ERROR' only  TODO: change to 1 or 2 and add support in response handlers */

		ATQ_CMD_DECLARE_ST(CMD_AT_CGSN, cmd10),		/* IMEI Read */
		ATQ_CMD_DECLARE_ST(CMD_AT_CIMI, cmd11),		/* IMSI Read */
		ATQ_CMD_DECLARE_ST(CMD_AT_CPIN, cmd12),		/* check is password authentication requirement and the remainder validation times */
		ATQ_CMD_DECLARE_ST(CMD_AT_COPS_INIT, cmd13),	/* Read operator name */

		ATQ_CMD_DECLARE_STI(CMD_AT_CREG_INIT,cmd14),	/* GSM registration status setting */
		ATQ_CMD_DECLARE_ST(CMD_AT_CREG, cmd15),		/* GSM registration status */
		ATQ_CMD_DECLARE_ST(CMD_AT_CNUM, cmd16),		/* Get Subscriber number */
		ATQ_CMD_DECLARE_ST(CMD_AT_CVOICE, cmd17),	/* read the current voice mode, and return sampling rate、data bit、frame period */

		ATQ_CMD_DECLARE_ST(CMD_AT_CSCA, cmd6),		/* Get SMS Service center address */
//		ATQ_CMD_DECLARE_ST(CMD_AT_CLIP, cmd18),		/* disable  Calling line identification presentation in unsolicited response +CLIP: <number>,<type>[,<subaddr>,<satype>[,[<alpha>][,<CLI validitity>]] */
		ATQ_CMD_DECLARE_ST(CMD_AT_CSSN, cmd19),		/* activate Supplementary Service Notification with CSSI and CSSU */
		ATQ_CMD_DECLARE_DYN(CMD_AT_CMGF),		/* Set Message Format */

		ATQ_CMD_DECLARE_STI(CMD_AT_CSCS, cmd21),	/* UCS-2 text encoding */

		ATQ_CMD_DECLARE_ST(CMD_AT_CPMS, cmd22),		/* SMS Storage Selection */
			/* pvt->initialized = 1 after successful of CMD_AT_CNMI */
		ATQ_CMD_DECLARE_ST(CMD_AT_CNMI, cmd23),		/* New SMS Notification Setting +CNMI=[<mode>[,<mt>[,<bm>[,<ds>[,<bfr>]]]]] */
		ATQ_CMD_DECLARE_ST(CMD_AT_CSQ, cmd24),		/* Query Signal quality */
		};
	unsigned in, out;
	int begin = -1;
	int err;
	char * ptmp1 = NULL;
	char * ptmp2 = NULL;
	pvt_t * pvt = cpvt->pvt;
	at_queue_cmd_t cmds[ITEMS_OF(st_cmds)];

	/* customize list */
	for(in = out = 0; in < ITEMS_OF(st_cmds); in++)
	{
		if(begin == -1)
		{
			if(st_cmds[in].cmd == from_command)
				begin = in;
			else
				continue;
		}

		if(st_cmds[in].cmd == CMD_AT_Z && !CONF_SHARED(pvt, resetdongle))
			continue;
		if(st_cmds[in].cmd == CMD_AT_U2DIAG && CONF_SHARED(pvt, u2diag) == -1)
			continue;

		memcpy(&cmds[out], &st_cmds[in], sizeof(st_cmds[in]));

		if(cmds[out].cmd == CMD_AT_U2DIAG)
		{
			err = at_fill_generic_cmd(&cmds[out], "AT^U2DIAG=%d\r", CONF_SHARED(pvt, u2diag));
			if(err)
				goto failure;
			ptmp1 = cmds[out].data;
		}
		else if(cmds[out].cmd == CMD_AT_CMGF)
		{
			err = at_fill_generic_cmd(&cmds[out], "AT+CMGF=%d\r", CONF_SHARED(pvt, smsaspdu) ? 0 : 1);
			if(err)
				goto failure;
			ptmp2 = cmds[out].data;
		}
		if(cmds[out].cmd == from_command)
			begin = out;
		out++;
	}

	if(out > 0)
		return at_queue_insert(cpvt, cmds, out, 0);
	return 0;
failure:
	if(ptmp1)
		ast_free(ptmp1);
	if(ptmp2)
		ast_free(ptmp2);
	return err;
}

/*!
 * \brief Enque the AT+COPS? command
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */

EXPORT_DEF int at_enque_cops (struct cpvt* cpvt)
{
	static const char cmd[] = "AT+COPS?\r";
	static at_queue_cmd_t at_cmd = ATQ_CMD_DECLARE_ST(CMD_AT_COPS, cmd);

	return at_queue_insert_const(cpvt, &at_cmd, 1, 0);
}


/* SMS sending */
EXPORT_DEF int at_enque_pdu(struct cpvt * cpvt, const char * pdu, attribute_unused const char * u1, attribute_unused unsigned u2, attribute_unused int u3, void ** id)
{
	char * ptr = (char *) pdu;
	char buf[8+25+1];
	at_queue_cmd_t at_cmd[] = {
		{ CMD_AT_CMGS,    RES_SMS_PROMPT, ATQ_CMD_FLAG_DEFAULT, { ATQ_CMD_TIMEOUT_2S, 0}  , NULL, 0 },
		{ CMD_AT_SMSTEXT, RES_OK,         ATQ_CMD_FLAG_DEFAULT, { ATQ_CMD_TIMEOUT_40S, 0} , NULL, 0 }
		};

	size_t length = strlen(pdu);
	size_t pdulen = length;

	int scalen = pdu_parse_sca(&ptr, &pdulen);
	
	if(scalen < 2 || length % 2 != 0)
	{
		return -EINVAL;
	}

	at_cmd[1].data = ast_malloc(length + 2);
	if(!at_cmd[1].data)
	{		
		return -ENOMEM;
	}

	at_cmd[1].length = length + 1;

	memcpy(at_cmd[1].data, pdu, length);
	at_cmd[1].data[length] = 0x1A;
	at_cmd[1].data[length+1] = 0x0;
		
	at_cmd[0].length = snprintf(buf, sizeof(buf), "AT+CMGS=%d\r", (int)(pdulen / 2));
	at_cmd[0].data = ast_strdup(buf);
	if(!at_cmd[0].data)
	{
		ast_free(at_cmd[1].data);
		return -ENOMEM;		
	}
			
/*		ast_debug (5, "[%s] PDU Head '%s'\n", PVT_ID(pvt), buf);
		ast_debug (5, "[%s] PDU Body '%s'\n", PVT_ID(pvt), at_cmd[1].data);
*/
	return at_queue_insert_task(cpvt, at_cmd, ITEMS_OF(at_cmd), 0, (struct at_queue_task **)id);
}

/*!
 * \brief Enque an SMS message
 * \param cpvt -- cpvt structure
 * \param number -- the destination of the message
 * \param msg -- utf-8 encoded message
 */

EXPORT_DEF int at_enque_sms (struct cpvt* cpvt, const char* destination, const char* msg, unsigned validity_minutes, int report_req, void ** id)
{
	ssize_t res;
	char buf[1024] = "AT+CMGS=\"";
	char pdu_buf[2048];
	pvt_t* pvt = cpvt->pvt;
	
	at_queue_cmd_t at_cmd[] = {
		{ CMD_AT_CMGS,    RES_SMS_PROMPT, ATQ_CMD_FLAG_DEFAULT, { ATQ_CMD_TIMEOUT_2S, 0}  , NULL, 0 },
		{ CMD_AT_SMSTEXT, RES_OK,         ATQ_CMD_FLAG_DEFAULT, { ATQ_CMD_TIMEOUT_40S, 0} , NULL, 0 }
		};

	if(pvt->use_pdu)
	{
		/* set default validity period */
		if(validity_minutes <= 0)
			validity_minutes = 3 * 24 * 60;
/*		res = pdu_build(pdu_buf, sizeof(pdu_buf), pvt->sms_scenter, destination, msg, validity_minutes, report_req);
*/
		res = pdu_build(pdu_buf, sizeof(pdu_buf), "", destination, msg, validity_minutes, report_req);
		if(res <= 0)
		{
			if(res == -E2BIG)
			{
			ast_verb (3, "[%s] SMS Message too long, PDU has limit 140 octets\n", PVT_ID(pvt));
			ast_log (LOG_WARNING, "[%s] SMS Message too long, PDU has limit 140 octets\n", PVT_ID(pvt));
			}
			/* TODO: complain on other errors */
			return res;
		}

		if(res > (int)(sizeof(pdu_buf) - 2))
			return -1;

		return at_enque_pdu(cpvt, pdu_buf, NULL, 0, 0, id);
	}
	else
	{
		at_cmd[0].length = 9;

		res = str_recode (RECODE_ENCODE, STR_ENCODING_UCS2_HEX, destination, strlen (destination), buf + at_cmd[0].length, sizeof(buf) - at_cmd[0].length - 3);
		if(res <= 0)
		{
			ast_log (LOG_ERROR, "[%s] Error converting SMS number to UCS-2\n", PVT_ID(pvt));
			return -4;
		}
		at_cmd[0].length += res;
		buf[at_cmd[0].length++] = '"';
		buf[at_cmd[0].length++] = '\r';
		buf[at_cmd[0].length] = '\0';
	}

	at_cmd[0].data = ast_strdup (buf);
	if(!at_cmd[0].data)
		return -ENOMEM;

	res = strlen (msg);

//	if(!pvt->use_pdu)
//	{
		if (pvt->use_ucs2_encoding)
		{
			/* NOTE: bg: i test limit of no response is 133, but for +CMS ERROR: ?  */
			/* message limit in 178 octet of TPDU (w/o SCA) Headers: Type(1)+MR(1)+DA(3..12)+PID(1)+DCS(1)+VP(0,1,7)+UDL(1) = 8..24 (usually 14)  */
			if(res > 70)
			{
				ast_log (LOG_ERROR, "[%s] SMS message too long, 70 symbols max\n", PVT_ID(pvt));
				return -4;
			}

			res = str_recode (RECODE_ENCODE, STR_ENCODING_UCS2_HEX, msg, res, pdu_buf, sizeof(pdu_buf) - 2);
			if (res < 0)
			{
				ast_free (at_cmd[0].data);
				ast_log (LOG_ERROR, "[%s] Error converting SMS to UCS-2: '%s'\n", PVT_ID(pvt), msg);
				return -4;
			}
			pdu_buf[res++] = 0x1a;
			pdu_buf[res] = 0;
			at_cmd[1].length = res;
		}
		else
		{
			if(res > 140)
			{
				ast_log (LOG_ERROR, "[%s] SMS message too long, 140 symbols max\n", PVT_ID(pvt));
				return -4;
			}

			at_cmd[1].length = snprintf (pdu_buf, sizeof(pdu_buf), "%.160s\x1a", msg);
		}
//	}

	at_cmd[1].data = ast_strdup(pdu_buf);
	if(!at_cmd[1].data)
	{
		ast_free(at_cmd[0].data);
		return -ENOMEM;
	}

	return at_queue_insert_task(cpvt, at_cmd, ITEMS_OF(at_cmd), 0, (struct at_queue_task **)id);
}

/*!
 * \brief Enque AT+CUSD.
 * \param cpvt -- cpvt structure
 * \param code the CUSD code to send
 */

EXPORT_DEF int at_enque_ussd (struct cpvt * cpvt, const char * code, attribute_unused const char * u1, attribute_unused unsigned u2, attribute_unused int u3, void ** id)
{
	static const char cmd[] = "AT+CUSD=1,\"";
	static const char cmd_end[] = "\",15\r";
	at_queue_cmd_t at_cmd = ATQ_CMD_DECLARE_DYN(CMD_AT_CUSD);	/* TODO: may be increase timeout ? */
	str_encoding_t cusd_encoding ;
	ssize_t res;
	int length;
	char buf[4096];
	pvt_t* pvt = cpvt->pvt;

	memcpy (buf, cmd, STRLEN(cmd));
	length = STRLEN(cmd);

	if (pvt->cusd_use_7bit_encoding)
		cusd_encoding = STR_ENCODING_7BIT_HEX;
	else if (pvt->use_ucs2_encoding)
		cusd_encoding = STR_ENCODING_UCS2_HEX;
	else
		cusd_encoding = STR_ENCODING_7BIT;
	res = str_recode(RECODE_ENCODE, cusd_encoding, code, strlen (code), buf + STRLEN(cmd), sizeof (buf) - STRLEN(cmd) - STRLEN(cmd_end) - 1);
	if (res <= 0)
	{
		ast_log (LOG_ERROR, "[%s] Error converting USSD code: %s\n", PVT_ID(pvt), code);
		return -1;
	}

	length += res;
	memcpy(buf + length, cmd_end, STRLEN(cmd_end)+1);
	length += STRLEN(cmd_end);

	at_cmd.length = length;
	at_cmd.data = ast_strdup (buf);
	if(!at_cmd.data)
		return -1;

	return at_queue_insert_task(cpvt, &at_cmd, 1, 0, (struct at_queue_task **)id);
}


/*!
 * \brief Enque a DTMF command
 * \param cpvt -- cpvt structure
 * \param digit -- the dtmf digit to send
 * \return -2 if digis is invalid, 0 on success
 */

EXPORT_DEF int at_enque_dtmf (struct cpvt* cpvt, char digit)
{
	switch (digit)
	{
/* unsupported, but AT^DTMF=1,22 OK and "2" sent
*/
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
			return -1974;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

		case '*':
		case '#':
			return at_enque_generic(cpvt, CMD_AT_DTMF, 1, "AT^DTMF=%d,%c\r", cpvt->call_idx, digit);
	}
	return -1;
}

/*!
 * \brief Enque the AT+CCWA command (disable call waiting)
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */

EXPORT_DEF int at_enque_set_ccwa (struct cpvt* cpvt, attribute_unused const char * unused1, attribute_unused const char * unused2, unsigned call_waiting)
{
	static const char cmd_ccwa_get[] = "AT+CCWA=1,2,1\r";
	static const char cmd_ccwa_set[] = "AT+CCWA=%d,%d,%d\r";
	int err;
	call_waiting_t value;
	at_queue_cmd_t cmds[] = {
		/* 5 seconds timeout */
		ATQ_CMD_DECLARE_DYNIT(CMD_AT_CCWA_SET, ATQ_CMD_TIMEOUT_15S, 0),				/* Set Call-Waiting On/Off */
		ATQ_CMD_DECLARE_STIT(CMD_AT_CCWA_STATUS, cmd_ccwa_get, ATQ_CMD_TIMEOUT_15S, 0),		/* Query CCWA Status for Voice Call  */

	};
	at_queue_cmd_t * pcmd = cmds;
	unsigned count = ITEMS_OF(cmds);

	if(call_waiting == CALL_WAITING_DISALLOWED || call_waiting == CALL_WAITING_ALLOWED)
	{
		value = call_waiting;
		err = call_waiting == CALL_WAITING_ALLOWED ? 1 : 0;
		err = at_fill_generic_cmd(&cmds[0], cmd_ccwa_set, err, err, CCWA_CLASS_VOICE);
		if(err)
		    return err;
	}
	else
	{
		value = CALL_WAITING_AUTO;
		pcmd++;
		count--;
	}
	CONF_SHARED(cpvt->pvt, callwaiting) = value;

	return at_queue_insert(cpvt, pcmd, count, 0);
}

/*!
 * \brief Enque the device reset command (AT+CFUN Operation Mode Setting)
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */

EXPORT_DEF int at_enque_reset (struct cpvt* cpvt)
{
	static const char cmd[] = "AT+CFUN=1,1\r";
	static const at_queue_cmd_t at_cmd = ATQ_CMD_DECLARE_ST(CMD_AT_CFUN, cmd);

	return at_queue_insert_const(cpvt, &at_cmd, 1, 0);
}


/*!
 * \brief Enque a dial commands
 * \param cpvt -- cpvt structure
 * \param number -- the called number
 * \param clir -- value of clir
 * \return 0 on success
 */
EXPORT_DEF int at_enque_dial(struct cpvt* cpvt, const char * number, int clir)
{
	struct pvt *pvt = cpvt->pvt;
	int err;
	int cmdsno = 0;
	char * tmp = NULL;
	at_queue_cmd_t cmds[6];

	if(PVT_STATE(pvt, chan_count[CALL_STATE_ACTIVE]) > 0 && CPVT_TEST_FLAG(cpvt, CALL_FLAG_HOLD_OTHER))
	{
		ATQ_CMD_INIT_ST(cmds[0], CMD_AT_CHLD_2, cmd_chld2);
/*  enable this cause response_clcc() see all calls are held and insert 'AT+CHLD=2'
		ATQ_CMD_INIT_ST(cmds[1], CMD_AT_CLCC, cmd_clcc);
*/
		cmdsno = 1;
	}

	if(clir != -1)
	{
		err = at_fill_generic_cmd(&cmds[cmdsno], "AT+CLIR=%d\r", clir);
		if(err)
			return err;
		tmp = cmds[cmdsno].data;
		ATQ_CMD_INIT_DYNI(cmds[cmdsno], CMD_AT_CLIR);
		cmdsno++;
	}

	err = at_fill_generic_cmd(&cmds[cmdsno], "ATD%s;\r", number);
	if(err)
	{
		ast_free(tmp);
		return err;
	}

	ATQ_CMD_INIT_DYNI(cmds[cmdsno], CMD_AT_D);
	cmdsno++;

/* on failed ATD this up held call */
	ATQ_CMD_INIT_ST(cmds[cmdsno], CMD_AT_CLCC, cmd_clcc);
	cmdsno++;

	ATQ_CMD_INIT_ST(cmds[cmdsno], CMD_AT_DDSETEX, cmd_ddsetex2);
	cmdsno++;


	err = at_queue_insert(cpvt, cmds, cmdsno, 1);
/* set CALL_FLAG_NEED_HANGUP early because ATD may be still in queue while local hangup called */
	if(!err)
		CPVT_SET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);

	return err;
}

/*!
 * \brief Enque a answer commands
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */
EXPORT_DEF int at_enque_answer(struct cpvt* cpvt)
{
	at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_DYN(CMD_AT_A),
		ATQ_CMD_DECLARE_ST(CMD_AT_DDSETEX, cmd_ddsetex2),
		};
	int err;
	int count = ITEMS_OF(cmds);
	const char * cmd1;

	if(cpvt->state == CALL_STATE_INCOMING)
	{
/* FIXME: channel number? */
		cmd1 = "ATA\r";
	}
	else if(cpvt->state == CALL_STATE_WAITING)
	{
		cmds[0].cmd = CMD_AT_CHLD_2x;
		cmd1 = "AT+CHLD=2%d\r";
/* no need CMD_AT_DDSETEX in this case? */
		count--;
	}
	else
	{
		ast_log (LOG_ERROR, "[%s] Request answer for call idx %d with state '%s'\n", PVT_ID(cpvt->pvt), cpvt->call_idx, call_state2str(cpvt->state));
		return -1;
	}

	err = at_fill_generic_cmd(&cmds[0], cmd1, cpvt->call_idx);
	if(err == 0)
		err = at_queue_insert(cpvt, cmds, count, 1);
	return err;
}

/*!
 * \brief Enque an activate commands 'Put active calls on hold and activate call x.'
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */
EXPORT_DEF int at_enque_activate (struct cpvt* cpvt)
{
	at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_DYN(CMD_AT_CHLD_2x),
		ATQ_CMD_DECLARE_ST(CMD_AT_CLCC, cmd_clcc),
		};
	int err;

	if (cpvt->state == CALL_STATE_ACTIVE)
		return 0;

	if (cpvt->state != CALL_STATE_ONHOLD && cpvt->state != CALL_STATE_WAITING)
	{
		ast_log (LOG_ERROR, "[%s] Imposible activate call idx %d from state '%s'\n", 
				PVT_ID(cpvt->pvt), cpvt->call_idx, call_state2str(cpvt->state));
		return -1;
	}


	err = at_fill_generic_cmd(&cmds[0], "AT+CHLD=2%d\r", cpvt->call_idx);
	if(err == 0)
		err = at_queue_insert(cpvt, cmds, ITEMS_OF(cmds), 1);
	return err;
}

/*!
 * \brief Enque an commands for 'Put active calls on hold and activate the waiting or held call.'
 * \param pvt -- pvt structure
 * \return 0 on success
 */
EXPORT_DEF int at_enque_flip_hold (struct cpvt* cpvt)
{
	static const at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_ST(CMD_AT_CHLD_2, cmd_chld2),
		ATQ_CMD_DECLARE_ST(CMD_AT_CLCC, cmd_clcc),
		};

	return at_queue_insert_const(cpvt, cmds, ITEMS_OF(cmds), 1);
}

/*!
 * \brief Enque ping command
 * \param pvt -- pvt structure
 * \return 0 on success
 */
EXPORT_DEF int at_enque_ping (struct cpvt * cpvt)
{
	static const at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_STIT(CMD_AT, cmd_at, ATQ_CMD_TIMEOUT_1S, 0),		/* 1 second timeout */
		};

	return at_queue_insert_const(cpvt, cmds, ITEMS_OF(cmds), 1);
}

/*!
 * \brief Enque user-specified command
 * \param cpvt -- cpvt structure
 * \param input -- user's command
 * \return 0 on success
 */
EXPORT_DEF int at_enque_user_cmd(struct cpvt* cpvt, const char * input)
{
	return at_enque_generic(cpvt, CMD_USER, 1, "%s\r", input);
}

/*!
 * \brief Enque commands for reading SMS
 * \param cpvt -- cpvt structure
 * \param index -- index of message in store
 * \param delete -- if non-zero also enque commands for delete message in store after reading
 * \return 0 on success
 */
EXPORT_DEF int at_enque_retrive_sms (struct cpvt* cpvt, int index, int delete)
{
	int err;
	at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_DYN2(CMD_AT_CMGR, RES_CMGR),
		ATQ_CMD_DECLARE_DYN(CMD_AT_CMGD)
		};
	unsigned cmdsno = ITEMS_OF (cmds);

	err = at_fill_generic_cmd (&cmds[0], "AT+CMGR=%d\r", index);
	if (err)
		return err;

	if (delete)
	{
		err = at_fill_generic_cmd (&cmds[1], "AT+CMGD=%d\r\r", index);
		if(err)
		{
			ast_free (cmds[0].data);
			return err;
		}
	}
	else
	{
		cmdsno--;
	}

	return at_queue_insert (cpvt, cmds, cmdsno, 0);
}

/*!
 * \brief Enque AT+CHLD1x or AT+CHUP hangup command
 * \param cpvt -- channel_pvt structure
 * \param call_idx -- call id
 * \return 0 on success
 */

EXPORT_DEF int at_enque_hangup (struct cpvt* cpvt, int call_idx)
{

/*
	this try of hangup non-active (held) channel as workaround for HW BUG 2

	int err;
	at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_ST(CMD_AT_CHLD_2, cmd_chld2),
		ATQ_CMD_DECLARE_DYN(CMD_AT_CHLD_1x),
		};
	at_queue_cmd_t * pcmds = cmds;
	unsigned count = ITEMS_OF(cmds);

	err = at_fill_generic_cmd(&cmds[1], "AT+CHLD=1%d\r", call_idx);
	if(err)
		return err;

	if(cpvt->state != CALL_STATE_ACTIVE)
	{
		pcmds++;
		count--;
	}
	return at_queue_insert(cpvt, pcmds, count, 1);
*/

/*
	HW BUG 1:
	    Sequence
		ATDnum;
		    OK
		    ^ORIG:1,0
		AT+CHLD=11		if this command write to modem E1550 before ^CONF: for ATD device no more write responses to any entered command at all
		    ^CONF:1
	Workaround
		a) send AT+CHUP if possible (single call)
		b) insert fake empty command after ATD expected ^CONF: response if CONF not received yet
	HW BUG 2:
	    Sequence
		ATDnum1;
		    OK
		    ^ORIG:1,0
		    ^CONF:1
		    ^CONN:1,0
		AT+CHLD=2
		    OK
		ATDnum2;
		    OK
		    ^ORIG:2,0
		    ^CONF:2
		    ^CONN:2,0
		AT+CHLD=11		after this command call 1 terminated, but call 2 no voice data and any other new calls created
		    OK
		    ^CEND:1,...
					same result if active call terminated with AT+CHLD=12
					same result if active call terminated by peer side1
	Workaround
		not found yes
*/
/*
	static const struct 
	{
		at_cmd_t	cmd;
		const char	*data;
	} commands[] = 
	{
		{ CMD_AT_CHUP, "AT+CHUP\r" },
		{ CMD_AT_CHLD_1x, "AT+CHLD=1%d\r" }
	};
	int idx = 0;
	if(cpvt == &cpvt->pvt->sys_chan || CPVT_TEST_FLAGS(cpvt, CALL_FLAG_CONF_DONE|CALL_FLAG_IDX_VALID))
	{
		if(cpvt->pvt->chansno > 1)
			idx = 1;
	}

	return at_enque_generic(cpvt, commands[idx].cmd, 1, commands[idx].data, call_idx);
*/
	static const char cmd_chup[] = "AT+CHUP\r";

	struct pvt* pvt = cpvt->pvt;
	int err;
	at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_ST(CMD_AT_CHUP, cmd_chup),
		ATQ_CMD_DECLARE_ST(CMD_AT_CLCC, cmd_clcc),
		};

	if(cpvt == &pvt->sys_chan || cpvt->dir == CALL_DIR_INCOMING || (cpvt->state != CALL_STATE_INIT && cpvt->state != CALL_STATE_DIALING))
	{
		/* FIXME: other channels may be in RELEASED or INIT state */
		if(PVT_STATE(pvt, chansno) > 1)
		{
			cmds[0].cmd = CMD_AT_CHLD_1x;
			err = at_fill_generic_cmd(&cmds[0], cmd_chld1x, call_idx);
			if(err)
				return err;
		}
	}

	/* early AT+CHUP before ^ORIG for outgoing call may not get ^CEND in future */
	if(cpvt->state == CALL_STATE_INIT)
		pvt->last_dialed_cpvt = 0;

	return at_queue_insert(cpvt, cmds, ITEMS_OF(cmds), 1);
}

/*!
 * \brief Enque AT+CLVL commands for volume synchronization
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */

EXPORT_DEF int at_enque_volsync (struct cpvt* cpvt)
{
	static const char cmd1[] = "AT+CLVL=1\r";
	static const char cmd2[] = "AT+CLVL=5\r";
	static const at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_ST(CMD_AT_CLVL, cmd1),
		ATQ_CMD_DECLARE_ST(CMD_AT_CLVL, cmd2),
		};
	return at_queue_insert_const (cpvt, cmds, ITEMS_OF(cmds), 1);
}

/*!
 * \brief Enque AT+CLCC command
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */
EXPORT_DEF int at_enque_clcc (struct cpvt* cpvt)
{
	static const at_queue_cmd_t at_cmd = ATQ_CMD_DECLARE_ST(CMD_AT_CLCC, cmd_clcc);

	return at_queue_insert_const(cpvt, &at_cmd, 1, 1);
}

/*!
 * \brief Enque AT+CHLD=3 command
 * \param cpvt -- cpvt structure
 * \return 0 on success
 */
EXPORT_DEF int at_enque_conference (struct cpvt* cpvt)
{
	static const char cmd_chld3[] = "AT+CHLD=3\r";
	static const at_queue_cmd_t cmds[] = {
		ATQ_CMD_DECLARE_ST(CMD_AT_CHLD_3, cmd_chld3),
		ATQ_CMD_DECLARE_ST(CMD_AT_CLCC, cmd_clcc),
		};

	return at_queue_insert_const(cpvt, cmds, ITEMS_OF(cmds), 1);
}


/*!
 * \brief SEND AT+CHUP command to device IMMEDIALITY
 * \param cpvt -- cpvt structure
 */
EXPORT_DEF void at_hangup_immediality(struct cpvt* cpvt)
{
	char buf[20];
	int length = snprintf(buf, sizeof(buf), cmd_chld1x, cpvt->call_idx);

	if(length > 0)
		at_write(cpvt->pvt, buf, length);
}

