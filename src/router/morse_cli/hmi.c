/*
 * Copyright 2026 Morse Micro
 */

#include <stdint.h>
#include <string.h>

#include "command.h"
#include "morse_commands.h"
#include "utilities.h"

static struct
{
    struct arg_rex *command;
} args;

static struct mm_argtable trigger;
static struct mm_argtable action;
static struct mm_argtable link_cmd;

static struct
{
    struct arg_rex *subcmd;
} trigger_args;
static struct mm_argtable trigger_gpio;

static struct
{
    struct arg_rex *subcmd;
} action_args;
static struct mm_argtable action_standby_exit;

static struct
{
    struct arg_int *gpio_num;
    struct arg_int *debounce_ms;
    struct arg_rex *pullup;
    struct arg_rex *edge;
} gpio_args;

static struct
{
    struct arg_int *id1;
    struct arg_int *id2;
} link_args;

static struct mm_argtable *subcmds[] = { &trigger,
                                         &trigger_gpio,
                                         &action,
                                         &action_standby_exit,
                                         &link_cmd };

int hmi_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args,
                     "Human-Machine Interface (HMI) commands",
                     args.command = arg_rex1(NULL,
                                             NULL,
                                             "(create_trigger|create_action|create_link)",
                                             "{create_trigger|create_action|create_link}",
                                             0,
                                             "HMI subcommand"),
                     arg_rem(NULL, "create_trigger - Hardware or software event source"),
                     arg_rem(NULL, "create_action  - Operation to perform when a trigger fires"),
                     arg_rem(NULL, "create_link    - Link a trigger with an action"));
    args.command->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(
        &trigger,
        "HMI trigger creation",
        trigger_args.subcmd = arg_rex1(NULL, NULL, "(gpio)", "{gpio}", 0, "Trigger type"),
        arg_rem(NULL, "gpio - GPIO edge trigger"));
    trigger_args.subcmd->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(
        &trigger_gpio,
        "GPIO edge trigger",
        gpio_args.gpio_num = arg_int1(NULL, "gpio", "<n>", "GPIO number"),
        gpio_args.debounce_ms =
            arg_int0(NULL, "debounce_ms", "<n>", "Debounce time in milliseconds (default: 0)"),
        gpio_args.pullup = arg_rex0(NULL,
                                    "pullup",
                                    "(enable|disable)",
                                    "{enable|disable}",
                                    0,
                                    "Pullup (default: disable)"),
        gpio_args.edge = arg_rex0(NULL,
                                  "edge",
                                  "(rising|falling)",
                                  "{rising|falling}",
                                  0,
                                  "Edge trigger type (default: rising)"));

    MM_INIT_ARGTABLE(&action,
                     "HMI action creation",
                     action_args.subcmd =
                         arg_rex1(NULL, NULL, "(standby_exit)", "{standby_exit}", 0, "Action type"),
                     arg_rem(NULL, "standby_exit - Exit standby mode"));
    action_args.subcmd->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(&action_standby_exit, "Exit standby mode");

    MM_INIT_ARGTABLE(&link_cmd,
                     "Link a trigger with an action",
                     link_args.id1 = arg_int1(NULL, NULL, "<trigger ID>", "Trigger ID"),
                     link_args.id2 = arg_int1(NULL, NULL, "<action ID>", "Action ID"));

    return 0;
}

static void hmi_error_code_hint(int error)
{
    switch (error)
    {
        case MORSE_RET_ENOENT:
            mctrl_err("ENOENT: Unknown trigger/action ID\n");
            break;
        case MORSE_RET_ENOBUFS:
            mctrl_err("ENOBUFS: No buffer available. You reached one of:\n");
            mctrl_err("- The maximum number of triggers or actions\n");
            mctrl_err("- The maximum number of triggers or actions of a specific type\n");
            mctrl_err("- The maximum number of actions a specific trigger can trigger\n");
            break;
        case MORSE_RET_ENOSYS:
            mctrl_err("ENOSYS: Feature is disabled in firmware\n");
            break;
        case MORSE_RET_EBUSY:
            mctrl_err("EBUSY: Resource is busy. "
                "For GPIO triggers it means there is already a trigger on this GPIO\n");
            break;
        case MORSE_RET_EINVAL:
            mctrl_err("EINVAL: Invalid argument. Example: GPIO doesn't exist");
            break;
        default:
            break;
    }
}

/* Returns trigger_id (>= 0) on success, < 0 on error */
static int hmi_trigger_gpio(struct morse_cmd_req_hmi_create_trigger *req, int argc, char *argv[])
{
    int ret = mm_parse_argtable("hmi create_trigger gpio", &trigger_gpio, argc, argv);
    if (ret)
    {
        return -1;
    }

    struct morse_cmd_hmi_trigger_params_gpio *gpio_params =
        (struct morse_cmd_hmi_trigger_params_gpio *)req->param_buff;
    gpio_params->gpio = (uint8_t)gpio_args.gpio_num->ival[0];
    gpio_params->debounce_ms =
        htole32((gpio_args.debounce_ms->count > 0) ? gpio_args.debounce_ms->ival[0] : 0);
    gpio_params->pullup =
        (uint8_t)(gpio_args.pullup->count > 0 && strcmp(gpio_args.pullup->sval[0], "enable") == 0) ?
            1 :
            0;
    gpio_params->edge =
        (uint8_t)(gpio_args.edge->count > 0 && strcmp(gpio_args.edge->sval[0], "falling") == 0) ?
            2 :
            1;

    return 0;
}

