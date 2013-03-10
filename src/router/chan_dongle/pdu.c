/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <errno.h>			/* EINVAL ENOMEM E2BIG */

#include "pdu.h"
#include "helpers.h"			/* dial_digit_code() */
#include "char_conv.h"			/* utf8_to_hexstr_ucs2() */

/* SMS-SUBMIT format
	SCA		1..12 octet(s)		Service Center Address information element
	  octets
	        1		Length of Address (minimal 0)
	        2		Type of Address
	    3  12		Address

	PDU-type	1 octet			Protocol Data Unit Type
	  bits
	      1 0 MTI	Message Type Indicator Parameter describing the message type 00 means SMS-DELIVER 01 means SMS-SUBMIT
				0 0	SMS-DELIVER (SMSC ==> MS)
						or
					SMS-DELIVER REPORT (MS ==> SMSC, is generated automatically by the MOBILE, after receiving a SMS-DELIVER)

				0 1	SMS-SUBMIT (MS ==> SMSC)
						or
					SMS-SUBMIT REPORT (SMSC ==> MS)

				1 0	SMS-STATUS REPORT (SMSC ==> MS)
						or
					SMS-COMMAND (MS ==> SMSC)
				1 1	Reserved

		2 RD	Reject Duplicate
				0    Instruct the SMSC to accept an SMS-SUBMIT for an short message still
					held in the SMSC which has the same MR and
				1    Instruct the SMSC to reject an SMS-SUBMIT for an short message still
					held in the SMSC which has the same MR and DA as a previosly
					submitted short message from the same OA.

	      4 3 VPF	Validity Period Format	Parameter indicating whether or not the VP field is present
	      			0 0	VP field is not present
	      			0 1	Reserved
	      			1 0	VP field present an integer represented (relative)
	      			1 1	VP field present an semi-octet represented (absolute)

		5 SRR	Status Report Request Parameter indicating if the MS has requested a status report
				0	A status report is not requested
				1	A status report is requested

		6 UDHI	User Data Header Indicator Parameter indicating that the UD field contains a header
				0	The UD field contains only the short message
				1	The beginning of the UD field contains a header in addition of the short message
		7 RP	Reply Path Parameter indicating that Reply Path exists
				0	Reply Path parameter is not set in this PDU
				1	Reply Path parameter is set in this PDU

	MR		1 octet		Message Reference
						The MR field gives an integer (0..255) representation of a reference number of the SMSSUBMIT submitted to the SMSC by the MS.
						! notice: at the MOBILE the MR is generated automatically, -anyway you have to generate it a possible entry is for example ”00H” !
	DA		2-12 octets	Destination Address
	  octets
		1	Length of Address (of BCD digits!)
		2	Type of Address
	     3 12	Address
	PID		1 octet		Protocol Identifier
						The PID is the information element by which the Transport Layer either refers to the higher
						layer protocol being used, or indicates interworking with a certain type of telematic device.
						here are some examples of PID codings:
		00H: The PDU has to be treat as a short message
		41H: Replace Short Message Type1
		  ....
		47H: Replace Short Message Type7
					Another description:

		Bit7 bit6 (bit 7 = 0, bit 6 = 0)
		l 0 0 Assign bits 0..5, the values are defined as follows.
		l 1 0 Assign bits 0..5, the values are defined as follows.
		l 0 1 Retain
		l 1 1 Assign bits 0..5 for special use of SC
		Bit5 values:
		l 0: No interworking, but SME-to-SME protocol
		l 1: Telematic interworking (in this situation , value of bits4...0 is
		valid)
		Interface Description for HUAWEI EV-DO Data Card AT Commands
		All rights reserved Page 73 , Total 140
		Bit4...Bit0: telematic devices type identifier. If the value is 1 0 0 1 0, it
		indicates email. Other values are not supported currently.


	DCS		1 octet			Data Coding Scheme

	VP		0,1,7 octet(s)		Validity Period
	UDL		1 octet			User Data Length
	UD		0-140 octets		User Data
*/

