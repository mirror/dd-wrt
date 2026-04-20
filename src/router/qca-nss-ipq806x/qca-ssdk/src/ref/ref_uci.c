/*
 * Copyright (c) 2013, 2015, 2017-2019, 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#ifdef IN_MISC
#include "fal_misc.h"
#endif
#ifdef IN_MIB
#include "fal_mib.h"
#endif
#ifdef IN_PORTCONTROL
#include "fal_port_ctrl.h"
#endif
#ifdef IN_PORTVLAN
#include "fal_portvlan.h"
#endif
#ifdef IN_FDB
#include "fal_fdb.h"
#endif
#ifdef IN_STP
#include "fal_stp.h"
#endif
#ifdef IN_IGMP
#include "fal_igmp.h"
#endif
#ifdef IN_QOS
#include "fal_qos.h"
#endif
#ifdef IN_ACL
#include "fal_acl.h"
#endif
#ifdef IN_PPPOE
#include "fal_pppoe.h"
#endif
#ifdef IN_SERVCODE
#include "fal_servcode.h"
#endif
#ifdef IN_CTRLPKT
#include "fal_ctrlpkt.h"
#endif
#ifdef IN_POLICER
#include "fal_policer.h"
#endif
#ifdef IN_SHAPER
#include "fal_shaper.h"
#endif
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include "shell.h"
#include "shell_io.h"
#include "shell_sw.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <generated/autoconf.h>
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#else
#include <net/switch.h>
#include <linux/ar8216_platform.h>
#endif
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include "ssdk_plat.h"
#include <linux/string.h>
#include "ref_uci.h"

#if defined(IN_TUNNEL)
#include "ref_tunnel.h"
#endif

#define MOD_NAME_MAX_LEN	32
#define COMMAND_NAME_MAX_LEN	128
#define	COMMAND_LINE_MAX_LEN	1024
#define	SWITCH_CFG_LEN_MAX	64
char module_name[MOD_NAME_MAX_LEN] = {0};
char command_name[COMMAND_NAME_MAX_LEN] = {0};
char whole_command_line[COMMAND_LINE_MAX_LEN] = {0};
char *val_ptr[SWITCH_CFG_LEN_MAX] = {0};
static unsigned int parameter_length = 0;
#ifdef IN_NAT
static char *vrf_dflt_str = "0";
static char *lb_dflt_str = "0";
static char *cookie_dflt_str = "0";
static char *priority_dflt_str = "no";
#endif
static char *param_dflt_str = " ";

#if defined(HPPE)
int parse_uci_option(struct switch_val *val, const char *option_names[], const int length)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0, index = 0;

	/* initialize all options to default value */
	while (index < length) {
		val_ptr[index++] = "default";
	}

	index = 0;
	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		}

		while (index < length) {
			if (!strcmp(ext_value_p->option_name, option_names[index])) {
				val_ptr[index] = (char*)ext_value_p->option_value;
				index = 0;
				break;
			}
			index++;
		}

		/* unknown option */
		if (index >= length) {
			SSDK_ERROR("Unknown option %s: %s\n",
					ext_value_p->option_name, ext_value_p->option_value);
			return -1;
		}
		switch_ext_p = switch_ext_p->next;
	}
	parameter_length = length;

	return rv;
}
#endif

#ifdef IN_QOS
#ifndef IN_QOS_MINI
static int
parse_qos_qtxbufsts(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "buffer_limit")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_qtxbufnr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "number")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_pttxbufsts(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "buffer_limit")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_pttxbufnr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "number")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptrxbufnr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "number")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "react_num")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptreden(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptmodepri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptschmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "weight")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptdefaultspri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "stag_pri")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptdefaultcpri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctag_pri")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptfsprists(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_stag_pri_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptfcprists(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_ctag_pri_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptquremark(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "table_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptgroup(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pcpgroup")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dscpgroup")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "flowgroup")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ptpri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pcpprec")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dscpprec")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "preheaderprec")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "flowprec")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "aclprec")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "postaclprec")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "pcpprecforce")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "dscpprecforce")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "preaclouterpriprece")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "preaclinnerpriprece")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifndef IN_QOS_MINI
static int
parse_qos_ptremark(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pcp_change_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dei_change_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dscp_change_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
static int
parse_qos_pcpmap(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pcp")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internalpcp")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldei")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internalpri")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldscp")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldropprec")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "dscpmask")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "dscpen")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "pcpen")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "deien")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "prien")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "dpen")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else if (!strcmp(ext_value_p->option_name, "qosprec")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_qos_flowmap(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "flow_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internalpcp")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldei")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internalpri")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldscp")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldropprec")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_dscpmap(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internalpcp")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldei")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internalpri")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldscp")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "internaldropprec")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_qscheduler(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "node_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "level")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "spid")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "edrrpri")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cdrrpri")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cdrr_id")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "edrr_id")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "edrrweight")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cdrrweight")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cdrrunit")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "edrrunit")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "drr_frame_mode")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_ringqueue(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "ring_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp0")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp1")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp2")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp3")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp4")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp5")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp6")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp7")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp8")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebmp9")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_dequeue(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dequeue_ctrl_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qos_portscheduler(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#endif
#endif

#ifdef IN_COSMAP
static int
parse_cos_mappri2q(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mappri2ehq(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "enhance_queue")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifndef IN_COSMAP_MINI
static int
parse_cos_mapdscp2pri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapdscp2dp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapup2pri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapup2dp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapdscp2ehpri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapdscp2ehdp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapup2ehpri(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapup2ehdp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_cos_mapegremark(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remark_dscp")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remark_up")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remark_dei")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "green_dscp")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dscp")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "green_up")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_up")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "green_dei")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dei")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_RATE
static int
parse_rate_portpolicer(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "combine_enable")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "couple_flag")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "color_aware")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "deficit_flag")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "c_bucket_enable")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "c_rate_flag")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "c_meter_interval")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "e_bucket_enable")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "e_rate_flag")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "e_meter_interval")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_rate_portshaper(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_rate_queueshaper(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_rate_aclpolicer(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "policer_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "couple_flag")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "color_aware")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "deficit_flag")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meter_interval")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_rate_ptaddratebyte(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "add_rate_bytes")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_rate_ptgolflowen(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "golbal_flow_control_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_PORTCONTROL
#ifndef IN_PORTCONTROL_MINI
static int
parse_port_duplex(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "duplex")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_speed(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "speed")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_autoadv(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "auto_adv")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_autonegenable(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_autonegrestart(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_port_txhdr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tx_frame_atheros_header_tag_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_rxhdr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rx_frame_atheros_header_tag_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_hdrtype(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "atheros_header_tag_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "atheros_header_tag_type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_PORTCONTROL_MINI
static int
parse_port_flowctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_control_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_flowctrlforcemode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_control_force_mode_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_powersave(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "power_save_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_hibernate(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "hibernate_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_txmacstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tx_mac_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_rxmacstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rx_mac_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_txfcstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tx_flow_control_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_rxfcstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rx_flow_control_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#ifndef IN_PORTCONTROL_MINI
static int
parse_port_bpstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "back_presure_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_linkforcemode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "link_force_mode_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_macloopback(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mac_loopback_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_congedrop(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_ringfcthresh(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ring_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "on_thresh")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "off_thresh")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_port_ptfcthresh(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "on_thresh")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "off_thresh")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_PORTCONTROL_MINI
static int
parse_port_ringfcen(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ring_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ringfc_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_ringunion(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ring_union_en")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_ieee8023az(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_crossover(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_prefermedium(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_fibermode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_localloopback(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_remoteloopback(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_magicframemac(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_wolstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_interfacemode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_interfacemodeapply(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_poweron(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_poweroff(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_reset(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_interface8023az(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_promiscmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "promisc_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
static int
parse_port_eeecfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eee_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eee_capability")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "lpi_sleep_timer")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "advertisement")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "lpi_tx_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eee_status")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "lpi_wakeup_timer")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "link_partner_advertisement")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}
		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_srcfiltercfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "srcfilter_enable")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "srcfilter_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_framemaxsize(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "frame_max_size")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_port_mtu(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mtusize")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mtuaction")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_port_mru(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mrusize")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mruaction")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_PORTCONTROL_MINI
static int
parse_port_srcfilter(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_switch_port_loopback(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "loopback_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "crc_stripped_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "loopback_rate")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#if defined (APPE)
#ifndef IN_PORTCONTROL_MINI
static int
parse_port8023ah(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "loopback_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_port_mtu_cfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mtu_enable")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mtu_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "extra_header_len")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eg_vlan_tag_flag")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#if defined(HPPE)
static const char *port_cntcfg[] = {
	"port_id",
	"rx_cnt_enable",
	"uc_tx_cnt_enable",
	"mc_tx_cnt_enable",
#if defined(APPE)
	"tl_rx_cnt_enable",
	"rx_cnt_mode",
	"tx_cnt_mode",
#endif
};
#endif

static int
parse_port_mru_mtu(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mru_size")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mtu_size")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_PORTVLAN
static int
parse_portvlan_ingress(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ingress_vlan_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_egress(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "egress_vlan_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_member(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "member")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_forcevid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_vid_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_forcemode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_svlantpid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "stag_tpid")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
static int
parse_portvlan_defaultsvid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "default_stag_vid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
static int
parse_portvlan_defaultcvid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "default_ctag_vid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_globalqinqmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ingress_qinq_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "egress_qinq_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "untouched_for_cpucode")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_ptqinqmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ingress_qinq_role")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "egress_qinq_role")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "tunnel_qinq_role")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}
#if defined(MPPE)
		else if (!strcmp(ext_value_p->option_name, "tunnel_ingress_port_select")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}
#endif
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#if defined(HPPE)
static int
parse_portvlan_intpid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctagtpid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stagtpid")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "tunnel_ctagtpid")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tunnel_stagtpid")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_egtpid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctagtpid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stagtpid")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_ingressfilter(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "membership_filter_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tagged_filter_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "untagged_filter_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "priority_tagged_filter_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "ctag_tagged_filter_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctag_untagged_filter_en")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctag_priority_tagged_filter_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_defaultvlantag(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "direction")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_ctag_vid_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_stag_vid_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_ctag_vid")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_stag_vid")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_ctag_pri")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_stag_pri")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_ctag_dei")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "default_stag_dei")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_tagpropagation(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "direction")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vid_propagation_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pri_propagation_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dei_propagation_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_translationmissaction(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "translation_miss_action")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_portvlan_egmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctag_egress_vlan_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stag_egress_vlan_mode")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifdef HPPE
static int
parse_portvlan_vsiegmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vsi_egress_vlan_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_vsiegmodeen(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vsi_egress_vlan_mode_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_counter(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static const char *portvlan_translationadv[] = {
	"port_id",
	"direction",
	"stagformat",
	"svid_en",
	"svid",
	"spcp_en",
	"spcp",
	"sdei_en",
	"sdei",
	"ctagformat",
	"cvid_en",
	"cvid",
	"cpcp_en",
	"cpcp",
	"cdei_en",
	"cdei",
	"frame_type_en",
	"frametype",
	"protocol_en",
	"protocol",
	"vsivalid",
	"vsi_en",
	"vsi",
#if defined(APPE)
	"vni_resv_enable",
	"vni_resv_type",
	"vni_resv",
#endif
	"swap_svid_cvid",
	"svid_translation_cmd",
	"svidtranslation",
	"cvid_translation_cmd",
	"cvidtranslation",
	"swap_spcp_cpcp",
	"spcp_translation_en",
	"spcptranslation",
	"cpcp_translation_en",
	"cpcptranslation",
	"swap_sdei_cdei",
	"sdei_translation_en",
	"sdeitranslation",
	"cdei_translation_en",
	"cdeitranslation",
	"counter_en",
	"counter_id",
	"vsi_translation_en",
	"vsitranslation",
#if defined(APPE)
	"src_info_enable",
	"src_info_type",
	"src_info",
	"vni_resv_enable",
	"vni_resv",
#endif
};

#endif
#ifdef APPE
static int
parse_portvlan_isol(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "isol_state")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
static int
parse_portvlan_isol_group(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "group_member")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#ifndef IN_PORTVLAN_MINI
static int
parse_portvlan_invlan(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ingress_tag_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_tlsmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tls_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_pripropagation(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_priority_propagation_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}



static int
parse_portvlan_vlanpropagation(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_propagation_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_translation(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "original_vid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "bi_direction")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "forward_direction")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "reverse_direction")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "svid")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cvid")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "original_vid_is_cvid")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "svid_enable")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cvid_enable")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "one_2_one_vlan")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_qinqmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "qinq_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_qinqrole(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "qinq_role")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_macvlanxlt(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "egress_mac_based_vlan")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_netiso(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "net_isolate")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_portvlan_egbypass(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "egress_translation_filter_bypass")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifdef DESS
static int
parse_portvlan_ptvrfid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif
#endif

#ifdef IN_VLAN
static int
parse_vlan_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_vlan_member(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tag_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_VLAN_MINI
static int
parse_vlan_learnsts(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_FDB
static int
parse_fdb_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		}else if(!strcmp(ext_value_p->option_name, "addr")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "fid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dacmd")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "sacmd")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dest_port")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "static")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "leaky")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_ver")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_override")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cross_pt_state")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "white_list_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance_en")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "type")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	if(val_ptr[12] == NULL) {
		val_ptr[12] = "no";
		parameter_length++;
	}
	return rv;
}
#ifndef IN_FDB_MINI
static int
parse_fdb_resventry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "addr")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "fid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dacmd")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "sacmd")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dest_port")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "static")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "leaky")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "clone")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_override")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cross_pt_state")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "white_list_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_fdb_portlearn(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_FDB_MINI
static int
parse_fdb_agectrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "aging_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_fdb_agetime(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "aging_time")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_FDB_MINI
static int
parse_fdb_vlansmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_searching_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_ptlearnlimit(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_counter")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_ptlearnexceedcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_exceed_cmd")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_learnlimit(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_counter")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_learnexceedcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "learn_exceed_cmd")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_ptlearnstatic(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_static_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_learnctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_ptlearnctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "learnaction")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_ptstationmove(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stationmove_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stationmove_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_fdb_ptmaclimitctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "maclimit_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "maclimit_counter")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "maclimit_exceed_action")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_RSS_HASH
static int
parse_rsshash_config(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "hash_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hask_mask")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_fragment_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_seed")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_sip_mix")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_dip_mix")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_protocol_mix")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_sport_mix")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_dport_mix")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_fin_inner")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_fin_outer")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_IGMP
static int
parse_igmp_mode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "igmp_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_cmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "igmp_command")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_portjoin(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "join_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_portleave(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "leave_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_rp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "route_port_bitmap")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_createstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "create_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_static(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "static_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_leaky(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "leaky_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_version3(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "version3_status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_queue(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_ptlearnlimit(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_counter")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_ptlearnexceedcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_exceed_cmd")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_igmp_multi(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "group_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "group_ip_addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_ip_addr")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "portmap")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlanid")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_SEC
static int
parse_sec_mac(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_ip(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_ip4(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_ip6(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_tcp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_udp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_icmp4(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_icmp6(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifdef HPPE
static int
parse_sec_expctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "excep_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "excep_cmd")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "deacclr_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3route_only_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2fwd_only_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2flow_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3flow_en")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "multicast_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
#ifdef APPE
		} else if (!strcmp(ext_value_p->option_name, "l2flow_type")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3flow_type")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
#endif
		}
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifndef IN_SEC_MINI
static int
parse_sec_l3parser(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "small_ip4ttl")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "small_ip6hoplimit")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_sec_l4parser(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag0")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag0_mask")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag1")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag1_mask")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag2")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag2_mask")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag3")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag3_mask")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag4")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag4_mask")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag5")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag5_mask")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag6")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag6_mask")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag7")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag7_mask")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#ifdef APPE
#ifndef IN_SEC_MINI
static int
parse_sec_l2expctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "excep_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "excep_cmd")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_tunnelexpctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "excep_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "excep_cmd")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "deacclr_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "profile0_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "profile1_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "profile2_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "profile3_en")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_tunnell3parser(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "small_ip4ttl")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "small_ip6hoplimit")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_tunnell4parser(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag0")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag0_mask")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag1")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag1_mask")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag2")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag2_mask")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag3")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag3_mask")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag4")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag4_mask")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag5")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag5_mask")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag6")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag6_mask")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag7")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcpflag7_mask")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_sec_tunnelflagsparser(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "entry_index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "entry_valid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "equal")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tunnel_header_type")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "flags")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mask")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif
#endif

