// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Linaro Ltd.
 * Copyright (C) 2014 Sony Mobile Communications AB
 * Copyright (c) 2012-2018, 2021 The Linux Foundation. All rights reserved.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/firmware/qcom/qcom_scm.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/soc/qcom/mdt_loader.h>
#include <linux/soc/qcom/smem.h>
#include <linux/soc/qcom/smem_state.h>
#include "qcom_common.h"
#include "qcom_q6v5.h"

#include "remoteproc_internal.h"

#define WCSS_CRASH_REASON		421
#define WCSS_SMEM_HOST			1

#define WCNSS_PAS_ID			6
#define MPD_WCNSS_PAS_ID        0xD

#define BUF_SIZE			35

#define MAX_UPD				3
#define MAX_FIRMWARE			3

#define RPD_SWID		MPD_WCNSS_PAS_ID
#define UPD_SWID		0x12
#define REMOTE_PID			1
#define UPD_BOOT_INFO_SMEM_SIZE		4096
#define UPD_BOOT_INFO_HEADER_TYPE	0x2
#define UPD_BOOT_INFO_SMEM_ID		507

enum q6_bootargs_version {
	VERSION1 = 1,
	VERSION2,
};

/**
 * struct userpd_boot_info_header - header of user pd bootinfo
 * @type:		type of bootinfo passing over smem
 * @length:		length of header in bytes
 */
struct userpd_boot_info_header {
	u8 type;
	u8 length;
};

/**
 * struct userpd_boot_info - holds info required to boot user pd
 * @header:		pointer to header
 * @pid:		unique id represents each user pd process
 * @bootaddr:		load address of user pd firmware
 * @data_size:		user pd firmware memory size
 */
struct userpd_boot_info {
	struct userpd_boot_info_header header;
	u8 pid;
	u32 bootaddr;
	u32 data_size;
} __packed;

struct q6_wcss {
	struct device *dev;
	struct qcom_rproc_glink glink_subdev;
	struct qcom_rproc_ssr ssr_subdev;
	struct qcom_q6v5 q6;
	phys_addr_t mem_phys;
	phys_addr_t mem_reloc;
	void *mem_region;
	size_t mem_size;
	struct clk_bulk_data *clks;
	int num_clks;
	const struct wcss_data *desc;
	const char **firmware;
	struct userpd *upd[MAX_UPD];
};

struct userpd {
	u8 pd_asid;
	struct device *dev;
	struct qcom_rproc_ssr ssr_subdev;
	struct qcom_q6v5 q6;
};

struct wcss_data {
	u32 pasid;
	bool share_upd_info_to_q6;
	u8 bootargs_version;
};

/**
 * qcom_get_pd_asid() - get the pd asid number from PD spawn bit
 * @rproc:	rproc handle
 *
 * Returns asid on success
 */
static u8 qcom_get_pd_asid(struct rproc *rproc)
{
	struct userpd *upd = rproc->priv;
	u8 bit = upd->q6.spawn_bit;

	return bit / 8;
}

static int q6_wcss_start(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;
	int ret;
	const struct wcss_data *desc = wcss->desc;

	qcom_q6v5_prepare(&wcss->q6);

	ret = qcom_scm_pas_auth_and_reset(desc->pasid);
	if (ret) {
		dev_err(wcss->dev, "wcss_reset failed\n");
		return ret;
	}

	ret = qcom_q6v5_wait_for_start(&wcss->q6, 5 * HZ);
	if (ret == -ETIMEDOUT)
		dev_err(wcss->dev, "start timed out\n");

	return ret;
}

static int q6_wcss_spawn_pd(struct rproc *rproc)
{
	int ret;
	struct userpd *upd = rproc->priv;

	ret = qcom_q6v5_request_spawn(&upd->q6);
	if (ret == -ETIMEDOUT) {
		dev_err(upd->dev, "%s spawn timedout\n", rproc->name);
		return ret;
	}

	ret = qcom_q6v5_wait_for_start(&upd->q6, msecs_to_jiffies(10000));
	if (ret == -ETIMEDOUT) {
		dev_err(upd->dev, "%s start timedout\n", rproc->name);
		upd->q6.running = false;
		return ret;
	}
	upd->q6.running = true;
	return ret;
}

