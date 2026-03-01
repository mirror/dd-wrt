/*
 * Copyright 2020 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <linux/nl80211.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <net/if.h>
#include <netlink/attr.h>
#include <fcntl.h>

#include "../utilities.h"
#include "transport.h"
#include "transport_private.h"

#define MORSE_OUI 0x0CBF74
#define MORSE_VENDOR_CMD_TO_MORSE 0x00
#define MORSE_VENDOR_WIPHY_CMD_TO_MORSE 0x01
#define NL80211_BUFFER_SIZE (8192)


static const struct morsectrl_transport_ops nl80211_ops;

/** @brief Configuration for the NL80211 interface. */
struct morsectrl_nl80211_cfg
{
    const char *interface_name;
};

/** @brief State information for the NL80211 interface. */
struct morsectrl_nl80211_state
{
    int interface_index;
    int wiphy_index;
    int nl80211_id;
    size_t *len;
    uint8_t *data;
    struct nl_sock* nl_socket;
    struct nl_cb *cb;
    struct nl_cb *s_cb;
    bool wait_for_ack;
};

/** @brief Data structure used to represent an instance of this trasport. */
struct morsectrl_nl80211_transport
{
    struct morsectrl_transport common;
    struct morsectrl_nl80211_cfg config;
    struct morsectrl_nl80211_state state;
};

/** Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *  config field. */
static struct morsectrl_nl80211_cfg *nl80211_cfg(struct morsectrl_transport *transport)
{
    struct morsectrl_nl80211_transport *nl80211_transport =
        (struct morsectrl_nl80211_transport *)transport;
    return &nl80211_transport->config;
}

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *        state field.
 */
static struct morsectrl_nl80211_state *nl80211_state(struct morsectrl_transport *transport)
{
    struct morsectrl_nl80211_transport *nl80211_transport =
        (struct morsectrl_nl80211_transport *)transport;
    return &nl80211_transport->state;
}


/**
 * @brief Prints an error message if possible.
 *
 * @param transport     Transport to print the error message from.
 * @param error_code    Error code.
 * @param error_msg     Error message.
 */
static void morsectrl_nl80211_error(int error_code, char *error_msg)
{
    morsectrl_transport_err("NL80211", error_code, error_msg);
}

static int morsectrl_nl80211_parse(struct morsectrl_transport **transport,
                                   bool debug,
                                   const char *iface_opts,
                                   const char *cfg_opts)
{
    struct morsectrl_nl80211_cfg *cfg;

    struct morsectrl_nl80211_transport *nl80211_transport = calloc(1, sizeof(*nl80211_transport));
    if (!nl80211_transport)
    {
        mctrl_err("Transport memory allocation failure\n");
        return -ETRANSNOMEM;
    }
    nl80211_transport->common.tops = &nl80211_ops;
    nl80211_transport->common.debug = debug;
    *transport = &nl80211_transport->common;
    cfg = nl80211_cfg(*transport);

    (void)cfg_opts;

    if (!iface_opts)
    {
        cfg->interface_name = DEFAULT_INTERFACE_NAME;
    }
    else
    {
        cfg->interface_name = iface_opts;
    }

    if (nl80211_transport->common.debug)
    {
        mctrl_print("Using %s interface\n", cfg->interface_name);
    }

    return ETRANSSUCC;
}

/**
 * @brief Handle errors from the netlink interface.
 *
 * @param nla   Netlink socket address.
 * @param nlerr Netlink error.
 * @param arg   @ref morsectrl_transport opaque pointer.
 * @return      NL_STOP always.
 */
static int morsectrl_nl80211_error_handler(struct sockaddr_nl *nla,
                                           struct nlmsgerr *nlerr,
                                           void *arg)
{
    morsectrl_nl80211_error(nlerr->error, "Error callback called");

    return NL_STOP;
}

/**
 * @brief Handle acks from the netlink interface.
 *
 * @param msg   Netlink message.
 * @param arg   @ref morsectrl_transport opaque pointer.
 * @return      NL_STOP always.
 */
static int morsectrl_nl80211_ack_handler(struct nl_msg *msg, void *arg)
{
    struct morsectrl_transport *transport = (struct morsectrl_transport *)arg;

    if (transport)
    {
        nl80211_state(transport)->wait_for_ack = false;

        if (transport->debug) {
            mctrl_print("nla_msg_dump\n");
            nl_msg_dump(msg, stdout);
        }
    }

    return NL_STOP;
}

/**
 * @brief Handle reception of response messages from the netlink interface.
 *
 * @param msg   Netlink message.
 * @param arg   @ref morsectrl_transport opaque pointer.
 * @return      NL_SKIP if reponse is missing otherwise NL_OK.
 */
