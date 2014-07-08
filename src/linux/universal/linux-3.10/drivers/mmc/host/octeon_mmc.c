/*
 * Driver for MMC and SSD cards for Cavium OCTEON SOCs.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012 Cavium Inc.
 */

#include <linux/blkdev.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/delay.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>

#include <asm/byteorder.h>
#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-mio-defs.h>

#define DRV_NAME	"octeon_mmc"
#define DRV_VERSION	"1.0"

#define OCTEON_MAX_MMC		4

#define OCT_MIO_NDF_DMA_CFG		0x00
#define OCT_MIO_NDF_DMA_INT		0x08
#define OCT_MIO_NDF_DMA_INT_EN		0x10

#define OCT_MIO_EMM_CFG			0x00
#define OCT_MIO_EMM_SWITCH		0x48
#define OCT_MIO_EMM_DMA			0x50
#define OCT_MIO_EMM_CMD			0x58
#define OCT_MIO_EMM_RSP_STS		0x60
#define OCT_MIO_EMM_RSP_LO		0x68
#define OCT_MIO_EMM_RSP_HI		0x70
#define OCT_MIO_EMM_INT			0x78
#define OCT_MIO_EMM_INT_EN		0x80
#define OCT_MIO_EMM_WDOG		0x88
#define OCT_MIO_EMM_STS_MASK		0x98
#define OCT_MIO_EMM_RCA			0xa0
#define OCT_MIO_EMM_BUF_IDX		0xe0
#define OCT_MIO_EMM_BUF_DAT		0xe8

struct octeon_mmc_host {
	spinlock_t		lock;
	u64	base;
	u64	ndf_base;
	u64	last_emm_switch;

	struct mmc_request	*current_req;
	struct sg_mapping_iter smi;
	int sg_idx;
	bool dma_active;

	struct semaphore mmc_serializer;

	struct platform_device	*pdev;

	struct octeon_mmc_slot	*slot[OCTEON_MAX_MMC];
};

struct octeon_mmc_slot {
	struct mmc_host         *mmc;
	struct octeon_mmc_host	*host;

	unsigned int		clock;
	unsigned int		sclock;

	int			bus_width;
	int			bus_id;
};

static void octeon_mmc_reset_bus(struct octeon_mmc_slot *slot, int preserve);

static bool octeon_mmc_switch_val_changed(struct octeon_mmc_slot *slot,
					  u64 new_val)
{
	/* Match BUS_ID, HS_TIMING, BUS_WIDTH, POWER_CLASS, CLK_HI, CLK_LO */
	u64 m = 0x3001070fffffffffull;
	return (slot->host->last_emm_switch & m) != (new_val & m);
}

static unsigned int octeon_mmc_timeout_to_wdog(struct octeon_mmc_slot *slot,
					       unsigned int ns)
{
	u64 bt = (u64)slot->clock * (u64)ns;
	return (unsigned int)(bt / 1000000000);
}

static void octeon_mmc_dma_next(struct octeon_mmc_host	*host)
{
	struct scatterlist *sg;
	struct mmc_data *data;
	union cvmx_mio_ndf_dma_cfg dma_cfg;
	u64 dma_int_en;

	data = host->current_req->data;
	sg = data->sg + host->sg_idx;

	dma_cfg.u64 = 0;
	dma_cfg.s.en = 1;
	dma_cfg.s.rw = (data->flags & MMC_DATA_WRITE) ? 1 : 0;
#ifdef __LITTLE_ENDIAN
	dma_cfg.s.endian = 1;
#endif
	dma_cfg.s.size = (sg->length / 8) - 1;
	dma_cfg.s.adr = sg_phys(sg);

	host->sg_idx++;

	if (host->sg_idx >= host->current_req->data->sg_len)
		dma_int_en = 0;
	else
		dma_int_en = 1;

	cvmx_write_csr(host->ndf_base + OCT_MIO_NDF_DMA_INT_EN, dma_int_en);
	cvmx_write_csr(host->ndf_base + OCT_MIO_NDF_DMA_CFG, dma_cfg.u64);

}