static int wcss_pd_start(struct rproc *rproc)
{
	struct userpd *upd = rproc->priv;
	struct rproc *rpd_rproc = dev_get_drvdata(upd->dev->parent);
	struct q6_wcss *wcss = rpd_rproc->priv;
	u32 pasid = (upd->pd_asid << 8) | UPD_SWID;
	int ret;

	ret = qcom_scm_msa_lock(pasid);
	if (ret) {
		dev_err(upd->dev, "failed to power up pd\n");
		return ret;
	}

	if (upd->q6.spawn_bit) {
		ret = q6_wcss_spawn_pd(rproc);
		if (ret)
			return ret;
	}

	if (upd->pd_asid == 1) {
		ret = qcom_scm_internal_wifi_powerup(wcss->desc->pasid);
		if (ret) {
			dev_err(upd->dev, "failed to power up internal radio\n");
			return ret;
		}
	}

	return ret;
}

static int q6_wcss_stop(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;
	const struct wcss_data *desc = wcss->desc;
	int ret;

	ret = qcom_q6v5_request_stop(&wcss->q6, NULL);
	if (ret) {
		dev_err(wcss->dev, "pd not stopped\n");
		return ret;
	}

	ret = qcom_scm_pas_shutdown(desc->pasid);
	if (ret) {
		dev_err(wcss->dev, "not able to shutdown\n");
		return ret;
	}
	qcom_q6v5_unprepare(&wcss->q6);

	return 0;
}

/**
 * wcss_pd_stop() - Stop WCSS user pd
 * @rproc:	rproc handle
 *
 * Stop root pd after user pd down. Root pd
 * is used to provide services to user pd, so
 * keeping root pd alive when user pd is down
 * is invalid.
 * ---------------------------------------------
 *
 *				-----------
 *		     |-------->| User PD1 |
 *		     |		-----------
 *		     |
 *		     |
 *	-----	     |		-----------
 *	| Q6 |---------------->| User Pd2 |
 *	-----	     |		-----------
 *		     |
 *		     |
 *		     |		-----------
 *		     |--------->| User Pd3 |
 *				-----------
 * ----------------------------------------------
 */
static int wcss_pd_stop(struct rproc *rproc)
{
	struct userpd *upd = rproc->priv;
	struct rproc *rpd_rproc = dev_get_drvdata(upd->dev->parent);
	struct q6_wcss *wcss = rpd_rproc->priv;
	u32 pasid = (upd->pd_asid << 8) | UPD_SWID;
	int ret;

	if (rproc->state != RPROC_CRASHED && upd->q6.stop_bit) {
		ret = qcom_q6v5_request_stop(&upd->q6, NULL);
		if (ret) {
			dev_err(upd->dev, "pd not stopped\n");
			return ret;
		}
	}

	if (upd->pd_asid == 1) {
		ret = qcom_scm_internal_wifi_shutdown(wcss->desc->pasid);
		if (ret) {
			dev_err(upd->dev, "failed to power down internal radio\n");
			return ret;
		}
	}

	ret = qcom_scm_msa_unlock(pasid);
	if (ret) {
		dev_err(upd->dev, "failed to power down pd\n");
		return ret;
	}

	rproc_shutdown(rpd_rproc);

	return 0;
}

static void *q6_wcss_da_to_va(struct rproc *rproc, u64 da, size_t len,
			      bool *is_iomem)
{
	struct q6_wcss *wcss = rproc->priv;
	int offset;

	offset = da - wcss->mem_reloc;
	if (offset < 0 || offset + len > wcss->mem_size)
		return NULL;

	return wcss->mem_region + offset;
}

