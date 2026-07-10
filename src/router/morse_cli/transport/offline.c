/*
 * Copyright 2026 Morse Micro
 */

/*
 * Transport layer for offline operation (no live device available)
 */

#include <stdlib.h>
#include "transport.h"
#include "transport_private.h"

static const struct morsectrl_transport_ops offline_ops;

/** @brief Data structure used to represent an instance of this trasport. */
struct morsectrl_offline_transport
{
    struct morsectrl_transport common;
};

/**
 * @brief Prints an error message if possible.
 *
 * @param transport     Transport to print the error message from.
 * @param error_code    Error code.
 * @param error_msg     Error message.
 */
static void offline_error(int error_code, char *error_msg)
{
    morsectrl_transport_err("OFFLINE", error_code, error_msg);
}

/**
 * @brief Parse the configuration for offline
 *
 * @param transport     The transport structure.
 * @param debug         Indicates whether debug print statements are enabled.
 * @param iface_opts    String containing the interface to use. May be NULL.
 * @param cfg_opts      Comma separated string with configuration options.
 * @return              0 on success otherwise relevant error.
 */
static int offline_parse(struct morsectrl_transport **transport,
                           bool debug,
                           const char *iface_opts,
                           const char *cfg_opts)
{
    static struct morsectrl_offline_transport offline_transport;
    offline_transport.common.debug = debug;
    offline_transport.common.tops = &offline_ops;
    *transport = &offline_transport.common;

    return 0;
}

/**
 * @brief Initalise offline transport
 *
 * @note This should be done after parsing the configuration.
 *
 * @param transport Transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int offline_init(struct morsectrl_transport *transport)
{
    return ETRANSSUCC;
}

/**
 * @brief De-initalise offline transport.
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int offline_deinit(struct morsectrl_transport *transport)
{
    return 0;
}

static struct morsectrl_transport_buff *offline_alloc(struct morsectrl_transport *transport,
                                                        size_t size)
{
    offline_error(-ETRANSERR, "Offline transport does not support this");
    return NULL;
}

static int offline_send(struct morsectrl_transport *transport,
                         struct morsectrl_transport_buff *req,
                         struct morsectrl_transport_buff *resp)
{
    offline_error(-ETRANSERR, "Offline transport does not support this");
    return -ETRANSERR;
}

static const struct morsectrl_transport_ops offline_ops = {
    .name = "offline",
    .description = "Offline commands only (no live device available)",
    .has_reset = false,
    .has_driver = false,
    .parse = offline_parse,
    .init = offline_init,
    .deinit = offline_deinit,
    .write_alloc = offline_alloc,
    .read_alloc = offline_alloc,
    .send = offline_send,
    .reg_read = NULL,
    .reg_write = NULL,
    .mem_read = NULL,
    .mem_write = NULL,
    .raw_read = NULL,
    .raw_write = NULL,
    .raw_read_write = NULL,
    .reset_device = NULL,
    .get_ifname = NULL,
    .connect = NULL,
};

REGISTER_TRANSPORT(offline_ops);
