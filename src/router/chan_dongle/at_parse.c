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

#include "memmem.h"

#include <stdio.h>			/* NULL */
#include <errno.h>			/* errno */
#include <stdlib.h>			/* strtol */

#include "at_parse.h"
#include "mutils.h"			/* ITEMS_OF() */
#include "chan_dongle.h"
#include "pdu.h"			/* pdu_parse() */

#/* */
static unsigned mark_line(char * line, const char * delimiters, char * pointers[])
{
	unsigned found = 0;
	
	for(; line[0] && delimiters[found]; line++)
	{
		if(line[0] == delimiters[found])
		{
			pointers[found] = line;
			found++;
		}
	}
	return found;
}

/*!
 * \brief Parse a CNUM response
 * \param str -- string to parse (null terminated)
 * @note str will be modified when the CNUM message is parsed
 * \return NULL on error (parse error) or a pointer to the subscriber number
 */

EXPORT_DEF char * at_parse_cnum (char* str)
{
	/*
	 * parse CNUM response in the following format:
	 * +CNUM: <name>,<number>,<type>
	 *   example
	 *   +CNUM: "Subscriber Number","+79139131234",145
	 *   +CNUM: "Subscriber Number","",145
	 *   +CNUM: "Subscriber Number",,145
	 */

	char delimiters[] = ":,,";
	char * marks[STRLEN(delimiters)];

	/* parse URC only here */
	if(mark_line(str, delimiters, marks) == ITEMS_OF(marks))
	{
		marks[1]++;
		if(marks[1][0] == '"')
			marks[1]++;
		if(marks[2][-1] == '"')
			marks[2]--;
		marks[2][0] = 0;
		return marks[1];
	}

	return NULL;
}

/*!
 * \brief Parse a COPS response
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * @note str will be modified when the COPS message is parsed
 * \return NULL on error (parse error) or a pointer to the provider name
 */

EXPORT_DEF char* at_parse_cops (char* str)
{
	/*
	 * parse COPS response in the following format:
	 * +COPS: <mode>[,<format>,<oper>,<?>]
	 *
	 * example 
	 *  +COPS: 0,0,"TELE2",0
	 */

	char delimiters[] = ":,,,";
	char * marks[STRLEN(delimiters)];

	/* parse URC only here */
	if(mark_line(str, delimiters, marks) == ITEMS_OF(marks))
	{
		marks[2]++;
		if(marks[2][0] == '"')
			marks[2]++;
		if(marks[3][-1] == '"')
			marks[3]--;
		marks[3][0] = 0;
		return marks[2];
	}

	return NULL;
}

/*!
 * \brief Parse a CREG response
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * \param gsm_reg -- a pointer to a int
 * \param gsm_reg_status -- a pointer to a int
 * \param lac -- a pointer to a char pointer which will store the location area code in hex format
 * \param ci  -- a pointer to a char pointer which will store the cell id in hex format
 * @note str will be modified when the CREG message is parsed
 * \retval  0 success
 * \retval -1 parse error
 */

EXPORT_DEF int at_parse_creg (char* str, unsigned len, int* gsm_reg, int* gsm_reg_status, char** lac, char** ci)
{
	unsigned	i;
	int	state;
	char*	p1 = NULL;
	char*	p2 = NULL;
	char*	p3 = NULL;
	char*	p4 = NULL;

	*gsm_reg = 0;
	*gsm_reg_status = -1;
	*lac = NULL;
	*ci  = NULL;

	/*
	 * parse CREG response in the following format:
	 * +CREG: [<p1>,]<p2>[,<p3>,<p4>]
	 */

	for (i = 0, state = 0; i < len && state < 8; i++)
	{
		switch (state)
		{
			case 0:
				if (str[i] == ':')
				{
					state++;
				}
				break;

			case 1:
				if (str[i] != ' ')
				{
					p1 = &str[i];
					state++;
				}
				/* fall through */

			case 2:
				if (str[i] == ',')
				{
					str[i] = '\0';
					state++;
				}
				break;

			case 3:
				if (str[i] != ' ')
				{
					p2 = &str[i];
					state++;
				}
				/* fall through */
			case 4:
				if (str[i] == ',')
				{
					str[i] = '\0';
					state++;
				}
				break;

			case 5:
				if (str[i] != ' ')
				{
					p3 = &str[i];
					state++;
				}
				/* fall through */

			case 6:
				if (str[i] == ',')
				{
					str[i] = '\0';
					state++;
				}
				break;

			case 7:
				if (str[i] != ' ')
				{
					p4 = &str[i];
					state++;
				}
				break;
		}
	}

	if (state < 2)
	{
		return -1;
	}

	if ((p2 && !p3 && !p4) || (p2 && p3 && p4))
	{
		p1 = p2;
	}

	if (p1)
	{
		errno = 0;
		*gsm_reg_status = (int) strtol (p1, (char**) NULL, 10);
		if (*gsm_reg_status == 0 && errno == EINVAL)
		{
			*gsm_reg_status = -1;
			return -1;
		}

		if (*gsm_reg_status == 1 || *gsm_reg_status == 5)
		{
			*gsm_reg = 1;
		}
	}

	if (p2 && p3 && !p4)
	{
		*lac = p2;
		*ci  = p3;
	}
	else if (p3 && p4)
	{
		*lac = p3;
		*ci  = p4;
	}

	return 0;
}

