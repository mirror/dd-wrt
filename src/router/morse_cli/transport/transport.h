/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#ifndef TRANSPORT_TRANSPORT_H_
#define TRANSPORT_TRANSPORT_H_ 1


#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>


enum moresctrl_transport_errnum
{
    ETRANSSUCC          = 0,
    ETRANSERR           = 2,
    ETRANSNL80211ERR    = 3,
    ETRANSFTDISPIERR    = 4,
    ETRANSNOTSUP        = 5,
    /* The following codes correspond to the errno equivalents. */
    ETRANSNOMEM         = 12,
    ETRANSNODEV         = 19,
};

/* Helper macros :) */
#define TBUFF_TO_REQ(req_tbuf, cmdtype) ((cmdtype *)((struct request *)req_tbuf->data)->data)
#define TBUFF_TO_RSP(rsp_tbuf, cmdtype) ((cmdtype *)((struct response *)rsp_tbuf->data)->data)

#define MAX_SERIAL_NUMBER_LEN (16)

struct morsectrl_transport;

/** Contains memory used to store commands and framing. */
struct morsectrl_transport_buff
{
    /** Memory block to be allocated with room for commands and transport framing. */
    uint8_t *memblock;
    /** Total capacity of the allocated memory block. */
    size_t capacity;
    /**
     * Pointer into the memory block to where the data starts (whichever is first out of commands
     * and framing).
     */
    uint8_t *data;
    /** Current size of the data (can be data or data and framing). */
    size_t data_len;
};

/**
 * Gets a regular expression string representing the supported transports.
 *
 * For example if the supported transports are called `nl80211` and `ftdi_spi` then this
 * will return `(nl80211|ftdi_spi)`.
 *
 * @returns the regular expression string, allocated using malloc. It is the responsibility
 *          of the caller to free it. @c NULL if no transports supported or error.
*/
char *morsectrl_transport_get_regex(void);

/**
 * Prints a list of available transports.
 */
void morsectrl_transport_list_available(void);

/**
 * @brief Checks whether one or more of the available transports support driver commands.
 *
 * @returns @c true if one or more of the available transports support driver commands;
 *          @c false if none of the transports support driver commands.
*/
bool morsectrl_transport_driver_commands_supported(void);

/**
 * @brief Parses the commandline options to set the correct transport and fill the configuration.
 *
 * @param[out] transport    On success a an opaque transport data structure instance will be
 *                          returned via this out argument.
 * @param debug             Indicates whether debug messages should be enabled.
 * @param trans_opts        Transport string from the commandline.
 * @param iface_opts        Interface string from the commandline.
 * @param cfg_opts          Configuration string from the commandline.
 * @return                  0 on success or relevant error.
 */
int morsectrl_transport_parse(struct morsectrl_transport **transport,
                              bool debug,
                              const char *trans_opts,
                              const char *iface_opts,
                              const char *cfg_opts);

/**
 * @brief Get the name of the given transport.
 *
 * @param transport     Transport instance to get the name of.
 *
 * @returns a null-terminated string that is the name of the transport, or "?" on error.
 */
const char *morsectrl_transport_name(struct morsectrl_transport *transport);

/**
 * @brief Initialises the transport.
 *
 * @param transport Initalised transport (has parse the configuration)
 * @return          0 on success or relevant error.
 */
int morsectrl_transport_init(struct morsectrl_transport *transport);

/**
 * @brief Deinitialises the transport
 *
 * @param transport Transport to deinit.
 * @return          0 on success or relevant error.
 */
int morsectrl_transport_deinit(struct morsectrl_transport *transport);

/**
 * @brief Allocates memory for a command to send using the provided transport.
 *
 * @param transport Transport to allocate command memory for.
 * @param size      Size of the commnd to allocate memory for. Includes the morse command header.
 * @return          the allocated @ref morsectrl_transport_buff or NULL on failure.
 */
struct morsectrl_transport_buff *morsectrl_transport_cmd_alloc(
    struct morsectrl_transport *transport, size_t size);

/**
 * @brief Allocates memory for a response to send using the provided transport.
 *
 * @param transport Transport to allocate command memory for.
 * @param size      Size of the response to allocate memory for. Includes the morse response header.
 * @return          the allocated @ref morsectrl_transport_buff or NULL on failure.
 */
struct morsectrl_transport_buff *morsectrl_transport_resp_alloc(
    struct morsectrl_transport *transport, size_t size);

/**
 * @brief Allocates memory for writing raw data using the provided transport.
 *
 * @param transport Transport to allocate command memory for.
 * @param size      Size of the data to allocate memory for.
 * @return          the allocated @ref morsectrl_transport_buff or NULL on failure.
 */
struct morsectrl_transport_buff *morsectrl_transport_raw_write_alloc(
    struct morsectrl_transport *transport, size_t size);

/**
 * @brief Allocates memory for reading raw data using the provided transport.
 *
 * @param transport Transport to allocate command memory for.
 * @param size      Size of the data to allocate memory for.
 * @return          the allocated @ref morsectrl_transport_buff or NULL on failure.
 */
