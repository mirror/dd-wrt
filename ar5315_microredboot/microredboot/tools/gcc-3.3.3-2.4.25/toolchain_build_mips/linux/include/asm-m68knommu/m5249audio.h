/****************************************************************************/

/*
 *	m5249audio.h -- ColdFire internal audio support defines.
 *
 *	(C) Copyright 2002, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/
#ifndef	m5249audio_h
#define	m5249audio_h
/****************************************************************************/

/*
 *	Define the ColdFire audio register set addresses.
 */
#define	MCFA_IISCONFIG		0x10		/* IIS config regs (r/w) */
#define	MCFA_IIS1CONFIG		0x10		/* IIS1 config reg (r/w) */
#define	MCFA_IIS2CONFIG		0x14		/* IIS2 config reg (r/w) */
#define	MCFA_IIS3CONFIG		0x18		/* IIS3 config reg (r/w) */
#define	MCFA_IIS4CONFIG		0x1c		/* IIS4 config reg (r/w) */

#define	MCFA_DATAINCTRL		0x30		/* Data in Control reg (r/w) */

#define	MCFA_PDIR1L		0x34		/* Data in left #1 (r) */
#define	MCFA_PDIR3L		0x44		/* Data in left #3 (r) */
#define	MCFA_PDIR1R		0x54		/* Data in right #1 (r) */
#define	MCFA_PDIR3R		0x64		/* Data in right #3 (r) */
#define	MCFA_PDOR1L		0x34		/* Data out left #1 (w) */
#define	MCFA_PDOR1R		0x44		/* Data out right #1 (w) */
#define	MCFA_PDOR2L		0x54		/* Data out left #2 (w) */
#define	MCFA_PDOR2R		0x64		/* Data out right #2 (w) */
#define	MCFA_PDOR3		0x74		/* Data out l+r #3 (w) */
#define	MCFA_PDIR2		0x74		/* Data in l+r #2 (w) */

#define	MCFA_INTENABLE		0x94		/* Interrupt enable (r/w) */
#define	MCFA_INTCLEAR		0x98		/* Interrupt clear (w) */
#define	MCFA_INTSTAT		0x98		/* Interupt status (r) */

#define	MCFA_DMACONF		0x9f		/* Audio DMA config (r/w) */

#define	MCFA_AUDIOGLOB		0xcc		/* Audio glob register (r/w) */


/*
 *	Flag defines for the IIS configuration registers.
 */
#define	MCFA_IIS_EFCFLG		0x00040000	/* EF/CFLG insert */
#define	MCFA_IIS_CFLGPOS	0x00020000	/* CFLG sample position */
#define	MCFA_IIS_CLKINPUT	0x00000000	/* CLK is SCLK/LRCLK input */
#define	MCFA_IIS_CLK24		0x00001000	/* CLK is AudioClk/24 */
#define	MCFA_IIS_CLK16		0x00002000	/* CLK is AudioClk/16 */
#define	MCFA_IIS_CLK12		0x00003000	/* CLK is AudioClk/12 */
#define	MCFA_IIS_CLK8		0x00004000	/* CLK is AudioClk/8 */
#define	MCFA_IIS_CLK6		0x00005000	/* CLK is AudioClk/6 */
#define	MCFA_IIS_CLK4		0x00006000	/* CLK is AudioClk/4 */
#define	MCFA_IIS_CLK3		0x00007000	/* CLK is AudioClk/3 */
#define	MCFA_IIS_CLK2		0x0000c000	/* CLK is AudioClk/2 */
#define	MCFA_IIS_CLKIIS1	0x00008000	/* CLK is IIS1 clock */
#define	MCFA_IIS_CLKIIS2	0x00009000	/* CLK is IIS2 clock */
#define	MCFA_IIS_CLKIIS3	0x0000a000	/* CLK is IIS3 clock */
#define	MCFA_IIS_CLKIIS4	0x0000b000	/* CLK is IIS4 clock */
#define	MCFA_IIS_FIFO		0x00000000	/* Normal FIFO operation */
#define	MCFA_IIS_FIFO1		0x00000800	/* 1 sample FIFO mode */
#define	MCFA_IIS_TXSRC_0	0x00000000	/* TX source is digital 0 */
#define	MCFA_IIS_TXSRC_PDOR1	0x00000100	/* TX source is PDOR1 */
#define	MCFA_IIS_TXSRC_PDOR2	0x00000200	/* TX source is PDOR2 */
#define	MCFA_IIS_TXSRC_PDOR3	0x00000300	/* TX source is PDOR3 */
#define	MCFA_IIS_TXSRC_IIS1	0x00000400	/* TX source is IIS1 in data */
#define	MCFA_IIS_TXSRC_IIS3	0x00000500	/* TX source is IIS3 in data */
#define	MCFA_IIS_TXSRC_IIS4	0x00000600	/* TX source is IIS4 in data */
#define	MCFA_IIS_TXSRC_EBU	0x00000700	/* TX source is EBU in data */
#define	MCFA_IIS_TXSRC_EBU2	0x00010000	/* TX source is EBU2 in data */
#define	MCFA_IIS_16BIT		0x00000000	/* 16 bit samples */
#define	MCFA_IIS_18BIT		0x00000040	/* 18 bit samples */
#define	MCFA_IIS_20BIT		0x00000080	/* 20 bit samples */
#define	MCFA_IIS_0BIT		0x000000c0	/* 0 sample */
#define	MCFA_IIS_MODE_IIS	0x00000000	/* Philips IIS mode */
#define	MCFA_IIS_MODE_EIAJ	0x00000000	/* Sony EIAJ mode */
#define	MCFA_IIS_LRCK64BIT	0x00000010	/* LRCK frequency 64 bit clk */
#define	MCFA_IIS_LRCK48BIT	0x00000008	/* LRCK frequency 48 bit clk */
#define	MCFA_IIS_LRCK32BIT	0x00000000	/* LRCK frequency 32 bit clk */
#define	MCFA_IIS_LRCKINV	0x00000002	/* Invert LRCK */
#define	MCFA_IIS_SCLKINV	0x00000001	/* Invert SCLK */