#ifdef IN_MISC
#ifndef IN_MISC_MINI
static int
parse_misc_framemaxsize(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "frame_max_size")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_misc_ptunkucfilter(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_ptunkmcfilter(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_ptbcfilter(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_MISC_MINI
static int
parse_misc_autoneg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_misc_cpuport(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_MISC_MINI
static int
parse_misc_pppoecmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_pppoe(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_ptdhcp(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_arpcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
static int
parse_misc_eapolcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_eapolstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifndef IN_MISC_MINI
static int
parse_misc_rip(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_ptarpreq(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_ptarpack(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_extendpppoe(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "session_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "multicast_session")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "unicast_session")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l3if_index")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l3if_index_valid")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "smacaddr")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "smacaddr_valid")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "tl_l3if_index")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name,
					"tl_l3if_index_valid")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_pppoeid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pppoe_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_cpuvid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_rtdpppoe(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_glomacaddr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_framecrc(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_misc_pppoeen(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "l3if_index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pppoe_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_IP
static int
parse_ip_hostentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	char* vrf_temp = "0";
	val_ptr[6] = vrf_temp;
	parameter_length ++;
	val_ptr[7] = vrf_temp;
	parameter_length ++;
	val_ptr[12] = param_dflt_str;
	parameter_length ++;
	val_ptr[13] = vrf_temp;
	parameter_length ++;
	val_ptr[14] = vrf_temp;
	parameter_length ++;
	val_ptr[15] = vrf_temp;
	parameter_length ++;
	val_ptr[16] = param_dflt_str;
	parameter_length ++;
	val_ptr[17] = param_dflt_str;
	parameter_length ++;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_addr")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "interface_id")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance_num")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
			parameter_length --;
		}  else if (!strcmp(ext_value_p->option_name, "dstinfo")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if (!strcmp(ext_value_p->option_name, "synctoggle")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if (!strcmp(ext_value_p->option_name, "lan_wan")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[16] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if (!strcmp(ext_value_p->option_name, "sip_addr")) {
			val_ptr[17] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#if !defined(IN_IP_MINI)
static int
parse_ip_intfentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	char* vrf_temp = "0";
	val_ptr[1] = vrf_temp;
	parameter_length ++;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "vlan_low")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_high")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ipv4_route")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ipv6_route")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_ptarplearn(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_arplearn(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_ptipsrcguard(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_guard_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_ptarpsrcguard(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_guard_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_routestatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_ipunksrc(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_arpunksrc(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_agetime(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "age_time")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_wcmphashmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "wcmp_hash_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_defaultflowcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_cmd")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_defaultrtflowcmd(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_cmd")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_hostroute(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_valid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_version")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_addr")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "prefix_length")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_defaultroute(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_valid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "route_type")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_vrfbaseaddr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "base_addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_vrfbasemask(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "base_mask")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_rfsip4(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip4_addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_rfsip6(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip6_addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_ip_vsiarpsg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_violation_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_port_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_svlan_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourcegurad_cvlan_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceunkown_action")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_violation_action")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_port_en")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_svlan_en")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_cvlan_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceunknown_action")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_vsisg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourcegurad_violation_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_port_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_svlan_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_cvlan_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceunkown_action")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_violation_action")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_port_en")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_svlan_en")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_cvlan_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceunknown_action")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_portarpsg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_violation_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_port_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_svlan_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceguard_cvlan_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_arp_sourceunkown_action")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_violation_action")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_port_en")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_svlan_en")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceguard_cvlan_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_nd_sourceunkown_action")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_portsg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_violation_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_port_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_svlan_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceguard_cvlan_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_sourceunkown_action")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_violation_action")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_port_en")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_svlan_en")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceguard_cvlan_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_sourceunkown_action")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_pubip(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#if !defined(IN_IP_MINI)
static int
parse_ip_networkroute(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dstinfo")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "lan_wan")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4addr")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4addr_mask")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_ip_intf(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mru")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mtu")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ttl_dec_bypass_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4_unicast_route_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6_unicast_route_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "icmp_trigger_en")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ttl_exceed_action")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ttl_exceed_deacclr_en")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "macaddr_bitmap")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "dmac_check_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ipv6_mru")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ipv6_mtu")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "udp_zero_csum_action")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vpn_id")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_vsiintf(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3if_valid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3if_index")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_portintf(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3if_valid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3if_index")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_nexthop(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l3if_index")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip_to_me_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pub_ip_index")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stag_fmt")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "svid")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctag_fmt")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cvid")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dnat_ip")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_portmac(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "valid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_routemiss(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_mcmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2_ip4_multicast_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2_ip4_multicast_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2_ip6_multicast_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2_ip6_multicast_mode")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ip_globalctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "mru_fail_action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mru_deacclr_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mtu_fail_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mtu_deacclr_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mtu_nonfrag_fail_action")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "mtu_nonfrag_deacclr_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "prefix_bc_action")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "prefix_deacclr_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "icmp_redirect_action")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "icmp_redirect_deacclr_en")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_mode_0")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hash_mode_1")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "route_fail_no_eth_action")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_NAT
static int
parse_nat_natentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	val_ptr[4] = vrf_dflt_str;
	parameter_length ++;
	val_ptr[12] = param_dflt_str;
	parameter_length ++;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "select_index")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "translate_addr")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_number")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_range")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_naptentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	val_ptr[3] = vrf_dflt_str;
	parameter_length ++;
	val_ptr[4] = cookie_dflt_str;
	parameter_length ++;
	val_ptr[5] = lb_dflt_str;
	parameter_length ++;
	val_ptr[15] = param_dflt_str;
	parameter_length ++;
	val_ptr[16] = priority_dflt_str;
	parameter_length ++;
	val_ptr[17] = param_dflt_str;
	parameter_length ++;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "flow_cookie")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "translate_addr")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "translate_port")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
			parameter_length --;
		}  else if(!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[16] = (char*)ext_value_p->option_value;
			parameter_length --;
		}  else if(!strcmp(ext_value_p->option_name, "priority_val")) {
			val_ptr[17] = (char*)ext_value_p->option_value;
			parameter_length --;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_flowentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	val_ptr[13] = param_dflt_str;
	parameter_length ++;
	val_ptr[15] = param_dflt_str;
	parameter_length ++;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_cookie")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "priority_val")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
			parameter_length --;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_flowcookie(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "proto")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "flow_cookie")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_flowrfs(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "proto")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "flow_rfs")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}



static int
parse_nat_natstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_naptstatus(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_nathash(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "hash_flag")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_naptmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "napt_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_prvbaseaddr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "base_addr")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_prvaddrmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_pubaddr(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pub_addr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_natunksess(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_prvbasemask(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "base_addr_mask")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_nat_global(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	char *sync_str = "disable";
	char *portbmp_str = "0x20";
	val_ptr[1] = sync_str;
	parameter_length++;
	val_ptr[2] = portbmp_str;
	parameter_length++;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "sync")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
			parameter_length--;
		}  else if(!strcmp(ext_value_p->option_name, "portbmp")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
			parameter_length--;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_STP
static int
parse_stp_portstate(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "stp_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "stp_status")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_MIRROR
static int
parse_mirror_analypt(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "analysis_port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_mirror_ptingress(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ingress_port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_mirror_ptegress(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "egress_port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_mirror_analycfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "direction")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "analysis_port")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "analysis_priority")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_LEAKY
static int
parse_leaky_ucmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "unicast_leaky_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_leaky_mcmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "multicast_leaky_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_leaky_arpmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_leaky_ptucmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_leaky_ptmcmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_TRUNK
static int
parse_trunk_group(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "trunk_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "trunk_port_bitmap")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_trunk_hashmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "trunk_hash_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_trunk_failover(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "failover_en")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_MIB
static int
parse_mib_status(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_mib_cpukeep(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

#ifdef IN_ACL
#ifdef APPE
static sw_error_t
parse_acl_tunnel_info_field(struct switch_ext *ext_value_p, fal_acl_tunnel_info_t *rule)
{
	a_uint32_t tmpdata = 0;
	if(!strcmp(ext_value_p->option_name, "tunnel_type")) {
		cmd_data_check_attr("tunnel_type", (char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->tunnel_type = tmpdata & 0x1f;
		rule->tunnel_type_mask = 0x1f;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_TUNNEL_TYPE);
	} else if(!strcmp(ext_value_p->option_name, "inner_type")) {
		cmd_data_check_attr("hdr_type", (char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->inner_type = tmpdata & 0x3;
		rule->inner_type_mask = 0x3;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_TUNNEL_INNER_TYPE);
	} else if(!strcmp(ext_value_p->option_name, "tunnel_key_valid")) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
			&(rule->tunnel_key_valid), sizeof(rule->tunnel_key_valid));
		rule->tunnel_key_valid_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_TUNNEL_KEY_VALID);
	} else if(!strcmp(ext_value_p->option_name, "tunnel_key")) {
		cmd_data_check_uint32((char*)ext_value_p->option_value,
			&(rule->tunnel_key), sizeof(rule->tunnel_key));
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_TUNNEL_KEY);
	} else if(!strcmp(ext_value_p->option_name, "tunnel_key_mask")) {
		cmd_data_check_uint32((char*)ext_value_p->option_value,
			&(rule->tunnel_key_mask), sizeof(rule->tunnel_key_mask));
	}  else if(!strcmp(ext_value_p->option_name, "tunnel_decap_en")) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
			&(rule->tunnel_decap_en), sizeof(rule->tunnel_decap_en));
		rule->tunnel_decap_en_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_TUNNEL_DECAP_EN);
	} else if(!strcmp(ext_value_p->option_name, "tunnel_inverse_check_fields")) {
		if (!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_TUNNEL_INVERSE_ALL);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")) {
			FAL_FIELD_FLG_CLR(rule->field_flg,
				FAL_ACL_FIELD_TUNNEL_INVERSE_ALL);
		}
	}
	return SW_OK;
}

static sw_error_t
acl_rule_field_convert(fal_acl_rule_t * rule,
		fal_acl_rule_field_t * rule_field, a_bool_t to_inner)
{
	if(to_inner)
	{
		/*fields flag*/
		aos_mem_copy(rule_field->field_flg,
			rule->field_flg, sizeof(fal_acl_field_map_t));

		/*mac fields*/
		rule_field->is_fake_mac_header_mask = rule->is_fake_mac_header_mask;
		rule_field->is_fake_mac_header_val = rule->is_fake_mac_header_val;
		rule_field->is_snap_mask = rule->is_snap_mask;
		rule_field->is_snap_val = rule->is_snap_val;
		rule_field->is_ethernet_mask = rule->is_ethernet_mask;
		rule_field->is_ethernet_val = rule->is_ethernet_val;
		rule_field->is_ip_mask = rule->is_ip_mask;
		rule_field->is_ip_val= rule->is_ip_val;
		rule_field->is_ipv6_mask = rule->is_ipv6_mask;
		rule_field->is_ipv6_val = rule->is_ipv6_val;
		aos_mem_copy(rule_field->dest_mac_val.uc,
			rule->dest_mac_val.uc, sizeof(fal_mac_addr_t));
		aos_mem_copy(rule_field->dest_mac_mask.uc,
			rule->dest_mac_mask.uc, sizeof(fal_mac_addr_t));
		aos_mem_copy(rule_field->src_mac_val.uc,
			rule->src_mac_val.uc, sizeof(fal_mac_addr_t));
		aos_mem_copy(rule_field->src_mac_mask.uc,
			rule->src_mac_mask.uc, sizeof(fal_mac_addr_t));
		rule_field->ethtype_val = rule->ethtype_val;
		rule_field->ethtype_mask = rule->ethtype_mask;
		rule_field->stagged_val = rule->stagged_val;
		rule_field->stagged_mask = rule->stagged_mask;
		rule_field->stag_vid_op = rule->stag_vid_op;
		rule_field->stag_vid_val = rule->stag_vid_val;
		rule_field->stag_vid_mask = rule->stag_vid_mask;
		rule_field->stag_pri_val = rule->stag_pri_val;
		rule_field->stag_pri_mask = rule->stag_pri_mask;
		rule_field->stag_dei_val = rule->stag_dei_val;
		rule_field->stag_dei_mask = rule->stag_dei_mask;
		rule_field->ctagged_val = rule->ctagged_val;
		rule_field->ctagged_mask = rule->ctagged_mask;
		rule_field->ctag_vid_op = rule->ctag_vid_op;
		rule_field->ctag_vid_val = rule->ctag_vid_val;
		rule_field->ctag_vid_mask = rule->ctag_vid_mask;
		rule_field->ctag_pri_val = rule->ctag_pri_val;
		rule_field->ctag_pri_mask = rule->ctag_pri_mask;
		rule_field->ctag_cfi_val = rule->ctag_cfi_val;
		rule_field->ctag_cfi_mask = rule->ctag_cfi_mask;
		rule_field->pppoe_sessionid = rule->pppoe_sessionid;
		rule_field->pppoe_sessionid_mask = rule->pppoe_sessionid_mask;

		/*ipv4 fields*/
		rule_field->src_ip4_val = rule->src_ip4_val;
		rule_field->src_ip4_mask = rule->src_ip4_mask;
		rule_field->dest_ip4_val = rule->dest_ip4_val;
		rule_field->dest_ip4_mask = rule->dest_ip4_mask;
		rule_field->is_ipv4_option_mask = rule->is_ipv4_option_mask;
		rule_field->is_ipv4_option_val = rule->is_ipv4_option_val;

		/*ipv6 fields*/
		aos_mem_copy(rule_field->src_ip6_val.ul,
			rule->src_ip6_val.ul, sizeof(fal_ip6_addr_t));
		aos_mem_copy(rule_field->src_ip6_mask.ul,
			rule->src_ip6_mask.ul, sizeof(fal_ip6_addr_t));
		aos_mem_copy(rule_field->dest_ip6_val.ul,
			rule->dest_ip6_val.ul, sizeof(fal_ip6_addr_t));
		aos_mem_copy(rule_field->dest_ip6_mask.ul,
			rule->dest_ip6_mask.ul, sizeof(fal_ip6_addr_t));
		rule_field->is_mobility_header_mask = rule->is_mobility_header_mask;
		rule_field->is_mobility_header_val = rule->is_mobility_header_val;
		rule_field->is_fragment_header_mask = rule->is_fragment_header_mask;
		rule_field->is_fragment_header_val = rule->is_fragment_header_val;
		rule_field->is_other_header_mask = rule->is_other_header_mask;
		rule_field->is_other_header_val = rule->is_other_header_val;

		/*ip fields*/
		rule_field->ip_proto_val = rule->ip_proto_val;
		rule_field->ip_proto_mask = rule->ip_proto_mask;
		rule_field->ip_dscp_val = rule->ip_dscp_val;
		rule_field->ip_dscp_mask = rule->ip_dscp_mask;
		rule_field->dest_l4port_op = rule->dest_l4port_op;
		rule_field->dest_l4port_val = rule->dest_l4port_val;
		rule_field->dest_l4port_mask = rule->dest_l4port_mask;
		rule_field->src_l4port_op = rule->src_l4port_op;
		rule_field->src_l4port_val = rule->src_l4port_val;
		rule_field->src_l4port_mask = rule->src_l4port_mask;
		rule_field->tcp_flag_val = rule->tcp_flag_val;
		rule_field->tcp_flag_mask = rule->tcp_flag_mask;
		rule_field->icmp_type_code_op = rule->icmp_type_code_op;
		rule_field->icmp_type_val = rule->icmp_type_val;
		rule_field->icmp_type_mask = rule->icmp_type_mask;
		rule_field->icmp_code_val = rule->icmp_code_val;
		rule_field->icmp_code_mask = rule->icmp_code_mask;
		rule_field->is_fragement_mask = rule->is_fragement_mask;
		rule_field->is_fragement_val = rule->is_fragement_val;
		rule_field->is_first_frag_mask = rule->is_first_frag_mask;
		rule_field->is_first_frag_val = rule->is_first_frag_val;
		rule_field->l3_ttl_mask = rule->l3_ttl_mask;
		rule_field->l3_ttl = rule->l3_ttl;
		rule_field->l3_length_op = rule->l3_length_op;
		rule_field->l3_length = rule->l3_length;
		rule_field->l3_length_mask = rule->l3_length_mask;
		rule_field->l3_pkt_type_mask = rule->l3_pkt_type_mask;
		rule_field->l3_pkt_type = rule->l3_pkt_type;
		rule_field->is_ah_header_mask = rule->is_ah_header_mask;
		rule_field->is_ah_header_val = rule->is_ah_header_val;
		rule_field->is_esp_header_mask = rule->is_esp_header_mask;
		rule_field->is_esp_header_val = rule->is_esp_header_val;

		/*udf fields*/
		rule_field->udf0_op = rule->udf0_op;
		rule_field->udf0_val = rule->udf0_val;
		rule_field->udf0_mask = rule->udf0_mask;
		rule_field->udf1_op = rule->udf1_op;
		rule_field->udf1_val = rule->udf1_val;
		rule_field->udf1_mask = rule->udf1_mask;
		rule_field->udf2_op = rule->udf2_op;
		rule_field->udf2_val = rule->udf2_val;
		rule_field->udf2_mask = rule->udf2_mask;
		rule_field->udf3_val = rule->udf3_val;
		rule_field->udf3_mask = rule->udf3_mask;
		rule_field->udfprofile_val = rule->udfprofile_val;
		rule_field->udfprofile_mask = rule->udfprofile_mask;
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}
	return SW_OK;
}
#endif

