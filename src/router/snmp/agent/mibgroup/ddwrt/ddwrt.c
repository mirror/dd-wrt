/*
 * ddwrt.c 
 * Copyright (C) 2011 Christian Scheele <chris@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _LINUX_IF_H
#define _NO_WLIOCTL_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>
#include <netinet/ether.h>

#ifdef HAVE_MADWIFI
/* madwifi */
#include "wireless_copy.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"
#endif

/* foreach ..*/
#include <shutils.h>
#include <utils.h>
/* i want a staging dir */
#ifdef HAVE_MADWIFI
#include "../../../../wireless-tools/iwlib.h"
#endif

// #include <net/ethernet.h>

/* nvram */
#include <bcmnvram.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ddwrt.h"
#include <wlutils.h>
#ifndef IEEE80211_ADDR_LEN
#define IEEE80211_ADDR_LEN 6
#endif

void ddxrWlStatTable_filltable();
void ddWlRtabTable_filltable();
void ddxrWlStatTable_madwifi();
void ddWlRtabTable_madwifi();
void ddWlRtabTable_madwifi_assoc(char *ifname, int cnt, int turbo, int ciface, int cvap, int is_wds, int wdscnt);
void ddWlRtabTable_mac80211_assoc(char *ifname, int cnt, int ciface, int cvap, int is_wds, int wdscnt);

time_t ddxrWlStatTable_next_update = 0;
time_t ddWlRtabTable_next_update = 0;

static char convert_to_snmp_bitfield(int value)
{
	int bit;
	int ret = 0;
	for (bit = 0; bit < 8; bit++) {
		if ((value) & (1 << (bit)))
			ret |= (1 << (6 - bit));
	}
	return (ret);
}

/*int snmp_set_var_typed_integer(netsnmp_variable_list * newvar, u_char type, long val)
{
	newvar->type = type;
	return snmp_set_var_value(newvar, (u_char *) & val, sizeof(long));
}*/
#ifdef HAVE_ATH9K
void special_mac80211_init(void);
#endif
/** Initializes the ddwrt module */
void init_ddwrt(void)
{
#ifdef HAVE_ATH9K
	special_mac80211_init();
#endif

	/* here we initialize all the tables we're planning on supporting */
	initialize_table_ddxrWlStatTable();
	initialize_table_ddWlRtabTable();
}

  // # Determine the first/last column names

/** Initialize the ddxrWlStatTable table by defining its contents and how it's structured */
void initialize_table_ddxrWlStatTable(void)
{
	const oid ddxrWlStatTable_oid[] = { 1, 3, 6, 1, 4, 1, 29205, 1, 1, 1, 1 };
	const size_t ddxrWlStatTable_oid_len = OID_LENGTH(ddxrWlStatTable_oid);
	netsnmp_handler_registration *reg;
	netsnmp_iterator_info *iinfo;
	netsnmp_table_registration_info *table_info;

	DEBUGMSGTL(("ddwrt:init", "initializing table ddxrWlStatTable\n"));

	reg = netsnmp_create_handler_registration("ddxrWlStatTable", ddxrWlStatTable_handler, ddxrWlStatTable_oid, ddxrWlStatTable_oid_len, HANDLER_CAN_RONLY);

	table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
	netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER,	/* index: ddxrWlStatIndexMajor */
					 ASN_INTEGER,	/* index: ddxrWlStatIndexMinor */
					 0);
	table_info->min_column = COLUMN_DDXRWLSTATMAJORINTERFACE;
	table_info->max_column = COLUMN_DDXRWLSTATANTENNANAME;

	iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);
	iinfo->get_first_data_point = ddxrWlStatTable_get_first_data_point;
	iinfo->get_next_data_point = ddxrWlStatTable_get_next_data_point;
	iinfo->table_reginfo = table_info;

	netsnmp_register_table_iterator(reg, iinfo);

	/* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct ddxrWlStatTable_entry {
	/* Index values */
	int ddxrWlStatIndexMajor;
	int ddxrWlStatIndexMinor;

	/* Column values */
	int ddxrWlStatMajorInterface;
	int ddxrWlStatMinorInterface;
	char ddxrWlStatIfaceName[32];
	size_t ddxrWlStatIfaceName_len;
	int ddxrWlStatTxPower;
	int ddxrWlStatAntennaGain;
	char ddxrWlStatBssid[6];
	size_t ddxrWlStatBssid_len;
	int ddxrWlStatFreq;
	char ddxrWlStatSsid[64];
	size_t ddxrWlStatSsid_len;
	char ddxrWlStatScanlist[256];
	size_t ddxrWlStatScanlist_len;
	int ddxrWlStatnetmode;
	int ddxrWlStatmode;
	char ddxrWlStatcardtype[128];
	size_t ddxrWlStatcardtype_len;
	u_int ddxrWlStatminrate;
	u_int ddxrWlStatmaxrate;
	int ddxrWlStatfastframing;
	u_int ddxrWlStatconfigack;
	u_int ddxrWlStatack;
	char ddxrWlStatTxAntenna[1];
	size_t ddxrWlStatTxAntenna_len;
	char ddxrWlStatRxAntenna[1];
	size_t ddxrWlStatRxAntenna_len;
	int ddxrWlStatOutdoorBand;
	int ddxrWlStat80211nWideChannel;
	int ddxrWlStatChannelBW;
	int ddxrWlStatSSIDBroadcast;
	char ddxrWlStatRegulatory[64];
	size_t ddxrWlStatRegulatory_len;
	char ddxrWlStatAntennaname[64];
	size_t ddxrWlStatAntennaname_len;

	/* Illustrate using a simple linked list */
	int valid;
	struct ddxrWlStatTable_entry *next;
};

struct ddxrWlStatTable_entry *ddxrWlStatTable_head;

/* create a new row in the (unsorted) table */
struct ddxrWlStatTable_entry *ddxrWlStatTable_createEntry(int ddxrWlStatIndexMajor, int ddxrWlStatIndexMinor)
{
	struct ddxrWlStatTable_entry *entry;

	entry = SNMP_MALLOC_TYPEDEF(struct ddxrWlStatTable_entry);
	if (!entry)
		return NULL;

	entry->ddxrWlStatIndexMajor = ddxrWlStatIndexMajor;
	entry->ddxrWlStatIndexMinor = ddxrWlStatIndexMinor;
	entry->next = ddxrWlStatTable_head;
	ddxrWlStatTable_head = entry;
	return entry;
}

