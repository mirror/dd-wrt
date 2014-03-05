#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/libata.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/slab.h>

#define DEBUG_UPM	0

#define DRV_NAME	"pata_rb_ppc"

#define DEV2SEL_OFFSET	0x00100000

#define IMMR_LBCFG_OFFSET	0x00005000
#define IMMR_LBCFG_SIZE		0x00001000

#define LOCAL_BUS_MCMR		0x00000078
#define   MxMR_OP_MASK			0x30000000
#define   MxMR_OP_NORMAL		0x00000000
#define   MxMR_OP_WRITE			0x10000000
#define   MxMR_OP_READ			0x20000000
#define   MxMR_OP_RUN			0x30000000
#define   MxMR_LUPWAIT_LOW		0x08000000
#define   MxMR_LUPWAIT_HIGH		0x00000000
#define   MxMR_LUPWAIT_ENABLE		0x00040000
#define   MxMR_RLF_MASK			0x0003c000
#define   MxMR_RLF_SHIFT			14
#define   MxMR_WLF_MASK			0x00003c00
#define   MxMR_WLF_SHIFT			10
#define   MxMR_MAD_MASK			0x0000003f
#define LOCAL_BUS_MDR		0x00000088
#define LOCAL_BUS_LCRR		0x000000D4
#define   LCRR_CLKDIV_MASK		0x0000000f

#define LOOP_SIZE	4

#define UPM_READ_SINGLE_OFFSET	0x00
#define UPM_WRITE_SINGLE_OFFSET	0x18
#define UPM_DATA_SIZE	0x40

#define LBT_CPUIN_MIN		0
#define LBT_CPUOUT_MIN		1
#define LBT_CPUOUT_MAX		2
#define LBT_EXTDEL_MIN		3
#define LBT_EXTDEL_MAX		4
#define LBT_SIZE		5

/* UPM machine configuration bits */
#define N_BASE	0x00f00000
#define N_CS	0xf0000000
#define N_CS_H1	0xc0000000
#define N_CS_H2	0x30000000
#define N_WE	0x0f000000
#define N_WE_H1	0x0c000000
#define N_WE_H2	0x03000000
#define N_OE	0x00030000
#define N_OE_H1	0x00020000
#define N_OE_H2	0x00010000
#define WAEN	0x00001000
#define REDO_2	0x00000100
#define REDO_3	0x00000200
#define REDO_4	0x00000300
#define LOOP	0x00000080
#define NA	0x00000008
#define UTA	0x00000004
#define LAST	0x00000001

#define REDO_VAL(mult)	(REDO_2 * ((mult) - 1))
#define REDO_MAX_MULT	4

#define READ_BASE	(N_BASE | N_WE)
#define WRITE_BASE	(N_BASE | N_OE)
#define EMPTY		(N_BASE | N_CS | N_OE | N_WE | LAST)

#define EOF_UPM_SETTINGS	0
#define ANOTHER_TIMING		1

#define OA_CPUIN_MIN		0x01
#define OA_CPUOUT_MAX		0x02
#define OD_CPUOUT_MIN		0x04
#define OA_CPUOUT_DELTA		0x06
#define OA_EXTDEL_MAX		0x08
#define OD_EXTDEL_MIN		0x10
#define OA_EXTDEL_DELTA		0x18
#define O_MIN_CYCLE_TIME	0x20
#define O_MINUS_PREV		0x40
#define O_HALF_CYCLE		0x80

extern void __iomem *localbus_map(unsigned long addr, unsigned int len);
extern void localbus_unmap(void __iomem *addr);

struct rbppc_info {
	unsigned lbcfg_addr;
	unsigned clk_time_ps;
	int cur_mode;
	u32 lb_timings[LBT_SIZE];
};
static struct rbppc_info *rbinfo = NULL;

struct upm_setting {
	unsigned value;
	unsigned ns[7];
	unsigned clk_minus;
	unsigned group_size;
	unsigned options;
};

