/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/resolver.c - Simple DNS RR lookup function
***********************************************************
 $Author: jeroen $
 $Id: resolver.c,v 1.3 2006-07-23 14:13:57 jeroen Exp $
 $Date: 2006-07-23 14:13:57 $
**********************************************************/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "resolver.h"

#ifndef _WIN32
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

int getrrs(const char *label, int rrtype, void gotrec(unsigned int num, int type, const char *record))
{
#if defined(_LINUX) && !defined(__UCLIBC__)
	struct __res_state	res;
#endif
	unsigned char		answer[8192];
	HEADER			*header = (HEADER *)answer;
	char			buf[2048];
	int			ret, count;
	unsigned int		i,j,k,rrnum = 0;
	unsigned char		*startptr, *endptr, *ptr;
	uint16_t		type = 0, class = 0;
	uint32_t		ttl = 0;

#if defined(_LINUX) && !defined(__UCLIBC__)
	memset(&res, 0, sizeof(res));
	res.options = RES_DEBUG;
	res_ninit(&res);
#else
	res_init();
#endif

	memset(answer, 0, sizeof(answer));
#if defined(_LINUX) && !defined(__UCLIBC__)
	ret = res_nquery(&res, label, C_IN, rrtype, answer, sizeof(answer));
#else
	ret = res_query(label, C_IN, rrtype, answer, sizeof(answer));
#endif
	if (ret < 0) return -1;

	/* Our start and end */
	startptr = &answer[0];
	endptr = &answer[ret];

	/* Skip the header */
	ptr = startptr + HFIXEDSZ;

	/* Skip Query part */
	for (count = ntohs(header->qdcount); count--; ptr += ret + QFIXEDSZ)
	{
		if ((ret = dn_skipname(ptr, endptr)) < 0) return -1;
	}

	/* Only look at the Answer section */
	count = ntohs(header->ancount);

	/* Go through all the Answer records */
	while (ptr < endptr && count > 0)
	{
		rrnum++;

		memset(buf, 0, sizeof(buf));
		ret = dn_expand (startptr, endptr, ptr, buf, sizeof(buf));
		if (ret < 0) break;
		ptr += ret;

		if (ptr + INT16SZ + INT16SZ + INT32SZ >= endptr) break;

		/* Get the type */
		NS_GET16(type, ptr);

		/* Get the class */
		NS_GET16(class, ptr);

		/* Get the TTL */
		NS_GET32(ttl, ptr);

		/* Get the RDLength */
		NS_GET16(ret, ptr);

		memset(buf, 0, sizeof(buf));

		switch (type)
		{
		case T_TXT:
			for (k = ret, j = 0; j < k && &ptr[j] < endptr; j += (i+1))
			{
				i = ptr[j];
				memcpy(buf, &ptr[j+1], i > sizeof(buf) ? sizeof(buf) : i);
				buf[i > sizeof(buf) ? sizeof(buf) : i] = '\0';
				if (rrtype == type || rrtype == T_ANY) gotrec(rrnum, type, buf);
			}
			break;

		case T_A:
			inet_ntop(AF_INET, ptr, buf, sizeof(buf));
			if (rrtype == type || rrtype == T_ANY) gotrec(rrnum, type, buf);
			break;

		case T_AAAA:
			inet_ntop(AF_INET6, ptr, buf, sizeof(buf));
			if (rrtype == type || rrtype == T_ANY) gotrec(rrnum, type, buf);
			break;

		case T_MX:
			/* Get the MX preference */
			NS_GET16(ttl, ptr);
			ret = dn_expand(startptr, endptr, ptr, buf, sizeof(buf));
			if (rrtype == type || rrtype == T_ANY) gotrec(rrnum, type, buf);
			break;

		case T_NS:
			ret = dn_expand(startptr, endptr, ptr, buf, sizeof(buf));
			if (rrtype == type || rrtype == T_ANY) gotrec(rrnum, type, buf);
			break;

		default:
			/* Unhandled */
			break;
		}

		ptr += ret;
		count--;
	}
	return 0;
}
#else
/*
 * Windows Resolver Code, as per:
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dns/dns/dnsquery.asp
 * http://support.microsoft.com/default.aspx?scid=kb%3Ben-us%3B831226
 */
#include <windns.h>

int getrrs(const char *label, int rrtype, void gotrec(unsigned int num, int type, const char *record))
{
	DNS_STATUS	status;			/* Return value of DnsQuery_A() function */
	PDNS_RECORD	pResult, pRec;		/* Pointer to DNS_RECORD structure */
	unsigned int	rrnum = 0, i;
	uint16_t	type;

	status = DnsQuery(label,		/* Pointer to OwnerName */
		rrtype,				/* Type of the record to be queried */
		DNS_QUERY_STANDARD,		/* Standard Query */
		NULL,				/* Contains DNS server IP address */
		&pResult,			/* Resource record that contains the response */
		NULL);				/* Reserved for future use */

	if (status) return -1;
	else
	{
		for (pRec = pResult; pRec; pRec = pRec->pNext)
		{
			rrnum++;
			type = pRec->wType;

			if (rrtype != type && rrtype != ns_t_any) continue;

			switch (type)
			{
			case ns_t_txt:
				for (i=0; i < pRec->Data.TXT.dwStringCount; i++)
				{
					gotrec(rrnum, type, pRec->Data.TXT.pStringArray[i]);
				}
				break;

			case ns_t_a:
				gotrec(rrnum, type, (const char *)&pRec->Data.A.IpAddress);
				break;

			case ns_t_aaaa:
				gotrec(rrnum, type, (const char *)&pRec->Data.AAAA.Ip6Address);
				break;

			case ns_t_mx:
				gotrec(rrnum, type, pRec->Data.MX.pNameExchange);
				break;

			case ns_t_ns:
				gotrec(rrnum, type, pRec->Data.NS.pNameHost);
				break;
			}
		}
	}

	/* Free memory allocated for DNS records. */
	DnsRecordListFree(pResult, DnsFreeRecordListDeep);

	return 0;
}
#endif

