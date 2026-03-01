/*
 * Copyright 2020 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#ifndef MORSE_WIN_BUILD
#include <regex.h>
#endif

#include "portable_endian.h"
#include "command.h"
#include "elf_file.h"
#include "offchip_statistics.h"
#include "stats_format.h"
#include "utilities.h"
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

/* Get the firmware path from the driver debugfs file
 * ...using phy name from sysfs file
 * ...using wlan name from morsectrl struct
 * The append it to the firmware path.
 *
 * If any of these steps fail, nothing is written to the firmware path.
 */
static void get_override_firmware_path(struct morsectrl *mors, char *firmware_full_path, size_t n)
{
    char path_buf[MAX_PATH];
    char content_buf[MAX_PATH];
    const char *sysfs_path_fmt = "/sys/class/net/%s/phy80211/name";
    const char *debugfs_path_fmt = "/sys/kernel/debug/ieee80211/%s/morse/firmware_path";
#ifndef CONFIG_ANDROID
    const char *firmware_path_fmt = "/lib/firmware/%s";
#else
    const char *firmware_path_fmt = "/vendor/firmware/%s";
#endif

    const char *interface_name;
    char *phy_name;
    char *firmware_path;

    interface_name = morsectrl_transport_get_ifname(mors->transport);
    if (!interface_name)
    {
        interface_name = DEFAULT_INTERFACE_NAME;
    }
    snprintf(path_buf, sizeof(path_buf), sysfs_path_fmt, interface_name);

    phy_name = get_word_from_file(path_buf, content_buf, sizeof(content_buf));
    if (phy_name)
    {
        snprintf(path_buf, sizeof(path_buf), debugfs_path_fmt, phy_name);
        firmware_path = get_word_from_file(path_buf, content_buf, sizeof(content_buf));
        if (firmware_path)
        {
            snprintf(firmware_full_path, n, firmware_path_fmt, firmware_path);
        }
    }
}

static int load_offchip_statistics(struct morsectrl *mors, const char *filename)
{
    FILE *infile;
#ifndef CONFIG_ANDROID
    char firmware_path[MAX_PATH] = "/lib/firmware/morse/mm6108.bin";
#else
    char firmware_path[MAX_PATH] = "/vendor/firmware/morse/mm6108.bin";
#endif

    if (!filename)
    {
        filename = firmware_path;
        get_override_firmware_path(mors, firmware_path, sizeof(firmware_path));
    }

    infile = fopen(filename, "rb");
    if (infile)
    {
        uint8_t *buf = NULL;

        load_file(infile, &buf);
        if (buf)
        {
            morse_stats_load(&mors->stats, &mors->n_stats, buf);
            free(buf);
        }
        fclose(infile);
    }
    else
    {
        mctrl_err("Error - could not open %s to read stats metadata\n", filename);
        return -1;
    }
    return 0;
}


#ifndef MORSE_WIN_BUILD
static regex_t *filter_re = NULL;

static int filter_init(const char *filter_string)
{
    int ret = 0;

#ifdef CONFIG_ANDROID
    /* In android, regcomp returns an error "empty (sub)expression" as empty string is
     * not considered as valid regex.
     */
    if (filter_string[0] == '\0')
    filter_string = "\\(\\)";
#endif

    filter_re = malloc(sizeof(*filter_re));
    ret = regcomp(filter_re, filter_string, 0);

    if (ret)
    {
        size_t len = regerror(ret, filter_re, NULL, 0);
        char *re_err_buf = malloc(len);
        regerror(ret, filter_re, re_err_buf, len);
        mctrl_err("Invalid filter string: %s\n", re_err_buf);
        free(re_err_buf);
        free(filter_re);
        filter_re = NULL;
    }

    return ret;
}

static int filter_stat(const char *key)
{
    return regexec(filter_re, key, 0, NULL, 0);
}

static void filter_deinit(void)
{
    if (filter_re)
    {
        regfree(filter_re);
        free(filter_re);
    }
    filter_re = NULL;
}

static const char *filter_help(void)
{
    return "uses a regular expression";
}
#else
static const char *filter_str = NULL;

static int filter_init(const char *filter_string)
{
    filter_str = filter_string;

    return 0;
}

static int filter_stat(const char *key)
{
    return strcmp(key, filter_str);
}

static void filter_deinit(void)
{
    filter_str = NULL;
}

static const char *filter_help(void)
{
    return "case sensitive, match from start of key";
}
#endif

