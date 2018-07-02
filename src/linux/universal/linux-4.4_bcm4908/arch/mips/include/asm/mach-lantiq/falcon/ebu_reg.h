/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _ebu_reg_h
#define _ebu_reg_h

/** \addtogroup EBU_REGISTER
   @{
*/
/* access macros */
#define ebu_r32(reg) reg_r32(&ebu->reg)
#define ebu_w32(val, reg) reg_w32(val, &ebu->reg)
#define ebu_w32_mask(clear, set, reg) reg_w32_mask(clear, set, &ebu->reg)
#define ebu_r32_table(reg, idx) reg_r32_table(ebu->reg, idx)
#define ebu_w32_table(val, reg, idx) reg_w32_table(val, ebu->reg, idx)
#define ebu_w32_table_mask(clear, set, reg, idx) reg_w32_table_mask(clear, set, ebu->reg, idx)
#define ebu_adr_table(reg, idx) adr_table(ebu->reg, idx)


/** EBU register structure */
struct gpon_reg_ebu
{
   /** Reserved */
   unsigned int res_0[2]; /* 0x00000000 */
   /** Module ID Register
       Module type and version identifier */
   unsigned int modid; /* 0x00000008 */
   /** Module Control Register
       This register contains general configuration information observed for all CS regions or dealing with EBU functionality that is not directly related to external memory access. */
   unsigned int modcon; /* 0x0000000C */
   /** Bus Read Configuration Register0
       Note: The actual length of field enable depends on the number of bus ports connected to the EBU. For the GPON it is a single port (the bridge to the Asynchronous Xbar) so only bit 0 is implemented with all other bits tied to '0'. */
   unsigned int busrcon0; /* 0x00000010 */
   /** Bus Read Parameters Register0 */
   unsigned int busrp0; /* 0x00000014 */
   /** Bus Write Configuration Register0
       Note: The actual length of field enable depends on the number of bus ports connected to the EBU. For the GPON it is a single port (the bridge to the Asynchronous Xbar) so only bit 0 is implemented with all other bits tied to '0'. */
   unsigned int buswcon0; /* 0x00000018 */
   /** Bus Write Parameters Register0 */
   unsigned int buswp0; /* 0x0000001C */
   /** Bus Read Configuration Register1
       Note: The actual length of field enable depends on the number of bus ports connected to the EBU. For the GPON it is a single port (the bridge to the Asynchronous Xbar) so only bit 0 is implemented with all other bits tied to '0'. */
   unsigned int busrcon1; /* 0x00000020 */
   /** Bus Read Parameters Register1 */
   unsigned int busrp1; /* 0x00000024 */
   /** Bus Write Configuration Register1
       Note: The actual length of field enable depends on the number of bus ports connected to the EBU. For the GPON it is a single port (the bridge to the Asynchronous Xbar) so only bit 0 is implemented with all other bits tied to '0'. */
   unsigned int buswcon1; /* 0x00000028 */
   /** Bus Write Parameters Register1 */
   unsigned int buswp1; /* 0x0000002C */
   /** Reserved */
   unsigned int res_1[8]; /* 0x00000030 */
   /** Bus Protocol Configuration Extension Register 0 */
   unsigned int busconext0; /* 0x00000050 */
   /** Bus Protocol Configuration Extension Register 1 */
   unsigned int busconext1; /* 0x00000054 */
   /** Reserved */
   unsigned int res_2[10]; /* 0x00000058 */
   /** Serial Flash Configuration Register
       The content of this register configures the EBU's Serial Flash protocol engine. */
   unsigned int sfcon; /* 0x00000080 */
   /** Serial Flash Timing Register
       This register defines the signal timing for the Serial Flash Access. See Section 3.18.3 on page 112 for details. */
   unsigned int sftime; /* 0x00000084 */
   /** Serial Flash Status Register
       This register holds status information on the Serial Flash device(s) attached and the EBU's Serial Flash protocol engine. */
   unsigned int sfstat; /* 0x00000088 */
   /** Serial Flash Command Register
       When writing to this register's opcode field, a command is started in the EBU's Serial Flash controller. */
   unsigned int sfcmd; /* 0x0000008C */
   /** Serial Flash Address Register
       This register holds the address to be sent (if any) with accesses to/from a Serial Flash started by writing to EBU_SFCMD (Indirect Access Mode, see Section 3.18.2.4.1 on page 103). */
   unsigned int sfaddr; /* 0x00000090 */
   /** Serial Flash Data Register
       This register holds the data being transferred (if any) with accesses to/from a Serial Flash started by writing to EBU_SFCMD (Indirect Access Mode, see Section 4.18.2.4.1 on page 116). */
   unsigned int sfdata; /* 0x00000094 */
   /** Serial Flash I/O Control Register
       This register provides additional configuration for controlling the IO pads of the Serial Flash interface. */
   unsigned int sfio; /* 0x00000098 */
   /** Reserved */
   unsigned int res_3[25]; /* 0x0000009C */
};


/* Fields of "Module ID Register" */
/** Feature Select
    This field indicates the types of external devices/protocols supported by the GPON version of the EBU. */
#define MODID_FSEL_MASK 0xE0000000
/** field offset */
#define MODID_FSEL_OFFSET 29
/** Support for SRAM, NAND/NOR/OneNand Flash and Cellular RAM is implemented. */
#define MODID_FSEL_SRAM_FLASH_CRAM 0x00000000
/** Support for SRAM, NAND/NOR/OneNand Flash, Cellular RAM and SDR SDRAM is implemented. */
#define MODID_FSEL_SRAM_FLASH_CRAM_SDR 0x20000000
/** Support for SRAM, NAND/NOR/OneNand Flash, Cellular RAM and SDR/DDR SDRAM is implemented. */
#define MODID_FSEL_SRAM_FLASH_CRAM_DDR 0x40000000
/** Support for SRAM, NAND/NOR/OneNand Flash, Cellular RAM, SDR/DDR SDRAM 0nd LPDDR-Flash is implemented. */
#define MODID_FSEL_SRAM_FLASH_CRAM_DDR_LPNVM 0x60000000
/** Serial Flash Support
    Indicates whether or not the support of Serial Flash devices is available. */
#define MODID_SF 0x10000000
/* Not Available
#define MODID_SF_NAV 0x00000000 */
/** Available */
#define MODID_SF_AV 0x10000000
/** AAD-mux Support
    Indicates whether or not the GPON EBU supports AAD-mux protocol for Burst Flash and Cellular RAM. */
#define MODID_AAD 0x08000000
/* Not Available
#define MODID_AAD_NAV 0x00000000 */
/** Available */
#define MODID_AAD_AV 0x08000000
/** Indicates whether or not the GPON EBU implements a DLL which is e.g. used for 50% duty cycle external clock generation. Note that a DLL is always implemented if DDR-SDRAM support is selected. */
#define MODID_DLL 0x04000000
/* Not Available
#define MODID_DLL_NAV 0x00000000 */
/** Available */
#define MODID_DLL_AV 0x04000000
/** Pad Multiplexing Scheme */
#define MODID_PMS_MASK 0x03000000
/** field offset */
#define MODID_PMS_OFFSET 24
/** The EBU comprises of dedicated address pins A[EXTAW-1=:16]. */
#define MODID_PMS_PMS_CLASSIC 0x00000000
/** Revision
    Revision Number */
#define MODID_REV_MASK 0x000F0000
/** field offset */
#define MODID_REV_OFFSET 16
/** Module ID
    This field contains the EBU's unique peripheral ID. */
#define MODID_ID_MASK 0x0000FF00
/** field offset */
#define MODID_ID_OFFSET 8
/** Version
    This field gives the EBU version number. */
#define MODID_VERSION_MASK 0x000000FF
/** field offset */
#define MODID_VERSION_OFFSET 0

/* Fields of "Module Control Register" */
/** Reserved */
#define MODCON_DLLUPDINT_MASK 0xC0000000
/** field offset */
#define MODCON_DLLUPDINT_OFFSET 30
/** Access Inhibit Acknowledge
    After suspension of all accesses to the External Bus has been requested by setting bit acc_inh, acc_inh_ack acknowledges the request and inidcates that access suspension is now in effect. The bit is cleared when acc_inh gets deasserted. */
#define MODCON_AIA 0x02000000
/* no access restriction are active in the EBU subsystem
#define MODCON_AIA_NO_INHIBIT 0x00000000 */
/** accesses are restricted to selected (configuration) system bus port(s) */
#define MODCON_AIA_INHIBIT 0x02000000
/** Access Inhibit request
    Setting this bit will suspend all non-CPU system bus ports and the EBU itself from accessing the External Bus. This feature is usually used when the CPU needs to reconfigure protocol parameters in the EBU in order to avoid external accesses with invalid settings. The EBU acknowledges that the access suspension is in effect by asserting acc_inh_ack. */
#define MODCON_AI 0x01000000
/* no access restriction are active in the EBU subsystem
#define MODCON_AI_NO_INHIBIT 0x00000000 */
/** accesses are restricted to selected (configuration) system bus port(s) */
#define MODCON_AI_INHIBIT 0x01000000
/** Lock Timeout */
#define MODCON_LTO_MASK 0x00FF0000
/** field offset */
#define MODCON_LTO_OFFSET 16
/** Reserved */
#define MODCON_DDREN 0x00008000
/** Pad Drive Control
    Intended to be used to control the EBU pad''s drive strength. Refer to the GPON chip specification to see which drive strnegth options are available and whether they are actually controlled by the EBU's register bit. The value stored in this register bit is directly connected to the corresponding output of the EBU module and takes no functional effect within the EBU itself. */
#define MODCON_PEXT 0x00004000
/* Normal drive
#define MODCON_PEXT_NORMAL 0x00000000 */
/** Strong drive */
#define MODCON_PEXT_STRONG 0x00004000
/** Pad Slew Falling Edge Control
    Intended to be used to trim the External Bus pad's falling edge slew rate. Refer to the GPON chip specification to see which slew rate options are available and whether they are actually controlled by the EBU's register bit. The value stored in this register bit is directly connected to the corresponding output of the EBU module and takes no functional effect within the EBU itself. */
#define MODCON_SLF 0x00002000
/* Slow slew rate
#define MODCON_SLF_SLOW 0x00000000 */
/** Fast slew rate */
#define MODCON_SLF_FAST 0x00002000
/** Pad Slew Rising Edge Control
    Intended to be used to trim the External Bus pad's rising edge slew rate. Refer to the GPON chip specification to see which slew rate options are available and whether they are actually controlled by the EBU's register bit. The value stored in this register bit is directly connected to the corresponding output of the EBU module and takes no functional effect within the EBU itself. */
#define MODCON_SLR 0x00001000
/* Slow slew rate
#define MODCON_SLR_SLOW 0x00000000 */
/** Fast slew rate */
#define MODCON_SLR_FAST 0x00001000
/** Write Buffering Mode
    This bit controls when the EBU starts a new write burst transaction from the Memport interface. */
#define MODCON_WBM 0x00000040
/* The EBU starts a write transaction on the External Bus as early as possible, expecting that the n beats of the write burst will be transferred within n or n+1 clock cycles over the EBU's Memport interface. Use this mode if the EBU is clocked at the same or a slower frequency than the system bus interconnect.
#define MODCON_WBM_START_WRITE_EARLY 0x00000000 */
/** The EBU start a write transaction only after all data of a write burst have been received over the EBU's Memport interface. Use this mode if the EBU is clocked at a higher frequency than the system bus interrconnect. */
#define MODCON_WBM_START_WRITE_LATE 0x00000040
/** Reserved */
#define MODCON_SDCLKEN 0x00000020
/** Standby Mode Enable
    When set allows the EBU subsystem to enter standby mode in response to a rising edge on input signal standby_req_i. See Section 3.9.3 for details. */
#define MODCON_STBYEN 0x00000010
/* Disable
#define MODCON_STBYEN_DIS 0x00000000 */
/** Enable */
#define MODCON_STBYEN_EN 0x00000010
/** Enable BFCLK1
    This field will enables or disables mirroring the clock that is output on BFCLKO_0 also on pad BFCLKO_1 to double the drive strength. See also Section 3.17.3) */