/* SMS-DELIVER format
	SCA		1..12 octet(s)		Service Center Address information element
	  octets
	        1		Length of Address (minimal 0)
	        2		Type of Address
	    3  12		Address

	PDU-type	1 octet			Protocol Data Unit Type
	  bits
	      1 0 MTI	Message Type Indicator Parameter describing the message type 00 means SMS-DELIVER 01 means SMS-SUBMIT
				0 0	SMS-DELIVER (SMSC ==> MS)
						or
					SMS-DELIVER REPORT (MS ==> SMSC, is generated automatically by the MOBILE, after receiving a SMS-DELIVER)

				0 1	SMS-SUBMIT (MS ==> SMSC)
						or
					SMS-SUBMIT REPORT (SMSC ==> MS)

				1 0	SMS-STATUS REPORT (SMSC ==> MS)
						or
					SMS-COMMAND (MS ==> SMSC)
				1 1	Reserved

		2 MMS	More Messages to Send	Parameter indicating whether or not there are more messages to send
				0 More messages are waiting for the MS in the SMSC
				1 No more messages are waiting for the MS in the SMSC
	      4 3 Reserved

		5 SRI	Status Report Indication	Parameter indicating if the SME has requested a status report
				0 A status report will not be returned to the SME
				1 A status report will be returned to the SME

		6 UDHI	User Data Header Indicator Parameter indicating that the UD field contains a header
				0	The UD field contains only the short message
				1	The beginning of the UD field contains a header in addition of the short message
		7 RP	Reply Path Parameter indicating that Reply Path exists
				0	Reply Path parameter is not set in this PDU
				1	Reply Path parameter is set in this PDU

	OA		2-12 octets	Originator Address
	  octets
		1	Length of Address (of BCD digits!)
		2	Type of Address
	     3 12	Address
	PID		1 octet		Protocol Identifier
						The PID is the information element by which the Transport Layer either refers to the higher
						layer protocol being used, or indicates interworking with a certain type of telematic device.
						here are some examples of PID codings:
		00H: The PDU has to be treat as a short message
		41H: Replace Short Message Type1
		  ....
		47H: Replace Short Message Type7
					Another description:

		Bit7 bit6 (bit 7 = 0, bit 6 = 0)
		l 0 0 Assign bits 0..5, the values are defined as follows.
		l 1 0 Assign bits 0..5, the values are defined as follows.
		l 0 1 Retain
		l 1 1 Assign bits 0..5 for special use of SC
		Bit5 values:
		l 0: No interworking, but SME-to-SME protocol
		l 1: Telematic interworking (in this situation , value of bits4...0 is
		valid)
		Interface Description for HUAWEI EV-DO Data Card AT Commands
		All rights reserved Page 73 , Total 140
		Bit4...Bit0: telematic devices type identifier. If the value is 1 0 0 1 0, it
		indicates email. Other values are not supported currently.


	DCS		1 octet			Data Coding Scheme

	SCTS		7 octets		Service Center Time Stamp
	UDL		1 octet			User Data Length
	UD		0-140 octets		User Data, may be prepended by User Data Header see UDHI flag
	    octets
	    	1 opt UDHL	Total number of Octets in UDH
	    	? IEIa
	    	? IEIDLa
	    	? IEIDa
	    	? IEIb
		  ...
*/

#define NUMBER_TYPE_INTERNATIONAL		0x91

/* Message Type Indicator Parameter */
#define PDUTYPE_MTI_SHIFT			0
#define PDUTYPE_MTI_SMS_DELIVER			(0x00 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI_SMS_DELIVER_REPORT		(0x00 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI_SMS_SUBMIT			(0x01 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI_SMS_SUBMIT_REPORT		(0x01 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI_SMS_STATUS_REPORT		(0x02 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI_SMS_COMMAND			(0x02 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI_RESERVED			(0x03 << PDUTYPE_MTI_SHIFT)