/**
 * share_upd_bootinfo_to_q6() - Share userpd boot info to Q6 root pd
 * @rproc:	rproc handle
 *
 * Q6 needs user pd parameters like loadaddress and
 * PIL size to authenticate user pd with underlying
 * security software. If authenticatoin success then
 * only Q6 spawns user pd and sends spawn ack to rproc
 * driver. This API is passing userpd boot info to Q6
 * over SMEM.
 *
 * User pd boot-info format mentioned below
 * <Version> <No of elements passing over smem> <Header type> <Header Length>
 * <Process Id> <Load address> <firmware mem Size>
 *
 * Returns 0 on success else negative value on failure.
 */
static int share_upd_bootinfo_to_q6(struct rproc *rproc)
{
	int i, ret;
	u32 rd_val;
	size_t size;
	u16 cnt = 0, version;
	void *ptr;
	u8 *bootargs_arr;
	struct q6_wcss *wcss = rproc->priv;
	struct device_node *np = wcss->dev->of_node;
	struct userpd *upd;
	struct userpd_boot_info upd_bootinfo = {0};
	const struct firmware *fw;

	ret = qcom_smem_alloc(REMOTE_PID, UPD_BOOT_INFO_SMEM_ID,
			      UPD_BOOT_INFO_SMEM_SIZE);
	if (ret && ret != -EEXIST) {
		dev_err(wcss->dev,
			"failed to allocate q6 bootinfo smem segment\n");
		return ret;
	}

	ptr = qcom_smem_get(REMOTE_PID, UPD_BOOT_INFO_SMEM_ID, &size);
	if (IS_ERR(ptr) || size != UPD_BOOT_INFO_SMEM_SIZE) {
		dev_err(wcss->dev,
			"Unable to acquire smp2p item(%d) ret:%ld\n",
			UPD_BOOT_INFO_SMEM_ID, PTR_ERR(ptr));
		return PTR_ERR(ptr);
	}

	/*Version*/
	version = (wcss->desc->bootargs_version) ? wcss->desc->bootargs_version : VERSION2;
	memcpy_toio(ptr, &version, sizeof(version));
	ptr += sizeof(version);

	cnt = ret = of_property_count_u32_elems(np, "boot-args");
	if (ret < 0) {
		if (ret == -ENODATA) {
			dev_err(wcss->dev, "failed to read boot args ret:%d\n", ret);
			return ret;
		}
		cnt = 0;
	}

	/* No of elements */
	memcpy_toio(ptr, &cnt, sizeof(u16));
	ptr += sizeof(u16);

	bootargs_arr = kzalloc(cnt, GFP_KERNEL);
	if (!bootargs_arr) {
		dev_err(wcss->dev, "failed to allocate memory\n");
		return PTR_ERR(bootargs_arr);
	}

	for (i = 0; i < cnt; i++) {
		ret = of_property_read_u32_index(np, "boot-args", i, &rd_val);
		if (ret) {
			dev_err(wcss->dev, "failed to read boot args\n");
			kfree(bootargs_arr);
			return ret;
		}
		bootargs_arr[i] = (u8)rd_val;
	}

	/* Copy bootargs */
	memcpy_toio(ptr, bootargs_arr, cnt);
	ptr += (cnt);

	of_node_put(np);
	kfree(bootargs_arr);
	cnt = 0;

	for (i = 0; i < ARRAY_SIZE(wcss->upd); i++)
		if (wcss->upd[i])
			cnt++;

	/* No of elements */
	cnt = (sizeof(upd_bootinfo) * cnt);
	memcpy_toio(ptr, &cnt, sizeof(u16));
	ptr += sizeof(u16);

	for (i = 0; i < ARRAY_SIZE(wcss->upd); i++) {
		upd = wcss->upd[i];
		if (!upd)
			continue;

		/* TYPE */
		upd_bootinfo.header.type = UPD_BOOT_INFO_HEADER_TYPE;

		/* LENGTH */
		upd_bootinfo.header.length =
			sizeof(upd_bootinfo) - sizeof(upd_bootinfo.header);

		/* Process ID */
		upd_bootinfo.pid = upd->pd_asid + 1;

		ret = request_firmware(&fw, upd->q6.rproc->firmware, upd->dev);
		if (ret < 0) {
			dev_err(upd->dev, "request_firmware failed: %d\n",	ret);
			return ret;
		}

		/* Load address */
		upd_bootinfo.bootaddr = rproc_get_boot_addr(upd->q6.rproc, fw);

		/* Firmware mem size */
		upd_bootinfo.data_size = qcom_mdt_get_size(fw);

		release_firmware(fw);

		/* copy into smem */
		memcpy_toio(ptr, &upd_bootinfo, sizeof(upd_bootinfo));
		ptr += sizeof(upd_bootinfo);
	}
	return 0;
}

