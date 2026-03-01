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
#include "morsectrl.h"
#include "portable_endian.h"

#include "command.h"

int hw_version_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Get the hardware version");
    return 0;
}

int hw_version(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_resp_get_hw_version *hw_version;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*hw_version));

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    hw_version = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_get_hw_version);

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_HW_VERSION,
                                 req_tbuff, rsp_tbuff);
exit:
    if (ret >= 0)
    {
        mctrl_print("HW Version: %s\n", hw_version->hw_version);
    }

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(hw_version, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
