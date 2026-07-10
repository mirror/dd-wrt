/*
 * Copyright 2020-2026 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>

#include "stats_decode.h"
#include "command.h"
#include "transport/transport.h"
#include "mm_argtable3.h"

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

static struct
{
    struct arg_lit *apps_core;
    struct arg_lit *mac_core;
    struct arg_lit *phy_core;
    struct arg_lit *reset;
    struct arg_lit *json_format;
    struct arg_lit *pprint_format;
    struct arg_str *filter_str;
    struct arg_str *firmware_path;
    struct arg_str *statsdump_path;
    struct arg_str *stats_decode_path;
} args;

/* Read and return a single word from a file.
 * The result is always null terminated.
 */
static char *get_word_from_file(const char *path, char *word, size_t n)
{
    FILE *infile = fopen(path, "r");
    if (infile)
    {
        fgets(word, n, infile); /* fgets always null terminates */
        fclose(infile);
        return strip(word);
    }
    return NULL;
}

/* Get the firmware path from the driver sysfs file
 *
 * If the file doesn't exist, attempt to find the file in its old location, in debugfs
 */
static void get_override_firmware_path(struct morsectrl *mors, char *firmware_full_path, size_t n)
{
    char path_buf[MAX_PATH];
    char content_buf[MAX_PATH];
    const char *phy_name_path_fmt = "/sys/class/net/%s/phy80211/name";
    const char *sysfs_path_fmt = "/sys/class/ieee80211/%s/device/firmware_path";
    const char *debugfs_path_fmt = "/sys/kernel/debug/ieee80211/%s/morse/firmware_path";
#ifndef CONFIG_ANDROID
    const char *firmware_path_fmt = "/lib/firmware/%s";
#else
    const char *firmware_path_fmt = "/vendor/firmware/%s";
#endif

    const char *interface_name;
    char *phy_name;
    char phy_name_buf[64];
    char *firmware_path;

    /* Get wlan interface name*/
    interface_name = morsectrl_transport_get_ifname(mors->transport);
    if (!interface_name)
    {
        interface_name = DEFAULT_INTERFACE_NAME;
    }

    /* Find phy interface name */
    snprintf(path_buf, sizeof(path_buf), phy_name_path_fmt, interface_name);
    phy_name = get_word_from_file(path_buf, phy_name_buf, sizeof(phy_name_buf));
    if (!phy_name)
        return;

    /* Substitute phy name into format and find firmware_path in sysfs */
    snprintf(path_buf, sizeof(path_buf), sysfs_path_fmt, phy_name);
    firmware_path = get_word_from_file(path_buf, content_buf, sizeof(content_buf));

    /* If the firmware_path doesn't exist in sysfs, attempt to find it in debugfs (old
     * behaviour)
     */
    if (!firmware_path) {
        /* Substitute phy name into format and find firmware_path in debugfs */
        snprintf(path_buf, sizeof(path_buf), debugfs_path_fmt, phy_name);
        firmware_path = get_word_from_file(path_buf, content_buf, sizeof(content_buf));
        if (!firmware_path)
            return;
    }

    /* Copy full firmware path to output */
    snprintf(firmware_full_path, n, firmware_path_fmt, firmware_path);
}


int morsectrl_stats_cmd(struct morsectrl *mors, int cmd, int reset, enum format_type format_val)
{
    int ret = -1;
    int resp_sz;
    struct stats_response *resp;
    struct morsectrl_transport_buff *cmd_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, 0);
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp));

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    resp = TBUFF_TO_RSP(rsp_tbuff, struct stats_response);

    if (reset)
        cmd += 1;

    ret = morsectrl_send_command(mors->transport, cmd, cmd_tbuff, rsp_tbuff);
    resp_sz = rsp_tbuff->data_len - sizeof(struct response);

    if (ret)
    {
        /* Try the deprecated command */
        ret = morsectrl_send_command(mors->transport, OLD_STATS_COMMAND_MASK & cmd,
                                     cmd_tbuff, rsp_tbuff);
        resp_sz = rsp_tbuff->data_len - sizeof(struct response);
        if (!reset && !ret)
        {
            mctrl_print("%s", resp->stats);
        }
        goto exit;
    }

    if (!reset && !ret)
    {
        ret = statistics_data_decode(mors, format_val, (uint8_t *)resp->stats, (size_t) resp_sz);
        if (ret)
        {
            mctrl_err("%d decode errors\n", ret);
        }
    }
exit:
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

#define ARGUMENTS_COMMON \
     args.json_format = arg_lit0("j", "json", "Format the statistics as JSON"), \
     args.pprint_format = arg_lit0("p", "pretty", "Format the statistics as human-readable JSON"), \
     args.filter_str = arg_str0("f", "filter", "<filter>", stats_filter_help()), \
     args.firmware_path = arg_str0("s", "firmware", "<firmware file>", \
                  "Path to the firmware to use to decode the statistics"), \
     args.statsdump_path = arg_str0("x", "statsdump", "<statistics file>", \
                  "Read statistics from a file of hex strings"), \
     args.stats_decode_path = arg_str0("o", "output", "<statistics decode output file>", \
                  "Path to file where statistics decode should be written " \
                  "(default is to console)")

int stats_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    if (strcmp(morsectrl_transport_name(mors->transport), "offline") == 0)
    {
        MM_INIT_ARGTABLE(mm_args, "Decode statistics from file",
                         ARGUMENTS_COMMON);
        /* Force some stats decode options to be mandatory if not talking to a chip */
        args.firmware_path->hdr.mincount = 1;
        args.statsdump_path->hdr.mincount = 1;
    }
    else
    {
        MM_INIT_ARGTABLE(mm_args, "Read statistics from the chip",
                         args.apps_core = arg_lit0("a", NULL, "read statistics from the Apps core"),
                         args.mac_core  = arg_lit0("m", NULL, "read statistics from the MAC core"),
                         args.phy_core  = arg_lit0("u", NULL, "read statistics from the PHY core"),
                         args.reset     = arg_lit0("r", NULL, "reset the statistics"),
                         ARGUMENTS_COMMON);
    }
    return 0;
}

