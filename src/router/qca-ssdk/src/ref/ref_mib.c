/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal_misc.h"
#include "fal_mib.h"
#include "fal_port_ctrl.h"
#include "fal_portvlan.h"
#include "fal_fdb.h"
#include "fal_stp.h"
#include "fal_igmp.h"
#include "fal_qos.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//#include <asm/mach-types.h>
#include <generated/autoconf.h>
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#elif defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/switch.h>
#else
#include <net/switch.h>
#include <linux/ar8216_platform.h>
#endif
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include "ssdk_plat.h"
#include "ref_vlan.h"

int
_qca_ar8327_sw_capture_port_counter(struct switch_dev *dev, int port)
{
    int pos = 0;
    fal_mib_info_t  mib_Info;
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

    memset(&mib_Info, 0, sizeof(mib_Info));
    fal_get_mib_info(dev_id, port, &mib_Info);
    pos = port * QCA_MIB_ITEM_NUMBER;
    priv->mib_counters[pos++] += mib_Info.RxBroad;
    priv->mib_counters[pos++] += mib_Info.RxPause;
    priv->mib_counters[pos++] += mib_Info.RxMulti;
    priv->mib_counters[pos++] += mib_Info.RxFcsErr;
    priv->mib_counters[pos++] += mib_Info.RxAllignErr;
    priv->mib_counters[pos++] += mib_Info.RxRunt;
    priv->mib_counters[pos++] += mib_Info.RxFragment;
    priv->mib_counters[pos++] += mib_Info.Rx64Byte;
    priv->mib_counters[pos++] += mib_Info.Rx128Byte;
    priv->mib_counters[pos++] += mib_Info.Rx256Byte;
    priv->mib_counters[pos++] += mib_Info.Rx512Byte;
    priv->mib_counters[pos++] += mib_Info.Rx1024Byte;
    priv->mib_counters[pos++] += mib_Info.Rx1518Byte;
    priv->mib_counters[pos++] += mib_Info.RxMaxByte;
    priv->mib_counters[pos++] += mib_Info.RxTooLong;
    priv->mib_counters[pos] += mib_Info.RxGoodByte_lo;
    priv->mib_counters[pos++] += (((u64)mib_Info.RxGoodByte_hi) << 32);
    priv->mib_counters[pos] += mib_Info.RxBadByte_lo;
    priv->mib_counters[pos++] += (((u64)mib_Info.RxBadByte_hi) << 32);
    priv->mib_counters[pos++] += mib_Info.RxOverFlow;
    priv->mib_counters[pos++] += mib_Info.Filtered;
    priv->mib_counters[pos++] += mib_Info.TxBroad;
    priv->mib_counters[pos++] += mib_Info.TxPause;
    priv->mib_counters[pos++] += mib_Info.TxMulti;
    priv->mib_counters[pos++] += mib_Info.TxUnderRun;
    priv->mib_counters[pos++] += mib_Info.Tx64Byte;
    priv->mib_counters[pos++] += mib_Info.Tx128Byte;
    priv->mib_counters[pos++] += mib_Info.Tx256Byte;
    priv->mib_counters[pos++] += mib_Info.Tx512Byte;
    priv->mib_counters[pos++] += mib_Info.Tx1024Byte;
    priv->mib_counters[pos++] += mib_Info.Tx1518Byte;
    priv->mib_counters[pos++] += mib_Info.TxMaxByte;
    priv->mib_counters[pos++] += mib_Info.TxOverSize;
    priv->mib_counters[pos] += mib_Info.TxByte_lo;
    priv->mib_counters[pos++] += (((u64)mib_Info.TxByte_hi) << 32);
    priv->mib_counters[pos++] += mib_Info.TxCollision;
    priv->mib_counters[pos++] += mib_Info.TxAbortCol;
    priv->mib_counters[pos++] += mib_Info.TxMultiCol;
    priv->mib_counters[pos++] += mib_Info.TxSingalCol;
    priv->mib_counters[pos++] += mib_Info.TxExcDefer;
    priv->mib_counters[pos++] += mib_Info.TxDefer;
    priv->mib_counters[pos++] += mib_Info.TxLateCol;
    priv->mib_counters[pos++] += mib_Info.RxUniCast;
    priv->mib_counters[pos++] += mib_Info.TxUniCast;

    return 0;
}