static sw_error_t
parse_acl_action_field(struct switch_ext *ext_value_p, fal_acl_rule_t *rule)
{
	a_uint32_t tmpdata = 0;
	if(!strcmp(ext_value_p->option_name, "packet_drop")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_DENY);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_DENY);
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_PERMIT);
		}
	} else if(!strcmp(ext_value_p->option_name, "dscp_of_remark")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->dscp = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_DSCP);
	}
#if defined(CPPE) || defined(APPE)
	else if(!strcmp(ext_value_p->option_name, "dscp_of_remark_mask")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->dscp_mask = tmpdata;
	}
#endif
	else if(!strcmp(ext_value_p->option_name, "vlan_priority_of_remark")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->up = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "queue_of_remark")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->queue = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_QUEUE);
	} else if(!strcmp(ext_value_p->option_name, "redirect_to_cpu")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_RDTCPU);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_RDTCPU);
		}
	} else if(!strcmp(ext_value_p->option_name, "copy_to_cpu")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_CPYCPU);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_CPYCPU);
		}
	} else if(!strcmp(ext_value_p->option_name, "redirect_to_ports")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REDPT);
		cmd_data_check_pbmp((char*)ext_value_p->option_value, &rule->ports, 4);
	} else if(!strcmp(ext_value_p->option_name, "mirror")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_MIRROR);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_MIRROR);
		}
	} else if(!strcmp(ext_value_p->option_name, "remark_lookup_vid")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_LOOKUP_VID);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_REMARK_LOOKUP_VID);
		}
	} else if(!strcmp(ext_value_p->option_name, "stag_vid_of_remark")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_STAG_VID);
		cmd_data_check_uint16((char*)ext_value_p->option_value,
				(a_uint32_t *)&rule->stag_vid, 4);
	} else if(!strcmp(ext_value_p->option_name, "stag_priority_of_remark")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_STAG_PRI);
		cmd_data_check_uint16((char*)ext_value_p->option_value,
				(a_uint32_t *)&rule->stag_pri, 4);
	} else if(!strcmp(ext_value_p->option_name, "stag_dei_of_remark")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_STAG_DEI);
		cmd_data_check_uint16((char*)ext_value_p->option_value,
				(a_uint32_t *)&rule->stag_dei, 4);
	} else if(!strcmp(ext_value_p->option_name, "ctag_vid_of_remark")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_CTAG_VID);
		cmd_data_check_uint16((char*)ext_value_p->option_value,
				(a_uint32_t *)&rule->ctag_vid, 4);
	} else if(!strcmp(ext_value_p->option_name, "ctag_priority_of_remark")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_CTAG_PRI);
		cmd_data_check_uint16((char*)ext_value_p->option_value,
				(a_uint32_t *)&rule->ctag_pri, 4);
	} else if(!strcmp(ext_value_p->option_name, "ctag_cfi_of_remark")) {
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_REMARK_CTAG_CFI);
		cmd_data_check_uint16((char*)ext_value_p->option_value,
				(a_uint32_t *)&rule->ctag_cfi, 4);
	} else if(!strcmp(ext_value_p->option_name, "action_policer_id")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					(a_uint32_t *)&(rule->policer_ptr), sizeof(a_uint32_t));
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_POLICER_EN);
	} else if(!strcmp(ext_value_p->option_name, "action_arp_ptr")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					(a_uint32_t *)&(rule->arp_ptr), sizeof(a_uint32_t));
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_ARP_EN);
	} else if(!strcmp(ext_value_p->option_name, "action_wcmp_ptr")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					(a_uint32_t *)&(rule->wcmp_ptr), sizeof(a_uint32_t));
		FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_WCMP_EN);
	} else if(!strcmp(ext_value_p->option_name, "action_snat")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_POLICY_FORWARD_EN);
			rule->policy_fwd |= FAL_ACL_POLICY_SNAT;
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			rule->policy_fwd &= ~ FAL_ACL_POLICY_SNAT;
		}
	} else if(!strcmp(ext_value_p->option_name, "action_dnat")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_POLICY_FORWARD_EN);
			rule->policy_fwd |= FAL_ACL_POLICY_DNAT;
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			rule->policy_fwd &= ~ FAL_ACL_POLICY_DNAT;
		}
	} else if(!strcmp(ext_value_p->option_name, "bypass_egress_translation")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_BYPASS_EGRESS_TRANS);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_BYPASS_EGRESS_TRANS);
		}
	} else if(!strcmp(ext_value_p->option_name, "interrupt_trigger")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_MATCH_TRIGGER_INTR);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			FAL_ACTION_FLG_CLR(rule->action_flg,
				FAL_ACL_ACTION_MATCH_TRIGGER_INTR);
		}
	} else if(!strcmp(ext_value_p->option_name, "bypass_bitmap")) {
		cmd_data_check_uint32((char*)ext_value_p->option_value,
			&(rule->bypass_bitmap), sizeof(rule->bypass_bitmap));
	} else if(!strcmp(ext_value_p->option_name, "enqueue_priority")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->enqueue_pri = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI);
	} else if(!strcmp(ext_value_p->option_name, "stagformat")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->stag_fmt = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID);
	} else if(!strcmp(ext_value_p->option_name, "ctagformat")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->ctag_fmt = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID);
	} else if(!strcmp(ext_value_p->option_name, "internaldropprec")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->int_dp = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_INT_DP);
	} else if(!strcmp(ext_value_p->option_name, "servicecode")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->service_code= tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_SERVICE_CODE);
	} else if(!strcmp(ext_value_p->option_name, "cpucode")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->cpu_code = tmpdata;
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_CPU_CODE);
	} else if(!strcmp(ext_value_p->option_name, "metadata_en")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		if(tmpdata)
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_METADATA_EN);
	} else if(!strcmp(ext_value_p->option_name, "synctoggle")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		if(tmpdata)
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_SYN_TOGGLE);
	}
#if defined(CPPE) || defined(APPE)
	else if(!strcmp(ext_value_p->option_name, "qos_res_prec")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->qos_res_prec = tmpdata;
	}
#endif
#if defined(APPE)
	else if(!strcmp(ext_value_p->option_name, "action_route")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_POLICY_FORWARD_EN);
			rule->policy_fwd |= FAL_ACL_POLICY_ROUTE;
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			rule->policy_fwd &= ~ FAL_ACL_POLICY_ROUTE;
		}
	} else if(!strcmp(ext_value_p->option_name, "action_snapt")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_POLICY_FORWARD_EN);
			rule->policy_fwd |= FAL_ACL_POLICY_SNAPT;
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			rule->policy_fwd &= ~ FAL_ACL_POLICY_SNAPT;
		}
	} else if(!strcmp(ext_value_p->option_name, "action_dnapt")) {
		if(!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_ACTION_FLG_SET(rule->action_flg,
				FAL_ACL_ACTION_POLICY_FORWARD_EN);
			rule->policy_fwd |= FAL_ACL_POLICY_DNAPT;
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")){
			rule->policy_fwd &= ~ FAL_ACL_POLICY_DNAPT;
		}
	} else if(!strcmp(ext_value_p->option_name, "napt_l4_port")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
			&(tmpdata), sizeof(tmpdata));
		rule->napt_l4_port = tmpdata & 0xffff;
	} else if(!strcmp(ext_value_p->option_name, "cascade_data")) {
		cmd_data_check_integer((char*)ext_value_p->option_value,
			&(tmpdata), 0x7f, 0);
		rule->cascade_data = tmpdata & 0x7f;
		FAL_ACTION_FLG_SET(rule->action_flg_ext, FAL_ACL_ACTION_CASCADE);
	} else if(!strcmp(ext_value_p->option_name, "vpn_type")) {
		cmd_data_check_attr("vpn_type", (char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->vpn_type= tmpdata & 0x1;
		FAL_ACTION_FLG_SET(rule->action_flg_ext, FAL_ACL_ACTION_VPN);
	} else if(!strcmp(ext_value_p->option_name, "vpn_id")) {
		cmd_data_check_integer((char*)ext_value_p->option_value,
			&(tmpdata), 0x3f, 0);
		rule->vpn_id= tmpdata & 0x3f;
	} else if(!strcmp(ext_value_p->option_name, "disable_learn")) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
			(a_bool_t *)&tmpdata, sizeof(tmpdata));
		if(tmpdata)
			FAL_ACTION_FLG_SET(rule->action_flg_ext, FAL_ACL_ACTION_LEARN_DIS);
	} else if(!strcmp(ext_value_p->option_name, "policy_id")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
			&(tmpdata), sizeof(tmpdata));
		rule->policy_id = tmpdata & 0xffff;
	}
#endif
#if defined(MPPE)
	else if(!strcmp(ext_value_p->option_name, "metadata_pri")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->metadata_pri = tmpdata & 0xf;
	} else if(!strcmp(ext_value_p->option_name, "cookie_val")) {
		cmd_data_check_uint64((char*)ext_value_p->option_value,
			&(rule->cookie_val), sizeof(a_uint64_t));
		rule->cookie_val = tmpdata & 0xffff;
	} else if(!strcmp(ext_value_p->option_name, "cookie_pri")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
			&tmpdata, sizeof(tmpdata));
		rule->cookie_pri = tmpdata & 0xf;
	}
#endif
	return SW_OK;
}

static sw_error_t
parse_acl_rule_field(struct switch_ext *ext_value_p, fal_acl_rule_t *rule, a_bool_t is_inner)
{
	a_uint32_t tmpdata = 0;

	if((!is_inner && !strcmp(ext_value_p->option_name, "dst_mac_address")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_dst_mac_address"))) {
		cmd_data_check_macaddr((char*)ext_value_p->option_value,
					&(rule->dest_mac_val), sizeof(fal_mac_addr_t));
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_DA);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "dst_mac_address_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_dst_mac_address_mask"))) {
		cmd_data_check_macaddr((char*)ext_value_p->option_value,
					&(rule->dest_mac_mask), sizeof(fal_mac_addr_t));
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "src_mac_address")) ||
		(is_inner &&!strcmp(ext_value_p->option_name, "inner_src_mac_address"))) {
		cmd_data_check_macaddr((char*)ext_value_p->option_value,
					&(rule->src_mac_val), sizeof(fal_mac_addr_t));
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_SA);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "src_mac_address_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_src_mac_address_mask"))) {
		cmd_data_check_macaddr((char*)ext_value_p->option_value,
					&(rule->src_mac_mask), sizeof(fal_mac_addr_t));
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ethernet_type")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ethernet_type"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&(tmpdata), sizeof(tmpdata));
		rule->ethtype_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_ETHTYPE);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ethernet_type_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ethernet_type_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&(tmpdata), sizeof(tmpdata));
		rule->ethtype_mask = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->vid_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_VID);
	} else if(!strcmp(ext_value_p->option_name, "vlan_id_mask")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->vid_mask = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "vlan_priority")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->up_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_UP);
	} else if(!strcmp(ext_value_p->option_name, "vlan_priority_mask")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->up_mask = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "tagged")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->tagged_val = tmpdata;
		rule->tagged_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_TAGGED);
	} else if(!strcmp(ext_value_p->option_name, "tagged_mask")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->tagged_mask = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "cfi")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->cfi_val = tmpdata;
		rule->cfi_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_CFI);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ctagged")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ctagged"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ctagged_val = tmpdata;
		rule->ctagged_mask = BITS(0,3);
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_CTAGGED);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ctag_vlan_id")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ctag_vlan_id"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&(tmpdata), sizeof(tmpdata));
		rule->ctag_vid_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_CTAG_VID);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ctag_vlan_id_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ctag_vlan_id_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ctag_vid_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ctag_vlan_priority")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ctag_vlan_priority"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ctag_pri_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_CTAG_PRI);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ctag_vlan_priority_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ctag_vlan_priority_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ctag_pri_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ctag_cfi")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ctag_cfi"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ctag_cfi_val = tmpdata;
		rule->ctag_cfi_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_CTAG_CFI);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "stagged")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_stagged"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->stagged_val = tmpdata;
		rule->stagged_mask = BITS(0,3);
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_STAGGED);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "stag_vlan_id")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_stag_vlan_id"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->stag_vid_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_STAG_VID);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "stag_vlan_id_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_stag_vlan_id_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->stag_vid_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "stag_vlan_priority")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_stag_vlan_priority"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->stag_pri_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_STAG_PRI);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "stag_vlan_priority_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_stag_vlan_priority_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&(tmpdata), sizeof(tmpdata));
		rule->stag_pri_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "stag_dei")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_stag_dei"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->stag_dei_val = tmpdata;
		rule->stag_dei_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_MAC_STAG_DEI);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv4_src_address")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv4_src_address"))) {
		cmd_data_check_ip4addr((char*)ext_value_p->option_value,
					&(rule->src_ip4_val), 4);
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP4_SIP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv4_src_address_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv4_src_address_mask"))) {
		cmd_data_check_ip4addr((char*)ext_value_p->option_value,
					&(rule->src_ip4_mask), 4);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv4_dst_address")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv4_dst_address"))) {
		cmd_data_check_ip4addr((char*)ext_value_p->option_value,
					&(rule->dest_ip4_val), 4);
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP4_DIP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv4_dst_address_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv4_dst_address_mask"))) {
		cmd_data_check_ip4addr((char*)ext_value_p->option_value,
					&(rule->dest_ip4_mask), 4);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv6_src_address")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv6_src_address"))) {
		cmd_data_check_ip6addr((char*)ext_value_p->option_value,
					&(rule->src_ip6_val), 16);
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP6_SIP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv6_src_address_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv6_src_address_mask"))) {
		cmd_data_check_ip6addr((char*)ext_value_p->option_value,
					&(rule->src_ip6_mask), 16);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv6_dst_address")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv6_dst_address"))) {
		cmd_data_check_ip6addr((char*)ext_value_p->option_value,
					&(rule->dest_ip6_val), 16);
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP6_DIP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ipv6_dst_address_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ipv6_dst_address_mask"))) {
		cmd_data_check_ip6addr((char*)ext_value_p->option_value,
					&(rule->dest_ip6_mask), 16);
	} else if(!strcmp(ext_value_p->option_name, "ipv6_flow_label")) {
		cmd_data_check_uint32((char*)ext_value_p->option_value,
					&(rule->ip6_lable_val),
					sizeof(rule->ip6_lable_val));
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP6_LABEL);
	} else if(!strcmp(ext_value_p->option_name, "ipv6_flow_label_mask")) {
		cmd_data_check_uint32((char*)ext_value_p->option_value,
					&(rule->ip6_lable_mask),
					sizeof(rule->ip6_lable_mask));
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_protocol")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_protocol"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ip_proto_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP_PROTO);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_protocol_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_protocol_mask"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ip_proto_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_dscp")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_dscp"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ip_dscp_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_IP_DSCP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_dscp_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_dscp_mask"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ip_dscp_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_dst_port")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_dst_port"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->dest_l4port_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_L4_DPORT);
		rule->dest_l4port_op = FAL_ACL_FIELD_MASK;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_dst_port_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_dst_port_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->dest_l4port_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_src_port")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_src_port"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->src_l4port_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_L4_SPORT);
		rule->src_l4port_op = FAL_ACL_FIELD_MASK;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "ip_src_port_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_ip_src_port_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->src_l4port_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "icmp_type")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_icmp_type"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->icmp_type_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_ICMP_TYPE);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "icmp_type_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_icmp_type_mask"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->icmp_type_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "icmp_code")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_icmp_code"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->icmp_code_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_ICMP_CODE);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "icmp_code_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_icmp_code_mask"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->icmp_code_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "tcp_flag")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_tcp_flag"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->tcp_flag_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_TCP_FLAG);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "tcp_flag_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_tcp_flag_mask"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->tcp_flag_mask = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "ripv1")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->ripv1_val = tmpdata;
		rule->ripv1_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_RIPV1);
	} else if(!strcmp(ext_value_p->option_name, "dhcpv4")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->dhcpv4_val = tmpdata;
		rule->dhcpv4_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_DHCPV4);
	} else if(!strcmp(ext_value_p->option_name, "dhcpv6")) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->dhcpv6_val = tmpdata;
		rule->dhcpv6_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_DHCPV6);
	} else if(!strcmp(ext_value_p->option_name, "udf_start")) {
		cmd_data_check_udf_type((char*)ext_value_p->option_value,
					&rule->udf_type, sizeof(rule->udf_type));
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_UDF);
	} else if(!strcmp(ext_value_p->option_name, "udf_offset")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf_offset = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "udf_length")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf_len = tmpdata;
	} else if(!strcmp(ext_value_p->option_name, "udf_value")) {
		tmpdata = sizeof(rule->udf_val);
		cmd_data_check_udf_element((char*)ext_value_p->option_value,
					(a_uint8_t *)&rule->udf_val, (a_uint32_t *)&tmpdata);
	} else if(!strcmp(ext_value_p->option_name, "udf_mask")) {
		tmpdata = sizeof(rule->udf_mask);
		cmd_data_check_udf_element((char*)ext_value_p->option_value,
					(a_uint8_t *)&rule->udf_mask, (a_uint32_t *)&tmpdata);
	} else if(!strcmp((char*)ext_value_p->option_name, "user_defined_field_value")) {
		cmd_data_check_udf_element((char*)ext_value_p->option_value,
				(a_uint8_t *)&(rule->udf_val[0]), (a_uint32_t *)&tmpdata);
		rule->udf_len = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_UDF);
	} else if(!strcmp(ext_value_p->option_name, "user_defined_field_mask")) {
		cmd_data_check_udf_element((char*)ext_value_p->option_value,
				(a_uint8_t *)&(rule->udf_mask[0]), (a_uint32_t *)&tmpdata);
		rule->udf_len = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_ip")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_ip"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_ip_val), sizeof(rule->is_ip_val));
		rule->is_ip_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_IP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_ip6")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_ip6"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_ipv6_val), sizeof(rule->is_ipv6_val));
		rule->is_ipv6_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_IPV6);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_fakemacheader")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_fakemacheader"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_fake_mac_header_val),
					sizeof(rule->is_fake_mac_header_val));
		rule->is_fake_mac_header_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_FAKE_MAC_HEADER);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_snap")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_snap"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_snap_val), sizeof(rule->is_snap_val));
		rule->is_snap_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_SNAP);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_ethernet")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_ethernet"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_ethernet_val), sizeof(rule->is_ethernet_val));
		rule->is_ethernet_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_ETHERNET);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_fragment")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_fragment"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_fragement_val), sizeof(rule->is_fragement_val));
		rule->is_fragement_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_ahheader")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_ahheader"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_ah_header_val), sizeof(rule->is_ah_header_val));
		rule->is_ah_header_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_AH_HEADER);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_espheader")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_espheader"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_esp_header_val), sizeof(rule->is_esp_header_val));
		rule->is_esp_header_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_ESP_HEADER);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_mobilityheader")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_mobilityheader"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_mobility_header_val),
					sizeof(rule->is_mobility_header_val));
		rule->is_mobility_header_val = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_MOBILITY_HEADER);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_fragmentheader")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_fragmentheader"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_fragment_header_val),
					sizeof(rule->is_fragment_header_val));
		rule->is_fragment_header_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_FRAGMENT_HEADER);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_otherheader")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_otherheader"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_other_header_val),
					sizeof(rule->is_other_header_val));
		rule->is_other_header_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_OTHER_EXT_HEADER);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_ip4option")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_ip4option"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_ipv4_option_val),
					sizeof(rule->is_ipv4_option_val));
		rule->is_ipv4_option_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_IPV4_OPTION);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "is_firstfragment")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_is_firstfragment"))) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->is_first_frag_val), sizeof(rule->is_first_frag_val));
		rule->is_first_frag_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_FIRST_FRAGMENT);
	} else if(!strcmp(ext_value_p->option_name, "vsi_valid")) {
		cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(rule->vsi_valid), sizeof(rule->vsi_valid));
		rule->vsi_valid_mask = 1;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_VSI_VALID);
	} else if(!strcmp(ext_value_p->option_name, "vsi")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->vsi = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_VSI);
	} else if(!strcmp(ext_value_p->option_name, "vsi_mask")) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->vsi_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "pppoe_session_id")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_pppoe_session_id"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->pppoe_sessionid = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_PPPOE_SESSIONID);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "pppoe_session_id_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_pppoe_session_id_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->pppoe_sessionid_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "l3ttl")) ||
		(!is_inner && !strcmp(ext_value_p->option_name, "inner_l3ttl"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->l3_ttl = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_L3_TTL);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "l3ttl_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_l3ttl_mask"))) {
		cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->l3_ttl_mask= tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "l3length")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_l3length"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->l3_length = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_L3_LENGTH);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "l3length_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_l3length_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->l3_length_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "l3packettype")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_l3packettype"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->l3_pkt_type = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "l3packettype_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_l3packettype_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->l3_pkt_type_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_op0")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_op0"))) {
		fal_acl_field_op_t op = FAL_ACL_FIELD_MASK;
		cmd_data_check_fieldop((char*)ext_value_p->option_value,
					FAL_ACL_FIELD_MASK, &op);
		printk("%s, %d", (char*)ext_value_p->option_value, op);
		rule->udf0_op= op;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val0")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val0"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf0_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_UDF0);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val0_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val0_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf0_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_op1")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_op1"))) {
		fal_acl_field_op_t op = FAL_ACL_FIELD_MASK;
		cmd_data_check_fieldop((char*)ext_value_p->option_value,
					FAL_ACL_FIELD_MASK, &op);
		rule->udf1_op= op;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val1")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val1"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf1_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_UDF1);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val1_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val1_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf1_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val2")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val2"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf2_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_UDF2);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val2_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val2_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf2_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val3")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val3"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf3_val = tmpdata;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_UDF3);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_val3_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_val3_mask"))) {
		cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
		rule->udf3_mask = tmpdata;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "inverse_check_fields")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_inverse_check_fields"))) {
		if (!strcmp(ext_value_p->option_value, "y") ||
			!strcmp(ext_value_p->option_value, "yes")) {
			FAL_FIELD_FLG_SET(rule->field_flg,
				FAL_ACL_FIELD_INVERSE_ALL);
		} else if(!strcmp(ext_value_p->option_value, "n") ||
			!strcmp(ext_value_p->option_value, "no")) {
			FAL_FIELD_FLG_CLR(rule->field_flg,
				FAL_ACL_FIELD_INVERSE_ALL);
		}