#define MODCON_BFCLK1EN 0x00000008
/* Disable
#define MODCON_BFCLK1EN_DIS 0x00000000 */
/** Enable */
#define MODCON_BFCLK1EN_EN 0x00000008
/** Ready/Busy Status Edge
    This is a read-only bit which shows a change of the logic level shown in the sts field since last read. It is reset by a read access. */
#define MODCON_STSEDGE 0x00000004
/** Ready/Busy Status
    This is a read-only bit which reflects the current logic level present on the RDY/BSY or STS input pin which is (optionally) fed-in from a General Purpose I/O pad which is not part of the EBU via the EBU's input pin signal gpio_nand_rdy_ */
#define MODCON_STS 0x00000002
/** External Bus Arbitration Mode
    This bit allows to disconnect the EBU from the External Bus. While EBU_MODCON.acc_inh_ack is 0, the value of arb_mode is forced to OWN_BUS. */
#define MODCON_AM 0x00000001
/* The EBU does not own the bus (multi-master)
#define MODCON_AM_SHAREDBUS 0x00000000 */
/** The EBU owns the external bus. */
#define MODCON_AM_OWNBUS 0x00000001

/* Fields of "Bus Read Configuration Register0" */
/** Device Type For Region
    After reset, the CS region is configured for a slow Asynchronous access protocol which is compatible with read access from an external multiplexed or demultiplexed 16-Bit Burst Flash in asynchronous mode. Reset: 0000B */
#define BUSRCON0_AGEN_MASK 0xF0000000
/** field offset */
#define BUSRCON0_AGEN_OFFSET 28
/** Muxed Asynchronous Type External Memory */
#define BUSRCON0_AGEN_MUXED_ASYNC_TYPE_EXT_MEM 0x00000000
/** Muxed Burst Type External Memory */
#define BUSRCON0_AGEN_MUXED_BURST_TYPE_EXT_MEM 0x10000000
/** NAND Flash (page optimised) */
#define BUSRCON0_AGEN_NAND_FLASH 0x20000000
/** Muxed Cellular RAM External Memory */
#define BUSRCON0_AGEN_MUXED_CELLULAR_RAM_EXT_MEM 0x30000000
/** Demuxed Asynchronous Type External Memory */
#define BUSRCON0_AGEN_DEMUXED_ASYNC_TYPE_EXT_MEM 0x40000000
/** Demuxed Burst Type External Memory */
#define BUSRCON0_AGEN_DEMUXED_BURST_TYPE_EXT_MEM 0x50000000
/** Demuxed Page Mode External Memory */
#define BUSRCON0_AGEN_DEMUXED_PAGE_MODE_EXT_MEM 0x60000000
/** Demuxed Cellular RAM External Memory */
#define BUSRCON0_AGEN_DEMUXED_CELLULAR_RAM_EXT_MEM 0x70000000
/** Serial Flash */
#define BUSRCON0_AGEN_SERIAL_FLASH 0xF0000000
/** Device Addressing Mode
    t.b.d. */
#define BUSRCON0_PORTW_MASK 0x0C000000
/** field offset */
#define BUSRCON0_PORTW_OFFSET 26
/** 8-bit multiplexed */
#define BUSRCON0_PORTW_8_BIT_MUX 0x00000000
/** 16-bit multiplexed */
#define BUSRCON0_PORTW_16_BIT_MUX 0x04000000
/** Twin, 16-bit multiplexed */
#define BUSRCON0_PORTW_TWIN_16_BIT_MUX 0x08000000
/** 32-bit multiplexed */
#define BUSRCON0_PORTW_32_BIT_MUX 0x0C000000
/** External Wait Control
    Function of the WAIT input. This is specific to the device type (i.e. the agen field). */
#define BUSRCON0_WAIT_MASK 0x03000000
/** field offset */
#define BUSRCON0_WAIT_OFFSET 24
/** WAIT is ignored (default after reset). */
#define BUSRCON0_WAIT_OFF 0x00000000
/** Synchronous Burst Devices: WAIT signal is provided one cycle ahead of the data cycle it applies to. */
#define BUSRCON0_WAIT_EARLY_WAIT 0x01000000
/** Asynchronous Devices: WAIT input passes through a two-stage synchronizer before being evaluated. */
#define BUSRCON0_WAIT_TWO_STAGE_SYNC 0x01000000
/** Synchronous Burst Devices: WAIT signal is provided in the same data cycle it applies to. */
#define BUSRCON0_WAIT_WAIT_WITH_DATA 0x02000000
/** Asynchronous Devices: WAIT input passes through a single-stage synchronizer before being evaluated. */
#define BUSRCON0_WAIT_SINGLE_STAGE_SYNC 0x02000000
/** Synchronous Burst Devices: Abort and retry access if WAIT asserted */
#define BUSRCON0_WAIT_ABORT_AND_RETRY 0x03000000
/** Disable Burst Address Wrapping */
#define BUSRCON0_DBA 0x00800000
/** Reversed polarity at wait */
#define BUSRCON0_WAITINV 0x00400000
/* Low active.
#define BUSRCON0_WAITINV_ACTLOW 0x00000000 */
/** High active */
#define BUSRCON0_WAITINV_ACTHI 0x00400000
/** Early ADV Enable for Synchronous Bursts */
#define BUSRCON0_EBSE 0x00200000
/* Low active.
#define BUSRCON0_EBSE_DELAYED 0x00000000 */
/** High active */
#define BUSRCON0_EBSE_NOT_DELAYED 0x00200000
/** Early Control Signals for Synchronous Bursts */
#define BUSRCON0_ECSE 0x00100000
/* Low active.
#define BUSRCON0_ECSE_DELAYED 0x00000000 */
/** High active */
#define BUSRCON0_ECSE_NOT_DELAYED 0x00100000
/** Synchronous Burst Buffer Mode Select */
#define BUSRCON0_FBBMSEL 0x00080000
/* FIXED_LENGTH
#define BUSRCON0_FBBMSEL_FIXED_LENGTH 0x00000000 */
/** CONTINUOUS */
#define BUSRCON0_FBBMSEL_CONTINUOUS 0x00080000
/** Burst Length for Synchronous Burst */
#define BUSRCON0_FETBLEN_MASK 0x00070000
/** field offset */
#define BUSRCON0_FETBLEN_OFFSET 16
/** Up to 1 data cycle (default after reset). */
#define BUSRCON0_FETBLEN_SINGLE 0x00000000
/** Up to 2 data cycles. */
#define BUSRCON0_FETBLEN_BURST2 0x00010000
/** Up to 4 data cycles. */
#define BUSRCON0_FETBLEN_BURST4 0x00020000
/** Up to 8 data cycles. */
#define BUSRCON0_FETBLEN_BURST8 0x00030000
/** Up to 16 data cycles. */
#define BUSRCON0_FETBLEN_BURST16 0x00040000
/** Reserved
    This field allows to configure how the EBU generates the CLE and ALE signals for a NAND Flash device. The following options are available */
#define BUSRCON0_NANDAMAP_MASK 0x0000C000
/** field offset */
#define BUSRCON0_NANDAMAP_OFFSET 14
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSRCON0_NANDAMAP_NAND_A17_16 0x00000000
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSRCON0_NANDAMAP_NAND_WAIT_ADV 0x00004000
/** CLE is taken from AMemport[18] and ALE from AMemport[17] and are output on pins AD[9:8] and A[9:8] on the External Bus. This mode may only be used with a 8-Bit NAND-Flash device. */
#define BUSRCON0_NANDAMAP_NAND_AD9_8 0x00008000
/** Reserved for future use. Do not use or unpredictable results may occur. */
#define BUSRCON0_NANDAMAP_NAND_RFU 0x0000C000
/** AAD-mux Protocol
    If this bit is set and the device is configured for a multiplexed access protocol in agen then the device is accessed in read mode using the AAD-mux protocol. If a non-multiplexed device type is selected in agen, field aadmux is ignored. */
#define BUSRCON0_AADMUX 0x00002000
/* Muxed device is write accessed in AD-mux mode.
#define BUSRCON0_AADMUX_AD_MUX 0x00000000 */
/** Muxed device is write accessed in AAD-mux mode. */
#define BUSRCON0_AADMUX_AAD_MUX 0x00002000
/** Asynchronous Address Phase */
#define BUSRCON0_AAP 0x00001000
/* Clock is enabled at beginning of access.
#define BUSRCON0_AAP_EARLY 0x00000000 */
/** Clock is enabled after address phase. */
#define BUSRCON0_AAP_LATE 0x00001000
/** Burst Flash Read Single Stage Synchronisation */
#define BUSRCON0_BFSSS 0x00000800
/* Two stages of synchronisation used.
#define BUSRCON0_BFSSS_TWO_STAGE 0x00000000 */
/** Single stage of synchronisation used. */
#define BUSRCON0_BFSSS_SINGLE_STAGE 0x00000800
/** Burst Flash Clock Feedback Enable */
#define BUSRCON0_FDBKEN 0x00000400
/* Disable
#define BUSRCON0_FDBKEN_DIS 0x00000000 */
/** Enable */
#define BUSRCON0_FDBKEN_EN 0x00000400
/** Auxiliary Chip Select Enable
    Not supported in GPON-EBU, field must be set to 0. */
#define BUSRCON0_CSA 0x00000200
/* Disable
#define BUSRCON0_CSA_DIS 0x00000000 */
/** Enable */
#define BUSRCON0_CSA_EN 0x00000200
/** Flash Non-Array Access Enable
    Set to logic one to enable workaround when region is accessed with internal address bit 28 set. See Section 3.17.13 on page 90 for details. */
#define BUSRCON0_NAA 0x00000100
/* Disable
#define BUSRCON0_NAA_DIS 0x00000000 */
/** Enable */
#define BUSRCON0_NAA_EN 0x00000100
/** Module Enable */
#define BUSRCON0_ENABLE 0x00000001
/* Disable
#define BUSRCON0_ENABLE_DIS 0x00000000 */
/** Enable */
#define BUSRCON0_ENABLE_EN 0x00000001

/* Fields of "Bus Read Parameters Register0" */
/** Address Cycles
    Number of cycles for address phase. */
#define BUSRP0_ADDRC_MASK 0xF0000000
/** field offset */
#define BUSRP0_ADDRC_OFFSET 28
/** Address Hold Cycles For Multiplexed Address
    Number of address hold cycles during multiplexed accesses. */
#define BUSRP0_ADHOLC_MASK 0x0F000000
/** field offset */
#define BUSRP0_ADHOLC_OFFSET 24
/** Programmed Command Delay Cycles
    Number of delay cycles during command delay phase. */
#define BUSRP0_CMDDELAY_MASK 0x00F00000
/** field offset */
#define BUSRP0_CMDDELAY_OFFSET 20
/** Extended Data */
#define BUSRP0_EXTDATA_MASK 0x000C0000
/** field offset */
#define BUSRP0_EXTDATA_OFFSET 18
/** External device outputs data every BFCLK cycle */
#define BUSRP0_EXTDATA_ONE 0x00000000
/** External device outputs data every 2nd BFCLK cycles */
#define BUSRP0_EXTDATA_TWO 0x00040000
/** External device outputs data every 4th BFCLK cycles */
#define BUSRP0_EXTDATA_FOUR 0x00080000
/** External device outputs data every 8th BFCLK cycles */
#define BUSRP0_EXTDATA_EIGHT 0x000C0000
/** Frequency of external clock at pin BFCLKO */
#define BUSRP0_EXTCLOCK_MASK 0x00030000
/** field offset */
#define BUSRP0_EXTCLOCK_OFFSET 16
/** Equal to ebu_clk frequency. */
#define BUSRP0_EXTCLOCK_ONE_TO_ONE 0x00000000
/** 1/2 of ebu_clk frequency. */
#define BUSRP0_EXTCLOCK_ONE_TO_TWO 0x00010000
/** 1/3 of ebu_clk frequency. */
#define BUSRP0_EXTCLOCK_ONE_TO_THREE 0x00020000
/** 1/4 of ebu_clk frequency (default after reset). */
#define BUSRP0_EXTCLOCK_ONE_TO_FOUR 0x00030000
/** Data Hold Cycles For read Accesses
    Number of data hold cycles during read accesses. Applies to spinner support only where the address is guaranteed stable for datac clocks after RD high */
