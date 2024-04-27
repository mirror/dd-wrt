/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

#ifndef __SYN_DMA_REG_H__
#define __SYN_DMA_REG_H__

/*
 * DMA Register offset
 */
#define SYN_DMA_BUS_MODE				0x1000
#define SYN_DMA_TX_POLL_DEMAND				0x1004
#define SYN_DMA_RX_POLL_DEMAND				0x1008
#define SYN_DMA_RX_DESCRIPTOR_LIST_ADDRESS		0x100C
#define SYN_DMA_TX_DESCRIPTOR_LIST_ADDRESS		0x1010
#define SYN_DMA_STATUS					0x1014
#define SYN_DMA_OPERATION_MODE				0x1018
#define SYN_DMA_INT_ENABLE				0x101C
#define SYN_DMA_MISSED_FRAME_AND_BUFF_OVERFLOW_COUNTER	0x1020
#define SYN_DMA_RX_INTERRUPT_WATCHDOG_TIMER		0x1024
#define SYN_DMA_AXI_BUS_MODE				0x1028
#define SYN_DMA_AHB_OR_AXI_STATUS			0x102C
#define SYN_DMA_CURRENT_HOST_TX_DESCRIPTOR		0x1048
#define SYN_DMA_CURRENT_HOST_RX_DESCRIPTOR		0x104C
#define SYN_DMA_CURRENT_HOST_TX_BUFFER_ADDRESS		0x1050
#define SYN_DMA_CURRENT_HOST_RX_BUFFER_ADDRESS		0x1054

/*
 * SYN_DMA_BUS_MODE = 0x0000,		CSR0 - Bus Mode
 */
enum syn_dma_bus_mode_reg {
	SYN_DMA_FIXED_BURST_ENABLE = 0x00010000,	/* (FB)Fixed Burst SINGLE, INCR4,
							   INCR8 or INCR16 */
	SYN_DMA_FIXED_BURST_DISABLE = 0x00000000,	/* SINGLE, INCR */
	SYN_DMA_TX_PRIORITY_RATIO11 = 0x00000000,	/* (PR)TX:RX DMA priority ratio 1:1 */
	SYN_DMA_TX_PRIORITY_RATIO21 = 0x00004000,	/* (PR)TX:RX DMA priority ratio 2:1 */
	SYN_DMA_TX_PRIORITY_RATIO31 = 0x00008000,	/* (PR)TX:RX DMA priority ratio 3:1 */
	SYN_DMA_TX_PRIORITY_RATIO41 = 0x0000C000,	/* (PR)TX:RX DMA priority ratio 4:1 */
	SYN_DMA_ADDRESS_ALIGNED_BEATS = 0x02000000,	/* Address Aligned beats */
	SYN_DMA_BURST_LENGTHX8 = 0x01000000,		/* When set mutiplies the PBL by 8 */
	SYN_DMA_BURST_LENGTH256 = 0x01002000,		/* (dma_burst_lengthx8 |
							   dma_burst_length32) = 256 */
	SYN_DMA_BURST_LENGTH128 = 0x01001000,		/* (dma_burst_lengthx8 |
							   dma_burst_length16) = 128 */
	SYN_DMA_BURST_LENGTH64 = 0x01000800,		/* (dma_burst_lengthx8 |
							   dma_burst_length8) = 64 */
	/* (PBL) programmable burst length */
	SYN_DMA_BURST_LENGTH32 = 0x00002000,		/* Dma burst length = 32 */
	SYN_DMA_BURST_LENGTH16 = 0x00001000,		/* Dma burst length = 16 */
	SYN_DMA_BURST_LENGTH8 = 0x00000800,		/* Dma burst length = 8 */
	SYN_DMA_BURST_LENGTH4 = 0x00000400,		/* Dma burst length = 4 */
	SYN_DMA_BURST_LENGTH2 = 0x00000200,		/* Dma burst length = 2 */
	SYN_DMA_BURST_LENGTH1 = 0x00000100,		/* Dma burst length = 1 */
	SYN_DMA_BURST_LENGTH0 = 0x00000000,		/* Dma burst length = 0 */

