/*
 * Copyright (c) 2014, 2016, 2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//#include <osal_common.h>
#include <sys_base.h>
#include "cli_lib.h"

/******************************************************************************
 *
 * description:
 *   convert str to MAC address, like HHHH.HHHH.HHHH
 *
 * input:
 *   str - address string
 *
 * output:
 *   mac - pointer to MAC address
 *
 * return:
 *   0  - okay
 *   -1 - invalid MAC string
 *
 ******************************************************************************/
int cli_str_2_mac(const char *str, sa_u8_t *mac)
{
#define CLI_MAC_STR_CHARS    "0123456789abcdefABCDEF."

	int strLen;
	int i;
	char valStr[8];
	int valLen;
	sa_u32_t val;
	const char *cp;
	const char *pDot;	/* pointer to '.' or '\0' */
	char *endPtr;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	/* string length check */
	strLen = strlen(str);
	if ((strLen < CLI_MAC_STR_LEN_MIN)
	    || (strLen > CLI_MAC_STR_LEN_MAX)) {
		return -1;
	}

	/* character check */
	if (strspn(str, CLI_MAC_STR_CHARS) != strLen) {
		return -1;
	}

	cp = str;

	for (i = 0; i < 3; i++) {
		if (i < 2) {
			pDot = strchr(cp, '.');
		} else {
			pDot = cp + strlen(cp);
		}

		valLen = pDot - cp;

		if ((NULL == pDot)
		    || (valLen <= 0)
		    || (valLen > 4)) {
			return -1;
		}

		memcpy(valStr, cp, valLen);
		valStr[valLen] = '\0';

		val = strtoul(valStr, &endPtr, 16);
		if ((*endPtr != '\0')
		    || (val > 0xffff)) {
			return -1;
		}

		if (mac != NULL) {
			mac[2 * i] = (sa_u8_t) ((val >> 8) & 0xff);
			mac[2 * i + 1] = (sa_u8_t) (val & 0xff);
		}

		cp = pDot + 1;
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert MAC address to str, like "1111.2222.3333"
 *
 * input:
 *   mac - pointer to MAC address
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_mac_2_str(const sa_u8_t *mac, char *str, unsigned int maxLen)
{
	int len = 0;
	if ((NULL == mac) || (NULL == str) || (maxLen < 15))
		return -1;

	len = snprintf(str, maxLen, "%02x%02x.%02x%02x.%02x%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (len < 0 || len >= maxLen)
		return -1;

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert u8 array to str, like "11112222.3333ffff.eeeeaaaa"
 *
 * input:
 *   array    - pointer to u8 array
 *   arrayNum - number of u8
 *   strLen   - str length
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_u8_array_2_str(const sa_u8_t *array, int arrayNum, char *str,
		       int strLen)
{
	int len = 0;
	int slen;
	int i;
	char tempStr[16];

	if ((NULL == array) || (NULL == str)) {
		return -1;
	}

	if (strLen < (arrayNum * 2 + arrayNum / 4)) {
		snprintf(str, strLen, "short buffer");
		return -1;
	}

	for (i = 0; i < arrayNum; i++) {
		slen = snprintf(tempStr, 3, "%02x", array[i]);
		if (slen < 0 || slen > 2)
			return -1;

		len += 2;

		if (((i % 4) == 3) && (i != (arrayNum - 1))) {
			strlcat(tempStr, ".", 1);
		}

		slen = strLen - len;
		if (slen < strlen(tempStr))
			slen = strlen(tempStr);

		strlcat(str, tempStr, strLen);
	}

	return 0;
}

#if 0
int cli_str_2_sak(const char *str, sa_u8_t *sak)
{
	int i;
	int strLen = strlen(str);
	sa_u32_t key_addr[4];

	if (str_len != 35) {
		return -1;
	}

	if (strspn(str, "1234567890abcdefABCDEF:") != str_len) {
		return -1;
	}

	for (i = 0; i < str_len; i++) {
		if (i == 8 || i == 17 || i == 26) {
			if (str[i] != ':') {
				return -1;
			}
		} else {
			if (str[i] == ':') {
				return -1;
			}
		}
	}

	sscanf(str, "%8x:%8x:%8x:%8x", (sa_u32_t *) & key_addr[0],
	       (sa_u32_t *) & key_addr[1],
	       (sa_u32_t *) & key_addr[2], (sa_u32_t *) & key_addr[3]);

	for (i = 0; i < 4; i++) {
		key[i * 4] = (key_addr[i] >> 24) & 0xff;
		key[i * 4 + 1] = (key_addr[i] >> 16) & 0xff;
		key[i * 4 + 2] = (key_addr[i] >> 8) & 0xff;
		key[i * 4 + 3] = (key_addr[i]) & 0xff;
	}

	return 0;
}
#endif

/******************************************************************************
 *
 * description:
 *   convert str to IPv4 address
 *
 * input:
 *   str - address string
 *
 * output:
 *   ip - pointer to IPv4 address
 *
 * return:
 *   0  - okay
 *   -1 - invalid ip string
 *
 ******************************************************************************/
int cli_str_2_ipv4(const char *str, sa_u8_t *ipv4)
{
#define CLI_IPV4_ADDR_CHARS     "0123456789."

	int strLen;
	int i;
	char valStr[4];
	int valLen;
	sa_u32_t val;
	const char *cp;
	const char *pDot;	/* pointer to '.' or '\0' */
	char *endPtr;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	/* string length check */
	strLen = strlen(str);
	if ((strLen < CLI_IPV4_STR_LEN_MIN)
	    || (strLen > CLI_IPV4_STR_LEN_MAX)) {
		return -1;
	}

	/* character check */
	if (strspn(str, CLI_IPV4_ADDR_CHARS) != strLen) {
		return -1;
	}

	cp = str;

	for (i = 0; i < 4; i++) {
		if (i < 3) {
			pDot = strchr(cp, '.');
		} else {
			pDot = cp + strlen(cp);
		}

		valLen = pDot - cp;

		if ((NULL == pDot)
		    || (valLen <= 0)
		    || (valLen > 3)) {
			return -1;
		}

		memcpy(valStr, cp, valLen);
		valStr[valLen] = '\0';

		val = strtoul(valStr, &endPtr, 10);
		if ((*endPtr != '\0')
		    || (val > 255)) {
			return -1;
		}

		if (ipv4 != NULL) {
			ipv4[i] = (sa_u8_t) val;
		}

		cp = pDot + 1;
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert str to IPv6 address
 *
 * input:
 *   str - address string
 *
 * output:
 *   ip - pointer 16 bytes for IPv6 address
 *
 * return:
 *   0  - okay
 *   -1 - invalid IPv6 string
 *
 ******************************************************************************/
int cli_str_2_ipv6(const char *str, sa_u8_t *ipV6)
{
#define CLI_IPV6_ADDR_LEN   16
#define CLI_IPV6_ADDR_CHARS "0123456789abcdefABCDEF:.%[]"

	sa_u16_t val = 0;
	sa_u32_t tmpVal;
	int zero = -1;		/* zero compress start index */
	int ipIndex = 0;
	char c;
	int strLen;
	int i;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	strLen = strlen(str);
	if ((strLen < CLI_IPV6_STR_LEN_MIN)
	    || (strLen > CLI_IPV6_STR_LEN_MAX)) {
		return -1;
	}

	/* character check */
	if (strspn(str, CLI_IPV6_ADDR_CHARS) != strLen) {
		return -1;
	}

	/* check [X:X::X:X] case */
	if ('[' == *str) {
		if (str[strLen - 1] != ']') {
			/* [ and ] dismatch */
			return -1;
		}
		str++;
	}

	/* handle every character */
	for (i = 0; i <= strLen; i++) {
		c = str[i];

		if ((':' == c) || ('\0' == c) || (']' == c)) {
			if (ipV6 != NULL) {
				ipV6[ipIndex] = (val >> 8) & 0xff;
				ipV6[ipIndex + 1] = val & 0xff;
			}
			ipIndex += 2;
			val = 0;

			if (('\0' == c) || (']' == c)) {
				break;
			}

			/* check Zero compression case */
			if (':' == str[i + 1]) {
				if (':' == str[i + 2]) {
					/* ::: is invalid */
					return -1;
				}

				if (zero > 0) {
					/* only alow one zero compression */
					return -1;
				}

				zero = ipIndex;
				i++;
			}
		} else {
			if ((c >= '0') && (c <= '9')) {
				tmpVal = c - '0';
			} else if ((c >= 'a') && (c <= 'f')) {
				tmpVal = c - 'a' + 10;
			} else if ((c >= 'A') && (c <= 'F')) {
				tmpVal = c - 'A' + 10;
			} else {
				/* illegal char */
				return -1;
			}

			val = (val << 4) + (tmpVal & 0xf);
		}
	}

	if (ipIndex < CLI_IPV6_ADDR_LEN) {
		if (zero < 0) {
			/* too short address */
			return -1;
		}

		if (ipV6 != NULL) {
			memmove(&ipV6[zero + CLI_IPV6_ADDR_LEN - ipIndex],
				&ipV6[zero], ipIndex - zero);
			memset(&ipV6[zero], 0, CLI_IPV6_ADDR_LEN - ipIndex);
		}
	}

	return 0;
}

int cli_ipv6_2_str_uncompress(const sa_u8_t *ipv6, char *str)
{
	sa_u16_t val;
	int len = 0;
	int tlen = 0;
	int i;

	/* parameter check */
	if ((NULL == ipv6) || (NULL == str)) {
		return -1;
	}

	str[0] = '\0';

	for (i = 0; i < 8; i++) {
		val = (ipv6[2 * i] << 8) + ipv6[2 * i + 1];

		tlen = 0;
		if (i == 7) {
			tlen = snprintf(str + len, 5, "%x", val);
			if (tlen < 0 || tlen >= 5)
				return -1;

			len += tlen;
		} else {
			tlen = snprintf(str + len, 6, "%x:", val);
			if (tlen < 0 || tlen >= 6)
				return -1;

			len += tlen;
		}
	}

	return 0;
}

int cli_ipv6_2_str_contiki(const sa_u8_t *ipv6, char *str)
{
	sa_u16_t a;
	unsigned int i;
	int f;
	int len = 0;
	int tlen = 0;

	/* parameter check */
	if ((NULL == ipv6) || (NULL == str)) {
		return -1;
	}

	for (i = 0, f = 0; i < 16; i += 2) {
		a = (ipv6[i] << 8) + ipv6[i + 1];

		if (a == 0 && f >= 0) {
			if (f++ == 0) {
				tlen = 0;
				tlen = snprintf(str + len, 3, "::");
				if (tlen < 0 || tlen >= 3)
					return -1;

				len += tlen;
			}
		} else {
			if (f > 0) {
				f = -1;
			} else if (i > 0) {
				tlen = 0;
				tlen = snprintf(str + len, 2, ":");
				if (tlen < 0 || tlen >= 2)
					return -1;

				len += tlen;
			}

			tlen = 0;
			tlen = snprintf(str + len, 5, "%x", a);
			if (tlen < 0 || tlen >= 5)
				return -1;

			len += tlen;
		}
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert str to hex, like "0x87ab"
 *
 * input:
 *   str - hex string
 *
 * output:
 *   pHex - pointer to unsigned 32 bits hex
 *
 * return:
 *   0  - okay
 *   -1 - invalid list string
 *
 ******************************************************************************/
int cli_str_2_hex(const char *str, sa_u32_t *pHex)
{
	char *endPtr;

	if ((NULL == str) || (NULL == pHex)) {
		return -1;
	}

	*pHex = strtoul(str, &endPtr, 16);
	if (*endPtr != '\0') {
		return -1;
	}

	return 0;
}

int cli_str_2_long_hex(const char *str, sa_u64_t *pU64)
{
	char *endPtr;

	if ((NULL == str) || (NULL == pU64)) {
		return -1;
	}

	*pU64 = strtoull(str, &endPtr, 16);
	if (*endPtr != '\0') {
		return -1;
	}

	return 0;
}

static int _cli_str_char_num(const char *str, char c)
{
	int strLen, i, count;

	if (NULL == str) {
		return 0;
	}

	strLen = strlen(str);
	count = 0;

	for (i = 0; i < strLen; i++) {
		if (str[i] == c) {
			count++;
		}
	}

	return count;
}

/******************************************************************************
 *
 * description:
 *   convert str to list, like "1,3,7-18,22"
 *
 * input:
 *   str - list string
 *   min - min value of each number
 *   max - max value of each number
 *
 * output:
 *   pPortArray - pointer to list array, if it is NULL then do not put result in it.
 *
 * return:
 *   0  - okay
 *   -1 - invalid list string
 *
 ******************************************************************************/
int cli_str_2_list(const char *str, const sa_u32_t min, const sa_u32_t max,
		   ARRAY_T *pPortArray)
{
#define CLI_LIST_CHARS "0123456789,-"
#define CLI_LIST_MAX_DECIMAL_LEN 10	/* max "4294967295" */
#define CLI_LIST_MAX_SECTION_LEN 21	/* max "4294967294-4294967295" */

	int strLen;
	const char *sectionStart;
	const char *pComma;	/* pointer to ',' or '\0' */
	char *pDash;		/* pointer to '-' or '\0' */
	int dashNum;
	char decimalStr[CLI_LIST_MAX_DECIMAL_LEN + 1];
	int decimalStrLen;
	char sectionStr[CLI_LIST_MAX_SECTION_LEN + 1];
	int sectionStrLen;
	sa_ul_t i, val, start, end, lastVal = 0;
	sa_bool_t lastValValid = SA_FALSE;
	char *endPtr;
	sa_bool_t endFlag = SA_FALSE;
	sa_u32_t **data_ptr;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	strLen = strlen(str);

	/* character check */
	if (strspn(str, CLI_LIST_CHARS) != strLen) {
		return -1;
	}

	/* the first character must be digital
	   if (!CLI_IS_DIGITAL(str[0])
	   {
	   return -1;
	   } */

	sectionStart = str;

	while (!endFlag) {
		pComma = strchr(sectionStart, ',');
		if (NULL == pComma) {
			endFlag = SA_TRUE;
			pComma = sectionStart + strlen(sectionStart);	/* pComma pointer to '\0' */
		}

		sectionStrLen = pComma - sectionStart;
		if (sectionStrLen > CLI_LIST_MAX_SECTION_LEN) {
			/* too long section string */
			return -1;
		}

		memcpy(sectionStr, sectionStart, sectionStrLen);
		sectionStr[sectionStrLen] = '\0';

		/* handle sectionStr, like "23" or "1-12" */
		dashNum = _cli_str_char_num(sectionStr, '-');
		if (0 == dashNum) {
			if (strlen(sectionStr) > CLI_LIST_MAX_DECIMAL_LEN) {
				/* too long decimal string */
				return -1;
			}

			/* convert sectionStr to value */
			val = strtoul(sectionStr, &endPtr, 10);
			if ((*endPtr != '\0')
			    || (min > val)
			    || (max < val)
			    || (lastValValid && (val <= lastVal))) {
				return -1;
			}

			if (pPortArray != NULL) {
				data_ptr = (sa_u32_t **)&val;
				array_insert_slot(pPortArray, *data_ptr);
			}

			lastVal = val;
			lastValValid = SA_TRUE;
		} else if (1 == dashNum) {
			/* check whether dash is first character or laster character */
			if (('-' == sectionStr[0])
			    || ('-' == sectionStr[sectionStrLen - 1])) {
				return -1;
			}

			pDash = strchr(sectionStr, '-');
			if (pDash == NULL)
				return -1;

			/*
			 * handle start number before '-'
			 */
			decimalStrLen = pDash - sectionStr;
			if (decimalStrLen > CLI_LIST_MAX_DECIMAL_LEN) {
				/* too long decimal string */
				return -1;
			}

			memcpy(decimalStr, sectionStr, decimalStrLen);
			decimalStr[decimalStrLen] = '\0';

			start = strtoul(decimalStr, &endPtr, 10);
			if ((*endPtr != '\0')
			    || (min > start)
			    || (max < start)
			    || (lastValValid && (start <= lastVal))) {
				return -1;
			}

			lastVal = start;
			lastValValid = SA_TRUE;

			/*
			 * handle end number after '-'
			 */
			decimalStrLen =
			    sectionStrLen - (pDash + 1 - sectionStr);
			if (decimalStrLen > CLI_LIST_MAX_DECIMAL_LEN) {
				/* too long decimal string */
				return -1;
			}

			memcpy(decimalStr, pDash + 1, decimalStrLen);
			decimalStr[decimalStrLen] = '\0';

			end = strtoul(decimalStr, &endPtr, 10);
			if ((*endPtr != '\0')
			    || (min > end)
			    || (max < end)
			    || (lastValValid && (end <= lastVal))) {
				return -1;
			}

			lastVal = end;
			lastValValid = SA_TRUE;

			/* insert every number in the range to array */
			for (i = start; i <= end; i++) {
				if (pPortArray != NULL) {
					data_ptr = (sa_u32_t **)&i;
					array_insert_slot(pPortArray, *data_ptr);
				}
			}
		} else {	/* dashNum > 1 */

			return -1;
		}

		/* move to character after ',' */
		sectionStart = pComma + 1;
	}

	return 0;
}

/* sci=11223344:55667788 */
int cli_str_2_sci(const char *str, sa_u8_t *sci)
{
	int i;
	int str_len = strlen(str);
	sa_u32_t sci_addr[2];

	if (str_len != 17) {
		return -1;
	}

	if (strspn(str, "1234567890abcdefABCDEF:") != str_len) {
		return -1;
	}

	for (i = 0; i < str_len; i++) {
		if (i == 8) {
			if (str[i] != ':') {
				return -1;
			}
		} else {
			if (str[i] == ':') {
				return -1;
			}
		}
	}

	sscanf(str, "%8x:%8x", (sa_u32_t *) & sci_addr[0],
	       (sa_u32_t *) & sci_addr[1]);

	sci[0] = (sci_addr[0] >> 24) & 0xff;
	sci[1] = (sci_addr[0] >> 16) & 0xff;
	sci[2] = (sci_addr[0] >> 8) & 0xff;
	sci[3] = (sci_addr[0]) & 0xff;
	sci[4] = (sci_addr[1] >> 24) & 0xff;
	sci[5] = (sci_addr[1] >> 16) & 0xff;
	sci[6] = (sci_addr[1] >> 8) & 0xff;
	sci[7] = (sci_addr[1]) & 0xff;

	return 0;
}

int cli_sci_2_str(const sa_u8_t *sci, char *str, unsigned int maxLen)
{
	int len = 0;
	sa_u32_t sci_addr[2];

	sci_addr[0] =
	    (sci[0] << 24) | (sci[1] << 16) | (sci[2] << 8) | (sci[3]);
	sci_addr[1] =
	    (sci[4] << 24) | (sci[5] << 16) | (sci[6] << 8) | (sci[7]);

	len = snprintf(str, maxLen, "%08x:%08x", sci_addr[0], sci_addr[1]);
	if (len < 0 || len >= maxLen)
		return -1;

	return 0;
}

/* sak=11223344:55667788:88776655:44332211 */
int cli_str_2_sak(const char *str, sa_u8_t *key)
{
	int i;
	int str_len = strlen(str);
	sa_u32_t key_addr[4];

	if (str_len != 35) {
		return -1;
	}

	if (strspn(str, "1234567890abcdefABCDEF:") != str_len) {
		return -1;
	}

	for (i = 0; i < str_len; i++) {
		if (i == 8 || i == 17 || i == 26) {
			if (str[i] != ':') {
				return -1;
			}
		} else {
			if (str[i] == ':') {
				return -1;
			}
		}
	}

	sscanf(str, "%8x:%8x:%8x:%8x", (sa_u32_t *) & key_addr[0],
	       (sa_u32_t *) & key_addr[1],
	       (sa_u32_t *) & key_addr[2], (sa_u32_t *) & key_addr[3]);

	for (i = 0; i < 4; i++) {
		((sa_u32_t *)key)[3-(i%4)] = key_addr[i];
	}

	return 0;
}

int cli_sak_2_str(const sa_u8_t *key, char *str, int strLen)
{
	/*sak read from big address to little address, so we revert the key*/
	sa_u8_t i;
	sa_u8_t newkey[16];
	for (i = 0; i < 16; i++) {
		newkey[i] = key[15-i%16];
	}
	return cli_u8_array_2_str(newkey, 16, str, strLen);
}