#define BUSRP0_DATAC_MASK 0x0000F000
/** field offset */
#define BUSRP0_DATAC_OFFSET 12
/** Programmed Wait States for read accesses
    Number of programmed wait states for read accesses. For synchronous accesses, this will always be adjusted so that the phase exits on a rising edge of the external clock. */
#define BUSRP0_WAITRDC_MASK 0x00000F80
/** field offset */
#define BUSRP0_WAITRDC_OFFSET 7
/** Recovery Cycles After read Accesses, same CS
    Number of idle cycles after read accesses when the next access is to the same chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSRCON. */
#define BUSRP0_RECOVC_MASK 0x00000070
/** field offset */
#define BUSRP0_RECOVC_OFFSET 4
/** Recovery Cycles After read Accesses, other CS
    Number of idle cycles after read accesses when the next access is to a different chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSRCON. */
#define BUSRP0_DTACS_MASK 0x0000000F
/** field offset */
#define BUSRP0_DTACS_OFFSET 0

/* Fields of "Bus Write Configuration Register0" */
/** Device Type For Region
    After reset, the CS region is configured for a slow Asynchronous access protocol which is compatible with read access from an external multiplexed or demultiplexed 16-Bit Burst Flash in asynchronous mode. Reset: 0000B */
#define BUSWCON0_AGEN_MASK 0xF0000000
/** field offset */
#define BUSWCON0_AGEN_OFFSET 28
/** Muxed Asynchronous Type External Memory */
#define BUSWCON0_AGEN_MUXED_ASYNC_TYPE_EXT_MEM 0x00000000
/** Muxed Burst Type External Memory */
#define BUSWCON0_AGEN_MUXED_BURST_TYPE_EXT_MEM 0x10000000
/** NAND Flash (page optimised) */
#define BUSWCON0_AGEN_NAND_FLASH 0x20000000
/** Muxed Cellular RAM External Memory */
#define BUSWCON0_AGEN_MUXED_CELLULAR_RAM_EXT_MEM 0x30000000
/** Demuxed Asynchronous Type External Memory */
#define BUSWCON0_AGEN_DEMUXED_ASYNC_TYPE_EXT_MEM 0x40000000
/** Demuxed Burst Type External Memory */
#define BUSWCON0_AGEN_DEMUXED_BURST_TYPE_EXT_MEM 0x50000000
/** Demuxed Page Mode External Memory */
#define BUSWCON0_AGEN_DEMUXED_PAGE_MODE_EXT_MEM 0x60000000
/** Demuxed Cellular RAM External Memory */
#define BUSWCON0_AGEN_DEMUXED_CELLULAR_RAM_EXT_MEM 0x70000000
/** Serial Flash */
#define BUSWCON0_AGEN_SERIAL_FLASH 0xF0000000
/** Device Addressing Mode
    t.b.d. */
#define BUSWCON0_PORTW_MASK 0x0C000000
/** field offset */
#define BUSWCON0_PORTW_OFFSET 26
/** External Wait Control
    Function of the WAIT input. This is specific to the device type (i.e. the agen field). */
#define BUSWCON0_WAIT_MASK 0x03000000
/** field offset */
#define BUSWCON0_WAIT_OFFSET 24
/** WAIT is ignored (default after reset). */
#define BUSWCON0_WAIT_OFF 0x00000000
/** Synchronous Burst Devices: WAIT signal is provided one cycle ahead of the data cycle it applies to. */
#define BUSWCON0_WAIT_EARLY_WAIT 0x01000000
/** Asynchronous Devices: WAIT input passes through a two-stage synchronizer before being evaluated. */
#define BUSWCON0_WAIT_TWO_STAGE_SYNC 0x01000000
/** Synchronous Burst Devices: WAIT signal is provided in the same data cycle it applies to. */
#define BUSWCON0_WAIT_WAIT_WITH_DATA 0x02000000
/** Asynchronous Devices: WAIT input passes through a single-stage synchronizer before being evaluated. */
#define BUSWCON0_WAIT_SINGLE_STAGE_SYNC 0x02000000
/** Synchronous Burst Devices: Abort and retry access if WAIT asserted */
#define BUSWCON0_WAIT_ABORT_AND_RETRY 0x03000000
/** Reserved */
#define BUSWCON0_LOCKCS 0x00800000
/** Reversed polarity at wait */
#define BUSWCON0_WAITINV 0x00400000
/* Low active.
#define BUSWCON0_WAITINV_ACTLOW 0x00000000 */
/** High active */
#define BUSWCON0_WAITINV_ACTHI 0x00400000
/** Early ADV Enable for Synchronous Bursts */
#define BUSWCON0_EBSE 0x00200000
/* Low active.
#define BUSWCON0_EBSE_DELAYED 0x00000000 */
/** High active */
#define BUSWCON0_EBSE_NOT_DELAYED 0x00200000
/** Early Control Signals for Synchronous Bursts */
#define BUSWCON0_ECSE 0x00100000
/* Low active.
#define BUSWCON0_ECSE_DELAYED 0x00000000 */
/** High active */
#define BUSWCON0_ECSE_NOT_DELAYED 0x00100000
/** Synchronous Burst Buffer Mode Select */
#define BUSWCON0_FBBMSEL 0x00080000
/* FIXED_LENGTH
#define BUSWCON0_FBBMSEL_FIXED_LENGTH 0x00000000 */
/** CONTINUOUS */
#define BUSWCON0_FBBMSEL_CONTINUOUS 0x00080000
/** Burst Length for Synchronous Burst */
#define BUSWCON0_FETBLEN_MASK 0x00070000
/** field offset */
#define BUSWCON0_FETBLEN_OFFSET 16
/** Up to 1 data cycle (default after reset). */
#define BUSWCON0_FETBLEN_SINGLE 0x00000000
/** Up to 2 data cycles. */
#define BUSWCON0_FETBLEN_BURST2 0x00010000
/** Up to 4 data cycles. */
#define BUSWCON0_FETBLEN_BURST4 0x00020000
/** Up to 8 data cycles. */
#define BUSWCON0_FETBLEN_BURST8 0x00030000
/** Up to 16 data cycles. */
#define BUSWCON0_FETBLEN_BURST16 0x00040000
/** Reserved
    This field allows to configure how the EBU generates the CLE and ALE signals for a NAND Flash device. The following options are available */
#define BUSWCON0_NANDAMAP_MASK 0x0000C000
/** field offset */
#define BUSWCON0_NANDAMAP_OFFSET 14
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSWCON0_NANDAMAP_NAND_A17_16 0x00000000
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSWCON0_NANDAMAP_NAND_WAIT_ADV 0x00004000
/** CLE is taken from AMemport[18] and ALE from AMemport[17] and are output on pins AD[9:8] and A[9:8] on the External Bus. This mode may only be used with a 8-Bit NAND-Flash device. */
#define BUSWCON0_NANDAMAP_NAND_AD9_8 0x00008000
/** Reserved for future use. Do not use or unpredictable results may occur. */
#define BUSWCON0_NANDAMAP_NAND_RFU 0x0000C000
/** AAD-mux Protocol
    If this bit is set and the device is configured for a multiplexed access protocol in agen then the device is accessed in read mode using the AAD-mux protocol. If a non-multiplexed device type is selected in agen, field aadmux is ignored. */
#define BUSWCON0_AADMUX 0x00002000
/* Muxed device is write accessed in AD-mux mode.
#define BUSWCON0_AADMUX_AD_MUX 0x00000000 */
/** Muxed device is write accessed in AAD-mux mode. */
#define BUSWCON0_AADMUX_AAD_MUX 0x00002000
/** Asynchronous Address Phase */
#define BUSWCON0_AAP 0x00001000
/* Clock is enabled at beginning of access.
#define BUSWCON0_AAP_EARLY 0x00000000 */
/** Clock is enabled after address phase. */
#define BUSWCON0_AAP_LATE 0x00001000
/** Auxiliary Chip Select Enable
    Not supported in GPON-EBU, field must be set to 0. */
#define BUSWCON0_CSA 0x00000200
/* Disable
#define BUSWCON0_CSA_DIS 0x00000000 */
/** Enable */
#define BUSWCON0_CSA_EN 0x00000200
/** Flash Non-Array Access Enable
    Set to logic one to enable workaround when region is accessed with internal address bit 28 set. See Section 3.17.13 on page 90 for details. */
#define BUSWCON0_NAA 0x00000100
/* Disable
#define BUSWCON0_NAA_DIS 0x00000000 */
/** Enable */
#define BUSWCON0_NAA_EN 0x00000100
/** Module Enable */
#define BUSWCON0_ENABLE 0x00000001
/* Disable
#define BUSWCON0_ENABLE_DIS 0x00000000 */
/** Enable */
#define BUSWCON0_ENABLE_EN 0x00000001

/* Fields of "Bus Write Parameters Register0" */
/** Address Cycles
    Number of cycles for address phase. */
#define BUSWP0_ADDRC_MASK 0xF0000000
/** field offset */
#define BUSWP0_ADDRC_OFFSET 28
/** Address Hold Cycles For Multiplexed Address
    Number of address hold cycles during multiplexed accesses. */
#define BUSWP0_ADHOLC_MASK 0x0F000000
/** field offset */
#define BUSWP0_ADHOLC_OFFSET 24
/** Programmed Command Delay Cycles
    Number of delay cycles during command delay phase. */
#define BUSWP0_CMDDELAY_MASK 0x00F00000
/** field offset */
#define BUSWP0_CMDDELAY_OFFSET 20
/** Extended Data */
#define BUSWP0_EXTDATA_MASK 0x000C0000
/** field offset */
#define BUSWP0_EXTDATA_OFFSET 18
/** External device outputs data every BFCLK cycle */
#define BUSWP0_EXTDATA_ONE 0x00000000
/** External device outputs data every 2nd BFCLK cycles */
#define BUSWP0_EXTDATA_TWO 0x00040000
/** External device outputs data every 4th BFCLK cycles */
#define BUSWP0_EXTDATA_FOUR 0x00080000
/** External device outputs data every 8th BFCLK cycles */
#define BUSWP0_EXTDATA_EIGHT 0x000C0000
/** Frequency of external clock at pin BFCLKO */
#define BUSWP0_EXTCLOCK_MASK 0x00030000
/** field offset */
#define BUSWP0_EXTCLOCK_OFFSET 16
/** Equal to ebu_clk frequency. */
#define BUSWP0_EXTCLOCK_ONE_TO_ONE 0x00000000
/** 1/2 of ebu_clk frequency. */
#define BUSWP0_EXTCLOCK_ONE_TO_TWO 0x00010000
/** 1/3 of ebu_clk frequency. */
#define BUSWP0_EXTCLOCK_ONE_TO_THREE 0x00020000
/** 1/4 of ebu_clk frequency (default after reset). */
#define BUSWP0_EXTCLOCK_ONE_TO_FOUR 0x00030000
/** Data Hold Cycles For write Accesses
    Number of data hold cycles during write accesses. */
#define BUSWP0_DATAC_MASK 0x0000F000
/** field offset */
#define BUSWP0_DATAC_OFFSET 12
/** Programmed Wait States For write Accesses
    Number of programmed wait states for write accesses. For synchronous accesses, this will always be adjusted so that the phase exits on a rising edge of the external clock. */
#define BUSWP0_WAITWDC_MASK 0x00000F80
/** field offset */
#define BUSWP0_WAITWDC_OFFSET 7
/** Recovery Cycles After write Accesses, same CS
    Number of idle cycles after write accesses when following access is to the same chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSWCON. */
#define BUSWP0_RECOVC_MASK 0x00000070
/** field offset */
#define BUSWP0_RECOVC_OFFSET 4
/** Recovery Cycles After write Accesses, other CS
    Number of idle cycles after write accesses when the following access is to a different chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSWCON. */
#define BUSWP0_DTACS_MASK 0x0000000F
/** field offset */
#define BUSWP0_DTACS_OFFSET 0

