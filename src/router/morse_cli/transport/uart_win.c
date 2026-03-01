/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#include "uart.h"
#include "../utilities.h"

#define TX_BUF_SIZE (512)
#define RX_BUF_SIZE (2064)

struct uart_ctx
{
    HANDLE hnd;
};

struct uart_ctx *uart_init(const struct uart_config *config)
{
    BOOL ok;
    struct uart_ctx *ctx;
    COMMTIMEOUTS comm_timeouts = {
        .ReadIntervalTimeout = 1000,
        .ReadTotalTimeoutMultiplier = 0,
        .ReadTotalTimeoutConstant = 0,
        .WriteTotalTimeoutMultiplier = 10,
        .WriteTotalTimeoutConstant = 100,
    };
    DCB dcb = {
        .DCBlength = sizeof(dcb),
        .BaudRate = config->baudrate,
        .fBinary = TRUE,
        .fParity = FALSE,
        .fOutxCtsFlow = FALSE,
        .fOutxDsrFlow = FALSE,
        .fDtrControl = DTR_CONTROL_DISABLE,
        .fDsrSensitivity = FALSE,
        .fTXContinueOnXoff = FALSE,
        .fOutX = FALSE,
        .fInX = FALSE,
        .fErrorChar = FALSE,
        .fNull = FALSE,
        .fRtsControl = RTS_CONTROL_DISABLE,
        .fAbortOnError = TRUE,
        .ByteSize = 8,
        .Parity = NOPARITY,
        .StopBits = ONESTOPBIT,
    };

    ctx = calloc(1, sizeof(*ctx));
    MCTRL_ASSERT(ctx != NULL, "Memory allocation failure");

    ctx->hnd = CreateFile(config->dev_name, GENERIC_READ | GENERIC_WRITE,
                          0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (ctx->hnd == INVALID_HANDLE_VALUE)
    {
        mctrl_err("Failed to open UART device\n");
        free(ctx);
        return NULL;
    }

    ok = SetupComm(ctx->hnd, RX_BUF_SIZE, TX_BUF_SIZE);
    if (!ok)
    {
        mctrl_err("Failed to configure UART device buffers\n");
        free(ctx);
        return NULL;
    }

    ok = SetCommTimeouts(ctx->hnd, &comm_timeouts);
    if (!ok)
    {
        mctrl_err("Failed to configure UART device timeouts\n");
        free(ctx);
        return NULL;
    }

    ok = SetCommState(ctx->hnd, &dcb);
    if (!ok)
    {
        mctrl_err("Failed to configure UART device\n");
        free(ctx);
        return NULL;
    }

    return ctx;
}

int uart_deinit(struct uart_ctx *ctx)
{
    int ret = 0;
    if (ctx != NULL)
    {
        if (ctx->hnd != INVALID_HANDLE_VALUE)
        {
            bool ok = CloseHandle(ctx->hnd);
            ret = ok ? 0 : -ETRANSERR;
        }
        free(ctx);
    }
    return ret;
}

int uart_read(struct uart_ctx *ctx, uint8_t *buf, size_t len)
{
    bool ok;
    DWORD actual_read_len;

    ok = ReadFile(ctx->hnd, buf, len, &actual_read_len, NULL);
    if (!ok)
    {
        return -ETRANSERR;
    }

    return actual_read_len;
}

int uart_write(struct uart_ctx *ctx, const uint8_t *buf, size_t len)
{
    bool ok;
    DWORD actual_write_len;

    ok = WriteFile(ctx->hnd, buf, len, &actual_write_len, NULL);
    if (!ok)
    {
        return -ETRANSERR;
    }

    return actual_write_len;
}