/*
 *	Data in Control register flags.
 */
#define	MCFA_DIC_PDIR3_0	0x00800000	/* PDIR3 zero control */
#define	MCFA_DIC_PDIR3_RESET	0x00400000	/* PDIR3 reset */
#define	MCFA_DIC_PDIR3_1SAMP	0x00000000	/* PDIR3 full on 1 sample */
#define	MCFA_DIC_PDIR3_2SAMP	0x00100000	/* PDIR3 full on 2 sample */
#define	MCFA_DIC_PDIR3_3SAMP	0x00200000	/* PDIR3 full on 3 sample */
#define	MCFA_DIC_PDIR3_6SAMP	0x00300000	/* PDIR3 full on 6 sample */
#define	MCFA_DIC_PDIR3_OFF	0x00000000	/* PDIR3 Disabled */
#define	MCFA_DIC_PDIR3_PDOR1	0x00010000	/* PDIR3 PDOR1 data */
#define	MCFA_DIC_PDIR3_PDOR2	0x00020000	/* PDIR3 PDOR2 data */
#define	MCFA_DIC_PDIR3_IIS1	0x00040000	/* PDIR3 IIS1 receiver data */
#define	MCFA_DIC_PDIR3_IIS3	0x00050000	/* PDIR3 IIS3 receiver data */
#define	MCFA_DIC_PDIR3_IIS4	0x00060000	/* PDIR3 IIS4 receiver data */
#define	MCFA_DIC_PDIR3_EBU1	0x00070000	/* PDIR3 EBU1 receiver data */
#define	MCFA_DIC_PDIR3_EBU2	0x00080000	/* PDIR3 EBU2 receiver data */
#define	MCFA_DIC_PDIR2_1SAMP	0x00000000	/* PDIR2 full on 1 sample */
#define	MCFA_DIC_PDIR2_2SAMP	0x00004000	/* PDIR2 full on 2 sample */
#define	MCFA_DIC_PDIR2_3SAMP	0x00008000	/* PDIR2 full on 3 sample */
#define	MCFA_DIC_PDIR2_6SAMP	0x0000c000	/* PDIR2 full on 6 sample */
#define	MCFA_DIC_PDIR2_0	0x00000800	/* PDIR2 zero control */
#define	MCFA_DIC_PDIR1_0	0x00000400	/* PDIR1 zero control */
#define	MCFA_DIC_PDIR2_RESET	0x00000200	/* PDIR2 reset */
#define	MCFA_DIC_PDIR1_RESET	0x00000100	/* PDIR1 reset */
#define	MCFA_DIC_PDIR1_1SAMP	0x00000000	/* PDIR1 full on 1 sample */
#define	MCFA_DIC_PDIR1_2SAMP	0x00000040	/* PDIR1 full on 2 sample */
#define	MCFA_DIC_PDIR1_3SAMP	0x00000080	/* PDIR1 full on 3 sample */
#define	MCFA_DIC_PDIR1_6SAMP	0x000000c0	/* PDIR1 full on 6 sample */
#define	MCFA_DIC_PDIR2_OFF	0x00000000	/* PDIR2 disable */
#define	MCFA_DIC_PDIR2_PDOR1	0x00000008	/* PDIR2 PDOR1 data */
#define	MCFA_DIC_PDIR2_PDOR2	0x00000010	/* PDIR2 PDOR2 data */
#define	MCFA_DIC_PDIR2_IIS1	0x00000020	/* PDIR2 IIS1 data */
#define	MCFA_DIC_PDIR2_IIS3	0x00000028	/* PDIR2 IIS3 data */
#define	MCFA_DIC_PDIR2_IIS4	0x00000030	/* PDIR2 IIS4 data */
#define	MCFA_DIC_PDIR2_EBU1	0x00000038	/* PDIR2 EBU1 data */
#define	MCFA_DIC_PDIR2_EBU2	0x00002000	/* PDIR2 EBU1 data */
#define	MCFA_DIC_PDIR1_OFF	0x00000000	/* PDIR1 disable */
#define	MCFA_DIC_PDIR1_PDOR1	0x00000001	/* PDIR1 PDOR1 data */
#define	MCFA_DIC_PDIR1_PDOR2	0x00000002	/* PDIR1 PDOR2 data */
#define	MCFA_DIC_PDIR1_IIS1	0x00000004	/* PDIR1 IIS1 data */
#define	MCFA_DIC_PDIR1_IIS3	0x00000005	/* PDIR1 IIS3 data */
#define	MCFA_DIC_PDIR1_IIS4	0x00000006	/* PDIR1 IIS4 data */
#define	MCFA_DIC_PDIR1_EBU1	0x00000007	/* PDIR1 EBU1 data */
#define	MCFA_DIC_PDIR1_EBU2	0x00001000	/* PDIR1 EBU2 data */

