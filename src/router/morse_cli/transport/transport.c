/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "transport.h"
#include "transport_private.h"
#include "../command.h"
#include "../utilities.h"

/** Maximum length of the string (excluding null-terminator) to return from
 *  @ref morsectrl_transport_get_regex(). */
#define TRANSPORT_REGEX_MAXLEN  (127)

/* These are the start and end of the custom section transport_ops_table and are provided
 * by the linker script. */
extern const struct morsectrl_transport_ops * const __start_transport_ops_table[];
extern const struct morsectrl_transport_ops * const __stop_transport_ops_table[];


static const struct morsectrl_transport_ops * const *ops_table_iterator_next(
    const struct morsectrl_transport_ops * const *iter)
{
    /* We loop until we find a transport ops entry whose name is not NULL. A NULL name indicates
     * the "dummy" entry that we use a placeholder to ensure the table is never empty. */
    do
    {
        const struct morsectrl_transport_ops *tops;

        iter += 1;
        if (iter >= __stop_transport_ops_table)
        {
            return NULL;
        }

        tops = *iter;
        MCTRL_ASSERT(tops, "Malformed transport table");
    } while ((*iter)->name == NULL);

    return iter;
}

static const struct morsectrl_transport_ops * const *get_ops_table_iterator(void)
{
    const struct morsectrl_transport_ops * const *iter = __start_transport_ops_table;

    if (iter < __stop_transport_ops_table)
    {
        /* A NULL name indicates the "dummy" entry that we use a placeholder to ensure the table
         * is never empty. We need to skip it. */
        if ((*iter)->name == NULL)
        {
            return ops_table_iterator_next(iter);
        }
        else
        {
            return iter;
        }
    }
    else
    {
        return NULL;
    }
}


static bool ops_table_iter_is_first(const struct morsectrl_transport_ops * const *iter)
{
    return iter == __start_transport_ops_table;
}

const struct morsectrl_transport_ops *find_transport_ops(const char *transport_name)
{
    const struct morsectrl_transport_ops * const *iter;

    for (iter = get_ops_table_iterator(); iter != NULL; iter = ops_table_iterator_next(iter))
    {
        const struct morsectrl_transport_ops *tops = *iter;

        if (!transport_name || strcmp(transport_name, tops->name) == 0)
        {
            return tops;
        }
    }

    return NULL;
}

char *morsectrl_transport_get_regex(void)
{
    char *str = calloc(1, TRANSPORT_REGEX_MAXLEN);
    int len = 0;
    bool first = true;
    const struct morsectrl_transport_ops * const *iter;

    for (iter = get_ops_table_iterator(); iter != NULL; iter = ops_table_iterator_next(iter))
    {
        const struct morsectrl_transport_ops *tops = *iter;
        int name_len = strlen(tops->name);

        if ((name_len + 1) == (TRANSPORT_REGEX_MAXLEN - len))
        {
            mctrl_print("Error constructing transport regex\n");
            free(str);
            return NULL;
        }

        if (first)
        {
            str[len++] = '(';
            first = false;
        }
        else
        {
            str[len++] = '|';
        }

        memcpy(str + len, tops->name, name_len);
        len += name_len;
    }

    if (len == 0)
    {
        mctrl_print("No transports supported\n");
        free(str);
        return NULL;
    }

    if ((TRANSPORT_REGEX_MAXLEN - len) <= 1)
    {
        mctrl_print("Error constructing transport regex\n");
        free(str);
        return NULL;
    }

    str[len++] = ')';
    return str;
}

void morsectrl_transport_list_available(void)
{
    const struct morsectrl_transport_ops * const *iter;

    mctrl_print("\nTransports Available ({-t|--transport}=<transport>):\n");

    /*
     * Important note
     *
     * We use the first transport as the default transport. The order of transports is
     * given by the order we pass the object files to the linker. Therefore, to prioritise
     * one transport over another, ensure it comes earlier in the link order.
     */

    for (iter = get_ops_table_iterator();  iter != NULL; iter = ops_table_iterator_next(iter))
    {
        const struct morsectrl_transport_ops *tops = *iter;
        const char *default_str = ops_table_iter_is_first(iter) ? " [default]" : "";

        mctrl_print("\t%-15s%s%s\n", tops->name, tops->description, default_str);
    }
}