static const struct upm_setting cfUpmReadSingle[] = {
	{ READ_BASE | N_OE,
	  /* t1 - ADDR setup time */
		{  70,  50,  30,  30,  25,  15,  10 }, 0, 0, (OA_CPUOUT_DELTA |
							      OA_EXTDEL_MAX) },
	{ READ_BASE | N_OE_H1,
		{   0,   0,   0,   0,   0,   0,   0 }, 0, 0, O_HALF_CYCLE },
	{ READ_BASE,
	  /* t2 - OE0 time */
		{ 290, 290, 290,  80,  70,  65,  55 }, 0, 2, (OA_CPUOUT_MAX |
							      OA_CPUIN_MIN) },
	{ READ_BASE | WAEN,
		{   1,   1,   1,   1,   1,   0,   0 }, 0, 0, 0 },
	{ READ_BASE | UTA,
		{   1,   1,   1,   1,   1,   1,   1 }, 0, 0, 0 },
	{ READ_BASE | N_OE,
	  /* t9 - ADDR hold time */
		{  20,  15,  10,  10,  10,  10,  10 }, 0, 0, (OA_CPUOUT_DELTA |
							      OD_EXTDEL_MIN) },
	{ READ_BASE | N_OE | N_CS_H2,
		{   0,   0,   0,   0,   0,   0,   0 }, 0, 0, O_HALF_CYCLE },
	{ READ_BASE | N_OE | N_CS,
	  /* t6Z -IORD data tristate */
		{  30,  30,  30,  30,  30,  20,  20 }, 1, 1, O_MINUS_PREV },
	{ ANOTHER_TIMING,
	  /* t2i -IORD recovery time */
		{   0,   0,   0,  70,  25,  25,  20 }, 2, 0, 0 },
	{ ANOTHER_TIMING,
	  /* CS 0 -> 1 MAX */
		{   0,   0,   0,   0,   0,   0,   0 }, 1, 0, (OA_CPUOUT_DELTA |
							      OA_EXTDEL_MAX) },
	{ READ_BASE | N_OE | N_CS | LAST,
		{   1,   1,   1,   1,   1,   1,   1 }, 0, 0, 0 },
	{ EOF_UPM_SETTINGS,
	  /* min total cycle time - includes turnaround and ALE cycle */
		{ 600, 383, 240, 180, 120, 100,  80 }, 2, 0, O_MIN_CYCLE_TIME },
};

static const struct upm_setting cfUpmWriteSingle[] = {
	{ WRITE_BASE | N_WE,
	  /* t1 - ADDR setup time */
		{  70,  50,  30,  30,  25,  15,  10 }, 0, 0, (OA_CPUOUT_DELTA |
							      OA_EXTDEL_MAX) },
	{ WRITE_BASE | N_WE_H1,
		{   0,   0,   0,   0,   0,   0,   0 }, 0, 0, O_HALF_CYCLE },
	{ WRITE_BASE,
	  /* t2 - WE0 time */
		{ 290, 290, 290,  80,  70,  65,  55 }, 0, 1, OA_CPUOUT_DELTA },
	{ WRITE_BASE | WAEN,
		{   1,   1,   1,   1,   1,   0,   0 }, 0, 0, 0 },
	{ WRITE_BASE | N_WE,
	  /* t9 - ADDR hold time */
		{  20,  15,  10,  10,  10,  10,  10 }, 0, 0, (OA_CPUOUT_DELTA |
							      OD_EXTDEL_MIN) },
	{ WRITE_BASE | N_WE | N_CS_H2,
		{   0,   0,   0,   0,   0,   0,   0 }, 0, 0, O_HALF_CYCLE },
	{ WRITE_BASE | N_WE | N_CS,
	  /* t4 - DATA hold time */
		{  30,  20,  15,  10,  10,  10,  10 }, 0, 1, O_MINUS_PREV },
	{ ANOTHER_TIMING,
	  /* t2i -IOWR recovery time */
		{   0,   0,   0,  70,  25,  25,  20 }, 1, 0, 0 },
	{ ANOTHER_TIMING,
	  /* CS 0 -> 1 MAX */
		{   0,   0,   0,   0,   0,   0,   0 }, 0, 0, (OA_CPUOUT_DELTA |
							      OA_EXTDEL_MAX) },
	{ WRITE_BASE | N_WE | N_CS | UTA | LAST,
		{   1,   1,   1,   1,   1,   1,   1 }, 0, 0, 0 },
	/* min total cycle time - includes ALE cycle */
	{ EOF_UPM_SETTINGS,
		{ 600, 383, 240, 180, 120, 100,  80 }, 1, 0, O_MIN_CYCLE_TIME },
};