	SYN_DMA_DESCRIPTOR8_WORDS = 0x00000080,		/* Enh Descriptor works  1=>
							   8 word descriptor */
	SYN_DMA_DESCRIPTOR4_WORDS = 0x00000000,		/* Enh Descriptor works  0=>
							   4 word descriptor */
	SYN_DMA_DESCRIPTOR_SKIP16 = 0x00000040,		/* (DSL)Descriptor skip length (no.of dwords) */
	SYN_DMA_DESCRIPTOR_SKIP8 = 0x00000020,		/* between two unchained descriptors */
	SYN_DMA_DESCRIPTOR_SKIP4 = 0x00000010,
	SYN_DMA_DESCRIPTOR_SKIP2 = 0x00000008,
	SYN_DMA_DESCRIPTOR_SKIP1 = 0x00000004,
	SYN_DMA_DESCRIPTOR_SKIP0 = 0x00000000,
	SYN_DMA_ARBIT_RR = 0x00000000,			/* (DA) DMA RR arbitration */
	SYN_DMA_ARBIT_PR = 0x00000002,			/* Rx has priority over Tx */
	SYN_DMA_RESET_ON = 0x00000001,			/* (SWR)Software Reset DMA engine */
	SYN_DMA_RESET_OFF = 0x00000000,
};

/*
 * SYN_DMA_STATUS = 0x0014,		CSR5 - Dma status Register
 */
enum syn_dma_status_reg {
	SYN_DMA_GMAC_PMT_INTR = 0x10000000,		/* (GPI)Gmac subsystem interrupt */
	SYN_DMA_GMAC_MMC_INTR = 0x08000000,		/* (GMI)Gmac MMC subsystem interrupt */
	SYN_DMA_GMAC_LINE_INTF_INTR = 0x04000000,	/* Line interface interrupt */
	SYN_DMA_ERROR_BIT2 = 0x02000000,		/* (EB)Error bits 0-data buffer, 1-desc access */
	SYN_DMA_ERROR_BIT1 = 0x01000000,		/* (EB)Error bits 0-write trnsf, 1-read transfer */
	SYN_DMA_ERROR_BIT0 = 0x00800000,		/* (EB)Error bits 0-Rx DMA, 1-Tx DMA */
	SYN_DMA_TX_STATE = 0x00700000,			/* (TS)Transmit process state */
	SYN_DMA_TX_STOPPED = 0x00000000,		/* Stopped - Reset or Stop Tx Command issued */
	SYN_DMA_TX_FETCHING = 0x00100000,		/* Running - fetching the Tx descriptor */
	SYN_DMA_TX_WAITING = 0x00200000,		/* Running - waiting for status */
	SYN_DMA_TX_READING = 0x00300000,		/* Running - reading the data from host memory */
	SYN_DMA_TX_SUSPENDED = 0x00600000,		/* Suspended - Tx Descriptor unavailabe */
	SYN_DMA_TX_CLOSING = 0x00700000,		/* Running - closing Rx descriptor */
	SYN_DMA_RX_STATE = 0x000E0000,			/* (RS)Receive process state */
	SYN_DMA_RX_STOPPED = 0x00000000,		/* Stopped - Reset or Stop Rx Command issued */
	SYN_DMA_RX_FETCHING = 0x00020000,		/* Running - fetching the Rx descriptor */
	SYN_DMA_RX_WAITING = 0x00060000,		/* Running - waiting for packet */
	SYN_DMA_RX_SUSPENDED = 0x00080000,		/* Suspended - Rx Descriptor unavailable */
	SYN_DMA_RX_CLOSING = 0x000A0000,		/* Running - closing descriptor */
	SYN_DMA_RX_QUEUING = 0x000E0000,		/* Running - queuing the receive frame into host memory */
	SYN_DMA_INT_NORMAL = 0x00010000,		/* (NIS)Normal interrupt summary */
	SYN_DMA_INT_ABNORMAL = 0x00008000,		/* (AIS)Abnormal interrupt summary */
	SYN_DMA_INT_EARLY_RX = 0x00004000,		/* Early receive interrupt (Normal) */
	SYN_DMA_INT_BUS_ERROR = 0x00002000,		/* Fatal bus error (Abnormal) */
	SYN_DMA_INT_EARLY_TX = 0x00000400,		/* Early transmit interrupt (Abnormal) */
	SYN_DMA_INT_RX_WDOG_TO = 0x00000200,		/* Receive Watchdog Timeout (Abnormal) */
	SYN_DMA_INT_RX_STOPPED = 0x00000100,		/* Receive process stopped (Abnormal) */
	SYN_DMA_INT_RX_NO_BUFFER = 0x00000080,		/* RX buffer unavailable (Abnormal) */
	SYN_DMA_INT_RX_COMPLETED = 0x00000040,		/* Completion of frame RX (Normal) */
	SYN_DMA_INT_TX_UNDERFLOW = 0x00000020,		/* Transmit underflow (Abnormal) */
	SYN_DMA_INT_RCV_OVERFLOW = 0x00000010,		/* RX Buffer overflow interrupt */
	SYN_DMA_INT_TX_JABBER_TO = 0x00000008,		/* TX Jabber Timeout (Abnormal) */
	SYN_DMA_INT_TX_NO_BUFFER = 0x00000004,		/* TX buffer unavailable (Normal) */
	SYN_DMA_INT_TX_STOPPED = 0x00000002,		/* TX process stopped (Abnormal) */
	SYN_DMA_INT_TX_COMPLETED = 0x00000001,		/* Transmit completed (Normal) */
};

