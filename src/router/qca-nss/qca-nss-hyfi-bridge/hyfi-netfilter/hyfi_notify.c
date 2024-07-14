/*
 *  QCA HyFi Notify
 *
 * Copyright (c) 2012-2014, 2016, The Linux Foundation. All rights reserved.
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

#define DEBUG_LEVEL HYFI_NF_DEBUG_LEVEL

#include <linux/kernel.h>
#include <linux/rtnetlink.h>
#include <net/net_namespace.h>
#include "hyfi_netlink.h"
#include "hyfi_bridge.h"
#include "hyfi_hatbl.h"
#include "mc_snooping2.h"
/* ref_port_ctrl.h and ref_fdb.h header file is  platform dependent code and this
   is not required for 3rd party platform. So avoided this header file inclusion
   by the flag HYFI_DISABLE_SSDK_SUPPORT
 */
#ifndef HYFI_DISABLE_SSDK_SUPPORT
#include "ref/ref_port_ctrl.h"
#include "ref/ref_fdb.h"
#endif

static int hyfi_device_event(struct notifier_block *unused, unsigned long event,
		void *ptr);

static struct notifier_block hyfi_device_notifier = { .notifier_call =
		hyfi_device_event };
#ifndef HYFI_DISABLE_SSDK_SUPPORT
static int hyfi_device_link_event(struct notifier_block *unused, unsigned long event,
                void *ptr);

static struct notifier_block hyfi_device_link_notifier = { .notifier_call =
                hyfi_device_link_event };

/*
 * Handle changes of per port link state information for ethernet interface
 */
static int hyfi_device_link_event(struct notifier_block *unused, unsigned long event,
		void *ptr)
{
	struct net_device *dev;
	struct hyfi_net_bridge *hyfi_br = NULL;
	ssdk_port_status *link_status_p;
	u_int8_t portstatus_event;
	/* switch port doesn't aware of netdevice, hence br-lan netdevice used,
	   So, looping from the first net_device found until hyfi linux_bridge found*/
	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);
	while (dev)
	{
		hyfi_br = hyfi_bridge_get_by_dev(dev);
		if (hyfi_br)
		{
			break;
		}
		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);
	link_status_p = ptr;
	portstatus_event = link_status_p->port_link ? HYFI_EVENT_LINK_PORT_UP : HYFI_EVENT_LINK_PORT_DOWN;
	if (hyfi_br)
	{
		hyfi_netlink_event_send(hyfi_br, portstatus_event, sizeof(ssdk_port_status), link_status_p);
	}
	return NOTIFY_DONE;
}
#endif


/*
 * @brief send interface bridge_join/leave event to application
 *
 * @param event [in] - type of event BR_JOIN/BR_LEAVE
 * @param ptr   [in] - data pointer which includes net_dev of wds_ext iface
 * @param hyfi_bridge [in] - bridge on which wds_ext iface is added or removed
 *
 */
static void hyfi_wdsExt_device_event(unsigned long event,
        void *ptr, struct hyfi_net_bridge *hyfi_bridge)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
#else
    struct net_device *dev = ptr;
#endif
    u_int32_t device_event;
    u_int32_t sys_index = 0;
    char buf[256] = {0};
    struct hyfi_net_bridge *hyfi_br;

    /* Application will take care of handling invalid bridge sys index */
    if (hyfi_bridge)
        sys_index = hyfi_bridge->dev->ifindex;

    /*NL event is registered only with primary bridge so, send event with primary bridge*/
    hyfi_br = hyfi_bridge_get_first_br();

    if (!hyfi_br || hyfi_br->event_pid == NLEVENT_INVALID_PID) {
        DEBUG_TRACE("Invalid event PID. wds_ext iface event failed\n");
        return;
    }

    switch (event) {
        case NETDEV_BR_JOIN:
        case NETDEV_BR_LEAVE:
            DEBUG_INFO("%s: iface %s %s %s:%d\n",__func__, dev->name,
                    (event == NETDEV_BR_JOIN ? "JOINED" : "LEAVED"),
                    hyfi_br->dev->name, sys_index);

            device_event = (event == NETDEV_BR_JOIN) ?
                HYFI_EVENT_BR_JOIN : HYFI_EVENT_BR_LEAVE;
            memcpy(buf, dev->name, IFNAMSIZ);
            memcpy((char *)(&buf[0] + IFNAMSIZ), (char *)(&sys_index), sizeof(u_int32_t));

            /* Send a link change notification */
            hyfi_netlink_event_send(hyfi_br, device_event, IFNAMSIZ+sizeof(u_int32_t), buf);
            break;

        default:
            return;
    }

    return;
}