/* Fields of "Bus Read Configuration Register1" */
/** Device Type For Region
    After reset, the CS region is configured for a slow Asynchronous access protocol which is compatible with read access from an external multiplexed or demultiplexed 16-Bit Burst Flash in asynchronous mode. Reset: 0000B */
#define BUSRCON1_AGEN_MASK 0xF0000000
/** field offset */
#define BUSRCON1_AGEN_OFFSET 28
/** Muxed Asynchronous Type External Memory */
#define BUSRCON1_AGEN_MUXED_ASYNC_TYPE_EXT_MEM 0x00000000
/** Muxed Burst Type External Memory */
#define BUSRCON1_AGEN_MUXED_BURST_TYPE_EXT_MEM 0x10000000
/** NAND Flash (page optimised) */
#define BUSRCON1_AGEN_NAND_FLASH 0x20000000
/** Muxed Cellular RAM External Memory */
#define BUSRCON1_AGEN_MUXED_CELLULAR_RAM_EXT_MEM 0x30000000
/** Demuxed Asynchronous Type External Memory */
#define BUSRCON1_AGEN_DEMUXED_ASYNC_TYPE_EXT_MEM 0x40000000
/** Demuxed Burst Type External Memory */
#define BUSRCON1_AGEN_DEMUXED_BURST_TYPE_EXT_MEM 0x50000000
/** Demuxed Page Mode External Memory */
#define BUSRCON1_AGEN_DEMUXED_PAGE_MODE_EXT_MEM 0x60000000
/** Demuxed Cellular RAM External Memory */
#define BUSRCON1_AGEN_DEMUXED_CELLULAR_RAM_EXT_MEM 0x70000000
/** Serial Flash */
#define BUSRCON1_AGEN_SERIAL_FLASH 0xF0000000
/** Device Addressing Mode
    t.b.d. */
#define BUSRCON1_PORTW_MASK 0x0C000000
/** field offset */
#define BUSRCON1_PORTW_OFFSET 26
/** 8-bit multiplexed */
#define BUSRCON1_PORTW_8_BIT_MUX 0x00000000
/** 16-bit multiplexed */
#define BUSRCON1_PORTW_16_BIT_MUX 0x04000000
/** Twin, 16-bit multiplexed */
#define BUSRCON1_PORTW_TWIN_16_BIT_MUX 0x08000000
/** 32-bit multiplexed */
#define BUSRCON1_PORTW_32_BIT_MUX 0x0C000000
/** External Wait Control
    Function of the WAIT input. This is specific to the device type (i.e. the agen field). */
#define BUSRCON1_WAIT_MASK 0x03000000
/** field offset */
#define BUSRCON1_WAIT_OFFSET 24
/** WAIT is ignored (default after reset). */
#define BUSRCON1_WAIT_OFF 0x00000000
/** Synchronous Burst Devices: WAIT signal is provided one cycle ahead of the data cycle it applies to. */
#define BUSRCON1_WAIT_EARLY_WAIT 0x01000000
/** Asynchronous Devices: WAIT input passes through a two-stage synchronizer before being evaluated. */
#define BUSRCON1_WAIT_TWO_STAGE_SYNC 0x01000000
/** Synchronous Burst Devices: WAIT signal is provided in the same data cycle it applies to. */
#define BUSRCON1_WAIT_WAIT_WITH_DATA 0x02000000
/** Asynchronous Devices: WAIT input passes through a single-stage synchronizer before being evaluated. */
#define BUSRCON1_WAIT_SINGLE_STAGE_SYNC 0x02000000
/** Synchronous Burst Devices: Abort and retry access if WAIT asserted */
#define BUSRCON1_WAIT_ABORT_AND_RETRY 0x03000000
/** Disable Burst Address Wrapping */
#define BUSRCON1_DBA 0x00800000
/** Reversed polarity at wait */
#define BUSRCON1_WAITINV 0x00400000
/* Low active.
#define BUSRCON1_WAITINV_ACTLOW 0x00000000 */
/** High active */
#define BUSRCON1_WAITINV_ACTHI 0x00400000
/** Early ADV Enable for Synchronous Bursts */
#define BUSRCON1_EBSE 0x00200000
/* Low active.
#define BUSRCON1_EBSE_DELAYED 0x00000000 */
/** High active */
#define BUSRCON1_EBSE_NOT_DELAYED 0x00200000
/** Early Control Signals for Synchronous Bursts */
#define BUSRCON1_ECSE 0x00100000
/* Low active.
#define BUSRCON1_ECSE_DELAYED 0x00000000 */
/** High active */
#define BUSRCON1_ECSE_NOT_DELAYED 0x00100000
/** Synchronous Burst Buffer Mode Select */
#define BUSRCON1_FBBMSEL 0x00080000
/* FIXED_LENGTH
#define BUSRCON1_FBBMSEL_FIXED_LENGTH 0x00000000 */
/** CONTINUOUS */
#define BUSRCON1_FBBMSEL_CONTINUOUS 0x00080000
/** Burst Length for Synchronous Burst */
#define BUSRCON1_FETBLEN_MASK 0x00070000
/** field offset */
#define BUSRCON1_FETBLEN_OFFSET 16
/** Up to 1 data cycle (default after reset). */
#define BUSRCON1_FETBLEN_SINGLE 0x00000000
/** Up to 2 data cycles. */
#define BUSRCON1_FETBLEN_BURST2 0x00010000
/** Up to 4 data cycles. */
#define BUSRCON1_FETBLEN_BURST4 0x00020000
/** Up to 8 data cycles. */
#define BUSRCON1_FETBLEN_BURST8 0x00030000
/** Up to 16 data cycles. */
#define BUSRCON1_FETBLEN_BURST16 0x00040000
/** Reserved
    This field allows to configure how the EBU generates the CLE and ALE signals for a NAND Flash device. The following options are available */
#define BUSRCON1_NANDAMAP_MASK 0x0000C000
/** field offset */
#define BUSRCON1_NANDAMAP_OFFSET 14
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSRCON1_NANDAMAP_NAND_A17_16 0x00000000
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSRCON1_NANDAMAP_NAND_WAIT_ADV 0x00004000
/** CLE is taken from AMemport[18] and ALE from AMemport[17] and are output on pins AD[9:8] and A[9:8] on the External Bus. This mode may only be used with a 8-Bit NAND-Flash device. */
#define BUSRCON1_NANDAMAP_NAND_AD9_8 0x00008000
/** Reserved for future use. Do not use or unpredictable results may occur. */
#define BUSRCON1_NANDAMAP_NAND_RFU 0x0000C000
/** AAD-mux Protocol
    If this bit is set and the device is configured for a multiplexed access protocol in agen then the device is accessed in read mode using the AAD-mux protocol. If a non-multiplexed device type is selected in agen, field aadmux is ignored. */
#define BUSRCON1_AADMUX 0x00002000
/* Muxed device is write accessed in AD-mux mode.
#define BUSRCON1_AADMUX_AD_MUX 0x00000000 */
/** Muxed device is write accessed in AAD-mux mode. */
#define BUSRCON1_AADMUX_AAD_MUX 0x00002000
/** Asynchronous Address Phase */
#define BUSRCON1_AAP 0x00001000
/* Clock is enabled at beginning of access.
#define BUSRCON1_AAP_EARLY 0x00000000 */
/** Clock is enabled after address phase. */
#define BUSRCON1_AAP_LATE 0x00001000
/** Burst Flash Read Single Stage Synchronisation */
#define BUSRCON1_BFSSS 0x00000800
/* Two stages of synchronisation used.
#define BUSRCON1_BFSSS_TWO_STAGE 0x00000000 */
/** Single stage of synchronisation used. */
#define BUSRCON1_BFSSS_SINGLE_STAGE 0x00000800
/** Burst Flash Clock Feedback Enable */
#define BUSRCON1_FDBKEN 0x00000400
/* Disable
#define BUSRCON1_FDBKEN_DIS 0x00000000 */
/** Enable */
#define BUSRCON1_FDBKEN_EN 0x00000400
/** Auxiliary Chip Select Enable
    Not supported in GPON-EBU, field must be set to 0. */
#define BUSRCON1_CSA 0x00000200
/* Disable
#define BUSRCON1_CSA_DIS 0x00000000 */
/** Enable */
#define BUSRCON1_CSA_EN 0x00000200
/** Flash Non-Array Access Enable
    Set to logic one to enable workaround when region is accessed with internal address bit 28 set. See Section 3.17.13 on page 90 for details. */
#define BUSRCON1_NAA 0x00000100
/* Disable
#define BUSRCON1_NAA_DIS 0x00000000 */
/** Enable */
#define BUSRCON1_NAA_EN 0x00000100
/** Module Enable */
#define BUSRCON1_ENABLE 0x00000001
/* Disable
#define BUSRCON1_ENABLE_DIS 0x00000000 */
/** Enable */
#define BUSRCON1_ENABLE_EN 0x00000001

/* Fields of "Bus Read Parameters Register1" */
/** Address Cycles
    Number of cycles for address phase. */
#define BUSRP1_ADDRC_MASK 0xF0000000
/** field offset */
#define BUSRP1_ADDRC_OFFSET 28
/** Address Hold Cycles For Multiplexed Address
    Number of address hold cycles during multiplexed accesses. */
#define BUSRP1_ADHOLC_MASK 0x0F000000
/** field offset */
#define BUSRP1_ADHOLC_OFFSET 24
/** Programmed Command Delay Cycles
    Number of delay cycles during command delay phase. */
#define BUSRP1_CMDDELAY_MASK 0x00F00000
/** field offset */
#define BUSRP1_CMDDELAY_OFFSET 20
/** Extended Data */
#define BUSRP1_EXTDATA_MASK 0x000C0000
/** field offset */
#define BUSRP1_EXTDATA_OFFSET 18
/** External device outputs data every BFCLK cycle */
#define BUSRP1_EXTDATA_ONE 0x00000000
/** External device outputs data every 2nd BFCLK cycles */
#define BUSRP1_EXTDATA_TWO 0x00040000
/** External device outputs data every 4th BFCLK cycles */
#define BUSRP1_EXTDATA_FOUR 0x00080000
/** External device outputs data every 8th BFCLK cycles */
#define BUSRP1_EXTDATA_EIGHT 0x000C0000
/** Frequency of external clock at pin BFCLKO */
#define BUSRP1_EXTCLOCK_MASK 0x00030000
/** field offset */
#define BUSRP1_EXTCLOCK_OFFSET 16
/** Equal to ebu_clk frequency. */
#define BUSRP1_EXTCLOCK_ONE_TO_ONE 0x00000000
/** 1/2 of ebu_clk frequency. */
#define BUSRP1_EXTCLOCK_ONE_TO_TWO 0x00010000
/** 1/3 of ebu_clk frequency. */
#define BUSRP1_EXTCLOCK_ONE_TO_THREE 0x00020000
/** 1/4 of ebu_clk frequency (default after reset). */
#define BUSRP1_EXTCLOCK_ONE_TO_FOUR 0x00030000
/** Data Hold Cycles For read Accesses
    Number of data hold cycles during read accesses. Applies to spinner support only where the address is guaranteed stable for datac clocks after RD high */
#define BUSRP1_DATAC_MASK 0x0000F000
/** field offset */
#define BUSRP1_DATAC_OFFSET 12
/** Programmed Wait States for read accesses
    Number of programmed wait states for read accesses. For synchronous accesses, this will always be adjusted so that the phase exits on a rising edge of the external clock. */
#define BUSRP1_WAITRDC_MASK 0x00000F80
/** field offset */
#define BUSRP1_WAITRDC_OFFSET 7
/** Recovery Cycles After read Accesses, same CS
    Number of idle cycles after read accesses when the next access is to the same chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSRCON. */
#define BUSRP1_RECOVC_MASK 0x00000070
/** field offset */
#define BUSRP1_RECOVC_OFFSET 4
/** Recovery Cycles After read Accesses, other CS
    Number of idle cycles after read accesses when the next access is to a different chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSRCON. */
#define BUSRP1_DTACS_MASK 0x0000000F
/** field offset */
#define BUSRP1_DTACS_OFFSET 0