/*
 * SYN_DMA_OPERATION_MODE = 0x0018,		CSR6 - Dma Operation Mode Register
 */
enum syn_dma_operation_mode_reg {
	SYN_DMA_DISABLE_DROP_TCP_CS = 0x04000000,	/* (DT) Dis. drop. of tcp/ip
							   CS error frames */
	SYN_DMA_RX_STORE_AND_FORWARD = 0x02000000,	/* Rx (SF)Store and forward */
	SYN_DMA_RX_FRAME_FLUSH = 0x01000000,		/* Disable Receive Frame Flush */
	SYN_DMA_TX_STORE_AND_FORWARD = 0x00200000,	/* Tx (SF)Store and forward */
	SYN_DMA_FLUSH_TX_FIFO = 0x00100000,		/* (FTF)Tx FIFO controller
							   is reset to default */
	SYN_DMA_TX_THRESH_CTRL = 0x0001C000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo */
	SYN_DMA_TX_THRESH_CTRL16 = 0x0001C000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 16 */
	SYN_DMA_TX_THRESH_CTRL24 = 0x00018000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 24 */
	SYN_DMA_TX_THRESH_CTRL32 = 0x00014000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 32 */
	SYN_DMA_TX_THRESH_CTRL40 = 0x00010000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 40 */
	SYN_DMA_TX_THRESH_CTRL256 = 0x0000c000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 256 */
	SYN_DMA_TX_THRESH_CTRL192 = 0x00008000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 192 */
	SYN_DMA_TX_THRESH_CTRL128 = 0x00004000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 128 */
	SYN_DMA_TX_THRESH_CTRL64 = 0x00000000,		/* (TTC)Controls thre Thresh of
							   MTL tx Fifo 64 */
	SYN_DMA_TX_START = 0x00002000,			/* (ST)Start/Stop transmission */
	SYN_DMA_RX_FLOW_CTRL_DEACT = 0x00401800,	/* (RFD)Rx flow control
							   deact. Threshold */
	SYN_DMA_RX_FLOW_CTRL_DEACT1K = 0x00000000,	/* (RFD)Rx flow control
							   deact. Threshold (1kbytes) */
	SYN_DMA_RX_FLOW_CTRL_DEACT2K = 0x00000800,	/* (RFD)Rx flow control
							   deact. Threshold (2kbytes) */
	SYN_DMA_RX_FLOW_CTRL_DEACT3K = 0x00001000,	/* (RFD)Rx flow control
							   deact. Threshold (3kbytes) */
	SYN_DMA_RX_FLOW_CTRL_DEACT4K = 0x00001800,	/* (RFD)Rx flow control
							   deact. Threshold (4kbytes) */
	SYN_DMA_RX_FLOW_CTRL_DEACT5K = 0x00400000,	/* (RFD)Rx flow control
							   deact. Threshold (4kbytes) */
	SYN_DMA_RX_FLOW_CTRL_DEACT6K = 0x00400800,	/* (RFD)Rx flow control
							   deact. Threshold (4kbytes) */
	SYN_DMA_RX_FLOW_CTRL_DEACT7K = 0x00401000,	/* (RFD)Rx flow control
							   deact. Threshold (4kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT = 0x00800600,		/* (RFA)Rx flow control
							   Act. Threshold */
	SYN_DMA_RX_FLOW_CTRL_ACT1K = 0x00000000,	/* (RFA)Rx flow control
							   Act. Threshold (1kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT2K = 0x00000200,	/* (RFA)Rx flow control
							   Act. Threshold (2kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT3K = 0x00000400,	/* (RFA)Rx flow control
							   Act. Threshold (3kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT4K = 0x00000600,	/* (RFA)Rx flow control
							   Act. Threshold (4kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT5K = 0x00800000,	/* (RFA)Rx flow control
							   Act. Threshold (5kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT6K = 0x00800200,	/* (RFA)Rx flow control
							   Act. Threshold (6kbytes) */
	SYN_DMA_RX_FLOW_CTRL_ACT7K = 0x00800400,	/* (RFA)Rx flow control
							   Act. Threshold (7kbytes) */
	SYN_DMA_RX_THRESH_CTRL = 0x00000018,		/* (RTC)Controls thre
							   Thresh of MTL rx Fifo */
	SYN_DMA_RX_THRESH_CTRL64 = 0x00000000,		/* (RTC)Controls thre
							   Thresh of MTL tx Fifo 64 */
	SYN_DMA_RX_THRESH_CTRL32 = 0x00000008,		/* (RTC)Controls thre
							   Thresh of MTL tx Fifo 32 */
	SYN_DMA_RX_THRESH_CTRL96 = 0x00000010,		/* (RTC)Controls thre
							   Thresh of MTL tx Fifo 96 */
	SYN_DMA_RX_THRESH_CTRL128 = 0x00000018,		/* (RTC)Controls thre
							   Thresh of MTL tx Fifo 128 */
	SYN_DMA_EN_HW_FLOW_CTRL = 0x00000100,		/* (EFC)Enable HW flow control */
	SYN_DMA_DIS_HW_FLOW_CTRL = 0x00000000,		/* Disable HW flow control */
	SYN_DMA_FWD_ERROR_FRAMES = 0x00000080,		/* (FEF)Forward error frames */
	SYN_DMA_FWD_UNDER_SZ_FRAMES = 0x00000040,	/* (FUF)Forward undersize
							   frames */
	SYN_DMA_TX_SECOND_FRAME = 0x00000004,		/* (OSF)Operate on 2nd frame */
	SYN_DMA_RX_START = 0x00000002,			/* (SR)Start/Stop reception */
};