/*!
 * \brief Parse a CMTI notification
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * @note str will be modified when the CMTI message is parsed
 * \return -1 on error (parse error) or the index of the new sms message
 */

EXPORT_DEF int at_parse_cmti (const char* str)
{
	int index;

	/*
	 * parse cmti info in the following format:
	 * +CMTI: <mem>,<index> 
	 */

	return sscanf (str, "+CMTI: %*[^,],%u", &index) == 1 ? index : -1;
}


static const char * parse_cmgr_text(char ** str, size_t len, char * oa, size_t oa_len, str_encoding_t * oa_enc, char ** msg, str_encoding_t * msg_enc)
{
	/*
	 * parse cmgr info in the following TEXT format:
	 * +CMGR: "<msg status>","+123456789",,timestamp<CR><LF>
	 * <message text><CR><LF><CR><LF>
	 * OK<CR><LF>
	 *	or
	 * +CMGR: "<msg status>","002B....",,timestamp<CR><LF>
	 * <message text><CR><LF><CR><LF>
	 * OK<CR><LF>
	 */

	char delimiters[] = ",,,\n";
	char * marks[STRLEN(delimiters)];
	size_t length;
	
	unsigned count = mark_line(*str, delimiters, marks);
	if(count == ITEMS_OF(marks))
	{
		/* unquote number */
		marks[0]++;
		if(marks[0][0] == '"')
			marks[0]++;
		if(marks[1][-1] == '"')
			marks[1]--;
		length = marks[1] - marks[0] + 1;
		if(oa_len < length)
			return "Not enought space for store number";
		*oa_enc = get_encoding(RECODE_DECODE, marks[0], length  - 1);
		marks[1][0] = 0;
		memcpy(oa, marks[0], length);

		*msg = marks[3] + 1;
		length = len - (*msg - *str);
		*msg_enc = get_encoding(RECODE_DECODE, *msg, length);
		return NULL;
	}
	else if(count > 0)
		*str = marks[count - 1];

	return "Can't parse +CMGR response text";
}

static const char* parse_cmgr_pdu(char** str, attribute_unused size_t len, char* oa, size_t oa_len, str_encoding_t* oa_enc, char** msg, str_encoding_t* msg_enc)
{
	/*
	 * parse cmgr info in the following PDU format
	 * +CMGR: message_status,[address_text],TPDU_length<CR><LF>
	 * SMSC_number_and_TPDU<CR><LF><CR><LF>
	 * OK<CR><LF>
	 *
	 *	sample
	 * +CMGR: 1,,31
	 * 07911234567890F3040B911234556780F20008012150220040210C041F04400438043204350442<CR><LF><CR><LF>
	 * OK<CR><LF>
	 */

	char delimiters[] = ",,\n";
	char * marks[STRLEN(delimiters)];
	char * end;
	size_t tpdu_length;

	if(mark_line(*str, delimiters, marks) == ITEMS_OF(marks))
	{
		tpdu_length = strtol(marks[1] + 1, &end, 10);
		if(tpdu_length <= 0 || end[0] != '\r')
			return "Invalid TPDU length in CMGR PDU status line";
		*str = marks[2] + 1;
		return pdu_parse(str, tpdu_length, oa, oa_len, oa_enc, msg, msg_enc);
	}

	return "Can't parse +CMGR response";
}

