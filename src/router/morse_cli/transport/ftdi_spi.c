/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ftd2xx.h"
#include "libmpsse_spi.h"
#include "transport.h"
#include "transport_private.h"
#include "sdio_over_spi.h"

/* We need this to trim the response to the correct length */
#include "../command.h"

#if MORSE_WIN_BUILD
#include "../win/strsep.h"
#endif


#define MM_OCTETS_OF_INIT_CLK           (18)

#define MMDEBUG_CS_PIN_DEFAULT          SPI_CONFIG_OPTION_CS_DBUS3
#define MMDEBUG_CS_ACTIVE_LOW_DEFAULT   true
#define MMDEBUG_JTAGRST_PIN_DEFAULT     (0)
#define MMDEBUG_RST_PIN_DEFAULT         (1)
#define MMDEBUG_RESET_MS_DEFAULT        (100)
#define MMDEBUG_CPOL_DEFAULT            false
#define MMDEBUG_CPHA_DEFAULT            false
#define MMDEBUG_LATENCY_DEFAULT         (0)
#define MMDEBUG_MAX_CHANNELS            (4UL)

#define FTDI_SPI_MIN_FREQ_KHZ           (1)
#define FTDI_SPI_MAX_FREQ_KHZ           (30000)

#define MMDEBUG_FREQ_KHZ_DEFAULT        FTDI_SPI_MAX_FREQ_KHZ

#define FTDI_SPI_OPTS_BITS              BIT(0)
#define FTDI_SPI_OPTS_CS_START          BIT(1)
#define FTDI_SPI_OPTS_CS_FINISH         BIT(2)

/* GPIO are from 0-7, 4 on each channel. */
#define FTDI_SPI_MAX_GPIO               (3)
#define FTDI_SPI_GPIO_OFFSET            (4)
#define FTDI_SPI_GPIOL_MASK             (0xFF)

#define FTDI_SPI_JUNK_OCTET             (0xFF)

#define FTDI_SPI_STR_CPOL               "cpol"
#define FTDI_SPI_STR_CPHA               "cpha"
#define FTDI_SPI_STR_FREQ               "freq_khz"
#define FTDI_SPI_STR_LAG                "latency"
#define FTDI_SPI_STR_CS_POL             "cs_pol"
#define FTDI_SPI_STR_CS_PIN             "cs_pin"
#define FTDI_SPI_STR_RST_PIN            "reset_pin_num"
#define FTDI_SPI_STR_JTAGRST_PIN        "jtag_reset_pin_num"
#define FTDI_SPI_STR_RESET_MS           "reset_ms"
#define FTDI_SPI_STR_SERIAL_NUM         "serial_num"
#define FTDI_SPI_STR_HELP               "help"

#define RESP_TIMEOUT_MS                 (3000)
#define RESP_POLL_INTERVAL_MS           (100)

#define FTDI_SPI_PINSTATE_TO_VAL(x)     (((x) >> 8) & 0xFF)
#define FTDI_SPI_PINSTATE_TO_DIR(x)     ((x) & 0xFF)

#define MM_MANIFEST_ADDR                (0x10054d40)
#define MM_TRIGGER_ADDR                 (0x100A6010)
#define MM_STATUS_ADDR                  (0x100A6060)
#define MM_STATUS_CLR_ADDR              (0x100A6068)
#define MM_CMD_MASK                     BIT(1)
#define MM_CMD_ADDR_OFFSET              (16)
#define MM_RESP_ADDR_OFFSET             (20)

#define FTDI_SPI_FREQ_KHZ_TO_HZ(freq_khz)  ((freq_khz) * 1000)


static const struct morsectrl_transport_ops ftdi_spi_ops;

struct morsectrl_ftdi_spi_chan_info
{
    DWORD spi_loc_id;
    DWORD spi_loc_id_ch;
    DWORD reset_loc_id;
    DWORD reset_loc_id_ch;
};

/** @brief Configuration for the FTDI SPI interface. */
struct morsectrl_ftdi_spi_cfg
{
    ChannelConfig channel;
    UCHAR reset_pin_num;
    UCHAR jtag_reset_pin_num;
    uint32_t reset_ms;
    /** Size taken from FT_DEVICE_LIST_INFO_NODE structure in libmpsse library. */
    char serial_num[MAX_SERIAL_NUMBER_LEN];
};

/** @brief State information for the FTDI SPI interface. */
struct morsectrl_ftdi_spi_state
{
    FT_HANDLE handle;
    FT_HANDLE reset_handle;
};

/** @brief Data structure used to represent an instance of this transport. */
struct morsectrl_ftdi_spi_transport
{
    struct morsectrl_transport common;
    struct morsectrl_ftdi_spi_cfg config;
    struct morsectrl_ftdi_spi_state state;
};

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *        config field.
 */
static struct morsectrl_ftdi_spi_cfg *ftdi_spi_cfg(struct morsectrl_transport *transport)
{
    struct morsectrl_ftdi_spi_transport *ftdi_spi_transport =
        (struct morsectrl_ftdi_spi_transport *)transport;
    return &ftdi_spi_transport->config;
}

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *        state field.
 */