/*
 * SYN_DMA_INT_ENABLE = 0x101C,		CSR7 - Interrupt enable Register Layout
 */
enum syn_dma_interrupt_reg {
	SYN_DMA_IE_NORMAL = SYN_DMA_INT_NORMAL,		/* Normal interrupt enable */
	SYN_DMA_IE_ABNORMAL = SYN_DMA_INT_ABNORMAL,	/* Abnormal interrupt enable */
	SYN_DMA_IE_EARLY_RX = SYN_DMA_INT_EARLY_RX,	/* Early RX interrupt enable */
	SYN_DMA_IE_BUS_ERROR = SYN_DMA_INT_BUS_ERROR,	/* Fatal bus error enable */
	SYN_DMA_IE_EARLY_TX = SYN_DMA_INT_EARLY_TX,	/* Early TX interrupt enable */
	SYN_DMA_IE_RX_WDOG_TO = SYN_DMA_INT_RX_WDOG_TO,	/* RX Watchdog Timeout enable */
	SYN_DMA_IE_RX_STOPPED = SYN_DMA_INT_RX_STOPPED,	/* RX process stopped enable */
	SYN_DMA_IE_RX_NO_BUFFER = SYN_DMA_INT_RX_NO_BUFFER,
							/* Receive buffer unavailable enable */
	SYN_DMA_IE_RX_COMPLETED = SYN_DMA_INT_RX_COMPLETED,
							/* Completion of frame reception enable */
	SYN_DMA_IE_TX_UNDERFLOW = SYN_DMA_INT_TX_UNDERFLOW,
							/* TX underflow enable */
	SYN_DMA_IE_RX_OVERFLOW = SYN_DMA_INT_RCV_OVERFLOW,
							/* RX Buffer overflow interrupt */
	SYN_DMA_IE_TX_JABBER_TO = SYN_DMA_INT_TX_JABBER_TO,
							/* TX Jabber Timeout enable */
	SYN_DMA_IE_TX_NO_BUFFER = SYN_DMA_INT_TX_NO_BUFFER,
							/* TX buffer unavailable enable */
	SYN_DMA_IE_TX_STOPPED = SYN_DMA_INT_TX_STOPPED,
							/* TX process stopped enable */
	SYN_DMA_IE_TX_COMPLETED = SYN_DMA_INT_TX_COMPLETED,
							/* TX completed enable */
};