/* remove a row from the table */
void ddxrWlStatTable_removeEntry(struct ddxrWlStatTable_entry *entry)
{
	struct ddxrWlStatTable_entry *ptr, *prev;

	if (!entry)
		return;		/* Nothing to remove */

	for (ptr = ddxrWlStatTable_head, prev = NULL; ptr != NULL; prev = ptr, ptr = ptr->next) {
		if (ptr == entry)
			break;
	}
	if (!ptr)
		return;		/* Can't find it */

	if (prev == NULL)
		ddxrWlStatTable_head = ptr->next;
	else
		prev->next = ptr->next;

	SNMP_FREE(entry);	/* XXX - release any other internal resources */
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *ddxrWlStatTable_get_first_data_point(void **my_loop_context, void **my_data_context, netsnmp_variable_list * put_index_data, netsnmp_iterator_info * mydata)
{
	ddxrWlStatTable_filltable();
	*my_loop_context = ddxrWlStatTable_head;
	return ddxrWlStatTable_get_next_data_point(my_loop_context, my_data_context, put_index_data, mydata);
}

netsnmp_variable_list *ddxrWlStatTable_get_next_data_point(void **my_loop_context, void **my_data_context, netsnmp_variable_list * put_index_data, netsnmp_iterator_info * mydata)
{
	struct ddxrWlStatTable_entry *entry = (struct ddxrWlStatTable_entry *)*my_loop_context;
	netsnmp_variable_list *idx = put_index_data;

	if (entry) {
		snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->ddxrWlStatIndexMajor);
		idx = idx->next_variable;
		snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->ddxrWlStatIndexMinor);
		idx = idx->next_variable;
		*my_data_context = (void *)entry;
		*my_loop_context = (void *)entry->next;
		return put_index_data;
	} else {
		return NULL;
	}
}

/** handles requests for the ddxrWlStatTable table */
int ddxrWlStatTable_handler(netsnmp_mib_handler * handler, netsnmp_handler_registration * reginfo, netsnmp_agent_request_info * reqinfo, netsnmp_request_info * requests)
{

	netsnmp_request_info *request;
	netsnmp_table_request_info *table_info;
	struct ddxrWlStatTable_entry *table_entry;

	// DEBUGMSGTL(("ddwrt:handler", "Processing request (%d)\n", reqinfo->mode));

	switch (reqinfo->mode) {
		/*
		 * Read-support (also covers GetNext requests)
		 */
	case MODE_GET:
		for (request = requests; request; request = request->next) {
			table_entry = (struct ddxrWlStatTable_entry *)
			    netsnmp_extract_iterator_context(request);
			table_info = netsnmp_extract_table_info(request);

			switch (table_info->colnum) {
			case COLUMN_DDXRWLSTATMAJORINTERFACE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatMajorInterface);
				break;
			case COLUMN_DDXRWLSTATMINORINTERFACE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatMinorInterface);
				break;
			case COLUMN_DDXRWLSTATIFACENAME:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatIfaceName, table_entry->ddxrWlStatIfaceName_len);
				break;
			case COLUMN_DDXRWLSTATTXPOWER:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatTxPower);
				break;
			case COLUMN_DDXRWLSTATANTENNAGAIN:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatAntennaGain);
				break;
			case COLUMN_DDXRWLSTATBSSID:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatBssid, table_entry->ddxrWlStatBssid_len);
				break;
			case COLUMN_DDXRWLSTATFREQ:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatFreq);
				break;
			case COLUMN_DDXRWLSTATSSID:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatSsid, table_entry->ddxrWlStatSsid_len);
				break;
			case COLUMN_DDXRWLSTATSCANLIST:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatScanlist, table_entry->ddxrWlStatScanlist_len);
				break;
			case COLUMN_DDXRWLSTATNETMODE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatnetmode);
				break;
			case COLUMN_DDXRWLSTATMODE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatmode);
				break;
			case COLUMN_DDXRWLSTATCARDTYPE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatcardtype, table_entry->ddxrWlStatcardtype_len);
				break;
			case COLUMN_DDXRWLSTATMINRATE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_GAUGE, table_entry->ddxrWlStatminrate);
				break;
			case COLUMN_DDXRWLSTATMAXRATE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_GAUGE, table_entry->ddxrWlStatmaxrate);
				break;
			case COLUMN_DDXRWLSTATFASTFRAMING:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatfastframing);
				break;
			case COLUMN_DDXRWLSTATCONFIGACK:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_GAUGE, table_entry->ddxrWlStatconfigack);
				break;
			case COLUMN_DDXRWLSTATACK:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_GAUGE, table_entry->ddxrWlStatack);
				break;
			case COLUMN_DDXRWLSTATTXANTENNA:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatTxAntenna, table_entry->ddxrWlStatTxAntenna_len);
				break;
			case COLUMN_DDXRWLSTATRXANTENNA:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatRxAntenna, table_entry->ddxrWlStatRxAntenna_len);
				break;
			case COLUMN_DDXRWLSTATOUTDOORBAND:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatOutdoorBand);
				break;
			case COLUMN_DDXRWLSTAT80211NWIDECHANNEL:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStat80211nWideChannel);
				break;
			case COLUMN_DDXRWLSTATCHANNELBW:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatChannelBW);
				break;
			case COLUMN_DDXRWLSTATSSIDBROADCASAT:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlStatSSIDBroadcast);
				break;
			case COLUMN_DDXRWLSTATREGULATORY:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatRegulatory, table_entry->ddxrWlStatRegulatory_len);
				break;
			case COLUMN_DDXRWLSTATANTENNANAME:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlStatAntennaname, table_entry->ddxrWlStatAntennaname_len);
				break;
			default:
				netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
				break;
			}
		}
		break;

	}
	return SNMP_ERR_NOERROR;
}

  // # Determine the first/last column names

/** Initialize the ddWlRtabTable table by defining its contents and how it's structured */
void initialize_table_ddWlRtabTable(void)
{
	const oid ddWlRtabTable_oid[] = { 1, 3, 6, 1, 4, 1, 29205, 1, 1, 1, 2 };
	const size_t ddWlRtabTable_oid_len = OID_LENGTH(ddWlRtabTable_oid);
	netsnmp_handler_registration *reg;
	netsnmp_iterator_info *iinfo;
	netsnmp_table_registration_info *table_info;

	DEBUGMSGTL(("ddwrt:init", "initializing table ddWlRtabTable\n"));

	reg = netsnmp_create_handler_registration("ddWlRtabTable", ddWlRtabTable_handler, ddWlRtabTable_oid, ddWlRtabTable_oid_len, HANDLER_CAN_RONLY);

	table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
	netsnmp_table_helper_add_indexes(table_info, ASN_OCTET_STR,	/* index: ddxrWlRtabIndexAddr */
					 ASN_INTEGER,	/* index: ddxrWlRtabIndexMajor */
					 ASN_INTEGER,	/* index: ddxrWlRtabIndexMinor */
					 0);
	table_info->min_column = COLUMN_DDXRWLRTABMAC;
	table_info->max_column = COLUMN_DDXRWLRTABANTENNANAME;

	iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);
	iinfo->get_first_data_point = ddWlRtabTable_get_first_data_point;
	iinfo->get_next_data_point = ddWlRtabTable_get_next_data_point;
	iinfo->table_reginfo = table_info;

	netsnmp_register_table_iterator(reg, iinfo);

	/* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct ddWlRtabTable_entry {
	/* Index values */
	char ddxrWlRtabIndexAddr[6];
	size_t ddxrWlRtabIndexAddr_len;
	int ddxrWlRtabIndexMajor;
	int ddxrWlRtabIndexMinor;

	/* Column values */
	char ddxrWlRtabMac[6];
	size_t ddxrWlRtabMac_len;
	int ddxrWlRtabMajorInterface;
	int ddxrWlRtabMinorInterface;
	char ddxrWlRtabBssid[6];
	size_t ddxrWlRtabBssid_len;
	int ddxrWlRtabSNR;
	char ddxrWlRtabIfaceName[32];
	size_t ddxrWlRtabIfaceName_len;
	int ddxrWlRtabnoise;
	int ddxrWlRtabSignal;
	u_int ddxrWlRtabtxrate;
	u_int ddxrWlRtabrxrate;
	int ddxrWlRtabType;
	char ddxrWlRtabSsid[64];
	size_t ddxrWlRtabSsid_len;
	char ddxrWlRtabAntennaname[64];
	size_t ddxrWlRtabAntennaname_len;

	/* Illustrate using a simple linked list */
	int valid;
	struct ddWlRtabTable_entry *next;
};