static irqreturn_t octeon_mmc_dma_interrupt(int irq, void *dev_id)
{
	struct octeon_mmc_host *host = dev_id;
	unsigned long flags;

	pr_debug("Got interrupt: NDF_DMA_INT\n");
	spin_lock_irqsave(&host->lock, flags);
	/* Clear any pending irqs */
	cvmx_write_csr(host->ndf_base + OCT_MIO_NDF_DMA_INT, 1);

	if (!host->current_req || !host->current_req->data) {
		pr_err("ERROR: no current_req for octeon_mmc_dma_interrupt\n");
		goto out;
	}

	if (host->sg_idx < host->current_req->data->sg_len)
		octeon_mmc_dma_next(host);
out:
	spin_unlock_irqrestore(&host->lock, flags);

	return IRQ_RETVAL(1);
}

static irqreturn_t octeon_mmc_interrupt(int irq, void *dev_id)
{
	struct octeon_mmc_host *host = dev_id;
	union cvmx_mio_emm_int emm_int;
	struct mmc_request	*req;
	bool host_done;
	union cvmx_mio_emm_rsp_sts rsp_sts;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	emm_int.u64 = cvmx_read_csr(host->base + OCT_MIO_EMM_INT);
	req = host->current_req;
	cvmx_write_csr(host->base + OCT_MIO_EMM_INT, emm_int.u64);

	pr_debug("Got interrupt: EMM_INT = 0x%llx\n", emm_int.u64);

	if (!req)
		goto out;

	rsp_sts.u64 = cvmx_read_csr(host->base + OCT_MIO_EMM_RSP_STS);
	pr_debug("octeon_mmc_interrupt  MIO_EMM_RSP_STS 0x%llx\n", rsp_sts.u64);

	if (!host->dma_active && emm_int.s.buf_done && req->cmd->data &&
	    ((rsp_sts.u64 >> 7) & 3) == 1) {
		/* Read */
		int dbuf = rsp_sts.s.dbuf;
		struct sg_mapping_iter *smi = &host->smi;
		unsigned int bytes_xfered = 0;
		u64 dat = 0;
		int shift = -1;

		/* Auto inc from offset zero */
		cvmx_write_csr(host->base + OCT_MIO_EMM_BUF_IDX, (u64)(0x10000 | (dbuf << 6)));

		for (;;) {
			if (smi->consumed >= smi->length) {
				if (!sg_miter_next(smi))
					break;
				smi->consumed = 0;
			}
			if (shift < 0) {
				dat = cvmx_read_csr(host->base + OCT_MIO_EMM_BUF_DAT);
				shift = 56;
			}

			while (smi->consumed < smi->length && shift >= 0) {
				*(u8 *)(smi->addr) = (dat >> shift) & 0xff;
				bytes_xfered++;
				smi->addr++;
				smi->consumed++;
				shift -= 8;
			}
		}
		sg_miter_stop(smi);
		req->cmd->data->bytes_xfered = bytes_xfered;
		req->cmd->data->error = 0;
	}
	host_done = emm_int.s.cmd_done || emm_int.s.dma_done ||
		emm_int.s.cmd_err || emm_int.s.dma_err;
	if (host_done && req->done) {
		if (rsp_sts.u64 & (7ull << 13))
			req->cmd->error = -EILSEQ;
		else
			req->cmd->error = 0;

		if (host->dma_active && req->cmd->data) {
			req->cmd->data->error = 0;
			req->cmd->data->bytes_xfered = req->cmd->data->blocks * req->cmd->data->blksz;
		}
		if (rsp_sts.s.rsp_val) {
			u64 rsp_hi;
			u64 rsp_lo = cvmx_read_csr(host->base + OCT_MIO_EMM_RSP_LO);

			switch (rsp_sts.s.rsp_type) {
			case 1:
			case 3:
				req->cmd->resp[0] = (rsp_lo >> 8) & 0xffffffff;
				req->cmd->resp[1] = 0;
				req->cmd->resp[2] = 0;
				req->cmd->resp[3] = 0;
				break;
			case 2:
				req->cmd->resp[3] = rsp_lo & 0xffffffff;
				req->cmd->resp[2] = (rsp_lo >> 32) & 0xffffffff;
				rsp_hi = cvmx_read_csr(host->base + OCT_MIO_EMM_RSP_HI);
				req->cmd->resp[1] = rsp_hi & 0xffffffff;
				req->cmd->resp[0] = (rsp_hi >> 32) & 0xffffffff;
				break;
			default:
				pr_debug("octeon_mmc_interrupt unhandled rsp_val %d\n",
					 rsp_sts.s.rsp_type);
				break;
			}
			pr_debug("octeon_mmc_interrupt  resp %08x %08x %08x %08x\n",
				 req->cmd->resp[0], req->cmd->resp[1], req->cmd->resp[2], req->cmd->resp[3]);
		}
		if (emm_int.s.dma_err && rsp_sts.s.dma_pend) {
			/* Try to clean up failed DMA */
			union cvmx_mio_emm_dma emm_dma;
			emm_dma.u64 = 0;
			emm_dma.s.dma_val = 1;
			emm_dma.s.dat_null = 1;
			emm_dma.s.bus_id = rsp_sts.s.bus_id;
			cvmx_write_csr(host->base + OCT_MIO_EMM_DMA, emm_dma.u64);
		}

		host->current_req = NULL;
		req->done(req);
	}
	spin_unlock_irqrestore(&host->lock, flags);
	if (host_done)
		up(&host->mmc_serializer);
out:
	return IRQ_RETVAL(emm_int.u64 != 0);
}