#if defined(APPE)
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_op2")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_op2"))) {
		fal_acl_field_op_t op = FAL_ACL_FIELD_MASK;
		cmd_data_check_fieldop((char*)ext_value_p->option_value,
					FAL_ACL_FIELD_MASK, &op);
		rule->udf2_op= op;
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_profile")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_profile"))) {
		cmd_data_check_integer((char*)ext_value_p->option_value,
					&tmpdata, 0x7, 0x0);
		rule->udfprofile_val = tmpdata & 0x7;
		FAL_FIELD_FLG_SET(rule->field_flg, FAL_ACL_FIELD_UDFPROFILE);
	} else if((!is_inner && !strcmp(ext_value_p->option_name, "user_defined_profile_mask")) ||
		(is_inner && !strcmp(ext_value_p->option_name, "inner_user_defined_profile_mask"))) {
		cmd_data_check_integer((char*)ext_value_p->option_value,
					&tmpdata, 0x7, 0x0);
		rule->udfprofile_mask = tmpdata & 0x7;
#endif
	}
	return SW_OK;
}

static int
parse_acl_rule(a_uint32_t dev_id, struct switch_val *val)
{
	a_uint32_t prio = 0;
	a_uint32_t i;
	a_uint32_t portmap = 0;
	a_uint32_t rule_id = 0;
	a_uint32_t obj_type = 0;
	a_uint32_t obj_value = 0;
	a_uint32_t list_id = 0xffffffff;
	fal_acl_rule_t rule;
#if defined(APPE)
	fal_acl_rule_t *inner_rule = NULL;
#endif
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	a_uint32_t tmpdata = 0;
	memset(&rule, 0, sizeof(fal_acl_rule_t));
#if defined(APPE)
	inner_rule = (fal_acl_rule_t *)aos_mem_alloc(sizeof(fal_acl_rule_t));
	if(inner_rule == NULL)
	{
		SSDK_ERROR("inner rule allocate fail\n");
		return SW_FAIL;
	}
#endif
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "list_id")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value,
						&list_id, sizeof(a_uint32_t));
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rule_id")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value,
						&rule_id, sizeof(a_uint32_t));
			if(rule_id == 0) {
				printk("ACL rule ID should begin with 1. Please Notice!\r\n");
			}
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "priority")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value,
						&prio, sizeof(a_uint32_t));
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rule_type")) {
			cmd_data_check_ruletype((char*)ext_value_p->option_value,
						&(rule.rule_type), sizeof(a_uint32_t));
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "is_postrouting")) {
			cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
						&(rule.post_routing), sizeof(rule.post_routing));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_POST_ROURING_EN);
		} else if(!strcmp(ext_value_p->option_name, "acl_pool")) {
			cmd_data_check_integer((char*)ext_value_p->option_value,
						&(tmpdata), 1, 0);
			rule.acl_pool = tmpdata;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_RES_CHAIN);
		} else if(!strcmp(ext_value_p->option_name, "port_bitmap")) {
			cmd_data_check_pbmp((char*)ext_value_p->option_value, &portmap, 4);
		} else if(!strcmp((char*)ext_value_p->option_name, "obj_type")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value, &obj_type, 4);
		} else if(!strcmp((char*)ext_value_p->option_name, "obj_value")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value, &obj_value, 4);
		}

		parse_acl_rule_field(ext_value_p, &rule, A_FALSE);
		parse_acl_action_field(ext_value_p, &rule);

#if defined(APPE)
		if(rule.rule_type == FAL_ACL_RULE_TUNNEL_MAC ||
			rule.rule_type == FAL_ACL_RULE_TUNNEL_IP4 ||
			rule.rule_type == FAL_ACL_RULE_TUNNEL_IP6 ||
			rule.rule_type == FAL_ACL_RULE_TUNNEL_UDF)
		{
			if(!strcmp(ext_value_p->option_name, "inner_rule_type")) {
				cmd_data_check_ruletype((char*)ext_value_p->option_value,
					&(rule.inner_rule_field.rule_type), sizeof(a_uint32_t));
			}
			parse_acl_rule_field(ext_value_p, inner_rule, A_TRUE);
			parse_acl_tunnel_info_field(ext_value_p, &rule.tunnel_info);
		}
#endif

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}
#if defined(APPE)
	if(rule.rule_type == FAL_ACL_RULE_TUNNEL_MAC ||
		rule.rule_type == FAL_ACL_RULE_TUNNEL_IP4 ||
		rule.rule_type == FAL_ACL_RULE_TUNNEL_IP6 ||
		rule.rule_type == FAL_ACL_RULE_TUNNEL_UDF)
	{
		acl_rule_field_convert(inner_rule, &rule.inner_rule_field, A_TRUE);
	}
	aos_mem_free(inner_rule);
	inner_rule = NULL;
#endif
	/*to be compatible with previous version which didn't define list id*/
	if(0xffffffff == list_id) {
		list_id = rule_id;
		rule_id = 0;
	} else {
		rule.pri = prio & 0x7;
		prio = (prio >> 3) & 0x3f;
	}
	SSDK_DEBUG("uci set acl list %d, rule %d\n", list_id, rule_id);
	SSDK_DEBUG("uci set acl portbitmap 0x%x, obj_type %d, obj_value %d\n",
			portmap, obj_type, obj_value);
	fal_acl_list_creat(dev_id, list_id, prio);
	fal_acl_rule_add(dev_id, list_id, rule_id, 1, &rule);
	/*bind to port bitmap*/
	if( portmap != 0 ) {
		for (i = 0; i < AR8327_NUM_PORTS; i++) {
			fal_acl_list_unbind(dev_id, list_id, 0, 0, i);
			if (portmap & (0x1 << i)) {
				rv = fal_acl_list_bind(dev_id, list_id, 0, 0, i);
				if(rv != SW_OK){
					SSDK_ERROR("uci set acl fail %d\n", rv);
				}
			}
		}
	} else {
		rv = fal_acl_list_bind(dev_id, list_id, 0, obj_type, obj_value);
		if(rv != SW_OK){
			SSDK_ERROR("uci set acl fail %d\n", rv);
		}
	}
	fal_acl_status_set(dev_id, A_TRUE);

	return rv;
}

static int
parse_acl_udfprofile(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "user_defined_type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "user_defined_offset")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "user_defined_length")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_acl_udf(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "packet_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "user_defined_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "user_defined_offset")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#ifdef APPE
static int
parse_acl_udfprofileentry(a_uint32_t dev_id, struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	a_uint32_t tmpdata = 0, profile_id = 0;
	fal_acl_udf_profile_entry_t entry;
	memset(&entry, 0, sizeof(fal_acl_udf_profile_entry_t));
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "profile_id")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value,
					&profile_id, sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "l3_type")) {
			cmd_data_check_attr("l3_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.l3_type = tmpdata & 0x3;
			FAL_FIELD_FLG_SET(entry.field_flag,
					FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
		} else if(!strcmp(ext_value_p->option_name, "l4_type")) {
			cmd_data_check_attr("l4_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.l4_type = tmpdata & 0x7;
			FAL_FIELD_FLG_SET(entry.field_flag,
					FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
		}
		switch_ext_p = switch_ext_p->next;
	}
	rv = fal_acl_udf_profile_entry_add(dev_id, profile_id, &entry);
	SSDK_DEBUG("uci set acl udfprofileentry rv %d\n", rv);

	return rv;
}

static int
parse_acl_udfprofilecfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "profile_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "user_defined_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "user_defined_offset")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_acl_vpgroup(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vport_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vport_type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vpgroup_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_FLOW
static int
parse_flow_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "add_mode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "entrytype")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "host_addr_type")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "host_addr_index")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "protocol")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "age")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "srcintf_valid")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "srcintf_index")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "fwdtype")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "snat_nexthop")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "snat_srcport")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dnat_nexthop")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dnat_dstport")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "route_nexthop")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "portvalid")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "routeport")) {
			val_ptr[16] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridgeport")) {
			val_ptr[17] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "deacclr_en")) {
			val_ptr[18] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "copy_tocpu_en")) {
			val_ptr[19] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "syntoggle")) {
			val_ptr[20] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "priprofile")) {
			val_ptr[21] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "sevicecode")) {
			val_ptr[22] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "iptype")) {
			val_ptr[23] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip4addr")) {
			val_ptr[26] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "srcport")) {
			val_ptr[24] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dstport")) {
			val_ptr[25] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tree_id")) {
			val_ptr[27] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "pmtu_check_l3")) {
			val_ptr[28] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pmtu")) {
			val_ptr[29] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vpn_id")) {
			val_ptr[30] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_vlan_format_valid")) {
			val_ptr[31] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_svlan_format")) {
			val_ptr[32] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_cvlan_format")) {
			val_ptr[33] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "wifi_qos_en")) {
			val_ptr[34] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "wifi_qos")) {
			val_ptr[35] = (char*)ext_value_p->option_value;
		}