struct ddWlRtabTable_entry *ddWlRtabTable_head;

/* create a new row in the (unsorted) table */
struct ddWlRtabTable_entry *ddWlRtabTable_createEntry(char *ddxrWlRtabIndexAddr, size_t ddxrWlRtabIndexAddr_len, int ddxrWlRtabIndexMajor, int ddxrWlRtabIndexMinor)
{
	struct ddWlRtabTable_entry *entry;

	entry = SNMP_MALLOC_TYPEDEF(struct ddWlRtabTable_entry);
	if (!entry)
		return NULL;

	memcpy(entry->ddxrWlRtabIndexAddr, ddxrWlRtabIndexAddr, ddxrWlRtabIndexAddr_len);
	entry->ddxrWlRtabIndexAddr_len = ddxrWlRtabIndexAddr_len;
	entry->ddxrWlRtabIndexMajor = ddxrWlRtabIndexMajor;
	entry->ddxrWlRtabIndexMinor = ddxrWlRtabIndexMinor;
	entry->next = ddWlRtabTable_head;
	ddWlRtabTable_head = entry;
	return entry;
}

/* remove a row from the table */
void ddWlRtabTable_removeEntry(struct ddWlRtabTable_entry *entry)
{
	struct ddWlRtabTable_entry *ptr, *prev;

	if (!entry)
		return;		/* Nothing to remove */

	for (ptr = ddWlRtabTable_head, prev = NULL; ptr != NULL; prev = ptr, ptr = ptr->next) {
		if (ptr == entry)
			break;
	}
	if (!ptr)
		return;		/* Can't find it */

	if (prev == NULL)
		ddWlRtabTable_head = ptr->next;
	else
		prev->next = ptr->next;

	SNMP_FREE(entry);	/* XXX - release any other internal resources */
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *ddWlRtabTable_get_first_data_point(void **my_loop_context, void **my_data_context, netsnmp_variable_list * put_index_data, netsnmp_iterator_info * mydata)
{
	ddWlRtabTable_filltable();
	*my_loop_context = ddWlRtabTable_head;
	return ddWlRtabTable_get_next_data_point(my_loop_context, my_data_context, put_index_data, mydata);
}

netsnmp_variable_list *ddWlRtabTable_get_next_data_point(void **my_loop_context, void **my_data_context, netsnmp_variable_list * put_index_data, netsnmp_iterator_info * mydata)
{
	struct ddWlRtabTable_entry *entry = (struct ddWlRtabTable_entry *)*my_loop_context;
	netsnmp_variable_list *idx = put_index_data;

	if (entry) {
		snmp_set_var_value(idx, (u_char *) entry->ddxrWlRtabIndexAddr, sizeof(entry->ddxrWlRtabIndexAddr));
		idx = idx->next_variable;
		snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->ddxrWlRtabIndexMajor);
		idx = idx->next_variable;
		snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->ddxrWlRtabIndexMinor);
		idx = idx->next_variable;
		*my_data_context = (void *)entry;
		*my_loop_context = (void *)entry->next;
		return put_index_data;
	} else {
		return NULL;
	}
}

/** handles requests for the ddWlRtabTable table */
int ddWlRtabTable_handler(netsnmp_mib_handler * handler, netsnmp_handler_registration * reginfo, netsnmp_agent_request_info * reqinfo, netsnmp_request_info * requests)
{

	netsnmp_request_info *request;
	netsnmp_table_request_info *table_info;
	struct ddWlRtabTable_entry *table_entry;

	// DEBUGMSGTL(("ddwrt:handler", "Processing request (%d)\n", reqinfo->mode));

	switch (reqinfo->mode) {
		/*
		 * Read-support (also covers GetNext requests)
		 */
	case MODE_GET:
		for (request = requests; request; request = request->next) {
			table_entry = (struct ddWlRtabTable_entry *)
			    netsnmp_extract_iterator_context(request);
			table_info = netsnmp_extract_table_info(request);

			switch (table_info->colnum) {
			case COLUMN_DDXRWLRTABMAC:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlRtabMac, table_entry->ddxrWlRtabMac_len);
				break;
			case COLUMN_DDXRWLRTABMAJORINTERFACE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlRtabMajorInterface);
				break;
			case COLUMN_DDXRWLRTABMINORINTERFACE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlRtabMinorInterface);
				break;
			case COLUMN_DDXRWLRTABBSSID:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlRtabBssid, table_entry->ddxrWlRtabBssid_len);
				break;
			case COLUMN_DDXRWLRTABSNR:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlRtabSNR);
				break;
			case COLUMN_DDXRWLRTABIFACENAME:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlRtabIfaceName, table_entry->ddxrWlRtabIfaceName_len);
				break;
			case COLUMN_DDXRWLRTABNOISE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlRtabnoise);
				break;
			case COLUMN_DDXRWLRTABSIGNAL:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlRtabSignal);
				break;
			case COLUMN_DDXRWLRTABTXRATE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_GAUGE, table_entry->ddxrWlRtabtxrate);
				break;
			case COLUMN_DDXRWLRTABRXRATE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_GAUGE, table_entry->ddxrWlRtabrxrate);
				break;
			case COLUMN_DDXRWLRTABTYPE:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, table_entry->ddxrWlRtabType);
				break;
			case COLUMN_DDXRWLRTABSSID:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlRtabSsid, table_entry->ddxrWlRtabSsid_len);
				break;
			case COLUMN_DDXRWLRTABANTENNANAME:
				if (!table_entry) {
					netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
					continue;
				}
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) table_entry->ddxrWlRtabAntennaname, table_entry->ddxrWlRtabAntennaname_len);
				break;
			default:
				netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
				snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER, 0 );
				break;
			}
		}
		break;

	}
	return SNMP_ERR_NOERROR;
}