static u8 rbppc_check_status(struct ata_port *ap)
{
	u8 val = ioread8(ap->ioaddr.status_addr);
	if (val == 0xF9) val = 0x7F;
	return val;
}

static u8 rbppc_check_altstatus(struct ata_port *ap)
{
	u8 val = ioread8(ap->ioaddr.altstatus_addr);
	if (val == 0xF9) val = 0x7F;
	return val;
}

static void rbppc_dummy_noret(struct ata_port *ap) { }
static int rbppc_dummy_ret0(struct ata_port *ap) { return 0; }

static int ps2clk(int ps, unsigned clk_time_ps) {
	int psMaxOver;
	if (ps <= 0) return 0;

	/* round down if <= 2% over clk border, but no more than 1/4 clk cycle */
	psMaxOver = ps * 2 / 100;
	if (4 * psMaxOver > clk_time_ps) {
		psMaxOver = clk_time_ps / 4;
	}
	return (ps + clk_time_ps - 1 - psMaxOver) / clk_time_ps;
}

static int upm_gen_ps_table(const struct upm_setting *upm,
			    int mode, struct rbppc_info *info,
			    int *psFinal) {
	int uidx;
	int lastUpmValIdx = 0;
	int group_start_idx = -1;
	int group_left_num = -1;
	int clk_time_ps = info->clk_time_ps;

	for (uidx = 0; upm[uidx].value != EOF_UPM_SETTINGS; ++uidx) {
		const struct upm_setting *us = upm + uidx;
		unsigned opt = us->options;
		int ps = us->ns[mode] * 1000 - us->clk_minus * clk_time_ps;

		if (opt & OA_CPUIN_MIN) ps += info->lb_timings[LBT_CPUIN_MIN];
		if (opt & OD_CPUOUT_MIN) ps -= info->lb_timings[LBT_CPUOUT_MIN];
		if (opt & OA_CPUOUT_MAX) ps += info->lb_timings[LBT_CPUOUT_MAX];
		if (opt & OD_EXTDEL_MIN) ps -= info->lb_timings[LBT_EXTDEL_MIN];
		if (opt & OA_EXTDEL_MAX) ps += info->lb_timings[LBT_EXTDEL_MAX];

		if (us->value == ANOTHER_TIMING) {
			/* use longest timing from alternatives */
			if (psFinal[lastUpmValIdx] < ps) {
				psFinal[lastUpmValIdx] = ps;
			}
			ps = 0;
		}
		else {
			if (us->group_size) {
				group_start_idx = uidx;
				group_left_num = us->group_size;
			}
			else if (group_left_num > 0) {
				/* group time is divided on all group members */
				int clk = ps2clk(ps, clk_time_ps);
				psFinal[group_start_idx] -= clk * clk_time_ps;
				--group_left_num;
			}
			if ((opt & O_MINUS_PREV) && lastUpmValIdx > 0) {
				int clk = ps2clk(psFinal[lastUpmValIdx],
						 clk_time_ps);
				ps -= clk * clk_time_ps;
			}
			lastUpmValIdx = uidx;
		}
		psFinal[uidx] = ps;
	}
	return uidx;
}

static int free_half(int ps, int clk, int clk_time_ps) {
    if (clk < 2) return 0;
    return (clk * clk_time_ps - ps) * 2 >= clk_time_ps;
}