#endif
#if defined(MPPE)
		else if (!strcmp(ext_value_p->option_name, "qos_type")) {
			val_ptr[36] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_nexthop_valid")) {
			val_ptr[37] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_nexthop")) {
			val_ptr[38] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "policer_valid")) {
			val_ptr[39] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "policer_index")) {
			val_ptr[40] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_flow_status(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_flow_age(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "agetime")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ageunit")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_flow_mgmt(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "flowtype")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "flowdirection")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "flow_miss_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "frag_bypass_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "tcp_specific_bypass_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "all_bypass_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "keysel")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static const char *flow_host[] = {
	"add_mode",
	"entry_id",
	"entrytype",
	"host_addr_type",
	"host_addr_index",
	"protocol",
	"age",
	"srcintf_valid",
	"srcintf_index",
	"fwdtype",
	"snat_nexthop",
	"snat_srcport",
	"dnat_nexthop",
	"dnat_dstport",
	"route_nexthop",
	"portvalid",
	"routeport",
	"bridgeport",
	"deacclr_en",
	"copy_tocpu_en",
	"syntoggle",
	"priprofile",
	"sevicecode",
	"iptype",
	"ip4addr",
	"srcport",
	"dstport",
	"tree_id",
#if defined(APPE)
	"pmtu_check_l3",
	"pmtu",
	"vpn_id",
	"bridge_vlan_format_valid",
	"bridge_svlan_format",
	"bridge_cvlan_format",
	"wifi_qos_en",
	"wifi_qos",
#endif
#if defined(MPPE)
	"qos_type",
	"bridge_nexthop_valid",
	"bridge_nexthop",
	"policer_valid",
	"policer_index",
#endif
	"ipentry_id",
	"entry_flags",
	"entry_status",
	"ip_addr",
	"mac_addr",
	"interface_id",
	"load_balance_num",
	"vrf_id",
	"port_id",
	"action",
	"mirror",
	"counter",
	"destinfo",
	"ip_synctoggle",
	"lan_wan",
};

static const char *flow_global[] = {
	"srcif_check_action",
	"srcif_check_deacclr_en",
	"service_loop_en",
	"service_loop_action",
	"service_loop_deacclr_en",
	"flow_deacclr_action",
	"sync_mismatch_action",
	"sync_mismatch_deacclr_en",
	"hash_mode_0",
	"hash_mode_1",
#if defined(APPE) || defined(CPPE)
	"flow_mismatch_copy_escape_en",
#endif
#if defined(APPE)
	"ptmu_fail_action",
	"ptmu_fail_deacclr_en",
	"ptmu_fail_df_action",
	"ptmu_fail_df_deacclr_en",
	"l2_vpn_en",
	"l3_vpn_en",
#endif
#if defined(MPPE)
	"flow_cookie_pri",
#endif
};

#endif

#ifdef IN_BM
static int
parse_bm_ctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "enable")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_bm_portgroupmap(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_bm_groupbuff(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bufnum")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_bm_portrsvbuff(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "prealloc_bufnum")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "react_bufnum")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_bm_portsthresh(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "maxthreshold")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "resume_offset")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_bm_portdthresh(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "weight")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "sharedceiling")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "resumeoffset")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "resume_min_threshold")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#endif

#ifdef IN_QM
static int
parse_qm_ucastqbase(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "srcprofile")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "servicecode_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "servicecode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cpucode_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cpucode")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "destport")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuebase")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "profile")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_ucastpriclass(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "profile")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "class")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#if !defined(IN_QM_MINI)
static int
parse_qm_mcastpriclass(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "class")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_qm_queue(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_ucasthash(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "profile")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "rsshash")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "queuehash")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#if !defined(IN_QM_MINI)
static int
parse_qm_ucastdflthash(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "queuevalue")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_mcastcpucode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "cpucode")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "class")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_qm_acctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "obj_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "admis_ctrl_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "admis_flowctrl_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_acprebuffer(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "obj_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bufnum")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_acqgroup(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_acsthresh(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "obj_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "color_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "wred_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "greenmax")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "green_min_offset")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "yel_max_offset")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "yel_min_offset")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "red_max_offset")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "red_min_offset")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "green_resume_offset")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "yel_resume_offset")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "red_resume_offset")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_acdthresh(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "color_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "wred_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "sharedweight")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "green_min_offset")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "yel_max_offset")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "yel_min_offset")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "red_max_offset")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "red_min_offset")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "green_resume_off")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "yel_resume_offset")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "red_resume_offset")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ceiling")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_acgbuff(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "group_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "prealloc_bufnum")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "total_bufnum")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_cntctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "cnt_en")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_cnt(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_enqueue(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "enqueue_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_qm_srcprofile(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "sourceprofile")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#endif

#ifdef IN_SERVCODE
static int
parse_servcode_config(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "servcode_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "destport_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "destport_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bypass_bitmap_0")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bypass_bitmap_1")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bypass_bitmap_2")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
#ifdef APPE
		} else if (!strcmp(ext_value_p->option_name, "bypass_bitmap_3")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
#endif
		} else if (!strcmp(ext_value_p->option_name, "direction")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "field_update_bitmap")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "next_servicecode")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "hardwareservices")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "offsetselection")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		switch_ext_p = switch_ext_p->next;
	}
	parameter_length = 12;

	return rv;
}

static int
parse_servcode_loopcheck(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "loopcheck_en")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#if defined(MPPE)
static int
parse_servcode_portservcode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "servcode_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_servcode_athtag(a_uint32_t dev_id, struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	a_uint32_t tmpdata = 0, servcode_index = 0;
	fal_servcode_athtag_t entry = {0};

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "servcode_id")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value,
						&servcode_index, sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "athtag_en")) {
			cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
						&(entry.athtag_en), sizeof(entry.athtag_en));
			entry.athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_INSERT);
		} else if (!strcmp(ext_value_p->option_name, "athtag_action")) {
			cmd_data_check_attr("athtag_action", (char*)ext_value_p->option_value,
						&tmpdata, sizeof(tmpdata));
			entry.action = tmpdata & 0x7;
			entry.athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_ACTION);
		} else if(!strcmp(ext_value_p->option_name, "athtag_bypass_fwd_en")) {
			cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(entry.bypass_fwd_en), sizeof(entry.bypass_fwd_en));
			entry.athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_BYPASS_FWD_EN);
		} else if(!strcmp(ext_value_p->option_name, "athtag_dest_port")) {
			cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(a_uint32_t));
			entry.dest_port = tmpdata & 0x7f;
			entry.athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_DEST_PORT);
		}  else if(!strcmp(ext_value_p->option_name, "athtag_field_disable")) {
			cmd_data_check_confirm((char*)ext_value_p->option_value, A_FALSE,
					&(entry.field_disable), sizeof(entry.field_disable));
			entry.athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_FIELD_DISABLE);
		}
		switch_ext_p = switch_ext_p->next;
	}
	rv = fal_servcode_athtag_set(dev_id, servcode_index, &entry);
	SSDK_DEBUG("uci set servcode athtag rv %d\n", rv);
	return rv;
}
#endif
#endif

#ifdef IN_CTRLPKT
static int
parse_ctrlpkt_ethernettype(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "profile_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ethernettype")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ctrlpkt_rfdb(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "profile_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "rfdb_macaddr")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ctrlpkt_appprofile(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_bitmap")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ethtype_profile_bitmap")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "rfdb_profile_bitmap")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "eapol_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "pppoe_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "igmp_en")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "arp_request_en")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "arp_reponse_en")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dhcp4_en")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dhcp6_en")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
#if defined (APPE)
		} else if (!strcmp(ext_value_p->option_name, "8023ah_oam_en")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
#endif
		} else if (!strcmp(ext_value_p->option_name, "mld_en")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6ns_en")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ip6na_en")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ctrlpkt_profile_action")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "sourceguard_bypass")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "l2filter_bypass")) {
			val_ptr[16] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ingress_stp_bypass")) {
			val_ptr[17] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "ingress_vlan_filter_bypass")) {
			val_ptr[18] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}
		switch_ext_p = switch_ext_p->next;
	}
	parameter_length = 19;

	return rv;
}
#if defined (APPE)
static int
parse_ctrlpkt_vpgroup(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vpgroup_id")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_ctrlpkt_tunneldecap(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "cpu_code")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "enable")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_POLICER
static int
parse_policer_timeslot(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "timeslot")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_policer_fcscompensation(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "fcscompensation_length")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_policer_portentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "metertype")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "policer_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "vppolicer_index")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "coupling_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "colormode")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "frametype")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "metermode")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meterunit")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "cir_max")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "eir_max")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "next_ptr")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_end")) {
			val_ptr[16] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_coupling_en")) {
			val_ptr[17] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "yellow_priremark_en")) {
			val_ptr[18] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dropprec_remark_en")) {
			val_ptr[19] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_pcpremark_en")) {
			val_ptr[20] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_deiremark_en")) {
			val_ptr[21] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "yellow_dscpremark_en")) {
			val_ptr[22] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_remap_en")) {
			val_ptr[23] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "yellowpri")) {
			val_ptr[24] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dropprec")) {
			val_ptr[25] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellowpcp")) {
			val_ptr[26] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellowdei")) {
			val_ptr[27] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "yellowdscp")) {
			val_ptr[28] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "redaction")) {
			val_ptr[29] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_priremark_en")) {
			val_ptr[30] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_dropprec_remark_en")) {
			val_ptr[31] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_pcpremark_en")) {
			val_ptr[32] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_deiremark_en")) {
			val_ptr[33] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "red_dscpremark_en")) {
			val_ptr[34] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_remap_en")) {
			val_ptr[35] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "redpri")) {
			val_ptr[36] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_dropprec")) {
			val_ptr[37] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "redpcp")) {
			val_ptr[38] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "reddei")) {
			val_ptr[39] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "reddscp")) {
			val_ptr[40] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}
		switch_ext_p = switch_ext_p->next;
	}
	parameter_length = 41;
	return rv;
}

static int
parse_policer_aclentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "metertype")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "policer_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "coupling_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "colormode")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "metermode")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meterunit")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "cir_max")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "eir_max")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "next_ptr")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_end")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_coupling_en")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "yellow_priremark_en")) {
			val_ptr[16] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dropprec_remark_en")) {
			val_ptr[17] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_pcpremark_en")) {
			val_ptr[18] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_deiremark_en")) {
			val_ptr[19] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "yellow_dscpremark_en")) {
			val_ptr[20] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_remap_en")) {
			val_ptr[21] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "yellowpri")) {
			val_ptr[22] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dropprec")) {
			val_ptr[23] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellowpcp")) {
			val_ptr[24] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellowdei")) {
			val_ptr[25] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "yellowdscp")) {
			val_ptr[26] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "redaction")) {
			val_ptr[27] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_priremark_en")) {
			val_ptr[28] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_dropprec_remark_en")) {
			val_ptr[29] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_pcpremark_en")) {
			val_ptr[30] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_deiremark_en")) {
			val_ptr[31] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "red_dscpremark_en")) {
			val_ptr[32] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_remap_en")) {
			val_ptr[33] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "redpri")) {
			val_ptr[34] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_dropprec")) {
			val_ptr[35] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "redpcp")) {
			val_ptr[36] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "reddei")) {
			val_ptr[37] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "reddscp")) {
			val_ptr[38] = (char*)ext_value_p->option_value;
		}
#endif
		else {
			rv = -1;
			break;
		}
		switch_ext_p = switch_ext_p->next;
	}
	parameter_length = 39;

	return rv;
}

static int
parse_policer_bypass(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "frame_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "bypass_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifdef APPE
#ifndef IN_POLICER_MINI
static int
parse_policer_priremap(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "internal_pri")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "internal_dp")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meter_color")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remap_dscp")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remap_pcp")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remap_dei")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_policer_ctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "head")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tail")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#endif

#ifdef IN_SHAPER
static int
parse_shaper_porttimeslot(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "timeslot")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_flowtimeslot(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "timeslot")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_queuetimeslot(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "timeslot")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_ipgcompensation(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ipgcompensation_length")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_porttoken(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctoken_negative_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctokennum")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_flowtoken(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "flow_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctoken_negative_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctokennum")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "etoken_negative_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "etokennum")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_queuetoken(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctoken_negative_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctokennum")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "etoken_negative_en")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "etokennum")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_portshaper(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meterunit")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cshaper_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "framemode")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_queueshaper(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "metertype")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "coupling_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meterunit")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cshaper_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "cir_max")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eshaper_en")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "eir_max")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "next_ptr")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_end")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_coupling_en")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "framemode")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}
		else {
			rv = -1;
			break;
		}
		switch_ext_p = switch_ext_p->next;
	}
	parameter_length = 16;

	return rv;
}

static int
parse_shaper_flowshaper(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;

	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "flow_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "metertype")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "coupling_en")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meterunit")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cshaper_en")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "cir_max")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eshaper_en")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if (!strcmp(ext_value_p->option_name, "eir_max")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[11] = (char*)ext_value_p->option_value;
		}
#if defined(APPE)
		else if(!strcmp(ext_value_p->option_name, "next_ptr")) {
			val_ptr[12] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_end")) {
			val_ptr[13] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "grp_coupling_en")) {
			val_ptr[14] = (char*)ext_value_p->option_value;
		}
#endif
		else if(!strcmp(ext_value_p->option_name, "framemode")) {
			val_ptr[15] = (char*)ext_value_p->option_value;
		}
		else {
			rv = -1;
			break;
		}
			switch_ext_p = switch_ext_p->next;
	}
	parameter_length = 16;

	return rv;
}

#ifdef APPE
static int
parse_shaper_queueshaperctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "head")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tail")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_shaper_flowshaperctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "head")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tail")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif
#endif

#ifdef IN_QOS
static int
parse_qos(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	#ifndef IN_QOS_MINI
	if(!strcmp(command_name, "QTxBufSts")) {
		rv = parse_qos_qtxbufsts(val);
	} else if(!strcmp(command_name, "QTxBufNr")) {
		rv = parse_qos_qtxbufnr(val);
	} else if(!strcmp(command_name, "PtTxBufSts")) {
		rv = parse_qos_pttxbufsts(val);
	} else if(!strcmp(command_name, "PtTxBufNr")) {
		rv = parse_qos_pttxbufnr(val);
	} else if(!strcmp(command_name, "PtRxBufNr")) {
		rv = parse_qos_ptrxbufnr(val);
	} else if(!strcmp(command_name, "PtRedEn")) {
		rv = parse_qos_ptreden(val);
	} else if(!strcmp(command_name, "PtMode")) {
		rv = parse_qos_ptmode(val);
	} else if(!strcmp(command_name, "PtModePri")) {
		rv = parse_qos_ptmodepri(val);
	} else if(!strcmp(command_name, "PtschMode")) {
		rv = parse_qos_ptschmode(val);
	} else if(!strcmp(command_name, "PtDefaultSpri")) {
		rv = parse_qos_ptdefaultspri(val);
	} else if(!strcmp(command_name, "PtDefaultCpri")) {
		rv = parse_qos_ptdefaultcpri(val);
	} else if(!strcmp(command_name, "PtFSpriSts")) {
		rv = parse_qos_ptfsprists(val);
	} else if(!strcmp(command_name, "PtFCpriSts")) {
		rv = parse_qos_ptfcprists(val);
	} else if(!strcmp(command_name, "PtQuRemark")) {
		rv = parse_qos_ptquremark(val);
	} else if (!strcmp(command_name, "Ptgroup")) {
		rv = parse_qos_ptgroup(val);
	} else if (!strcmp(command_name, "Ptpriprece")) {
		rv = parse_qos_ptpri(val);
	}
#ifndef IN_QOS_MINI
	else if (!strcmp(command_name, "Ptremark")) {
		rv = parse_qos_ptremark(val);
	} else if (!strcmp(command_name, "Pcpmap")) {
		rv = parse_qos_pcpmap(val);
	}
#endif
	else if (!strcmp(command_name, "Flowmap")) {
		rv = parse_qos_flowmap(val);
	} else if (!strcmp(command_name, "Dscpmap")) {
		rv = parse_qos_dscpmap(val);
	} else if (!strcmp(command_name, "Qscheduler")) {
		rv = parse_qos_qscheduler(val);
	} else if (!strcmp(command_name, "Ringqueue")) {
		rv = parse_qos_ringqueue(val);
	} else if (!strcmp(command_name, "Dequeue")) {
		rv = parse_qos_dequeue(val);
	} else if (!strcmp(command_name, "Portscheduler")) {
		rv = parse_qos_portscheduler(val);
	}
	#endif

	return rv;
}
#endif

#ifdef IN_COSMAP
static int
parse_cos(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Pri2Q")) {
		rv = parse_cos_mappri2q(val);
	} else if(!strcmp(command_name, "Pri2Ehq")) {
		rv = parse_cos_mappri2ehq(val);
	}
	#ifndef IN_COSMAP_MINI
	else if(!strcmp(command_name, "Dscp2Pri")) {
		rv = parse_cos_mapdscp2pri(val);
	} else if(!strcmp(command_name, "Dscp2Dp")) {
		rv = parse_cos_mapdscp2dp(val);
	} else if(!strcmp(command_name, "Up2Pri")) {
		rv = parse_cos_mapup2pri(val);
	} else if(!strcmp(command_name, "Up2Dp")) {
		rv = parse_cos_mapup2dp(val);
	} else if(!strcmp(command_name, "Dscp2ehPri")) {
		rv = parse_cos_mapdscp2ehpri(val);
	} else if(!strcmp(command_name, "Dscp2ehDp")) {
		rv = parse_cos_mapdscp2ehdp(val);
	} else if(!strcmp(command_name, "Up2ehPri")) {
		rv = parse_cos_mapup2ehpri(val);
	} else if(!strcmp(command_name, "Up2ehDp")) {
		rv = parse_cos_mapup2ehdp(val);
	} else if(!strcmp(command_name, "EgRemark")) {
		rv = parse_cos_mapegremark(val);
	}
	#endif

	return rv;
}
#endif

#ifdef IN_RATE
static int
parse_rate(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "PortPolicer")) {
		rv = parse_rate_portpolicer(val);
	} else if(!strcmp(command_name, "PortShaper")) {
		rv = parse_rate_portshaper(val);
	} else if(!strcmp(command_name, "QueueShaper")) {
		rv = parse_rate_queueshaper(val);
	} else if(!strcmp(command_name, "AclPolicer")) {
		rv = parse_rate_aclpolicer(val);
	} else if(!strcmp(command_name, "PtAddRateByte")) {
		rv = parse_rate_ptaddratebyte(val);
	} else if(!strcmp(command_name, "PtGolflowen")) {
		rv = parse_rate_ptgolflowen(val);
	}

	return rv;
}
#endif