/* Fields of "Bus Write Configuration Register1" */
/** Device Type For Region
    After reset, the CS region is configured for a slow Asynchronous access protocol which is compatible with read access from an external multiplexed or demultiplexed 16-Bit Burst Flash in asynchronous mode. Reset: 0000B */
#define BUSWCON1_AGEN_MASK 0xF0000000
/** field offset */
#define BUSWCON1_AGEN_OFFSET 28
/** Muxed Asynchronous Type External Memory */
#define BUSWCON1_AGEN_MUXED_ASYNC_TYPE_EXT_MEM 0x00000000
/** Muxed Burst Type External Memory */
#define BUSWCON1_AGEN_MUXED_BURST_TYPE_EXT_MEM 0x10000000
/** NAND Flash (page optimised) */
#define BUSWCON1_AGEN_NAND_FLASH 0x20000000
/** Muxed Cellular RAM External Memory */
#define BUSWCON1_AGEN_MUXED_CELLULAR_RAM_EXT_MEM 0x30000000
/** Demuxed Asynchronous Type External Memory */
#define BUSWCON1_AGEN_DEMUXED_ASYNC_TYPE_EXT_MEM 0x40000000
/** Demuxed Burst Type External Memory */
#define BUSWCON1_AGEN_DEMUXED_BURST_TYPE_EXT_MEM 0x50000000
/** Demuxed Page Mode External Memory */
#define BUSWCON1_AGEN_DEMUXED_PAGE_MODE_EXT_MEM 0x60000000
/** Demuxed Cellular RAM External Memory */
#define BUSWCON1_AGEN_DEMUXED_CELLULAR_RAM_EXT_MEM 0x70000000
/** Serial Flash */
#define BUSWCON1_AGEN_SERIAL_FLASH 0xF0000000
/** Device Addressing Mode
    t.b.d. */
#define BUSWCON1_PORTW_MASK 0x0C000000
/** field offset */
#define BUSWCON1_PORTW_OFFSET 26
/** External Wait Control
    Function of the WAIT input. This is specific to the device type (i.e. the agen field). */
#define BUSWCON1_WAIT_MASK 0x03000000
/** field offset */
#define BUSWCON1_WAIT_OFFSET 24
/** WAIT is ignored (default after reset). */
#define BUSWCON1_WAIT_OFF 0x00000000
/** Synchronous Burst Devices: WAIT signal is provided one cycle ahead of the data cycle it applies to. */
#define BUSWCON1_WAIT_EARLY_WAIT 0x01000000
/** Asynchronous Devices: WAIT input passes through a two-stage synchronizer before being evaluated. */
#define BUSWCON1_WAIT_TWO_STAGE_SYNC 0x01000000
/** Synchronous Burst Devices: WAIT signal is provided in the same data cycle it applies to. */
#define BUSWCON1_WAIT_WAIT_WITH_DATA 0x02000000
/** Asynchronous Devices: WAIT input passes through a single-stage synchronizer before being evaluated. */
#define BUSWCON1_WAIT_SINGLE_STAGE_SYNC 0x02000000
/** Synchronous Burst Devices: Abort and retry access if WAIT asserted */
#define BUSWCON1_WAIT_ABORT_AND_RETRY 0x03000000
/** Reserved */
#define BUSWCON1_LOCKCS 0x00800000
/** Reversed polarity at wait */
#define BUSWCON1_WAITINV 0x00400000
/* Low active.
#define BUSWCON1_WAITINV_ACTLOW 0x00000000 */
/** High active */
#define BUSWCON1_WAITINV_ACTHI 0x00400000
/** Early ADV Enable for Synchronous Bursts */
#define BUSWCON1_EBSE 0x00200000
/* Low active.
#define BUSWCON1_EBSE_DELAYED 0x00000000 */
/** High active */
#define BUSWCON1_EBSE_NOT_DELAYED 0x00200000
/** Early Control Signals for Synchronous Bursts */
#define BUSWCON1_ECSE 0x00100000
/* Low active.
#define BUSWCON1_ECSE_DELAYED 0x00000000 */
/** High active */
#define BUSWCON1_ECSE_NOT_DELAYED 0x00100000
/** Synchronous Burst Buffer Mode Select */
#define BUSWCON1_FBBMSEL 0x00080000
/* FIXED_LENGTH
#define BUSWCON1_FBBMSEL_FIXED_LENGTH 0x00000000 */
/** CONTINUOUS */
#define BUSWCON1_FBBMSEL_CONTINUOUS 0x00080000
/** Burst Length for Synchronous Burst */
#define BUSWCON1_FETBLEN_MASK 0x00070000
/** field offset */
#define BUSWCON1_FETBLEN_OFFSET 16
/** Up to 1 data cycle (default after reset). */
#define BUSWCON1_FETBLEN_SINGLE 0x00000000
/** Up to 2 data cycles. */
#define BUSWCON1_FETBLEN_BURST2 0x00010000
/** Up to 4 data cycles. */
#define BUSWCON1_FETBLEN_BURST4 0x00020000
/** Up to 8 data cycles. */
#define BUSWCON1_FETBLEN_BURST8 0x00030000
/** Up to 16 data cycles. */
#define BUSWCON1_FETBLEN_BURST16 0x00040000
/** Reserved
    This field allows to configure how the EBU generates the CLE and ALE signals for a NAND Flash device. The following options are available */
#define BUSWCON1_NANDAMAP_MASK 0x0000C000
/** field offset */
#define BUSWCON1_NANDAMAP_OFFSET 14
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSWCON1_NANDAMAP_NAND_A17_16 0x00000000
/** is taken from AMemport[18] and ALE from AMemport[17] and are output on pins A[17:16] on the External Bus (default after reset). */
#define BUSWCON1_NANDAMAP_NAND_WAIT_ADV 0x00004000
/** CLE is taken from AMemport[18] and ALE from AMemport[17] and are output on pins AD[9:8] and A[9:8] on the External Bus. This mode may only be used with a 8-Bit NAND-Flash device. */
#define BUSWCON1_NANDAMAP_NAND_AD9_8 0x00008000
/** Reserved for future use. Do not use or unpredictable results may occur. */
#define BUSWCON1_NANDAMAP_NAND_RFU 0x0000C000
/** AAD-mux Protocol
    If this bit is set and the device is configured for a multiplexed access protocol in agen then the device is accessed in read mode using the AAD-mux protocol. If a non-multiplexed device type is selected in agen, field aadmux is ignored. */
#define BUSWCON1_AADMUX 0x00002000
/* Muxed device is write accessed in AD-mux mode.
#define BUSWCON1_AADMUX_AD_MUX 0x00000000 */
/** Muxed device is write accessed in AAD-mux mode. */
#define BUSWCON1_AADMUX_AAD_MUX 0x00002000
/** Asynchronous Address Phase */
#define BUSWCON1_AAP 0x00001000
/* Clock is enabled at beginning of access.
#define BUSWCON1_AAP_EARLY 0x00000000 */
/** Clock is enabled after address phase. */
#define BUSWCON1_AAP_LATE 0x00001000
/** Auxiliary Chip Select Enable
    Not supported in GPON-EBU, field must be set to 0. */
#define BUSWCON1_CSA 0x00000200
/* Disable
#define BUSWCON1_CSA_DIS 0x00000000 */
/** Enable */
#define BUSWCON1_CSA_EN 0x00000200
/** Flash Non-Array Access Enable
    Set to logic one to enable workaround when region is accessed with internal address bit 28 set. See Section 3.17.13 on page 90 for details. */
#define BUSWCON1_NAA 0x00000100
/* Disable
#define BUSWCON1_NAA_DIS 0x00000000 */
/** Enable */
#define BUSWCON1_NAA_EN 0x00000100
/** Module Enable */
#define BUSWCON1_ENABLE 0x00000001
/* Disable
#define BUSWCON1_ENABLE_DIS 0x00000000 */
/** Enable */
#define BUSWCON1_ENABLE_EN 0x00000001

/* Fields of "Bus Write Parameters Register1" */
/** Address Cycles
    Number of cycles for address phase. */
#define BUSWP1_ADDRC_MASK 0xF0000000
/** field offset */
#define BUSWP1_ADDRC_OFFSET 28
/** Address Hold Cycles For Multiplexed Address
    Number of address hold cycles during multiplexed accesses. */
#define BUSWP1_ADHOLC_MASK 0x0F000000
/** field offset */
#define BUSWP1_ADHOLC_OFFSET 24
/** Programmed Command Delay Cycles
    Number of delay cycles during command delay phase. */
#define BUSWP1_CMDDELAY_MASK 0x00F00000
/** field offset */
#define BUSWP1_CMDDELAY_OFFSET 20
/** Extended Data */
#define BUSWP1_EXTDATA_MASK 0x000C0000
/** field offset */
#define BUSWP1_EXTDATA_OFFSET 18
/** External device outputs data every BFCLK cycle */
#define BUSWP1_EXTDATA_ONE 0x00000000
/** External device outputs data every 2nd BFCLK cycles */
#define BUSWP1_EXTDATA_TWO 0x00040000
/** External device outputs data every 4th BFCLK cycles */
#define BUSWP1_EXTDATA_FOUR 0x00080000
/** External device outputs data every 8th BFCLK cycles */
#define BUSWP1_EXTDATA_EIGHT 0x000C0000
/** Frequency of external clock at pin BFCLKO */
#define BUSWP1_EXTCLOCK_MASK 0x00030000
/** field offset */
#define BUSWP1_EXTCLOCK_OFFSET 16
/** Equal to ebu_clk frequency. */
#define BUSWP1_EXTCLOCK_ONE_TO_ONE 0x00000000
/** 1/2 of ebu_clk frequency. */
#define BUSWP1_EXTCLOCK_ONE_TO_TWO 0x00010000
/** 1/3 of ebu_clk frequency. */
#define BUSWP1_EXTCLOCK_ONE_TO_THREE 0x00020000
/** 1/4 of ebu_clk frequency (default after reset). */
#define BUSWP1_EXTCLOCK_ONE_TO_FOUR 0x00030000
/** Data Hold Cycles For write Accesses
    Number of data hold cycles during write accesses. */
#define BUSWP1_DATAC_MASK 0x0000F000
/** field offset */
#define BUSWP1_DATAC_OFFSET 12
/** Programmed Wait States For write Accesses
    Number of programmed wait states for write accesses. For synchronous accesses, this will always be adjusted so that the phase exits on a rising edge of the external clock. */
#define BUSWP1_WAITWDC_MASK 0x00000F80
/** field offset */
#define BUSWP1_WAITWDC_OFFSET 7
/** Recovery Cycles After write Accesses, same CS
    Number of idle cycles after write accesses when following access is to the same chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSWCON. */
#define BUSWP1_RECOVC_MASK 0x00000070
/** field offset */
#define BUSWP1_RECOVC_OFFSET 4
/** Recovery Cycles After write Accesses, other CS
    Number of idle cycles after write accesses when the following access is to a different chip select. For synchronous accesses, this will always be adjusted so that the phase exits on a rising clock edge. Note that at least one recovery cycle must be programmed in case the region is configured for delayed control signals in field ecse of register EBU_BUSWCON. */
#define BUSWP1_DTACS_MASK 0x0000000F
/** field offset */
#define BUSWP1_DTACS_OFFSET 0

/* Fields of "Bus Protocol Configuration Extension Register 0" */
/** Byte Control Mapping
    Remapping of byte enable signals on address lines is not supported in the GPON-EBU. */
#define BUSCONEXT0_BCMAP_MASK 0x00030000
/** field offset */
#define BUSCONEXT0_BCMAP_OFFSET 16
/** No mirroring of byte enables. */
#define BUSCONEXT0_BCMAP_NOBCMAP 0x00000000
/** Asynchronous Early Write
    This bit is obsolete and must be set to 0 or unpredictable results may result. */
#define BUSCONEXT0_AEW 0x00008000
/** AAD-mux Consecutive Address Cycles
    This bit selects whether ADV gets deasserted between the high and the low address phase of a synchronous AAD-mux access or the two address cycles are consecutive. See Figure 32 for a waveform example that results when acac is set. acac only takes effect if the CS region is configured for synchronous AADmux access (agen = 1 or 3, aadmux = 1) and is ignored otherwise. */