static struct morsectrl_ftdi_spi_state *ftdi_spi_state(struct morsectrl_transport *transport)
{
    struct morsectrl_ftdi_spi_transport *ftdi_spi_transport =
        (struct morsectrl_ftdi_spi_transport *)transport;
    return &ftdi_spi_transport->state;
}

/**
 * @brief Prints an error message if possible.
 *
 * @param transport     Transport to print the error message from.
 * @param error_code    Error code.
 * @param error_msg     Error message.
 */
static void ftdi_spi_error(int error_code, char *error_msg)
{
    morsectrl_transport_err("FTDI_SPI", error_code, error_msg);
}

/**
 * @brief Checks to see if a string contains the key and fills the boolean value.
 *
 * @param str   String that may contain the key.
 * @param key   Key to search for.
 * @param value Value to fill with key value, otherwise unchanged if key absent.
 * @return      true if key was found, otherwise false.
 */
static bool ftdi_spi_get_bool(const char *str, const char *key, bool *value)
{
    if (!strncmp(str, key, strlen(key)))
    {
        int temp = expression_to_int(&str[strlen(key) + 1]);

        *value = !!temp;
        return true;
    }

    return false;
}

/**
 * @brief Checks to see if a string contains the key and fills the uint32_t value.
 *
 * @param str   String that may contain the key.
 * @param key   Key to search for.
 * @param value Value to fill with key value, otherwise unchanged if key absent.
 * @return      true if key was found, otherwise false.
 */
static bool ftdi_spi_get_uint32(const char *str, const char *key, uint32_t *value)
{
    if (!strncmp(str, key, strlen(key)))
    {
        int ret;
        uint32_t temp;
        ret = str_to_uint32(&str[strlen(key) + 1], &temp);
        if (!ret)
        {
            *value = temp;
            return true;
        }
    }

    return false;
}

/**
 * @brief Checks to see if a string contains the key and fills the uint8_t value.
 *
 * @param str   String that may contain the key.
 * @param key   Key to search for.
 * @param value Value to fill with key value, otherwise unchanged if key absent.
 * @return      true if key was found, otherwise false.
 */
static bool ftdi_spi_get_uint8(const char *str, const char *key, uint8_t *value)
{
    uint32_t temp;
    bool ret;

    ret = ftdi_spi_get_uint32(str, key, &temp);

    if (ret)
    {
        *value = (uint8_t)temp;
        return true;
    }

    return false;
}

/**
 * @brief Checks to see if a string contains the key and fills the string value.
 *
 * @param str   String that may contain the key.
 * @param key   Key to search for.
 * @param value Value to fill with key value, otherwise unchanged if key absent.
 * @return      true if key was found, otherwise false.
 */
static bool ftdi_spi_get_string(const char *str, const char *key, char *val, int len_max)
{
    uint8_t key_len = strlen(key);

    if ((strlen(str) > (key_len + 1)) && (!strncmp(str, key, key_len)) && (str[key_len] == '='))
    {
        if (snprintf(val, len_max, "%s", (str + key_len + 1)) >= len_max)
        {
            mctrl_err("Length of %s exceeds max (max len=%d)\n", key, len_max);
            return false;
        }
        return true;
    }

    return false;
}

static bool ftdi_spi_print_config_usage(const char *str, const char *key)
{
    if (strncmp(str, key, strlen(key)))
    {
        return false;
    }
    mctrl_print("<config string> is a comma-separated list of <keyword>=<value>, "
                "where <keyword> is one of the following\n");
    mctrl_print("\t%s - Clock polarity (default %d)\n", FTDI_SPI_STR_CPOL,
                    MMDEBUG_CPOL_DEFAULT);
    mctrl_print("\t%s - Clock phase (default %d)\n", FTDI_SPI_STR_CPHA, MMDEBUG_CPHA_DEFAULT);
    mctrl_print("\t%s - Frequency to use (default %d)\n", FTDI_SPI_STR_FREQ,
                    MMDEBUG_FREQ_KHZ_DEFAULT);
    mctrl_print("\t%s - Latency (default %d)\n", FTDI_SPI_STR_LAG, MMDEBUG_LATENCY_DEFAULT);
    mctrl_print("\t%s - CS Polarity (default %d)\n", FTDI_SPI_STR_CS_POL,
                    MMDEBUG_CS_ACTIVE_LOW_DEFAULT);
    mctrl_print("\t%s - CS pin number (default %d)\n", FTDI_SPI_STR_CS_PIN,
                    MMDEBUG_CS_PIN_DEFAULT);
    mctrl_print("\t%s - Reset pin number (default %d)\n", FTDI_SPI_STR_RST_PIN,
                    MMDEBUG_RST_PIN_DEFAULT);
    mctrl_print("\t%s - JTAG reset pin number (default %d)\n", FTDI_SPI_STR_JTAGRST_PIN,
                    MMDEBUG_JTAGRST_PIN_DEFAULT);
    mctrl_print("\t%s - Reset time (default %d)\n", FTDI_SPI_STR_RESET_MS,
                    MMDEBUG_RESET_MS_DEFAULT);
    mctrl_print("\t%s - Serial number to use\n", FTDI_SPI_STR_SERIAL_NUM);
    mctrl_print("\t%s - Prints this message\n", FTDI_SPI_STR_HELP);

    return true;
}