static void upm_gen_clk_table(const struct upm_setting *upm,
			      int mode, int clk_time_ps,
			      int max_uidx, const int *psFinal, int *clkFinal) {
	int clk_cycle_time;
	int clk_total;
	int uidx;

	/* convert picoseconds to clocks */
	clk_total = 0;
	for (uidx = 0; uidx < max_uidx; ++uidx) {
		int clk = ps2clk(psFinal[uidx], clk_time_ps);
		clkFinal[uidx] = clk;
		clk_total += clk;
	}

	/* check possibility of half cycle usage */
	for (uidx = 1; uidx < max_uidx - 1; ++uidx) {
		if ((upm[uidx].options & O_HALF_CYCLE) &&
		    free_half(psFinal[uidx - 1], clkFinal[uidx - 1],
			      clk_time_ps) &&
		    free_half(psFinal[uidx + 1], clkFinal[uidx + 1],
			      clk_time_ps)) {
			++clkFinal[uidx];
			--clkFinal[uidx - 1];
			--clkFinal[uidx + 1];
		}
	}

	if ((upm[max_uidx].options & O_MIN_CYCLE_TIME) == 0) return;

	/* check cycle time, adjust timings if needed */
	clk_cycle_time = (ps2clk(upm[max_uidx].ns[mode] * 1000, clk_time_ps) -
			  upm[max_uidx].clk_minus);
	uidx = 0;
	while (clk_total < clk_cycle_time) {
		/* extend all timings in round-robin to match cycle time */
		if (clkFinal[uidx]) {
#if DEBUG_UPM
			printk(KERN_INFO "extending %u by 1 clk\n", uidx);
#endif
			++clkFinal[uidx];
			++clk_total;
		}
		++uidx;
		if (uidx == max_uidx) uidx = 0;
	}
}

static void add_data_val(unsigned val, int *clkLeft, int maxClk,
			unsigned *data, int *dataIdx) {
	if (*clkLeft == 0) return;

	if (maxClk == 0 && *clkLeft >= LOOP_SIZE * 2) {
		int times;
		int times1;
		int times2;

		times = *clkLeft / LOOP_SIZE;
		if (times > REDO_MAX_MULT * 2) times = REDO_MAX_MULT * 2;
		times1 = times / 2;
		times2 = times - times1;

		val |= LOOP;
		data[*dataIdx] = val | REDO_VAL(times1);
		++(*dataIdx);
		data[*dataIdx] = val | REDO_VAL(times2);
		++(*dataIdx);

		*clkLeft -= times * LOOP_SIZE;
		return;
	}

	if (maxClk < 1 || maxClk > REDO_MAX_MULT) maxClk = REDO_MAX_MULT;
	if (*clkLeft < maxClk) maxClk = *clkLeft;

	*clkLeft -= maxClk;
	val |= REDO_VAL(maxClk);

	data[*dataIdx] = val;
	++(*dataIdx);
}

static int upm_gen_final_data(const struct upm_setting *upm,
			       int max_uidx, int *clkFinal, unsigned *data) {
	int dataIdx;
	int uidx;

	dataIdx = 0;
	for (uidx = 0; uidx < max_uidx; ++uidx) {
		int clk = clkFinal[uidx];
		while (clk > 0) {
			add_data_val(upm[uidx].value, &clk, 0,
				     data, &dataIdx);
		}
	}
	return dataIdx;
}

static int conv_upm_table(const struct upm_setting *upm,
			  int mode, struct rbppc_info *info,
			  unsigned *data) {
#if DEBUG_UPM
	int uidx;
#endif
	int psFinal[32];
	int clkFinal[32];
	int max_uidx;
	int data_len;

	max_uidx = upm_gen_ps_table(upm, mode, info, psFinal);

	upm_gen_clk_table(upm, mode, info->clk_time_ps, max_uidx,
			  psFinal, clkFinal);

#if DEBUG_UPM
	/* dump out debug info */
	for (uidx = 0; uidx < max_uidx; ++uidx) {
		if (clkFinal[uidx]) {
			printk(KERN_INFO "idx %d val %08x clk %d ps %d\n",
				uidx, upm[uidx].value,
				clkFinal[uidx], psFinal[uidx]);
		}
	}
#endif

	data_len = upm_gen_final_data(upm, max_uidx, clkFinal, data);

#if DEBUG_UPM
	for (uidx = 0; uidx < data_len; ++uidx) {
		printk(KERN_INFO "cf UPM x result: idx %d val %08x\n",
		       uidx, data[uidx]);
	}
#endif
	return 0;
}

