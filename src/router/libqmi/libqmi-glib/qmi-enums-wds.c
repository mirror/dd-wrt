/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2012 Aleksander Morgado <aleksander@lanedo.com>
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "qmi-enums-wds.h"
#include "qmi-enum-types.h"

const gchar *
qmi_wds_verbose_call_end_reason_get_string (QmiWdsVerboseCallEndReasonType type,
                                            gint16 reason)
{
    switch (type) {
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_MIP:
        return qmi_wds_verbose_call_end_reason_mip_get_string ((QmiWdsVerboseCallEndReasonMip)reason);
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_INTERNAL:
        return qmi_wds_verbose_call_end_reason_internal_get_string ((QmiWdsVerboseCallEndReasonInternal)reason);
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_CM:
        return qmi_wds_verbose_call_end_reason_cm_get_string ((QmiWdsVerboseCallEndReasonCm)reason);
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_3GPP:
        return qmi_wds_verbose_call_end_reason_3gpp_get_string ((QmiWdsVerboseCallEndReason3gpp)reason);
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_PPP:
        return qmi_wds_verbose_call_end_reason_ppp_get_string ((QmiWdsVerboseCallEndReasonPpp)reason);
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_EHRPD:
        return qmi_wds_verbose_call_end_reason_ehrpd_get_string ((QmiWdsVerboseCallEndReasonEhrpd)reason);
    case QMI_WDS_VERBOSE_CALL_END_REASON_TYPE_IPV6:
        return qmi_wds_verbose_call_end_reason_ipv6_get_string ((QmiWdsVerboseCallEndReasonIpv6)reason);
    default:
        return NULL;
    }
}