static int q6_wcss_load(struct rproc *rproc, const struct firmware *fw)
{
	struct q6_wcss *wcss = rproc->priv;
	const struct firmware *fw_hdl;
	int ret;
	const struct wcss_data *desc = wcss->desc;
	int loop;

	/* Share user pd boot info to Q6 remote processor */
	if (desc->share_upd_info_to_q6) {
		if (of_property_present(wcss->dev->of_node, "boot-args")) {
			ret = share_upd_bootinfo_to_q6(rproc);
			if (ret) {
				dev_err(wcss->dev,
					"user pd boot info sharing with q6 failed %d\n",
					ret);
				return ret;
			}
		}
	}

	ret = qcom_mdt_load(wcss->dev, fw, rproc->firmware,
			    desc->pasid, wcss->mem_region,
			    wcss->mem_phys, wcss->mem_size,
			    &wcss->mem_reloc);
	if (ret)
		return ret;

	for (loop = 1; loop < MAX_FIRMWARE; loop++) {
		if (!wcss->firmware[loop])
			continue;

		ret = request_firmware(&fw_hdl, wcss->firmware[loop],
				       wcss->dev);
		if (ret)
			continue;

		ret = qcom_mdt_load_no_init(wcss->dev, fw_hdl,
					    wcss->firmware[loop], 0,
					    wcss->mem_region,
					    wcss->mem_phys,
					    wcss->mem_size,
					    &wcss->mem_reloc);

		release_firmware(fw_hdl);

		if (ret) {
			dev_err(wcss->dev,
				"can't load %s ret:%d\n", wcss->firmware[loop], ret);
			return ret;
		}
	}
	return 0;
}

/**
 * wcss_pd_load() - Load WCSS user pd firmware
 * @rproc:	rproc handle
 * @fw:		firmware handle
 *
 * User pd get services from root pd. So first
 * bring up root pd and then load userpd firmware.
 * ---------------------------------------------
 *
 *				-----------
 *		     |-------->| User PD1 |
 *		     |		-----------
 *		     |
 *		     |
 *	-----	     |		-----------
 *	| Q6 |---------------->| User Pd2 |
 *	-----	     |		-----------
 *		     |
 *		     |
 *		     |		-----------
 *		     |--------->| User Pd3 |
 *				-----------
 * ----------------------------------------------
 *
 */
static int wcss_pd_load(struct rproc *rproc, const struct firmware *fw)
{
	struct userpd *upd = rproc->priv;
	struct rproc *rpd_rproc = dev_get_drvdata(upd->dev->parent);
	struct q6_wcss *wcss = rpd_rproc->priv;
	int ret;

	ret = rproc_boot(rpd_rproc);
	if (ret)
		return ret;

	return qcom_mdt_load_pd_seg(upd->dev, fw, rproc->firmware,
			     wcss->desc->pasid, upd->pd_asid, wcss->mem_region,
			     wcss->mem_phys, wcss->mem_size,
			     NULL);
}

static unsigned long q6_wcss_panic(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;

	return qcom_q6v5_panic(&wcss->q6);
}