static int morsectrl_nl80211_receive_handler(struct nl_msg *msg, void *arg)
{
    struct morsectrl_transport *transport = (struct morsectrl_transport *)arg;
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *attr;
    struct morsectrl_nl80211_state *state;
    uint8_t *data;
    int len;

    if (!transport)
    {
        morsectrl_nl80211_error(-ETRANSNL80211ERR, "Transport for receive invalid");
        return NL_SKIP;
    }

    state = nl80211_state(transport);

    if (transport->debug)
    {
        mctrl_print("nla_msg_dump\n");
        nl_msg_dump(msg, stdout);
    }

    attr = nla_find(genlmsg_attrdata(gnlh, 0),
                    genlmsg_attrlen(gnlh, 0),
                    NL80211_ATTR_VENDOR_DATA);
    if (!attr)
    {
        morsectrl_nl80211_error(0, "Vendor data attribute missing");
        return NL_SKIP;
    }

    data = (uint8_t *) nla_data(attr);
    len = nla_len(attr);

    if (len > *state->len)
    {
        morsectrl_nl80211_error(-ETRANSNL80211ERR, "Output buffer too small limiting output");
        len = *state->len;
    }

    memcpy(state->data, data, len);
    *state->len = len;
    return NL_OK;
}

/**
 * @brief Look up the wiphy index from the FS.
 *
 * @param name  wiphy interface name. e.g. phy0
 * @return  wiphy index or -1 on failure.
 */
static int morsectrl_nl80211_phy_lookup(const char *name)
{
    char buf[DEVICE_NAME_LEN];
    int fd, pos;
    int phy_id = -1;

    snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);

    fd = open(buf, O_RDONLY);
    if (fd < 0)
        return phy_id;

    pos = read(fd, buf, sizeof(buf) - 1);
    if (pos < 0) {
        close(fd);
        return phy_id;
    }

    /* we read a newline as well above, replace it */
    buf[pos-1] = '\0';
    close(fd);

    if (str_to_int32(buf, &phy_id))
    {
        morsectrl_nl80211_error(ETRANSNL80211ERR, "morsectrl_nl80211_phy_lookup failed");
        phy_id = -1;
    }

    return phy_id;
}

static int morsectrl_nl80211_init(struct morsectrl_transport *transport)
{
    struct morsectrl_nl80211_state *state;
    struct morsectrl_nl80211_cfg *cfg;
    int ret = 0;
#ifdef NETLINK_EXT_ACK
    int option_value;
#endif

    if (!transport)
        return -ETRANSNL80211ERR;

    cfg = nl80211_cfg(transport);
    state = nl80211_state(transport);
    state->interface_index = if_nametoindex(cfg->interface_name);
    state->wiphy_index = -1;

    if (state->interface_index == 0)
    {
        /* This could be a wiphy interface. */
        state->wiphy_index = morsectrl_nl80211_phy_lookup(cfg->interface_name);
        if (state->wiphy_index < 0)
        {
            ret = -ETRANSNL80211ERR;
            morsectrl_nl80211_error(state->wiphy_index, "Invalid wiphy or interface index");
            return ret;
        }
    }

    state->wait_for_ack = false;
    state->nl_socket = nl_socket_alloc();

    if (state->nl_socket == NULL)
    {
        ret = -ENOMEM;
        morsectrl_nl80211_error(ret, "Failed to allocate netlink socket");
        return ret;
    }

    ret = genl_connect(state->nl_socket);
    if (ret < ETRANSSUCC)
    {
        morsectrl_nl80211_error(ret, "genl_connect failed");
        goto exit_socket_free;
    }
    nl_socket_set_buffer_size(state->nl_socket,
                              NL80211_BUFFER_SIZE,
                              NL80211_BUFFER_SIZE);
#ifdef NETLINK_EXT_ACK
    /* try to set NETLINK_EXT_ACK to 1, ignoring errors */
    option_value = 1;
    setsockopt(nl_socket_get_fd(state->nl_socket),
               SOL_NETLINK, NETLINK_EXT_ACK,
               &option_value, sizeof(option_value));
#endif
    state->nl80211_id = genl_ctrl_resolve(state->nl_socket, "nl80211");
    if (state->nl80211_id < 0)
    {
        ret = -ENOENT;
        morsectrl_nl80211_error(ret, "Failed to get netlink id");
        goto exit_socket_free;
    }

    state->s_cb = nl_cb_alloc(transport->debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
    state->cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!state->cb || !state->s_cb)
    {
        ret = -ENOMEM;
        morsectrl_nl80211_error(ret, "Failed to allocate netlink callbacks");
        goto exit_cb_free;
    }

    nl_cb_err(state->cb, NL_CB_CUSTOM, morsectrl_nl80211_error_handler, transport);
    nl_cb_set(state->cb, NL_CB_VALID, NL_CB_CUSTOM, morsectrl_nl80211_receive_handler, transport);
    nl_cb_set(state->cb, NL_CB_ACK, NL_CB_CUSTOM, morsectrl_nl80211_ack_handler, transport);
    nl_socket_set_cb(state->nl_socket, state->s_cb);

    return ret;

exit_cb_free:
    nl_cb_put(state->cb);
    nl_cb_put(state->s_cb);
exit_socket_free:
    nl_socket_free(state->nl_socket);
    return ret;
}