#define PDUTYPE_MTI_MASK			(0x03 << PDUTYPE_MTI_SHIFT)
#define PDUTYPE_MTI(pdutype)			((pdutype) & PDUTYPE_MTI_MASK)

/* Reject Duplicate */
#define PDUTYPE_RD_SHIFT			2
#define PDUTYPE_RD_ACCEPT			(0x00 << PDUTYPE_RD_SHIFT)
#define PDUTYPE_RD_REJECT			(0x01 << PDUTYPE_RD_SHIFT)

/* Validity Period Format */
#define PDUTYPE_VPF_SHIFT			3
#define PDUTYPE_VPF_NOT_PRESENT			(0x00 << PDUTYPE_VPF_SHIFT)
#define PDUTYPE_VPF_RESERVED			(0x01 << PDUTYPE_VPF_SHIFT)
#define PDUTYPE_VPF_RELATIVE			(0x02 << PDUTYPE_VPF_SHIFT)
#define PDUTYPE_VPF_ABSOLUTE			(0x03 << PDUTYPE_VPF_SHIFT)

/* Status Report Request */
#define PDUTYPE_SRR_SHIFT			5
#define PDUTYPE_SRR_NOT_REQUESTED		(0x00 << PDUTYPE_SRR_SHIFT)
#define PDUTYPE_SRR_REQUESTED			(0x01 << PDUTYPE_SRR_SHIFT)

/* User Data Header Indicator */
#define PDUTYPE_UDHI_SHIFT			6
#define PDUTYPE_UDHI_NO_HEADER			(0x00 << PDUTYPE_UDHI_SHIFT)
#define PDUTYPE_UDHI_HAS_HEADER			(0x01 << PDUTYPE_UDHI_SHIFT)
#define PDUTYPE_UDHI_MASK			(0x01 << PDUTYPE_UDHI_SHIFT)
#define PDUTYPE_UDHI(pdutype)			((pdutype) & PDUTYPE_UDHI_MASK)

/* eply Path Parameter */
#define PDUTYPE_RP_SHIFT			7
#define PDUTYPE_RP_IS_NOT_SET			(0x00 << PDUTYPE_RP_SHIFT)
#define PDUTYPE_RP_IS_SET			(0x01 << PDUTYPE_RP_SHIFT)

#define PDU_MESSAGE_REFERENCE			0x00		/* assigned by MS */

#define PDU_PID_SMS				0x00		/* bit5 No interworking, but SME-to-SME protocol = SMS */
#define PDU_PID_EMAIL				0x32		/* bit5 Telematic interworking, bits 4..0 0x 12  = email */

/* DCS */
/*   bits 1..0 Class */
#define PDU_DCS_CLASS_SHIFT			0
#define PDU_DCS_CLASS0				(0x00 << PDU_DCS_CLASS_SHIFT)	/* Class 0, provides display and responds SC */
#define PDU_DCS_CLASS1				(0x01 << PDU_DCS_CLASS_SHIFT)	/* Class 1, saves to MS (NV); or saves to the UIM card when MS is full */
#define PDU_DCS_CLASS2				(0x02 << PDU_DCS_CLASS_SHIFT)	/* Class 2, dedicated for the UIM card The storage status is reported to SC after storage If the UIM card is full, failure and reason are reported to SC */
#define PDU_DCS_CLASS3				(0x03 << PDU_DCS_CLASS_SHIFT)	/* Class 3, stored to TE, MS receives messages and does not sent to TE, but responds to SC */
#define PDU_DCS_CLASS_MASK			(0x03 << PDU_DCS_CLASS_SHIFT)
#define PDU_DCS_CLASS(dcs)			((dcs) & PDU_DCS_CLASS_MASK)