static void octeon_mmc_dma_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct octeon_mmc_slot	*slot;
	struct octeon_mmc_host	*host;
	struct mmc_command *cmd;
	struct mmc_data *data;
	union cvmx_mio_emm_int emm_int;
	union cvmx_mio_emm_dma emm_dma;
	unsigned long flags;

	cmd = mrq->cmd;
	if (mrq->data == NULL || mrq->data->sg == NULL || !mrq->data->sg_len ||
	    mrq->stop == NULL || mrq->stop->opcode != MMC_STOP_TRANSMISSION) {
		pr_err("Error: octeon_mmc_dma_request no data\n");
		cmd->error = -EINVAL;
		if (mrq->done)
			mrq->done(mrq);
		return;
	}

	slot = mmc_priv(mmc);
	host = slot->host;

	/* Only a single user of the host block at a time. */
	down(&host->mmc_serializer);

	data = mrq->data;

	if (data->timeout_ns) {
		cvmx_write_csr(host->base + OCT_MIO_EMM_WDOG,
			       octeon_mmc_timeout_to_wdog(slot, data->timeout_ns));
			pr_debug("OCT_MIO_EMM_WDOG %llu\n", cvmx_read_csr(host->base + OCT_MIO_EMM_WDOG));	}

	spin_lock_irqsave(&host->lock, flags);
	WARN_ON(host->current_req);
	host->current_req = mrq;

	host->sg_idx = 0;

	/* Clear any pending irqs */
	cvmx_write_csr(host->ndf_base + OCT_MIO_NDF_DMA_INT, 1);

	octeon_mmc_dma_next(host);

	emm_dma.u64 = 0;
	emm_dma.s.bus_id = slot->bus_id;
	emm_dma.s.dma_val = 1;
	emm_dma.s.rw = (data->flags & MMC_DATA_WRITE) ? 1 : 0;
	emm_dma.s.multi = 1;
	emm_dma.s.block_cnt = data->blocks;
	emm_dma.s.card_addr = cmd->arg;


	emm_int.u64 = 0;
	emm_int.s.dma_done = 1;
	emm_int.s.cmd_err = 1;
	emm_int.s.dma_err = 1;
	/* Clear the bit. */
	cvmx_write_csr(host->base + OCT_MIO_EMM_INT, emm_int.u64);
	cvmx_write_csr(host->base + OCT_MIO_EMM_INT_EN, emm_int.u64);
	host->dma_active = true;

	spin_unlock_irqrestore(&host->lock, flags);

	cvmx_write_csr(host->base + OCT_MIO_EMM_DMA, emm_dma.u64);
	pr_debug("Send the dma command: %llx\n", emm_dma.u64);
}

