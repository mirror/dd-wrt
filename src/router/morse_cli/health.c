/*
 * Copyright 2021 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "portable_endian.h"

#include "command.h"

int health_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Check the health of the FW cores");
    return 0;
}

int health(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_HEALTH_CHECK,
                                 cmd_tbuff, rsp_tbuff);

exit:
    if (ret < 0)
        mctrl_err("health check: failed\n");
    else
        mctrl_print("health check: success\n");

    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(health, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