/*   bits 3..2 Alpabet */
#define PDU_DCS_ALPABET_SHIFT			2
#define PDU_DCS_ALPABET_7BIT			(0x00 << PDU_DCS_ALPABET_SHIFT)
#define PDU_DCS_ALPABET_8BIT			(0x01 << PDU_DCS_ALPABET_SHIFT)
#define PDU_DCS_ALPABET_UCS2			(0x02 << PDU_DCS_ALPABET_SHIFT)
#define PDU_DCS_ALPABET_MASK			(0x03 << PDU_DCS_ALPABET_SHIFT)
#define PDU_DCS_ALPABET(dcs)			((dcs) & PDU_DCS_ALPABET_MASK)

/*   bit 4 */
#define PDU_DCS_BITS10_CTRL_SHIFT		4
#define PDU_DCS_BITS10_RETAIN			(0x00 << PDU_DCS_BIT10_CTRL_SHIFT)
#define PDU_DCS_BITS10_INUSE			(0x01 << PDU_DCS_BIT10_CTRL_SHIFT)
#define PDU_DCS_BITS10_CTRL_MASK		(0x01 << PDU_DCS_BIT10_CTRL_SHIFT)
#define PDU_DCS_BITS10_CTRL(dcs)		((dcs) & PDU_DCS_BITS10_CTRL_MASK)

/*   bit 5 */
#define PDU_DCS_COMPRESSION_SHIFT		5
#define PDU_DCS_NOT_COMPESSED			(0x00 << PDU_DCS_COMPRESSION_SHIFT)
#define PDU_DCS_COMPESSED			(0x01 << PDU_DCS_COMPRESSION_SHIFT)
#define PDU_DCS_COMPRESSION_MASK		(0x01 << PDU_DCS_COMPRESSION_SHIFT)
#define PDU_DCS_COMPRESSION(dcs)		((dcs) & PDU_DCS_COMPRESSION_MASK)

/*   bit 7..6 */
#define PDU_DCS_76_SHIFT			6
#define PDU_DCS_76_00				(0x00 << PDU_DCS_76_SHIFT)
#define PDU_DCS_76_MASK				(0x03 << PDU_DCS_76_SHIFT)
#define PDU_DCS_76(dcs)				((dcs) & PDU_DCS_76_MASK)

#define ROUND_UP2(x)				(((x) + 1) & (0xFFFFFFFF << 1))
#define LENGTH2OCTETS(x)			(((x) + 1)/2)

#/* get digit code, 0 if invalid  */
EXPORT_DEF char pdu_digit2code(char digit)
{
	switch(digit)
	{
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
			break;
		case '*':
			digit = 'A';
			break;
		case '#':
			digit = 'B';
			break;
		case 'a':
		case 'A':
			digit = 'C';
			break;
		case 'b':
		case 'B':
			digit = 'D';
			break;
		case 'c':
		case 'C':
			digit = 'E';
			break;
		default:
			return 0;
	}
	return digit;
}

#/* */
static char pdu_code2digit(char code)
{
	switch(code)
	{
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
			break;
		case 'a':
		case 'A':
			code = '*';
			break;
		case 'b':
		case 'B':
			code = '#';
			break;
		case 'c':
		case 'C':
			code = 'A';
			break;
		case 'd':
		case 'D':
			code = 'B';
			break;
		case 'e':
		case 'E':
			code = 'C';
			break;
		case 'F':
			return 0;
		default:
			return -1;
	}
	return code;
}