/*
 * SYN_DMA_AXI_BUS_MODE = 0x1028
 */
enum syn_dma_axi_bus_mode_reg {
	SYN_DMA_EN_LPI = 0x80000000,
	SYN_DMA_LPI_XIT_FRM = 0x40000000,
	SYN_DMA_WR_OSR_NUM_REQS16 = 0x00F00000,
	SYN_DMA_WR_OSR_NUM_REQS8 = 0x00700000,
	SYN_DMA_WR_OSR_NUM_REQS4 = 0x00300000,
	SYN_DMA_WR_OSR_NUM_REQS2 = 0x00100000,
	SYN_DMA_WR_OSR_NUM_REQS1 = 0x00000000,
	SYN_DMA_RD_OSR_NUM_REQS16 = 0x000F0000,
	SYN_DMA_RD_OSR_NUM_REQS8 = 0x00070000,
	SYN_DMA_RD_OSR_NUM_REQS4 = 0x00030000,
	SYN_DMA_RD_OSR_NUM_REQS2 = 0x00010000,
	SYN_DMA_RD_OSR_NUM_REQS1 = 0x00000000,
	SYN_DMA_ONEKBBE = 0x00002000,
	SYN_DMA_AXI_AAL = 0x00001000,
	SYN_DMA_AXI_BLEN256 = 0x00000080,
	SYN_DMA_AXI_BLEN128 = 0x00000040,
	SYN_DMA_AXI_BLEN64 = 0x00000020,
	SYN_DMA_AXI_BLEN32 = 0x00000010,
	SYN_DMA_AXI_BLEN16 = 0x00000008,
	SYN_DMA_AXI_BLEN8 = 0x00000004,
	SYN_DMA_AXI_BLEN4 = 0x00000002,
	SYN_DMA_UNDEFINED = 0x00000001,
};

/*
 * SYN_DMA_MISSED_FRAME_AND_BUFF_OVERFLOW_COUNTER = 0x1020
 */
enum syn_dma_missed_and_buffer_overflow_cnt_reg {
	SYN_DMA_FIFO_OVERFLOW_SHIFT = 17,
	SYN_DMA_FIFO_OVERFLOW_MASK = 0x7FF,
	SYN_DMA_RX_MISSED_MASK = 0xFFFF,
};

/*
 * Values to initialize DMA registers
 */
enum syn_dma_init_values {
	SYN_DMA_INT_ERROR_MASK = SYN_DMA_INT_BUS_ERROR,		/* Error */
	SYN_DMA_INT_RX_ABN_MASK = SYN_DMA_INT_RX_NO_BUFFER,	/* RX abnormal intr */
	SYN_DMA_INT_RX_NORM_MASK = SYN_DMA_INT_RX_COMPLETED,	/* RXnormal intr */
	SYN_DMA_INT_RX_STOPPED_MASK = SYN_DMA_INT_RX_STOPPED,	/* RXstopped */
	SYN_DMA_INT_TX_ABN_MASK = SYN_DMA_INT_TX_UNDERFLOW,	/* TX abnormal intr */
	SYN_DMA_INT_TX_NORM_MASK = SYN_DMA_INT_TX_COMPLETED,	/* TX normal intr */
	SYN_DMA_INT_TX_STOPPED_MASK = SYN_DMA_INT_TX_STOPPED,	/* TX stopped */