#define BUSCONEXT0_ACAC 0x00004000
/* ADV is deasserted between high and low address phase.
#define BUSCONEXT0_ACAC_SEPERATED 0x00000000 */
/** ADV is not deasserted between high and low address phase. */
#define BUSCONEXT0_ACAC_CONSECUTIVE 0x00004000
/** AAD-mux Write Address-to-Address Delay
    Gives the length of the AA-Phase (in multiples of the ebu_clk cycle) to be used when writing to the CS region. The parameter is only observed if the CS region is configured for use of the AAD-mux protocol in register EBU_BUSWCON, fields agen and aadmux. */
#define BUSCONEXT0_WAAC_MASK 0x00003800
/** field offset */
#define BUSCONEXT0_WAAC_OFFSET 11
/** AAD-mux Read Address-to-Address Delay
    Gives the length of the AA-Phase (in multiples of the ebu_clk cycle) to be used when reading from the CS region. The parameter is only observed if the CS region is configured for use of the AAD-mux protocol in register EBU_BUSRCON, fields agen and aadmux. */
#define BUSCONEXT0_RAAC_MASK 0x00000700
/** field offset */
#define BUSCONEXT0_RAAC_OFFSET 8
/** AAD-mux Paging Enable for CS0
    If the external device is configured for AAD-mux protocol in register EBU_BUSRCON, then this field selects whether or not to use paging. If paging is enabled, the EBU skips the high address cycle in case the upper address that would be sent are the same as in the most recent access to the device.configures how to set the AD[15:14] in the high address cycle of an access with the following encoding: */
#define BUSCONEXT0_PAGE_EN 0x00000080
/* Disable
#define BUSCONEXT0_PAGE_EN_DIS 0x00000000 */
/** Enable */
#define BUSCONEXT0_PAGE_EN_EN 0x00000080
/** AAD-mux Address Extension Bit Generation Mode
    If the external device is configured for AAD-mux protocol in register EBU_BUSRCON, then this field configures how to set the AD[15:14] in the high address cycle of an access with the following encoding: */
#define BUSCONEXT0_AEBM_MASK 0x00000070
/** field offset */
#define BUSCONEXT0_AEBM_OFFSET 4
/** A[15] in the high address cycle is set to AMemport[amsb+17], A[14] is set to 0 */
#define BUSCONEXT0_AEBM_AMAP_CRE_RFU0 0x00000000
/** A[15] in the high address cycle is set to AMemport[amsb+17], A[14] is set to 1 */
#define BUSCONEXT0_AEBM_AMAP_CRE_RFU1 0x00000010
/** A[15] in the high address cycle is set to AMemport[amsb+18], A[14] is set to AMemport[amsb+17] */
#define BUSCONEXT0_AEBM_AMAP_CRE_AND_RFU 0x00000020
/** Do not use */
#define BUSCONEXT0_AEBM_reserved 0x00000030
/** A[15:14] in the high address cycle is set to 00B. */
#define BUSCONEXT0_AEBM_DIRECT_00 0x00000040
/** A[15:14] in the high address cycle is set to 01B */
#define BUSCONEXT0_AEBM_DIRECT_01 0x00000050
/** A[15:14] in the high address cycle is set to 10B */
#define BUSCONEXT0_AEBM_DIRECT_10 0x00000060
/** A[15:14] in the high address cycle is set to 11B. */
#define BUSCONEXT0_AEBM_DIRECT_11 0x00000070
/** Most Significant Address Bit of External Device
    If the external device is configured for AAD-mux protocol in register EBU_BUSRCON, then for amsb < 14 the EBU always sets A[13:amsb] = 0 in the high address cycle of an access. The value of A[15:14] is defined in field aebm. A value of amsb > 13 therefore has no effect. It is recommended to set amsb that it matches the addressable range of the external device according to the following formula: amsb = n - 16 for a device with 2n addressable words. */
#define BUSCONEXT0_AMSB_MASK 0x0000000F
/** field offset */
#define BUSCONEXT0_AMSB_OFFSET 0

/* Fields of "Bus Protocol Configuration Extension Register 1" */
/** Byte Control Mapping
    Remapping of byte enable signals on address lines is not supported in the GPON-EBU. */
#define BUSCONEXT1_BCMAP_MASK 0x00030000
/** field offset */
#define BUSCONEXT1_BCMAP_OFFSET 16
/** No mirroring of byte enables. */
#define BUSCONEXT1_BCMAP_NOBCMAP 0x00000000
/** Asynchronous Early Write
    This bit is obsolete and must be set to 0 or unpredictable results may result. */
#define BUSCONEXT1_AEW 0x00008000
/** AAD-mux Consecutive Address Cycles
    This bit selects whether ADV gets deasserted between the high and the low address phase of a synchronous AAD-mux access or the two address cycles are consecutive. See Figure 32 for a waveform example that results when acac is set. acac only takes effect if the CS region is configured for synchronous AADmux access (agen = 1 or 3, aadmux = 1) and is ignored otherwise. */
#define BUSCONEXT1_ACAC 0x00004000
/* ADV is deasserted between high and low address phase.
#define BUSCONEXT1_ACAC_SEPERATED 0x00000000 */
/** ADV is not deasserted between high and low address phase. */
#define BUSCONEXT1_ACAC_CONSECUTIVE 0x00004000
/** AAD-mux Write Address-to-Address Delay
    Gives the length of the AA-Phase (in multiples of the ebu_clk cycle) to be used when writing to the CS region. The parameter is only observed if the CS region is configured for use of the AAD-mux protocol in register EBU_BUSWCON, fields agen and aadmux. */
#define BUSCONEXT1_WAAC_MASK 0x00003800
/** field offset */
#define BUSCONEXT1_WAAC_OFFSET 11
/** AAD-mux Read Address-to-Address Delay
    Gives the length of the AA-Phase (in multiples of the ebu_clk cycle) to be used when reading from the CS region. The parameter is only observed if the CS region is configured for use of the AAD-mux protocol in register EBU_BUSRCON, fields agen and aadmux. */
#define BUSCONEXT1_RAAC_MASK 0x00000700
/** field offset */
#define BUSCONEXT1_RAAC_OFFSET 8
/** AAD-mux Paging Enable for CS0
    If the external device is configured for AAD-mux protocol in register EBU_BUSRCON, then this field selects whether or not to use paging. If paging is enabled, the EBU skips the high address cycle in case the upper address that would be sent are the same as in the most recent access to the device.configures how to set the AD[15:14] in the high address cycle of an access with the following encoding: */
#define BUSCONEXT1_PAGE_EN 0x00000080
/* Disable
#define BUSCONEXT1_PAGE_EN_DIS 0x00000000 */
/** Enable */
#define BUSCONEXT1_PAGE_EN_EN 0x00000080
/** AAD-mux Address Extension Bit Generation Mode
    If the external device is configured for AAD-mux protocol in register EBU_BUSRCON, then this field configures how to set the AD[15:14] in the high address cycle of an access with the following encoding: */
#define BUSCONEXT1_AEBM_MASK 0x00000070
/** field offset */
#define BUSCONEXT1_AEBM_OFFSET 4
/** A[15] in the high address cycle is set to AMemport[amsb+17], A[14] is set to 0 */
#define BUSCONEXT1_AEBM_AMAP_CRE_RFU0 0x00000000
/** A[15] in the high address cycle is set to AMemport[amsb+17], A[14] is set to 1 */
#define BUSCONEXT1_AEBM_AMAP_CRE_RFU1 0x00000010
/** A[15] in the high address cycle is set to AMemport[amsb+18], A[14] is set to AMemport[amsb+17] */
#define BUSCONEXT1_AEBM_AMAP_CRE_AND_RFU 0x00000020
/** Do not use */
#define BUSCONEXT1_AEBM_reserved 0x00000030
/** A[15:14] in the high address cycle is set to 00B. */
#define BUSCONEXT1_AEBM_DIRECT_00 0x00000040
/** A[15:14] in the high address cycle is set to 01B */
#define BUSCONEXT1_AEBM_DIRECT_01 0x00000050
/** A[15:14] in the high address cycle is set to 10B */
#define BUSCONEXT1_AEBM_DIRECT_10 0x00000060
/** A[15:14] in the high address cycle is set to 11B. */
#define BUSCONEXT1_AEBM_DIRECT_11 0x00000070
/** Most Significant Address Bit of External Device
    If the external device is configured for AAD-mux protocol in register EBU_BUSRCON, then for amsb < 14 the EBU always sets A[13:amsb] = 0 in the high address cycle of an access. The value of A[15:14] is defined in field aebm. A value of amsb > 13 therefore has no effect. It is recommended to set amsb that it matches the addressable range of the external device according to the following formula: amsb = n - 16 for a device with 2n addressable words. */
#define BUSCONEXT1_AMSB_MASK 0x0000000F
/** field offset */
#define BUSCONEXT1_AMSB_OFFSET 0

/* Fields of "Serial Flash Configuration Register" */
/** Direct Access Device Port Width
    DA_PORTW Defines the number of signal lines to be used with direct read access from a Serial Flash as defined for the command with opcode rd_opc. Depending on thedevice type and/or command, the number of used signal lines might differbetween command, address, and data phase of the transaction. */
#define SFCON_DA_PORTW_MASK 0xE0000000
/** field offset */
#define SFCON_DA_PORTW_OFFSET 29
/** One signal line used in all phases of the transaction. */
#define SFCON_DA_PORTW_WIDTH_1_1_1 0x00000000
/** One signal line used in the COMMAND and ADDRESS phase of the transaction and two signal lines used in the DATA phase. */
#define SFCON_DA_PORTW_WIDTH_1_1_2 0x20000000
/** One signal used in the COMMAND phase of the transaction and two signal lines used in the ADDRESS/DUMMY phase and the DATA phase. */
#define SFCON_DA_PORTW_WIDTH_1_2_2 0x40000000
/** Two signal lines used in all phases of the transaction. */
#define SFCON_DA_PORTW_WIDTH_2_2_2 0x60000000
/** One signal line used in the COMMAND and ADDRESS phase of the transaction and four signal lines used in the DATA phase. */
#define SFCON_DA_PORTW_WIDTH_1_1_4 0x80000000
/** One signal used in the COMMAND phase of the transaction and four signal lines used in the ADDRESS/DUMMY phase and the DATA phase. */
#define SFCON_DA_PORTW_WIDTH_1_4_4 0xA0000000
/** Four signal lines used in all phases of the transaction. */
#define SFCON_DA_PORTW_WIDTH_4_4_4 0xC0000000
/** for future use. */
#define SFCON_DA_PORTW_WIDTH_reserved 0xE0000000
/** Read Abort Enable
    If set, a read access from the external device can be aborted via signal sf_rd_abort_i. See Section 3.18.2.9 for details. */
#define SFCON_RD_ABORT_EN 0x10000000
/** Device Size
    Defines the number of significant address bits for the Serial Flash device(s). All address bits above the MSB are forced to 0. The configuration in this field also defines for the address auto-increment feature when to wrap around from the upper most address to 0. */
#define SFCON_DEV_SIZE_MASK 0x0F000000
/** field offset */
#define SFCON_DEV_SIZE_OFFSET 24
/** 16 MBit device */
#define SFCON_DEV_SIZE_A20_0 0x00000000
/** 32 MBit device */
#define SFCON_DEV_SIZE_A21_0 0x01000000
/** 64 MBit device */
#define SFCON_DEV_SIZE_A22_0 0x02000000
/** 128 MBit device */
#define SFCON_DEV_SIZE_A23_0 0x03000000
/** 256 MBit device */
#define SFCON_DEV_SIZE_A24_0 0x04000000
/** 512 MBit device */
#define SFCON_DEV_SIZE_A25_0 0x05000000
/** 1 GBit device */
#define SFCON_DEV_SIZE_A26_0 0x06000000
/** 2 GBit device */
#define SFCON_DEV_SIZE_A27_0 0x07000000
/** 4 GBit device */
#define SFCON_DEV_SIZE_A28_0 0x08000000
/** 8 GBit device */
#define SFCON_DEV_SIZE_A29_0 0x09000000
/** 16 GBit device */
#define SFCON_DEV_SIZE_A30_0 0x0A000000
/** 32 GBit device */
#define SFCON_DEV_SIZE_A31_0 0x0B000000
/** Device Page Size
    Defines the page size employed by all connected Serial Flash devices. The device page size is used to determine the address wrap-around for the write address auto-increment feature. */