static void octeon_mmc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct octeon_mmc_slot	*slot;
	struct octeon_mmc_host	*host;
	struct mmc_command *cmd;
	union cvmx_mio_emm_int emm_int;
	union cvmx_mio_emm_cmd emm_cmd;
	unsigned long flags;

	cmd = mrq->cmd;

	if (cmd->opcode == MMC_READ_MULTIPLE_BLOCK || cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) {
		octeon_mmc_dma_request(mmc, mrq);
		return;
	}

	slot = mmc_priv(mmc);
	host = slot->host;

	/* Only a single user of the host block at a time. */
	down(&host->mmc_serializer);

	spin_lock_irqsave(&host->lock, flags);
	WARN_ON(host->current_req);
	host->current_req = mrq;

	emm_int.u64 = 0;
	emm_int.s.cmd_done = 1;
	emm_int.s.cmd_err = 1;
	if (cmd->data) {
		pr_debug("command has data\n");
		cmd->error = -EINPROGRESS;
		if (cmd->data->flags & MMC_DATA_READ) {
			emm_int.s.buf_done = 1;
			sg_miter_start(&host->smi, mrq->cmd->data->sg,
				       mrq->cmd->data->sg_len, SG_MITER_ATOMIC | SG_MITER_TO_SG);
		} else {
			struct sg_mapping_iter *smi = &host->smi;
			unsigned int bytes_xfered = 0;
			u64 dat = 0;
			int shift = 56;
			/* Copy data to the xmit buffer before issuing the command */
			sg_miter_start(smi, mrq->cmd->data->sg,
				       mrq->cmd->data->sg_len, SG_MITER_TO_SG);
			/* Auto inc from offset zero, dbuf zero */
			cvmx_write_csr(host->base + OCT_MIO_EMM_BUF_IDX, 0x10000ull);

			for (;;) {
				if (smi->consumed >= smi->length) {
					if (!sg_miter_next(smi))
						break;
					smi->consumed = 0;
				}

				while (smi->consumed < smi->length && shift >= 0) {
					dat |= (u64)(*(u8 *)(smi->addr)) << shift;
					bytes_xfered++;
					smi->addr++;
					smi->consumed++;
					shift -= 8;
				}
				if (shift < 0) {
					cvmx_write_csr(host->base + OCT_MIO_EMM_BUF_DAT, dat);
					shift = 56;
					dat = 0;
				}
			}
			sg_miter_stop(smi);
			cmd->data->bytes_xfered = bytes_xfered;
			cmd->data->error = 0;
		}
		if (cmd->data->timeout_ns) {
			cvmx_write_csr(host->base + OCT_MIO_EMM_WDOG,
				       octeon_mmc_timeout_to_wdog(slot, cmd->data->timeout_ns));
			pr_debug("OCT_MIO_EMM_WDOG %llu\n", cvmx_read_csr(host->base + OCT_MIO_EMM_WDOG));		}
	} else {
		cvmx_write_csr(host->base + OCT_MIO_EMM_WDOG,
			       ((u64)slot->clock * 850ull) / 1000ull);
			pr_debug("OCT_MIO_EMM_WDOG %llu\n", cvmx_read_csr(host->base + OCT_MIO_EMM_WDOG));
	}
	/* Clear the bit. */
	cvmx_write_csr(host->base + OCT_MIO_EMM_INT, emm_int.u64);
	cvmx_write_csr(host->base + OCT_MIO_EMM_INT_EN, emm_int.u64);
	host->dma_active = false;
	spin_unlock_irqrestore(&host->lock, flags);

	emm_cmd.u64 = 0;
	emm_cmd.s.cmd_val = 1;
	emm_cmd.s.bus_id = slot->bus_id;
	emm_cmd.s.cmd_idx = cmd->opcode;
	emm_cmd.s.arg = cmd->arg;
	cvmx_write_csr(host->base + OCT_MIO_EMM_CMD, emm_cmd.u64);
	pr_debug("Send the command: %llx\n", emm_cmd.u64);

}

