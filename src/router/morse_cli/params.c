/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "portable_endian.h"

#include "command.h"
#include "utilities.h"

struct param_entry;

/**
 * @brief Callback to process user input for setting a parameter entry.
 *
 * @param entry The parameter entry to set.
 * @param value The value provided by the user on the CLI.
 * @param req On success of parsing, the value field of the req struct will be filled.
 *
 * @return 0 on success, else specific error code.
 */
typedef int (*param_process_t)(const struct param_entry* entry,
    const char* value, struct morse_cmd_req_get_set_generic_param* req);

/**
 * @brief Callback that formats the response of a get operation, printing to stdout.
 *
 * @param entry The parameter entry to format.
 * @param resp Pointer to the response to format.
 *
 * @return 0 on success, else specific error code.
 */
typedef int (*param_format_t)(const struct param_entry* entry,
    struct morse_cmd_resp_get_set_generic_param* resp);

struct param_entry {
    /** ID of the parameter */
    enum morse_cmd_param_id id;
    /** Name of the parameter (used to match on user CLI input) */
    const char *name;
    /** The help message to display for the parameter */
    const char *help;
    /** Minimum allowed value of the parameter (can be of a different int type,
     * but cast to uint32) */
    uint32_t min_val;
    /** Maximum allowed value of the parameter (can be of a different int type,
     * but cast to uint32) */
    uint32_t max_val;
    /** Function that processes user input for the set command */
    param_process_t set_fn;
    /** Function that formats response of the get command to stdout */
    param_format_t get_fn;
};

static int param_set_uint32(const struct param_entry* entry, const char* value,
    struct morse_cmd_req_get_set_generic_param* req)
{
    int ret;
    uint32_t val;

    ret = str_to_uint32_range(value, &val, entry->min_val, entry->max_val);
    if (ret)
    {
        mctrl_err("Failed to parse value for '%s' [min:%u, max:%u]\n",
            entry->name, entry->min_val, entry->max_val);
        return ret;
    }

    req->value = htole32(val);
    return 0;
}

static int param_get_uint32(const struct param_entry* entry,
    struct morse_cmd_resp_get_set_generic_param* resp)
{
    mctrl_print("%u\n", resp->value);
    return 0;
}

static int param_set_int32(const struct param_entry* entry, const char* value,
    struct morse_cmd_req_get_set_generic_param* req)
{
    int ret;
    int32_t val;

    ret = str_to_int32_range(value, &val, (int32_t)(entry->min_val), (int32_t)(entry->max_val));
    if (ret)
    {
        mctrl_err("Failed to parse value for '%s' [min:%d, max:%d]\n",
            entry->name, entry->min_val, entry->max_val);
        return ret;
    }

    req->value = htole32((uint32_t)val);
    return 0;
}

static int param_get_int32(const struct param_entry* entry,
                           struct morse_cmd_resp_get_set_generic_param* resp)
{
    mctrl_print("%d\n", le32toh(resp->value));
    return 0;
}

static struct {
    struct arg_str *param;
    struct arg_str *value;
} args;

int get_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Get a chip parameter",
    args.param = arg_str1(NULL, NULL, "<param>", "Parameter name"));
    return 0;
}

int set_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set a chip parameter",
    args.param = arg_str1(NULL, NULL, "<param>", "Parameter name"),
    args.value = arg_str1(NULL, NULL, "<value>", "Value"));
    return 0;
}

/* Help strings for parameters should not have line control characters (e.g. '\n') embedded
 * within.
 */