static int ddxrcopystr(char *dest, char *src, int maxlen)
{
	int len = strlen(src);
	if (len > maxlen)
		len = maxlen;
	strncpy(dest, src, len);
	return len;
}

#define set_strbuf(buf, src) \
   buf##_len = ddxrcopystr(buf, src, sizeof(buf))

void ddxrWlStatTable_filltable()
{
	struct ddxrWlStatTable_entry *entry;
	time_t t;

	if (time(&t) - ddxrWlStatTable_next_update < DDXRWLSTATTABLE_TIMEOUT) {
		// DEBUGMSGTL(("ddwrt:filltable", "filltable NO update\n"));
		return;
	} else {
		ddxrWlStatTable_next_update = time(&t);
	}

	while (ddxrWlStatTable_head) {
		entry = ddxrWlStatTable_head->next;
		ddxrWlStatTable_head->next = NULL;
		SNMP_FREE(ddxrWlStatTable_head);
		ddxrWlStatTable_head = entry;
	}
	ddxrWlStatTable_head = NULL;

	DEBUGMSGTL(("ddwrt:ddxrWlStatTable_filltable", "filltable %d\n", time(&t)));

	ddxrWlStatTable_madwifi();

}

void ddWlRtabTable_filltable()
{
	struct ddWlRtabTable_entry *entry;
	time_t t;

	if (time(&t) - ddWlRtabTable_next_update < DDWLRTABTABLE_TIMEOUT) {
		// DEBUGMSGTL(("ddwrt:filltable", "filltable NO update\n"));
		return;
	} else {
		ddWlRtabTable_next_update = time(&t);
	}

	while (ddWlRtabTable_head) {
		entry = ddWlRtabTable_head->next;
		ddWlRtabTable_head->next = NULL;
		SNMP_FREE(ddWlRtabTable_head);
		ddWlRtabTable_head = entry;
	}
	ddWlRtabTable_head = NULL;
	DEBUGMSGTL(("ddwrt:ddWlRtabTable_filltable", "filltable %d\n", time(&t)));
	ddWlRtabTable_madwifi();
}

static const char *ieee80211_ntoa(const unsigned char mac[IEEE80211_ADDR_LEN])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

void set_ddxrWlStatMajorInterface(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatMajorInterface = var;
}

void set_ddxrWlStatMinorInterface(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatMinorInterface = var;
}

void set_ddxrWlStatIfaceName(struct ddxrWlStatTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlStatIfaceName, var);
}

void set_ddxrWlStatTxPower(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatTxPower = var;
}

void set_ddxrWlStatAntennaGain(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatAntennaGain = var;
}

void set_ddxrWlStatBssid(struct ddxrWlStatTable_entry *entry, char *var)
{
	memcpy(entry->ddxrWlStatBssid, var, 6);
	entry->ddxrWlStatBssid_len = 6;
}

void set_ddxrWlStatFreq(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatFreq = var;
}

void set_ddxrWlStatSsid(struct ddxrWlStatTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlStatSsid, var);
}

void set_ddxrWlStatScanlist(struct ddxrWlStatTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlStatScanlist, var);
}

void set_ddxrWlStatnetmode(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatnetmode = var;
}

void set_ddxrWlStatmode(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatmode = var;
}

void set_ddxrWlStatcardtype(struct ddxrWlStatTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlStatcardtype, var);
}

void set_ddxrWlStatminrate(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatminrate = var;
}

void set_ddxrWlStatmaxrate(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatmaxrate = var;
}

void set_ddxrWlStatfastframing(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatfastframing = var;
}

void set_ddxrWlStatconfigack(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatconfigack = var;
}

void set_ddxrWlStatack(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatack = var;
}

void set_ddxrWlStatTxAntenna(struct ddxrWlStatTable_entry *entry, int var)
{
	int bit;
	entry->ddxrWlStatTxAntenna[0] = convert_to_snmp_bitfield(var);
	entry->ddxrWlStatTxAntenna_len = 1;
}

void set_ddxrWlStatRxAntenna(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatRxAntenna[0] = convert_to_snmp_bitfield(var);
	entry->ddxrWlStatRxAntenna_len = 1;
}

void set_ddxrWlStatOutdoorBand(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatOutdoorBand = var;
}

void set_ddxrWlStat80211nWideChannel(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStat80211nWideChannel = var;
}

void set_ddxrWlStatChannelBW(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatChannelBW = var;
}

void set_ddxrWlStatSSIDBroadcast(struct ddxrWlStatTable_entry *entry, int var)
{
	entry->ddxrWlStatSSIDBroadcast = var;
}

void set_ddxrWlStatRegulatory(struct ddxrWlStatTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlStatRegulatory, var);
}

void set_ddxrWlStatAntennaname(struct ddxrWlStatTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlStatAntennaname, var);
}

int madwifi_getwirelessnetmode(char *iface)
{

	char mode[32];
	char m[32];
	enum WLA_NETMODES nm;

	strncpy(m, iface, 4);
	m[4] = 0;
	sprintf(mode, "%s_net_mode", m);

	nm = nm_unknown;
	if (nvram_match(mode, "disabled"))
		nm = nm_disabled;
	if (nvram_match(mode, "mixed"))
		nm = nm_mixed;
	if (nvram_match(mode, "bg-mixed"))
		nm = nm_bgmixed;
	if (nvram_match(mode, "g-only"))
		nm = nm_gonly;
	if (nvram_match(mode, "b-only"))
		nm = nm_bonly;
	if (nvram_match(mode, "n-only"))
		nm = nm_nonly;
	if (nvram_match(mode, "a-only"))
		nm = nm_aonly;
	if (nvram_match(mode, "na-only"))
		nm = nm_naonly;
	if (nvram_match(mode, "n5-only"))
		nm = nm_naonly;
	if (nvram_match(mode, "ng-only"))
		nm = nm_ngonly;
	if (nvram_match(mode, "n2-only"))
		nm = nm_ngonly;
	DEBUGMSGTL(("ddwrt:madwifi", "netmode %s %d found\n", mode, nm));
	return nm;
}

int madwifi_getwirelessmode(char *iface)
{
	char mode[32];
	char m[32];

	enum WLA_MODES nm;

	strncpy(m, iface, 4);
	m[4] = 0;
	sprintf(mode, "%s_mode", m);

	nm = n_unknown;
	if (nvram_match(mode, "wet"))
		nm = m_wet;
	if (nvram_match(mode, "ap"))
		nm = m_ap;
	if (nvram_match(mode, "sta"))
		nm = m_sta;
	if (nvram_match(mode, "infra"))
		nm = m_adhoc;
	if (nvram_match(mode, "apsta"))
		nm = m_apsta;
	if (nvram_match(mode, "apstawet"))
		nm = m_apstawet;
	if (nvram_match(mode, "wdssta"))
		nm = m_wdssta;
	if (nvram_match(mode, "wdsap"))
		nm = m_wdsap;
	if (nvram_match(mode, "mesh"))
		nm = m_mesh;
	DEBUGMSGTL(("ddwrt:madwifi", "mode %s %d found\n", mode, nm));
	return nm;
}

