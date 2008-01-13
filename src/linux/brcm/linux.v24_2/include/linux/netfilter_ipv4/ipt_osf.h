/*
 * ipt_osf.h
 *
 * Copyright (c) 2003 Evgeniy Polyakov <johnpol@2ka.mipt.ru>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _IPT_OSF_H
#define _IPT_OSF_H

#define MAXGENRELEN		32
#define MAXDETLEN		64

#define IPT_OSF_GENRE		1
#define	IPT_OSF_SMART		2
#define IPT_OSF_LOG		4
#define IPT_OSF_NETLINK		8

#define IPT_OSF_LOGLEVEL_ALL	0
#define IPT_OSF_LOGLEVEL_FIRST	1

#include <linux/list.h>

#ifndef __KERNEL__
#include <netinet/ip.h>
#include <netinet/tcp.h>

struct list_head
{
	struct list_head *prev, *next;
};
#endif

struct ipt_osf_info
{
	char 			genre[MAXGENRELEN];
	int			len;
	unsigned long		flags;
	int			loglevel;
	int			invert; /* UNSUPPORTED */
};

struct osf_wc
{
	char			wc;
	unsigned long		val;
};

/* This struct represents IANA options
 * http://www.iana.org/assignments/tcp-parameters
 */
struct osf_opt
{
	unsigned char		kind;
	unsigned char		length;
	struct osf_wc		wc;
};

struct osf_finger
{
	struct list_head	flist;
	struct osf_wc		wss;
	unsigned char		ttl;
	unsigned char		df;
	unsigned long		ss;
	unsigned char		genre[MAXGENRELEN];
	unsigned char		version[MAXGENRELEN], subtype[MAXGENRELEN];
	
	/* Not needed, but for consistency with original table from Michal Zalewski */
	unsigned char		details[MAXDETLEN]; 

	int 			opt_num;
	struct osf_opt		opt[MAX_IPOPTLEN]; /* In case it is all NOP or EOL */

};

struct ipt_osf_nlmsg
{
	struct osf_finger	f;
	struct iphdr 		ip;
	struct tcphdr 		tcp;
};

#ifdef __KERNEL__

/* Defines for IANA option kinds */

#define OSFOPT_EOL		0	/* End of options */
#define OSFOPT_NOP		1	/* NOP */
#define OSFOPT_MSS		2	/* Maximum segment size */
#define OSFOPT_WSO		3	/* Window scale option */
#define OSFOPT_SACKP		4	/* SACK permitted */
#define OSFOPT_SACK		5	/* SACK */
#define OSFOPT_ECHO		6	
#define OSFOPT_ECHOREPLY	7
#define OSFOPT_TS		8	/* Timestamp option */
#define OSFOPT_POCP		9	/* Partial Order Connection Permitted */
#define OSFOPT_POSP		10	/* Partial Order Service Profile */
/* Others are not used in current OSF */

static struct osf_opt IANA_opts[] = 
{
	{0, 1,},
	{1, 1,},
	{2, 4,},
	{3, 3,},
	{4, 2,},
	{5, 1 ,}, /* SACK length is not defined */
	{6, 6,},
	{7, 6,},
	{8, 10,},
	{9, 2,},
	{10, 3,},
	{11, 1,}, /* CC: Suppose 1 */
	{12, 1,}, /* the same */
	{13, 1,}, /* and here too */
	{14, 3,},
	{15, 1,}, /* TCP Alternate Checksum Data. Length is not defined */
	{16, 1,},
	{17, 1,},
	{18, 3,},
	{19, 18,},
	{20, 1,},
	{21, 1,},
	{22, 1,},
	{23, 1,},
	{24, 1,},
	{25, 1,},
	{26, 1,},
};

#endif /* __KERNEL__ */

#endif /* _IPT_OSF_H */
