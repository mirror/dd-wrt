
#include <linux/ixp_npe.h>
#include <asm/hardware.h>

#define RESET_NPE_PARITY                0x0800
#define PARITY_BIT_MASK             0x3F00FFFF
#define CONFIG_CTRL_REG_MASK        0x3F3FFFFF
#define MAX_RETRIES                    1000000
#define NPE_PHYS_REG                        32
#define RESET_MBST_VAL              0x0000F0F0
#define NPE_REGMAP	            0x0000001E
#define INSTR_WR_REG_SHORT          0x0000C000
#define INSTR_WR_REG_BYTE           0x00004000
#define MASK_ECS_REG_0_NEXTPC       0x1FFF0000

#define INSTR_RD_FIFO               0x0F888220
#define INSTR_RESET_MBOX            0x0FAC8210

#define ECS_REG_0_LDUR                 8
#define ECS_REG_1_CCTXT               16
#define ECS_REG_1_SELCTXT              0

#define ECS_BG_CTXT_REG_0           0x00
#define ECS_BG_CTXT_REG_1           0x01
#define ECS_BG_CTXT_REG_2           0x02
#define ECS_PRI_1_CTXT_REG_0        0x04
#define ECS_PRI_1_CTXT_REG_1        0x05
#define ECS_PRI_1_CTXT_REG_2        0x06
#define ECS_PRI_2_CTXT_REG_0        0x08
#define ECS_PRI_2_CTXT_REG_1        0x09
#define ECS_PRI_2_CTXT_REG_2        0x0A
#define ECS_DBG_CTXT_REG_0          0x0C
#define ECS_DBG_CTXT_REG_1          0x0D
#define ECS_DBG_CTXT_REG_2          0x0E
#define ECS_INSTRUCT_REG            0x11

#define ECS_BG_CTXT_REG_0_RESET     0xA0000000
#define ECS_BG_CTXT_REG_1_RESET     0x01000000
#define ECS_BG_CTXT_REG_2_RESET     0x00008000
#define ECS_PRI_1_CTXT_REG_0_RESET  0x20000080
#define ECS_PRI_1_CTXT_REG_1_RESET  0x01000000
#define ECS_PRI_1_CTXT_REG_2_RESET  0x00008000
#define ECS_PRI_2_CTXT_REG_0_RESET  0x20000080
#define ECS_PRI_2_CTXT_REG_1_RESET  0x01000000
#define ECS_PRI_2_CTXT_REG_2_RESET  0x00008000
#define ECS_DBG_CTXT_REG_0_RESET    0x20000000
#define ECS_DBG_CTXT_REG_1_RESET    0x00000000
#define ECS_DBG_CTXT_REG_2_RESET    0x001E0000
#define ECS_INSTRUCT_REG_RESET      0x1003C00F

static struct { u32 reg; u32 val; } ecs_reset[] =
{
    { ECS_BG_CTXT_REG_0,    ECS_BG_CTXT_REG_0_RESET },
    { ECS_BG_CTXT_REG_1,    ECS_BG_CTXT_REG_1_RESET },
    { ECS_BG_CTXT_REG_2,    ECS_BG_CTXT_REG_2_RESET },
    { ECS_PRI_1_CTXT_REG_0, ECS_PRI_1_CTXT_REG_0_RESET },
    { ECS_PRI_1_CTXT_REG_1, ECS_PRI_1_CTXT_REG_1_RESET },
    { ECS_PRI_1_CTXT_REG_2, ECS_PRI_1_CTXT_REG_2_RESET },
    { ECS_PRI_2_CTXT_REG_0, ECS_PRI_2_CTXT_REG_0_RESET },
    { ECS_PRI_2_CTXT_REG_1, ECS_PRI_2_CTXT_REG_1_RESET },
    { ECS_PRI_2_CTXT_REG_2, ECS_PRI_2_CTXT_REG_2_RESET },
    { ECS_DBG_CTXT_REG_0,   ECS_DBG_CTXT_REG_0_RESET },
    { ECS_DBG_CTXT_REG_1,   ECS_DBG_CTXT_REG_1_RESET },
    { ECS_DBG_CTXT_REG_2,   ECS_DBG_CTXT_REG_2_RESET },
    { ECS_INSTRUCT_REG,     ECS_INSTRUCT_REG_RESET }
};

/* actually I have no idea what I'm doing here !!
 * I only rewrite the "reset" sequence the way Intel does it.
 */

