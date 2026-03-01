/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>

#include "command.h"


int coredump_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Generate a FW coredump at /var/log/mmcd_<hostname>_<ip>_<date>/)");
    return 0;
}

int coredump(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_COREDUMP,
                                 cmd_tbuff, rsp_tbuff);
exit:
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(coredump, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