/*!
 * \brief Parse a CMGR message
 * \param str -- pointer to pointer of string to parse (null terminated)
 * \param len -- string lenght
 * \param number -- a pointer to a char pointer which will store the from number
 * \param text -- a pointer to a char pointer which will store the message text
 * @note str will be modified when the CMGR message is parsed
 * \retval  0 success
 * \retval -1 parse error
 */

EXPORT_DEF const char * at_parse_cmgr(char ** str, size_t len, char * oa, size_t oa_len, str_encoding_t * oa_enc, char ** msg, str_encoding_t * msg_enc)
{
	const char* rv = "Can't parse +CMGR response line";

	/* skip "+CMGR:" */
	*str += 6;
	len -= 6;

	/* skip leading spaces */
	while(len > 0 && str[0][0] == ' ')
	{
		(*str)++;
		len--;
	}

	if(len > 0)
	{
		/* check PDU or TEXT mode */
		const char* (*fptr)(char** str, size_t len, char* num, size_t num_len, str_encoding_t * oa_enc, char** msg, str_encoding_t * msg_enc);
		fptr = str[0][0] == '"' ? parse_cmgr_text : parse_cmgr_pdu;

		rv = (*fptr)(str, len, oa, oa_len, oa_enc, msg, msg_enc);
	}

	return rv;
}

 /*!
 * \brief Parse a CUSD answer
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * @note str will be modified when the CUSD string is parsed
 * \retval  0 success
 * \retval -1 parse error
 */

EXPORT_DEF int at_parse_cusd (char* str, int * type, char** cusd, int * dcs)
{
	/*
	 * parse cusd message in the following format:
	 * +CUSD: <m>,[<str>,<dcs>]
	 * 
	 * examples
	 *   +CUSD: 5
	 *   +CUSD: 0,"100,00 EURO, valid till 01.01.2010, you are using tariff "Mega Tariff". More informations *111#.",15
	 */

	char delimiters[] = ":,,";
	char * marks[STRLEN(delimiters)];
	unsigned count;

	*type = -1;
	*cusd = "";
	*dcs = -1;

	count = mark_line(str, delimiters, marks);
// 0, 1, 2, 3
	if(count > 0)
	{
		if(sscanf(marks[0] + 1, "%u", type) == 1)
		{
			if(count > 1)
			{
				marks[1]++;
				if(marks[1][0] == '"')
					marks[1]++;
				*cusd = marks[1];

				if(count > 2) {
					sscanf(marks[2] + 1, "%u", dcs);
					if(marks[2][-1] == '"')
						marks[2]--;
					marks[2][0] = 0;
				} else {
					int len = strlen(*cusd);
					if(len > 0 && (*cusd)[len - 1] == '"')
						(*cusd)[len-1] = 0;
				}
			}
			return 0;
		}
	}
	return -1;
}

/*!
 * \brief Parse a CPIN notification
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * \return  2 if PUK required
 * \return  1 if PIN required
 * \return  0 if no PIN required
 * \return -1 on error (parse error) or card lock
 */

EXPORT_DEF int at_parse_cpin (char* str, size_t len)
{
	static const struct {
		const char	* value;
		unsigned	length;
	} resp[] = {
		{ "READY", 5 },
		{ "SIM PIN", 7 },
		{ "SIM PUK", 7 },
	};

	unsigned idx;
	for(idx = 0; idx < ITEMS_OF(resp); idx++)
	{
		if(memmem (str, len, resp[idx].value, resp[idx].length) != NULL)
			return idx;
	}
	return -1;
}

/*!
 * \brief Parse +CSQ response
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * \retval  0 success
 * \retval -1 error
 */

EXPORT_DEF int at_parse_csq (const char* str, int* rssi)
{
	/*
	 * parse +CSQ response in the following format:
	 * +CSQ: <RSSI>,<BER>
	 */

	return sscanf (str, "+CSQ:%2d,", rssi) == 1 ? 0 : -1;
}