static void octeon_mmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct octeon_mmc_slot	*slot;
	struct octeon_mmc_host	*host;
	int bus_width;
	int clock;
	int hs_timing;
	int power_class = 10;
	int clk_period;
	int timeout = 2000;
	union cvmx_mio_emm_switch emm_switch;
	union cvmx_mio_emm_rsp_sts emm_sts;

	slot = mmc_priv(mmc);
	host = slot->host;

	/* Only a single user of the host block at a time. */
	down(&host->mmc_serializer);

	pr_debug("Calling set_ios: slot: clk = 0x%x, bus_width = %d\n", slot->clock, slot->bus_width);
	pr_debug("Calling set_ios: ios: clk = 0x%x, vdd = %u, bus_width = %u, power_mode = %u, timing = %u\n",
		 ios->clock, ios->vdd, ios->bus_width, ios->power_mode, ios->timing);
	pr_debug("Calling set_ios: mmc: caps = 0x%lx, bus_width = %d\n", mmc->caps, mmc->ios.bus_width);

	/*
	 * Reset the chip on each power off
	 */
	if (ios->power_mode == MMC_POWER_OFF)
		octeon_mmc_reset_bus(slot, 1);

	switch (ios->bus_width) {
	case MMC_BUS_WIDTH_8:
		bus_width = 2;
		break;
	case MMC_BUS_WIDTH_4:
		bus_width = 1;
		break;
	case MMC_BUS_WIDTH_1:
		bus_width = 0;
		break;
	default:
		pr_debug("unknown bus width %d\n", slot->bus_width);
		bus_width = 0;
		break;
	}
	hs_timing = (ios->timing == MMC_TIMING_MMC_HS);
	if (ios->clock) {
		slot->clock = ios->clock;

		clock = slot->clock;

		if (clock > 52000000)
			clock = 50000000;

		clk_period = (octeon_get_io_clock_rate() + clock - 1) / (2 * clock);

		emm_switch.u64 = 0;
		emm_switch.s.bus_id = slot->bus_id;
		emm_switch.s.hs_timing = hs_timing;
		emm_switch.s.bus_width = bus_width;
		emm_switch.s.power_class = power_class;
		emm_switch.s.clk_hi = clk_period;
		emm_switch.s.clk_lo = clk_period;

		if (!octeon_mmc_switch_val_changed(slot, emm_switch.u64)) {
			pr_debug("No change from 0x%llx mio_emm_switch, returning.\n", emm_switch.u64);
			goto out;
		}

		pr_debug("Writing 0x%llx to mio_emm_wdog\n", ((u64)clock * 850ull) / 1000ull);
		cvmx_write_csr(host->base + OCT_MIO_EMM_WDOG, ((u64)clock * 850ull) / 1000ull);
		pr_debug("Writing 0x%llx to mio_emm_switch\n", emm_switch.u64);
		cvmx_write_csr(host->base + OCT_MIO_EMM_SWITCH, emm_switch.u64);
		slot->host->last_emm_switch = emm_switch.u64;

		do {
			emm_sts.u64 = cvmx_read_csr(host->base + OCT_MIO_EMM_RSP_STS);
			if (!emm_sts.s.switch_val)
				break;
			udelay(100);
		} while (timeout-- > 0);

		if (timeout <= 0) {
			pr_debug("%s: switch command timed out, status=0x%llx\n",
				 __func__, emm_sts.u64);
			goto out;
		}
	}