/**
 * @brief Parse the configuration for the FTDI SPI interface.
 *
 * @param transport     The transport structure.
 * @param debug         Indicates whether debug print statements are enabled.
 * @param iface_opts    String containing the interface to use. May be NULL.
 * @param cfg_opts      Comma separated string with FTDI SPI configuration options.
 * @return              0 on success otherwise relevant error.
 */
static int ftdi_spi_parse(struct morsectrl_transport **transport,
                          bool debug,
                          const char *iface_opts,
                          const char *cfg_opts)
{
    struct morsectrl_ftdi_spi_cfg *config;
    ChannelConfig *chan_config;
    char *cpy;
    char *ptr;
    /* Use some sane defaults. */
    bool cpol = MMDEBUG_CPOL_DEFAULT;
    bool cpha = MMDEBUG_CPHA_DEFAULT;
    bool cs_active_low = MMDEBUG_CS_ACTIVE_LOW_DEFAULT;
    uint32_t cs_pin = MMDEBUG_CS_PIN_DEFAULT;
    uint32_t freq_khz = MMDEBUG_FREQ_KHZ_DEFAULT;
    uint8_t reset_pin_num = MMDEBUG_RST_PIN_DEFAULT;
    uint8_t jtag_reset_pin_num = MMDEBUG_JTAGRST_PIN_DEFAULT;
    int config_error = 0;

    struct morsectrl_ftdi_spi_transport *ftdi_spi_transport =
        calloc(1, sizeof(*ftdi_spi_transport));
    if (!ftdi_spi_transport)
    {
        mctrl_err("Transport memory allocation failure\n");
        return -ETRANSNOMEM;
    }

    ftdi_spi_transport->common.tops = &ftdi_spi_ops;
    ftdi_spi_transport->common.debug = debug;
    *transport = &ftdi_spi_transport->common;
    config = ftdi_spi_cfg(*transport);

    chan_config = &config->channel;

    chan_config->LatencyTimer = MMDEBUG_LATENCY_DEFAULT;
    chan_config->Pin = 0xFFFFFFFF;
    chan_config->configOptions = 0;
    config->reset_ms = MMDEBUG_RESET_MS_DEFAULT;

    if (cfg_opts)
    {
        cpy = strdup(cfg_opts);

        while ((ptr = strsep(&cpy, ",")) != NULL)
        {
            if (ftdi_spi_get_bool(ptr, FTDI_SPI_STR_CPOL, &cpol))
                continue;
            if (ftdi_spi_get_bool(ptr, FTDI_SPI_STR_CPHA, &cpha))
                continue;
            if (ftdi_spi_get_uint32(ptr, FTDI_SPI_STR_FREQ, &freq_khz))
                continue;
            if (ftdi_spi_get_uint8(ptr, FTDI_SPI_STR_LAG, &chan_config->LatencyTimer))
                continue;
            if (ftdi_spi_get_bool(ptr, FTDI_SPI_STR_CS_POL, &cs_active_low))
                continue;
            if (ftdi_spi_get_uint32(ptr, FTDI_SPI_STR_CS_PIN, &cs_pin))
                continue;
            if (ftdi_spi_get_uint8(ptr, FTDI_SPI_STR_RST_PIN, &reset_pin_num))
                continue;
            if (ftdi_spi_get_uint8(ptr, FTDI_SPI_STR_JTAGRST_PIN, &jtag_reset_pin_num))
                continue;
            if (ftdi_spi_get_uint32(ptr, FTDI_SPI_STR_RESET_MS, &config->reset_ms))
                continue;
            if (ftdi_spi_get_string(ptr, FTDI_SPI_STR_SERIAL_NUM, config->serial_num,
                                    sizeof(config->serial_num)))
                continue;
            if (ftdi_spi_print_config_usage(ptr, FTDI_SPI_STR_HELP))
                exit(ETRANSSUCC);

            config_error++;
        }
    }

    if (config_error)
    {
        mctrl_err("FTDI SPI configuration error\n");
        ftdi_spi_print_config_usage("help", FTDI_SPI_STR_HELP);
        return ETRANSERR;
    }

    if (!cpol)
    {
        if (!cpha)
            chan_config->configOptions |= SPI_CONFIG_OPTION_MODE0;
        else
            chan_config->configOptions |= SPI_CONFIG_OPTION_MODE1;
    }
    else
    {
        if (!cpha)
            chan_config->configOptions |= SPI_CONFIG_OPTION_MODE2;
        else
            chan_config->configOptions |= SPI_CONFIG_OPTION_MODE3;
    }

    if (freq_khz > FTDI_SPI_MAX_FREQ_KHZ)
        chan_config->ClockRate = FTDI_SPI_FREQ_KHZ_TO_HZ(FTDI_SPI_MAX_FREQ_KHZ);
    else if (freq_khz < FTDI_SPI_MIN_FREQ_KHZ)
        chan_config->ClockRate = FTDI_SPI_FREQ_KHZ_TO_HZ(FTDI_SPI_MIN_FREQ_KHZ);
    else
        chan_config->ClockRate = FTDI_SPI_FREQ_KHZ_TO_HZ(freq_khz);

    if (cs_active_low)
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_ACTIVELOW;
    else
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_ACTIVEHIGH;

    switch (cs_pin)
    {
    case 3:
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_DBUS3;
        break;

    case 4:
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_DBUS4;
        break;

    case 5:
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_DBUS5;
        break;

    case 6:
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_DBUS6;
        break;

    case 7:
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_DBUS7;
        break;

    default:
        chan_config->configOptions |= SPI_CONFIG_OPTION_CS_DBUS3;
        cs_pin = MMDEBUG_CS_PIN_DEFAULT;
    }

    config->jtag_reset_pin_num = BIT(jtag_reset_pin_num + FTDI_SPI_GPIO_OFFSET);
    config->reset_pin_num = BIT(reset_pin_num + FTDI_SPI_GPIO_OFFSET);

    if (ftdi_spi_transport->common.debug)
    {
        mctrl_print("Frequency       = %u Hz\n", chan_config->ClockRate);
        mctrl_print("Latency         = %d Cycles\n", chan_config->LatencyTimer);
        mctrl_print("CPOL            = %d\n", cpol ? 1 : 0);
        mctrl_print("CPHA            = %d\n", cpha ? 1 : 0);
        mctrl_print("CS Polarity     = Active %s\n", cs_active_low ? "low" : "high");
        mctrl_print("CS Pin          = DBUS%d\n", cs_pin);
        mctrl_print("Reset Pin       = %d\n", reset_pin_num);
        mctrl_print("JTAG Reset Pin  = %d\n", jtag_reset_pin_num);
        mctrl_print("Reset time (ms) = %u\n", config->reset_ms);
        mctrl_print("Serial Number   = %s\n", strlen(config->serial_num) ?
                                              config->serial_num : "N/A");
    }

    return 0;
}