int
_qca_ar8327_sw_capture_port_rx_counter(struct switch_dev *dev, int port)
{
    int pos = 0;
    fal_mib_info_t  mib_Info;
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

    memset(&mib_Info, 0, sizeof(fal_mib_info_t));
    fal_get_rx_mib_info(dev_id, port, &mib_Info);
    pos = port * QCA_MIB_ITEM_NUMBER;
    priv->mib_counters[pos++] += mib_Info.RxBroad;
    priv->mib_counters[pos++] += mib_Info.RxPause;
    priv->mib_counters[pos++] += mib_Info.RxMulti;
    priv->mib_counters[pos++] += mib_Info.RxFcsErr;
    priv->mib_counters[pos++] += mib_Info.RxAllignErr;
    priv->mib_counters[pos++] += mib_Info.RxRunt;
    priv->mib_counters[pos++] += mib_Info.RxFragment;
    priv->mib_counters[pos++] += mib_Info.Rx64Byte;
    priv->mib_counters[pos++] += mib_Info.Rx128Byte;
    priv->mib_counters[pos++] += mib_Info.Rx256Byte;
    priv->mib_counters[pos++] += mib_Info.Rx512Byte;
    priv->mib_counters[pos++] += mib_Info.Rx1024Byte;
    priv->mib_counters[pos++] += mib_Info.Rx1518Byte;
    priv->mib_counters[pos++] += mib_Info.RxMaxByte;
    priv->mib_counters[pos++] += mib_Info.RxTooLong;
    priv->mib_counters[pos] += mib_Info.RxGoodByte_lo;
    priv->mib_counters[pos++] += (((u64)mib_Info.RxGoodByte_hi) << 32);
    priv->mib_counters[pos] += mib_Info.RxBadByte_lo;
    priv->mib_counters[pos++] += (((u64)mib_Info.RxBadByte_hi) << 32);
    priv->mib_counters[pos++] += mib_Info.RxOverFlow;
    priv->mib_counters[pos++] += mib_Info.Filtered;

    pos = pos + 20;
    priv->mib_counters[pos++] += mib_Info.RxUniCast;

    return 0;
}

int
_qca_ar8327_sw_capture_port_tx_counter(struct switch_dev *dev, int port)
{
    int pos = 0;
    fal_mib_info_t  mib_Info;
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

    memset(&mib_Info, 0, sizeof(fal_mib_info_t));
    fal_get_tx_mib_info(dev_id, port, &mib_Info);
    pos = port * QCA_MIB_ITEM_NUMBER + 19;

    priv->mib_counters[pos++] += mib_Info.TxBroad;
    priv->mib_counters[pos++] += mib_Info.TxPause;
    priv->mib_counters[pos++] += mib_Info.TxMulti;
    priv->mib_counters[pos++] += mib_Info.TxUnderRun;
    priv->mib_counters[pos++] += mib_Info.Tx64Byte;
    priv->mib_counters[pos++] += mib_Info.Tx128Byte;
    priv->mib_counters[pos++] += mib_Info.Tx256Byte;
    priv->mib_counters[pos++] += mib_Info.Tx512Byte;
    priv->mib_counters[pos++] += mib_Info.Tx1024Byte;
    priv->mib_counters[pos++] += mib_Info.Tx1518Byte;
    priv->mib_counters[pos++] += mib_Info.TxMaxByte;
    priv->mib_counters[pos++] += mib_Info.TxOverSize;
    priv->mib_counters[pos] += mib_Info.TxByte_lo;
    priv->mib_counters[pos++] += (((u64)mib_Info.TxByte_hi) << 32);
    priv->mib_counters[pos++] += mib_Info.TxCollision;
    priv->mib_counters[pos++] += mib_Info.TxAbortCol;
    priv->mib_counters[pos++] += mib_Info.TxMultiCol;
    priv->mib_counters[pos++] += mib_Info.TxSingalCol;
    priv->mib_counters[pos++] += mib_Info.TxExcDefer;
    priv->mib_counters[pos++] += mib_Info.TxDefer;
    priv->mib_counters[pos++] += mib_Info.TxLateCol;

    pos++;
    priv->mib_counters[pos++] += mib_Info.TxUniCast;

    return 0;
}