#ifdef IN_PORTCONTROL
static int
parse_port(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "TxHdr")) {
		rv = parse_port_txhdr(val);
	} else if(!strcmp(command_name, "RxHdr")) {
		rv = parse_port_rxhdr(val);
	} else if(!strcmp(command_name, "HdrType")) {
		rv = parse_port_hdrtype(val);
	}
#ifndef IN_PORTCONTROL_MINI
	else if(!strcmp(command_name, "Duplex")) {
		rv = parse_port_duplex(val);
	} else if(!strcmp(command_name, "Speed")) {
		rv = parse_port_speed(val);
	} else if(!strcmp(command_name, "AutoAdv")) {
		rv = parse_port_autoadv(val);
	} else if(!strcmp(command_name, "AutoNegEnable")) {
		rv = parse_port_autonegenable(val);
	} else if(!strcmp(command_name, "AutoNegRestart")) {
		rv = parse_port_autonegrestart(val);
	} else if(!strcmp(command_name, "FlowCtrl")) {
		rv = parse_port_flowctrl(val);
	} else if(!strcmp(command_name, "FlowCtrlForceMode")) {
		rv = parse_port_flowctrlforcemode(val);
	} else if(!strcmp(command_name, "PowerSave")) {
		rv = parse_port_powersave(val);
	} else if(!strcmp(command_name, "Hibernate")) {
		rv = parse_port_hibernate(val);
	} else if(!strcmp(command_name, "TxMacStatus")) {
		rv = parse_port_txmacstatus(val);
	} else if(!strcmp(command_name, "RxMacStatus")) {
		rv = parse_port_rxmacstatus(val);
	} else if(!strcmp(command_name, "TxFcStatus")) {
		rv = parse_port_txfcstatus(val);
	} else if(!strcmp(command_name, "RxFcStatus")) {
		rv = parse_port_rxfcstatus(val);
	} else if(!strcmp(command_name, "BpStatus")) {
		rv = parse_port_bpstatus(val);
	} else if(!strcmp(command_name, "LinkForceMode")) {
		rv = parse_port_linkforcemode(val);
	} else if(!strcmp(command_name, "MacLoopback")) {
		rv = parse_port_macloopback(val);
	} else if(!strcmp(command_name, "CongeDrop")) {
		rv = parse_port_congedrop(val);
	} else if(!strcmp(command_name, "RingFcThresh")) {
		rv = parse_port_ringfcthresh(val);
	}
#endif
	else if(!strcmp(command_name, "PtFcThresh")) {
		rv = parse_port_ptfcthresh(val);
	}
#ifndef IN_PORTCONTROL_MINI
	else if(!strcmp(command_name, "RingUnion")) {
		rv = parse_port_ringunion(val);
	} else if(!strcmp(command_name, "RingFcen")) {
		rv = parse_port_ringfcen(val);
	} else if(!strcmp(command_name, "Ieee8023az")) {
		rv = parse_port_ieee8023az(val);
	} else if(!strcmp(command_name, "Crossover")) {
		rv = parse_port_crossover(val);
	} else if(!strcmp(command_name, "PreferMedium")) {
		rv = parse_port_prefermedium(val);
	} else if(!strcmp(command_name, "FiberMode")) {
		rv = parse_port_fibermode(val);
	} else if(!strcmp(command_name, "LocalLoopback")) {
		rv = parse_port_localloopback(val);
	} else if(!strcmp(command_name, "RemoteLoopback")) {
		rv = parse_port_remoteloopback(val);
	} else if(!strcmp(command_name, "MagicFrameMac")) {
		rv = parse_port_magicframemac(val);
	} else if(!strcmp(command_name, "Wolstatus")) {
		rv = parse_port_wolstatus(val);
	} else if(!strcmp(command_name, "InterfaceMode")) {
		rv = parse_port_interfacemode(val);
	} else if(!strcmp(command_name, "InterfaceModeApply")) {
		rv = parse_port_interfacemodeapply(val);
	} else if(!strcmp(command_name, "Poweron")) {
		rv = parse_port_poweron(val);
	} else if(!strcmp(command_name, "Poweroff")) {
		rv = parse_port_poweroff(val);
	} else if(!strcmp(command_name, "Reset")) {
		rv = parse_port_reset(val);
	} else if(!strcmp(command_name, "FrameMaxSize")) {
		rv = parse_port_framemaxsize(val);
	}
#endif
	else if(!strcmp(command_name, "Mtu")) {
		rv = parse_port_mtu(val);
	} else if(!strcmp(command_name, "Mru")) {
		rv = parse_port_mru(val);
	} else if(!strcmp(command_name, "Mrumtu")) {
		rv = parse_port_mru_mtu(val);
	}
#ifndef IN_PORTCONTROL_MINI
	else if(!strcmp(command_name, "Srcfilter")) {
		rv = parse_port_srcfilter(val);
	} else if(!strcmp(command_name, "Interface3az")) {
		rv = parse_port_interface8023az(val);
	} else if(!strcmp(command_name, "Promiscmode")) {
		rv = parse_port_promiscmode(val);
	} else if(!strcmp(command_name, "Eeecfg")) {
		rv = parse_port_eeecfg(val);
	}else if(!strcmp(command_name, "Srcfiltercfg")) {
		rv = parse_port_srcfiltercfg(val);
	} else if(!strcmp(command_name, "SwitchPortLoopback")) {
		rv = parse_switch_port_loopback(val);
	}
#endif
#if defined (APPE)
#ifndef IN_PORTCONTROL_MINI
	else if(!strcmp(command_name, "Port8023ah")) {
		rv = parse_port8023ah(val);
	}
#endif
	else if(!strcmp(command_name, "Mtucfg")) {
		rv = parse_port_mtu_cfg(val);
	}
#endif

#if defined(HPPE)
	else if (!strcmp(command_name, "Cntcfg")) {
		rv = parse_uci_option(val, port_cntcfg,
				sizeof(port_cntcfg)/sizeof(char *));
	}
#endif

	return rv;
}
#endif

#ifdef IN_PORTVLAN
static int
parse_portvlan(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Ingress")) {
		rv = parse_portvlan_ingress(val);
	} else if(!strcmp(command_name, "Egress")) {
		rv = parse_portvlan_egress(val);
	} else if(!strcmp(command_name, "Member")) {
		rv = parse_portvlan_member(val);
	} else if(!strcmp(command_name, "ForceVid")) {
		rv = parse_portvlan_forcevid(val);
	} else if(!strcmp(command_name, "ForceMode")) {
		rv = parse_portvlan_forcemode(val);
	} else if(!strcmp(command_name, "SVlanTPID")) {
		rv = parse_portvlan_svlantpid(val);
	} else if(!strcmp(command_name, "DefaultSvid")) {
		rv = parse_portvlan_defaultsvid(val);
	} else if(!strcmp(command_name, "DefaultCvid")) {
		rv = parse_portvlan_defaultcvid(val);
	} else if (!strcmp(command_name, "GlobalQinQMode")) {
		rv = parse_portvlan_globalqinqmode(val);
	} else if (!strcmp(command_name, "PtQinQMode")) {
		rv = parse_portvlan_ptqinqmode(val);
#ifdef HPPE
	} else if (!strcmp(command_name, "InTpid")) {
		rv = parse_portvlan_intpid(val);
	} else if (!strcmp(command_name, "EgTpid")) {
		rv = parse_portvlan_egtpid(val);
	} else if (!strcmp(command_name, "IngressFilter")) {
		rv = parse_portvlan_ingressfilter(val);
	} else if (!strcmp(command_name, "DefaultVlanTag")) {
		rv = parse_portvlan_defaultvlantag(val);
	} else if (!strcmp(command_name, "TagPropagation")) {
		rv = parse_portvlan_tagpropagation(val);
	} else if (!strcmp(command_name, "TranslationMissAction")) {
		rv = parse_portvlan_translationmissaction(val);
#endif
	} else if (!strcmp(command_name, "EgMode")) {
		rv = parse_portvlan_egmode(val);
#ifdef HPPE
	} else if (!strcmp(command_name, "VsiEgMode")) {
		rv = parse_portvlan_vsiegmode(val);
	} else if (!strcmp(command_name, "VsiEgModeEn")) {
		rv = parse_portvlan_vsiegmodeen(val);
	} else if (!strcmp(command_name, "Counter")) {
		rv = parse_portvlan_counter(val);
	} else if (!strcmp(command_name, "TranslationAdv")) {
		rv = parse_uci_option(val, portvlan_translationadv,
				sizeof(portvlan_translationadv)/sizeof(char *));
#endif
#ifdef APPE
	} else if (!strcmp(command_name, "Isol")) {
		rv = parse_portvlan_isol(val);
	} else if (!strcmp(command_name, "IsolGroup")) {
		rv = parse_portvlan_isol_group(val);
#endif
	}
#ifndef IN_PORTVLAN_MINI
	else if(!strcmp(command_name, "InVlan")) {
		rv = parse_portvlan_invlan(val);
	} else if(!strcmp(command_name, "TlsMode")) {
		rv = parse_portvlan_tlsmode(val);
	} else if(!strcmp(command_name, "PriPropagation")) {
		rv = parse_portvlan_pripropagation(val);
	} else if(!strcmp(command_name, "VlanPropagation")) {
		rv = parse_portvlan_vlanpropagation(val);
	} else if(!strcmp(command_name, "Translation")) {
		rv = parse_portvlan_translation(val);
	} else if(!strcmp(command_name, "QinqMode")) {
		rv = parse_portvlan_qinqmode(val);
	} else if(!strcmp(command_name, "QinqRole")) {
		rv = parse_portvlan_qinqrole(val);
	} else if(!strcmp(command_name, "MacVlanXlt")) {
		rv = parse_portvlan_macvlanxlt(val);
	} else if(!strcmp(command_name, "Netiso")) {
		rv = parse_portvlan_netiso(val);
	} else if(!strcmp(command_name, "EgBypass")) {
		rv = parse_portvlan_egbypass(val);
#ifdef DESS
	} else if(!strcmp(command_name, "Ptvrfid")) {
		rv = parse_portvlan_ptvrfid(val);
#endif
	}
#endif

	return rv;
}
#endif

#ifdef IN_VLAN
static int
parse_vlan(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Entry")) {
		rv = parse_vlan_entry(val);
	} else if(!strcmp(command_name, "Member")) {
		rv = parse_vlan_member(val);
	}
	#ifndef IN_VLAN_MINI
	else if(!strcmp(command_name, "LearnSts")) {
		rv = parse_vlan_learnsts(val);
	}
	#endif

	return rv;
}
#endif

#ifdef IN_FDB
static int
parse_fdb(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "PortLearn")) {
		rv = parse_fdb_portlearn(val);
	}
	#ifndef IN_FDB_MINI
	else if(!strcmp(command_name, "Resventry")) {
		rv = parse_fdb_resventry(val);
	}
	#endif
	else if(!strcmp(command_name, "Entry")) {
		rv = parse_fdb_entry(val);
	}
	#ifndef IN_FDB_MINI
	else if(!strcmp(command_name, "AgeCtrl")) {
		rv = parse_fdb_agectrl(val);
	}
	#endif
	else if(!strcmp(command_name, "AgeTime")) {
		rv = parse_fdb_agetime(val);
	}
	#ifndef IN_FDB_MINI
	else if(!strcmp(command_name, "Vlansmode")) {
		rv = parse_fdb_vlansmode(val);
	} else if(!strcmp(command_name, "Ptlearnlimit")) {
		rv = parse_fdb_ptlearnlimit(val);
	} else if(!strcmp(command_name, "Ptlearnexceedcmd")) {
		rv = parse_fdb_ptlearnexceedcmd(val);
	} else if(!strcmp(command_name, "Learnlimit")) {
		rv = parse_fdb_learnlimit(val);
	} else if(!strcmp(command_name, "Learnexceedcmd")) {
		rv = parse_fdb_learnexceedcmd(val);
	} else if(!strcmp(command_name, "PtLearnstatic")) {
		rv = parse_fdb_ptlearnstatic(val);
	} else if (!strcmp(command_name, "LearnCtrl")) {
		rv = parse_fdb_learnctrl(val);
	} else if (!strcmp(command_name, "PtLearnCtrl")) {
		rv = parse_fdb_ptlearnctrl(val);
	} else if (!strcmp(command_name, "PtStationMove")) {
		rv = parse_fdb_ptstationmove(val);
	} else if (!strcmp(command_name, "PtMacLimitCtrl")) {
		rv = parse_fdb_ptmaclimitctrl(val);
	}
	#endif

	return rv;
}
#endif

#ifdef IN_RSS_HASH

static int
parse_rsshash(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Config")) {
		rv = parse_rsshash_config(val);
	}


	return rv;
}
#endif

#ifdef IN_IGMP
static int
parse_igmp(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Mode")) {
		rv = parse_igmp_mode(val);
	} else if(!strcmp(command_name, "Cmd")) {
		rv = parse_igmp_cmd(val);
	} else if(!strcmp(command_name, "PortJoin")) {
		rv = parse_igmp_portjoin(val);
	} else if(!strcmp(command_name, "PortLeave")) {
		rv = parse_igmp_portleave(val);
	} else if(!strcmp(command_name, "Rp")) {
		rv = parse_igmp_rp(val);
	} else if(!strcmp(command_name, "CreateStatus")) {
		rv = parse_igmp_createstatus(val);
	} else if(!strcmp(command_name, "Static")) {
		rv = parse_igmp_static(val);
	} else if(!strcmp(command_name, "Leaky")) {
		rv = parse_igmp_leaky(val);
	} else if(!strcmp(command_name, "Version3")) {
		rv = parse_igmp_version3(val);
	} else if(!strcmp(command_name, "Queue")) {
		rv = parse_igmp_queue(val);
	} else if(!strcmp(command_name, "Ptlearnlimit")) {
		rv = parse_igmp_ptlearnlimit(val);
	} else if(!strcmp(command_name, "Ptlearnexceedcmd")) {
		rv = parse_igmp_ptlearnexceedcmd(val);
	} else if(!strcmp(command_name, "Multi")) {
		rv = parse_igmp_multi(val);
	}

	return rv;
}
#endif

#ifdef IN_SEC
static int
parse_sec(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Mac")) {
		rv = parse_sec_mac(val);
	} else if(!strcmp(command_name, "Ip")) {
		rv = parse_sec_ip(val);
	} else if(!strcmp(command_name, "Ip4")) {
		rv = parse_sec_ip4(val);
	} else if(!strcmp(command_name, "Ip6")) {
		rv = parse_sec_ip6(val);
	} else if(!strcmp(command_name, "Tcp")) {
		rv = parse_sec_tcp(val);
	} else if(!strcmp(command_name, "Udp")) {
		rv = parse_sec_udp(val);
	} else if(!strcmp(command_name, "Icmp4")) {
		rv = parse_sec_icmp4(val);
	} else if(!strcmp(command_name, "Icmp6")) {
		rv = parse_sec_icmp6(val);
#ifdef HPPE
	} else if (!strcmp(command_name, "Expctrl")) {
		rv = parse_sec_expctrl(val);
#ifndef IN_SEC_MINI
	} else if (!strcmp(command_name, "L3parser")) {
		rv = parse_sec_l3parser(val);
#endif
	} else if (!strcmp(command_name, "L4parser")) {
		rv = parse_sec_l4parser(val);
#endif
#ifdef APPE
#ifndef IN_SEC_MINI
	} else if (!strcmp(command_name, "L2expctrl")) {
		rv = parse_sec_l2expctrl(val);
	} else if (!strcmp(command_name, "Tunnelexpctrl")) {
		rv = parse_sec_tunnelexpctrl(val);
	} else if (!strcmp(command_name, "Tunnell3parser")) {
		rv = parse_sec_tunnell3parser(val);
	} else if (!strcmp(command_name, "Tunnell4parser")) {
		rv = parse_sec_tunnell4parser(val);
	} else if (!strcmp(command_name, "Tunnelflagsparser")) {
		rv = parse_sec_tunnelflagsparser(val);
#endif
#endif
	}

	return rv;
}
#endif

