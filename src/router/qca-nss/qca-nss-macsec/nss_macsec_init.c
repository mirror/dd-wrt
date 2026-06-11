/*
 * Copyright (c) 2014, 2018-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <net/sock.h>
#include <linux/types.h>
#include <linux/errno.h>
#include "nss_macsec_sdk_api.h"
#include "nss_macsec_fal_api.h"
#include "nss_macsec.h"
#include "nss_macsec_utility.h"


static struct sock *sdk_nl_sk = NULL;

static unsigned int nss_macsec_msg_handle(void *msg_data, unsigned int *sync)
{
	struct sdk_msg_header *header = (struct sdk_msg_header *)msg_data;
	unsigned short cmd_type = header->cmd_type;
	unsigned int ret = 0;

	*sync = 1;
	switch (cmd_type) {
	case SDK_FAL_CMD:{
			ret = nss_macsec_fal_msg_handle(header);
		}
		break;

	default:
		break;
	}

	return ret;
}

static void nss_macsec_netlink_recv(struct sk_buff *__skb)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct sdk_msg_header *header = NULL;
	void *msg_data;
	u32 pid = 0, msg_type = 0, sync = 0, ret = 0, msg_len = 0;

	if ((skb = skb_clone(__skb, GFP_KERNEL)) != NULL) {
		nlh = nlmsg_hdr(skb);
		pid = nlh->nlmsg_pid;
		msg_data = NLMSG_DATA(nlh);
		msg_type = nlh->nlmsg_type;
		msg_len = sizeof(struct nlmsghdr) + sizeof(struct sdk_msg_header);
		header = (struct sdk_msg_header *)msg_data;
		if (header->buf_len > (U32_MAX - msg_len))
			return;
		msg_len += header->buf_len;

		if(skb->len < msg_len) {
			osal_print("Unexpected msg received! skb_len [0x%x] less than 0x%x\n",
				skb->len, msg_len);
			return;
		} else if (msg_type == SDK_CALL_MSG) {
			ret = nss_macsec_msg_handle(msg_data, &sync);
			header->ret = ret;
		} else {
			osal_print("Unexpected msg:0x%x received!\n", msg_type);
			return;
		}

		NETLINK_CB(skb).portid = 0; /* from kernel */
		NETLINK_CB(skb).dst_group = 0;	/* unicast */
		netlink_unicast(sdk_nl_sk, skb, pid, MSG_DONTWAIT);
	}

	return;
}

static int nss_macsec_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
			.input	= nss_macsec_netlink_recv,
	};

	sdk_nl_sk = netlink_kernel_create(&init_net, NETLINK_SDK, &cfg);
	if (sdk_nl_sk == NULL) {
		osal_print("Create netlink socket fail\n");
		return -ENODEV;
	}

	return 0;
}

u32 nss_macsec_dt_secy_id_get(u8 *dev_name, u32 *secy_id)
{
	u32 dev_id = INVALID_DEVICE_ID;
	struct net_device *netdev = NULL;

	/* get net device by ifname */
	netdev = __dev_get_by_name(&init_net, (const char *)dev_name);
	if (unlikely(!netdev)) {
		*secy_id = INVALID_DEVICE_ID;
		/* no net device, return not found */
		return ERROR_NOT_FOUND;
	}

	/* get phydev by net device and find secy_id */
	if (netdev->phydev)
		dev_id = macsec_get_device_id(netdev->phydev);

	*secy_id = dev_id;

	return ERROR_OK;
}

static void nss_macsec_netlink_fini(void)
{
	if (sdk_nl_sk) {
		sock_release(sdk_nl_sk->sk_socket);
		sdk_nl_sk = NULL;
	}
}

static int nss_macsec_dev_event(struct notifier_block  *nb, unsigned long event, void  *info)
{
	struct net_device *event_dev = netdev_notifier_info_to_dev(info);
	int ret = 0;

	if (NULL == event_dev->phydev)
		return NOTIFY_DONE;

	switch (event_dev->phydev->phy_id) {
	case QCA8081_PHY:
	case QCA8081_PHY_V1_1:
	case QCA8084_PHY:
		break;
	default:
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_REGISTER:
		ret = qca_macsec_device_attach(event_dev->phydev);
		if (ret)
			ret = NOTIFY_DONE;
		else
			ret = NOTIFY_OK;
		break;
	case NETDEV_UNREGISTER:
		ret = qca_macsec_device_detach(event_dev->phydev);
		if (ret)
			ret = NOTIFY_DONE;
		else
			ret = NOTIFY_OK;
		break;
	default:
		ret = NOTIFY_DONE;
		break;
	}

	return ret;
}

/*
 * Linux Net device Notifier
 */
static struct notifier_block nss_macsec_netdev_notifier = {
	.notifier_call = nss_macsec_dev_event,
};

static int __init nss_macsec_init(void)
{
	int ret = 0;

	nss_macsec_netlink_init();
	nss_macsec_mutex_init();

	ret = register_netdevice_notifier(&nss_macsec_netdev_notifier);
	if (ret) {
		osal_print("Failed to register NETDEV notifier, error=%d\n", ret);
		return ret;
	}

	osal_print("nss_macsec init success\n");

	return 0;
}

static void __exit nss_macsec_fini(void)
{
	unregister_netdevice_notifier(&nss_macsec_netdev_notifier);

	nss_macsec_netlink_fini();
	nss_macsec_mutex_destroy();
	qca_macsec_device_cleanup();

	osal_print("nss_macsec exit success\n");
}

module_init(nss_macsec_init);
module_exit(nss_macsec_fini);
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif
