/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

/*
 * UART platform abstration API.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "../utilities.h"

struct uart_ctx;

struct uart_config
{
    char dev_name[DEVICE_NAME_LEN];

    int baudrate;
};

struct uart_ctx *uart_init(const struct uart_config *cfg);

int uart_deinit(struct uart_ctx *ctx);

int uart_read(struct uart_ctx *ctx, uint8_t *buf, size_t len);

int uart_write(struct uart_ctx *ctx, const uint8_t *buf, size_t len);