int
qca_ar8327_sw_set_reset_mibs(struct switch_dev *dev,
			 						const struct switch_attr *attr,
			 						struct switch_val *val)
{
    int i = 0;
    int len = 0;
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    a_uint32_t dev_id = qca_devname_2_devid(dev->devname);
    fal_mib_info_t mib_Info;
    len = dev->ports * QCA_MIB_ITEM_NUMBER *
             sizeof(*priv->mib_counters);

    mutex_lock(&priv->mib_lock);
    memset(priv->mib_counters, '\0', len);
    for (i = 0; i < dev->ports; i++)
    {
        fal_get_mib_info(dev_id, i, &mib_Info);
        fal_mib_port_flush_counters(dev_id, i);
    }
    mutex_unlock(&priv->mib_lock);

    return 0;
}

int
qca_ar8327_sw_set_port_reset_mib(struct switch_dev *dev,
			     					const struct switch_attr *attr,
			     					struct switch_val *val)
{
    int len = 0;
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    fal_mib_info_t mib_Info;
    len = QCA_MIB_ITEM_NUMBER * sizeof(*priv->mib_counters);
    a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

    mutex_lock(&priv->mib_lock);

    memset(priv->mib_counters + (val->port_vlan * QCA_MIB_ITEM_NUMBER), '\0', len);

    fal_get_mib_info(dev_id, val->port_vlan, &mib_Info);
    fal_mib_port_flush_counters(dev_id, val->port_vlan);
    mutex_unlock(&priv->mib_lock);

    return 0;
}

int
qca_ar8327_sw_get_port_mib(struct switch_dev *dev,
		       						const struct switch_attr *attr,
		       						struct switch_val *val)
{
    int port = 0;
    int len = 0;
    int pos = 0;

    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    char *buf = (char *)(priv->buf);

    port = val->port_vlan;
    if (port >= dev->ports)
        return -EINVAL;

    mutex_lock(&priv->mib_lock);
    _qca_ar8327_sw_capture_port_counter(dev, port);
    pos = port * QCA_MIB_ITEM_NUMBER;
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "Port %d MIB counters\n",
                            port);

    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxBroad",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxPause",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxMulti",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxFcsErr",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxAlignErr",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxRunt",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxFragment",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Rx64Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Rx128Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Rx256Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Rx512Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Rx1024Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Rx1518Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxMaxByte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxTooLong",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxGoodByte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxBadByte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxOverFlow",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Filtered",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxBroad",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxPause",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxMulti",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxUnderRun",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Tx64Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Tx128Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Tx256Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Tx512Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Tx1024Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "Tx1518Byte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxMaxByte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxOverSize",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxByte",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxCollision",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxAbortCol",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxMultiCol",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxSingleCol",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxExcDefer",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxDefer",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxLateCol",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "RxUniCast",
                            priv->mib_counters[pos++]);
    len += snprintf(buf + len, sizeof(priv->buf) - len,
                            "%-12s: %llu\n",
                            "TxUniCast",
                            priv->mib_counters[pos++]);
    mutex_unlock(&priv->mib_lock);

    val->value.s = buf;
    val->len = len;

    return 0;
}

void
qca_ar8327_sw_mib_task(struct switch_dev *dev)
{
	int i = 0;
	static int loop = 0;
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);

	mutex_lock(&priv->reg_mutex);
	if ((loop % 2) == 0)
		_qca_ar8327_sw_capture_port_rx_counter(dev, loop/2);
	else
		_qca_ar8327_sw_capture_port_tx_counter(dev, loop/2);

	if(++loop == (2 * (dev->ports))) {
		loop = 0;
	}

	mutex_unlock(&priv->reg_mutex);

	return;
}


