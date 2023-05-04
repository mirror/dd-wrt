/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/**************************************************************************
 *
 * imap_log.c
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file handles IMAP alerts.
 *
 * Entry point functions:
 *
 *    IMAP_GenerateAlert()
 *
 *
 **************************************************************************/

#include <stdarg.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_debug.h"
#include "imap_config.h"
#include "imap_log.h"
#include "snort_imap.h"
#include "sf_dynamic_preprocessor.h"

extern IMAPConfig *imap_eval_config;
extern IMAP *imap_ssn;

char imap_event[IMAP_EVENT_MAX][EVENT_STR_LEN];


void IMAP_GenerateAlert(int event, char *format, ...)
{
    va_list ap;

    /* Only log a specific alert once per session */
    if (imap_ssn->alert_mask & (1 << event))
    {
#ifdef DEBUG_MSGS
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "Already alerted on: %s - "
                                "ignoring event.\n", imap_event[event]););
#endif
        return;
    }

    /* set bit for this alert so we don't alert on again
     * in this session */
    imap_ssn->alert_mask |= (1 << event);

    va_start(ap, format);

    imap_event[event][0] = '\0';
    vsnprintf(&imap_event[event][0], EVENT_STR_LEN - 1, format, ap);
    imap_event[event][EVENT_STR_LEN - 1] = '\0';

    _dpd.alertAdd(GENERATOR_SPP_IMAP, event, 1, 0, 3, &imap_event[event][0], 0);

    DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP Alert generated: %s\n", imap_event[event]););

    va_end(ap);
}

void IMAP_DecodeAlert(void *ds)
{
    Email_DecodeState *decode_state = (Email_DecodeState *)ds;
    switch( decode_state->decode_type )
    {
    case DECODE_B64:
        if (imap_eval_config->decode_conf.b64_depth > -1)
            IMAP_GenerateAlert(IMAP_B64_DECODING_FAILED, "%s", IMAP_B64_DECODING_FAILED_STR);
        break;
    case DECODE_QP:
        if (imap_eval_config->decode_conf.qp_depth > -1)
            IMAP_GenerateAlert(IMAP_QP_DECODING_FAILED, "%s", IMAP_QP_DECODING_FAILED_STR);
        break;
    case DECODE_UU:
        if (imap_eval_config->decode_conf.uu_depth > -1)
            IMAP_GenerateAlert(IMAP_UU_DECODING_FAILED, "%s", IMAP_UU_DECODING_FAILED_STR);
        break;

    default:
        break;
    }
}