bool morsectrl_transport_driver_commands_supported(void)
{
    const struct morsectrl_transport_ops * const *iter;

    for (iter = get_ops_table_iterator();  iter != NULL; iter = ops_table_iterator_next(iter))
    {
        const struct morsectrl_transport_ops *tops = *iter;

        if (tops->has_driver)
        {
            return true;
        }
    }

    return false;
}


int morsectrl_transport_parse(struct morsectrl_transport **transport,
                              bool debug,
                              const char *trans_opts,
                              const char *iface_opts,
                              const char *cfg_opts)
{
    int ret = -ETRANSERR;
    const struct morsectrl_transport_ops *tops;

    *transport = NULL;

    tops = find_transport_ops(trans_opts);
    if (!tops)
    {
        morsectrl_transport_err("Transport parsing", -ETRANSERR, "Invalid transport");
        return -ETRANSNODEV;
    }

    if (debug)
    {
        if (!trans_opts)
        {
            mctrl_print("Transport set to default: %s\n", tops->name);
        }
        else
        {
            mctrl_print("Transport set to: %s\n", tops->name);
        }
    }

    ret = tops->parse(transport, debug, iface_opts, cfg_opts);

    return ret;
}

const char *morsectrl_transport_name(struct morsectrl_transport *transport)
{
    if (!transport || !transport->tops)
    {
        return "<invalid>";
    }

    return transport->tops->name;
}

int morsectrl_transport_init(struct morsectrl_transport *transport)
{
    if (transport->tops && transport->tops->init)
        return transport->tops->init(transport);

    return -ETRANSERR;
}

int morsectrl_transport_deinit(struct morsectrl_transport *transport)
{
    int ret = ETRANSSUCC;

    if (transport->tops && transport->tops->deinit)
        ret = transport->tops->deinit(transport);

    transport->tops = NULL;

    return ret;
}

struct morsectrl_transport_buff *morsectrl_transport_cmd_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    if (!transport->tops)
        return NULL;

    /* Add the size of the command header and pass down to the correct transport. */
    return transport->tops->write_alloc(transport, sizeof(struct request) + size);
}

struct morsectrl_transport_buff *morsectrl_transport_resp_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    if (!transport->tops)
        return NULL;

    /* Add the size of the response header and pass down to the correct transport. */
    return transport->tops->read_alloc(transport, sizeof(struct response) + size);
}

struct morsectrl_transport_buff *morsectrl_transport_raw_read_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    if (!transport->tops || !transport->tops->read_alloc)
        return NULL;

    /* Add the size of the response header and pass down to the correct transport. */
    return transport->tops->read_alloc(transport, size);
}

struct morsectrl_transport_buff *morsectrl_transport_raw_write_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    if (!transport->tops || !transport->tops->write_alloc)
        return NULL;

    /* Add the size of the response header and pass down to the correct transport. */
    return transport->tops->write_alloc(transport, size);
}

int morsectrl_transport_buff_free(struct morsectrl_transport_buff *buff)
{
    if (!buff)
        return -ETRANSERR;

    free(buff->memblock);
    free(buff);

    return ETRANSSUCC;
}

void morsectrl_transport_set_cmd_data_length(struct morsectrl_transport_buff *tbuff,
                                             uint16_t length)
{
    tbuff->data_len = sizeof(struct request) + length;
}

int morsectrl_transport_reg_read(struct morsectrl_transport *transport,
                                 uint32_t addr, uint32_t *value)
{
    if (!transport->tops || !transport->tops->reg_read)
        return -ETRANSERR;

    return transport->tops->reg_read(transport, addr, value);
}