static const struct rproc_ops wcss_ops = {
	.start = wcss_pd_start,
	.stop = wcss_pd_stop,
	.load = wcss_pd_load,
	.get_boot_addr = rproc_elf_get_boot_addr,
};

static const struct rproc_ops q6_wcss_ops = {
	.start = q6_wcss_start,
	.stop = q6_wcss_stop,
	.da_to_va = q6_wcss_da_to_va,
	.load = q6_wcss_load,
	.get_boot_addr = rproc_elf_get_boot_addr,
	.panic = q6_wcss_panic,
};

static int q6_alloc_memory_region(struct q6_wcss *wcss)
{
	struct reserved_mem *rmem = NULL;
	struct device_node *node;
	struct device *dev = wcss->dev;

	node = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (node)
		rmem = of_reserved_mem_lookup(node);

	of_node_put(node);

	if (!rmem) {
		dev_err(dev, "unable to acquire memory-region\n");
		return -EINVAL;
	}

	wcss->mem_phys = rmem->base;
	wcss->mem_reloc = rmem->base;
	wcss->mem_size = rmem->size;
	wcss->mem_region = devm_ioremap_wc(dev, wcss->mem_phys, wcss->mem_size);
	if (!wcss->mem_region) {
		dev_err(dev, "unable to map memory region: %pa+%pa\n",
			&rmem->base, &rmem->size);
		return -EBUSY;
	}

	return 0;
}

static int q6_get_inbound_irq(struct qcom_q6v5 *q6,
			      struct platform_device *pdev,
			      const char *int_name,
			      int index, int *pirq,
			      irqreturn_t (*handler)(int irq, void *data))
{
	int ret, irq;
	char *interrupt, *tmp = (char *)int_name;
	struct userpd *upd = q6->rproc->priv;

	irq = platform_get_irq(pdev, index);
	if (irq < 0)
		return irq;

	*pirq = irq;

	interrupt = devm_kzalloc(&pdev->dev, BUF_SIZE, GFP_KERNEL);
	if (!interrupt)
		return -ENOMEM;

	snprintf(interrupt, BUF_SIZE, "q6v5_wcss_userpd%d_%s", upd->pd_asid, tmp);

	ret = devm_request_threaded_irq(&pdev->dev, *pirq,
					NULL, handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					interrupt, q6);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to acquire %s irq\n", interrupt);
	return 0;
}

static int q6_get_outbound_irq(struct qcom_q6v5 *q6,
			       struct platform_device *pdev,
			       const char *int_name)
{
	struct qcom_smem_state *tmp_state;
	unsigned  bit;

	tmp_state = qcom_smem_state_get(&pdev->dev, int_name, &bit);
	if (IS_ERR(tmp_state))
		return dev_err_probe(&pdev->dev, PTR_ERR(tmp_state),
				     "failed to acquire %s state\n", int_name);

	if (!strcmp(int_name, "stop")) {
		q6->state = tmp_state;
		q6->stop_bit = bit;
	} else if (!strcmp(int_name, "spawn")) {
		q6->spawn_state = tmp_state;
		q6->spawn_bit = bit;
	}

	return 0;
}