struct param_entry params[] = {
    {
        .id = MORSE_CMD_PARAM_ID_MAX_TRAFFIC_DELIVERY_WAIT_US,
        .name = "traffic_delivery_wait",
        .help = "Time to wait for traffic delivery from the AP after the TIM "
                "is set in a busy BSS (usecs).",
        .min_val = 0,
        .max_val = UINT32_MAX,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_EXTRA_ACK_TIMEOUT_ADJUST_US,
        .name = "ack_timeout_adjust",
        .help = "Extra time to wait for 802.11 control response frames to be "
                "delivered (usecs).",
        .min_val = 0,
        .max_val = UINT32_MAX,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_WAKE_ACTION_GPIO,
        .name = "wake_action_gpio",
        .help = "Specify GPIO to pulse on reception of a Morse Micro "
                "wake action frame (-1 to disable).",
        .min_val = (uint32_t) -1,
        .max_val = INT32_MAX,
        .set_fn = param_set_int32,
        .get_fn = param_get_int32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_WAKE_ACTION_GPIO_PULSE_MS,
        .name = "wake_action_gpio_pulse",
        .help = "Time to hold wake action GPIO high after reception of "
                "a Morse Micro wake action frame (msecs).",
        .min_val = 50,
        .max_val = UINT32_MAX,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_CONNECTION_MONITOR_GPIO,
        .name = "connection_monitor_gpio",
        .help = "Specify GPIO that monitors and reflects device's "
                "802.11 connection status (-1 to disable).",
        .min_val = (uint32_t) -1,
        .max_val = INT32_MAX,
        .set_fn = param_set_int32,
        .get_fn = param_get_int32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_INPUT_TRIGGER_GPIO,
        .name = "input_trigger_gpio",
        .help = "Specify GPIO that listens for an input signal to "
                "wake an external host (-1 to disable).",
        .min_val = (uint32_t) -1,
        .max_val = INT32_MAX,
        .set_fn = param_set_int32,
        .get_fn = param_get_int32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_INPUT_TRIGGER_MODE,
        .name = "input_trigger_mode",
        .help = "Specify the active mode (high or low) for the trigger GPIO",
        .min_val = (uint32_t) -1,
        .max_val = INT32_MAX,
        .set_fn = param_set_int32,
        .get_fn = param_get_int32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_NON_TIM_MODE,
        .name = "non_tim_mode",
        .help = "Enable non-TIM mode (must be run before association)",
        .min_val = 0,
        .max_val = 1,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_DYNAMIC_PS_TIMEOUT_MS,
        .name = "dynamic_ps_timeout_ms",
        .help = "Dynamic powersave timeout (in ms) after network activity",
        .min_val = 0,
        .max_val = UINT32_MAX,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_HOME_CHANNEL_DWELL_MS,
        .name = "home_channel_dwell_ms",
        .help = "Time to dwell on home channel during scans while associated (ms)",
        .min_val = 0,
        .max_val = UINT32_MAX,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
    {
        .id = MORSE_CMD_PARAM_ID_BEACON_LOSS_COUNT,
        .name = "beacon_loss_count",
        .help = "Number of lost beacons before a beacon loss event is triggered",
        .min_val = 1,
        .max_val = UINT8_MAX,
        .set_fn = param_set_uint32,
        .get_fn = param_get_uint32,
    },
};

static int get_line(const char **start, const char *end)
{
    const int max_len = 60; /* Max line length after leading tabs */
    int len = 0;
    int last_space = max_len + 1;
    const char *p;
    char *eol = MIN((char *)end, (char *)(*start) + max_len);

    /* Find first char to print on this line */
    while ((*start < eol) && ((**start == ' ') || (**start == '\n')))
    {
        (*start)++;
    }
    p = *start;

    /* Search up to the max characters that can be printed for a line
     * termination or end-of-text.
     * Keep track of the last space found, so the text is not split in the
     * middle of a word.
     */
    while (p <= eol)
    {
        if (*p == '\0' || *p == '\n')
        {
            return len;
        }
        if (*p == ' ')
        {
            last_space = len;
        }
        len++;
        p++;
    }

    return MIN(last_space, len);
}

static void print_param_help(const struct param_entry* param)
{
    const char *prefix = "            ";
    const char *start = param->help;
    const char *end = start + strlen(param->help);
    int len = 0;

    len = get_line(&start, end);
    while (start < end)
    {
        mctrl_print("%s%.*s\n", prefix, len, start);
        start += len;
        len = get_line(&start, end);
    }
}

int set_help(void)
{
    mctrl_print("    Available parameters:\n");
    for (unsigned int entry = 0; entry < MORSE_ARRAY_SIZE(params); entry++)
    {
        const struct param_entry* param = &params[entry];

        if (param->set_fn == NULL)
        {
            continue;
        }

        mctrl_print("        %s\n", param->name);
        print_param_help(param);
    }
    return 0;
}

int get_help(void)
{
    mctrl_print("    Available parameters:\n");
    for (unsigned int entry = 0; entry < MORSE_ARRAY_SIZE(params); entry++)
    {
        const struct param_entry* param = &params[entry];

        if (param->get_fn == NULL)
        {
            continue;
        }

        mctrl_print("        %s\n", param->name);
        print_param_help(param);
    }
    return 0;
}

static const struct param_entry* match_str_to_param(const char *str,
                                                    enum morse_cmd_param_action action)
{
    for (unsigned int entry = 0; entry < MORSE_ARRAY_SIZE(params); entry++)
    {
        const struct param_entry* param = &params[entry];

        if (strncmp(str, param->name, strlen(str)) == 0)
        {
            if ((action == MORSE_CMD_PARAM_ACTION_SET) && (param->set_fn == NULL))
            {
                break;
            }

            if ((action == MORSE_CMD_PARAM_ACTION_GET) && (param->get_fn == NULL))
            {
                break;
            }

            return param;
        }
    }

    return NULL;
}

static void param_help(struct morsectrl *mors, enum morse_cmd_param_action action)
{
    if (action == MORSE_CMD_PARAM_ACTION_SET)
    {
        set_help();
    }
    else if (action == MORSE_CMD_PARAM_ACTION_GET)
    {
        get_help();
    }
}

static int param_get_set(struct morsectrl *mors,
    enum morse_cmd_param_action action, int argc, char *argv[])
{
    int ret = MORSE_ARG_ERR;
    const struct param_entry *param;
    struct morse_cmd_req_get_set_generic_param *req;
    struct morse_cmd_resp_get_set_generic_param *rsp;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    param = match_str_to_param(args.param->sval[0], action);
    if (param == NULL)
    {
        mctrl_err("Invalid parameter: '%s'\n", args.param->sval[0]);
        param_help(mors, action);
        return ret;
    }

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_get_set_generic_param);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_get_set_generic_param);

    req->param_id = htole32(param->id);
    req->action = htole32(action);
    req->flags = 0;

    if (action == MORSE_CMD_PARAM_ACTION_SET)
    {
        ret = param->set_fn(param, args.value->sval[0], req);
        if (ret < 0)
        {
            goto exit;
        }
    }

    ret = morsectrl_send_command(mors->transport,
        MORSE_CMD_ID_GET_SET_GENERIC_PARAM, req_tbuff, rsp_tbuff);
exit:
    if (ret == 0)
    {
        if (action == MORSE_CMD_PARAM_ACTION_GET)
        {
            param->get_fn(param, rsp);
        }
    }
    else
    {
        ret = -MORSE_CMD_ERR;
    }

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

int get(struct morsectrl *mors, int argc, char *argv[])
{
    return param_get_set(mors, MORSE_CMD_PARAM_ACTION_GET, argc, argv);
}

int set(struct morsectrl *mors, int argc, char *argv[])
{
    return param_get_set(mors, MORSE_CMD_PARAM_ACTION_SET, argc, argv);
}

MM_CLI_HANDLER_CUSTOM_HELP(get, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
MM_CLI_HANDLER_CUSTOM_HELP(set, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