/*!
 * \brief Parse a ^RSSI notification
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * \return -1 on error (parse error) or the rssi value
 */

EXPORT_DEF int at_parse_rssi (const char* str)
{
	int rssi = -1;

	/*
	 * parse RSSI info in the following format:
	 * ^RSSI:<rssi>
	 */

	sscanf (str, "^RSSI:%d", &rssi);
	return rssi;
}

/*!
 * \brief Parse a ^MODE notification (link mode)
 * \param str -- string to parse (null terminated)
 * \param len -- string lenght
 * \return -1 on error (parse error) or the the link mode value
 */

EXPORT_DEF int at_parse_mode (char * str, int * mode, int * submode)
{
	/*
	 * parse RSSI info in the following format:
	 * ^MODE:<mode>,<submode>
	 */

	return sscanf (str, "^MODE:%d,%d", mode, submode) == 2 ? 0 : -1;
}

#/* */
EXPORT_DEF int at_parse_csca(char* str, char ** csca)
{
	/*
	 * parse CSCA info in the following format:
	 * +CSCA: <SCA>,<TOSCA>
	 *  +CSCA: "+79139131234",145
	 *  +CSCA: "",145
	 */
	char delimiters[] = "\"\"";
	char * marks[STRLEN(delimiters)];

	if(mark_line(str, delimiters, marks) == ITEMS_OF(marks))
	{
		*csca = marks[0] + 1;
		marks[1][0] = 0;
		return 0;
	}

	return -1;
}

#/* */
EXPORT_DEF int at_parse_clcc(char* str, unsigned * call_idx, unsigned * dir, unsigned * state, unsigned * mode, unsigned * mpty, char ** number, unsigned * toa)
{
	/*
	 * +CLCC:<id1>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>[,<priority>]]]\r\n
	 *  ...
	 * +CLCC:<id1>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>[,<priority>]]]\r\n
	 *  examples
	 *   +CLCC: 1,1,4,0,0,"",145
	 *   +CLCC: 1,1,4,0,0,"+79139131234",145
	 *   +CLCC: 1,1,4,0,0,"0079139131234",145
	 *   +CLCC: 1,1,4,0,0,"+7913913ABCA",145
	 */
	char delimiters[] = ":,,,,,,";
	char * marks[STRLEN(delimiters)];

	*call_idx = 0;
	*dir = 0;
	*state = 0;
	*mode = 0;
	*mpty = 0;
	*number = "";
	*toa = 0;

	if(mark_line(str, delimiters, marks) == ITEMS_OF(marks))
	{
		if( sscanf(marks[0] + 1, "%u", call_idx) == 1
			&&
		    sscanf(marks[1] + 1, "%u", dir) == 1
			&&
		    sscanf(marks[2] + 1, "%u", state) == 1
			&&
		    sscanf(marks[3] + 1, "%u", mode) == 1
			&&
		    sscanf(marks[4] + 1, "%u", mpty) == 1
			&&
		    sscanf(marks[6] + 1, "%u", toa) == 1) 
		{
			marks[5]++;
			if(marks[5][0] == '"')
				marks[5]++;
			if(marks[6][-1] == '"')
				marks[6]--;
			*number = marks[5];
			marks[6][0] = 0;

			return 0;
		}
	}

	return -1;
}

#/* */
EXPORT_DEF int at_parse_ccwa(char* str, unsigned * class)
{
	/* 
	 * CCWA may be in form:
	 *	in response of AT+CCWA=?
	 *		+CCWA: (0,1)
	 *	in response of AT+CCWA=?
	 *		+CCWA: <n>
	 *	in response of "AT+CCWA=[<n>[,<mode>[,<class>]]]"
	 *		+CCWA: <status>,<class1>
	 *
	 *	unsolicited result code
	 *		+CCWA: <number>,<type>,<class>,[<alpha>][,<CLI validity>[,<subaddr>,<satype>[,<priority>]]]
	 */
	char delimiters[] = ":,,";
	char * marks[STRLEN(delimiters)];

	/* parse URC only here */
	if(mark_line(str, delimiters, marks) == ITEMS_OF(marks))
	{
		if(sscanf(marks[2] + 1, "%u", class) == 1)
			return 0;
	}

	return -1;
}