static int init_irq(struct qcom_q6v5 *q6,
		    struct platform_device *pdev, struct rproc *rproc,
		    int crash_reason, const char *load_state,
		    void (*handover)(struct qcom_q6v5 *q6))
{
	int ret;
	struct userpd *upd = rproc->priv;

	q6->rproc = rproc;
	q6->dev = &pdev->dev;
	q6->crash_reason = crash_reason;
	q6->handover = handover;

	init_completion(&q6->start_done);
	init_completion(&q6->stop_done);
	init_completion(&q6->spawn_done);

	ret = q6_get_outbound_irq(q6, pdev, "stop");
	if (ret)
		return ret;

	ret = q6_get_outbound_irq(q6, pdev, "spawn");
	if (ret)
		return ret;

	/* Get pd_asid to prepare interrupt names */
	upd->pd_asid = qcom_get_pd_asid(rproc);

	ret = q6_get_inbound_irq(q6, pdev, "fatal", 0, &q6->fatal_irq,
				 q6v5_fatal_interrupt);
	if (ret)
		return ret;

	ret = q6_get_inbound_irq(q6, pdev, "ready", 1, &q6->ready_irq,
				 q6v5_ready_interrupt);
	if (ret)
		return ret;

	ret = q6_get_inbound_irq(q6, pdev, "stop-ack", 3, &q6->stop_irq,
				 q6v5_stop_interrupt);
	if (ret)
		return ret;

	ret = q6_get_inbound_irq(q6, pdev, "spawn-ack", 2, &q6->spawn_irq,
				 q6v5_spawn_interrupt);
	if (ret)
		return ret;
	return 0;
}

static void q6_release_resources(struct q6_wcss *wcss)
{
	struct userpd *upd;
	int i;

	/* Release userpd resources */
	for (i = 0; i < ARRAY_SIZE(wcss->upd); i++) {
		upd = wcss->upd[i];
		if (!upd)
			continue;

		rproc_del(upd->q6.rproc);
		rproc_free(upd->q6.rproc);
	}
}

static int q6_register_userpd(struct q6_wcss *wcss,
			      struct device_node *userpd_np)
{
	struct userpd *upd;
	struct rproc *rproc = NULL;
	int ret;
	struct platform_device *userpd_pdev;
	const char *firmware_name = NULL;
	const char *label = NULL;

	ret = of_property_read_string(userpd_np, "firmware-name",
				      &firmware_name);
	if (ret < 0) {
		/* All userpd's who want to register as rproc must have firmware.
		 * Other than userpd like glink they don't need any firmware.
		 * So for glink child simply return success.
		 */
		if (ret == -EINVAL) {
			/* Confirming userpd_np is glink node or not */
			if (!of_property_read_string(userpd_np, "label", &label))
				return 0;
		}
		return ret;
	}

	dev_info(wcss->dev, "%s node found\n", userpd_np->name);

	userpd_pdev = of_platform_device_create(userpd_np, userpd_np->name,
						wcss->dev);
	if (!userpd_pdev)
		return dev_err_probe(wcss->dev, -ENODEV,
				     "failed to create %s platform device\n",
				     userpd_np->name);

	userpd_pdev->dev.driver = wcss->dev->driver;
	rproc = rproc_alloc(&userpd_pdev->dev, userpd_pdev->name, &wcss_ops,
			    firmware_name, sizeof(*upd));
	if (!rproc) {
		ret = -ENOMEM;
		goto free_rproc;
	}

	upd = rproc->priv;
	upd->dev = &userpd_pdev->dev;

	ret = init_irq(&upd->q6, userpd_pdev, rproc,
		       WCSS_CRASH_REASON, NULL, NULL);
	if (ret)
		goto free_rproc;

	rproc->auto_boot = false;
	ret = rproc_add(rproc);
	if (ret)
		goto free_rproc;

	wcss->upd[upd->pd_asid] = upd;
	platform_set_drvdata(userpd_pdev, rproc);
	qcom_add_ssr_subdev(rproc, &upd->ssr_subdev, userpd_pdev->name);
	return 0;

free_rproc:
	kfree(rproc);
	return ret;
}

