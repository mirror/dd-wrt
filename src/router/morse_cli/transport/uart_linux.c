/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "uart.h"
#include "../utilities.h"

struct uart_ctx
{
    int fd;
};

struct uart_ctx *uart_init(const struct uart_config *config)
{
    int ret;
    struct termios tty = {};
    struct uart_ctx *ctx = calloc(1, sizeof(*ctx));
    speed_t uart_baud;
    MCTRL_ASSERT(ctx != NULL, "Memory allocation failure");

    switch (config->baudrate)
    {
        case 9600:
            uart_baud = B9600;
            break;
        case 19200:
            uart_baud = B19200;
            break;
        case 38400:
            uart_baud = B38400;
            break;
        case 57600:
            uart_baud = B57600;
            break;
        case 115200:
            uart_baud = B115200;
            break;
        case 230400:
            uart_baud = B230400;
            break;
        case 460800:
            uart_baud = B460800;
            break;
        case 500000:
            uart_baud = B500000;
            break;
        case 576000:
            uart_baud = B576000;
            break;
        case 921600:
            uart_baud = B921600;
            break;
        case 1000000:
            uart_baud = B1000000;
            break;
        case 1152000:
            uart_baud = B1152000;
            break;
        case 1500000:
            uart_baud = B1500000;
            break;
        case 2000000:
            uart_baud = B2000000;
            break;
        case 2500000:
            uart_baud = B2500000;
            break;
        case 3000000:
            uart_baud = B3000000;
            break;
        case 3500000:
            uart_baud = B3500000;
            break;
        case 4000000:
            uart_baud = B4000000;
            break;
        default:
            mctrl_err("Invalid baudrate %d\n", config->baudrate);
            free(ctx);
            return NULL;
        }

    ret = open(config->dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (ret <= 0)
    {
        mctrl_err("Failed to open UART device\n");
        free(ctx);
        return NULL;
    }

    ctx->fd = ret;
    fcntl(ctx->fd, F_SETFL, 0);

    /* Configure as 8N1, no flow control or modem status lines */
    tty.c_cflag = CS8 | CLOCAL;
    cfsetospeed(&tty, uart_baud);
    cfsetispeed(&tty, uart_baud);

    ret = tcsetattr(ctx->fd, TCSANOW, &tty);
    if (ret)
    {
        mctrl_err("Failed tcsetattr on UART device\n");
        free(ctx);
        return NULL;
    }

    ret = tcsetattr(ctx->fd, TCSANOW, &tty);
    if (ret)
    {
        mctrl_err("Failed tcsetattr on UART device\n");
        free(ctx);
        return NULL;
    }
    tcflush(ctx->fd, TCIOFLUSH);
    return ctx;
}

int uart_deinit(struct uart_ctx *ctx)
{
    int ret = 0;
    if (ctx != NULL)
    {
        if (ctx->fd > 0)
        {
            tcflush(ctx->fd, TCIOFLUSH);
            ret = close(ctx->fd);
        }
        free(ctx);
    }
    return ret;
}

int uart_read(struct uart_ctx *ctx, uint8_t *buf, size_t len)
{
    if (ctx->fd <= 0)
    {
        return -ETRANSERR;
    }

    return read(ctx->fd, buf, len);
}

int uart_write(struct uart_ctx *ctx, const uint8_t *buf, size_t len)
{
    if (ctx->fd <= 0)
    {
        return -ETRANSERR;
    }

    return write(ctx->fd, buf, len);
}
