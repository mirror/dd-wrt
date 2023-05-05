/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 * smtp_log.c
 *
 * Author: Andy Mullican
 *
 * Description:
 *
 * This file handles SMTP alerts.
 *
 * Entry point functions:
 *
 *    SMTP_GenerateAlert()
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
#include "smtp_config.h"
#include "smtp_log.h"
#include "snort_smtp.h"
#include "sf_dynamic_preprocessor.h"

extern SMTPConfig *smtp_eval_config;
extern SMTP *smtp_ssn;

char smtp_event[SMTP_EVENT_MAX][EVENT_STR_LEN];


void SMTP_GenerateAlert(int event, char *format, ...)
{
    va_list ap;

    /* Only log a specific alert once per session */
    if (smtp_ssn->alert_mask & (1 << event))
    {
#ifdef DEBUG_MSGS
        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Already alerted on: %s - "
                                "ignoring event.\n", smtp_event[event]););
#endif
        return;
    }

    /* set bit for this alert so we don't alert on again
     * in this session */
    smtp_ssn->alert_mask |= (1 << event);

    if (smtp_eval_config->no_alerts)
    {
#ifdef DEBUG_MSGS
        va_start(ap, format);

        smtp_event[event][0] = '\0';
        vsnprintf(&smtp_event[event][0], EVENT_STR_LEN - 1, format, ap);
        smtp_event[event][EVENT_STR_LEN - 1] = '\0';

        DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "Ignoring alert: %s\n", smtp_event[event]););

        va_end(ap);
#endif

        return;
    }

    va_start(ap, format);

    smtp_event[event][0] = '\0';
    vsnprintf(&smtp_event[event][0], EVENT_STR_LEN - 1, format, ap);
    smtp_event[event][EVENT_STR_LEN - 1] = '\0';

    _dpd.alertAdd(GENERATOR_SMTP, event, 1, 0, 3, &smtp_event[event][0], 0);

    DEBUG_WRAP(DebugMessage(DEBUG_SMTP, "SMTP Alert generated: %s\n", smtp_event[event]););

    va_end(ap);
}

void SMTP_DecodeAlert(void *ds)
{
    Email_DecodeState *decode_state = (Email_DecodeState *)ds;
    switch( decode_state->decode_type )
    {
    case DECODE_B64:
        if (smtp_eval_config->decode_conf.b64_depth > -1)
            SMTP_GenerateAlert(SMTP_B64_DECODING_FAILED, "%s", SMTP_B64_DECODING_FAILED_STR);
        break;
    case DECODE_QP:
        if (smtp_eval_config->decode_conf.qp_depth > -1)
            SMTP_GenerateAlert(SMTP_QP_DECODING_FAILED, "%s", SMTP_QP_DECODING_FAILED_STR);
        break;
    case DECODE_UU:
        if (smtp_eval_config->decode_conf.uu_depth > -1)
            SMTP_GenerateAlert(SMTP_UU_DECODING_FAILED, "%s", SMTP_UU_DECODING_FAILED_STR);
        break;

    default:
        break;
    }
}