/**
 * @brief Sets the spi and reset channel info to use
 *
 * @param loc_id         Location id of the port.
 * @param channel_num    Channel number on which the port is present.
 * @param spi_chan_info  Structure to contain the SPI and reset port channel number and id
 */
static void ftdi_spi_set_spi_and_reset_chan(DWORD loc_id,
                                            unsigned int channel_num,
                                            struct morsectrl_ftdi_spi_chan_info *spi_chan_info)
{
    /* The lowest location ID will be the SPI port and the second will be the reset port. */
    if (loc_id < spi_chan_info->spi_loc_id)
    {
        spi_chan_info->reset_loc_id = spi_chan_info->spi_loc_id;
        spi_chan_info->reset_loc_id_ch = spi_chan_info->spi_loc_id_ch;
        spi_chan_info->spi_loc_id = loc_id;
        spi_chan_info->spi_loc_id_ch = channel_num;
    }
    else if (loc_id < spi_chan_info->reset_loc_id)
    {
        spi_chan_info->reset_loc_id = loc_id;
        spi_chan_info->reset_loc_id_ch = channel_num;
    }
}

/**
 * @brief Initalise an FTDI SPI interface.
 *
 * @note This should be done after parsing the configuration.
 *
 * @param transport Transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_init(struct morsectrl_transport *transport)
{
    struct morsectrl_ftdi_spi_state *state = ftdi_spi_state(transport);
    struct morsectrl_ftdi_spi_cfg *config = ftdi_spi_cfg(transport);
    struct morsectrl_ftdi_spi_chan_info spi_chan_info = {
        .spi_loc_id = UINT32_MAX,
        .spi_loc_id_ch = UINT32_MAX,
        .reset_loc_id = UINT32_MAX,
        .reset_loc_id_ch = UINT32_MAX
    };
    FT_DEVICE_LIST_INFO_NODE device_node;
    FT_STATUS status;
    DWORD num_chan;
    DWORD verMPSSE = 0;
    DWORD verD2XX = 0;
    unsigned int ii;
    char serial_num_list[MMDEBUG_MAX_CHANNELS][MAX_SERIAL_NUMBER_LEN] = {"\0"};

    Init_libMPSSE();

    if (transport->debug)
    {
        Ver_libMPSSE(&verMPSSE, &verD2XX);
        mctrl_print("libmpsse version:  0x%08x\n", verMPSSE);
        mctrl_print("libftd2xx version: 0x%08x\n", verD2XX);
    }

    status = SPI_GetNumChannels(&num_chan);

    if (status != FT_OK)
        return -ETRANSFTDISPIERR;

    if (transport->debug)
    {
        mctrl_print("Number of available SPI channels %u\n", num_chan);
    }

    if (num_chan > MMDEBUG_MAX_CHANNELS)
    {
        ftdi_spi_error(num_chan, "Too many SPI channels");
        return -ETRANSFTDISPIERR;
    }

    for (ii = 0; ii < num_chan; ii ++)
    {
        status = SPI_GetChannelInfo(ii, &device_node);
        if (status != FT_OK)
        {
            mctrl_err("FTDI_SPI, code %d: Failed to get SPI channel %d information\n", status, ii);
            continue;
        }
        strncpy(serial_num_list[ii], device_node.SerialNumber, MAX_SERIAL_NUMBER_LEN);

        if (config->serial_num[0])
        {
            if (!strncmp(config->serial_num, device_node.SerialNumber, strlen(config->serial_num)))
                ftdi_spi_set_spi_and_reset_chan(device_node.LocId, ii, &spi_chan_info);
        }
        else
        {
            ftdi_spi_set_spi_and_reset_chan(device_node.LocId, ii, &spi_chan_info);
        }

        if (transport->debug)
        {
            mctrl_print("Information on channel number %d:\n", ii + 1);
            /* print the dev info */
            mctrl_print("  Flags        = 0x%08x\n", device_node.Flags);
            mctrl_print("  Type         = 0x%08x\n", device_node.Type);
            mctrl_print("  ID           = 0x%08x\n", device_node.ID);
            mctrl_print("  LocId        = 0x%08x\n", device_node.LocId);
            mctrl_print("  SerialNumber = %s\n", device_node.SerialNumber);
            mctrl_print("  Description  = %s\n", device_node.Description);
            mctrl_print("  ftHandle     = %p\n", device_node.ftHandle);
        }
    }

    if (transport->debug)
    {
        mctrl_print("SPI, reset on channels: %u, %u\n", spi_chan_info.spi_loc_id_ch,
                                                        spi_chan_info.reset_loc_id_ch);
    }

    if (spi_chan_info.spi_loc_id_ch == UINT32_MAX || (spi_chan_info.reset_loc_id_ch == UINT32_MAX))
    {
        if ((config->serial_num[0]))
        {
            mctrl_print("Serial number %s not valid. Avalable serial numbers:\n",
                        config->serial_num);
            for (int i = 0; i < MORSE_ARRAY_SIZE(serial_num_list); i++)
            {
                if (serial_num_list[i][0])
                    mctrl_print("\tchannel %d - %s\n", i, serial_num_list[i]);
            }
        }
        return -ETRANSFTDISPIERR;
    }

    status = SPI_OpenChannel(spi_chan_info.spi_loc_id_ch, &state->handle);
    if (status != FT_OK)
    {
        ftdi_spi_error(status, "Failed to open MPSSE SPI channel");
        return -ETRANSFTDISPIERR;
    }

    status = SPI_InitChannel(state->handle, &config->channel);
    if (status != FT_OK)
    {
        ftdi_spi_error(status, "Failed to init MPSSE SPI channel");
        return -ETRANSFTDISPIERR;
    }

    status = SPI_OpenChannel(spi_chan_info.reset_loc_id_ch, &state->reset_handle);
    if (status != FT_OK)
    {
        ftdi_spi_error(status, "Failed to open MPSSE reset channel");
        return -ETRANSFTDISPIERR;
    }

    status = SPI_InitChannel(state->reset_handle, &config->channel);
    if (status != FT_OK)
    {
        ftdi_spi_error(status, "Failed to init MPSSE reset channel");
        return -ETRANSFTDISPIERR;
    }

    /* Set all GPIO as High Inputs except for reset pins (High Outputs). */
    FT_WriteGPIOL(state->reset_handle,
                  config->jtag_reset_pin_num | config->reset_pin_num,
                  FTDI_SPI_GPIOL_MASK);

    return ETRANSSUCC;
}

