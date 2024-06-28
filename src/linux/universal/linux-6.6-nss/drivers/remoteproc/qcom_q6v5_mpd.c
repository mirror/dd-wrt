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

#define BUF_SIZE				35

#define MAX_FIRMWARE			3
/**
 * enum state - state of a wcss (private)
 * @WCSS_NORMAL: subsystem is operating normally
 * @WCSS_SHUTDOWN: subsystem has been shutdown
 *
 */
enum q6_wcss_state {
	WCSS_NORMAL,
	WCSS_SHUTDOWN,
};

enum {
	Q6_IPQ,
	WCSS_AHB_IPQ,
	WCSS_PCIE_IPQ,
};

struct q6_wcss {
	struct device *dev;
	struct qcom_rproc_glink glink_subdev;
	struct qcom_rproc_ssr ssr_subdev;
	struct qcom_q6v5 q6;
	phys_addr_t mem_phys;
	phys_addr_t mem_reloc;
	void *mem_region;
	size_t mem_size;
	int crash_reason_smem;
	s8 pd_asid;
	enum q6_wcss_state state;
	const struct wcss_data *desc;
	const char **firmware;
};

struct wcss_data {
	int (*init_irq)(struct qcom_q6v5 *q6, struct platform_device *pdev,
			struct rproc *rproc, int crash_reason,
			const char *load_state,
			void (*handover)(struct qcom_q6v5 *q6));
	int crash_reason_smem;
	u32 version;
	const char *ssr_name;
	const struct rproc_ops *ops;
	bool glink_subdev_required;
	bool reset_seq;
	u32 pasid;
	int (*mdt_load_sec)(struct device *dev, const struct firmware *fw,
			    const char *fw_name, int pas_id, void *mem_region,
			    phys_addr_t mem_phys, size_t mem_size,
			    phys_addr_t *reloc_base);
	int (*powerup_scm)(u32 peripheral);
	int (*powerdown_scm)(u32 peripheral);
};

/**
 * qcom_get_pd_asid() - get the pd asid number from DT node
 * @node:	device tree node
 *
 * Returns asid if node name has 'pd' string
 */
s8 qcom_get_pd_asid(struct device_node *node)
{
	char *str;
	s8 pd_asid;

	if (!node)
		return -EINVAL;

	str = strstr(node->name, "pd");
	if (!str)
		return 0;

	str += strlen("pd") + 1;
	return kstrtos8(str, 10, &pd_asid) ? -EINVAL : pd_asid;
}
EXPORT_SYMBOL(qcom_get_pd_asid);

static int q6_wcss_start(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;
	int ret;
	struct device_node *upd_np;
	struct platform_device *upd_pdev;
	struct rproc *upd_rproc;
	struct q6_wcss *upd_wcss;
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

	/* On rootpd restart still user pd wcss state's
	 * initialized to WCSS_SHUTDOWN and it leads to
	 * user pd FW load (user pd fw load should happen
	 * only on user pd restart, not on root pd restart).
	 * So bring userpd wcss state to default value.
	 */
	for_each_available_child_of_node(wcss->dev->of_node, upd_np) {
		upd_pdev = of_find_device_by_node(upd_np);
		if (!upd_pdev)
			continue;
		upd_rproc = platform_get_drvdata(upd_pdev);
		upd_wcss = upd_rproc->priv;
		upd_wcss->state = WCSS_NORMAL;
	}
	return ret;
}

static int q6_wcss_spawn_pd(struct rproc *rproc)
{
	int ret;
	struct q6_wcss *wcss = rproc->priv;

	ret = qcom_q6v5_request_spawn(&wcss->q6);
	if (ret == -ETIMEDOUT) {
		pr_err("%s spawn timedout\n", rproc->name);
		return ret;
	}

	ret = qcom_q6v5_wait_for_start(&wcss->q6, msecs_to_jiffies(10000));
	if (ret == -ETIMEDOUT) {
		pr_err("%s start timedout\n", rproc->name);
		wcss->q6.running = false;
		return ret;
	}
	wcss->q6.running = true;
	return ret;
}

static int wcss_ahb_pcie_pd_start(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;
	const struct wcss_data *desc = wcss->desc;
	int ret;

	if (!desc->reset_seq)
		return 0;

	if (desc->powerup_scm) {
		ret = desc->powerup_scm(desc->pasid);
		if (ret) {
			dev_err(wcss->dev, "failed to power up pd\n");
			return ret;
		}
	}

	ret = q6_wcss_spawn_pd(rproc);
	if (ret)
		return ret;

	wcss->state = WCSS_NORMAL;
	return ret;
}

static int q6_wcss_stop(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;
	const struct wcss_data *desc = wcss->desc;
	int ret;

	ret = qcom_scm_pas_shutdown(desc->pasid);
	if (ret) {
		dev_err(wcss->dev, "not able to shutdown\n");
		return ret;
	}
	qcom_q6v5_unprepare(&wcss->q6);

	return 0;
}

