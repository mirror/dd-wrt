/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#include "command.h"
#include "utilities.h"
#include "transport/transport.h"
#include "channel.h"

/** Max line length for a config file line */
#define MAX_LINE_LENGTH     (255)

struct standby_config_parse_context
{
    struct morse_cmd_standby_set_config *set_cfg;
    struct morse_cmd_standby_set_wake_filter *filter_cfg;
};

struct standby_session_parse_context
{
    uint8_t *bssid;
    struct morse_cmd_req_set_channel *req;
};

static struct {
    struct arg_rex *command;
} args;

static struct mm_argtable enter;
static struct mm_argtable exit_cmd;
static struct mm_argtable payload;
static struct mm_argtable config;
static struct mm_argtable store;

static struct mm_argtable *subcmds[] =
{
    &enter, &exit_cmd, &payload, &config, &store
};

static struct {
    struct arg_file *session_dir;
} enter_args;

static struct {
    struct arg_lit *json_format;
} exit_args;

static struct {
    struct arg_str *data;
} payload_args;

static struct {
    struct arg_file *file;
} config_args;

static struct {
    struct arg_rex *bssid;
    struct arg_file *dir;
} store_args;

static int standby_get_cmd(const char str[])
{
    if (strcmp("enter", str) == 0) return MORSE_CMD_STANDBY_MODE_ENTER;
    else if (strcmp("exit", str) == 0) return MORSE_CMD_STANDBY_MODE_EXIT;
    else if (strcmp("config", str) == 0) return MORSE_CMD_STANDBY_MODE_SET_CONFIG_V3;
    else if (strcmp("payload", str) == 0) return MORSE_CMD_STANDBY_MODE_SET_STATUS_PAYLOAD;
    else
    {
        return -1;
    }
}

int standby_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    if (mors->debug)
    {
        MM_INIT_ARGTABLE(mm_args, "Control standby state and configuration",
            args.command = arg_rex1(NULL, NULL, "(enter|exit|payload|config|store)",
                "{enter|exit|config|payload|store}", 0, "Standby subcommand"));
    }
    else
    {
        MM_INIT_ARGTABLE(mm_args, "Control standby state and configuration",
            args.command = arg_rex1(NULL, NULL, "(enter|exit|payload|config|store)",
                "{enter|exit|config|payload}", 0, "Standby subcommand"));
    }
    args.command->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(&enter, "Put the STA FW into standby mode",
        enter_args.session_dir = arg_file0(NULL, NULL, "<session dir>",
            "The full directory path for storing persistent sessions"),
        arg_rem(NULL, "Obtained from wpa_supplicant standby_config_dir configuration parameter"),
        arg_rem(NULL, "No longer required and is retained for backwards compatibility"));

    MM_INIT_ARGTABLE(&exit_cmd, "Tell the firmware that the host is awake",
                        arg_rem(NULL, "Firmware responds with one of the following reason codes"),
                        arg_rem(NULL, "0 - none"),
                        arg_rem(NULL, "1 - wake-up frame received"),
                        arg_rem(NULL, "2 - association lost"),
                        arg_rem(NULL, "3 - external input pin fired"),
                        arg_rem(NULL, "4 - whitelisted packet received"),
                        arg_rem(NULL, "6 - TCP connection lost"),
                        arg_rem(NULL, "A message is printed in the following format."),
                        arg_rem(NULL, "Standby mode exited with reason <code> - <description>"),
        exit_args.json_format = arg_lit0("j", "json", "Print the exit message in JSON format"));

    MM_INIT_ARGTABLE(&payload, "Data to append to standby status frames",
        payload_args.data = arg_str1(NULL, NULL, "<hex string>",
            "Hex string of user data to append to standby status frames"));

    MM_INIT_ARGTABLE(&config, "Configure standby mode",
        config_args.file = arg_file1(NULL, NULL, "<config file>",
            "Path to file containing standby mode configuration parameters"));

    MM_INIT_ARGTABLE(&store,
        "Store session information when associated (internal use only)",
            store_args.bssid = arg_rex1("b", NULL, MAC_CMD_REGEX,
                "<BSSID MAC Address>", ARG_REX_ICASE, "Association BSSID"),
            store_args.dir = arg_file1("d", NULL, "<dir>",
                "The full directory path for storing persistent sessions"));

    return 0;
}