#/* convert minutes to relative VP value */
static int pdu_relative_validity(unsigned minutes)
{
#define DIV_UP(x,y)	(((x)+(y)-1)/(y))
/*
	0 ... 143  (vp + 1) * 5 minutes				   5  ...   720		m = (vp + 1) * 5		m / 5 - 1 = vp
	144...167  12 hours + (vp - 143) * 30 minutes		 750  ...  1440		m = 720 + (vp - 143) * 30	(m - 720) / 30 + 143 = m / 30 + 119
	168...196  (vp - 166) * 1 day				2880  ... 43200		m = (vp - 166) * 1440		(m / 1440) + 166
	197...255  (vp - 192) * 1 week			       50400  ...635040		m = (vp - 192) * 10080		(m / 10080) + 192
*/
	int validity;
	if(minutes <= 720)
		validity = DIV_UP(minutes, 5) - 1;
	else if(minutes <= 1440)
		validity = DIV_UP(minutes, 30) + 119;
	else if(minutes <= 43200)
		validity = DIV_UP(minutes, 1440) + 166;
	else if(minutes <= 635040)
		validity = DIV_UP(minutes, 10080) + 192;
	else
		validity = 0xFF;
	return validity;
#undef DIV_UP
}

#/* convert 2 hex digits of PDU to byte, return < 0 on error */
static int pdu_parse_byte(char ** digits2hex, size_t * length)
{
	int res = -1;
	int res2;

	if(*length >= 2)
	{
		res = parse_hexdigit(*digits2hex[0]);
		if(res >= 0)
		{
			(*digits2hex)++;
			(*length)--;
			res2 = parse_hexdigit(*digits2hex[0]);
			if(res2 >= 0)
			{
				(*digits2hex)++;
				(*length)--;
				return (res << 4) | res2;
			}
		}
	}
	return res;
}

/*!
 * \brief Store number in PDU 
 * \param buffer -- pointer to place where number will be stored, CALLER MUST be provide length + 2 bytes of buffer
 * \param number -- phone number w/o leading '+'
 * \param length -- length of number
 * \return number of bytes written to buffer
 */
static int pdu_store_number(char* buffer, const char* number, unsigned length)
{
	int i;
	for(i = 0; length > 1; length -=2, i +=2)
	{
		buffer[i] = pdu_digit2code(number[i + 1]);
		buffer[i + 1] = pdu_digit2code(number[i]);
	}

	if(length)
	{
		buffer[i] = 'F';
		buffer[i+1] = pdu_digit2code(number[i]);
		i += 2;
	}
	return i;
}

/*
failed parse 07 91  97 62 02 00 01 F9  44  14 D0 F7 FB DD D5 2E 9F C3 E6 B7 1B  0008117050815073618C0500037A020100680066006C0067006800200066006800670020006800640066006A006C006700680066006400680067000A002F00200415043604350434043D04350432043D044B04390020043B04380447043D044B043900200433043E0440043E0441043A043E043F003A0020002A003500300035002300360023002000200028003300200440002F0441 

                                              ^^  not a international format
failed parse 07 91  97 30 07 11 11 F1  04  14 D0 D9B09B5CC637DFEE721E0008117020616444617E041A043E04340020043F043E04340442043204350440043604340435043D0438044F003A00200036003900320037002E0020041D0438043A043E043C04430020043D043500200441043E043E043104490430043904420435002C002004320432043504340438044204350020043D0430002004410430043904420435002E 
                                              ^^  not a international format
*/
#/* reverse of pdu_store_number() */
static int pdu_parse_number(char ** pdu, size_t * pdu_length, unsigned digits, int * toa, char * number, size_t num_len)
{
	const char * begin;

	if(num_len < digits + 1)
		return -ENOMEM;

	begin = *pdu;
	*toa = pdu_parse_byte(pdu, pdu_length);
	if(*toa >= 0)
	{
		unsigned syms = ROUND_UP2(digits);
		if(syms <= *pdu_length)
		{
			char digit;
			if(*toa == NUMBER_TYPE_INTERNATIONAL)
				*number++ = '+';
			for(; syms > 0; syms -= 2, *pdu += 2, *pdu_length -= 2)
			{
				digit = pdu_code2digit(pdu[0][1]);
				if(digit <= 0)
					return -1;
				*number++ = digit;

				digit = pdu_code2digit(pdu[0][0]);
				if(digit < 0 || (digit == 0 && (syms != 2 || (digits & 0x1) == 0)))
					return -1;

				*number++ = digit;
			}
			if((digits & 0x1) == 0)
				*number = 0;
			return *pdu - begin;
		}
	}

	return -EINVAL;
}