#define SFCON_DPS_MASK 0x00C00000
/** field offset */
#define SFCON_DPS_OFFSET 22
/** Device page size is 256 Bytes */
#define SFCON_DPS_DPS_256 0x00000000
/** Device page size is 512 Bytes */
#define SFCON_DPS_DPS_512 0x00400000
/** Page Buffer Size
    Defines the size of the EBU's page buffer used in Buffered Access. Page buffer size configured here must be less than or equal to the maximum page buffer size which is a built option of the EBU (256 Bytes for GPON). */
#define SFCON_PB_SIZE_MASK 0x00300000
/** field offset */
#define SFCON_PB_SIZE_OFFSET 20
/** No read buffer is available/used. */
#define SFCON_PB_SIZE_NONE 0x00000000
/** 128 Bytes */
#define SFCON_PB_SIZE_SIZE_128 0x00100000
/** 256 Bytes */
#define SFCON_PB_SIZE_SIZE_256 0x00200000
/** Bidirectional Data Bus
    Defines whether the Serial Flash uses a unidirectional or a bidirectional data bus. */
#define SFCON_BIDIR 0x00080000
/* The Serial Flash interface uses a pair of two unidirectional busses (one for write, one for read)
#define SFCON_BIDIR_UNIDIRECTIONAL 0x00000000 */
/** The Serial Flash interface uses a bidirectional data bus. */
#define SFCON_BIDIR_BIDIRECTIONAL 0x00080000
/** No Busy Error termination
    By default, the EBU error-terminates all direct access to a Serial Flash while EBU_SFSTAT.busy is set. By setting NO_BUSY_ERR, the EBU can be configured to permit direct accesses to proceed to the Serial Flash, e.g. for devices that support a read-while-write functionality. */
#define SFCON_NO_BUSY_ERR 0x00040000
/** End-of-Busy Detection Mode
    Defines how the EBU detects the end of a busy phase in the Serial Flash device. The current version of the EBU requires the software to explicitly poll the device's status register and then inform the EBU on the end of the busy status by clearing the corresponding bit in register EBU_SF_STAT. */
#define SFCON_EOBDM_MASK 0x00030000
/** field offset */
#define SFCON_EOBDM_OFFSET 16
/** No read buffer is available/used. */
#define SFCON_EOBDM_SOFTWARE 0x00000000
/** Poll device status register (not supported yet) */
#define SFCON_EOBDM_POLL_SR 0x00010000
/** Poll devices busy/ready pin fed into EBU via WAIT pin (not supported yet). */
#define SFCON_EOBDM_POLL_RDY 0x00020000
/** Same as POLL_RDY, but CS must be asserted to have the device output its busy/ready status (not supported yet). */
#define SFCON_EOBDM_POLL_RDY_WITH_CS 0x00030000
/** Direct Access Keep Chip Select
    Defines whether the Serial Flash remains selected after a direct access transaction has been finished. */
#define SFCON_DA_KEEP_CS 0x00008000
/* After a direct read access, the Serial Flash device is always deselected (CS deasserted). Follow-up read accesses always require sending command opcode and address.
#define SFCON_DA_KEEP_CS_DESELECT 0x00000000 */
/** Chip Select of device is kept active after direct read access so that device is ready for follow-up read of next sequential byte without the need to send command and address. If the next command is to another Chip Select, is a different command or accesses a different address, the EBU first deactivates the kept Chip Select before it starts the new transaction with sending the command opcode and address. */
#define SFCON_DA_KEEP_CS_KEEP_SELECTED 0x00008000
/** Early Read Abort Enable
    When aborting a Serial Flash Read is enabled in bit EBU_SFCON.rd_abort_en, bit early_abort selects at what point in the protocol an external access might be aborted. Datasheets of many Serial Flash devices are not explicit on what happens (and whether it is allowed) when a read access is cut-short by deselecting the device during the CMD, ADDR or DUMMY phase of the protocol. */
#define SFCON_EARLY_ABORT 0x00004000
/* DISABLE Early abortion is disabled (default after reset). Once the EBU has started the access on the External Bus (first bit time slot), the EBU continues the external transfer until the first data byte has been received. After a direct read access, the Serial Flash device is always deselected (CS deasserted). Follow-up read accesses always require sending command opcode and address.
#define SFCON_EARLY_ABORT_DISABLE 0x00000000 */
/** Early abortion is not yet supported in the current version of the EBU. Do not use. The feature is a late improvement to the EBU and could not be verified completely before the final release. After proven to work, it should be made officially available to reduce access latency after aborted Serial Flash reads. Setting early_abort to ENABLE alters the read abort handling in the following way: Once the EBU has started the access on the External Bus, the transfer is cut-short after transferring the CMD byte, the three address bytes, any DUMMY bits or at the end of the next data byte - whatever comes first. */
#define SFCON_EARLY_ABORT_ENABLE 0x00004000
/** Direct Access Address Length
    Defines the number of address bytes to be sent (MSB first) to the device with a direct read access transaction. Other values than listed below are not supported and have unpredictable results. */
#define SFCON_DA_ALEN_MASK 0x00003000
/** field offset */
#define SFCON_DA_ALEN_OFFSET 12
/** 3 address bytes (bits 23:0 of the internal address) */
#define SFCON_DA_ALEN_THREE 0x00000000
/** Read Access Dummy Bytes
    This field defines the number of dummy bytes to send between the last address byte before the EBU starts capturing read data from the bus for a direct read access. The number of dummy bytes depends on the data access command being used (see field), the clock frequency and the type of device being used. */
#define SFCON_RD_DUMLEN_MASK 0x00000F00
/** field offset */
#define SFCON_RD_DUMLEN_OFFSET 8
/** Direct Read Access Command Opcode
    This byte defines the command opcode to send when performing a data read from the Serial Flash in Direct Access Mode. Any value can be set (the EBU does not interpret the value, but directly uses the contents of this register field in the command phase of the transaction). Common opcodes to be used and understood by most devices are READ (03H) and FAST_READ (0BH), but some devices might provide additional opcodes, e.g. to support higher clock frequencies requiring additional dummy bytes or to define a wider interface bus. */
#define SFCON_RD_OPC_MASK 0x000000FF
/** field offset */
#define SFCON_RD_OPC_OFFSET 0
/** READ */
#define SFCON_RD_OPC_READ 0x00000003
/** FAST_READ */
#define SFCON_RD_OPC_FAST_READ 0x0000000B

/* Fields of "Serial Flash Timing Register" */
/** CS Idle time
    This field defines the minimum time the device's Chip Select has to be deasserted in between accesses. Most devices require a minimum deselect time between 50 and 100 ns. See Table 43 for the encoding used in this field. */
#define SFTIME_CS_IDLE_MASK 0xF0000000
/** field offset */
#define SFTIME_CS_IDLE_OFFSET 28
/** 1 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_0 0x00000000
/** 2 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_1 0x10000000
/** 3 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_2 0x20000000
/** 4 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_3 0x30000000
/** 6 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_4 0x40000000
/** 8 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_5 0x50000000
/** 10 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_6 0x60000000
/** 12 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_7 0x70000000
/** 14 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_8 0x80000000
/** 16 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_9 0x90000000
/** 20 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_10 0xA0000000
/** 24 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_11 0xB0000000
/** 32 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_12 0xC0000000
/** 40 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_13 0xD0000000
/** 48 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_14 0xE0000000
/** 64 EBU clock cycles */
#define SFTIME_CS_IDLE_CLKC_15 0xF0000000
/** CS Hold time
    This field defines (in multiples of the EBU internal clock's period) the minimum time the device's Chip Select must remain asserted after transfer of the last bit of a write transaction. This CS hold time does not apply to read accesses */
#define SFTIME_CS_HOLD_MASK 0x0C000000
/** field offset */
#define SFTIME_CS_HOLD_OFFSET 26
/** CS Setup time
    This field defines (in multiples of the EBU internal clock's period) when to assert the device's Chip Select before the first SCK clock period for transferring the command is started on the External Bus */
#define SFTIME_CS_SETUP_MASK 0x03000000
/** field offset */
#define SFTIME_CS_SETUP_OFFSET 24
/** Write-to-Read Pause
    This field defines the length of the optional pause when switching from write to read direction in the transaction. During this pause, SCK is held stable. */
#define SFTIME_WR2RD_PAUSE_MASK 0x00300000
/** field offset */
#define SFTIME_WR2RD_PAUSE_OFFSET 20
/** Read Data Position
    This field defines when to capture valid read data bit(s) (in multiples of half of the EBU internal clock's period) relative to the beginning of the SCK clock's period defined in EBU_SFTIME.sck_per. RD_POS must be less than or equal to EBU_SFTIME.sck_per (not checked in hardware) or unpredictable results may occur. */
#define SFTIME_RD_POS_MASK 0x000F0000
/** field offset */
#define SFTIME_RD_POS_OFFSET 16
/** SCK Fall-edge Position
    This field defines the positioning of the SCK fall edge (in multiples of half of the EBU internal clock's period) with respect to the beginning of the SCK clock's period defined in EBU_SFTIME.sck_per. SCKF_POS must be less than or equal to SCK_PER (not checked in hardware) or unpredictable results may occur. If EBU_SFTIME.sck_inv is set, SCKF_POS defines the positioning of the falling instead of the rising edge of SCK. In the current version of the EBU, SCKF_POS must be set 0 or unpredictable results may occur. */
#define SFTIME_SCKF_POS_MASK 0x0000F000
/** field offset */
#define SFTIME_SCKF_POS_OFFSET 12
/** SCK Rise-edge Position
    This field defines the positioning of the SCK rise edge (in multiples of half of the EBU internal clock's period) with respect to the beginning of the SCK clock's period defined in EBU_SFTIME.sck_per. SCKR_POS must be less than EBU_SFTIME.sck_per (not checked in hardware) or unpredictable results may occur. If EBU_SFTIME.sck_inv is set, SCKR_POS defines the positioning of the falling instead of the rising edge of SCK. */
#define SFTIME_SCKR_POS_MASK 0x00000F00
/** field offset */
#define SFTIME_SCKR_POS_OFFSET 8
/** SCK Feedback Clock Inversion
    If set, read data gets captured with the falling instead of the rising edge of SCK if clock feedback is enabled in EBU_SFTIME.sck_fdbk_en. */
#define SFTIME_SCK_FDBK_INV 0x00000040
/** SCK Clock Feedback
    If set, read data is captured using the external SCK clock feedback into the chip instead of the EBU's internal clock. Using the feedback clock compensate for the high delay over the pads and its use is required at higher frequencies. A penalty for synchronizing the read data from the SCK into the ebu_clk domain applies to the read access latency. */
#define SFTIME_SCK_FDBK_EN 0x00000020
/** Inverted SCK
    If set, the clock to the Serial Flash devices is inverted. This also results in SCK high while a Serial Flash remains selected between transactions (keep_cs feature). In the current version of the EBU, clock inversion is not supported. SCK_INV must be set to 0 or unpredictable results may occur. */
#define SFTIME_SCK_INV 0x00000010
/** SCK Period
    This field defines the period of the SCK clock in multiples of half of the EBU clock period. The EBU supports values between 2 and 14, corresponding to a frequency ratio range from 1:1. to 1:7 between SCK and the internal clock. Other values are prohibited and result in unpredictable behaviour. In the current version of the EBU, odd values for SCK_PER are not supported. */
#define SFTIME_SCK_PER_MASK 0x0000000F
/** field offset */
#define SFTIME_SCK_PER_OFFSET 0

/* Fields of "Serial Flash Status Register" */
/** Command Overwrite Error
    This bit is set on an attempt to start an indirect access while a previous indirect access has not finished. The bit remains unaltered when the software writes a '0' and is toggled when a '1' is written. This toggle-by-write-1 behavior allows to also set the bit for testing purposes. In normal operation, the software is supposed to only write a '1' to this bit to clear after it has been set by the Serial Flash protocol engine. */