	SYN_DMA_BUS_MODE_INIT = SYN_DMA_FIXED_BURST_ENABLE | SYN_DMA_BURST_LENGTH8
		| SYN_DMA_DESCRIPTOR_SKIP2 | SYN_DMA_RESET_OFF,

	SYN_DMA_BUS_MODE_VAL_CLSIZE_64B = SYN_DMA_BURST_LENGTH32
		| SYN_DMA_BURST_LENGTHX8 | SYN_DMA_DESCRIPTOR_SKIP4
		| SYN_DMA_DESCRIPTOR8_WORDS | SYN_DMA_ARBIT_PR | SYN_DMA_ADDRESS_ALIGNED_BEATS,

	SYN_DMA_OMR = SYN_DMA_TX_STORE_AND_FORWARD | SYN_DMA_RX_STORE_AND_FORWARD
		| SYN_DMA_RX_THRESH_CTRL128 | SYN_DMA_TX_SECOND_FRAME,

	SYN_DMA_INT_EN = SYN_DMA_INT_NORMAL
		| SYN_DMA_INT_ABNORMAL | SYN_DMA_INT_BUS_ERROR
		| SYN_DMA_INT_RX_NO_BUFFER | SYN_DMA_INT_RX_COMPLETED
		| SYN_DMA_INT_RX_STOPPED | SYN_DMA_INT_TX_UNDERFLOW
		| SYN_DMA_INT_TX_COMPLETED | SYN_DMA_INT_TX_STOPPED,

	SYN_DMA_RX_INT_EN = SYN_DMA_INT_RX_NO_BUFFER | SYN_DMA_INT_RX_COMPLETED
		| SYN_DMA_INT_RX_STOPPED,

	SYN_DMA_TX_INT_EN = SYN_DMA_INT_TX_UNDERFLOW
		| SYN_DMA_INT_TX_COMPLETED | SYN_DMA_INT_TX_STOPPED,

	SYN_DMA_INT_DISABLE = 0,

	SYN_DMA_AXI_BUS_MODE_VAL = SYN_DMA_AXI_BLEN16 | SYN_DMA_RD_OSR_NUM_REQS8
		| SYN_DMA_WR_OSR_NUM_REQS8,

	SYN_DMA_BUS_MODE_VAL_CLSIZE_32B = SYN_DMA_BURST_LENGTH32
		| SYN_DMA_BURST_LENGTHX8 | SYN_DMA_DESCRIPTOR_SKIP0
		| SYN_DMA_DESCRIPTOR8_WORDS | SYN_DMA_ARBIT_PR | SYN_DMA_ADDRESS_ALIGNED_BEATS,

};

/*
 * syn_dma_bus_mode_init_clsize_64byte()
 *	Function to program DMA bus mode register to access cached descriptors.
 */
static inline void syn_dma_bus_mode_init_clsize_64byte(void __iomem *mac_base)
{
	hal_write_reg(mac_base, SYN_DMA_BUS_MODE, SYN_DMA_BUS_MODE_VAL_CLSIZE_64B);
}

/*
 * syn_dma_bus_mode_init_clsize_32byte()
 *	Function to program DMA bus mode register to access uncached descriptors..
 */
static inline void syn_dma_bus_mode_init_clsize_32byte(void __iomem *mac_base)
{
	hal_write_reg(mac_base, SYN_DMA_BUS_MODE, SYN_DMA_BUS_MODE_VAL_CLSIZE_32B);
}

/*
 * syn_dma_axi_bus_mode_init()
 *	Function to program DMA AXI bus mode register.
 */
static inline void syn_dma_axi_bus_mode_init(void __iomem *mac_base)
{
	hal_write_reg(mac_base, SYN_DMA_AXI_BUS_MODE, SYN_DMA_AXI_BUS_MODE_VAL);
}

/*
 * syn_dma_operation_mode_init()
 *	Function to program DMA Operation Mode register.
 */
static inline void syn_dma_operation_mode_init(void __iomem *mac_base)
{
	hal_write_reg(mac_base, SYN_DMA_OPERATION_MODE, SYN_DMA_OMR);
}