#/* return bytes (not octets!) of pdu occupied by SCA or <0 on errors */
EXPORT_DEF int pdu_parse_sca(char ** pdu, size_t * length)
{
	/* get length of SCA field */
	int sca_len = pdu_parse_byte(pdu, length);

	if(sca_len >= 0)
	{
		sca_len *= 2;
		if((size_t)sca_len <= *length)
		{
			*pdu += sca_len;
			*length -= sca_len;

			/* TODO: Parse SCA Address */
			return sca_len + 2;
		}
	}
	return -EINVAL;
}

#/* TODO: implement */
static int pdu_parse_timestamp(char ** pdu, size_t * length)
{
	if(*length >= 14)
	{
		*pdu += 14;
		*length -= 14;
		return 14;
	}
	return -EINVAL;
}

#/* TODO: remove / TODO: append 8 bit */
static int check_encoding(const char* msg, unsigned length)
{
	str_encoding_t possible_enc = get_encoding(RECODE_ENCODE, msg, length);
	if(possible_enc == STR_ENCODING_7BIT_HEX)
		return PDU_DCS_ALPABET_7BIT;
	return PDU_DCS_ALPABET_UCS2;
}

/*!
 * \brief Build PDU text for SMS
 * \param buffer -- pointer to place where PDU will be stored
 * \param length -- length of buffer
 * \param sca -- number of SMS center may be with leading '+' in International format
 * \param dst -- destination number for SMS may be with leading '+' in International format
 * \param msg -- SMS message in utf-8
 * \param valid_minutes -- Validity period
 * \param srr -- Status Report Request
 * \param sca_len -- pointer where length of SCA header (in bytes) will be stored
 * \return number of bytes written to buffer w/o trailing 0x1A or 0, -ENOMEM if buffer too short, -EINVAL on iconv recode errors, -E2BIG if message too long
 */
