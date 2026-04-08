// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Linaro Ltd.
 * Copyright (C) 2014 Sony Mobile Communications AB
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2024-2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/clk.h>
#include <linux/firmware/qcom/qcom_scm.h>
#include <linux/io.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox/tmelcom-qmp.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/soc/qcom/mdt_loader.h>

#include "qcom_common.h"
#include "qcom_q6v5.h"
#include "qcom_pil_info.h"

#define WCSS_CRASH_REASON		421

#define WCSS_PAS_ID			0x6
#define MPD_WCSS_PAS_ID			0xd

#define Q6_WAIT_TIMEOUT			(5 * HZ)

#define MAX_FIRMWARE			3

struct wcss_sec {
	struct device *dev;
	struct qcom_rproc_glink glink_subdev;
	struct qcom_rproc_ssr ssr_subdev;
	struct qcom_q6v5 q6;
	phys_addr_t mem_phys;
	phys_addr_t mem_reloc;
	void *mem_region;
	size_t mem_size;
	const struct wcss_data *desc;
	const char **firmware;

	struct mbox_client mbox_client;
	struct mbox_chan *mbox_chan;
	void *metadata;
	size_t metadata_len;
};

struct wcss_data {
	u32 pasid;
	const char *ss_name;
	bool auto_boot;
	bool use_tmelcom;
};

static int wcss_sec_start(struct rproc *rproc)
{
	struct wcss_sec *wcss = rproc->priv;
	struct device *dev = wcss->dev;
	int ret;

	ret = qcom_q6v5_prepare(&wcss->q6);
	if (ret)
		return ret;

	if (wcss->desc->use_tmelcom) {
		struct tmel_sec_auth tsa;
		struct tmel_qmp_msg tqm;

		tsa.data = wcss->metadata;
		tsa.size = wcss->metadata_len;
		tsa.pas_id = wcss->desc->pasid;
		tqm.msg = &tsa;
		tqm.msg_id = TMEL_MSG_UID_SECBOOT_SEC_AUTH;

		ret = mbox_send_message(wcss->mbox_chan, (void *)&tqm);
		if (ret < 0) {
			dev_err(dev, "Failed to send message via mailbox\n");
			goto unprepare;
		}
	} else {
		ret = qcom_scm_pas_auth_and_reset(wcss->desc->pasid);
		if (ret) {
			dev_err(dev, "wcss_reset failed\n");
			goto unprepare;
		}
	}

	ret = qcom_q6v5_wait_for_start(&wcss->q6, Q6_WAIT_TIMEOUT);
	if (ret == -ETIMEDOUT)
		dev_err(dev, "start timed out\n");

unprepare:
	qcom_q6v5_unprepare(&wcss->q6);

	return ret;
}

static int wcss_sec_stop(struct rproc *rproc)
{
	struct wcss_sec *wcss = rproc->priv;
	struct device *dev = wcss->dev;
	int ret;

	if (wcss->desc->use_tmelcom) {
		struct tmel_sec_auth tsa = {0};
		struct tmel_qmp_msg tqm;

		tsa.pas_id = wcss->desc->pasid;
		tqm.msg = &tsa;
		tqm.msg_id = TMEL_MSG_UID_SECBOOT_SS_TEAR_DOWN;

		mbox_send_message(wcss->mbox_chan, (void *)&tqm);
	} else {
		ret = qcom_scm_pas_shutdown(wcss->desc->pasid);
		if (ret) {
			dev_err(dev, "not able to shutdown\n");
			return ret;
		}
	}

	qcom_q6v5_unprepare(&wcss->q6);

	return 0;
}

static void *wcss_sec_da_to_va(struct rproc *rproc, u64 da, size_t len,
			       bool *is_iomem)
{
	struct wcss_sec *wcss = rproc->priv;
	int offset;

	offset = da - wcss->mem_reloc;
	if (offset < 0 || offset + len > wcss->mem_size)
		return NULL;

	return wcss->mem_region + offset;
}