/*
 *	Audio glob register bit flags.
 */
#define	MCFA_GB_PDIR3_AUTOSYNC	0x00001000	/* PDIR3 auto sync mode */
#define	MCFA_GB_EBU1_AUTOSYNC	0x00000400	/* EBU1 TX auto sync mode */
#define	MCFA_GB_IIS2_AUTOSYNC	0x00000200	/* IIS2 auto sync mode */
#define	MCFA_GB_IIS1_AUTOSYNC	0x00000100	/* IIS1 auto sync mode */
#define	MCFA_GB_PDIR2_AUTOSYNC	0x00000080	/* PDIR2 auto sync mode */
#define	MCFA_GB_PDIR1_AUTOSYNC	0x00000040	/* PDIR1 auto sync mode */
#define	MCFA_GB_1TICK		0x00000000	/* Interrupt on 1 tick */
#define	MCFA_GB_2TICK		0x00000008	/* Interrupt on 2 ticks */
#define	MCFA_GB_3TICK		0x00000010	/* Interrupt on 3 ticks */
#define	MCFA_GB_4TICK		0x00000018	/* Interrupt on 4 ticks */
#define	MCFA_GB_5TICK		0x00000020	/* Interrupt on 5 ticks */
#define	MCFA_GB_TICK_OFF	0x00000000	/* Tick disable */
#define	MCFA_GB_TICK_IIS1TX	0x00000001	/* Tick on IIS1 TX */
#define	MCFA_GB_TICK_IIS2TX	0x00000002	/* Tick on IIS2 TX */
#define	MCFA_GB_TICK_EBUTX	0x00000003	/* Tick on EBU TX */
#define	MCFA_GB_TICK_IIS1RX	0x00000004	/* Tick on IIS1 RX */
#define	MCFA_GB_TICK_IIS3RX	0x00000005	/* Tick on IIS3 RX */
#define	MCFA_GB_TICK_IIS4RX	0x00000006	/* Tick on IIS4 RX */
#define	MCFA_GB_TICK_EBU1RX	0x00000007	/* Tick on EBU1 RX */
#define	MCFA_GB_TICK_EBU2RX	0x00001000	/* Tick on EBU2 RX */

/*
 *	Flags for the DMA configuration register.
 */
#define	MCFA_DMA_0REQ		0x01		/* PDOR3 DMA on chan 0 */
#define	MCFA_DMA_1REQ		0x02		/* PDOR3 DMA on chan 1 */

/****************************************************************************/
#endif	/* m5249audio_h */