out:
	up(&host->mmc_serializer);
}

static const struct mmc_host_ops octeon_mmc_ops = {
	.request        = octeon_mmc_request,
	.set_ios        = octeon_mmc_set_ios,
};

static void octeon_mmc_reset_bus(struct octeon_mmc_slot *slot, int preserve)
{
	union cvmx_mio_emm_cfg emm_cfg;
	union cvmx_mio_emm_switch emm_switch;
	union cvmx_mio_emm_sts_mask emm_sts;
	u64 wdog = 0;

	emm_cfg.u64 = cvmx_read_csr(slot->host->base + OCT_MIO_EMM_CFG);
	if (preserve) {
		emm_switch.u64 = cvmx_read_csr(slot->host->base + OCT_MIO_EMM_SWITCH);
		wdog = cvmx_read_csr(slot->host->base + OCT_MIO_EMM_WDOG);
	}

	/* Reset the bus */
	emm_cfg.u64 &= ~(1 << slot->bus_id);
	cvmx_write_csr(slot->host->base + OCT_MIO_EMM_CFG, emm_cfg.u64);
	msleep(10);  /* Wait 10ms */
	emm_cfg.u64 |= 1 << slot->bus_id;
	cvmx_write_csr(slot->host->base + OCT_MIO_EMM_CFG, emm_cfg.u64);

	msleep(10);

	emm_sts.u64 = 0;
	emm_sts.s.sts_msk = 0x80;
	cvmx_write_csr(slot->host->base + OCT_MIO_EMM_STS_MASK, emm_sts.u64);

	/* Restore switch settings */
	if (preserve) {
		emm_switch.s.switch_exe = 0;
		emm_switch.s.switch_err0 = 0;
		emm_switch.s.switch_err1 = 0;
		emm_switch.s.switch_err2 = 0;
		slot->host->last_emm_switch = emm_switch.u64;
		cvmx_write_csr(slot->host->base + OCT_MIO_EMM_SWITCH, emm_switch.u64);
		msleep(10);
		cvmx_write_csr(slot->host->base + OCT_MIO_EMM_WDOG, wdog);
	} else {
		slot->host->last_emm_switch = 0;
	}
}

static void octeon_mmc_set_clock(struct octeon_mmc_slot *slot, unsigned int clock)
{
	struct mmc_host *mmc = slot->mmc;
	clock = min(clock, mmc->f_max);
	clock = max(clock, mmc->f_min);
	slot->clock = clock;
}

static int octeon_mmc_initlowlevel(struct octeon_mmc_slot *slot, int id,
			int bus_width)
{
	union cvmx_mio_emm_switch emm_switch;
	struct octeon_mmc_host *host = slot->host;

	octeon_mmc_reset_bus(slot, 0);

	octeon_mmc_set_clock(slot, 400000);

	/* Program initial clock speed and power */
	emm_switch.u64 = 0;
	emm_switch.s.bus_id = id;
	emm_switch.s.power_class = 10;
	emm_switch.s.clk_hi = (slot->sclock / slot->clock) / 2;
	emm_switch.s.clk_lo = (slot->sclock / slot->clock) / 2;
	slot->host->last_emm_switch = emm_switch.u64;
	cvmx_write_csr(host->base + OCT_MIO_EMM_SWITCH, emm_switch.u64);
	cvmx_write_csr(host->base + OCT_MIO_EMM_WDOG, ((u64)slot->clock * 850ull) / 1000ull);
	cvmx_write_csr(host->base + OCT_MIO_EMM_STS_MASK, 0xe4f90080ull);

	return 0;
}

