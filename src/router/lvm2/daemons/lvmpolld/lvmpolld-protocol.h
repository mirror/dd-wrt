/*
 * Copyright (C) 2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_LVMPOLLD_PROTOCOL_H
#define _LVM_LVMPOLLD_PROTOCOL_H

#include "daemons/lvmpolld/polling_ops.h"

#define LVMPOLLD_PROTOCOL "lvmpolld"
#define LVMPOLLD_PROTOCOL_VERSION 1

#define LVMPD_REQ_CONVERT	CONVERT_POLL
#define LVMPD_REQ_DUMP		"dump"
#define LVMPD_REQ_MERGE		MERGE_POLL
#define LVMPD_REQ_MERGE_THIN	MERGE_THIN_POLL
#define LVMPD_REQ_PROGRESS	"progress_info"
#define LVMPD_REQ_PVMOVE	PVMOVE_POLL

#define LVMPD_PARM_ABORT		"abort"
#define LVMPD_PARM_HANDLE_MISSING_PVS	"handle_missing_pvs"
#define LVMPD_PARM_INTERVAL		"interval"
#define LVMPD_PARM_LVID			"lvid"
#define LVMPD_PARM_LVNAME		"lvname"
#define LVMPD_PARM_SYSDIR		"sysdir"
#define LVMPD_PARM_VALUE		"value" /* either retcode or signal value */
#define LVMPD_PARM_VGNAME		"vgname"

#define LVMPD_RESP_FAILED	"failed"
#define LVMPD_RESP_FINISHED	"finished"
#define LVMPD_RESP_IN_PROGRESS	"in_progress"
#define LVMPD_RESP_EINVAL	"invalid"
#define LVMPD_RESP_NOT_FOUND	"not_found"
#define LVMPD_RESP_OK		"OK"

#define LVMPD_REAS_RETCODE	"retcode" /* lvm cmd ret code */
#define LVMPD_REAS_SIGNAL	"signal" /* lvm cmd terminating singal */

#define LVMPD_RET_DUP_FAILED	100
#define LVMPD_RET_EXC_FAILED	101

#endif /* _LVM_LVMPOLLD_PROTOCOL_H */