// 0 = ap, 1 = sta, 2=adhoc
int madwifi_getapsta(char *iface)
{
	char mode[32];
	char m[32];

	enum WLA_MODES nm;

	strncpy(m, iface, 4);
	m[4] = 0;
	sprintf(mode, "%s_mode", m);

	nm = n_unknown;
	if (nvram_match(mode, "wet"))
		return 1;
	if (nvram_match(mode, "sta"))
		return 1;
	if (nvram_match(mode, "infra"))
		return 2;
	if (nvram_match(mode, "wdssta"))
		return 1;
	return 0;
}

int get_distance_madwifi(char *iface)
{
	char path[64];
	int ifcount, distance = 0;

	strcpy(path, iface);
	sscanf(path, "ath%d", &ifcount);
	sprintf(path, "/proc/sys/dev/wifi%d/distance", ifcount);
	FILE *in = fopen(path, "rb");

	if (in != NULL) {
		fscanf(in, "%d", &distance);
		fclose(in);
	}
	return distance;
}

static float ag_rates[] = { 6, 9, 12, 18, 24, 36, 48, 54 };
static float turbo_rates[] = { 12, 18, 24, 36, 48, 72, 96, 108 };
static float b_rates[] = { 1, 2, 5.5, 11 };
static float bg_rates[] = { 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54 };
static float half_rates[] = { 3, 4.5, 6, 9, 12, 18, 24, 27 };
static float quarter_rates[] = { 1.5, 2, 3, 4.5, 6, 9, 12, 13.5 };
static float subquarter_rates[] = { 0.75, 1, 1.5, 2.25, 3, 4.5, 6, 6.75 };

int get_minmaxrates_madwifi(char *ifname, int maxrate)
{
	float *rate;
	float *showrates = NULL;
	int len;
	char mode[32];
	char bw[16];

	sprintf(bw, "%s_channelbw", ifname);

	sprintf(mode, "%s_net_mode", ifname);
	if (nvram_match(mode, "b-only")) {
		rate = b_rates;
		len = sizeof(b_rates) / sizeof(float);
	}
	if (nvram_match(mode, "g-only")) {
		rate = ag_rates;
		len = sizeof(ag_rates) / sizeof(float);
		if (nvram_match(bw, "40")) {
			showrates = turbo_rates;
		}
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(float);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(float);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(float);
		}
	}
	if (nvram_match(mode, "a-only")) {
		rate = ag_rates;
		len = sizeof(ag_rates) / sizeof(float);
		if (nvram_match(bw, "40")) {
			showrates = turbo_rates;
		}
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(float);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(float);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(float);
		}
	}
	if (nvram_match(mode, "bg-mixed")) {
		rate = bg_rates;
		len = sizeof(bg_rates) / sizeof(float);
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(float);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(float);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(float);
		}
	}
	if (nvram_match(mode, "mixed")) {
		rate = bg_rates;
		len = sizeof(bg_rates) / sizeof(float);
		if (nvram_match(bw, "40")) {
			rate = ag_rates;
			len = sizeof(ag_rates) / sizeof(float);
			showrates = turbo_rates;
		}
		if (nvram_match(bw, "10")) {
			rate = half_rates;
			len = sizeof(half_rates) / sizeof(float);
		}
		if (nvram_match(bw, "5")) {
			rate = quarter_rates;
			len = sizeof(quarter_rates) / sizeof(float);
		}
		if (nvram_match(bw, "2")) {
			rate = subquarter_rates;
			len = sizeof(subquarter_rates) / sizeof(float);
		}
	}
	if (maxrate > 0) {
		maxrate--;
		if (maxrate >= len)
			maxrate = len - 1;
		if (showrates)
			return (int)(showrates[maxrate] * 1000000.0);
		else
			return (int)(rate[maxrate] * 1000000.0);
	}
	return (0);
}