static void npe_debg_preexec(struct npe_info *npe)
{
	u32 r = IX_NPEDL_MASK_ECS_DBG_REG_2_IF | IX_NPEDL_MASK_ECS_DBG_REG_2_IE;

	npe->exec_count = npe_reg_read(npe, IX_NPEDL_REG_OFFSET_EXCT);
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXCT, 0);
	npe->ctx_reg2 = npe_read_ecs_reg(npe, ECS_DBG_CTXT_REG_2);
	npe_write_ecs_reg(npe, ECS_DBG_CTXT_REG_2, npe->ctx_reg2 | r);
}

static void npe_debg_postexec(struct npe_info *npe)
{
	npe_write_ecs_reg(npe, ECS_DBG_CTXT_REG_0, 0);
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXCT, npe->exec_count);
	npe_write_ecs_reg(npe, ECS_DBG_CTXT_REG_2, npe->ctx_reg2);
}

static int
npe_debg_inst_exec(struct npe_info *npe, u32 instr, u32 ctx, u32 ldur)
{
	u32 regval, wc;
	int c = 0;

	regval = IX_NPEDL_MASK_ECS_REG_0_ACTIVE |
		(ldur << ECS_REG_0_LDUR);
	npe_write_ecs_reg(npe, ECS_DBG_CTXT_REG_0 , regval);
	/* set CCTXT at ECS DEBUG L3 to specify in which context
	 * to execute the instruction
	 */
	regval = (ctx << ECS_REG_1_CCTXT) |
		 (ctx << ECS_REG_1_SELCTXT);
	npe_write_ecs_reg(npe, ECS_DBG_CTXT_REG_1, regval);

	/* clear the pipeline */
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);

	/* load NPE instruction into the instruction register */
	npe_write_ecs_reg(npe, ECS_INSTRUCT_REG, instr);
	/* we need this value later to wait for
	 * completion of NPE execution step
	 */
	wc = npe_reg_read(npe, IX_NPEDL_REG_OFFSET_WC);
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_STEP);

	/* Watch Count register increments when NPE completes an instruction */
	while (wc == npe_reg_read(npe, IX_NPEDL_REG_OFFSET_WC) &&
			++c < MAX_RETRIES);

	if (c >= MAX_RETRIES) {
		printk(KERN_ERR "%s reset:npe_debg_inst_exec(): Timeout\n",
				npe->plat->name);
		return 1;
	}
	return 0;
}

static int npe_logical_reg_write8(struct npe_info *npe, u32 addr, u32 val)
{
	u32 instr;
	val &= 0xff;
	/* here we build the NPE assembler instruction:
	 * mov8 d0, #0 */
	instr = INSTR_WR_REG_BYTE |	/* OpCode */
		addr << 9 |		/* base Operand */
		(val & 0x1f) << 4 |	/* lower 5 bits to immediate data */
		(val & ~0x1f) << (18-5);/* higher 3 bits to CoProc instr. */
	/* and execute it */
	return npe_debg_inst_exec(npe, instr, 0, 1);
}

static int npe_logical_reg_write16(struct npe_info *npe, u32 addr, u32 val)
{
	u32 instr;
	/* here we build the NPE assembler instruction:
	 * mov16 d0, #0 */
	val &= 0xffff;
	instr = INSTR_WR_REG_SHORT |	/* OpCode */
		addr << 9 |		/* base Operand */
		(val & 0x1f) << 4 |	/* lower 5 bits to immediate data */
		(val & ~0x1f) << (18-5);/* higher 11 bits to CoProc instr. */
	/* and execute it */
	return npe_debg_inst_exec(npe, instr, 0, 1);
}

static int npe_logical_reg_write32(struct npe_info *npe, u32 addr, u32 val)
{
	/* write in 16 bit steps first the high and then the low value */
	npe_logical_reg_write16(npe, addr, val >> 16);
	return npe_logical_reg_write16(npe, addr+2, val & 0xffff);
}