#ifdef IN_MISC
static int
parse_misc(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Eapolcmd")) {
		rv = parse_misc_eapolcmd(val);
	} else if(!strcmp(command_name, "Eapolstatus")) {
		rv = parse_misc_eapolstatus(val);
	} else if(!strcmp(command_name, "CpuPort")) {
		rv = parse_misc_cpuport(val);
	} else if(!strcmp(command_name, "PtUnkUcFilter")) {
		rv = parse_misc_ptunkucfilter(val);
	} else if(!strcmp(command_name, "PtUnkMcFilter")) {
		rv = parse_misc_ptunkmcfilter(val);
	} else if(!strcmp(command_name, "PtBcFilter")) {
		rv = parse_misc_ptbcfilter(val);
	}
	#ifndef IN_MISC_MINI
	else if(!strcmp(command_name, "CpuVid")) {
		rv = parse_misc_cpuvid(val);
	}  else if(!strcmp(command_name, "FrameMaxSize")) {
		rv = parse_misc_framemaxsize(val);
	} else if(!strcmp(command_name, "AutoNeg")) {
		rv = parse_misc_autoneg(val);
	} else if(!strcmp(command_name, "PppoeCmd")) {
		rv = parse_misc_pppoecmd(val);
	} else if(!strcmp(command_name, "Pppoe")) {
		rv = parse_misc_pppoe(val);
	} else if(!strcmp(command_name, "PtDhcp")) {
		rv = parse_misc_ptdhcp(val);
	} else if(!strcmp(command_name, "Arpcmd")) {
		rv = parse_misc_arpcmd(val);
	} else if(!strcmp(command_name, "Rip")) {
		rv = parse_misc_rip(val);
	} else if(!strcmp(command_name, "Ptarpreq")) {
		rv = parse_misc_ptarpreq(val);
	} else if(!strcmp(command_name, "Ptarpack")) {
		rv = parse_misc_ptarpack(val);
	} else if(!strcmp(command_name, "Extendpppoe")) {
		rv = parse_misc_extendpppoe(val);
	} else if(!strcmp(command_name, "Pppoeid")) {
		rv = parse_misc_pppoeid(val);
	} else if(!strcmp(command_name, "RtdPppoe")) {
		rv = parse_misc_rtdpppoe(val);
	} else if(!strcmp(command_name, "GloMacAddr")) {
		rv = parse_misc_glomacaddr(val);
	} else if(!strcmp(command_name, "Framecrc")) {
		rv = parse_misc_framecrc(val);
	} else if(!strcmp(command_name, "Pppoeen")) {
		rv = parse_misc_pppoeen(val);
	}
	#endif

	return rv;
}
#endif

#ifdef IN_IP
static int
parse_ip(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Hostentry")) {
		rv = parse_ip_hostentry(val);
	}
#if !defined(IN_IP_MINI)
	else if(!strcmp(command_name, "Intfentry")) {
		rv = parse_ip_intfentry(val);
	} else if(!strcmp(command_name, "Ptarplearn")) {
		rv = parse_ip_ptarplearn(val);
	} else if(!strcmp(command_name, "Arplearn")) {
		rv = parse_ip_arplearn(val);
	} else if(!strcmp(command_name, "Ptipsrcguard")) {
		rv = parse_ip_ptipsrcguard(val);
	} else if(!strcmp(command_name, "Ptarpsrcguard")) {
		rv = parse_ip_ptarpsrcguard(val);
	} else if(!strcmp(command_name, "Routestatus")) {
		rv = parse_ip_routestatus(val);
	} else if(!strcmp(command_name, "Ipunksrc")) {
		rv = parse_ip_ipunksrc(val);
	} else if(!strcmp(command_name, "Arpunksrc")) {
		rv = parse_ip_arpunksrc(val);
	} else if(!strcmp(command_name, "IpAgetime")) {
		rv = parse_ip_agetime(val);
	} else if(!strcmp(command_name, "Wcmphashmode")) {
		rv = parse_ip_wcmphashmode(val);
	} else if(!strcmp(command_name, "Defaultflowcmd")) {
		rv = parse_ip_defaultflowcmd(val);
	} else if(!strcmp(command_name, "Defaultrtflowcmd")) {
		rv = parse_ip_defaultrtflowcmd(val);
	} else if(!strcmp(command_name, "HostRoute")) {
		rv = parse_ip_hostroute(val);
	} else if(!strcmp(command_name, "DefaultRoute")) {
		rv = parse_ip_defaultroute(val);
	} else if(!strcmp(command_name, "Vrfbaseaddr")) {
		rv = parse_ip_vrfbaseaddr(val);
	} else if(!strcmp(command_name, "Vrfbasemask")) {
		rv = parse_ip_vrfbasemask(val);
	} else if(!strcmp(command_name, "Rfsip4")) {
		rv = parse_ip_rfsip4(val);
	} else if(!strcmp(command_name, "Rfsip6")) {
		rv = parse_ip_rfsip6(val);
	}
#endif
	else if (!strcmp(command_name, "Vsiarpsg")) {
		rv = parse_ip_vsiarpsg(val);
	} else if (!strcmp(command_name, "Vsisg")) {
		rv = parse_ip_vsisg(val);
	} else if (!strcmp(command_name, "Portarpsg")) {
		rv = parse_ip_portarpsg(val);
	} else if (!strcmp(command_name, "Portsg")) {
		rv = parse_ip_portsg(val);
	} else if (!strcmp(command_name, "Pubip")) {
		rv = parse_ip_pubip(val);
	}
#if !defined(IN_IP_MINI)
	else if (!strcmp(command_name, "Networkroute")) {
		rv = parse_ip_networkroute(val);
	}
#endif
	else if (!strcmp(command_name, "Intf")) {
		rv = parse_ip_intf(val);
	} else if (!strcmp(command_name, "Vsiintf")) {
		rv = parse_ip_vsiintf(val);
	} else if (!strcmp(command_name, "Portintf")) {
		rv = parse_ip_portintf(val);
	} else if (!strcmp(command_name, "Nexthop")) {
		rv = parse_ip_nexthop(val);
	} else if (!strcmp(command_name, "Portmac")) {
		rv = parse_ip_portmac(val);
	} else if (!strcmp(command_name, "Routemiss")) {
		rv = parse_ip_routemiss(val);
	} else if (!strcmp(command_name, "Mcmode")) {
		rv = parse_ip_mcmode(val);
	} else if (!strcmp(command_name, "Globalctrl")) {
		rv = parse_ip_globalctrl(val);
	} else if (!strcmp(command_name, "Hostentry")) {
		rv = parse_ip_hostentry(val);
	}
	return rv;
}
#endif

#ifdef IN_NAT
static int
parse_nat(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Natentry")) {
		rv = parse_nat_natentry(val);
	} else if(!strcmp(command_name, "Naptentry")) {
		rv = parse_nat_naptentry(val);
	} else if(!strcmp(command_name, "Flowentry")) {
		rv = parse_nat_flowentry(val);
	} else if(!strcmp(command_name, "Flowcookie")) {
		rv = parse_nat_flowcookie(val);
	} else if(!strcmp(command_name, "Flowrfs")) {
		rv = parse_nat_flowrfs(val);
	} else if(!strcmp(command_name, "Natstatus")) {
		rv = parse_nat_natstatus(val);
	} else if(!strcmp(command_name, "Naptstatus")) {
		rv = parse_nat_naptstatus(val);
	} else if(!strcmp(command_name, "Nathash")) {
		rv = parse_nat_nathash(val);
	} else if(!strcmp(command_name, "Naptmode")) {
		rv = parse_nat_naptmode(val);
	} else if(!strcmp(command_name, "Prvbaseaddr")) {
		rv = parse_nat_prvbaseaddr(val);
	} else if(!strcmp(command_name, "Prvaddrmode")) {
		rv = parse_nat_prvaddrmode(val);
	} else if(!strcmp(command_name, "Pubaddr")) {
		rv = parse_nat_pubaddr(val);
	} else if(!strcmp(command_name, "Natunksess")) {
		rv = parse_nat_natunksess(val);
	} else if(!strcmp(command_name, "Prvbasemask")) {
		rv = parse_nat_prvbasemask(val);
	} else if(!strcmp(command_name, "Global")) {
		rv = parse_nat_global(val);
	}

	return rv;
}
#endif

#ifdef IN_STP
static int
parse_stp(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "PortState")) {
		rv = parse_stp_portstate(val);
	}

	return rv;
}
#endif

#ifdef IN_MIRROR
static int
parse_mirror(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "AnalyPt")) {
		rv = parse_mirror_analypt(val);
	} else if(!strcmp(command_name, "PtIngress")) {
		rv = parse_mirror_ptingress(val);
	} else if(!strcmp(command_name, "PtEgress")) {
		rv = parse_mirror_ptegress(val);
	} else if (!strcmp(command_name, "AnalyCfg")) {
		rv = parse_mirror_analycfg(val);
	}

	return rv;
}
#endif

#ifdef IN_LEAKY
static int
parse_leaky(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "UcMode")) {
		rv = parse_leaky_ucmode(val);
	} else if(!strcmp(command_name, "McMode")) {
		rv = parse_leaky_mcmode(val);
	} else if(!strcmp(command_name, "ArpMode")) {
		rv = parse_leaky_arpmode(val);
	} else if(!strcmp(command_name, "PtUcMode")) {
		rv = parse_leaky_ptucmode(val);
	} else if(!strcmp(command_name, "PtMcMode")) {
		rv = parse_leaky_ptmcmode(val);
	}

	return rv;
}
#endif

#ifdef IN_TRUNK
static int
parse_trunk(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Group")) {
		rv = parse_trunk_group(val);
	} else if(!strcmp(command_name, "Hashmode")) {
		rv = parse_trunk_hashmode(val);
	} else if (!strcmp(command_name, "Failover")) {
		rv = parse_trunk_failover(val);
	}

	return rv;
}
#endif

#ifdef IN_MIB
static int
parse_mib(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Status")) {
		rv = parse_mib_status(val);
	} else if(!strcmp(command_name, "CpuKeep")) {
		rv = parse_mib_cpukeep(val);
	}

	return rv;
}
#endif

#ifdef IN_ACL
static int
parse_acl(a_uint32_t dev_id, const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Rule")) {
		rv = parse_acl_rule(dev_id, val);
	} else if(!strcmp(command_name, "Udfprofile")) {
		rv = parse_acl_udfprofile(val);
	} else if(!strcmp(command_name, "Udf")) {
		rv = parse_acl_udf(val);
#ifdef APPE
	} else if(!strcmp(command_name, "UdfprofileEntry")) {
		rv = parse_acl_udfprofileentry(dev_id, val);
	} else if(!strcmp(command_name, "UdfprofileCfg")) {
		rv = parse_acl_udfprofilecfg(val);
	} else if(!strcmp(command_name, "Vpgroup")) {
		rv = parse_acl_vpgroup(val);
#endif
	}

	return rv;
}
#endif

#ifdef IN_FLOW
static int
parse_flow(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Entry")) {
		rv = parse_flow_entry(val);
	} else if (!strcmp(command_name, "Status")) {
		rv = parse_flow_status(val);
	} else if (!strcmp(command_name, "Agetime")) {
		rv = parse_flow_age(val);
	} else if (!strcmp(command_name, "Mgmt")) {
		rv = parse_flow_mgmt(val);
	} else if (!strcmp(command_name, "Host")) {
		rv = parse_uci_option(val, flow_host, sizeof(flow_host)/sizeof(char *));
	} else if (!strcmp(command_name, "Global")) {
		rv = parse_uci_option(val, flow_global, sizeof(flow_global)/sizeof(char *));
	}

	return rv;
}
#endif

#ifdef IN_BM
static int
parse_bm(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Ctrl")) {
		rv = parse_bm_ctrl(val);
	} else if (!strcmp(command_name, "Portgroupmap")) {
		rv = parse_bm_portgroupmap(val);
	} else if (!strcmp(command_name, "Groupbuff")) {
		rv = parse_bm_groupbuff(val);
	} else if (!strcmp(command_name, "Portrsvbuff")) {
		rv = parse_bm_portrsvbuff(val);
	} else if (!strcmp(command_name, "Portsthresh")) {
		rv = parse_bm_portsthresh(val);
	} else if (!strcmp(command_name, "Portdthresh")) {
		rv = parse_bm_portdthresh(val);
	}

	return rv;
}
#endif

#ifdef IN_QM

#if defined(APPE)
#if !defined(IN_QM_MINI)
static const char *enqueue_cfg[] = {
	"enqueue_type",
	"flow_pri_profile",
	"vsi",
	"phy_port",
	"dest_vport",
	"enqueue_servcode",
	"enqueue_servcode_phyport",
	"enqueue_en",
	"enqueue_vport",
};
#endif
#endif

static int
parse_qm(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Ucastqbase")) {
		rv = parse_qm_ucastqbase(val);
	} else if (!strcmp(command_name, "Ucastpriclass")) {
		rv = parse_qm_ucastpriclass(val);
#if !defined(IN_QM_MINI)
	} else if (!strcmp(command_name, "Mcastpriclass")) {
		rv = parse_qm_mcastpriclass(val);
#endif
	} else if (!strcmp(command_name, "Queue")) {
		rv = parse_qm_queue(val);
	} else if (!strcmp(command_name, "Ucasthash")) {
		rv = parse_qm_ucasthash(val);
#if !defined(IN_QM_MINI)
	} else if (!strcmp(command_name, "Ucastdflthash")) {
		rv = parse_qm_ucastdflthash(val);
	} else if (!strcmp(command_name, "Mcastcpucode")) {
		rv = parse_qm_mcastcpucode(val);
#endif
	} else if (!strcmp(command_name, "Acctrl")) {
		rv = parse_qm_acctrl(val);
	} else if (!strcmp(command_name, "Acprebuffer")) {
		rv = parse_qm_acprebuffer(val);
	} else if (!strcmp(command_name, "Acqgroup")) {
		rv = parse_qm_acqgroup(val);
	} else if (!strcmp(command_name, "Acstaticthresh")) {
		rv = parse_qm_acsthresh(val);
	} else if (!strcmp(command_name, "Acdynamicthresh")) {
		rv = parse_qm_acdthresh(val);
	} else if (!strcmp(command_name, "Acgroupbuff")) {
		rv = parse_qm_acgbuff(val);
	} else if (!strcmp(command_name, "Cntctrl")) {
		rv = parse_qm_cntctrl(val);
	} else if (!strcmp(command_name, "Cnt")) {
		rv = parse_qm_cnt(val);
	} else if (!strcmp(command_name, "Enqueue")) {
		rv = parse_qm_enqueue(val);
	} else if (!strcmp(command_name, "Srcprofile")) {
		rv = parse_qm_srcprofile(val);
	}
#if defined(APPE)
#if !defined(IN_QM_MINI)
	else if (!strcmp(command_name, "EnqueueCfg")) {
		rv = parse_uci_option(val, enqueue_cfg,
				sizeof(enqueue_cfg)/sizeof(char *));
	}
#endif
#endif
	return rv;
}
#endif

#ifdef IN_SERVCODE
static int
parse_servcode(a_uint32_t dev_id, const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Config")) {
		rv = parse_servcode_config(val);
	} else if (!strcmp(command_name, "Loopcheck")) {
		rv = parse_servcode_loopcheck(val);
#if defined(MPPE)
	} else if (!strcmp(command_name, "PortServcode")) {
		rv = parse_servcode_portservcode(val);
	} else if (!strcmp(command_name, "Athtag")) {
		rv = parse_servcode_athtag(dev_id, val);
#endif
	}

	return rv;
}
#endif

#ifdef IN_CTRLPKT
static int
parse_ctrlpkt(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "EthernetType")) {
		rv = parse_ctrlpkt_ethernettype(val);
	} else if (!strcmp(command_name, "Rfdb")) {
		rv = parse_ctrlpkt_rfdb(val);
	} else if (!strcmp(command_name, "AppProfile")) {
		rv = parse_ctrlpkt_appprofile(val);
#if defined (APPE)
	} else if (!strcmp(command_name, "Vpgroup")) {
		rv = parse_ctrlpkt_vpgroup(val);
	} else if (!strcmp(command_name, "Tunneldecap")) {
		rv = parse_ctrlpkt_tunneldecap(val);
#endif
	}

	return rv;
}
#endif

#ifdef IN_POLICER
static int
parse_policer(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Timeslot")) {
		rv = parse_policer_timeslot(val);
	} else if(!strcmp(command_name, "Fcscompensation")) {
		rv = parse_policer_fcscompensation(val);
	} else if(!strcmp(command_name, "Portentry")) {
		rv = parse_policer_portentry(val);
	} else if(!strcmp(command_name, "Aclentry")) {
		rv = parse_policer_aclentry(val);
	} else if(!strcmp(command_name, "Bypass")) {
		rv = parse_policer_bypass(val);
	}
#if defined(APPE)
#ifndef IN_POLICER_MINI
	else if(!strcmp(command_name, "Priremap")) {
		rv = parse_policer_priremap(val);
	} else if(!strcmp(command_name, "Ctrl")) {
		rv = parse_policer_ctrl(val);
	}
#endif
#endif
	return rv;
}
#endif

#ifdef IN_SHAPER
static int
parse_shaper(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Porttimeslot")) {
		rv = parse_shaper_porttimeslot(val);
	} else if(!strcmp(command_name, "Flowtimeslot")) {
		rv = parse_shaper_flowtimeslot(val);
	} else if(!strcmp(command_name, "Queuetimeslot")) {
		rv = parse_shaper_queuetimeslot(val);
	} else if(!strcmp(command_name, "Ipgcompensation")) {
		rv = parse_shaper_ipgcompensation(val);
	} else if(!strcmp(command_name, "Porttoken")) {
		rv = parse_shaper_porttoken(val);
	} else if(!strcmp(command_name, "Flowtoken")) {
		rv = parse_shaper_flowtoken(val);
	} else if(!strcmp(command_name, "Queuetoken")) {
		rv = parse_shaper_queuetoken(val);
	} else if(!strcmp(command_name, "Portshaper")) {
		rv = parse_shaper_portshaper(val);
	} else if(!strcmp(command_name, "Queueshaper")) {
		rv = parse_shaper_queueshaper(val);
	} else if(!strcmp(command_name, "Flowshaper")) {
		rv = parse_shaper_flowshaper(val);
	}