/**
 * @brief De-initalise an FTDI Transport.
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_deinit(struct morsectrl_transport *transport)
{
    struct morsectrl_ftdi_spi_state *state = ftdi_spi_state(transport);
    int ret = ETRANSSUCC;

    SPI_CloseChannel(state->handle);
    SPI_CloseChannel(state->reset_handle);
    Cleanup_libMPSSE();

    return ret;
}

/**
 * @brief Allocate @ref morsectrl_transport_buff.
 *
 * @param transport Transport structure.
 * @param size      Size of command and morse headers or raw data.
 * @return          Allocated @ref morsectrl_transport_buff or NULL on failure.
 */
static struct morsectrl_transport_buff *ftdi_spi_alloc(struct morsectrl_transport *transport,
                                                       size_t size)
{
    struct morsectrl_transport_buff *buff;
    size_t aligned_size;

    if (!transport)
        return NULL;

    if (size <= 0)
        return NULL;

    /* Alignment to word boundaries. */
    aligned_size = align_size(size, sizeof(uint32_t));

    buff = malloc(sizeof(*buff));
    if (!buff)
        return NULL;

    buff->capacity = aligned_size;
    buff->memblock = (uint8_t *)malloc(buff->capacity);
    if (!buff->memblock)
    {
        free(buff);
        return NULL;
    }
    buff->data = buff->memblock;
    buff->data_len = size;
    memset(&buff->data[size], FTDI_SPI_JUNK_OCTET, aligned_size - size);

    return buff;
}

/**
 * @brief Allocate @ref morsectrl_transport_buff that is used for writing data over the transport.
 *
 * @param transport
 * @param size
 * @return struct morsectrl_transport_buff*
 */
static struct morsectrl_transport_buff *ftdi_spi_write_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    return ftdi_spi_alloc(transport, size);
}