/*
 * Handle changes in state of network devices enslaved to a bridge.
 */
static int hyfi_device_event(struct notifier_block *unused, unsigned long event,
		void *ptr)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
#else
	struct net_device *dev = ptr;
#endif
	struct net_bridge_port *p = hyfi_br_port_get(dev);
	struct net_bridge *br;
	struct hyfi_net_bridge *hyfi_br = hyfi_bridge_get_by_dev(dev);
	u_int32_t device_event;

	if (strstr(dev->name, ".sta") != NULL) {
		/* port is NULL during BR_JOIN & it is not required for wdsExt event.
		 so using separate function for wdsExt event */
		hyfi_wdsExt_device_event(event, ptr, hyfi_br);
	}

	if (!hyfi_br)
		return NOTIFY_DONE;

	/* A bridge event */
	if (!hyfi_bridge_dev_event(hyfi_br, event, dev))
		return NOTIFY_DONE;

	/* Not a port of a bridge */
	if (p == NULL) {
	    return NOTIFY_DONE;
	}

	br = p->br;

	/* Not our bridge */
	if (br->dev != hyfi_br->dev)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_UP:
	case NETDEV_DOWN:
		device_event = (event == NETDEV_UP) ?
			HYFI_EVENT_LINK_UP : HYFI_EVENT_LINK_DOWN;
		/* Send a link change notification */
		hyfi_netlink_event_send(hyfi_br, device_event, sizeof(u_int32_t), p);
		break;
	case NETDEV_CHANGE:
		device_event =
				netif_carrier_ok(dev) ?
			HYFI_EVENT_LINK_UP : HYFI_EVENT_LINK_DOWN;
		/* Send a link change notification */
		hyfi_netlink_event_send(hyfi_br, device_event, sizeof(u_int32_t), p);
		break;

	case NETDEV_FEAT_CHANGE:
		break;

	case NETDEV_UNREGISTER:
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

void hyfi_br_notify(int group, int event, const void *ptr)
{
	struct hyfi_net_bridge *hf_br = NULL;

	switch (group) {
		case RTNLGRP_LINK:
		{
			struct net_bridge_port *p = (struct net_bridge_port *)ptr;
			if (p)
				hf_br = hyfi_bridge_get(p->br);

			if (hf_br == NULL)
				return;

			switch (event) {
			case RTM_NEWLINK: {
				hyfi_bridge_init_port(hf_br, p);
				break;
			}

			case RTM_DELLINK: {
#ifndef DISABLE_APS_HOOKS
				hyfi_hdtbl_delete_by_port(hf_br, p);
				hyfi_hatbl_delete_by_port(hf_br, p);
#endif
				hyfi_bridge_delete_port(hf_br, p);

				mc_nbp_change(hf_br, p, event);
				break;
			}

			default:
				break;
			}
		}
		break;

		case RTNLGRP_NEIGH:
		{
			struct net_bridge_fdb_entry *fdb = (struct net_bridge_fdb_entry *)ptr;

			if (fdb && fdb->dst)
				hf_br = hyfi_bridge_get(fdb->dst->br);

			if (hf_br == NULL)
				return;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			mc_fdb_change(hf_br, fdb->key.addr.addr, event);
#else
			mc_fdb_change(hf_br, fdb->addr.addr, event);
#endif
		}
		break;

		default:
			break;
	}
}

int __init hyfi_notify_init(void)
{
	int ret;
	ret = register_netdevice_notifier(&hyfi_device_notifier);
	rcu_assign_pointer(br_notify_hook, hyfi_br_notify);

	if (ret) {
		DEBUG_ERROR("hyfi: Failed to register to netdevice notifier\n" );
	}
/* ssdk support is platform dependent code and this is not required for 3rd party platform.
   so avoided the ssdk support by using the flag HYFI_DISABLE_SSDK_SUPPORT.
 */
#ifndef HYFI_DISABLE_SSDK_SUPPORT
	ret = ssdk_port_link_notify_register(&hyfi_device_link_notifier);
	if (ret < 0) {
		DEBUG_ERROR("hyfi: Failed to register to ssdk_port_link notifier\n" );
	}
#endif
	return ret;
}

void hyfi_notify_fini(void)
{
	unregister_netdevice_notifier(&hyfi_device_notifier);
    rcu_assign_pointer(br_notify_hook, NULL);
/* ssdk support is platform dependent code and this is not required for 3rd party platform.
   so avoided the ssdk support by using the flag HYFI_DISABLE_SSDK_SUPPORT.
 */
#ifndef HYFI_DISABLE_SSDK_SUPPORT
	ssdk_port_link_notify_unregister(&hyfi_device_link_notifier);
#endif
}