static int octeon_init_slot(struct octeon_mmc_host *host, int id,
				   int bus_width, int max_freq)
{
	struct mmc_host *mmc;
	struct octeon_mmc_slot *slot;
	int ret;

	/*
	 * Allocate MMC structue
	 */
	mmc = mmc_alloc_host(sizeof(struct octeon_mmc_slot), &host->pdev->dev);
	if (!mmc) {
		pr_debug("alloc host failed\n");
		return -ENOMEM;
	}

	slot = mmc_priv(mmc);
	slot->mmc = mmc;
	slot->host = host;

	/*
	 * Set up host parameters.
	 */
	mmc->ops = &octeon_mmc_ops;
	mmc->f_min = 400000;
	mmc->f_max = max_freq;
	mmc->caps = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;
	mmc->ocr_avail = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 |
			 MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
			 MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36;

	/*
	 * Maximum number of segments.
	 */
	mmc->max_segs = 64;
	mmc->max_seg_size = (1 << 23) - 8;
	mmc->max_req_size = mmc->max_seg_size;
	mmc->max_blk_size = 512;
	mmc->max_blk_count = mmc->max_req_size / 512;



//	mmc->max_segs = 64;
//	mmc->max_phys_segs = 64;
//	mmc->max_req_size = 32768 * 512;
//	mmc->max_blk_size = 4095;
//	mmc->max_blk_count = mmc->max_req_size;


	slot->clock = mmc->f_min;
	slot->sclock = octeon_get_io_clock_rate();

	slot->bus_width = bus_width;
	slot->bus_id = id;

	/* Initialize MMC Block. */
	octeon_mmc_initlowlevel(slot, id, bus_width);

	host->slot[id] = slot;
	ret = mmc_add_host(mmc);
	pr_debug("mmc_add_host returned %d\n", ret);

	return 0;
}

static int octeon_mmc_probe(struct platform_device *pdev)
{
	union cvmx_mio_emm_cfg emm_cfg;
	struct octeon_mmc_host *host;
	struct resource	*res;
	int mmc_irq, dma_irq;
	int ret = 0;
	struct device_node *node;
	int found = 0;

	mmc_irq = platform_get_irq(pdev, 0);
	if (mmc_irq < 0)
		return mmc_irq;

	dma_irq = platform_get_irq(pdev, 1);
	if (dma_irq < 0)
		return dma_irq;

	host = devm_kzalloc(&pdev->dev, sizeof(*host), GFP_KERNEL);
	if (!host) {
		dev_err(&pdev->dev, "devm_kzalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	host->pdev = pdev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Platform resource is missing\n");
		ret = -ENXIO;
		goto err;
	}
	if (!devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
				     res->name)) {
		dev_err(&pdev->dev, "request_mem_region 0 failed\n");
		goto err;
	}

	host->base = (u64)devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!host->base) {
		dev_err(&pdev->dev, "Platform resource0 is missing\n");
		ret = -EINVAL;
		goto err;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_err(&pdev->dev, "Platform resource1 is missing\n");
		ret = -EINVAL;
		goto err;
	}
	if (!devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
				     res->name)) {
		dev_err(&pdev->dev, "request_mem_region 1 failed\n");
		goto err;
	}
	host->ndf_base = (u64)devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!host->ndf_base) {
		dev_err(&pdev->dev, "Platform resource0 is missing\n");
		ret = -EINVAL;
		goto err;
	}

	ret = devm_request_irq(&pdev->dev, mmc_irq, octeon_mmc_interrupt, IRQF_DISABLED, DRV_NAME,
			       host);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error: devm_request_irq %d\n", mmc_irq);
		goto err;
	}

	ret = devm_request_irq(&pdev->dev, dma_irq, octeon_mmc_dma_interrupt,
			       IRQF_DISABLED | IRQF_SHARED, DRV_NAME,
			       host);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error: devm_request_irq %d\n", dma_irq);
		goto err;
	}


	spin_lock_init(&host->lock);
	sema_init(&host->mmc_serializer, 1);

	platform_set_drvdata(pdev, host);

	node = of_get_next_child(pdev->dev.of_node, NULL);
	while (node) {
		int size;
		const __be32 *data;
		found = 0;

		data = of_get_property(node, "reg", &size);
		if (data) {
			int bus_width, max_freq;
			int slot = be32_to_cpu(*data);
			data = of_get_property(node,
					       "cavium,bus-max-width",
					       &size);
			if (!data || size != 4) {
				pr_info("Bus width not found for slot %d\n", slot);
				bus_width = 8;
			} else {
				bus_width = be32_to_cpup(data);
				switch (bus_width) {
				case 1:
				case 4:
				case 8:
					break;
				default:
					bus_width = 8;
					break;
				}
			}
			data = of_get_property(node,
					       "spi-max-frequency",
					       &size);
			if (!data || size != 4) {
				max_freq = 52000000;
				pr_info("no spi-max-frequency for slot %d, defautling to %d\n",
					slot, max_freq);
			} else {
				max_freq = be32_to_cpup(data);
			}

			ret = octeon_init_slot(host, slot, bus_width, max_freq);
			pr_debug("init slot %d, ret = %d\n", slot, ret);
		}
		node = of_get_next_child(pdev->dev.of_node, node);
	}

	return ret;