void ddxrWlStatTable_madwifi()
{
	char *next;
	char *mac;
	// struct ether_addr *mac;
	char var[32];
	char temp[32];
	int count = getdevicecount();
	int i;
	int vap;
	char vapindex;
	struct ddxrWlStatTable_entry *entry, *mainentry;
	int txpower = 0;
	int antgain = 0;

	DEBUGMSGTL(("ddwrt:madwifi", "ddxrWlStatTable_madwifi\n"));

	for (i = 0; i < count; i++) {
		sprintf(var, "ath%d", i);
		vap = 0;
		DEBUGMSGTL(("ddwrt:madwifi", "main interface %d %d\n", i, vap));
		//  printf("<option value=\"%s\" %s >%s</option>\n", var, nvram_match("wifi_display", var) ? "selected=\"selected\"" : "", var);
		entry = ddxrWlStatTable_createEntry(i, vap);
		set_ddxrWlStatMajorInterface(entry, i);
		set_ddxrWlStatMinorInterface(entry, vap);
		set_ddxrWlStatIfaceName(entry, var);
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
		txpower = wifi_gettxpower(var);
		set_ddxrWlStatTxPower(entry, txpower);
#endif
		sprintf(temp, "%s_antgain", var);
		antgain = atoi(nvram_safe_get(temp));
		set_ddxrWlStatAntennaGain(entry, antgain);

		sprintf(temp, "%s_hwaddr", var);
		if (strlen(nvram_safe_get(temp)) > 0) {
			mac = (char *)ether_aton(nvram_safe_get(temp));
			set_ddxrWlStatBssid(entry, mac);
		}
		// only show frequency if interface is different than disabled
#ifdef HAVE_MADWIFI
		if (madwifi_getwirelessnetmode(var) > 1) {
			struct wifi_interface *freq = wifi_getfreq(var);
			if (freq)
				set_ddxrWlStatFreq(entry, get_wififreq(var, freq->freq));
			else
				set_ddxrWlStatFreq(entry, -1);
		} else
#endif
			set_ddxrWlStatFreq(entry, -1);

		sprintf(temp, "%s_ssid", var);
		set_ddxrWlStatSsid(entry, nvram_safe_get(temp));
		sprintf(temp, "%s_scanlist", var);
		set_ddxrWlStatScanlist(entry, nvram_safe_get(temp));

		set_ddxrWlStatnetmode(entry, (int)madwifi_getwirelessnetmode(var));
		set_ddxrWlStatmode(entry, (int)madwifi_getwirelessmode(var));
#ifdef HAVE_ATH9K
		sprintf(temp, "A 2ghz:%d 5ghz:%d 802.11n:%d 80211ac:%d", has_2ghz(var), has_5ghz(var), is_ath9k(var) | has_ac(var), has_ac(var));
#else
		sprintf(temp, "A 2ghz:%d 5ghz:%d 802.11n:%d 80211ac:%d", has_2ghz(var), has_5ghz(var), 0, 0);
#endif
		set_ddxrWlStatcardtype(entry, temp);
		sprintf(temp, "%s_minrate", var);
		set_ddxrWlStatminrate(entry, get_minmaxrates_madwifi(var, atoi(nvram_safe_get(temp))));
#ifdef HAVE_ATH9K
		if (is_mac80211(var)) {
			set_ddxrWlStatmaxrate(entry, wifi_getrate(var));
		} else {
#else
		{
#endif
			sprintf(temp, "%s_maxrate", var);
			set_ddxrWlStatmaxrate(entry, get_minmaxrates_madwifi(var, atoi(nvram_safe_get(temp))));
		}
		sprintf(temp, "%s_ff", var);
		set_ddxrWlStatfastframing(entry, (int)atoi(nvram_safe_get(temp)) + 1);
#ifdef HAVE_ATH9K
		if (is_mac80211(var))
			set_ddxrWlStatack(entry, (int)(mac80211_get_coverageclass(var) * 450));
		else
#endif
			set_ddxrWlStatack(entry, (int)get_distance_madwifi(var));
		sprintf(temp, "%s_txantenna", var);
		set_ddxrWlStatTxAntenna(entry, atoi(nvram_safe_get(temp)));
		sprintf(temp, "%s_rxantenna", var);
		set_ddxrWlStatRxAntenna(entry, atoi(nvram_safe_get(temp)));

		sprintf(temp, "%s_outdoor", var);
		set_ddxrWlStatOutdoorBand(entry, (int)atoi(nvram_safe_get(temp)) + 1);

		sprintf(temp, "%s_channelbw", var);
		set_ddxrWlStatChannelBW(entry, (int)atoi(nvram_safe_get(temp)));

#ifdef HAVE_ATH9K
		if (is_mac80211(var) && (atoi(nvram_safe_get(temp)) == 40 || atoi(nvram_safe_get(temp)) == 2040)) {
			sprintf(temp, "%s_nctrlsb", var);
			if (!strcmp(nvram_safe_get(temp), "upper"))
				set_ddxrWlStat80211nWideChannel(entry, (int)3);
			else
				set_ddxrWlStat80211nWideChannel(entry, (int)2);
		} else
#endif
			set_ddxrWlStat80211nWideChannel(entry, (int)1);
		sprintf(temp, "%s_closed", var);
		set_ddxrWlStatSSIDBroadcast(entry, (int)atoi(nvram_safe_get(temp)) + 1);

		sprintf(temp, "%s_distance", var);
		set_ddxrWlStatconfigack(entry, (int)atoi(nvram_safe_get(temp)));

		if (nvram_match("ath_regulatory", "0")) {
			set_ddxrWlStatRegulatory(entry, "SUPERCHANNEL");
		} else {
			sprintf(temp, "%s_regdomain", var);
			set_ddxrWlStatRegulatory(entry, nvram_safe_get(temp));
		}

		mainentry = entry;
		char *names = nvram_nget("ath%d_vifs", i);

		foreach(var, names, next) {
			vapindex = var[strlen(var) - 1];
			if (vapindex >= 49 && vapindex <= 57)
				vap = vapindex - 48;
			else
				vap = 99;
			DEBUGMSGTL(("ddwrt:madwifi", "vifs interace %d %d\n", i, vap));
			entry = ddxrWlStatTable_createEntry(i, vap);
			set_ddxrWlStatMajorInterface(entry, i);
			set_ddxrWlStatMinorInterface(entry, vap);
			set_ddxrWlStatIfaceName(entry, var);
			// same as above
			set_ddxrWlStatTxPower(entry, txpower);
			set_ddxrWlStatAntennaGain(entry, antgain);
			sprintf(temp, "%s_hwaddr", var);
			DEBUGMSGTL(("ddwrt:madwifi", "vifs hwaddr %s %s \n", var, nvram_safe_get(temp)));
			// mac=(char* ) ether_aton_r(nvram_safe_get(temp));
			// ether_aton_r(nvram_safe_get(temp),mac);
			if (strlen(nvram_safe_get(temp)) > 0) {
				mac = (char *)ether_aton(nvram_safe_get(temp));
				set_ddxrWlStatBssid(entry, mac);
			}

			set_ddxrWlStatFreq(entry, mainentry->ddxrWlStatFreq);
			sprintf(temp, "%s_ssid", var);
			set_ddxrWlStatSsid(entry, nvram_safe_get(temp));
			set_ddxrWlStatScanlist(entry, mainentry->ddxrWlStatScanlist);
			set_ddxrWlStatnetmode(entry, (int)madwifi_getwirelessnetmode(var));
			set_ddxrWlStatmode(entry, (int)madwifi_getwirelessmode(var));
			set_ddxrWlStatcardtype(entry, mainentry->ddxrWlStatcardtype);

			set_ddxrWlStatminrate(entry, mainentry->ddxrWlStatminrate);
			set_ddxrWlStatmaxrate(entry, mainentry->ddxrWlStatmaxrate);
			set_ddxrWlStatfastframing(entry, mainentry->ddxrWlStatfastframing);
			set_ddxrWlStatconfigack(entry, mainentry->ddxrWlStatconfigack);
			set_ddxrWlStatack(entry, mainentry->ddxrWlStatack);
			memcpy(entry->ddxrWlStatTxAntenna, mainentry->ddxrWlStatTxAntenna, sizeof(entry->ddxrWlStatTxAntenna));
			entry->ddxrWlStatTxAntenna_len = sizeof(entry->ddxrWlStatTxAntenna);
			memcpy(entry->ddxrWlStatRxAntenna, mainentry->ddxrWlStatRxAntenna, sizeof(entry->ddxrWlStatRxAntenna));
			entry->ddxrWlStatRxAntenna_len = sizeof(entry->ddxrWlStatRxAntenna);
			set_ddxrWlStatOutdoorBand(entry, mainentry->ddxrWlStatOutdoorBand);
			set_ddxrWlStatChannelBW(entry, mainentry->ddxrWlStatChannelBW);
			sprintf(temp, "%s_closed", var);
			set_ddxrWlStatSSIDBroadcast(entry, (int)atoi(nvram_safe_get(temp)) + 1);
			set_ddxrWlStatRegulatory(entry, mainentry->ddxrWlStatRegulatory);
		}
	}
}

void set_ddxrWlRtabMac(struct ddWlRtabTable_entry *entry, char *var)
{
	memcpy(entry->ddxrWlRtabMac, var, 6);
	entry->ddxrWlRtabMac_len = 6;
}

void set_ddxrWlRtabMajorInterface(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabMajorInterface = var;
}

void set_ddxrWlRtabMinorInterface(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabMinorInterface = var;
}

void set_ddxrWlRtabBssid(struct ddWlRtabTable_entry *entry, char *var)
{
	memcpy(entry->ddxrWlRtabBssid, var, 6);
	entry->ddxrWlRtabBssid_len = 6;
}

void set_ddxrWlRtabSNR(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabSNR = var;
}

void set_ddxrWlRtabIfaceName(struct ddWlRtabTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlRtabIfaceName, var);
}

