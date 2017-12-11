/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#ifndef _QCOM_BAM_DMA_H
#define _QCOM_BAM_DMA_H

#include <linux/dma-mapping.h>

#define DESC_FLAG_INT BIT(15)
#define DESC_FLAG_EOT BIT(14)
#define DESC_FLAG_EOB BIT(13)
#define DESC_FLAG_NWD BIT(12)
#define DESC_FLAG_CMD BIT(11)

/*
 * QCOM BAM DMA SGL struct
 *
 * @sgl: DMA SGL
 * @dma_flags: BAM DMA flags
 */
struct qcom_bam_sgl {
	struct scatterlist sgl;
	unsigned int dma_flags;
};

/*
 * This data type corresponds to the native Command Element
 * supported by BAM DMA Engine.
 *
 * @addr - register address.
 * @command - command type.
 * @data - for write command: content to be written into peripheral register.
 *	 for read command: dest addr to write peripheral register value to.
 * @mask - register mask.
 * @reserved - for future usage.
 *
 */
struct bam_cmd_element {
	__le32 addr:24;
	__le32 command:8;
	__le32 data;
	__le32 mask;
	__le32 reserved;
};

/*
 * This enum indicates the command type in a command element
 */
enum bam_command_type {
	BAM_WRITE_COMMAND = 0,
	BAM_READ_COMMAND,
};

/*
 * QCOM BAM DMA custom data
 *
 * @sgl_cnt: number of sgl in bam_sgl
 * @dir: DMA data transfer direction
 * @bam_sgl: BAM SGL pointer
 */
struct qcom_bam_custom_data {
	u32 sgl_cnt;
	enum dma_transfer_direction dir;
	struct qcom_bam_sgl *bam_sgl;
};

/*
 * qcom_bam_sg_init_table - Init QCOM BAM SGL
 * @bam_sgl: bam sgl
 * @nents: number of entries in bam sgl
 *
 * This function performs the initialization for each SGL in BAM SGL
 * with generic SGL API.
 */
static inline void qcom_bam_sg_init_table(struct qcom_bam_sgl *bam_sgl,
		unsigned int nents)
{
	int i;

	for (i = 0; i < nents; i++)
		sg_init_table(&bam_sgl[i].sgl, 1);
}

/*
 * qcom_bam_unmap_sg - Unmap QCOM BAM SGL
 * @dev: device for which unmapping needs to be done
 * @bam_sgl: bam sgl
 * @nents: number of entries in bam sgl
 * @dir: dma transfer direction
 *
 * This function performs the DMA unmapping for each SGL in BAM SGL
 * with generic SGL API.
 */
static inline void qcom_bam_unmap_sg(struct device *dev,
	struct qcom_bam_sgl *bam_sgl, int nents, enum dma_data_direction dir)
{
	int i;

	for (i = 0; i < nents; i++)
		dma_unmap_sg(dev, &bam_sgl[i].sgl, 1, dir);
}

/*
 * qcom_bam_map_sg - Map QCOM BAM SGL
 * @dev: device for which mapping needs to be done
 * @bam_sgl: bam sgl
 * @nents: number of entries in bam sgl
 * @dir: dma transfer direction
 *
 * This function performs the DMA mapping for each SGL in BAM SGL
 * with generic SGL API.
 *
 * returns 0 on error and > 0 on success
 */
static inline int qcom_bam_map_sg(struct device *dev,
	struct qcom_bam_sgl *bam_sgl, int nents, enum dma_data_direction dir)
{
	int i, ret = 0;

	for (i = 0; i < nents; i++) {
		ret = dma_map_sg(dev, &bam_sgl[i].sgl, 1, dir);
		if (!ret)
			break;
	}

	/* unmap the mapped sgl from previous loop in case of error */
	if (!ret)
		qcom_bam_unmap_sg(dev, bam_sgl, i, dir);

	return ret;
}

/*
 * qcom_prep_bam_ce - Wrapper function to prepare a single BAM command element
 *	with the data that is passed to this function.
 * @bam_ce: bam command element
 * @addr: target address
 * @command: command in bam_command_type
 * @data: actual data for write and dest addr for read
 */
static inline void qcom_prep_bam_ce(struct bam_cmd_element *bam_ce,
				uint32_t addr, uint32_t command, uint32_t data)
{
	bam_ce->addr = cpu_to_le32(addr);
	bam_ce->command = cpu_to_le32(command);
	bam_ce->data = cpu_to_le32(data);
	bam_ce->mask = 0xFFFFFFFF;
}
#endif