struct morsectrl_transport_buff *morsectrl_transport_raw_read_alloc(
    struct morsectrl_transport *transport, size_t size);

/**
 * @brief Frees memory for a @ref morsectrl_transport_buff.
 *
 * @param transport Transport to free memory for.
 * @return          0 on success or relevant error.
 */
int morsectrl_transport_buff_free(struct morsectrl_transport_buff *buff);

int morsectrl_transport_reg_read(struct morsectrl_transport *transport,
                                 uint32_t addr, uint32_t *value);

int morsectrl_transport_reg_write(struct morsectrl_transport *transport,
                                  uint32_t addr, uint32_t value);

int morsectrl_transport_mem_read(struct morsectrl_transport *transport,
                                 struct morsectrl_transport_buff *read,
                                 uint32_t addr);

int morsectrl_transport_mem_write(struct morsectrl_transport *transport,
                                  struct morsectrl_transport_buff *write,
                                  uint32_t addr);

/**
 * @brief Send a command using the specified transport.
 *
 * @param transport Transport to send the command on.
 * @param req       Buffer containing command to send.
 * @param resp      Buffer to received response into.
 * @return          0 on success or relevant error.
 */
int morsectrl_transport_send(struct morsectrl_transport *transport,
                             struct morsectrl_transport_buff *req,
                             struct morsectrl_transport_buff *resp);

/**
 * @brief Reads raw data from the transport.
 *
 * @param transport Transport to read raw data from.
 * @param read      Buffer to read data into.
 * @param finish    Finishes the transaction if true, otherwise leave open for
 *                  more raw reads/writes/read writes.
 * @return int      0 on success or relevenat error.
 */
int morsectrl_transport_raw_read(struct morsectrl_transport *transport,
                                 struct morsectrl_transport_buff *read,
                                 bool start,
                                 bool finish);

/**
 * @brief Writes raw data to the transport.
 *
 * @param transport Transport to read raw data from.
 * @param write     Buffer to write data from.
 * @param finish    Finishes the transaction if true, otherwise leave open for
 *                  more raw reads/writes/read writes.
 * @return int      0 on success or relevenat error.
 */
int morsectrl_transport_raw_write(struct morsectrl_transport *transport,
                                  struct morsectrl_transport_buff *write,
                                  bool start,
                                  bool finish);

/**
 * @brief Reads and writes raw data to/from the transport.
 *
 * @note If read and write buffer lengths differ, behaviour is transport dependent.
 *
 * @param transport Transport to read/write raw data from/to.
 * @param read      Buffer to read data into.
 * @param write     Buffer to write data from.
 * @param finish    Finishes the transaction if true, otherwise leave open for
 *                  more raw reads/writes/read writes.
 * @return int      0 on success or relevenat error.
 */
int morsectrl_transport_raw_read_write(struct morsectrl_transport *transport,
                                       struct morsectrl_transport_buff *read,
                                       struct morsectrl_transport_buff *write,
                                       bool start,
                                       bool finish);

/**
 * @brief Reset a device using transport's hardware methods.
 *
 * @param transport Transport to perform hard reset.
 * @return int      0 on success or relevant error.
 */
int morsectrl_transport_reset_device(struct morsectrl_transport *transport);

/**
 * @brief Get the interface name
 *
 * @param transport Transport
 *
 * @return Interface name, or NULL if not defined.
 */
const char *morsectrl_transport_get_ifname(struct morsectrl_transport *transport);

/**
 * @brief Set the length of the data actually used in a command
 *
 * @param tbuff Command transport buffer
 * @param length Length of command data
 */
void morsectrl_transport_set_cmd_data_length(struct morsectrl_transport_buff *tbuff,
                                             uint16_t length);

/**
 * @brief Checks whether the given transport has reset support.
 *
 * @param transport The transport instance.
 *
 * @return @c true if the transport has reset support else @c false.
 */
bool morsectrl_transport_has_reset(struct morsectrl_transport *transport);

/**
 * @brief Checks whether the given transport supports driver commands or interfaces directly to
 *        firmware.
 *
 * @param transport The transport instance.
 *
 * @return @c true if the transport supports driver commands else @c false.
 */
bool morsectrl_transport_has_driver(struct morsectrl_transport *transport);

/**
 * Print an error message.
 *
 * @param prefix        Message prefix
 * @param error_code    Integer error code
 * @param error_msg     Textual error message with some human readable information about the error.
 */
void morsectrl_transport_err(const char *prefix, int error_code, const char *error_msg);

/**
 * @brief Print a debug message, if enabled.
 *
 * @param transport The transport instance.
 * @param fmt       The printf format string.
 * @param ...       Optional printf arguments.
 */
void morsectrl_transport_debug(struct morsectrl_transport *transport, const char *fmt, ...);

#endif // TRANSPORT_TRANSPORT_H_