static int q6_wcss_probe(struct platform_device *pdev)
{
	const struct wcss_data *desc;
	struct q6_wcss *wcss;
	struct rproc *rproc;
	int ret;
	const char **firmware;
	struct device_node *userpd_np;
	const struct rproc_ops *ops = &q6_wcss_ops;

	desc = of_device_get_match_data(&pdev->dev);
	if (!desc)
		return -EINVAL;

	firmware = devm_kcalloc(&pdev->dev, MAX_FIRMWARE,
				sizeof(*firmware), GFP_KERNEL);
	if (!firmware)
		return -ENOMEM;

	ret = of_property_read_string_array(pdev->dev.of_node, "firmware-name",
					    firmware, MAX_FIRMWARE);
	if (ret < 0)
		return ret;

	rproc = rproc_alloc(&pdev->dev, pdev->name, ops,
			    firmware[0], sizeof(*wcss));
	if (!rproc)
		return -ENOMEM;

	wcss = rproc->priv;
	wcss->dev = &pdev->dev;
	wcss->desc = desc;
	wcss->firmware = firmware;

	ret = q6_alloc_memory_region(wcss);
	if (ret)
		goto free_rproc;

	wcss->num_clks = devm_clk_bulk_get_all(wcss->dev, &wcss->clks);
	if (wcss->num_clks < 0)
		return dev_err_probe(wcss->dev, wcss->num_clks,
				     "failed to acquire clocks\n");

	ret = clk_bulk_prepare_enable(wcss->num_clks, wcss->clks);
	if (ret)
		return dev_err_probe(wcss->dev, ret,
				     "failed to enable clocks\n");

	ret = qcom_q6v5_init(&wcss->q6, pdev, rproc,
			     WCSS_CRASH_REASON, NULL, NULL);
	if (ret)
		goto free_rproc;

	qcom_add_glink_subdev(rproc, &wcss->glink_subdev, "q6wcss");
	qcom_add_ssr_subdev(rproc, &wcss->ssr_subdev, "q6wcss");

	rproc->auto_boot = false;
	ret = rproc_add(rproc);
	if (ret)
		goto free_rproc;

	platform_set_drvdata(pdev, rproc);

	/* Iterate over userpd child's and register with rproc */
	for_each_available_child_of_node(pdev->dev.of_node, userpd_np) {
		ret = q6_register_userpd(wcss, userpd_np);
		if (ret) {
			/* release resources of successfully allocated userpd rproc's */
			q6_release_resources(wcss);
			return dev_err_probe(&pdev->dev, ret,
					     "Failed to register userpd(%s)\n",
					     userpd_np->name);
		}
	}
	return 0;

free_rproc:
	rproc_free(rproc);

	return ret;
}

static void q6_wcss_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct q6_wcss *wcss = rproc->priv;

	qcom_q6v5_deinit(&wcss->q6);

	rproc_del(rproc);
	rproc_free(rproc);
}

static const struct wcss_data q6_ipq5018_res_init = {
	.pasid = MPD_WCNSS_PAS_ID,
	.share_upd_info_to_q6 = true,
	.bootargs_version = VERSION1,
	// .mdt_load_sec = qcom_mdt_load_pd_seg,
};

static const struct wcss_data q6_ipq5332_res_init = {
	.pasid = MPD_WCNSS_PAS_ID,
	.share_upd_info_to_q6 = true,
	.bootargs_version = VERSION2,
};

static const struct wcss_data q6_ipq9574_res_init = {
	.pasid = WCNSS_PAS_ID,
};

static const struct of_device_id q6_wcss_of_match[] = {
	{ .compatible = "qcom,ipq5018-q6-mpd", .data = &q6_ipq5018_res_init },
	{ .compatible = "qcom,ipq5332-q6-mpd", .data = &q6_ipq5332_res_init },
	{ .compatible = "qcom,ipq9574-q6-mpd", .data = &q6_ipq9574_res_init },
	{ },
};
MODULE_DEVICE_TABLE(of, q6_wcss_of_match);

static struct platform_driver q6_wcss_driver = {
	.probe = q6_wcss_probe,
	.remove_new = q6_wcss_remove,
	.driver = {
		.name = "qcom-q6-mpd",
		.of_match_table = q6_wcss_of_match,
	},
};
module_platform_driver(q6_wcss_driver);

MODULE_DESCRIPTION("Hexagon WCSS Multipd Peripheral Image Loader");
MODULE_LICENSE("GPL v2");