static int gen_upm_data(int mode, struct rbppc_info *info, unsigned *data) {
	int i;

	for (i = 0; i < UPM_DATA_SIZE; ++i) {
		data[i] = EMPTY;
	}

	if (conv_upm_table(cfUpmReadSingle, mode, info, 
			   data + UPM_READ_SINGLE_OFFSET)) {
		return -1;
	}
	if (conv_upm_table(cfUpmWriteSingle, mode, info,
			   data + UPM_WRITE_SINGLE_OFFSET)) {
		return -1;
	}
	return 0;
}

static void program_upm(void *upmMemAddr,
			volatile void *lbcfg_mxmr, volatile void *lbcfg_mdr,
			const unsigned *upmData,
			unsigned offset, unsigned len) {
    unsigned i;
    unsigned mxmr;

    mxmr = in_be32(lbcfg_mxmr);
    mxmr &= ~(MxMR_OP_MASK | MxMR_MAD_MASK);
    mxmr |= (MxMR_OP_WRITE | offset);
    out_be32(lbcfg_mxmr, mxmr);
    in_be32(lbcfg_mxmr);			/* flush MxMR write */

    for (i = 0; i < len; ++i) {
	int to;
	unsigned data = upmData[i + offset];
	out_be32(lbcfg_mdr, data);
	in_be32(lbcfg_mdr);			/* flush MDR write */

	iowrite8(1, upmMemAddr);	/* dummy write to any CF addr */

	/* wait for dummy write to complete */
	for (to = 10000; to >= 0; --to) {
	    mxmr = in_be32(lbcfg_mxmr);
	    if (((mxmr ^ (i + 1)) & MxMR_MAD_MASK) == 0) break;
	    if (to == 0) {
		printk(KERN_ERR "rbppc cf error: "
		       "UPMx program error at 0x%x: timeout\n", i);
	    }
	}
    }
    mxmr &= ~(MxMR_OP_MASK | MxMR_RLF_MASK | MxMR_WLF_MASK);
    mxmr |= (MxMR_OP_NORMAL |
	     (LOOP_SIZE << MxMR_RLF_SHIFT) |
	     (LOOP_SIZE << MxMR_WLF_SHIFT));
    out_be32(lbcfg_mxmr, mxmr);
}

static int rbppc_update_piomode(struct ata_port *ap, int mode) {
	struct rbppc_info *info = (struct rbppc_info *)ap->host->private_data;
	void *lbcfgBase;
	unsigned upmData[UPM_DATA_SIZE];

	if (gen_upm_data(mode, info, upmData)) {
		return -1;
	}

	lbcfgBase = ioremap_nocache(info->lbcfg_addr, IMMR_LBCFG_SIZE);

	program_upm(ap->ioaddr.cmd_addr,
		    ((char *)lbcfgBase) + LOCAL_BUS_MCMR,
		    ((char *)lbcfgBase) + LOCAL_BUS_MDR,
		    upmData, 0, UPM_DATA_SIZE);
	iounmap(lbcfgBase);
	return 0;
}

static void rbppc_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	struct rbppc_info *info = (struct rbppc_info *)ap->host->private_data;
	int mode = adev->pio_mode - XFER_PIO_0;

	DPRINTK("rbppc_set_piomode PIO %d\n", mode);
	if (mode < 0) mode = 0;
	if (mode > 6) mode = 6;

	if (info->cur_mode < 0 || info->cur_mode > mode) {
		if (rbppc_update_piomode(ap, mode) == 0) {
			printk(KERN_INFO "CF PIO mode changed to %d\n", mode);
			info->cur_mode = mode;
		}
	}
}