/*
 * syn_dma_init()
 *	Initialize settings for GMAC DMA and AXI bus.
 */
static inline void syn_dma_init(void __iomem *mac_base)
{
	syn_dma_bus_mode_init_clsize_64byte(mac_base);
	syn_dma_axi_bus_mode_init(mac_base);
	syn_dma_operation_mode_init(mac_base);
}

/*
 * syn_rx_tcpip_chksum_drop_enable()
 *	Instruct the DMA to drop the packets that fail TCP/IP checksum.
 *
 * This is to instruct the receive DMA engine to drop the recevied
 * packet if they fails the tcp/ip checksum in hardware. Valid only when
 * full checksum offloading is enabled(type-2).
 */
static inline void syn_rx_tcpip_chksum_drop_enable(void __iomem *mac_base)
{
	uint32_t data;

	data = ~(SYN_DMA_DISABLE_DROP_TCP_CS) & hal_read_reg(mac_base, SYN_DMA_OPERATION_MODE);
	hal_write_reg(mac_base, SYN_DMA_OPERATION_MODE, data);
}

/*
 * syn_init_rx_desc_base()
 *	Programs the Rx Dma base address with the starting address of the descriptor ring or chain.
 */
static inline void syn_init_rx_desc_base(void __iomem *mac_base, uint32_t desc_dma)
{
	hal_write_reg(mac_base, SYN_DMA_RX_DESCRIPTOR_LIST_ADDRESS, desc_dma);
}

/*
 * syn_init_tx_desc_base()
 *	Programs the Tx Dma base address with the starting address of the descriptor ring or chain.
 */
static inline void syn_init_tx_desc_base(void __iomem *mac_base, uint32_t desc_dma)
{
	hal_write_reg(mac_base, SYN_DMA_TX_DESCRIPTOR_LIST_ADDRESS, desc_dma);
}

/*
 * syn_enable_dma_tx()
 *	Enable Rx GMAC operation
 */
static inline void syn_enable_dma_tx(void __iomem *mac_base)
{
	uint32_t data;

	data = hal_read_reg(mac_base, SYN_DMA_OPERATION_MODE);
	data |= SYN_DMA_TX_START;
	hal_write_reg(mac_base, SYN_DMA_OPERATION_MODE, data);
}

/*
 * syn_disable_dma_tx()
 *	Disable Rx GMAC operation
 */
static inline void syn_disable_dma_tx(void __iomem *mac_base)
{
	uint32_t data;

	data = hal_read_reg(mac_base, SYN_DMA_OPERATION_MODE);
	data &= ~SYN_DMA_TX_START;
	hal_write_reg(mac_base, SYN_DMA_OPERATION_MODE, data);
}

/*
 * syn_enable_dma_rx()
 *	Enable Rx GMAC operation
 */
static inline void syn_enable_dma_rx(void __iomem *mac_base)
{
	uint32_t data;

	data = hal_read_reg(mac_base, SYN_DMA_OPERATION_MODE);
	data |= SYN_DMA_RX_START;
	hal_write_reg(mac_base, SYN_DMA_OPERATION_MODE, data);
}

/*
 * syn_disable_dma_rx()
 *	Disable Rx GMAC operation
 */
static inline void syn_disable_dma_rx(void __iomem *mac_base)
{
	uint32_t data;

	data = hal_read_reg(mac_base, SYN_DMA_OPERATION_MODE);
	data &= ~SYN_DMA_RX_START;
	hal_write_reg(mac_base, SYN_DMA_OPERATION_MODE, data);
}

/*
 * syn_enable_tx_dma_interrupt()
 *	Enables Rx DMA interrupts.
 */
static inline void syn_enable_tx_dma_interrupt(void __iomem *mac_base)
{
	uint32_t data;

	data = SYN_DMA_INT_TX_COMPLETED | hal_read_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE);
	hal_write_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE, data);
}

/*
 * syn_enable_rx_dma_interrupt()
 *	Enables Rx DMA interrupts.
 */
static inline void syn_enable_rx_dma_interrupt(void __iomem *mac_base)
{
	uint32_t data;

	data = SYN_DMA_INT_RX_COMPLETED | hal_read_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE);
	hal_write_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE, data);
}