int morsectrl_stats_cmd(struct morsectrl *mors, int cmd, int reset,
                            const char *filter_string, enum format_type format_val)
{
    int ret = -1;
    int resp_sz;
    const struct format_table *table;
    struct stats_response *resp;
    struct morsectrl_transport_buff *cmd_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, 0);
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp));

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    if (filter_string)
    {
        ret = filter_init(filter_string);

        if (ret)
            goto exit;
    }

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
        uint8_t *buf = (uint8_t *)resp->stats;

        switch (format_val)
        {
            case FORMAT_REGULAR:
            {
                table = stats_format_regular_get_formatter_table();
                break;
            }
            case FORMAT_JSON_PPRINT:
            {
                stats_format_json_set_pprint(true);
                /* fall through */
            }
            case FORMAT_JSON:
            {
                table = stats_format_json_get_formatter_table();
                break;
            }
            default:
                ret = -1;
                goto exit;
        }

        while (resp_sz > STATS_TLV_OVERHEAD )
        {
            stats_tlv_tag_t tag =  *((stats_tlv_tag_t *)buf);
            buf += sizeof(stats_tlv_tag_t);

            stats_tlv_len_t len =  le16toh(*((__force __le16 *)buf));
            buf += sizeof(stats_tlv_len_t);

            if ((len > resp_sz) || (len == 0))
            {
                mctrl_err("error: malformed TLV (tag %d/0x%x, len %u/0x%x, size %u)\n",
                        tag, tag, len, len, resp_sz);
                break;
            }

            struct statistics_offchip_data *offchip = get_stats_offchip(mors, tag);
            if (offchip)
            {
                uint32_t format = le32toh((__force __le32)offchip->format);
                if ((format == MORSE_STATS_FMT_DEC) &&
                        !strncmp(offchip->type_str, "uint", 4))
                {
                    format = MORSE_STATS_FMT_U_DEC;
                }

                if (!filter_string || !filter_stat(offchip->key))
                {
                    if (format_val == FORMAT_JSON || format_val == FORMAT_JSON_PPRINT)
                    {
                        stats_format_json_init();
                    }

                    if (format > MORSE_STATS_FMT_LAST)
                    {
                        format = MORSE_STATS_FMT_LAST;
                    }
                    table->format_func[format]((const char *) offchip->key,
                                                                (const uint8_t *)buf, len);
                }
            }
            else
            {
                mctrl_err("UNKOWN KEY for tag %d: ", tag);
                hexdump(buf, len);
                mctrl_err("\n");
            }
            buf += len;

            resp_sz -= (STATS_TLV_OVERHEAD + len);
        }
    }
exit:
    filter_deinit();
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

static void dump_stats_types(struct morsectrl *mors)
{
    int ii;

    mctrl_print("Stats types\n");
    for (ii = 0; ii < mors->n_stats; ii++)
    {
        mctrl_print("Type: %s\n", mors->stats[ii].type_str);
        mctrl_print("Name: %s\n", mors->stats[ii].name);
        mctrl_print("Key: %s\n\n", mors->stats[ii].key);
    }
}

int stats_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Read statistics from the chip",
                     args.apps_core = arg_lit0("a", NULL, "read statistics from the Apps core"),
                     args.mac_core = arg_lit0("m", NULL, "read statistics from the MAC core"),
                     args.phy_core = arg_lit0("u", NULL, "read statistics from the PHY core"),
                     args.reset = arg_lit0("r", NULL, "reset the statistics"),
                     args.json_format = arg_lit0("j", "json", "Format the statistics in JSON"),
                     args.pprint_format = arg_lit0("p", NULL,
                         "Format the statistics in human-readable JSON"),
                     args.filter_str = arg_str0("f", "filter", "<filter>", filter_help()),
                     args.firmware_path =
                         arg_str0("s", "firmware",
                                  "<firmware>",
                                  "Path to the firmware used to process the statistics"));
    return 0;
}

int stats(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = 0;
    bool reset = false, app_c = false, mac_c = false, uph_c = false;
    enum format_type format = FORMAT_REGULAR;

    ret = load_offchip_statistics(mors, args.firmware_path->count >
                                  0 ? args.firmware_path->sval[0] : NULL);

    if (ret)
    {
        goto exit_stats;
    }

    if (mors->debug)
        dump_stats_types(mors);

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

    if (args.json_format->count > 0)
        format = FORMAT_JSON;
    else if (args.pprint_format->count > 0)
        format = FORMAT_JSON_PPRINT;

    if (format == FORMAT_JSON)
    {
        mctrl_print("{");
    }
    else if (format == FORMAT_JSON_PPRINT)
    {
        mctrl_print("{\n");
    }

    reset = !!(args.reset->count);

    if (app_c)
    {
        ret = morsectrl_stats_cmd(mors, MORSE_CMD_ID_HOST_STATS_LOG, reset,
            args.filter_str->sval[0], format);
        if (ret) goto exit_stats;
    }
    if (mac_c)
    {
        ret = morsectrl_stats_cmd(mors, MORSE_CMD_ID_MAC_STATS_LOG, reset,
            args.filter_str->sval[0], format);
        if (ret) goto exit_stats;
    }
    if (uph_c)
    {
        ret = morsectrl_stats_cmd(mors, MORSE_CMD_ID_UPHY_STATS_LOG, reset,
            args.filter_str->sval[0], format);
        if (ret) goto exit_stats;
    }

    if (format == FORMAT_JSON)
    {
        mctrl_print("}\n");
    }
    else if (format == FORMAT_JSON_PPRINT)
    {
        mctrl_print("\n}\n");
    }

exit_stats:
    return ret;
}

MM_CLI_HANDLER(stats, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