static irqreturn_t rbppc_interrupt(int irq, void *dev_instance)
{
	irqreturn_t ret = ata_sff_interrupt(irq, dev_instance);
	if (ret == IRQ_RETVAL(0)) {
		struct ata_host *host = dev_instance;
		struct ata_port *ap = host->ports[0];

		/* clear interrupt */
		rbppc_check_status(ap);

		printk(KERN_WARNING "ata%u: irq %d not handled\n",
		       ap->print_id, irq);
	}
	return ret;
}

static struct scsi_host_template rbppc_sht = {
	.module			= THIS_MODULE,
	.name			= DRV_NAME,
	.ioctl			= ata_scsi_ioctl,
	.queuecommand		= ata_scsi_queuecmd,
	.can_queue		= ATA_DEF_QUEUE,
	.this_id		= ATA_SHT_THIS_ID,
	.sg_tablesize		= LIBATA_MAX_PRD,
	.cmd_per_lun		= ATA_SHT_CMD_PER_LUN,
	.emulated		= ATA_SHT_EMULATED,
	.use_clustering		= ATA_SHT_USE_CLUSTERING,
	.proc_name		= DRV_NAME,
	.dma_boundary		= ATA_DMA_BOUNDARY,
	.slave_configure	= ata_scsi_slave_config,
	.slave_destroy		= ata_scsi_slave_destroy,
	.bios_param		= ata_std_bios_param,
#ifdef CONFIG_PM
	.resume			= ata_scsi_device_resume,
	.suspend		= ata_scsi_device_suspend,
#endif
};

static struct ata_port_operations rbppc_port_ops = {
	.inherits		= &ata_sff_port_ops,

	.set_piomode		= rbppc_set_piomode,

	.sff_check_status	= rbppc_check_status,
	.sff_check_altstatus	= rbppc_check_altstatus,

	.sff_irq_clear		= rbppc_dummy_noret,

	.port_start		= rbppc_dummy_ret0,
};

static int init_rbppc_info(struct platform_device *pdev, struct rbppc_info *info) {
	struct device_node *np;
	struct resource res;
	const u32 *u32ptr;
	void *lbcfgBase;
	void *lbcfg_lcrr;
	unsigned lbc_clk_khz;
	unsigned lbc_extra_divider = 1;
	unsigned ccb_freq_hz;
	unsigned lb_div;

	u32ptr = of_get_property(pdev->dev.of_node, "lbc_extra_divider", NULL);
	if (u32ptr && *u32ptr) {
		lbc_extra_divider = *u32ptr;
#if DEBUG_UPM
		printk(KERN_INFO "CF: LBC extra divider %u\n",
		       lbc_extra_divider);
#endif
	}

	np = of_find_node_by_type(NULL, "serial");
	if (!np) {
		printk(KERN_ERR "rbppc cf error: no serial node found\n");
		return -1;
	}
	u32ptr = of_get_property(np, "clock-frequency", NULL);
	if (u32ptr == 0 || *u32ptr == 0) {
		printk(KERN_ERR "rbppc cf error: "
		       "serial does not have clock-frequency\n");
		of_node_put(np);
		return -1;
	}
	ccb_freq_hz = *u32ptr;
	of_node_put(np);

	np = of_find_node_by_type(NULL, "soc");
	if (!np) {
		printk(KERN_ERR "rbppc cf error: no soc node found\n");
		return -1;
	}
	if (of_address_to_resource(np, 0, &res)) {
		printk(KERN_ERR "rbppc cf error: soc does not have resource\n");
		of_node_put(np);
		return -1;
	}
	info->lbcfg_addr = res.start + IMMR_LBCFG_OFFSET;
	of_node_put(np);

	lbcfgBase = ioremap_nocache(info->lbcfg_addr, IMMR_LBCFG_SIZE);
	lbcfg_lcrr = ((char*)lbcfgBase) + LOCAL_BUS_LCRR;
	lb_div = (in_be32(lbcfg_lcrr) & LCRR_CLKDIV_MASK) * lbc_extra_divider;
	iounmap(lbcfgBase);

	lbc_clk_khz = ccb_freq_hz / (1000 * lb_div);
	info->clk_time_ps = 1000000000 / lbc_clk_khz;
	printk(KERN_INFO "CF: using Local-Bus clock %u kHz %u ps\n",
	       lbc_clk_khz, info->clk_time_ps);

	u32ptr = of_get_property(pdev->dev.of_node, "lb-timings", NULL);
	if (u32ptr) {
		memcpy(info->lb_timings, u32ptr, LBT_SIZE * sizeof(*u32ptr));
#if DEBUG_UPM
		printk(KERN_INFO "CF: got LB timings <%u %u %u %u %u>\n",
		       u32ptr[0], u32ptr[1], u32ptr[2], u32ptr[3], u32ptr[4]);
#endif
	}
	info->cur_mode = -1;
	return 0;
}