#/* */
EXPORT_DEF int pdu_build(char* buffer, size_t length, const char* sca, const char* dst, const char* msg, unsigned valid_minutes, int srr)
{
	char tmp;
	int len = 0;
	int data_len;

	int sca_toa = NUMBER_TYPE_INTERNATIONAL;
	int dst_toa = NUMBER_TYPE_INTERNATIONAL;
	int pdutype = PDUTYPE_MTI_SMS_SUBMIT | PDUTYPE_RD_ACCEPT | PDUTYPE_VPF_RELATIVE | PDUTYPE_SRR_NOT_REQUESTED | PDUTYPE_UDHI_NO_HEADER | PDUTYPE_RP_IS_NOT_SET;
	int dcs;

	unsigned dst_len;
	unsigned sca_len;
	unsigned msg_len;

	/* detect msg encoding and use 7Bit or UCS-2, not use 8Bit */
	msg_len = strlen(msg);
	dcs = check_encoding(msg, msg_len);

	/* cannot exceed 140 octets for not compressed or cannot exceed 160 septets for compressed */
/*
	if(((PDU_DCS_ALPABET(dcs) == PDU_DCS_ALPABET_UCS2) && msg_len > 70) || (PDU_DCS_ALPABET(dcs) == PDU_DCS_ALPABET_8BIT && msg_len > 140) || msg_len > 160)
	{
		return -E2BIG;
	}
*/
	if(sca[0] == '+')
		sca++;

	if(dst[0] == '+')
		dst++;

	/* count length of strings */
	sca_len = strlen(sca);
	dst_len = strlen(dst);

	/* check buffer has enougth space */
	if(length < ((sca_len == 0 ? 2 : 4 + ROUND_UP2(sca_len)) + 8 + ROUND_UP2(dst_len) + 8 + msg_len * 4 + 4))
		return -ENOMEM;

	/* SCA Length */
	/* Type-of-address of the SMSC */
	/* Address of SMSC */
	if(sca_len)
	{
		len += snprintf(buffer + len, length - len, "%02X%02X", 1 + LENGTH2OCTETS(sca_len), sca_toa);
		len += pdu_store_number(buffer + len, sca, sca_len);
	}
	else
	{
		buffer[len++] = '0';
		buffer[len++] = '0';
	}
	sca_len = len;

	if(srr)
		pdutype |= PDUTYPE_SRR_REQUESTED;

	/* PDU-type */
	/* TP-Message-Reference. The "00" value here lets the phone set the message reference number itself */
	/* Address-Length */
	/* Type-of-address of the sender number */
	len += snprintf(buffer + len, length - len, "%02X%02X%02X%02X", pdutype, PDU_MESSAGE_REFERENCE, dst_len, dst_toa);

	/*  Destination address */
	len += pdu_store_number(buffer + len, dst, dst_len);

	/* forward TP-User-Data */
	data_len = str_recode(RECODE_ENCODE, dcs == PDU_DCS_ALPABET_UCS2 ? STR_ENCODING_UCS2_HEX : STR_ENCODING_7BIT_HEX, msg, msg_len, buffer + len + 8, length - len - 11);
	if(data_len < 0)
	{
		return -EINVAL;
	}
	else if(data_len > 160 * 2)
	{
		return -E2BIG;
	}

	/* calc UDL */
	if(dcs == PDU_DCS_ALPABET_UCS2)
		msg_len = data_len / 2;

	/* TP-PID. Protocol identifier  */
	/* TP-DCS. Data coding scheme */
	/* TP-Validity-Period */
	/* TP-User-Data-Length */
	tmp = buffer[len + 8];
	len += snprintf(buffer + len, length - len, "%02X%02X%02X%02X", PDU_PID_SMS, dcs, pdu_relative_validity(valid_minutes), msg_len);
	buffer[len] = tmp;

	len += data_len;

	/* also check message limit in 178 octets of TPDU (w/o SCA) */
	if(len - sca_len > 178 * 2)
	{
		return -E2BIG;
	}

	return len;
}


#/* */
static str_encoding_t pdu_dcs_alpabet2encoding(int alpabet)
{
	str_encoding_t rv = STR_ENCODING_UNKNOWN;

	alpabet >>= PDU_DCS_ALPABET_SHIFT;
	switch(alpabet)
	{
		case (PDU_DCS_ALPABET_7BIT >> PDU_DCS_ALPABET_SHIFT):
			rv = STR_ENCODING_7BIT_HEX;
			break;
		case (PDU_DCS_ALPABET_8BIT >> PDU_DCS_ALPABET_SHIFT):
			rv = STR_ENCODING_8BIT_HEX;
			break;
		case (PDU_DCS_ALPABET_UCS2 >> PDU_DCS_ALPABET_SHIFT):
			rv = STR_ENCODING_UCS2_HEX;
			break;
	}

	return rv;
}

/*!
 * \brief Parse PDU
 * \param pdu -- SCA + TPDU
 * \param tpdu_length -- length of TPDU in octets
 * \return 0 on success
 */