#if defined(APPE)
	else if(!strcmp(command_name, "Queueshaperctrl")) {
		rv = parse_shaper_queueshaperctrl(val);
	} else if(!strcmp(command_name, "Flowshaperctrl")) {
		rv = parse_shaper_flowshaperctrl(val);
	}
#endif
	return rv;
}
#endif

#ifdef IN_VSI
static int
parse_vsi_portbasedvsi(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_vsi_vlanbasedvsi(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "svid")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "cvid")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_vsi_learnctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "learnaction")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_vsi_stationmove(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stationmove_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "stationmove_action")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_vsi_member(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "membership")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "unknown_unicast_membership")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "unknown_multicast_membership")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "broadcast_membership")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
#if defined (APPE)
		} else if (!strcmp(ext_value_p->option_name, "vports_bitmap0")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vports_bitmap1")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vports_bitmap2")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vports_bitmap3")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vports_bitmap4")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "vports_bitmap5")) {
			val_ptr[10] = (char*)ext_value_p->option_value;
#endif
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#if defined (APPE)
static int
parse_vsi_bridgevsi(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "vsi")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_vsi_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "bridge_vsi_id")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_vsi_invalidvsi_ctrl(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dest_en")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dest_info_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "dest_info_value")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_vsi(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Portbasedvsi")) {
		rv = parse_vsi_portbasedvsi(val);
	} else if (!strcmp(command_name, "Vlanbasedvsi")) {
		rv = parse_vsi_vlanbasedvsi(val);
	} else if (!strcmp(command_name, "Learnctrl")) {
		rv = parse_vsi_learnctrl(val);
	} else if (!strcmp(command_name, "Stationmove")) {
		rv = parse_vsi_stationmove(val);
	} else if (!strcmp(command_name, "Member")) {
		rv = parse_vsi_member(val);
#if defined (APPE)
	} else if (!strcmp(command_name, "Bridgevsi")) {
		rv = parse_vsi_bridgevsi(val);
	} else if (!strcmp(command_name, "Invalidvsi_ctrl")) {
		rv = parse_vsi_invalidvsi_ctrl(val);
#endif
	}

	return rv;
}
#endif

#ifdef IN_VXLAN
static int
parse_vxlan_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vxlan_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_ver")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "udp_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_port_type")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_port")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifndef IN_VXLAN_MINI
static int
parse_vxlan_gpeprotocfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ipv4")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ipv6")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ethernet")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int
parse_vxlan(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Entry")) {
		rv = parse_vxlan_entry(val);
#ifndef IN_VXLAN_MINI
	} else if (!strcmp(command_name, "GpeProtoCfg")) {
		rv = parse_vxlan_gpeprotocfg(val);
#endif
	}
	return rv;
}
#endif

#ifdef IN_GENEVE
static int
parse_geneve_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ip_ver")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "udp_type")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_port_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_port")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_geneve(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Entry")) {
		rv = parse_geneve_entry(val);
	}
	return rv;
}
#endif

#ifdef IN_TUNNEL_PROGRAM
static int
parse_tunnelprogram_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "program_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_ver")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "outer_hdr_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ethernet_type")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ethernet_type_mask")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_protocol")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_protocol_mask")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tunnel_hdr_f32bit")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tunnel_hdr_f32bit_mask")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_dst_port")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_dst_port_mask")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_src_port")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "l4_src_port_mask")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

static int
parse_tunnelprogram_cfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "program_type")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "program_pos_mode")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "inner_type_mode")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "inner_hdr_type")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "basic_hdr_length")) {
			val_ptr[4] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "option_length_unit")) {
			val_ptr[5] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "option_length_mask")) {
			val_ptr[6] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "udf0_offset")) {
			val_ptr[7] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "udf1_offset")) {
			val_ptr[8] = (char*)ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "udf2_offset")) {
			val_ptr[9] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}
	if (val_ptr[3] == NULL) {
		val_ptr[3] = "ethernet";
		parameter_length ++;
	}

	return rv;
}

static int
parse_tunnelprogram_udf(a_uint32_t dev_id, struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	a_uint32_t tmpdata = 0;
	fal_tunnel_program_udf_t entry;
	fal_tunnel_program_type_t program_type = 0;
	memset(&entry, 0, sizeof(fal_tunnel_program_udf_t));
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "program_type")) {
			cmd_data_check_attr("tunnel_program_type",
					(char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			program_type = tmpdata;
		} else if(!strcmp(ext_value_p->option_name, "udf0")) {
			cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_val[0] = tmpdata & 0xffff;
			FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF0);
		} else if(!strcmp(ext_value_p->option_name, "udf0_mask")) {
			cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_mask[0] = tmpdata & 0xffff;
		} else if(!strcmp(ext_value_p->option_name, "udf1")) {
			cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_val[1] = tmpdata & 0xffff;
			FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF1);
		} else if(!strcmp(ext_value_p->option_name, "udf1_mask")) {
			cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_mask[1] = tmpdata & 0xffff;
		} else if(!strcmp(ext_value_p->option_name, "udf2")) {
			cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_val[2] = tmpdata & 0xffff;
			FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF2);
		} else if(!strcmp(ext_value_p->option_name, "udf2_mask")) {
			cmd_data_check_uint16((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_mask[2] = tmpdata & 0xffff;
		} else if(!strcmp(ext_value_p->option_name, "rule_inverse")) {
			if (!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_INVERSE);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")) {
				FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_CLR(entry.field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_INVERSE);
			}
		} else if(!strcmp(ext_value_p->option_name, "inner_hdr_type")) {
			cmd_data_check_attr("hdr_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.inner_hdr_type = tmpdata & 0x3;
			FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(entry.action_flag,
					FAL_TUNNEL_PROGRAM_UDF_ACTION_INNER_HDR_TYPE);
		} else if(!strcmp(ext_value_p->option_name, "udf_hdr_length")) {
			cmd_data_check_uint8((char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.udf_hdr_len = tmpdata;
			FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(entry.action_flag,
					FAL_TUNNEL_PROGRAM_UDF_ACTION_UDF_HDR_LEN);
		} else if(!strcmp(ext_value_p->option_name, "exception")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(entry.action_flag,
					FAL_TUNNEL_PROGRAM_UDF_ACTION_EXCEPTION_EN);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_CLR(entry.action_flag,
					FAL_TUNNEL_PROGRAM_UDF_ACTION_EXCEPTION_EN);
			}
		}
		switch_ext_p = switch_ext_p->next;
	}
	rv = fal_tunnel_program_udf_add(dev_id, program_type, &entry);
	SSDK_DEBUG("uci set tunnelprogram udf rv %d\n", rv);
	return rv;
}

static int
parse_tunnelprogram(a_uint32_t dev_id, const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if (!strcmp(command_name, "Entry")) {
		rv = parse_tunnelprogram_entry(val);
	} else if (!strcmp(command_name, "Cfg")) {
		rv = parse_tunnelprogram_cfg(val);
	} else if (!strcmp(command_name, "Udf")) {
		rv = parse_tunnelprogram_udf(dev_id, val);
	}
	return rv;
}
#endif

#ifdef IN_TUNNEL
int parse_tunnel_udfprofileentry(a_uint32_t dev_id, struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;
	a_uint32_t tmpdata = 0, profile_id = 0;
	fal_tunnel_udf_profile_entry_t entry;
	memset(&entry, 0, sizeof(fal_tunnel_udf_profile_entry_t));
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "profile_id")) {
			cmd_data_check_uint32((char*)ext_value_p->option_value,
					&profile_id, sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "l3_type")) {
			cmd_data_check_attr("l3_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.l3_type = tmpdata & 0x3;
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
		} else if(!strcmp(ext_value_p->option_name, "l4_type")) {
			cmd_data_check_attr("l4_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.l4_type = tmpdata & 0x7;
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
		} else if(!strcmp(ext_value_p->option_name, "overlay_type")) {
			cmd_data_check_attr("tunnel_overlay_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.overlay_type = tmpdata & 0x3;
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_OVERLAY_TYPE);
		} else if(!strcmp(ext_value_p->option_name, "program_type")) {
			cmd_data_check_attr("tunnel_program_type", (char*)ext_value_p->option_value,
					&tmpdata, sizeof(tmpdata));
			entry.program_type = tmpdata & 0x7;
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry.field_flag,
					FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_PROGRAM_TYPE);
		}
		switch_ext_p = switch_ext_p->next;
	}
	rv = fal_tunnel_udf_profile_entry_add(dev_id, profile_id, &entry);
	SSDK_DEBUG("uci set tunnel udfprofileentry rv %d\n", rv);

	return rv;
}

int parse_tunnel_udfprofilecfg(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	int rv = 0;

	switch_ext_p = val->value.ext_val;
	while (switch_ext_p) {
		ext_value_p = switch_ext_p;

		if (!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if (!strcmp(ext_value_p->option_name, "profile_id")) {
			val_ptr[0] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[1] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "user_defined_type")) {
			val_ptr[2] = (char*)ext_value_p->option_value;
		} else if (!strcmp(ext_value_p->option_name, "user_defined_offset")) {
			val_ptr[3] = (char*)ext_value_p->option_value;
		} else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}
#endif

static int name_transfer(char *name, char *module, char *cmd)
{
        char *p;
        unsigned int i = 0, len = 0;

        p = name + 1;
        len = strlen(name);
        for(i=1; i<len; i++) {
                if(*p>='A'&&*p<='Z')
			break;
		p++;
        }

        if(i<len) {
		strlcpy(module, name, i+1);
		strlcpy(cmd, p, COMMAND_NAME_MAX_LEN);
		return 0;
        }

        return -1;
}

int
qca_ar8327_sw_switch_ext(struct switch_dev *dev,
				const struct switch_attr *attr,
			 	struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *ext_value_p;
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);
	unsigned int i = 0;
	int rv = -1;
	switch_ext_p = val->value.ext_val;

	memset(whole_command_line, 0, sizeof(whole_command_line));
	memset(module_name, 0, sizeof(module_name));
	memset(command_name, 0, sizeof(command_name));
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			name_transfer((char*)ext_value_p->option_value, module_name, command_name);
			SSDK_DEBUG("module_name:%s command_name:%s\n", module_name, command_name);
			break;
		}
		switch_ext_p = switch_ext_p->next;
	}

	parameter_length = 0;

	if(!strcmp(module_name, "Qos")) {
#ifdef IN_QOS
		rv = parse_qos(command_name, val);
#endif
	} else if(!strcmp(module_name, "Cosmap")) {
#ifdef IN_COSMAP
		rv = parse_cos(command_name, val);
#endif
	} else if(!strcmp(module_name, "Rate")) {
#ifdef IN_RATE
		rv = parse_rate(command_name, val);
#endif
	} else if(!strcmp(module_name, "Port")) {
#ifdef IN_PORTCONTROL
		rv = parse_port(command_name, val);
#endif
	} else if(!strcmp(module_name, "Portvlan")) {
#ifdef IN_PORTVLAN
		rv = parse_portvlan(command_name, val);
#endif
	} else if(!strcmp(module_name, "Vlan")) {
#ifdef IN_VLAN
		rv = parse_vlan(command_name, val);
#endif
	} else if(!strcmp(module_name, "Fdb")) {
#ifdef IN_FDB
		rv = parse_fdb(command_name, val);
#endif
	} else if(!strcmp(module_name, "Rsshash")) {
#ifdef IN_RSS_HASH
		rv = parse_rsshash(command_name, val);
#endif
	} else if(!strcmp(module_name, "Igmp")) {
#ifdef IN_IGMP
		rv = parse_igmp(command_name, val);
#endif
	} else if(!strcmp(module_name, "Sec")) {
#ifdef IN_SEC
		rv = parse_sec(command_name, val);
#endif
	} else if(!strcmp(module_name, "Misc")) {
#ifdef IN_MISC
		rv = parse_misc(command_name, val);
#endif
	} else if(!strcmp(module_name, "Ip")) {
#ifdef IN_IP
		rv = parse_ip(command_name, val);
#endif
	} else if(!strcmp(module_name, "Nat")) {
#ifdef IN_NAT
		rv = parse_nat(command_name, val);
#endif
	} else if(!strcmp(module_name, "Stp")) {
#ifdef IN_STP
		rv = parse_stp(command_name, val);
#endif
	} else if(!strcmp(module_name, "Mirror")) {
#ifdef IN_MIRROR
		rv = parse_mirror(command_name, val);
#endif
	} else if(!strcmp(module_name, "Leaky")) {
#ifdef IN_LEAKY
		rv = parse_leaky(command_name, val);
#endif
	} else if(!strcmp(module_name, "Trunk")) {
#ifdef IN_TRUNK
		rv = parse_trunk(command_name, val);
#endif
	} else if(!strcmp(module_name, "Mib")) {
#ifdef IN_MIB
		rv = parse_mib(command_name, val);
#endif
	} else if(!strcmp(module_name, "Acl")) {
#ifdef IN_ACL
		rv = parse_acl(priv->device_id, command_name, val);
#endif
	} else if(!strcmp(module_name, "Flow")) {
#ifdef IN_FLOW
		rv = parse_flow(command_name, val);
#endif
	} else if(!strcmp(module_name, "Bm")) {
#ifdef IN_BM
		rv = parse_bm(command_name, val);
#endif
	} else if(!strcmp(module_name, "Qm")) {
#ifdef IN_QM
		rv = parse_qm(command_name, val);
#endif
	} else if(!strcmp(module_name, "Servcode")) {
#ifdef IN_SERVCODE
		rv = parse_servcode(priv->device_id, command_name, val);
#endif
	} else if(!strcmp(module_name, "Ctrlpkt")) {
#ifdef IN_CTRLPKT
		rv = parse_ctrlpkt(command_name, val);
#endif
	} else if(!strcmp(module_name, "Policer")) {
#ifdef IN_POLICER
		rv = parse_policer(command_name, val);
#endif
	} else if(!strcmp(module_name, "Shaper")) {
#ifdef IN_SHAPER
		rv = parse_shaper(command_name, val);
#endif
	} else if(!strcmp(module_name, "Vsi")) {
#ifdef IN_VSI
		rv = parse_vsi(command_name, val);
#endif
	} else if(!strcmp(module_name, "Vxlan")) {
#ifdef IN_VXLAN
		rv = parse_vxlan(command_name, val);
#endif
	} else if(!strcmp(module_name, "Geneve")) {
#ifdef IN_GENEVE
		rv = parse_geneve(command_name, val);
#endif
	} else if(!strcmp(module_name, "Tunnelprogram")) {
#ifdef IN_TUNNEL_PROGRAM
		rv = parse_tunnelprogram(priv->device_id, command_name, val);
#endif
	} else if(!strcmp(module_name, "Tunnel")) {
#ifdef IN_TUNNEL
		rv = parse_tunnel(priv->device_id, command_name, val);
#endif
	} else if(!strcmp(module_name, "Mapt")) {
#ifdef IN_MAPT
		rv = parse_mapt(command_name, val);
#endif
	} else if(!strcmp(module_name, "Vport")) {
#ifdef IN_VPORT
		rv = parse_vport(command_name, val);
#endif
	} else if(!strcmp(module_name, "Athtag")) {
#ifdef IN_ATHTAG
		rv = parse_athtag(command_name, val);
#endif
	}

	if(!rv) {
		strlcat(whole_command_line, module_name, sizeof(whole_command_line));
		strlcat(whole_command_line, " ", sizeof(whole_command_line));
		strlcat(whole_command_line, command_name, sizeof(whole_command_line));
		strlcat(whole_command_line, " ", sizeof(whole_command_line));
		strlcat(whole_command_line, "set", sizeof(whole_command_line));
		strlcat(whole_command_line, " ", sizeof(whole_command_line));
		for(i=0; i<parameter_length; i++) {
			if(val_ptr[i]) {
				strlcat(whole_command_line, val_ptr[i], sizeof(whole_command_line));
				strlcat(whole_command_line, " ", sizeof(whole_command_line));
			}

			val_ptr[i] = NULL;
		}

		SSDK_DEBUG("command_line:%s\n", whole_command_line);

	}
	uci_set_devid(priv->device_id);
	set_talk_mode(0);
	rv = cmd_run_one(whole_command_line);
	set_talk_mode(1);

	SSDK_DEBUG("cmd_run_one: ret=%d\r\n", rv);

	return rv;
}
