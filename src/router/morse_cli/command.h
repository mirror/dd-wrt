/*
 * Copyright 2020 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef MORSE_WIN_BUILD
#include <winsock2.h>
#include <windows.h>
#endif

#include "portable_endian.h"
#include "morsectrl.h"
#include "morse_commands.h"

/** The maximum size of a confirm packet. */
#define MORSE_CMD_CFM_LEN 1584

#define PACKED __attribute__((packed))

struct PACKED request
{
    /** The request command starts with a header */
    struct morse_cmd_header hdr;
    /** An opaque data pointer */
    uint8_t data[0];
};

struct PACKED response
{
    /** The confirm header */
    struct morse_cmd_header hdr;
    /** The status of the of the command. @see morse_error_t */
    uint32_t status;
    /** An opaque data pointer */
    uint8_t data[0];
};

/**
 * Error numbers the FW may return from commands
 * Defined here as errno numbers are not portable across architectures
 */
enum morse_cmd_return_code {
    MORSE_RET_SUCCESS     = 0,
    MORSE_RET_EPERM       = -1,
    MORSE_RET_ENXIO       = -6,
    MORSE_RET_ENOMEM      = -12,
    MORSE_RET_EINVAL      = -22,
    MORSE_RET_SET_INVALID_CHAN_CONFIG = -(0x7FF3)
};

int morsectrl_send_command(struct morsectrl_transport *transport,
                           int message_id,
                           struct morsectrl_transport_buff *req,
                           struct morsectrl_transport_buff *resp);