static int wcss_ahb_pcie_pd_stop(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;
	struct rproc *rpd_rproc = dev_get_drvdata(wcss->dev->parent);
	const struct wcss_data *desc = wcss->desc;
	int ret;

	if (!desc->reset_seq)
		goto shut_down_rpd;

	if (rproc->state != RPROC_CRASHED && wcss->q6.stop_bit) {
		ret = qcom_q6v5_request_stop(&wcss->q6, NULL);
		if (ret) {
			dev_err(&rproc->dev, "pd not stopped\n");
			return ret;
		}
	}

	if (desc->powerdown_scm) {
		ret = desc->powerdown_scm(desc->pasid);
		if (ret) {
			dev_err(wcss->dev, "failed to power down pd\n");
			return ret;
		}
	}

shut_down_rpd:
	rproc_shutdown(rpd_rproc);

	wcss->state = WCSS_SHUTDOWN;
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

static int q6_wcss_load(struct rproc *rproc, const struct firmware *fw)
{
	struct q6_wcss *wcss = rproc->priv;
	const struct firmware *fw_hdl;
	int ret;
	const struct wcss_data *desc = wcss->desc;
	int loop;

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

/* This function load's userpd firmware. Since Userpd depends on rootpd
 * first bring up root pd and then load. User pd firmware load is required
 * only during user pd restart because root pd loads user pd FW pil segments
 * during it's bringup.
 */
static int wcss_ahb_pcie_pd_load(struct rproc *rproc, const struct firmware *fw)
{
	struct q6_wcss *wcss = rproc->priv, *wcss_rpd;
	struct rproc *rpd_rproc = dev_get_drvdata(wcss->dev->parent);
	const struct wcss_data *desc = wcss->desc;
	int ret;

	wcss_rpd = rpd_rproc->priv;

	/* Boot rootpd rproc */
	ret = rproc_boot(rpd_rproc);
	if (ret || wcss->state == WCSS_NORMAL)
		return ret;

	return desc->mdt_load_sec(wcss->dev, fw, rproc->firmware,
				  desc->pasid, wcss->mem_region,
				  wcss->mem_phys, wcss->mem_size,
				  &wcss->mem_reloc);
}

static unsigned long q6_wcss_panic(struct rproc *rproc)
{
	struct q6_wcss *wcss = rproc->priv;

	return qcom_q6v5_panic(&wcss->q6);
}

static const struct rproc_ops wcss_ahb_pcie_ipq5018_ops = {
	.start = wcss_ahb_pcie_pd_start,
	.stop = wcss_ahb_pcie_pd_stop,
	.load = wcss_ahb_pcie_pd_load,
};

static const struct rproc_ops q6_wcss_ipq5018_ops = {
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
	const struct wcss_data *desc = wcss->desc;

	if (desc->version == Q6_IPQ) {
		node = of_parse_phandle(dev->of_node, "memory-region", 0);
		if (node)
			rmem = of_reserved_mem_lookup(node);

		of_node_put(node);

		if (!rmem) {
			dev_err(dev, "unable to acquire memory-region\n");
			return -EINVAL;
		}
	} else {
		struct rproc *rpd_rproc = dev_get_drvdata(dev->parent);
		struct q6_wcss *rpd_wcss = rpd_rproc->priv;

		wcss->mem_phys = rpd_wcss->mem_phys;
		wcss->mem_reloc = rpd_wcss->mem_reloc;
		wcss->mem_size = rpd_wcss->mem_size;
		wcss->mem_region = rpd_wcss->mem_region;
		return 0;
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
	struct q6_wcss *wcss = q6->rproc->priv;

	irq = platform_get_irq(pdev, index);
	if (irq < 0)
		return irq;

	*pirq = irq;

	interrupt = devm_kzalloc(&pdev->dev, BUF_SIZE, GFP_KERNEL);
	if (!interrupt)
		return -ENOMEM;

	snprintf(interrupt, BUF_SIZE, "q6v5_wcss_userpd%d_%s", wcss->pd_asid, tmp);

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
	if (IS_ERR(tmp_state)) {
		dev_err(&pdev->dev, "failed to acquire %s state\n", int_name);
		return PTR_ERR(tmp_state);
	}

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
	struct q6_wcss *wcss = rproc->priv;

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
	wcss->pd_asid = qcom_get_pd_asid(pdev->dev.of_node);

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

static int q6_wcss_probe(struct platform_device *pdev)
{
	const struct wcss_data *desc;
	struct q6_wcss *wcss;
	struct rproc *rproc;
	int ret;
	char *subdev_name;
	const char **firmware;

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

	rproc = rproc_alloc(&pdev->dev, pdev->name, desc->ops,
			    firmware[0], sizeof(*wcss));
	if (!rproc) {
		dev_err(&pdev->dev, "failed to allocate rproc\n");
		return -ENOMEM;
	}
	wcss = rproc->priv;
	wcss->dev = &pdev->dev;
	wcss->desc = desc;
	wcss->firmware = firmware;

	ret = q6_alloc_memory_region(wcss);
	if (ret)
		goto free_rproc;

	wcss->pd_asid = qcom_get_pd_asid(wcss->dev->of_node);
	if (wcss->pd_asid < 0)
		goto free_rproc;

	if (desc->init_irq) {
		ret = desc->init_irq(&wcss->q6, pdev, rproc,
				     desc->crash_reason_smem, NULL, NULL);
		if (ret)
			goto free_rproc;
	}

	if (desc->glink_subdev_required)
		qcom_add_glink_subdev(rproc, &wcss->glink_subdev, desc->ssr_name);

	subdev_name = (char *)(desc->ssr_name ? desc->ssr_name : pdev->name);
	qcom_add_ssr_subdev(rproc, &wcss->ssr_subdev, subdev_name);

	rproc->auto_boot = false;
	ret = rproc_add(rproc);
	if (ret)
		goto free_rproc;

	platform_set_drvdata(pdev, rproc);

	ret = of_platform_populate(wcss->dev->of_node, NULL,
				   NULL, wcss->dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to populate wcss pd nodes\n");
		goto free_rproc;
	}
	return 0;

free_rproc:
	rproc_free(rproc);

	return ret;
}

static int q6_wcss_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);

	rproc_del(rproc);
	rproc_free(rproc);

	return 0;
}

static const struct wcss_data q6_ipq5018_res_init = {
	.init_irq = qcom_q6v5_init,
	.crash_reason_smem = WCSS_CRASH_REASON,
	.ssr_name = "q6wcss",
	.ops = &q6_wcss_ipq5018_ops,
	.version = Q6_IPQ,
	.glink_subdev_required = true,
	.pasid = MPD_WCNSS_PAS_ID,
};

static const struct wcss_data q6_ipq9574_res_init = {
	.init_irq = qcom_q6v5_init,
	.crash_reason_smem = WCSS_CRASH_REASON,
	.ssr_name = "q6wcss",
	.ops = &q6_wcss_ipq5018_ops,
	.version = Q6_IPQ,
	.glink_subdev_required = true,
	.pasid = WCNSS_PAS_ID,
};

static const struct wcss_data wcss_ahb_ipq5018_res_init = {
	.init_irq = init_irq,
	.crash_reason_smem = WCSS_CRASH_REASON,
	.ops = &wcss_ahb_pcie_ipq5018_ops,
	.version = WCSS_AHB_IPQ,
	.pasid = MPD_WCNSS_PAS_ID,
	.reset_seq = true,
	.mdt_load_sec = qcom_mdt_load_pd_seg,
	.powerup_scm = qcom_scm_pas_power_up,
	.powerdown_scm = qcom_scm_pas_power_down,
};

static const struct wcss_data wcss_ahb_ipq9574_res_init = {
	.crash_reason_smem = WCSS_CRASH_REASON,
	.ops = &wcss_ahb_pcie_ipq5018_ops,
	.version = WCSS_AHB_IPQ,
	.pasid = WCNSS_PAS_ID,
	.mdt_load_sec = qcom_mdt_load,
};

static const struct wcss_data wcss_pcie_ipq5018_res_init = {
	.init_irq = init_irq,
	.crash_reason_smem = WCSS_CRASH_REASON,
	.ops = &wcss_ahb_pcie_ipq5018_ops,
	.version = WCSS_PCIE_IPQ,
	.reset_seq = true,
	.mdt_load_sec = qcom_mdt_load_pd_seg,
	.pasid = MPD_WCNSS_PAS_ID,
};

static const struct of_device_id q6_wcss_of_match[] = {
	{ .compatible = "qcom,ipq5018-q6-mpd", .data = &q6_ipq5018_res_init },
	{ .compatible = "qcom,ipq9574-q6-mpd", .data = &q6_ipq9574_res_init },
	{ .compatible = "qcom,ipq5018-wcss-ahb-mpd",
		.data = &wcss_ahb_ipq5018_res_init },
	{ .compatible = "qcom,ipq9574-wcss-ahb-mpd",
		.data = &wcss_ahb_ipq9574_res_init },
	{ .compatible = "qcom,ipq5018-wcss-pcie-mpd",
		.data = &wcss_pcie_ipq5018_res_init },
	{ },
};
MODULE_DEVICE_TABLE(of, q6_wcss_of_match);

static struct platform_driver q6_wcss_driver = {
	.probe = q6_wcss_probe,
	.remove = q6_wcss_remove,
	.driver = {
		.name = "qcom-q6-mpd",
		.of_match_table = q6_wcss_of_match,
	},
};
module_platform_driver(q6_wcss_driver);

MODULE_DESCRIPTION("Hexagon WCSS Multipd Peripheral Image Loader");
MODULE_LICENSE("GPL v2");
