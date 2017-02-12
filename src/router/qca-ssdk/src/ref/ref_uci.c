/*
 * Copyright (c) 2013, 2015, The Linux Foundation. All rights reserved.
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
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
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
#if 0 //efined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#elif defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
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

#define MOD_NAME_MAX_LEN	32
#define COMMAND_NAME_MAX_LEN	128
#define	COMMAND_LINE_MAX_LEN	1024
#define	SWITCH_CFG_LEN_MAX	64
static char module_name[MOD_NAME_MAX_LEN] = {0};
static char command_name[COMMAND_NAME_MAX_LEN] = {0};
static char whole_command_line[COMMAND_LINE_MAX_LEN] = {0};
static char *val_ptr[SWITCH_CFG_LEN_MAX] = {0};
static unsigned int parameter_length = 0;
static char *vrf_dflt_str = "0";
static char *lb_dflt_str = "0";
static char *cookie_dflt_str = "0";
static char *priority_dflt_str = "no";
static char *param_dflt_str = " ";

#ifdef IN_QOS
#ifndef IN_QOS_MINI
static int
parse_qos_qtxbufsts(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "buffer_limit")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "number")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "buffer_limit")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "number")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "number")) {
			val_ptr[1] = ext_value_p->option_value;
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
parse_qos_ptreden(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "red_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "weight")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "stag_pri")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ctag_pri")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_stag_pri_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_ctag_pri_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "table_id")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[3] = ext_value_p->option_value;
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

#ifdef IN_COSMAP
static int
parse_cos_mappri2q(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "enhance_queue")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "dscp")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pri")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "up")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remark_dscp")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remark_up")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "remark_dei")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "green_dscp")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dscp")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "green_up")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_up")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "green_dei")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "yellow_dei")) {
			val_ptr[9] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "combine_enable")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "couple_flag")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "color_aware")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "deficit_flag")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "c_bucket_enable")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "c_rate_flag")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "c_meter_interval")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "e_bucket_enable")) {
			val_ptr[11] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[12] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[13] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "e_rate_flag")) {
			val_ptr[14] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "e_meter_interval")) {
			val_ptr[15] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[6] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[7] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "policer_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_mode")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "byte_based")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "couple_flag")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "color_aware")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "deficit_flag")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cir")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cbs")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "eir")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ebs")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "meter_interval")) {
			val_ptr[10] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "add_rate_bytes")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "golbal_flow_control_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "duplex")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "speed")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "auto_adv")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tx_frame_atheros_header_tag_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rx_frame_atheros_header_tag_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "atheros_header_tag_status")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "atheros_header_tag_type")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_control_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_control_force_mode_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "power_save_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "hibernate_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tx_mac_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rx_mac_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tx_flow_control_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rx_flow_control_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "back_presure_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "link_force_mode_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mac_loopback_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ring_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "on_thresh")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "off_thresh")) {
			val_ptr[2] = ext_value_p->option_value;
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
parse_port_ieee8023az(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
parse_port_poweron(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
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

#ifdef IN_PORTVLAN
static int
parse_portvlan_ingress(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ingress_vlan_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "egress_vlan_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "member")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_vid_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "force_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "stag_tpid")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "default_stag_vid")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "default_ctag_vid")) {
			val_ptr[1] = ext_value_p->option_value;
		}  else {
			rv = -1;
			break;
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}

	return rv;
}

#ifndef IN_PORTVLAN_MINI
static int
parse_portvlan_invlan(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ingress_tag_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tls_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_priority_propagation_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_propagation_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "original_vid")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "bi_direction")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "forward_direction")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "reverse_direction")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "svid")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cvid")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "original_vid_is_cvid")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "svid_enable")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cvid_enable")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "one_2_one_vlan")) {
			val_ptr[10] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "qinq_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "qinq_role")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "egress_mac_based_vlan")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "net_isolate")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "egress_translation_filter_bypass")) {
			val_ptr[0] = ext_value_p->option_value;
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
parse_portvlan_ptvrfid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[1] = ext_value_p->option_value;
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

#ifdef IN_VLAN
static int
parse_vlan_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "tag_mode")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
#ifndef IN_FDB_MINI
static int
parse_fdb_entry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "addr")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "fid")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dacmd")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "sacmd")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dest_port")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "static")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "leaky")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "clone")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_override")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cross_pt_state")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "white_list_en")) {
			val_ptr[11] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance_en")) {
			val_ptr[12] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[13] = ext_value_p->option_value;
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

static int
parse_fdb_resventry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "addr")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "fid")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dacmd")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "sacmd")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dest_port")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "static")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "leaky")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "clone")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_override")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "cross_pt_state")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "white_list_en")) {
			val_ptr[11] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "aging_status")) {
			val_ptr[0] = ext_value_p->option_value;
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
parse_fdb_agetime(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "aging_time")) {
			val_ptr[0] = ext_value_p->option_value;
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
parse_fdb_vlansmode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vlan_searching_mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_status")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_counter")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_exceed_cmd")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_status")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_counter")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "learn_exceed_cmd")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_static_status")) {
			val_ptr[1] = ext_value_p->option_value;
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

#ifdef IN_IGMP
static int
parse_igmp_mode(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "igmp_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "igmp_command")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "join_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "leave_status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "route_port_bitmap")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "create_status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "static_status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "leaky_status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "version3_status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "queue_id")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_status")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_limit_counter")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "learn_exceed_cmd")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "group_type")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "group_ip_addr")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_type")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_ip_addr")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "portmap")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlanid")) {
			val_ptr[5] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "item")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "value")) {
			val_ptr[1] = ext_value_p->option_value;
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

#ifdef IN_MISC
#ifndef IN_MISC_MINI
static int
parse_misc_framemaxsize(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "frame_max_size")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "session_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "multicast_session")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "unicast_seesion")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[4] = ext_value_p->option_value;
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
parse_misc_pppoeid(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pppoe_id")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "macaddr")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	char* vrf_temp = "0";
	val_ptr[6] = vrf_temp;
	parameter_length ++;
	val_ptr[7] = vrf_temp;
	parameter_length ++;
	val_ptr[12] = param_dflt_str;
	parameter_length ++;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_addr")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "interface_id")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance_num")) {
			val_ptr[6] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[7] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[11] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[12] = ext_value_p->option_value;
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
parse_ip_intfentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
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
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[1] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "vlan_low")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_high")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ipv4_route")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ipv6_route")) {
			val_ptr[6] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_guard_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "source_guard_mode")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "age_time")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "wcmp_hash_mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_type")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_cmd")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_type")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_cmd")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_valid")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_version")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip_addr")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "prefix_length")) {
			val_ptr[5] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_valid")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "route_type")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "index")) {
			val_ptr[4] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "base_addr")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "base_mask")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip4_addr")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[3] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "mac_addr")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "ip6_addr")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[3] = ext_value_p->option_value;
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

#ifdef IN_NAT
static int
parse_nat_natentry(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
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
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "select_index")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[4] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "translate_addr")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_number")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_range")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[11] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[12] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
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
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[3] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "flow_cookie")) {
			val_ptr[4] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[5] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "translate_addr")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "translate_port")) {
			val_ptr[11] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[12] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[13] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[14] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[15] = ext_value_p->option_value;
			parameter_length --;
		}  else if(!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[16] = ext_value_p->option_value;
			parameter_length --;
		}  else if(!strcmp(ext_value_p->option_name, "priority_val")) {
			val_ptr[17] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
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
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_flags")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "entry_status")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "vrf_id")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "flow_cookie")) {
			val_ptr[4] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "load_balance")) {
			val_ptr[5] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[6] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[7] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[8] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[9] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[10] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			val_ptr[11] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter")) {
			val_ptr[12] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "counter_id")) {
			val_ptr[13] = ext_value_p->option_value;
			parameter_length --;
		} else if(!strcmp(ext_value_p->option_name, "priority")) {
			val_ptr[14] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "priority_val")) {
			val_ptr[15] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "proto")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[3] = ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[4] = ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "flow_cookie")) {
			val_ptr[5] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "proto")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_addr")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_addr")) {
			val_ptr[3] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "src_port")) {
			val_ptr[4] = ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "dst_port")) {
			val_ptr[5] = ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "flow_rfs")) {
			val_ptr[6] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "hash_flag")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "napt_mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "base_addr")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "entry_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "pub_addr")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "action")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "base_addr_mask")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	char *sync_str = "disable";
	val_ptr[1] = sync_str;
	parameter_length++;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
		}  else if(!strcmp(ext_value_p->option_name, "sync")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "stp_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "port_id")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "stp_status")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "analyst_port")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "ingress_port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "egress_port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "unicast_leaky_mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "multicast_leaky_mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "trunk_id")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "trunk_port_bitmap")) {
			val_ptr[2] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "trunk_hash_mode")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "status")) {
			val_ptr[0] = ext_value_p->option_value;
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
static int
parse_acl_rule(struct switch_val *val)
{
	a_uint32_t prio = 0;
	a_uint32_t i;
	a_uint32_t portmap = 0;
	a_uint32_t rule_id = 0;
	fal_acl_rule_t  rule;
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	memset(&rule, 0, sizeof(fal_acl_rule_t));
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "rule_id")) {
			cmd_data_check_uint32(ext_value_p->option_value,
						&rule_id, sizeof(a_uint32_t));
			if(rule_id == 0) {
				printk("ACL rule ID should begin with 1. Please Notice!\r\n");
			}
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "priority")) {
			cmd_data_check_uint32(ext_value_p->option_value,
						&prio, sizeof(a_uint32_t));
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "rule_type")) {
			cmd_data_check_ruletype(ext_value_p->option_value,
						&(rule.rule_type), sizeof(a_uint32_t));
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "dst_mac_address")) {
			cmd_data_check_macaddr(ext_value_p->option_value,
						&(rule.dest_mac_val), sizeof(fal_mac_addr_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_DA);
		} else if(!strcmp(ext_value_p->option_name, "dst_mac_address_mask")) {
			cmd_data_check_macaddr(ext_value_p->option_value,
						&(rule.dest_mac_mask), sizeof(fal_mac_addr_t));
		} else if(!strcmp(ext_value_p->option_name, "src_mac_address")) {
			cmd_data_check_macaddr(ext_value_p->option_value,
						&(rule.src_mac_val), sizeof(fal_mac_addr_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_SA);
		} else if(!strcmp(ext_value_p->option_name, "src_mac_address_mask")) {
			cmd_data_check_macaddr(ext_value_p->option_value,
						&(rule.src_mac_mask), sizeof(fal_mac_addr_t));
		} else if(!strcmp(ext_value_p->option_name, "ethernet_type")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ethtype_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_ETHTYPE);
		} else if(!strcmp(ext_value_p->option_name, "ethernet_type_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ethtype_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "vlan_id")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.vid_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_VID);
		} else if(!strcmp(ext_value_p->option_name, "vlan_id_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.vid_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "vlan_priority")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.up_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_UP);
		} else if(!strcmp(ext_value_p->option_name, "vlan_priority_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.up_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "tagged")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.tagged_val), sizeof(a_uint32_t));
			rule.tagged_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_TAGGED);
		} else if(!strcmp(ext_value_p->option_name, "cfi")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.cfi_val), sizeof(a_uint32_t));
			rule.cfi_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_CFI);
		} else if(!strcmp(ext_value_p->option_name, "ctag_vlan_id")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ctag_vid_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_CTAG_VID);
		} else if(!strcmp(ext_value_p->option_name, "ctag_vlan_id_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ctag_vid_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ctag_vlan_priority")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ctag_pri_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_CTAG_PRI);
		} else if(!strcmp(ext_value_p->option_name, "ctag_vlan_priority_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ctag_pri_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ctagged")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ctagged_val), sizeof(a_uint32_t));
			rule.ctagged_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_CTAGGED);
		} else if(!strcmp(ext_value_p->option_name, "ctag_cfi")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ctag_cfi_val), sizeof(a_uint32_t));
			rule.ctag_cfi_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_CTAG_CFI);
		} else if(!strcmp(ext_value_p->option_name, "stag_vlan_id")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.stag_vid_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_STAG_VID);
		} else if(!strcmp(ext_value_p->option_name, "stag_vlan_id_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.stag_vid_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "stag_vlan_priority")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.stag_pri_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_STAG_PRI);
		} else if(!strcmp(ext_value_p->option_name, "stag_vlan_priority_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.stag_pri_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "stagged")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.stagged_val), sizeof(a_uint32_t));
			rule.stagged_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_STAGGED);
		} else if(!strcmp(ext_value_p->option_name, "stag_dei")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.stag_dei_val), sizeof(a_uint32_t));
			rule.stag_dei_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_MAC_STAG_DEI);
		} else if(!strcmp(ext_value_p->option_name, "tagged_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.tagged_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ipv4_src_address")) {
			cmd_data_check_ip4addr(ext_value_p->option_value,
						&(rule.src_ip4_val), 4);
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP4_SIP);
		} else if(!strcmp(ext_value_p->option_name, "ipv4_src_address_mask")) {
			cmd_data_check_ip4addr(ext_value_p->option_value,
						&(rule.src_ip4_mask), 4);
		} else if(!strcmp(ext_value_p->option_name, "ipv4_dst_address")) {
			cmd_data_check_ip4addr(ext_value_p->option_value,
						&(rule.dest_ip4_val), 4);
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP4_DIP);
		} else if(!strcmp(ext_value_p->option_name, "ipv4_dst_address_mask")) {
			cmd_data_check_ip4addr(ext_value_p->option_value,
						&(rule.dest_ip4_mask), 4);
		} else if(!strcmp(ext_value_p->option_name, "ipv6_src_address")) {
			cmd_data_check_ip6addr(ext_value_p->option_value,
						&(rule.src_ip6_val), 16);
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP6_SIP);
		} else if(!strcmp(ext_value_p->option_name, "ipv6_src_address_mask")) {
			cmd_data_check_ip6addr(ext_value_p->option_value,
						&(rule.src_ip6_mask), 16);
		} else if(!strcmp(ext_value_p->option_name, "ipv6_dst_address")) {
			cmd_data_check_ip6addr(ext_value_p->option_value,
						&(rule.dest_ip6_val), 16);
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP4_DIP);
		} else if(!strcmp(ext_value_p->option_name, "ipv6_dst_address_mask")) {
			cmd_data_check_ip6addr(ext_value_p->option_value,
						&(rule.dest_ip6_mask), 16);
		} else if(!strcmp(ext_value_p->option_name, "ipv6_flow_label")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ip6_lable_val), 16);
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP6_LABEL);
		} else if(!strcmp(ext_value_p->option_name, "ipv6_flow_label_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ip6_lable_mask), 16);
		} else if(!strcmp(ext_value_p->option_name, "ip_protocol")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ip_proto_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP_PROTO);
		} else if(!strcmp(ext_value_p->option_name, "ip_protocol_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ip_proto_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ip_dscp")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ip_dscp_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_IP_DSCP);
		} else if(!strcmp(ext_value_p->option_name, "ip_dscp_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ip_dscp_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ip_dst_port")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.dest_l4port_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_L4_DPORT);
			rule.dest_l4port_op = FAL_ACL_FIELD_MASK;
		} else if(!strcmp(ext_value_p->option_name, "ip_dst_port_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.dest_l4port_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ip_src_port")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.src_l4port_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_L4_SPORT);
			rule.src_l4port_op = FAL_ACL_FIELD_MASK;
		} else if(!strcmp(ext_value_p->option_name, "ip_src_port_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.src_l4port_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "icmp_type")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.icmp_type_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_ICMP_TYPE);
		} else if(!strcmp(ext_value_p->option_name, "icmp_type_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.icmp_type_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "icmp_code")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.icmp_code_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_ICMP_CODE);
		} else if(!strcmp(ext_value_p->option_name, "icmp_code_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.icmp_code_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "tcp_flag")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.tcp_flag_val), sizeof(a_uint32_t));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_TCP_FLAG);
		} else if(!strcmp(ext_value_p->option_name, "tcp_flag_mask")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.tcp_flag_mask), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "ripv1")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.ripv1_val), sizeof(a_uint32_t));
			rule.ripv1_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_RIPV1);
		} else if(!strcmp(ext_value_p->option_name, "dhcpv4")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.dhcpv4_val), sizeof(a_uint32_t));
			rule.dhcpv4_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_DHCPV4);
		} else if(!strcmp(ext_value_p->option_name, "dhcpv6")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.dhcpv6_val), sizeof(a_uint32_t));
			rule.dhcpv6_mask = 1;
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_DHCPV6);
		} else if(!strcmp(ext_value_p->option_name, "inverse_check_fields")) {
			if (!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_INVERSE_ALL);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")) {
				FAL_FIELD_FLG_CLR(rule.field_flg,
					FAL_ACL_FIELD_INVERSE_ALL);
			}
		} else if(!strcmp(ext_value_p->option_name, "packet_drop")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_DENY);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_DENY);
			}
		} else if(!strcmp(ext_value_p->option_name, "dscp_of_remark")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.dscp), sizeof(a_uint32_t));
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_DSCP);
		} else if(!strcmp(ext_value_p->option_name, "vlan_priority_of_remark")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.up), sizeof(a_uint32_t));
		} else if(!strcmp(ext_value_p->option_name, "queue_of_remark")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.queue), sizeof(a_uint32_t));
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_QUEUE);
		} else if(!strcmp(ext_value_p->option_name, "port_bitmap")) {
			cmd_data_check_pbmp(ext_value_p->option_value, &portmap, 4);
		} else if(!strcmp(ext_value_p->option_name, "user_defined_field_value")) {
			cmd_data_check_udf_element(ext_value_p->option_value,
						&(rule.udf_val[0]), &(rule.udf_len));
			FAL_FIELD_FLG_SET(rule.field_flg,
					FAL_ACL_FIELD_UDF);
		} else if(!strcmp(ext_value_p->option_name, "user_defined_field_mask")) {
			cmd_data_check_udf_element(ext_value_p->option_value,
						&(rule.udf_mask[0]), &(rule.udf_len));
		} else if(!strcmp(ext_value_p->option_name, "redirect_to_cpu")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_RDTCPU);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_RDTCPU);
			}
		} else if(!strcmp(ext_value_p->option_name, "copy_to_cpu")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_CPYCPU);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_CPYCPU);
			}
		} else if(!strcmp(ext_value_p->option_name, "redirect_to_ports")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REDPT);
			cmd_data_check_pbmp(ext_value_p->option_value, &rule.ports, 4);
		} else if(!strcmp(ext_value_p->option_name, "mirror")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_MIRROR);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_MIRROR);
			}
		} else if(!strcmp(ext_value_p->option_name, "remark_lookup_vid")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_LOOKUP_VID);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_REMARK_LOOKUP_VID);
			}
		} else if(!strcmp(ext_value_p->option_name, "stag_vid_of_remark")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_STAG_VID);
			cmd_data_check_uint16(ext_value_p->option_value, &rule.stag_vid, 4);
		} else if(!strcmp(ext_value_p->option_name, "stag_priority_of_remark")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_STAG_PRI);
			cmd_data_check_uint16(ext_value_p->option_value, &rule.stag_pri, 4);
		} else if(!strcmp(ext_value_p->option_name, "stag_dei_of_remark")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_STAG_DEI);
			cmd_data_check_uint16(ext_value_p->option_value, &rule.stag_dei, 4);
		} else if(!strcmp(ext_value_p->option_name, "ctag_vid_of_remark")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_CTAG_VID);
			cmd_data_check_uint16(ext_value_p->option_value, &rule.ctag_vid, 4);
		} else if(!strcmp(ext_value_p->option_name, "ctag_priority_of_remark")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_CTAG_PRI);
			cmd_data_check_uint16(ext_value_p->option_value, &rule.ctag_pri, 4);
		} else if(!strcmp(ext_value_p->option_name, "ctag_cfi_of_remark")) {
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_REMARK_CTAG_CFI);
			cmd_data_check_uint16(ext_value_p->option_value, &rule.ctag_cfi, 4);
		} else if(!strcmp(ext_value_p->option_name, "action_policer_id")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.policer_ptr), sizeof(a_uint32_t));
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_POLICER_EN);
		} else if(!strcmp(ext_value_p->option_name, "action_arp_ptr")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.arp_ptr), sizeof(a_uint32_t));
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_ARP_EN);
		} else if(!strcmp(ext_value_p->option_name, "action_wcmp_ptr")) {
			cmd_data_check_uint16(ext_value_p->option_value,
						&(rule.wcmp_ptr), sizeof(a_uint32_t));
			FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_WCMP_EN);
		} else if(!strcmp(ext_value_p->option_name, "action_snat")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_POLICY_FORWARD_EN);
				rule.policy_fwd |= FAL_ACL_POLICY_SNAT;
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				rule.policy_fwd &= ~ FAL_ACL_POLICY_SNAT;
			}
		} else if(!strcmp(ext_value_p->option_name, "action_dnat")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_POLICY_FORWARD_EN);
				rule.policy_fwd |= FAL_ACL_POLICY_DNAT;
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				rule.policy_fwd &= ~ FAL_ACL_POLICY_DNAT;
			}
		} else if(!strcmp(ext_value_p->option_name, "bypass_egress_translation")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_BYPASS_EGRESS_TRANS);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_BYPASS_EGRESS_TRANS);
			}
		} else if(!strcmp(ext_value_p->option_name, "interrupt_trigger")) {
			if(!strcmp(ext_value_p->option_value, "y") ||
				!strcmp(ext_value_p->option_value, "yes")) {
				FAL_ACTION_FLG_SET(rule.action_flg,
					FAL_ACL_ACTION_MATCH_TRIGGER_INTR);
			} else if(!strcmp(ext_value_p->option_value, "n") ||
				!strcmp(ext_value_p->option_value, "no")){
				FAL_ACTION_FLG_CLR(rule.action_flg,
					FAL_ACL_ACTION_MATCH_TRIGGER_INTR);
			}
		}

		parameter_length++;
		switch_ext_p = switch_ext_p->next;
	}
	fal_acl_list_creat(0, rule_id, prio);
	fal_acl_rule_add(0, rule_id, 0, 1, &rule);
	for (i = 0; i < AR8327_NUM_PORTS; i ++) {
		fal_acl_list_unbind(0, rule_id, 0, 0, i);
		if (portmap & (0x1 << i)) {
			fal_acl_list_bind(0, rule_id, 0, 0, i);
		}
	}
	fal_acl_status_set(0, A_TRUE);

	return rv;
}

static int
parse_acl_udfprofile(struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	int rv = 0;
	switch_ext_p = val->value.ext_val;
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;

		if(!strcmp(ext_value_p->option_name, "name")) {
			switch_ext_p = switch_ext_p->next;
			continue;
		} else if(!strcmp(ext_value_p->option_name, "port")) {
			val_ptr[0] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "user_defined_type")) {
			val_ptr[1] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "user_defined_offset")) {
			val_ptr[2] = ext_value_p->option_value;
		} else if(!strcmp(ext_value_p->option_name, "user_defined_length")) {
			val_ptr[3] = ext_value_p->option_value;
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
	} else if(!strcmp(command_name, "Poweron")) {
		rv = parse_port_poweron(val);
	} else if(!strcmp(command_name, "Poweroff")) {
		rv = parse_port_poweroff(val);
	} else if(!strcmp(command_name, "Reset")) {
		rv = parse_port_reset(val);
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
	} else if(!strcmp(command_name, "Ptvrfid")) {
		rv = parse_portvlan_ptvrfid(val);
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
	} else if(!strcmp(command_name, "Entry")) {
		rv = parse_fdb_entry(val);
	} else if(!strcmp(command_name, "AgeCtrl")) {
		rv = parse_fdb_agectrl(val);
	} else if(!strcmp(command_name, "AgeTime")) {
		rv = parse_fdb_agetime(val);
	} else if(!strcmp(command_name, "Vlansmode")) {
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
	}
	#endif

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
	} else if(!strcmp(command_name, "Intfentry")) {
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
parse_acl(const char *command_name, struct switch_val *val)
{
	int rv = -1;
	if(!strcmp(command_name, "Rule")) {
		rv = parse_acl_rule(val);
	} else if(!strcmp(command_name, "Udfprofile")) {
		rv = parse_acl_udfprofile(val);
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
		strncpy(module, name, i);
		strncpy(cmd, p, len-i);
		return 0;
        }

        return -1;
}

int
qca_ar8327_sw_switch_ext(struct switch_dev *dev,
				const struct switch_attr *attr,
			 	struct switch_val *val)
{
	struct switch_ext *switch_ext_p, *switch_ext_tmp, *ext_value_p;
	unsigned int i = 0;
	int rv = -1;
	switch_ext_p = val->value.ext_val;

	memset(whole_command_line, 0, sizeof(whole_command_line));
	memset(module_name, 0, sizeof(module_name));
	memset(command_name, 0, sizeof(command_name));
	while(switch_ext_p) {
		ext_value_p = switch_ext_p;
		if(!strcmp(ext_value_p->option_name, "name")) {
			name_transfer(ext_value_p->option_value, module_name, command_name);
#ifdef DEBUG
			printk("module_name:%s command_name:%s\n", module_name, command_name);
#endif
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
		rv = parse_acl(command_name, val);
#endif
	}

	if(!rv) {
		strcat(whole_command_line, module_name);
		strcat(whole_command_line, " ");
		strcat(whole_command_line, command_name);
		strcat(whole_command_line, " ");
		strcat(whole_command_line, "set");
		strcat(whole_command_line, " ");
		for(i=0; i<parameter_length; i++) {
			if(val_ptr[i]) {
				strcat(whole_command_line, val_ptr[i]);
				strcat(whole_command_line, " ");
			}

			val_ptr[i] = NULL;
		}
#ifdef DEBUG
		printk("command_line:%s\n", whole_command_line);
#endif
	}
	uci_set_devid(qca_devname_2_devid(dev->devname));
	set_talk_mode(0);
	rv = cmd_run_one(whole_command_line);
	set_talk_mode(1);

#ifdef DEBUG
	printk("cmd_run_one: ret=%d\r\n", rv);
#endif

	return rv;
}
