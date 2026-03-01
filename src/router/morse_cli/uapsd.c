/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#ifndef MORSE_WIN_BUILD
#include <net/if.h>
#endif
#include "portable_endian.h"

#include "command.h"
#include "utilities.h"

#define AUTO_TRIGGER_DISABLED           ((uint8_t)0)
#define AUTO_TRIGGER_ENABLED            ((uint8_t)1)
#define AUTO_TRIGGER_FLAG_DEFAULT       ((uint8_t)0xFF)
#define AUTO_TRIGGER_TIMEOUT_MIN        (100U)
#define AUTO_TRIGGER_TIMEOUT_MAX        (10000U)
#define AUTO_TRIGGER_TIMEOUT_DEFAULT    (0U)

static struct
{
    struct arg_rex *enable;
    struct arg_int *timeout;
} args;

int uapsd_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "U-APSD auto trigger frame control",
        args.enable = arg_rex1("a", NULL, MM_ARGTABLE_ENABLE_REGEX, MM_ARGTABLE_ENABLE_DATATYPE, 0,
            "Enable/disable auto trigger frame"),
        args.timeout = arg_rint0("t", "timeout", "<duration>",
            AUTO_TRIGGER_TIMEOUT_MIN, AUTO_TRIGGER_TIMEOUT_MAX,
            "Timeout at which a trigger frame is sent when enabled (ms)"));
    return 0;
}

int uapsd(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint8_t is_auto_trigger_enabled = AUTO_TRIGGER_FLAG_DEFAULT;
    struct morse_cmd_req_uapsd_config *req;
    struct morse_cmd_resp_uapsd_config *rsp;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_uapsd_config);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_uapsd_config);

    memset(req, 0, sizeof(*req));

    if (expression_to_int(args.enable->sval[0]))
    {
        is_auto_trigger_enabled = AUTO_TRIGGER_ENABLED;
    }
    else
    {
        is_auto_trigger_enabled = AUTO_TRIGGER_DISABLED;
    }

    if ((is_auto_trigger_enabled && args.timeout->count == 0) ||
        (!is_auto_trigger_enabled && args.timeout->count == 1))
    {
        mctrl_err("Invalid argument combination, -t required only if enabling auto trigger\n");
        goto exit;
    }

    req->auto_trigger_enabled = is_auto_trigger_enabled;

    if (args.timeout->count)
    {
        req->auto_trigger_timeout = htole32(args.timeout->ival[0]);
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_UAPSD_CONFIG,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(uapsd, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