static int morsectrl_nl80211_deinit(struct morsectrl_transport *transport)
{
    struct morsectrl_nl80211_state *state;

    if (!transport)
        return -ETRANSNL80211ERR;

    state = nl80211_state(transport);
    nl_cb_put(state->cb);
    nl_cb_put(state->s_cb);
    nl_socket_free(state->nl_socket);
    memset(state, 0, sizeof(*state));
    return ETRANSSUCC;
}

/**
 * @brief Allocate @ref morsectrl_transport_buff for commands and responses.
 *
 * @param size  Size of command and morse headers.
 * @return      Allocated @ref morsectrl_transport_buff or NULL on failure.
 */
static struct morsectrl_transport_buff *morsectrl_nl80211_alloc(size_t size)
{
    struct morsectrl_transport_buff *buff;

    if (size <= 0)
        return NULL;

    buff = (struct morsectrl_transport_buff *)malloc(sizeof(struct morsectrl_transport_buff));
    if (!buff)
        return NULL;

    buff->capacity = size;
    buff->memblock = (uint8_t *)malloc(buff->capacity);
    if (!buff->memblock)
    {
        free(buff);
        return NULL;
    }

    /* In this case there isn't any framing required in a contiguous block of memory. */
    buff->data = buff->memblock;
    buff->data_len = buff->capacity;

    return buff;
}

static struct morsectrl_transport_buff *morsectrl_nl80211_write_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    if (!transport)
        return NULL;

    return morsectrl_nl80211_alloc(size);
}

static struct morsectrl_transport_buff *morsectrl_nl80211_read_alloc(
    struct morsectrl_transport *transport, size_t size)
{
    if (!transport)
        return NULL;

    return morsectrl_nl80211_alloc(size);
}

static int morsectrl_nl80211_send(struct morsectrl_transport *transport,
                                  struct morsectrl_transport_buff *cmd,
                                  struct morsectrl_transport_buff *resp)
{
    int ret = ETRANSSUCC;
    void* header;
    struct morsectrl_nl80211_state *state;
    struct nl_msg* msg;

    if (!transport)
        return -ETRANSNL80211ERR;

    state = nl80211_state(transport);
    state->data = resp->data;
    state->len = &resp->data_len;

    msg = nlmsg_alloc();
    if (msg == NULL)
    {
        ret = -ENOMEM;
        morsectrl_nl80211_error(ret, "Failed to allocate netlink message");
        goto exit;
    }

    header = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id,
                         0, 0, NL80211_CMD_VENDOR, 0);
    if (header == NULL)
    {
        ret = -ENOMEM;
        morsectrl_nl80211_error(ret, "Unable to put msg");
        goto exit_message_free;
    }

    NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_ID, MORSE_OUI);
    if (state->wiphy_index >= 0)
    {
        NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, state->wiphy_index);
        NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, MORSE_VENDOR_WIPHY_CMD_TO_MORSE);
    }
    else
    {
        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, state->interface_index);
        NLA_PUT_U32(msg, NL80211_ATTR_VENDOR_SUBCMD, MORSE_VENDOR_CMD_TO_MORSE);
    }

    NLA_PUT(msg, NL80211_ATTR_VENDOR_DATA, cmd->data_len, cmd->data);

    state->wait_for_ack = true;
    ret = nl_send_auto_complete(state->nl_socket, msg);
    if (ret < ETRANSSUCC)
    {
        morsectrl_nl80211_error(ret, "Failed to send_auto_complete");
        goto exit_message_free;
    }

    ret = nl_recvmsgs(state->nl_socket, state->cb);

    if (ret < ETRANSSUCC)
    {
        morsectrl_nl80211_error(ret, "Failed to rcvmsgs");
        goto exit_message_free;
    }

    if (state->wait_for_ack)
    {
        ret = nl_wait_for_ack(state->nl_socket);
        if (ret < 0)
        {
            morsectrl_nl80211_error(ret, "Failed to wait for ACK");
        }
    }

nla_put_failure:
exit_message_free:
    state->wait_for_ack = false;
    nlmsg_free(msg);
exit:
    return ret;
}

const char *morsectrl_nl80211_get_ifname(struct morsectrl_transport *transport)
{
    return nl80211_cfg(transport)->interface_name;
}


static const struct morsectrl_transport_ops nl80211_ops = {
    .name = "nl80211",
    .description = "Linux kernel netlink interface",
    .has_reset = false,
    .has_driver = true,
    .parse = morsectrl_nl80211_parse,
    .init = morsectrl_nl80211_init,
    .deinit = morsectrl_nl80211_deinit,
    .write_alloc = morsectrl_nl80211_write_alloc,
    .read_alloc = morsectrl_nl80211_read_alloc,
    .send = morsectrl_nl80211_send,
    .reg_read = NULL,
    .reg_write = NULL,
    .mem_read = NULL,
    .mem_write = NULL,
    .raw_read = NULL,
    .raw_write = NULL,
    .raw_read_write = NULL,
    .reset_device = NULL,
    .get_ifname = morsectrl_nl80211_get_ifname,
};

REGISTER_TRANSPORT(nl80211_ops);