/* TODO: split long function */
EXPORT_DEF const char * pdu_parse(char ** pdu, size_t tpdu_length, char * oa, size_t oa_len, str_encoding_t * oa_enc, char ** msg, str_encoding_t * msg_enc)
{
	const char * err = NULL;
	size_t pdu_length = strlen(*pdu);

	/* decode SCA */
	int field_len = pdu_parse_sca(pdu, &pdu_length);
	if(field_len > 0)
	{
	    if(tpdu_length * 2 == pdu_length)
	    {
		int pdu_type = pdu_parse_byte(pdu, &pdu_length);
		if(pdu_type >= 0)
		{
			/* TODO: also handle PDUTYPE_MTI_SMS_SUBMIT_REPORT and PDUTYPE_MTI_SMS_STATUS_REPORT */
			if(PDUTYPE_MTI(pdu_type) == PDUTYPE_MTI_SMS_DELIVER)
			{
				int oa_digits = pdu_parse_byte(pdu, &pdu_length);
				if(oa_digits > 0)
				{
					int oa_toa;
					field_len = pdu_parse_number(pdu, &pdu_length, oa_digits, &oa_toa, oa, oa_len);
					if(field_len > 0)
					{
						int pid = pdu_parse_byte(pdu, &pdu_length);
						*oa_enc = STR_ENCODING_7BIT;
						if(pid >= 0)
						{
						   /* TODO: support other types of messages */
						   if(pid == PDU_PID_SMS)
						   {
							int dcs = pdu_parse_byte(pdu, &pdu_length);
							if(dcs >= 0)
							{
							    // TODO: support compression
							    if( PDU_DCS_76(dcs) == PDU_DCS_76_00
							    		&&
							    	PDU_DCS_COMPRESSION(dcs) == PDU_DCS_NOT_COMPESSED
							    		&&
							    		(
							    		PDU_DCS_ALPABET(dcs) == PDU_DCS_ALPABET_7BIT
							    			||
							    		PDU_DCS_ALPABET(dcs) == PDU_DCS_ALPABET_8BIT
							    			||
							    		PDU_DCS_ALPABET(dcs) == PDU_DCS_ALPABET_UCS2
							    		)
							    	)
							    {
								int ts = pdu_parse_timestamp(pdu, &pdu_length);
								*msg_enc = pdu_dcs_alpabet2encoding(PDU_DCS_ALPABET(dcs));
								if(ts >= 0)
								{
									int udl = pdu_parse_byte(pdu, &pdu_length);
									if(udl >= 0)
									{
										/* calculate number of octets in UD */
										if(PDU_DCS_ALPABET(dcs) == PDU_DCS_ALPABET_7BIT)
											udl = ((udl + 1) * 7) >> 3;
										if((size_t)udl * 2 == pdu_length)
										{
											if(PDUTYPE_UDHI(pdu_type) == PDUTYPE_UDHI_HAS_HEADER)
											{
												/* TODO: implement header parse */
												int udhl = pdu_parse_byte(pdu, &pdu_length);
												if(udhl >= 0)
												{
													/* NOTE: UDHL count octets no need calculation */
													if(pdu_length >= (size_t)(udhl * 2))
													{
														/* skip UDH */
														*pdu += udhl * 2;
														pdu_length -= udhl * 2;
													}
													else
													{
														err = "Invalid UDH";
													}
												}
												else
												{
													err = "Can't parse UDHL";
												}
											}
											/* save message */
											*msg = *pdu;
										}
										else
										{
											*pdu -= 2;
											err = "UDL not match with UD length";
										}
									}
									else
									{
										err = "Can't parse UDL";
									}
								}
								else
								{
									err = "Can't parse Timestamp";
								}
							    }
							    else
							    {
								*pdu -= 2;
								err = "Unsupported DCS value";
							    }
							}
							else
							{
								err = "Can't parse DSC";
							}
						    }
						    else
						    {
						    	err = "Unhandled PID value, only SMS supported";
						    }
						}
						else
						{
							err = "Can't parse PID";
						}
					}
					else
					{
						err = "Can't parse OA";
					}
				}
				else
				{
					err = "Can't parse length of OA";
				}
			}
			else
			{
				*pdu -= 2;
				err = "Unhandled PDU Type MTI only SMS-DELIVER supported";
			}
		}
		else
		{
			err = "Can't parse PDU Type";
		}
	    }
	    else
	    {
		err = "TPDU length not matched with actual length";
	    }
	}
	else
	{
		err = "Can't parse SCA";
	}

	return err;
}