void set_ddxrWlRtabnoise(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabnoise = var;
}

void set_ddxrWlRtabSignal(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabSignal = var;
}

void set_ddxrWlRtabtxrate(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabtxrate = var;
}

void set_ddxrWlRtabrxrate(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabrxrate = var;
}

void set_ddxrWlRtabType(struct ddWlRtabTable_entry *entry, int var)
{
	entry->ddxrWlRtabType = var;
}

void set_ddxrWlRtabSsid(struct ddWlRtabTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlRtabSsid, var);
}

void set_ddxrWlRtabAntennaname(struct ddWlRtabTable_entry *entry, char *var)
{
	set_strbuf(entry->ddxrWlRtabAntennaname, var);
}

void ddWlRtabTable_madwifi()
{
	char *next;
	char var[32];
	char temp[32];
	int count;
	int i;
	int turbo = 1;
	int vap;
	char vapindex;

	count = getdevicecount();
	DEBUGMSGTL(("ddwrt:init ", "deviceount %d\n", count));
	for (i = 0; i < count; i++) {
		sprintf(var, "ath%d", i);
		vap = 0;
		DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC interace %d %d\n", i, vap));
		sprintf(temp, "%s_channelbw", var);
		if (atoi(nvram_safe_get(temp)) == 40)
			turbo = 2;
		else
			turbo = 1;
#ifdef HAVE_MADWIFI
#ifdef HAVE_ATH9K
		if (is_mac80211(var)) {
			ddWlRtabTable_mac80211_assoc(var, 0, i, vap, 0, 0);
		} else
#endif
			ddWlRtabTable_madwifi_assoc(var, 0, turbo, i, vap, 0, 0);
#endif
		char *names = nvram_nget("ath%d_vifs", i);

		foreach(var, names, next) {
			vapindex = var[strlen(var) - 1];
			if (vapindex >= 49 && vapindex <= 57)
				vap = vapindex - 48;
			else
				vap = 99;
#ifdef HAVE_MADWIFI
#ifdef HAVE_ATH9K
			if (is_mac80211(var)) {
				ddWlRtabTable_mac80211_assoc(var, 0, i, vap, 0, 0);
			} else
#endif
				ddWlRtabTable_madwifi_assoc(var, 0, turbo, i, vap, 0, 0);
#endif
			DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC interace %d %d\n", i, vap));
		}
	}

	// show wds links
	int c = getdevicecount();
	vap = 0;
	for (i = 0; i < c; i++) {

		int s;

		for (s = 1; s <= 10; s++) {
			char wdsvarname[32] = { 0 };
			char wdsdevname[32] = { 0 };
			char wdsmacname[32] = { 0 };
			char *dev;
			char *hwaddr;
			char var[80];

			sprintf(wdsvarname, "ath%d_wds%d_enable", i, s);
			sprintf(wdsdevname, "ath%d_wds%d_if", i, s);
			sprintf(wdsmacname, "ath%d_wds%d_hwaddr", i, s);
			sprintf(temp, "ath%d_channelbw", i);
			if (nvram_match(temp, "40"))
				turbo = 2;
			else
				turbo = 1;
			DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC WDS %d %d \n", i, s));

			dev = nvram_safe_get(wdsdevname);
			// DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC WDS %d %d %s\n",i,vap,dev));
			if (dev == NULL || strlen(dev) == 0)
				continue;
			if (nvram_match(wdsvarname, "0"))
				continue;
			// wds-links are already shown aboave
#ifdef HAVE_ATH9K
			if (!is_mac80211(var))
#endif
#ifdef HAVE_MADWIFI
				ddWlRtabTable_madwifi_assoc(dev, 0, turbo, i, vap, 1, s);
#endif
			// ddWlRtabTable_mac80211_assoc(dev,0,i,vap,1,s);
		}
	}
}

#ifdef HAVE_MADWIFI
void ddWlRtabTable_madwifi_assoc(char *ifname, int cnt, int turbo, int ciface, int cvap, int is_wds, int wdscnt)
{
	char temp[32];
	char *bssid;
	int type = 0;
	struct ddWlRtabTable_entry *entry;

	unsigned char *cp;
	int s, len;
	struct iwreq iwr;
	char nb[32];
	sprintf(nb, "%s_bias", ifname);
	int bias = atoi(nvram_default_get(nb, "0"));
	if (!ifexists(ifname)) {
		DEBUGMSGTL(("ddwrt:madwifiassoc", "IOCTL_STA_INFO ifresolv %s failed!\n", ifname));
		return;
	}
	int state = get_radiostate(ifname);

	if (state == 0 || state == -1) {
		DEBUGMSGTL(("ddwrt:madwifiassoc", "IOCTL_STA_INFO radio %s not enabled!\n", ifname));
		return;
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		DEBUGMSGTL(("ddwrt:madwifiassoc", "socket(SOCK_DRAGM)\n"));
		return;
	}
	(void)memset(&iwr, 0, sizeof(struct iwreq));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	unsigned char *buf = (unsigned char *)malloc(24 * 1024);

	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		DEBUGMSGTL(("ddwrt:madwifiassoc", "IOCTL_STA_INFO for %s failed!\n", ifname));
		close(s);
		free(buf);
		return;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		// fprintf(stderr,"IOCTL_STA_INFO len<struct %s failed!\n",ifname);
		close(s);
		free(buf);
		return;
	}
	cp = buf;
	do {
		struct ieee80211req_sta_info *si;
		uint8_t *vp;

		si = (struct ieee80211req_sta_info *)cp;
		vp = (u_int8_t *)(si + 1);

		// if (cnt)
		// printf( ",");
		cnt++;
		char mac[32];

		strcpy(mac, ieee80211_ntoa(si->isi_macaddr));
		if (si->isi_noise == 0) {
			si->isi_noise = -95;
		}
		DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC %d %d %s\n", ciface, cvap, mac));
		entry = ddWlRtabTable_createEntry((char *)si->isi_macaddr, 6, ciface, cvap);
		set_ddxrWlRtabMac(entry, (char *)si->isi_macaddr);
		set_ddxrWlRtabMajorInterface(entry, (int)ciface);
		set_ddxrWlRtabMinorInterface(entry, (int)cvap);
		if (is_wds) {
			// take the hwaddr of the main interface
			sprintf(temp, "ath%d_hwaddr", ciface);
		} else {
			sprintf(temp, "%s_hwaddr", ifname);
		}
		DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC %d %d %s hwaddr: %s\n", ciface, cvap, temp, nvram_safe_get(temp)));
		if (strlen(nvram_safe_get(temp)) != 0) {
			bssid = (char *)ether_aton(nvram_safe_get(temp));
			set_ddxrWlRtabBssid(entry, bssid);
		}
		set_ddxrWlRtabSNR(entry, (int)si->isi_rssi);
		set_ddxrWlRtabIfaceName(entry, ifname);
		set_ddxrWlRtabnoise(entry, (int)(si->isi_noise + bias));
		set_ddxrWlRtabSignal(entry, (int)(si->isi_noise + si->isi_rssi + bias));
		if (si->isi_rates && ((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) != 0))
			set_ddxrWlRtabtxrate(entry, (int)(((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) / 2) * turbo) * (int)1000000);
		if (((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) != 0))
			set_ddxrWlRtabrxrate(entry, (int)(((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) / 2) * turbo) * (int)1000000);
		if (is_wds) {
			type = 3;	// other side is wdspeer
		} else {
			// AP / WDSAP
			if (madwifi_getapsta(ifname) == 0) {
				if (si->isi_athflags & IEEE80211_ATHC_WDS)
					type = 2;	// other side is wdsstation
				else
					type = 1;	// other side is station

			}
			// STA / WDSSTA
			if (madwifi_getapsta(ifname) == 1) {
				if (si->isi_athflags & IEEE80211_ATHC_WDS)
					type = 5;	// other side is wdsap
				else
					type = 4;	// other side is ap
			}
		}
		set_ddxrWlRtabType(entry, type);
		if (!is_wds) {
			int skfd;
			int getssidfromnvram = 1;
			struct wireless_config info;
			if ((skfd = iw_sockets_open()) >= 0) {
				if (iw_get_basic_config(skfd, ifname, &(info)) >= 0) {
					if (info.has_essid)
						DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC %d %d ifname: %s SSID FROM INTERFACE IS: _%s_ \n", ciface, cvap, ifname, info.essid));
					if (strlen(info.essid)) {
						set_ddxrWlRtabSsid(entry, info.essid);
						getssidfromnvram = 0;
					}
				}
				close(skfd);
			}
			if (getssidfromnvram) {
				sprintf(temp, "%s_hwaddr", ifname);
				set_ddxrWlRtabSsid(entry, nvram_safe_get(temp));
			}
		}
		if (is_wds) {
			sprintf(temp, "ath%d_wds%d_desc", ciface, wdscnt);
			set_ddxrWlRtabAntennaname(entry, nvram_safe_get(temp));
		}

/*
		int qual = (si->isi_noise + si->isi_rssi) * 124 + 11600;
		qual /= 10;

		if (si->isi_rates
		    && ((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) !=
			0)
		    && ((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) !=
			0)) {
			printf(
				  "'%s','%s','%s','%3dM','%3dM','%d','%d','%d','%d'",
				  mac, ifname, UPTIME(si->isi_uptime),
				  ((si->isi_rates[si->isi_txrate] &
				    IEEE80211_RATE_VAL) / 2) * turbo,
				  ((si->isi_rates[si->isi_rxrate] &
				    IEEE80211_RATE_VAL) / 2) * turbo,
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		} else {
			printf(
				  "'%s','%s','%s','N/A','N/A','%d','%d','%d','%d'",
				  mac, ifname, UPTIME(si->isi_uptime),
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		} */
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	free(buf);
	close(s);

	return;
}
#endif