void npe_reset(struct npe_info *npe)
{
	u32 reg, cfg_ctrl;
	int i;
	struct { u32 reset; int addr; int size; } ctx_reg[] = {
		{ 0x80,  0x1b, 8  },
		{ 0,     0x1c, 16 },
		{ 0x820, 0x1e, 16 },
		{ 0,     0x1f, 8  }
	}, *cr;

	cfg_ctrl = npe_reg_read(npe, IX_NPEDL_REG_OFFSET_CTL);
	cfg_ctrl |= 0x3F000000;
	/* disable the parity interrupt */
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_CTL, cfg_ctrl & PARITY_BIT_MASK);

	npe_debg_preexec(npe);

	/* clear the FIFOs */
	while (npe_reg_read(npe, IX_NPEDL_REG_OFFSET_WFIFO) ==
					IX_NPEDL_MASK_WFIFO_VALID);
	while (npe_reg_read(npe, IX_NPEDL_REG_OFFSET_STAT) ==
					IX_NPEDL_MASK_STAT_OFNE)
	{
		u32 reg;
		reg = npe_reg_read(npe, IX_NPEDL_REG_OFFSET_FIFO);
		printk("%s reset: Read FIFO:=%x\n", npe->plat->name, reg);
	}
	while (npe_reg_read(npe, IX_NPEDL_REG_OFFSET_STAT) ==
					IX_NPEDL_MASK_STAT_IFNE) {
		npe_debg_inst_exec(npe, INSTR_RD_FIFO, 0, 0);
	}

	/*  Reset the mailbox reg */
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_MBST, RESET_MBST_VAL);
	npe_debg_inst_exec(npe, INSTR_RESET_MBOX, 0, 0);

	/* Reset the physical registers in the NPE register file */
	for (i=0; i<NPE_PHYS_REG; i++) {
		npe_logical_reg_write16(npe, NPE_REGMAP, i >> 1);
		npe_logical_reg_write32(npe, (i&1) *4, 0);
	}

	/* Reset the context store. Iterate over the 16 ctx s */
	for(i=0; i<16; i++) {
		for (reg=0; reg<4; reg++) {
			/* There is no (STEVT) register for Context 0.
			 * ignore if register=0 and ctx=0 */
			if (!(reg || i))
				continue;
			 /* Context 0 has no STARTPC. Instead, this value is
			  * used to set NextPC for Background ECS,
			  * to set where NPE starts executing code
			  */
			if (!i && reg==1) {
				u32 r;
				r = npe_read_ecs_reg(npe, ECS_BG_CTXT_REG_0);
				r &= ~MASK_ECS_REG_0_NEXTPC;
				r |= (cr->reset << 16) & MASK_ECS_REG_0_NEXTPC;
				continue;
			}
			cr = ctx_reg + reg;
			switch (cr->size) {
				case 8:
					npe_logical_reg_write8(npe, cr->addr,
						cr->reset);
					break;
				case 16:
					npe_logical_reg_write16(npe, cr->addr,
						cr->reset);
			}
		}
	}
	npe_debg_postexec(npe);

	for (i=0; i< ARRAY_SIZE(ecs_reset); i++) {
		npe_write_ecs_reg(npe, ecs_reset[i].reg, ecs_reset[i].val);
	}
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_CLR_PROFILE_CNT);

	for (i=IX_NPEDL_REG_OFFSET_EXCT; i<=IX_NPEDL_REG_OFFSET_AP3; i+=4) {
		npe_reg_write(npe, i, 0);
	}

	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_WC, 0);

	reg = *IXP4XX_EXP_CFG2;
	reg |= 0x800 << npe->plat->id;  /* IX_FUSE_NPE[ABC] */
	*IXP4XX_EXP_CFG2 = reg;
	reg &= ~(0x800 << npe->plat->id);  /* IX_FUSE_NPE[ABC] */
	*IXP4XX_EXP_CFG2 = reg;

	npe_stop(npe);

	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_CTL,
			cfg_ctrl & CONFIG_CTRL_REG_MASK);
	npe->loaded = 0;
}


void npe_stop(struct npe_info *npe)
{
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_STOP);
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);
}

static void npe_reset_active(struct npe_info *npe, u32 reg)
{
	u32 regval;

	regval = npe_read_ecs_reg(npe, reg);
	regval &= ~IX_NPEDL_MASK_ECS_REG_0_ACTIVE;
	npe_write_ecs_reg(npe, reg, regval);
}

void npe_start(struct npe_info *npe)
{
	npe_reset_active(npe, IX_NPEDL_ECS_PRI_1_CTXT_REG_0);
	npe_reset_active(npe, IX_NPEDL_ECS_PRI_2_CTXT_REG_0);
	npe_reset_active(npe, IX_NPEDL_ECS_DBG_CTXT_REG_0);

	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE);
	npe_write_exctl(npe, IX_NPEDL_EXCTL_CMD_NPE_START);
}

EXPORT_SYMBOL(npe_stop);
EXPORT_SYMBOL(npe_start);
EXPORT_SYMBOL(npe_reset);