static struct morsectrl_transport_buff *ftdi_spi_read_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    return ftdi_spi_alloc(transport, size);
}

/**
 * @brief Read a 32bit register.
 *
 * @note It can also be a word aligned 32bit memory value.
 *
 * @param transport The transport structure.
 * @param addr      The address to read from. Must be word aligned.
 * @param value     Pointer to memory to store the result.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_reg_read(struct morsectrl_transport *transport,
                             uint32_t addr, uint32_t *value)
{
    return sdio_over_spi_read_reg_32bit(transport, addr, value);
}

/**
 * @brief Write a 32bit register.
 *
 * @note It can also be a word aligned 32bit memory value.
 *
 * @param transport The transport structure.
 * @param addr      The address to write to. Must be word aligned.
 * @param value     Value to write.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_reg_write(struct morsectrl_transport *transport,
                              uint32_t addr, uint32_t value)
{
    return sdio_over_spi_write_reg_32bit(transport, addr, value);
}

/**
 * @brief Read a block of memory.
 *
 * @note Memory must be word aligned.
 *
 * @param transport The transport structure.
 * @param read      Buffer to read data into.
 * @param addr      The address to read from. Must be word aligned.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_mem_read(struct morsectrl_transport *transport,
                             struct morsectrl_transport_buff *read,
                             uint32_t addr)
{
    return sdio_over_spi_read_memblock(transport, read, addr);
}

/**
 * @brief Write a block of memory.
 *
 * @note Memory must be word aligned.
 *
 * @param transport The transport structure.
 * @param write     Buffer to write data from.
 * @param addr      The address to write to. Must be word aligned.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_mem_write(struct morsectrl_transport *transport,
                              struct morsectrl_transport_buff *write,
                              uint32_t addr)
{
    return sdio_over_spi_write_memblock(transport, write, addr);
}

/**
 * @brief Set the state of the chip select line.
 *
 * @param transport The transport structure.
 * @param assert    If true assert CS, otherwise de-assert.
 * @return          0 on success otherwise relevant error.
 */
static int ftdi_spi_set_cs(struct morsectrl_transport *transport, bool assert)
{
    FT_HANDLE handle =  ftdi_spi_state(transport)->handle;
    FT_STATUS status;

    /* There's no delay so apply final state. */
    if (assert)
        status = SPI_ToggleCS(handle, true);
    else
        status = SPI_ToggleCS(handle, false);

    if (status)
    {
        ftdi_spi_error(status, "Failed to set CS");
        return -ETRANSFTDISPIERR;
    }

    return ETRANSSUCC;
}

/**
 * @brief Read data from the FTDI SPI device.
 *
 * @param transport The transport structure.
 * @param read      Buffer to read data into.
 * @param start     Whether to assert CS before data transmission.
 * @param finish    Whether to de-assert CS after data transmission.
 * @return          0 on success or relevant error.
 */
static int ftdi_spi_raw_read(struct morsectrl_transport *transport,
                             struct morsectrl_transport_buff *read,
                             bool start,
                             bool finish)
{
    DWORD options = 0;
    FT_STATUS status;
    DWORD size_transferred;

    if (read == NULL)
    {
        if (finish)
            return ftdi_spi_set_cs(transport, false);
        else if (start)
            return ftdi_spi_set_cs(transport, true);

        ftdi_spi_error(-ETRANSFTDISPIERR, "Empty SPI read");
        return -ETRANSFTDISPIERR;
    }

    if (start)
        options |= FTDI_SPI_OPTS_CS_START;
    if (finish)
        options |= FTDI_SPI_OPTS_CS_FINISH;

    status = SPI_Read(ftdi_spi_state(transport)->handle,
                      read->data, read->data_len,
                      &size_transferred, options);

    if (status)
    {
        ftdi_spi_error(status, "Failed to raw read");
        return -ETRANSFTDISPIERR;
    }

    if (size_transferred != read->data_len)
    {
        ftdi_spi_error(status, "Raw read size mismatch");
        return -ETRANSFTDISPIERR;
    }

    return ETRANSSUCC;
}

/**
 * @brief Write data to the FTDI SPI device.
 *
 * @param transport The transport structure.
 * @param write     Buffer to write data from.
 * @param start     Whether to assert CS before data transmission.
 * @param finish    Whether to de-assert CS after data transmission.
 * @return          0 on success or relevant error.
 */
int ftdi_spi_raw_write(struct morsectrl_transport *transport,
                       struct morsectrl_transport_buff *write,
                       bool start,
                       bool finish)
{
    DWORD options = 0;
    FT_STATUS status;
    DWORD size_transferred;

    if (write == NULL)
    {
        if (finish)
            return ftdi_spi_set_cs(transport, false);
        else if (start)
            return ftdi_spi_set_cs(transport, true);

        ftdi_spi_error(-ETRANSFTDISPIERR, "Empty SPI read");
        return -ETRANSFTDISPIERR;
    }

    if (start)
        options |= FTDI_SPI_OPTS_CS_START;
    if (finish)
        options |= FTDI_SPI_OPTS_CS_FINISH;

    status = SPI_Write(ftdi_spi_state(transport)->handle,
                       write->data, write->data_len,
                       &size_transferred, options);

    if (status)
    {
        ftdi_spi_error(status, "Failed to raw write");
        return -ETRANSFTDISPIERR;
    }

    if (size_transferred != write->data_len)
    {
        ftdi_spi_error(status, "Raw write size mismatch");
        return -ETRANSFTDISPIERR;
    }

    return ETRANSSUCC;
}