static int rbppc_cf_probe(struct platform_device *op)
{
	struct ata_host *host;
	struct ata_port *ap;
	struct rbppc_info *info = NULL;
	struct resource res;
	void *baddr;
	const u32 *u32ptr;
	int irq_level = 0;
	int err = -ENOMEM;

	printk(KERN_INFO "RB_PPC CF\n");

	if (rbinfo == NULL) {
		info = kmalloc(sizeof(*info), GFP_KERNEL);
		if (info == NULL) {
			printk(KERN_ERR "rbppc_cf_probe: OOM\n");
			goto err_info;
		}
		memset(info, 0, sizeof(*info));

		if (init_rbppc_info(op, info)) {
			goto err_info;
		}
		rbinfo = info;
	}
	u32ptr = of_get_property(op->dev.of_node, "interrupt-at-level", NULL);
	if (u32ptr) {
		irq_level = *u32ptr;
		printk(KERN_INFO "CF: irq level %u\n", irq_level);
	}

	if (of_address_to_resource(op->dev.of_node, 0, &res)) {
	    printk(KERN_ERR "rbppc cf error: no reg property found\n");
	    goto err_info;
	}

	host = ata_host_alloc(&op->dev, 1);
	if (!host)
	    goto err_info;

	baddr = localbus_map(res.start, res.end - res.start + 1);
	host->iomap = baddr;
	host->private_data = rbinfo;

	ap = host->ports[0];
	ap->ops = &rbppc_port_ops;
	ap->pio_mask = 0x7F;	/* PIO modes 0-6 */
//	ap->flags = ATA_FLAG_NO_LEGACY;
	ap->mwdma_mask = 0;

	ap->ioaddr.cmd_addr = baddr;
	ata_sff_std_ports(&ap->ioaddr);
	ap->ioaddr.ctl_addr = ap->ioaddr.cmd_addr + 14;
	ap->ioaddr.altstatus_addr = ap->ioaddr.ctl_addr;
	ap->ioaddr.bmdma_addr = 0;

	err = ata_host_activate(
		host,
		irq_of_parse_and_map(op->dev.of_node, 0), rbppc_interrupt,
		irq_level ? IRQF_TRIGGER_HIGH : IRQF_TRIGGER_LOW,
		&rbppc_sht);
	if (!err) return 0;

	localbus_unmap(baddr);
err_info:
	if (info) {
		kfree(info);
		rbinfo = NULL;
	}
	return err;
}

static int rbppc_cf_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);

	if (host == NULL) return -1;

	ata_host_detach(host);
	return 0;
}

static struct of_device_id rbppc_cf_ids[] = {
	{ .name = "cf" },
	{}
};

static struct platform_driver rbppc_cf_driver = {
//	.name   = "cf",
	.probe	= rbppc_cf_probe,
	.remove = rbppc_cf_remove,
//	.match_table = rbppc_cf_ids,
	.driver	= {
		.name = "cf-rbppc",
		.of_match_table = rbppc_cf_ids,
		.owner = THIS_MODULE,
	},
};

static int __init rbppc_init(void)
{
	platform_driver_register(&rbppc_cf_driver);
	return 0;
}

static void __exit rbppc_exit(void)
{
	platform_driver_unregister(&rbppc_cf_driver);
}

module_init(rbppc_init);
module_exit(rbppc_exit);