#define SFSTAT_CMD_OVWRT_ERR 0x40000000
/** Command Error
    This bit is set when the EBU discards an indirect or direct access to/from a Serial Flash. The bit remains unaltered when the software writes a '0' and is toggled when a '1' is written. This toggle-by-write-1 behavior allows to also set the bit for testing purposes. In normal operation, the software is supposed to only write a '1' to this bit to clear after it has been set by the Serial Flash protocol engine. */
#define SFSTAT_CMD_ERR 0x20000000
/** Access Command Pending
    If set, indicates that access from/to a Serial Flash device has not finished yet. */
#define SFSTAT_CMD_PEND 0x00400000
/** External Device Selected
    If set, indicates that the Chip Select of a Serial Flash device is currently active on the External Bus. */
#define SFSTAT_SELECTED 0x00200000
/** Protocol Engine Active
    If set, indicates that the EBU's Serial Flash protocol engine is active. */
#define SFSTAT_ACTIVE 0x00100000
/** Page Buffer Invalidate
    When writing a one to this bit, bits PB_VALID and PB_UPDATE are both cleared, thereby invalidating the page buffer for access to/from the Serial Flash device. After invalidating the buffer, PB_INVALID is automatically cleared so that it always reads as 0. */
#define SFSTAT_PB_INVALID 0x00010000
/** Page Buffer Update
    This bit is set when data in the page buffer gets modified. It is cleared when new data gets loaded to the page buffer, when it is written back to the device (WRITE_PAGE command) or when PB_VALID gets cleared. */
#define SFSTAT_PB_UPDATE 0x00002000
/** Page Buffer Valid
    This bit is set after the last data byte of a LOAD_PAGE command has been stored in the page buffer or when the page buffer is explicitely validated via a VALIDATE_PAGE special command. It remains set until the page buffer gets invalidated by writing a 1 to PB_INVALID or any of the LOAD_PAGE special commands. While PB_VALID is set, all accesses to the buffered address range are diverted to the page buffer with no access being performed on the External Bus. */
#define SFSTAT_PB_VALID 0x00001000
/** Page Buffer Busy
    The bit is set when the EBU starts executing a LOAD_PAGE or a WRITE_PAGE command and cleared when the last byte of the requested page has been transferred from/to the external device. The inverted value of PB_BUSY is output on the EBU interface and may trigger a system interrupt. */
#define SFSTAT_PB_BUSY 0x00000100
/** Device Busy
    This bit is set by the Serial Flash protocol engine when an indirect access is performed via register EBU_SFCMD with SET_BUSY being set. While busy is set, access to the Serial Flash is very limited and all transactions are error-terminated except when explicitly marked to ignore the busy status. If the EBU is configured in EBU_SFCON.EOBDM to automatically poll the busy status of the device, busy is cleared as soon as the device is found to be idle again. On a software write, busy remains unaltered when written with a '0' and is toggled when written with a '1', respectively.This toggle-by-write-1 behaviour allows to also set the bit for testing purposes. In normal operation, the software is supposed to only write a '1' to this bit after it got set by the Serial Flash protocol engine and no automatic busy detection is configured in EBU_SFCON.EOBDM Then the software has to clear busy when it finds the device to be no longer busy by either polling the device's status register via the EBU or by waiting for the maximum busy time of the operation started in the device. */
#define SFSTAT_BUSY 0x00000001

/* Fields of "Serial Flash Command Register" */
/** Command Type
    This field is a qualifier of the command opcode in EBU_SFCMD.opc. Two types */
#define SFCMD_CMDTYPE 0x80000000
/* The opcode in EBU_SFCMD.opc is directly used in the command phase of a single transaction to the Serial Flash device.
#define SFCMD_CMDTYPE_ACCESS_CMD 0x00000000 */
/** The opcode in EBU_SFCMD.opc is used to start a special command in the Serial Flash Controller which might include any number of external transactions to/from the Serial Flash device. */
#define SFCMD_CMDTYPE_SPECIAL_CMD 0x80000000
/** Device Port Width
    Defines the number of signal lines to be used with direct read access from a Serial Flash as defined for the command with opcode opc. The encoding of this field is the same as forDA_PORTW. */
#define SFCMD_PORTW_MASK 0x70000000
/** field offset */
#define SFCMD_PORTW_OFFSET 28
/** Bidirectional Signal Lines
    If set selects bidirectional signal lines to be used for the data transfer. */
#define SFCMD_BIDIR 0x08000000
/** Chip Select
    This field selects which of the EBU's Chip Selects to activated for the command that is written to EBU_SFCMD.opc. A value between 0 and 3 selects one of the EBU's main CSs while 4 to 7 chooses one of the Auxiliary Chip Selects CSA[3:0], respectively. */
#define SFCMD_CS_MASK 0x07000000
/** field offset */
#define SFCMD_CS_OFFSET 24
/** Disable Auto Address Increment
    By default, the address in register EBU_SFADDR is automatically incremented with each data byte being transferred. By setting this bit, the auto-increment can be disabled. */
#define SFCMD_DIS_AAI 0x00800000
/** Address Length
    Defines the number of address bytes from register EBU_SFADDR to sent in the address phase of the transaction to/from the Serial Flash. Note: Address bytes are also sent when the command has no data. */
#define SFCMD_ALEN_MASK 0x00700000
/** field offset */
#define SFCMD_ALEN_OFFSET 20
/** Dummy Phase Length
    Defines the number of dummy bytes to send to the device between the command/address phase and the data phase of a transaction. Note:Dummy bytes are also sent when the command has no address and/or no data. */
#define SFCMD_DUMLEN_MASK 0x000F0000
/** field offset */
#define SFCMD_DUMLEN_OFFSET 16
/** Keep Chip Select
    Defines whether the Serial Flash remains selected after the indirect access transaction has been finished. */
#define SFCMD_KEEP_CS 0x00008000
/* After a direct read access, the Serial Flash device is always deselected (CS deasserted). Follow-up read accesses always require sending command opcode and address.
#define SFCMD_KEEP_CS_DESELECT 0x00000000 */
/** Chip Select of device is kept active after direct read access so that device is ready for follow-up read of next sequential byte without the need to send command and address. If the next command is to another Chip Select, is a different command or accesses a different address, the EBU first deactivates the kept Chip Select before it starts the new transaction with sending the command opcode and address. */
#define SFCMD_KEEP_CS_KEEP_SELECTED 0x00008000
/** Set Busy Flag
    If set, starting the command sets EBU_SFSTAT.busy. */
#define SFCMD_SET_BUSY 0x00004000
/** Ignore Busy
    By default, the EBU error terminates all attempts to access a Serial Flash while EBU_SFSTAT.busy is set. Setting this bit overrules this error termination and permits the command written to EBU_SFCMD.opc to proceed to the External Bus. Normally, this bit is only set to execute a Read Status Register command to the Serial Flash, but may also be used for any other type of access the device is able to handle while it is busy. */
#define SFCMD_IGNORE_BUSY 0x00002000
/** Skip Opcode
    If this bit is set, the opcode in field OPC is not sent to the External Bus, but the external transaction starts with sending the first address byte (if ALEN 0), the first dummy byte (if alen = 0 and DUMLEN 0), or directly with transferring the data bytes (if ALEN = DUMLEN = 0 and DLEN 0). Limiting the external transfer to just the data phase - together with the keep_cs feature - allow to transfer any number of data bytes for a device command sent via EBU_SFCMD by keeping the device selected between accesses and chaining multiple indirect access commands each transferring up to 4 data bytes from/to register EBU_SFDATA. */
#define SFCMD_SKIP_OPC 0x00001000
/** Data Length
    This field defines the number of data bytes to transfer in the data phase of the command. For a read command, the data bytes are stored in register EBU_SFDATA, for a write transfer they are taken from that register. As the data register can hold at most 4 bytes, DLEN is restricted to the range [0..4]. */
#define SFCMD_DLEN_MASK 0x00000E00
/** field offset */
#define SFCMD_DLEN_OFFSET 9
/** Direction
    Defines the direction of the data transfer (if any) in the data phase of the transaction to/from the serial bus. */
#define SFCMD_DIR 0x00000100
/* dlen bytes of data are read from the Serial Flash during the data phase of the transaction and stored in register EBU_SFDATA.
#define SFCMD_DIR_READ 0x00000000 */
/** dlen bytes of data are read from register EBU_SFDATA and written to the Serial Flash during the data phase of the transactione */
#define SFCMD_DIR_WRITE 0x00000100
/** Command Opcode
    A write access to this field starts an Indirect Access command in the EBU's Serial Flash controller. Two types of commands are supported (selected in EBU_SFCMD.cmdtype) and determine how the EBU interprets the opcode:- - For a ACCESS_CMD, a single transaction is executed to/from the Serial Flash device and the OPC is sent to the device in the command phase of the protocol. The number of address, dummy and data bytes to transfer with the command are given in fields ALEN, DUMLEN, and DLEN of register EBU_SFCMD, respectively. - For a SPECIAL_CMD, the EBU starts a complex operation that usually involves multiple transactions to/from the Serial Flash device. See Section 3.18.2.5 for an overview of the complex commands currently supported. */
#define SFCMD_OPC_MASK 0x000000FF
/** field offset */
#define SFCMD_OPC_OFFSET 0

/* Fields of "Serial Flash Address Register" */
/** Address
    Before writing to register EBU_SFCMD to start a command that requires the transfer of an address, the address to use must be stored in this register. If not disabled in EBU_SFCMD.dis_aai, ADDR is incremented automatically with each data byte transferred between the EBU and the Serial Flash for an indirect access. Note:Register EBU_SFADDR is only used for access in Indirect Access Mode and is ignored/remains unaltered for all accesses in Direct Access Mode. */
#define SFADDR_ADDR_MASK 0xFFFFFFFF
/** field offset */
#define SFADDR_ADDR_OFFSET 0

/* Fields of "Serial Flash Data Register" */
/** Data Bytes
    Before writing to register EBU_SFCMD to start a command that requires the transfer of data from the EBU to the Serial Flash device (write access), the data to send must be stored in this register. The data bytes have to be right-aligned in this register, that is, the last byte to send must be placed in bits DATA[7:0], the second-to-last byte in bits DATA[15:8], etc.. Similarly, for a read access with data being transferred from the Serial Flash to the EBU, this register collects the read data received from the device. The read data is right-aligned, that is, the last byte received gets placed in bits DATA[7:0], the second-to-last byte in bits DATA[15:8], etc... The number of data bytes to be transferred between EBU and the Serial Flash is defined in EBU_SFCMD.DLEN. Note:Register EBU_SFDATA is only used for accesses in Indirect Access Mode and is ignored/remains unaltered for all accesses in Direct Access Mode. */
#define SFDATA_DATA_MASK 0xFFFFFFFF
/** field offset */
#define SFDATA_DATA_OFFSET 0

/* Fields of "Serial Flash I/O Control Register" */
/** Start of Write Delay
    By default, the EBU starts driving to AD[3:0] two EBU clock cycles before asserting the CS for an external Serial Flash access. For write accesses, this delay can be increased via field SOWD. */
#define SFIO_SOWD_MASK 0x0000F000
/** field offset */
#define SFIO_SOWD_OFFSET 12
/** End of Write Delay
    This field defines the time (in number of EBU clock cycles) for which the EBU keeps driving the External Bus AD[3:0] after deassertion of the device's CS. */
#define SFIO_EOWD_MASK 0x00000F00
/** field offset */
#define SFIO_EOWD_OFFSET 8
/** Data Output
    The EBU always controls the AD[3:0] pins while a CS for a Serial Flash device is asserted. Field UNUSED_WD defines the values being driven to these pins while the Serial Flash controller is not writing data to or is reading data from the device via the respective line. See Section 3.18.6 for details. */
#define SFIO_UNUSED_WD_MASK 0x0000000F
/** field offset */
#define SFIO_UNUSED_WD_OFFSET 0

/*! @} */ /* EBU_REGISTER */

#endif /* _ebu_reg_h */