int morsectrl_transport_reg_write(struct morsectrl_transport *transport,
                                  uint32_t addr, uint32_t value)
{
    if (!transport->tops)
        return -ETRANSERR;

    if (!transport->tops->reg_write)
        return -ETRANSNOTSUP;

    return transport->tops->reg_write(transport, addr, value);
}

int morsectrl_transport_mem_read(struct morsectrl_transport *transport,
                                 struct morsectrl_transport_buff *read,
                                 uint32_t addr)
{
    if (!transport->tops)
        return -ETRANSERR;

    if (!transport->tops->mem_read)
        return -ETRANSNOTSUP;

    return transport->tops->mem_read(transport, read, addr);
}

int morsectrl_transport_mem_write(struct morsectrl_transport *transport,
                                  struct morsectrl_transport_buff *write,
                                  uint32_t addr)
{
    if (!transport->tops)
        return -ETRANSERR;

    if (!transport->tops->mem_write)
        return -ETRANSNOTSUP;

    return transport->tops->mem_write(transport, write, addr);
}

int morsectrl_transport_send(struct morsectrl_transport *transport,
                             struct morsectrl_transport_buff *req,
                             struct morsectrl_transport_buff *resp)
{
    if (!transport->tops)
        return -ETRANSERR;

    return transport->tops->send(transport, req, resp);
}

int morsectrl_transport_raw_read(struct morsectrl_transport *transport,
                                 struct morsectrl_transport_buff *read,
                                 bool start,
                                 bool finish)
{
    if (!transport->tops)
        return -ETRANSERR;

    return transport->tops->raw_read(transport, read, start, finish);
}

int morsectrl_transport_raw_write(struct morsectrl_transport *transport,
                                  struct morsectrl_transport_buff *write,
                                  bool start,
                                  bool finish)
{
    if (!transport->tops)
        return -ETRANSERR;

    return transport->tops->raw_write(transport, write, start, finish);
}

int morsectrl_transport_raw_read_write(struct morsectrl_transport *transport,
                                       struct morsectrl_transport_buff *read,
                                       struct morsectrl_transport_buff *write,
                                       bool start,
                                       bool finish)
{
    if (!transport->tops)
        return -ETRANSERR;

    return transport->tops->raw_read_write(transport, read, write, start, finish);
} /* NOLINT */

int morsectrl_transport_reset_device(struct morsectrl_transport *transport)
{
    if (!transport->tops)
        return -ETRANSERR;

    if (!transport->tops->reset_device)
        return -ETRANSNOTSUP;


    return transport->tops->reset_device(transport);
}

const char *morsectrl_transport_get_ifname(struct morsectrl_transport *transport)
{
    if (!transport->tops || !transport->tops->get_ifname)
        return NULL;

    return transport->tops->get_ifname(transport);
}

bool morsectrl_transport_has_reset(struct morsectrl_transport *transport)
{
    if (transport != NULL && transport->tops != NULL)
    {
        return transport->tops->has_reset;
    }
    else
    {
        return false;
    }
}

bool morsectrl_transport_has_driver(struct morsectrl_transport *transport)
{
    if (transport != NULL && transport->tops != NULL)
    {
        return transport->tops->has_driver;
    }
    else
    {
        return false;
    }
}

void morsectrl_transport_err(const char *prefix, int error_code, const char *error_msg)
{
    mctrl_err("%s, code %d: %s\n", prefix, error_code, error_msg);
}

void morsectrl_transport_debug(struct morsectrl_transport *transport, const char *fmt, ...)
{
    if (transport != NULL && transport->debug)
    {
        va_list args;
        va_start(args, fmt);
        mctrl_vprint(fmt, args);
        va_end(args);
    }
}

/* Dummy transport to ensure the transport_ops table always exists. */
const struct morsectrl_transport_ops dummy = { };
REGISTER_TRANSPORT(dummy);
