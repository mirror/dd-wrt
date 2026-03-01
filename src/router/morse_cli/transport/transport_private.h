/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

/*
 * WARNING: this function is for internal use within the transport subsystem only! Do not include
 *          from outside the transport directory. All access to transport interfaces must go
 *          via transport.h.
 */

#pragma once


#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#include "transport.h"

/**
 * @brief Transport operations and information data structure
 *
 * Each transport must register a const instance of this data structure using REGISTER_TRANSPORT().
 */
struct morsectrl_transport_ops
{
    const char *name;
    const char *description;
    bool has_reset;
    bool has_driver;
    /** Parse the configuration for the chosen transport. */
    int (*parse)(struct morsectrl_transport **transport,
                 bool debug,
                 const char *iface_opts,
                 const char *cfg_opts);
    /** Initialise the provided transport. */
    int (*init)(struct morsectrl_transport *transport);
    /** Deinitialise the provided transport. */
    int (*deinit)(struct morsectrl_transport *transport);
    /** Allocate memory for a command. */
    struct morsectrl_transport_buff *(*write_alloc)(
        struct morsectrl_transport *transport, size_t size);
    /** Allocate memory for a response. */
    struct morsectrl_transport_buff *(*read_alloc)(
        struct morsectrl_transport *transport, size_t size);
    /** Send a command and receive a response. */
    int (*send)(struct morsectrl_transport *transport,
                struct morsectrl_transport_buff *cmd,
                struct morsectrl_transport_buff *resp);
    /** Read a 32bit register. */
    int (*reg_read)(struct morsectrl_transport *transport,
                    uint32_t addr, uint32_t *value);
    /** Write a 32bit register. */
    int (*reg_write)(struct morsectrl_transport *transport,
                     uint32_t addr, uint32_t value);
    /** Read a word aligned memory block. */
    int (*mem_read)(struct morsectrl_transport *transport,
                    struct morsectrl_transport_buff *read,
                    uint32_t addr);
    /** Write a word aligned memory block. */
    int (*mem_write)(struct morsectrl_transport *transport,
                     struct morsectrl_transport_buff *write,
                     uint32_t addr);
    /** Perform a raw read from the transport. */
    int (*raw_read)(struct morsectrl_transport *transport,
                    struct morsectrl_transport_buff *read,
                    bool start,
                    bool finish);
    /** Perform a raw write from the transport. */
    int (*raw_write)(struct morsectrl_transport *transport,
                     struct morsectrl_transport_buff *write,
                     bool start,
                     bool finish);
    /** Perform a raw read and write simultaneously from the transport. */
    int (*raw_read_write)(struct morsectrl_transport *transport,
                          struct morsectrl_transport_buff *read,
                          struct morsectrl_transport_buff *write,
                          bool start,
                          bool finish);
    /** Reset the device. */
    int (*reset_device)(struct morsectrl_transport *transport);
    /** Retrieve the interface name, if supported (optional; may be NULL if not supported). */
    const char *(*get_ifname)(struct morsectrl_transport *transport);
};

/**
 * @brief Common transport  data.
 *
 * This is "subclassed" by transports by making it the first member of a super struct.
 */
struct morsectrl_transport
{
    /** Transport operations data structure for this transport. */
    const struct morsectrl_transport_ops *tops;
    /** Flag indicating whether debug messages are enabled. */
    bool debug;
};


#define REGISTER_TRANSPORT(_ops) \
    const struct morsectrl_transport_ops * const \
    __attribute__((section("transport_ops_table"))) transport_##_ops = &(_ops)