err:
	/* Disable MMC controller */
	emm_cfg.s.bus_ena = 0;
	cvmx_write_csr(host->base + OCT_MIO_EMM_CFG, emm_cfg.u64);
	return ret;
}

static int octeon_mmc_remove(struct platform_device *pdev)
{
	union cvmx_mio_ndf_dma_int ndf_dma_int;
	union cvmx_mio_ndf_dma_cfg ndf_dma_cfg;
	struct octeon_mmc_host *host = platform_get_drvdata(pdev);

	pr_debug("Calling mmc_remove, host = %p\n", host);
	if (host) {
		/* Reset bus_id */
		ndf_dma_cfg.u64 = cvmx_read_csr(host->ndf_base + OCT_MIO_NDF_DMA_CFG);
		ndf_dma_cfg.s.en = 0;
		cvmx_write_csr(host->ndf_base + OCT_MIO_NDF_DMA_CFG, ndf_dma_cfg.u64);

		/* Disable the interrupt */
		ndf_dma_int.u64 = 0;
		cvmx_write_csr(host->ndf_base + OCT_MIO_NDF_DMA_INT, ndf_dma_int.u64);

		pr_debug("MMC Removed\n");
	}

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct of_device_id octeon_mmc_match[] = {
	{
		.compatible = "cavium,octeon-6130-mmc",
	},
	{},
};
MODULE_DEVICE_TABLE(of, octeon_mmc_match);

static struct platform_driver octeon_mmc_driver = {
	.probe		= octeon_mmc_probe,
	.remove 	= octeon_mmc_remove,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = octeon_mmc_match,
	},
};

static int __init octeon_mmc_init(void)
{
	int ret;

	pr_debug("calling octeon_mmc_init\n");

	ret = platform_driver_register(&octeon_mmc_driver);
	pr_debug("driver probe returned %d\n", ret);

	if (ret)
		pr_err("%s: Failed to register driver\n", DRV_NAME);

	return ret;
}

static void octeon_mmc_cleanup(void)
{
	/* Unregister MMC driver */
	platform_driver_unregister(&octeon_mmc_driver);
}

module_init(octeon_mmc_init);
module_exit(octeon_mmc_cleanup);

MODULE_AUTHOR("Cavium Inc. <support@caviumn.com>");
MODULE_DESCRIPTION("low-level driver for Cavium OCTEON MMC/SSD card");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