/*
 * syn_enable_dma_interrupt()
 *	Enables all DMA interrupts.
 */
static inline void syn_enable_dma_interrupt(void __iomem *mac_base)
{
	hal_write_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE, (SYN_DMA_INT_NORMAL | SYN_DMA_INT_RX_COMPLETED | SYN_DMA_INT_TX_COMPLETED));
}

/*
 * syn_disable_rx_dma_interrupt()
 *	Disables Rx DMA interrupts.
 */
static inline void syn_disable_rx_dma_interrupt(void __iomem *mac_base)
{
	uint32_t data;

	data = (~SYN_DMA_INT_RX_COMPLETED) & hal_read_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE);
	hal_write_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE, data);
}

/*
 * syn_disable_tx_dma_interrupt()
 *	Disables Tx DMA interrupts.
 */
static inline void syn_disable_tx_dma_interrupt(void __iomem *mac_base)
{
	uint32_t data;

	data = (~SYN_DMA_INT_TX_COMPLETED) & hal_read_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE);
	hal_write_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE, data);
}

/*
 * syn_disable_dma_interrupt()
 *	Disables all DMA interrupts.
 */
static inline void syn_disable_dma_interrupt(void __iomem *mac_base)
{
	hal_write_relaxed_reg(mac_base, SYN_DMA_INT_ENABLE, SYN_DMA_INT_DISABLE);
}

/*
 * syn_resume_dma_tx()
 *	Resumes the DMA Transmission.
 */
static inline void syn_resume_dma_tx(void __iomem *mac_base)
{
	hal_write_relaxed_reg(mac_base, SYN_DMA_TX_POLL_DEMAND, 0);
}

/*
 * syn_resume_dma_rx()
 *	Resumes the DMA Receive.
 */
static inline void syn_resume_dma_rx(void __iomem *mac_base)
{
	hal_write_relaxed_reg(mac_base, SYN_DMA_RX_POLL_DEMAND, 0);
}

/*
 * syn_clear_tx_dma_status()
 *	Clear all pending Tx dma interrupts.
 */
static inline void syn_clear_tx_dma_status(void __iomem *mac_base)
{
	hal_write_relaxed_reg(mac_base, SYN_DMA_STATUS, (SYN_DMA_INT_NORMAL | SYN_DMA_INT_TX_COMPLETED));
}

/*
 * syn_clear_rx_dma_status()
 *	Clear all pending Rx dma interrupts.
 */
static inline void syn_clear_rx_dma_status(void __iomem *mac_base)
{
	hal_write_relaxed_reg(mac_base, SYN_DMA_STATUS, (SYN_DMA_INT_NORMAL | SYN_DMA_INT_RX_COMPLETED));
}

/*
 * syn_clear_dma_status()
 *	Clear all the pending dma interrupts.
 */
static inline void syn_clear_dma_status(void __iomem *mac_base)
{
	uint32_t data;

	data = hal_read_relaxed_reg(mac_base, SYN_DMA_STATUS);
	hal_write_relaxed_reg(mac_base, SYN_DMA_STATUS, data);
}

/*
 * syn_get_rx_missed()
 *	Get Rx missed errors
 */
static inline uint32_t syn_get_rx_missed(void __iomem *mac_base)
{
	uint32_t missed_frame_buff_overflow;

	missed_frame_buff_overflow = hal_read_reg(mac_base, SYN_DMA_MISSED_FRAME_AND_BUFF_OVERFLOW_COUNTER);
	return missed_frame_buff_overflow & SYN_DMA_RX_MISSED_MASK;
}

/*
 * syn_get_fifo_overflows()
 *	Get FIFO overflows
 */
static inline uint32_t syn_get_fifo_overflows(void __iomem *mac_base)
{
	uint32_t missed_frame_buff_overflow;

	missed_frame_buff_overflow = hal_read_reg(mac_base, SYN_DMA_MISSED_FRAME_AND_BUFF_OVERFLOW_COUNTER);
	return (missed_frame_buff_overflow >> SYN_DMA_FIFO_OVERFLOW_SHIFT) & SYN_DMA_FIFO_OVERFLOW_MASK;
}

#endif	// __SYN_DMA_REG_H__
