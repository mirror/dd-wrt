/*
 * Copyright 2020 Morse Micro
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
#include "mm_argtable3.h"

int version_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Get software versions");
    return 0;
}

int version(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint32_t len;
    struct morse_cmd_resp_get_version *version;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    /* Allocate space for the resp, the max version str len and a null character */
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*version) +
                    sizeof(version->version[0]) * (MORSE_CMD_MAX_VERSION_LEN) + 1);

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    version = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_get_version);

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_VERSION,
                                 cmd_tbuff, rsp_tbuff);
exit:
    if (ret >= 0)
    {
        len = le32toh(version->length);
        version->version[len] = '\0';
        mctrl_print("FW Version: %s\n", version->version);
    }

    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(version, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