static int wcss_sec_load(struct rproc *rproc, const struct firmware *fw)
{
	struct wcss_sec *wcss = rproc->priv;
	struct device *dev = wcss->dev;
	const struct firmware *fw_hdl;
	int i, ret;

	if (wcss->desc->use_tmelcom) {
		wcss->metadata = qcom_mdt_read_metadata(fw, &wcss->metadata_len,
							rproc->firmware, wcss->dev);
		if (IS_ERR(wcss->metadata)) {
			ret = PTR_ERR(wcss->metadata);
			dev_err(wcss->dev, "error %d reading firmware %s metadata\n",
				ret, rproc->firmware);
			return ret;
		}

		ret = qcom_mdt_load_no_init(wcss->dev, fw, rproc->firmware, wcss->desc->pasid,
					    wcss->mem_region, wcss->mem_phys, wcss->mem_size,
					    &wcss->mem_reloc);
		if (ret) {
			kfree(wcss->metadata);
			return ret;
		}
	} else {
		ret = qcom_mdt_load(dev, fw, rproc->firmware, wcss->desc->pasid, wcss->mem_region,
				    wcss->mem_phys, wcss->mem_size, &wcss->mem_reloc);
		if (ret)
			return ret;

		for (i = 1; i < MAX_FIRMWARE; i++) {
			if (!wcss->firmware[i])
				continue;

			ret = request_firmware(&fw_hdl, wcss->firmware[i], dev);

			if (ret)
				continue;

			ret = qcom_mdt_load_no_init(dev, fw_hdl, wcss->firmware[i], 0,
						    wcss->mem_region, wcss->mem_phys,
						    wcss->mem_size, &wcss->mem_reloc);

			release_firmware(fw_hdl);

			if (ret) {
				dev_err(dev, "error %d loading firmware %s\n",
					ret, wcss->firmware[i]);
				return ret;
			}
		}
	}

	qcom_pil_info_store("wcss", wcss->mem_phys, wcss->mem_size);

	return 0;
}

static unsigned long wcss_sec_panic(struct rproc *rproc)
{
	struct wcss_sec *wcss = rproc->priv;

	return qcom_q6v5_panic(&wcss->q6);
}

static void wcss_sec_copy_segment(struct rproc *rproc,
				  struct rproc_dump_segment *segment,
				  void *dest, size_t offset, size_t size)
{
	struct wcss_sec *wcss = rproc->priv;
	struct device *dev = wcss->dev;

	if (!segment->io_ptr)
		segment->io_ptr = ioremap_wc(segment->da, segment->size);

	if (!segment->io_ptr) {
		dev_err(dev, "Failed to ioremap segment %pad size 0x%zx\n",
			&segment->da, segment->size);
		return;
	}

	if (offset + size < segment->size) {
		memcpy(dest, segment->io_ptr + offset, size);
	} else {
		iounmap(segment->io_ptr);
		segment->io_ptr = NULL;
	}
}