/**
 * @brief Write data to the FTDI SPI device and read at the same time.
 *
 * @param transport The transport structure.
 * @param read      Buffer to read data into.
 * @param write     Buffer to write data from.
 * @param start     Whether to assert CS before data transmission.
 * @param finish    Whether to de-assert CS after data transmission.
 * @return          0 on success or relevant error.
 */
int ftdi_spi_raw_read_write(struct morsectrl_transport *transport,
                            struct morsectrl_transport_buff *read,
                            struct morsectrl_transport_buff *write,
                            bool start,
                            bool finish)
{
    DWORD options = 0;
    FT_STATUS status;
    DWORD size_transferred;
    /* If we want to do an asymmetric read set finish to false and use another raw function. */
    DWORD transfer_size;

    if (!read || !write)
    {
        ftdi_spi_error(0, "Raw read/write missing buffer");
        return -ETRANSFTDISPIERR;
    }

    transfer_size = MIN(read->data_len, write->data_len);

    if (start)
        options |= FTDI_SPI_OPTS_CS_START;
    if (finish)
        options |= FTDI_SPI_OPTS_CS_FINISH;

    status = SPI_ReadWrite(ftdi_spi_state(transport)->handle,
                           read->data, write->data, transfer_size,
                           &size_transferred, options);

    if (status)
    {
        ftdi_spi_error(status, "Failed to raw read/write");
        return -ETRANSFTDISPIERR;
    }

    if (size_transferred != transfer_size)
    {
        ftdi_spi_error(status, "Raw read/write size mismatch");
        return -ETRANSFTDISPIERR;
    }

    return ETRANSSUCC;
} /* NOLINT */

static int ftdi_spi_send(struct morsectrl_transport *transport,
                         struct morsectrl_transport_buff *cmd,
                         struct morsectrl_transport_buff *resp)
{
    const struct morsectrl_transport_ops *tops;
    struct response *response;
    uint32_t host_table_ptr;
    uint32_t cmd_addr;
    uint32_t resp_addr;
    uint32_t status;
    int ii;
    int ret;

    if (!transport || !transport->tops ||
        !transport->tops->reg_read || !transport->tops->reg_write ||
        !transport->tops->mem_read || !transport->tops->mem_write ||
        !cmd || !resp)
    {
        return -ETRANSFTDISPIERR;
    }

    tops = transport->tops;

    /* Locate command and response memory locations. */
    ret = tops->reg_read(transport, MM_MANIFEST_ADDR, &host_table_ptr);
    if (ret)
        goto fail;
    if (transport->debug)
        mctrl_print("\nHost table ptr: 0x%08x\n\n", host_table_ptr);

    ret = tops->reg_read(transport, host_table_ptr + MM_CMD_ADDR_OFFSET, &cmd_addr);
    /* For production firmware which doesn't support memcmd, the address supplied to write commands
     * is 0.
     */
    if (ret)
    {
        goto fail;
    }
    if (transport->debug)
    {
        mctrl_print("\nCommand addr: 0x%08x\n\n", cmd_addr);
    }
    if (!cmd_addr)
    {
        ftdi_spi_error(cmd_addr, "This transport is not supported for production firmware");
        return -ETRANSFTDISPIERR;
    }

    ret = tops->reg_read(transport, host_table_ptr + MM_RESP_ADDR_OFFSET, &resp_addr);
    if (ret)
    {
        goto fail;
    }
    if (transport->debug)
    {
        mctrl_print("\nResponse addr: 0x%08x\n\n", resp_addr);
    }

    ret = tops->reg_write(transport, MM_STATUS_CLR_ADDR, MM_CMD_MASK);
    if (ret)
    {
        goto fail;
    }
    if (transport->debug)
    {
        mctrl_print("\nCleared status\n\n");
    }

    ret = tops->mem_write(transport, cmd, cmd_addr);
    if (ret)
    {
        goto fail;
    }
    if (transport->debug)
    {
        mctrl_print("\nWrote command\n\n");
    }

    ret = tops->reg_write(transport, MM_TRIGGER_ADDR, MM_CMD_MASK);
    if (ret)
    {
        goto fail;
    }
    if (transport->debug)
    {
        mctrl_print("\nTriggered command\n\n");
    }

    /* Poll for reponse. */
    for (ii = 0; ii < RESP_TIMEOUT_MS; ii += RESP_POLL_INTERVAL_MS)
    {
        ret = tops->reg_read(transport, MM_STATUS_ADDR, &status);
        if (ret)
            goto fail;

        if (transport->debug)
            mctrl_print("\nStatus: 0x%08x\n\n", status);

        if (status & MM_CMD_MASK)
        {
            break;
        }
        sleep_ms(RESP_POLL_INTERVAL_MS);
    }