static int process_hmi_trigger(struct morsectrl *mors, int argc, char *argv[])
{
    struct morse_cmd_req_hmi_create_trigger *req;
    struct morse_cmd_resp_hmi_create_trigger *rsp;

    int ret = mm_parse_argtable("hmi create_trigger", &trigger, argc, argv);
    if (ret)
    {
        return ret;
    }

    enum morse_cmd_hmi_trigger_type trigger_type = 0;
    uint16_t param_size = 0;
    int (*trigger_type_parser)(
        struct morse_cmd_req_hmi_create_trigger *req, int argc, char *argv[]);

    if (strcmp("gpio", trigger_args.subcmd->sval[0]) == 0)
    {
        trigger_type = MORSE_CMD_HMI_TRIGGER_TYPE_GPIO;
        param_size = sizeof(struct morse_cmd_hmi_trigger_params_gpio);
        trigger_type_parser = hmi_trigger_gpio;
    }
    else
    {
        /* Shouldn't be reachable. This case is caught by argtable */
        MCTRL_ASSERT(false, "not reached");
        return -1;
    }

    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req) + param_size);
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_create_trigger);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_hmi_create_trigger);
    req->trigger_type = trigger_type;
    req->param_len = htole16(param_size);
    ret = trigger_type_parser(req,
                              argc - trigger_args.subcmd->hdr.idx,
                              argv + trigger_args.subcmd->hdr.idx);
    if (ret)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return ret;
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_HMI_CREATE_TRIGGER,
                                 req_tbuff,
                                 rsp_tbuff);
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    if (ret)
    {
        hmi_error_code_hint(ret);
        return ret;
    }

    uint8_t trigger_id = rsp->trigger_id;
    mctrl_print("%d\n", trigger_id);

    return 0;
}

static int hmi_action_standby_exit(struct morse_cmd_req_hmi_create_action *req,
                                   int argc,
                                   char *argv[])
{
    return mm_parse_argtable("hmi create_action standby_exit", &action_standby_exit, argc, argv) ?
               -1 :
               0;
}

static int process_hmi_action(struct morsectrl *mors, int argc, char *argv[])
{
    struct morse_cmd_req_hmi_create_action *req;
    struct morse_cmd_resp_hmi_create_action *rsp;

    int ret = mm_parse_argtable("hmi create_action", &action, argc, argv);
    if (ret)
    {
        return ret;
    }

    enum morse_cmd_hmi_action_type action_type = 0;
    uint16_t param_size = 0;
    int (*action_type_parser)(struct morse_cmd_req_hmi_create_action *req, int argc, char *argv[]);

    if (strcmp("standby_exit", action_args.subcmd->sval[0]) == 0)
    {
        action_type = MORSE_CMD_HMI_ACTION_TYPE_STANDBY_EXIT;
        param_size = 0;
        action_type_parser = hmi_action_standby_exit;
    }
    else
    {
        /* Shouldn't be reachable. This case is caught by argtable */
        MCTRL_ASSERT(false, "not reached");
        return -1;
    }

    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req) + param_size);
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_create_action);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_hmi_create_action);
    req->action_type = action_type;
    req->param_len = htole16(param_size);
    ret = action_type_parser(req,
                             argc - action_args.subcmd->hdr.idx,
                             argv + action_args.subcmd->hdr.idx);
    if (ret)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return ret;
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_HMI_CREATE_ACTION,
                                 req_tbuff,
                                 rsp_tbuff);
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    if (ret)
    {
        hmi_error_code_hint(ret);
        return ret;
    }

    uint8_t action_id = rsp->action_id;
    mctrl_print("%d\n", action_id);

    return 0;
}

static int process_hmi_link(struct morsectrl *mors, int argc, char *argv[])
{
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_req_hmi_create_link *req;

    int ret = mm_parse_argtable("hmi create_link", &link_cmd, argc, argv);
    if (ret)
    {
        return ret;
    }

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport,
                                               sizeof(struct morse_cmd_resp_hmi_create_link));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_create_link);
    req->trigger_id = (uint8_t)link_args.id1->ival[0];
    req->action_id = (uint8_t)link_args.id2->ival[0];

    ret =
        morsectrl_send_command(mors->transport, MORSE_CMD_ID_HMI_CREATE_LINK, req_tbuff, rsp_tbuff);
    hmi_error_code_hint(ret);

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

int hmi(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    const char *subcmd = args.command->sval[0];

    if (strcmp("create_trigger", subcmd) == 0)
    {
        ret = process_hmi_trigger(mors, argc, argv);
    }
    else if (strcmp("create_action", subcmd) == 0)
    {
        ret = process_hmi_action(mors, argc, argv);
    }
    else if (strcmp("create_link", subcmd) == 0)
    {
        ret = process_hmi_link(mors, argc, argv);
    }

    if (mm_check_help_argtable(subcmds, MORSE_ARRAY_SIZE(subcmds)))
    {
        ret = 0;
    }

    for (int i = 0; i < MORSE_ARRAY_SIZE(subcmds); i++)
    {
        mm_free_argtable(subcmds[i]);
    }

    return ret;
}

int hmi_help(void)
{
    mm_help_argtable("hmi create_trigger", &trigger);
    mm_help_argtable("hmi create_action", &action);
    mm_help_argtable("hmi create_link", &link_cmd);
    return 0;
}

MM_CLI_HANDLER_CUSTOM_HELP(hmi, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