static int wcss_sec_dump_segments(struct rproc *rproc,
				  const struct firmware *fw)
{
	struct device *dev = rproc->dev.parent;
	struct reserved_mem *rmem = NULL;
	struct device_node *node;
	int num_segs, index;
	int ret;

	/*
	 * Parse through additional reserved memory regions for the rproc
	 * and add them to the coredump segments
	 */
	num_segs = of_count_phandle_with_args(dev->of_node,
					      "memory-region", NULL);
	for (index = 0; index < num_segs; index++) {
		node = of_parse_phandle(dev->of_node,
					"memory-region", index);
		if (!node)
			return -EINVAL;

		rmem = of_reserved_mem_lookup(node);
		of_node_put(node);
		if (!rmem) {
			dev_err(dev, "unable to acquire memory-region index %d num_segs %d\n",
				index, num_segs);
			return -EINVAL;
		}

		dev_dbg(dev, "Adding segment 0x%pa size 0x%pa",
			&rmem->base, &rmem->size);
		ret = rproc_coredump_add_custom_segment(rproc,
							rmem->base,
							rmem->size,
							wcss_sec_copy_segment,
							NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct rproc_ops wcss_sec_ops = {
	.start = wcss_sec_start,
	.stop = wcss_sec_stop,
	.da_to_va = wcss_sec_da_to_va,
	.load = wcss_sec_load,
	.get_boot_addr = rproc_elf_get_boot_addr,
	.panic = wcss_sec_panic,
	.parse_fw = wcss_sec_dump_segments,
};

static int wcss_sec_alloc_memory_region(struct wcss_sec *wcss)
{
	struct reserved_mem *rmem = NULL;
	struct device_node *node;
	struct device *dev = wcss->dev;

	node = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (!node) {
		dev_err(dev, "can't find phandle memory-region\n");
		return -EINVAL;
	}

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
		return -ENOMEM;
	}

	return 0;
}

static int wcss_sec_probe(struct platform_device *pdev)
{
	struct rproc *rproc;
	struct wcss_sec *wcss;
	struct clk *sleep_clk;
	struct clk *int_clk;
	const char **firmware = NULL;
	const struct wcss_data *desc = of_device_get_match_data(&pdev->dev);
	int ret;

	firmware = devm_kcalloc(&pdev->dev, MAX_FIRMWARE,
				sizeof(*firmware), GFP_KERNEL);

	ret = of_property_read_string_array(pdev->dev.of_node, "firmware-name",
					    firmware, MAX_FIRMWARE);
	if (ret < 0)
		return ret;

	rproc = devm_rproc_alloc(&pdev->dev, desc->ss_name, &wcss_sec_ops,
				 firmware[0], sizeof(*wcss));
	if (!rproc) {
		dev_err(&pdev->dev, "failed to allocate rproc\n");
		return -ENOMEM;
	}

	wcss = rproc->priv;
	wcss->dev = &pdev->dev;
	wcss->desc = desc;
	wcss->firmware = firmware;

	ret = wcss_sec_alloc_memory_region(wcss);
	if (ret)
		return ret;

	sleep_clk = devm_clk_get_optional_enabled(&pdev->dev, "sleep");
	if (IS_ERR(sleep_clk))
		return dev_err_probe(&pdev->dev, PTR_ERR(sleep_clk),
				     "Failed to get sleep clock\n");

	int_clk = devm_clk_get_optional_enabled(&pdev->dev, "interconnect");
	if (IS_ERR(int_clk))
		return dev_err_probe(&pdev->dev, PTR_ERR(int_clk),
				     "Failed to get interconnect clock\n");

	ret = qcom_q6v5_init(&wcss->q6, pdev, rproc,
			     WCSS_CRASH_REASON, NULL, NULL);
	if (ret)
		return ret;

	qcom_add_glink_subdev(rproc, &wcss->glink_subdev, desc->ss_name);
	qcom_add_ssr_subdev(rproc, &wcss->ssr_subdev, desc->ss_name);

	rproc->auto_boot = false;
	rproc->dump_conf = RPROC_COREDUMP_INLINE;
	rproc_coredump_set_elf_info(rproc, ELFCLASS32, EM_NONE);

	if (desc->use_tmelcom) {
		wcss->mbox_client.dev = wcss->dev;
		wcss->mbox_client.knows_txdone = true;
		wcss->mbox_client.tx_block = true;
		wcss->mbox_chan = mbox_request_channel(&wcss->mbox_client, 0);
		if (IS_ERR_OR_NULL(wcss->mbox_chan))
			return dev_err_probe(wcss->dev, PTR_ERR(wcss->mbox_chan),
					     "mbox chan for IPC is missing\n");
	}

	ret = devm_rproc_add(&pdev->dev, rproc);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, rproc);

	return 0;
}

static void wcss_sec_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct wcss_sec *wcss = rproc->priv;

	mbox_free_channel(wcss->mbox_chan);
	qcom_remove_glink_subdev(rproc, &wcss->glink_subdev);
	qcom_remove_ssr_subdev(rproc, &wcss->ssr_subdev);
	qcom_q6v5_deinit(&wcss->q6);
}

static const struct wcss_data wcss_sec_ipq5332_res_init = {
	.pasid = MPD_WCSS_PAS_ID,
	.ss_name = "q6wcss",
};

static const struct wcss_data wcss_sec_ipq5424_res_init = {
	.pasid = MPD_WCSS_PAS_ID,
	.ss_name = "q6wcss",
	.use_tmelcom = true,
};

static const struct wcss_data wcss_sec_ipq9574_res_init = {
	.pasid = WCSS_PAS_ID,
	.ss_name = "q6wcss",
};

static const struct of_device_id wcss_sec_of_match[] = {
	{ .compatible = "qcom,ipq5018-wcss-sec-pil", .data = &wcss_sec_ipq5332_res_init },
	{ .compatible = "qcom,ipq5332-wcss-sec-pil", .data = &wcss_sec_ipq5332_res_init },
	{ .compatible = "qcom,ipq5424-wcss-sec-pil", .data = &wcss_sec_ipq5424_res_init },
	{ .compatible = "qcom,ipq9574-wcss-sec-pil", .data = &wcss_sec_ipq9574_res_init },
	{ },
};
MODULE_DEVICE_TABLE(of, wcss_sec_of_match);

static struct platform_driver wcss_sec_driver = {
	.probe = wcss_sec_probe,
	.remove = wcss_sec_remove,
	.driver = {
		.name = "qcom-wcss-secure-pil",
		.of_match_table = wcss_sec_of_match,
	},
};
module_platform_driver(wcss_sec_driver);

MODULE_DESCRIPTION("Hexagon WCSS Secure Peripheral Image Loader");
MODULE_LICENSE("GPL");