#ifdef HAVE_ATH9K
void ddWlRtabTable_mac80211_assoc(char *ifname, int cnt, int ciface, int cvap, int is_wds, int wdscnt)
{
	char temp[32];
	char *bssid;
	int type = 0;
	struct ddWlRtabTable_entry *entry;

	unsigned char *cp;
	int s, len;

	char mac[32];
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	char nb[32];
	int bias, qual, it;
	char mode[32];
	char m[32];

	sprintf(nb, "%s_bias", ifname);
	bias = atoi(nvram_default_get(nb, "0"));
	// sprintf(it, "inactivity_time", ifname);
	it = atoi(nvram_default_get("inacttime", "300000"));
	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		if (cnt && wc->inactive_time < it)
			cnt++;
		DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC %d %d %s\n", ciface, cvap, mac));
		entry = ddWlRtabTable_createEntry((char *)wc->etheraddr, 6, ciface, cvap);
		set_ddxrWlRtabMac(entry, (char *)wc->etheraddr);
		set_ddxrWlRtabMajorInterface(entry, (int)ciface);
		set_ddxrWlRtabMinorInterface(entry, (int)cvap);
		if (is_wds) {
			// take the hwaddr of the main interface
			sprintf(temp, "ath%d_hwaddr", ciface);
		} else {
			sprintf(temp, "%s_hwaddr", ifname);
		}
		DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC %d %d %s hwaddr: %s\n", ciface, cvap, temp, nvram_safe_get(temp)));
		if (strlen(nvram_safe_get(temp)) != 0) {
			bssid = (char *)ether_aton(nvram_safe_get(temp));
			set_ddxrWlRtabBssid(entry, bssid);
		}
		set_ddxrWlRtabSNR(entry, (int)(wc->signal - wc->noise));
		set_ddxrWlRtabIfaceName(entry, ifname);
		set_ddxrWlRtabnoise(entry, (int)(wc->noise + bias));
		set_ddxrWlRtabSignal(entry, (int)(wc->signal + bias));
		if (wc->txrate)
			set_ddxrWlRtabtxrate(entry, (int)wc->txrate * (int)100000);
		if (wc->rxrate)
			set_ddxrWlRtabrxrate(entry, (int)wc->rxrate * (int)100000);
		if (is_wds) {
			type = 3;	// other side is wdspeer
		} else {
			// AP / WDSAP
			if (madwifi_getapsta(ifname) == 0) {
				if (wc->is_wds)	// TODO
					type = 2;	// other side is wdsstation
				else
					type = 1;	// other side is station

			}
			// STA / WDSSTA
			if (madwifi_getapsta(ifname) == 1) {
				strncpy(m, ifname, 4);
				m[4] = 0;
				sprintf(mode, "%s_mode", m);
				if (nvram_match(mode, "wdssta"))
					type = 5;	// other side is wdsap
				else
					type = 4;	// other side is ap
			}
		}
		set_ddxrWlRtabType(entry, type);
		if (!is_wds) {
			int skfd;
			int getssidfromnvram = 1;
			struct wireless_config info;
			if ((skfd = iw_sockets_open()) >= 0) {
				if (iw_get_basic_config(skfd, ifname, &(info)) >= 0) {
					if (info.has_essid)
						DEBUGMSGTL(("ddwrt:madwifiassoc", "ASSOC %d %d ifname: %s SSID FROM INTERFACE IS: _%s_ \n", ciface, cvap, ifname, info.essid));
					if (strlen(info.essid)) {
						set_ddxrWlRtabSsid(entry, info.essid);
						getssidfromnvram = 0;
					}
				}
				close(skfd);
			}
			if (getssidfromnvram) {
				sprintf(temp, "%s_hwaddr", ifname);
				set_ddxrWlRtabSsid(entry, nvram_safe_get(temp));
			}
		}
		if (is_wds) {
			sprintf(temp, "ath%d_wds%d_desc", ciface, wdscnt);
			set_ddxrWlRtabAntennaname(entry, nvram_safe_get(temp));
		}
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);

	return;
}
#endif				// HAVE_ATH9K