int standby_help(void)
{
    mm_help_argtable("standby enter", &enter);
    mm_help_argtable("standby exit", &exit_cmd);
    mm_help_argtable("standby payload", &payload);
    mm_help_argtable("standby config", &config);
    mm_help_argtable("standby store", &store);
    return 0;
}

static int parse_standby_config_keyval(struct morsectrl *mors, void *context, const char *key,
                                    const char *val)
{
    struct standby_config_parse_context *config = context;
    uint32_t temp;
    ipv4_addr_t temp_ip;

    if (mors->debug)
    {
        mctrl_print("standby_config: %s - %s\n", key, val);
    }

    if (strcmp("notify_period_s", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->notify_period_s = htole32(temp);
        return 0;
    }
    else if (strcmp("bss_inactivity_before_deep_sleep_s", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->bss_inactivity_before_deep_sleep_s = htole32(temp);
        return 0;
    }
    else if (strcmp("deep_sleep_period_s", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->deep_sleep_period_s = htole32(temp);
        return 0;
    }
    else if (strcmp("src_ip", key) == 0)
    {
        if (str_to_ip(val, &temp_ip) < 0)
        {
            goto error;
        }
        memcpy(&config->set_cfg->src_ip, &temp_ip, sizeof(config->set_cfg->src_ip));
        return 0;
    }
    else if (strcmp("dest_ip", key) == 0)
    {
        if (str_to_ip(val, &temp_ip) < 0)
        {
            goto error;
        }
        memcpy(&config->set_cfg->dst_ip, &temp_ip, sizeof(config->set_cfg->dst_ip));
        return 0;
    }
    else if (strcmp("dest_port", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->dst_port = htole16((uint16_t) temp);
        return 0;
    }
    else if (strcmp("deep_sleep_increment_s", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->deep_sleep_increment_s = htole32(temp);
        return 0;
    }
    else if (strcmp("deep_sleep_max_s", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->deep_sleep_max_s = htole32(temp);
        return 0;
    }
    else if (strcmp("deep_sleep_scan_iterations", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->set_cfg->deep_sleep_scan_iterations = htole32(temp);
        return 0;
    }
    else if (strcmp("wake_packet_filter", key) == 0)
    {
        uint32_t len = MIN(strlen(val) / 2, MORSE_ARRAY_SIZE(config->filter_cfg->filter));
        hexstr2bin(val, config->filter_cfg->filter, len);
        config->filter_cfg->len = htole32(len);
        return 0;
    }
    else if (strcmp("wake_packet_filter_offset", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        config->filter_cfg->offset = htole32(temp);
        return 0;
    }
    mctrl_err("Key is not a recognised parameter: %s\n", key);
    return 0;

error:
    mctrl_err("Failed to parse value for %s (val: %s)\n", key, val);
    return -1;
}

static int parse_standby_session_keyval(struct morsectrl *mors, void *context, const char *key,
                                            const char *val)
{
    struct standby_session_parse_context *ctx = context;
    uint32_t temp;

    if (mors->debug)
    {
        mctrl_print("standby_session: %s - %s\n", key, val);
    }

    if (strcmp("bssid", key) == 0)
    {
        if (str_to_mac_addr(ctx->bssid, val) < 0)
        {
            goto error;
        }
        return 0;
    }
    else if (strcmp("op_chan_freq", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        ctx->req->op_chan_freq_hz = htole32(temp);
        return 0;
    }
    else if (strcmp("op_chan_bw", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        ctx->req->op_bw_mhz = (uint8_t) temp;
        return 0;
    }
    else if (strcmp("pri_chan_bw", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        ctx->req->pri_bw_mhz = (uint8_t) temp;
        return 0;
    }
    else if (strcmp("pri_1mhz_chan", key) == 0)
    {
        if (str_to_uint32(val, &temp) < 0)
        {
            goto error;
        }
        ctx->req->pri_1mhz_chan_idx = (uint8_t) temp;
        return 0;
    }

    mctrl_err("Key is not a recognised parameter: %s\n", key);
    return 0;

error:
    mctrl_err("Failed to parse value for %s (val: %s)\n", key, val);
    return -1;
}

/**
 * @brief Generic config file parser.
 * Scans lines and calls a handler function for each `key=value` it finds.
 * Lines starting with a `#` will be interpreted as a comment and ignored.
 *
 * @param mors Morsectrl object
 * @param conf_file File to parse
 * @param keyval_process Callback function called for each key value
 * @param context Context pointer for callback function
 * @return int 0 if successfully parsed, otherwise negative value
 */
static int config_parse(struct morsectrl *mors, const char *conf_file,
                        int (*keyval_process)(
                                    struct morsectrl *mors,
                                    void *context,
                                    const char *key,
                                    const char *val),
                        void *context)
{
    int ret;
    FILE *cfg_file;
    char line[MAX_LINE_LENGTH];
    uint16_t line_num = 0;

    if (!conf_file || !keyval_process || is_dir(conf_file))
        return -1;

    cfg_file = fopen(conf_file, "r");

    if (cfg_file == NULL)
        return -1;

    while (fgets(line, MAX_LINE_LENGTH, cfg_file))
    {
        char *key, *val;

        line_num++;

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        key = strip(line);
        val = strchr(line, '=');

        if (val)
            *val++ = '\0';

        if (key && key[0] && val && val[0])
        {
            ret = keyval_process(mors, context, key, val);
            if (ret)
            {
                fclose(cfg_file);
                return ret;
            }
        }
        else
        {
            mctrl_err("No key=value on line %d\n", line_num);
            fclose(cfg_file);
            return -1;
        }
    }

    fclose(cfg_file);
    return 0;
}

static int standby_session_store(struct morsectrl *mors, const char *ifname, const uint8_t *bssid,
    const char *standby_session_dir, struct morse_cmd_resp_get_channel *rsp)
{
    FILE *f;
    char dir[MORSE_FILENAME_LEN_MAX];
    char fname[MORSE_FILENAME_LEN_MAX];
    struct stat buf;

    if (snprintf(dir, sizeof(dir), "%s", standby_session_dir) < 0)
    {
        mctrl_err("%s: Failed to set dir name (%d)\n",
                dir, errno);
        return -1;
    }

    if (stat(dir, &buf) != 0) {
        if (mkdir_path(dir) < 0)
        {
            mctrl_err("%s: Failed to create %s (%d)\n",
                    ifname, dir, errno);
            return -1;
        }
    }

    if (snprintf(fname, sizeof(fname), "%s/%s", dir, ifname) < 0) {
        mctrl_err("%s: Failed to set file name (%d)\n",
                ifname, errno);
        return -1;
    }

    f = fopen(fname, "w");
    if (!f) {
        mctrl_err("%s: Failed to open %s\n",
                ifname, fname);
        return -1;
    }

    fprintf(f, "bssid=" MACSTR "\n", MAC2STR(bssid));
    fprintf(f, "op_chan_freq=%u\n", rsp->op_chan_freq_hz);
    fprintf(f, "op_chan_bw=%u\n", rsp->op_chan_bw_mhz);
    fprintf(f, "pri_chan_bw=%u\n", rsp->pri_chan_bw_mhz);
    fprintf(f, "pri_1mhz_chan=%u\n", rsp->pri_1mhz_chan_idx);

    fclose(f);

    if (mors->debug)
    {
        mctrl_print("%s: Created %s\n", ifname, fname);
    }

    return 0;
}

static int standby_session_load(struct morsectrl *mors, const char *standby_session_dir,
        uint8_t *bssid, struct morse_cmd_req_set_channel *req)
{
    const char *ifname = morsectrl_transport_get_ifname(mors->transport);
    struct standby_session_parse_context context = {
        .bssid = bssid,
        .req = req
    };

    char session_path[MORSE_FILENAME_LEN_MAX];

    if (snprintf(session_path, sizeof(session_path),
                "%s/%s", standby_session_dir, ifname) < 0)
    {
        mctrl_err("%s: Failed to set session path name (%d)\n",
                ifname, errno);
        return -1;
    }

    if (config_parse(mors, session_path, parse_standby_session_keyval, &context))
    {
        goto err_parse;
    }

    return 0;

err_parse:
    mctrl_err("%s: Failed to parse %s\n", ifname, standby_session_dir);
    return -1;
}

static int process_standby_enter(struct morsectrl *mors,
                                    struct morse_cmd_req_standby_mode *standby_cmd,
                                    int argc, char *argv[])
{
    const char *standby_session_dir = NULL;
    int ret;
    struct morse_cmd_req_set_channel *ch_cmd;
    struct morsectrl_transport_buff *cmd_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    ret = mm_parse_argtable("standby enter", &enter, argc, argv);
    if (ret != 0)
    {
        goto exit;
    }

    if (enter_args.session_dir->count)
    {
        standby_session_dir = enter_args.session_dir->filename[0];
    }
    else
    {
        return 0;
    }

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*ch_cmd));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport,
                    sizeof(struct morse_cmd_resp_set_channel));

    if (!cmd_tbuff || !rsp_tbuff)
    {
        mctrl_err("Alloc failure\n");
        ret = -1;
        goto exit;
    }

    ch_cmd = TBUFF_TO_REQ(cmd_tbuff, struct morse_cmd_req_set_channel);

    /* load the saved standby parameters */
    ret = standby_session_load(mors, standby_session_dir,
                standby_cmd->enter.monitor_bssid.octet, ch_cmd);
    if (ret < 0)
    {
        mctrl_err("Failed to load session info\n");
        goto exit;
    }

    if (mors->debug)
    {
        mctrl_print("Loaded session info:\n");
        mctrl_print("bssid " MACSTR "\n", MAC2STR(standby_cmd->enter.monitor_bssid.octet));
        mctrl_print("op ch freq %d\n", ch_cmd->op_chan_freq_hz);
        mctrl_print("op ch bw %d\n", ch_cmd->op_bw_mhz);
        mctrl_print("pri ch bw %d\n", ch_cmd->pri_bw_mhz);
        mctrl_print("pri 1mhz idx %d\n", ch_cmd->pri_1mhz_chan_idx);
    }

    /* Set the channel before we go to sleep */
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_CHANNEL,
                                 cmd_tbuff, rsp_tbuff);
    if (ret < 0)
    {
        mctrl_err("Failed to set channel info\n");
        goto exit;
    }

exit:
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

static int process_standby_exit(struct morsectrl *mors,
                                    int argc, char *argv[])
{
    return mm_parse_argtable("standby exit", &exit_cmd, argc, argv);
}

/* Standby store commands are invoked from wpa_supplicant, so provide some context for error
 * messages.
 */
static void standby_store_print_msg(const char *msg)
{
    mctrl_err("morsectrl standby store failed - %s\n", msg);
}

static int standby_store_session_cmd(struct morsectrl *mors, int argc, char *argv[])
{
    uint8_t bssid[MAC_ADDR_LEN] = { 0 };
    const char *standby_session_dir = NULL;
    int ret;
    const char *ifname = morsectrl_transport_get_ifname(mors->transport);
    struct morse_cmd_req_set_channel *req;
    struct morse_cmd_resp_get_channel *rsp;
    struct morsectrl_transport_buff *cmd_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    ret = mm_parse_argtable("standby store", &store, argc, argv);
    if (ret != 0)
    {
        goto exit;
    }

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));

    if (ifname == NULL)
    {
        standby_store_print_msg("no interface - transport not supported");
        ret = -1;
        goto exit;
    }

    if (!cmd_tbuff || !rsp_tbuff)
    {
        standby_store_print_msg("alloc failure");
        ret = -1;
        goto exit;
    }

    req = TBUFF_TO_REQ(cmd_tbuff, struct morse_cmd_req_set_channel);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_get_channel);

    if (str_to_mac_addr(bssid, store_args.bssid->sval[0]) < 0)
    {
        /* Shouldn't get here with regexp parsing above */
        ret = -1;
        goto exit;
    }

    standby_session_dir = store_args.dir->filename[0];

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_CHANNEL_FULL,
                                 cmd_tbuff, rsp_tbuff);
    if (ret < 0)
    {
        standby_store_print_msg("failed to get channel info");
        ret = -1;
        goto exit;
    }

    ret = standby_session_store(mors, ifname, bssid, standby_session_dir, rsp);

exit:
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

static int send_wake_filter_cmd(struct morsectrl *mors,
                                    struct morse_cmd_standby_set_wake_filter *wake_cmd)
{
    int ret = -1;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_req_standby_mode *req;
    struct morse_cmd_resp_standby_mode *rsp = NULL;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));

    if (!req_tbuff || !rsp_tbuff)
    {
        goto exit;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_standby_mode);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_standby_mode);

    req->cmd = htole32(MORSE_CMD_STANDBY_MODE_SET_WAKE_FILTER);

    memcpy(&req->set_filter, wake_cmd, sizeof(*wake_cmd));

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_STANDBY_MODE,
        req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

static int process_set_config_cmd(struct morsectrl *mors,
                struct morse_cmd_req_standby_mode* req, int argc, char *argv[])
{
    struct morse_cmd_standby_set_wake_filter wake_filter = {0};
    static struct morse_cmd_standby_set_config config_default = {
        .src_ip = 0,
        .dst_ip = 0,
        .deep_sleep_increment_s = 0, /* Default no increment */
    };
    config_default.bss_inactivity_before_deep_sleep_s = htole32(60);
    config_default.deep_sleep_period_s = htole32(120);
    config_default.notify_period_s = htole32(15);
    config_default.dst_port = htole16(22000);
    config_default.deep_sleep_max_s = htole32(UINT32_MAX); /* Default max int / no max */

    int ret;

    ret = mm_parse_argtable("standby config", &config, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    memcpy(&req->config, &config_default, sizeof(config_default));

    struct standby_config_parse_context context = {
        .filter_cfg = &wake_filter,
        .set_cfg = &req->config
    };

    if (config_parse(mors, config_args.file->filename[0], parse_standby_config_keyval, &context))
    {
        mctrl_err("Failed to parse config file\n");
        return -1;
    }

    /* If the wake filter has been configured, send the command to set it here. */
    if (wake_filter.len != 0)
    {
        return send_wake_filter_cmd(mors, &wake_filter);
    }

    return 0;
}

static int process_set_status_payload(struct morse_cmd_req_standby_mode* req,
                                      int argc, char *argv[])
{
    uint32_t payload_len;

    int ret;
    ret = mm_parse_argtable("standby payload", &payload, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    payload_len = strlen(payload_args.data->sval[0]);
    if ((payload_len % 2) != 0)
    {
        mctrl_err("Invalid hex string, length must be a multiple of 2\n");
        return -1;
    }

    payload_len = (payload_len / 2);
    if (payload_len > MORSE_CMD_STANDBY_STATUS_FRAME_USER_PAYLOAD_MAX_LEN)
    {
        mctrl_err("Supplied payload is too large: %d > %d\n", payload_len,
            MORSE_CMD_STANDBY_WAKE_FRAME_USER_FILTER_MAX_LEN);
        return -1;
    }

    req->set_payload.len = htole32(payload_len);
    for (int i = 0; i < payload_len; i++)
    {
        ret = sscanf(&(payload_args.data->sval[0][i * 2]), "%2hhx", &req->set_payload.payload[i]);
        if (ret != 1)
        {
            mctrl_err("Invalid hex string\n");
            return -1;
        }
    }

    return 0;
}

static const char *standby_exit_reason_to_str(int reason)
{
    switch (reason) {
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_NONE:
        return "none";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_WAKEUP_FRAME:
        return "wake-up frame received";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_ASSOCIATE:
        return "association lost";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_EXT_INPUT:
        return "external input pin fired";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_WHITELIST_PKT:
        return "whitelisted packet received";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_TCP_CONNECTION_LOST:
        return "TCP connection lost";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_HW_SCAN_NOT_ENABLED:
        return "HW scan not enabled";
    case MORSE_CMD_STANDBY_MODE_EXIT_REASON_HW_SCAN_FAILED_TO_START:
        return "HW scan failed to start";
    default:
        return "unknown";
    }
}

int standby(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    int i = 0;
    bool json = false;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_req_standby_mode *req;
    struct morse_cmd_resp_standby_mode *rsp = NULL;

    /* Local-only command - not sent to firmware */
    if (strcmp("store", args.command->sval[0]) == 0)
    {
        return standby_store_session_cmd(mors, argc, argv);
    }

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));

    if (!req_tbuff || !rsp_tbuff)
    {
        goto exit;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_standby_mode);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_standby_mode);

    req->cmd = htole32(standby_get_cmd(args.command->sval[0]));

    switch (le32toh(req->cmd))
    {
        case MORSE_CMD_STANDBY_MODE_SET_CONFIG_V3:
        {
            ret = process_set_config_cmd(mors, req, argc, argv);
            if (ret)
            {
                goto exit;
            }

            if (mors->debug)
            {
                mctrl_print("Setting standby configuration:\n");
                mctrl_print("  Deep sleep inactivity period: %d\n",
                        req->config.bss_inactivity_before_deep_sleep_s);
                mctrl_print("  Deep_sleep period: %d\n", req->config.deep_sleep_period_s);
                mctrl_print("  Deep sleep scan iterations: %d\n",
                        req->config.deep_sleep_scan_iterations);
                mctrl_print("  Notify period : %d\n", req->config.notify_period_s);
                mctrl_print("  Dst port: %d\n", req->config.dst_port);
                ipv4_addr_t dst_ip, src_ip;
                dst_ip.as_u32 = le32toh(req->config.src_ip);
                src_ip.as_u32 = le32toh(req->config.dst_ip);

                mctrl_print("  Dst ip: " IPSTR "\n", IP2STR(dst_ip.octet));
                mctrl_print("  Src ip: " IPSTR "\n", IP2STR(src_ip.octet));
            }

            break;
        }
        case MORSE_CMD_STANDBY_MODE_SET_STATUS_PAYLOAD:
        {
            ret = process_set_status_payload(req, argc, argv);
            if (ret)
            {
                goto exit;
            }

            break;
        }
        case MORSE_CMD_STANDBY_MODE_ENTER:
        {
            ret = process_standby_enter(mors, req, argc, argv);
            if (ret)
            {
                goto exit;
            }
            break;
        }
        case MORSE_CMD_STANDBY_MODE_EXIT:
            ret = process_standby_exit(mors, argc, argv);
            if (ret)
            {
                goto exit;
            }
            json = (exit_args.json_format->count > 0);
            break;
        default:
            break;
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_STANDBY_MODE,
        req_tbuff, rsp_tbuff);

    if (req->cmd == htole32(MORSE_CMD_STANDBY_MODE_EXIT) && ret == 0)
    {
        if (json)
        {
            mctrl_print("[");
            mctrl_print("{\"Standby mode exited with reason\": %u - %s}",
                    rsp->info.reason, standby_exit_reason_to_str(rsp->info.reason));
            mctrl_print("]\n");
        }
        else
        {
            mctrl_print("Standby mode exited with reason %u - %s\n", rsp->info.reason,
                standby_exit_reason_to_str(rsp->info.reason));
        }
    }

exit:
    /* Check if the reason we got here is because --help was given */
    if (mm_check_help_argtable(subcmds, MORSE_ARRAY_SIZE(subcmds)))
    {
        ret = 0;
    }

    for (i = 0; i < MORSE_ARRAY_SIZE(subcmds); i++)
    {
        mm_free_argtable(subcmds[i]);
    }

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER_CUSTOM_HELP(standby, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