    if (ii >= RESP_TIMEOUT_MS)
    {
        goto fail;
    }

    /* Read in response. */
    ret = tops->mem_read(transport, resp, resp_addr);
    if (ret)
    {
        goto fail;
    }
    if (transport->debug)
    {
        mctrl_print("\nRead response\n\n");
    }

    /* Clear status. */
    tops->reg_write(transport, MM_STATUS_CLR_ADDR, MM_CMD_MASK);
    if (transport->debug)
    {
        mctrl_print("\nCleared status\n\n");
    }

    /* Trim response length (required for variable length responses). */
    response = (struct response *)resp->data;
    /* Size the response buffer to the size of the data, status rc, and cmd hdr */
    resp->data_len = response->hdr.len + sizeof(response->hdr);

    return ETRANSSUCC;

fail:
    ftdi_spi_error(ret, "Failed to send command");
    return ret;
}


static int ftdi_spi_reset(struct morsectrl_transport *transport)
{
    struct morsectrl_transport_buff *buff =
        morsectrl_transport_raw_write_alloc(transport, MM_OCTETS_OF_INIT_CLK);
    struct morsectrl_ftdi_spi_state *state = ftdi_spi_state(transport);
    struct morsectrl_ftdi_spi_cfg *config = ftdi_spi_cfg(transport);
    ChannelConfig *channel_cfg = NULL;
    FT_STATUS status;
    UCHAR value;
    UCHAR dir;
    int ret;

    if (!buff)
    {
        return -ETRANSFTDISPIERR;
    }

    status = SPI_GetChannelConfig(state->reset_handle, &channel_cfg);
    if (status != FT_OK)
    {
        morsectrl_transport_buff_free(buff);
        ftdi_spi_error(status, "Failed to get channel config during GPIO reset");
        return -ETRANSFTDISPIERR;
    }

    value = FTDI_SPI_PINSTATE_TO_VAL(channel_cfg->currentPinState);
    dir = FTDI_SPI_PINSTATE_TO_DIR(channel_cfg->currentPinState);
    value &= ~(config->reset_pin_num | config->jtag_reset_pin_num);
    dir |= (config->reset_pin_num | config->jtag_reset_pin_num);
    status = FT_WriteGPIOL(state->reset_handle, dir, value);
    if (status != FT_OK)
    {
        morsectrl_transport_buff_free(buff);
        ftdi_spi_error(status, "Failed to write GPIO reset line");
        return -ETRANSFTDISPIERR;
    }
    status = ftdi_spi_set_cs(transport, false);
    if (status != FT_OK)
    {
        morsectrl_transport_buff_free(buff);
        return -ETRANSFTDISPIERR;
    }

    sleep_ms(config->reset_ms);
    value |= config->reset_pin_num;
    status = FT_WriteGPIOL(state->reset_handle, dir, value);
    if (status != FT_OK)
    {
        morsectrl_transport_buff_free(buff);
        ftdi_spi_error(status, "Failed to write GPIO reset line");
        return -ETRANSFTDISPIERR;
    }

    /* De-assert JTAG-reset after reset. */
    sleep_ms(config->reset_ms);
    value |= config->jtag_reset_pin_num;
    status = FT_WriteGPIOL(state->reset_handle, dir, value);
    if (status != FT_OK)
    {
        morsectrl_transport_buff_free(buff);
        ftdi_spi_error(status, "Failed to write GPIO JTAG reset line");
        return -ETRANSFTDISPIERR;
    }

    /* Writing of GPIO doesn't seem to block and reset does some weird stuff. */
    sleep_ms(config->reset_ms);

    /* Put CS into correct state (deasserted). */
    ftdi_spi_set_cs(transport, false);

    /* Put some clock cycles into the SPI port. (Data can be garbage). */
    memset(buff->data, 0xFF, buff->data_len);
    transport->tops->raw_write(transport, buff, false, false);

    morsectrl_transport_buff_free(buff);

    /* Use this to send the CMD63 to enter SPI mode. */
    ret = sdio_over_spi_post_hard_reset(transport);

    return ret;
}

static const struct morsectrl_transport_ops ftdi_spi_ops = {
    .name = "ftdi_spi",
    .description = "FTDI SPI interface direct to transceiver",
    .has_reset = true,
    .has_driver = false,
    .parse = ftdi_spi_parse,
    .init = ftdi_spi_init,
    .deinit = ftdi_spi_deinit,
    .write_alloc = ftdi_spi_write_alloc,
    .read_alloc = ftdi_spi_read_alloc,
    .send = ftdi_spi_send,
    .reg_read = ftdi_spi_reg_read,
    .reg_write = ftdi_spi_reg_write,
    .mem_read = ftdi_spi_mem_read,
    .mem_write = ftdi_spi_mem_write,
    .raw_read = ftdi_spi_raw_read,
    .raw_write = ftdi_spi_raw_write,
    .raw_read_write = ftdi_spi_raw_read_write,
    .reset_device = ftdi_spi_reset,
    .get_ifname = NULL,
};

REGISTER_TRANSPORT(ftdi_spi_ops);