int stats(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = 0;
    bool reset = false, app_c = false, mac_c = false, uph_c = false;
    enum format_type format;
    bool offline = false;
    FILE *decode_output_file = NULL;

#ifndef CONFIG_ANDROID
    char firmware_default_path[MAX_PATH] = "/lib/firmware/morse/mm6108.bin";
#else
    char firmware_default_path[MAX_PATH] = "/vendor/firmware/morse/mm6108.bin";
#endif
    const char *statistics_definitions_path;

    if (args.firmware_path->count == 0)
    {
        get_override_firmware_path(mors, firmware_default_path, sizeof(firmware_default_path));
        statistics_definitions_path = firmware_default_path;
    }
    else
    {
        statistics_definitions_path = args.firmware_path->sval[0];
    }

    ret = load_offchip_statistics_definitions(mors, statistics_definitions_path);

    if (ret)
    {
        if (args.firmware_path->count == 0)
        {
            mm_print_missing_argument(&args.firmware_path->hdr);
        }
        goto exit_stats;
    }

    if (mors->debug)
    {
        statistics_types_dump(mors);
    }

    if (strcmp(morsectrl_transport_name(mors->transport), "offline") == 0)
    {
        offline = true;
    }

    if (!offline)
    {
        reset = !!(args.reset->count);
        app_c = (bool)(args.apps_core->count > 0);
        mac_c = (bool)(args.mac_core->count > 0);
        uph_c = (bool)(args.phy_core->count > 0);

        /* If no core selected then enable all. */
        if ((!app_c) && (!mac_c) && (!uph_c))
        {
            app_c = true;
            mac_c = true;
            uph_c = true;
        }
    }

    if (args.pprint_format->count > 0)
    {
        format = FORMAT_JSON_PPRINT;
    }
    else if (args.json_format->count > 0)
    {
        format = FORMAT_JSON;
    }
    else
    {
        format = FORMAT_REGULAR;
    }

    if (args.filter_str->count > 0)
    {
        /* Initialise the filter */
        ret = stats_filter_init(args.filter_str->sval[0]);

        if (ret)
        {
            goto exit_stats;
        }
    }

    if (args.stats_decode_path->count > 0)
    {
        /* Output to file */
        decode_output_file = fopen(args.stats_decode_path->sval[0], "w");
        if (decode_output_file != NULL)
        {
            mctrl_print("Output to '%s'\n", args.stats_decode_path->sval[0]);
            mctrl_print_set_stream(decode_output_file);
        }
        else
        {
            mctrl_print("Failed to output to '%s': %s\n",
                        args.stats_decode_path->sval[0], strerror(errno));
            goto exit_stats;
        }
    }

    /* Start the stats output formatting */
    stats_format_start(format);

    if (!offline && (args.statsdump_path->count == 0))
    {
        if (app_c)
        {
            ret = morsectrl_stats_cmd(mors, MORSE_CMD_ID_HOST_STATS_LOG, reset, format);
            if (ret) goto exit_stats;
        }
        if (mac_c)
        {
            ret = morsectrl_stats_cmd(mors, MORSE_CMD_ID_MAC_STATS_LOG, reset, format);
            if (ret) goto exit_stats;
        }
        if (uph_c)
        {
            ret = morsectrl_stats_cmd(mors, MORSE_CMD_ID_UPHY_STATS_LOG, reset, format);
            if (ret) goto exit_stats;
        }
    }
    else
    {
        uint8_t *external_stats = NULL;
        size_t external_stats_size = 0;

        if (args.statsdump_path->count == 0)
        {
            mctrl_err("Offline mode requires a file containing a statistics dump.\n");
            mm_print_missing_argument(&args.statsdump_path->hdr);
            ret = -1;
            goto exit_stats;
        }

        if (format == FORMAT_REGULAR)
        {
            mctrl_print("Decode '%s' using '%s'\n",
                        args.statsdump_path->sval[0], statistics_definitions_path);
        }

        ret = load_offchip_statistics_data(args.statsdump_path->sval[0],
                                  &external_stats, &external_stats_size);
        if (ret == 0)
        {
            if (format == FORMAT_REGULAR)
            {
                mctrl_print("Decoding %zu bytes of statistics data\n",
                    external_stats_size);
            }

            ret = statistics_data_decode(mors, format, external_stats, external_stats_size);
        }
        else
        {
            if (args.stats_decode_path->count > 0)
            {
                mctrl_err("Failed to decode '%s' - see '%s' for detail\n",
                          args.statsdump_path->sval[0],
                          args.stats_decode_path->sval[0]);
            }
        }

        if (external_stats != NULL)
        {
            free(external_stats);
        }
    }

    stats_format_finish(format);

exit_stats:

    if ((args.stats_decode_path->count > 0) && (decode_output_file != NULL))
    {
        /* Close the output file */
        if (decode_output_file != NULL)
        {
            mctrl_print_set_stream(stdout);
            fclose(decode_output_file);
        }
    }
    else
    {
        /* Make a bit of space on the console following the output */
        mctrl_print("\n");
    }

    /* Free the filter */
    if (stats_filter_is_set())
    {
        stats_filter_deinit();
    }

    free_offchip_statistics_definitions(mors);

    return ret;
}

MM_CLI_HANDLER(stats, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
