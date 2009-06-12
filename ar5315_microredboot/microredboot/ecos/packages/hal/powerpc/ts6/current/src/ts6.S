#==============================================================================
##
##      ts6.S
##
##      MPC8260 TS6 board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002 Gary Thomas
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   pfine, wpd
## Contributors: nickg
## Date:        2002-02-27
## Purpose:     TigerSHARC-6 board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on the Delphi TigerSHARC-6 board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/arch.inc>
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */
#------------------------------------------------------------------------------
                
        .globl  hal_hardware_init
hal_hardware_init:
// It is possible that the board may experience a soft reset, which
// will cause this hal_hardware_init routine to be called.  As a 
// result of the soft reset, the IMMR, memory controller, system
// protection logic, interrupt controller and parallel I/O pins are
// NOT reset (MPC8260 User Manual p 5-2), and therefor we do not want to
// run the setup in this routine.  Luckily, a hard reset will clear
// the NHR bit (bit 15) of the HID0 register.  We will check for this bit
// to be 0 (implying a hard reset).  If it is 0, we will run the 
// initialization.  If non-zero, we will skip the initialization.
// 
// If we do the initialization, then we must set the NHR bit so that
// next time we hit this point in the routine, we can determine the type
// of reset.

   mfspr  r22,CYGARC_REG_HID0    # Get contents of HID0
   rlwinm r23,r22,0,15,15 #shift HID0 by 0, mask with 0x10000, store in r23
   cmpwi  r23,0       # Compare r23 with 0
   bne    hardware_init_done

   mfspr r31,CYGARC_REG_LR


   #----------------------------------------------
   # Load the IMMR register with the base address
   #----------------------------------------------

   addis    r4,0,0x0471   # IMMR base addr = 0x04700000+10000. We add 
                          # 0x10000 because using relative addressing
                          # in load and store instructions only allow a
                          # offset from the base of +/-32767.
   addis    r5,0,0x0470

   addis    r3,0,0x0F01            

# The default IMMR base address was 0x0F0000000 as 
# originally programmed into the Hard Reset 
# Configuration Word.
   stw      r5,CYGARC_REG_IMM_IMMR(r3)
                          


   mfspr    r21,CYGARC_REG_HID0	  # get HID0 in R21
   oris	    r21,r21,0x1     # Set bit 15 of HID0 (NHR)

   mtspr    CYGARC_REG_HID0,r21       # load HID0 with NHR set


   #*******************************************
   # Main System Clock Configuration Registers 
   #*******************************************

   #-------------------------------------------------------------------------
   # We only need to program the System Clock Control Register (SCCR). The 
   # System Clock Mode Register (SCMR) doesn t need to be programmed here
   # because the MODCLK_HI bits in the Hard Reset Configuration Word and the
   # MODCK pins dictate the values in the SCCR during power-on reset.
   #-------------------------------------------------------------------------

   #-------------------------------------------------------------------------
   # Program the System Clock Control Register (SCCR).
   #
   # - Bits 0 - 28 Reserved. Clear to 0.
   #
   # - CLPD (CPM Low Power Disable) = 0 = 
   #
   #     CPM does not enter low power mode when the core enters low power 
   #     mode.
   #
   # - DFBRG (Division Factor of BRGCLK) = 01 - Divide by 8.
   #
   #-------------------------------------------------------------------------
 	

   addis    r3,0,0x0000
   ori      r3,r3,0x0001   # SCCR = 0x00000001
   stw      r3,CYGARC_REG_IMM_SCCR(r4)

   #~~~~~~~~~~~~~~~~~~~~
   # Initialize the SIU
   #~~~~~~~~~~~~~~~~~~~~

   bl       init_siu

   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   # Initialize the memory controller and SDRAM
   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 
   bl       init_memc

   mtspr    CYGARC_REG_LR,r31      # restore original Link Register value 

hardware_init_done:

#define nFLASH_LEDS_FOREVER
#ifdef FLASH_LEDS_FOREVER
   bl   init_user_leds
   b    flash_leds_forever
#endif
#if 0
	b memory_test
#endif


        bclr 20,0

	
#########################################################################
# Function: init_siu
#
# Description: Initializes the System Interface Unit
#
#  History:
#
#
#########################################################################        

init_siu:

   #-----------------------------------------------------------------------
   # Program the System Protection Control Register with the following 
   # values:
   #
   # - SWTC (Software Watchdog Timer Count) = 0xFFFF = programmed to max
   #   count value.
   #
   # - BMT (Bus Monitor Timing) = 0xFF = Set to max timeout period.
   #
   # - PBME (60x Bus Monitor Enable) = 1 = enabled.
   #
   # - LBME (Local Bus Monitor Enable) = 1 = enabled.
   #  
   # - Bits 26-28 Reserved. Set to 0.
   #
   # - SWE (Software Watchdog Enabled) = 0 = disabled for now. User will 
   #   have to enable this in a non-debug application.
   #
   # - SWRI (Software Watchdog Reset/Interrupt Select) = 1 = Software 
   #   watchdog timeout or bus monitor time-out causes a soft reset.
   #
   # - SWP (Software Watchdog Prescale) = 1 = clock is prescaled.
   #-----------------------------------------------------------------------

   addis    r3,0,0xFFFF
   ori      r3,r3,0xFFC3   # SYPCR = 0xFFFFFFC3
   stw      r3,CYGARC_REG_IMM_SYPCR(r4)    


   #-------------------------------------------------------------------------
   # Program the Bus Configuration Register. The details are as follows:
   #
   # - EBM (External Bus Mode) = 0 = Single MPC8260 bus mode is assumed.
   #
   # - APD (Address Phase delay) = 001 = 
   #
   #     One address tenure wait states for address operations initiated by 
   #     a 60x bus master.
   #                              
   # - L2C (Secondary Cache Controller) = 0 = No secondary cache controller
   #                                          is assumed.
   #
   # - L2D (L2 cache hit Delay) = 000 = This is a don t care because we re 
   #                                    not using the L2 cache.
   #
   # - PLDP (PipeLine maximum Depth) = 0 = The pipeline max depth is one.
   #
   # - Bits 9-11 = Reserved. Set to 0.
   #
   # - ETM (Compatibility Mode enable) = 1 = Extended transfer mode is
   #                                         enabled.
   #
   # - LETM (Local Bus Compatibility Mode Enable) = 1 = 
   #
   #     Extended transfer mode is enabled on the local bus.
   #
   # - EPAR (Even Parity) = 0 = This is a do not care.
   #
   # - LEPAR (Local Bus Even Parity) = 0 = This is a do not care.
   #
   # - Bits 16-20 = Reserved. Set to 0.
   #
   # - EXDD (External Master Delay Disable) = 0 = 
   #
   #     The memroy controller inserts one wait state between the assertion 
   #     of TS and the assertion of CS when an external master accesses an 
   #     address space controlled by the memory controller.
   #
   # - Bits 22-26 = Reserved. Set to 0.
   #
   # - ISPS (Internal Space Port Size) = 0 = 
   #
   #     MPC8260 acts as a 64-bit slave to external master accesses to its 
   #     internal space.
   #
   # - Bits 28-31 = Reserved. Set to 0.
   #                                              
   #-------------------------------------------------------------------------

   addis    r3,0,0x100c
   stw      r3,CYGARC_REG_IMM_BCR(r4)

   #-------------------------------------------------------------------------
   # Program the 60x Bus Arbiter Configuration Register. The details are as
   # follows:
   #
   # - Bits 0-1 = Reserved. Set to 0. 
   #
   # - DBGD (Data Bus Grant Delay) = Minimum of zero wait states for PowerPC
   #                                 master-initiated data operations. This 
   #                                 is the minimum delay between TS/ and 
   #                                 DBG/.
   #
   # - Bits 3 = Reserved. Set to 0.
   #
   # - PRKM (Parking Master) = 0010 = CPM is at a low request level for the 
   #                                  parked master.
   #
   #-------------------------------------------------------------------------

   addi     r3,0,0x0002
   stb      r3,CYGARC_REG_IMM_PPC_ACR(r4)

   
   #-------------------------------------------------------------------------
   # Program the 60x Bus Arbitration-Level[High] Register. The priority for 
   # potential bus masters are defined by locating the value associated with
   # a particular master from the PRKM field in the PPC_ACR register and 
   # inserting its value in the priority field. Priority field 0 is the
   # highest priority and the lowest is Priority field 15 in the PPC_ALRH
   # register.
   #
   # - Priority Field 0 = 0000 = CPM high request level
   #
   # - Priority Field 1 = 0001 = CPM middle request level
   #
   # - Priority Field 2 = 0010 = CPM low request level
   #
   # - Priority Field 3 = 0110 = Internal Core
   #
   # - Priority Field 4 = 0111 = External Master 1
   #
   # - Priority Field 5 = 1000 = External Master 2
   #
   # - Priority Field 6 = 1001 = External Master 3
   #
   # - Priority Field 7 = 0011 = Reserved
   #
   #-------------------------------------------------------------------------

   addis    r3,0,0x0126
   ori      r3,r3,0x7893
   stw      r3,CYGARC_REG_IMM_PPC_ALRH(r4)


   #------------------------------------------------------------------------
   #
   # First program the SIUMCR. The details are as follows:
   #
   # - BBD (Bus Busy Disable) = 0 = [ABB/]/[IRQ2] pin is ABB/ and [DBB/]/IRQ2 
   #                                pin is DBB/.
   #
   # - ESE (External Snoop Enable = 0 = [GPL/]/[IRQ1/] pin is IRQ1/
   #
   # - PBSE (Parity Byte Select Enable) = 0 = Parity byte select is disabled.
   #
   # - CDIS (Core DISable) = 0 = The PowerQUICCII CPU core is enabled.
   #
   # - DPPC (Data Parity Pins Configuration) = 10 = 
   #
   #    Gives the following pin meanings:   DP(0)|RSRV/|EXT_BR2/  = RSRV/
   #                                        DP(1)|IRQ1/|EXT_BG2/  = IRQ1/
   #                                        DP(2)|TLBISYNC/|IRQ2/ = TLBISYNC/
   #                                        DP(3)|IRQ3/           = IRQ3/    
   #                                        DP(4)|IRQ4/           = IRQ4/    
   #                                        DP(5)|TBEN/|IRQ5      = TBEN/
   #                                        DP(6)|CSE(0)|IRQ6/    = CSE(0)
   #                                        DP(7)|CSE(1)|IRQ7/    = CSE(1)
   #
   # - L2CPC (L2 Cache Pins Configuration) = 00 =
   #
   #    Gives the following pin meanings:   CI/|BADDR(29)|IRQ2/ = CI/ 
   #                                        WT/|BADDR(30)|IRQ3/ = WT/ 
   #                                        L2_HIT/|IRQ4        = L2_HIT/ 
   #                                                                     
   #                                        CPU_BG/|BADDR(31)    
   #                                        |IRQ5/              = CPU_BG/ 
   #
   # - LBPC (Local Bus Pins Configuration) = 00 = 
   #
   #    Local Bus pins function as local bus.
   #
   # - APPC (Address Parity Pins Configuration) = 11 = 
   #
   #    Gives the following pin meanings: MODCK1|AP(1)|TC(0)  = -
   #                                      MODCK2|AP(2)|TC(1)  = -
   #                                      MODCK3|AP(3)|TC(2)  = -
   #                                      IRQ7/|APE/|INT_OUT/ = IRQ7/+INT_OUT/
   #                                      CS11/|AP(0)         = -
   #
   # - CS10PC (Chip Select 10-pin Configuration) = 00 = 
   #
   #    CS10/|BCTL1/|DBG_DIS/ = CS10/
   #
   # - BCTLC (Buffer Control Configuration) = 00 = 
   #
   #    BCTL0 pin is used as W|R/ control. BCTL1 is used as OE/ control if 
   #    BCTL1 was connected to its alternate pin.
   #
   # - MMR (Mask Masters Requests) = 00 = No masking on bus request lines.
   #
   # - LPBSE (Local Bus Parity Byte Select Enable) = 0
   # 
   #    Parity byte select is disabled.
   #
   # Bits 19-31 are reserved and set to 0.
   #
   #------------------------------------------------------------------------
          
   #--------------------------------------------------------------------------
   # Program the 60x Bus Transfer Error Status and Control Register 1. The
   # details are as follows:
   #
   # - DMD (Data Errors Disable) = 1 = 
   #
   #     Disable all data errors on the 60x bus. Also parity single and double
   #     ECC error.
   #
   # - All other bits are either status or reserved bits. All reserved bits 
   #   should be set to 0.
   #--------------------------------------------------------------------------

   addis    r3,0,0x0000
   ori      r3,r3,0x4000
   stw      r3,CYGARC_REG_IMM_TESCR1(r4)

   #--------------------------------------------------------------------------
   # Program the Local Bus Transfer Error Status and Control Register 1. The
   # details are as follows:
   #
   # - DMD (Data Errors Disable) = 1 = 
   #
   #     Disable parity errors on the Local bus. 
   #
   # - All other bits are either status or reserved bits. All reserved bits 
   #   should be set to 0.
   #--------------------------------------------------------------------------

   addis    r3,0,0x0E30
   ori      r3,r3,0x0000
   stw      r3,CYGARC_REG_IMM_SIUMCR(r4)

   # FIXME - What is this programming from??????
   addis    r3,0,0x0000
   ori      r3,r3,0x4000
   stw      r3,CYGARC_REG_IMM_LTESCR1(r4)


   #-------------------------------------------------------------------------
   # First, program the Memory Periodic Timer Prescaler Register (MPTPR).
   # Finding the value to this ties in with the desired SDRAM Refresh
   # Timer (PSRT) value and the required Refresh Command interval for 
   # refreshing each row. The Refresh Command interval is found by 
   # determining the number of rows on the SDRAM device. In this case
   # its 4096 (12 bits to address rows). The "Refresh Period" value in the
   # AC characteristic section of the data sheet is 64 msec. This value
   # is divided by the number of rows to give the number of Refresh
   # commands that needs to be sent in a 64 msec interval. This value
   # is 64 msec/4096 = 15.625 usec.
   # Due to probable contention from time to time with other memory 
   # controller bus requests, lets make the refresh command interval to be
   # around 15.5 usec.
   #
   # Using the calculation found in the MPC8260 User Manual errata, the
   # timer period is 
   #
   #        Timer Period = (PSRT + 1) * (MPTPR[PTP] + 1)
   #                       ----------------------------
   #                              Bus Frequency
   #
   #        - Set PSRT       = 16
   #        - Set MPTPR[PTP] = 40
   #        - Use 45 MHz as Bus Frequency
   #          
   #
   #        - Timer Period = 15.489 usec
   #
   #
   # The clock distribution block diagram looks like this:
   #
   #   ----------      -------------         -----------    -------------
   #   | Clock   |     |   MPT     |         | SDRAM   |    | Refresh   |
   #   | Gen.    |-----| Prescaler |---------| Refresh |----| Command   |
   #   | 45 Mhz  |     -------------         | Timer   |    | Logic in  |
   #   | on Ts6 |                           -----------    | Mem.Cont. |
   #   | Board   |                                          -------------
   #   -----------
   #      
   #-------------------------------------------------------------------------

#   addi     r5,0,0x4000      # load 0x40 or 64 into the PTP field of MPTPR
   addi     r5,0,0x2800      # load 0x28 or 40 into the PTP field of MPTPR
   sth      r5,CYGARC_REG_IMM_MPTPR(r4)     # store half word - bits[16-31]



   #-----------------------
   # return from init_siu
   #-----------------------

   bclr  20,0           # jump unconditionally to effective address in Link
                        # register


#########################################################################
# Function: init_memc
#
# Description: 
#
#  The following registers directly control the memory controllers 
#  operation:
#
#  BR0-BR11 - Base Register Banks 0-11
#  OR0-OR11 - Option Register Banks 0-11
#  PSDMR    - 60x bus SDRAM machine mode register
#  LSDMR    - Local bus SDRAM machine mode register
#  MAMR     - UPMA mode register
#  MBMR     - UPMB mode register
#  MCMR     - UPMC mode register
#  MDR      - Memory data register
#  MAR      - Memory address register
#  MPTPR    - Memory periodic timer pre-scaler register
#  PURT     - 60x bus assigned UPM refresh timer
#  PSRT     - 60x bus assigned SDRAM refresh timer
#  LURT     - Local Bus assigned UPM refresh timer
#  LSRT     - Local Bus assigned SDRAM refresh timer
#
#  This example will program the following registers. The rest will remain at
#  their default values.
#
#  BR0      - Base Register for Flash Memory
#  OR0      - Option Register for Flash Memory
#  BR1      - Base Register for BCSR (Board Control and Status Registers)
#  OR1      - Option Register for BCSR
#  BR2      - Base Register for 60x SDRAM
#  OR2      - Option Register for 60x SDRAM
#  BR3      - Base Register for 60x Local Bus SDRAM
#  OR3      - Option Register for 60x Local Bus SDRAM
#  PSDMR    - 60x bus SDRAM machine mode register
#  LSDMR    - Local bus SDRAM machine mode register
#  MPTPR    - Memory periodic timer pre-scaler register
#  PSRT     - 60x bus assigned SDRAM refresh timer
#  LSRT     - Local Bus assigned SDRAM refresh timer
#              
#
#  History:
#
#########################################################################        

init_memc:

   mfspr    r30,CYGARC_REG_LR        # Save the Link Register value. The link registers
                          # value will be restored so that this function 
                          # can return to the calling address.

   bl    init_flash       # 8 Mbyte of flash memory

   bl    init_fpga_mem    # 64 KByte memory map for FPGA access

   bl    init_cluster_bus # 256 MByte Memory mapped for Tiger Sharc cluster bus

#ifdef MORE_FLASH
#endif
#if 0 // The Delphi TS6 Digital Board does not have BCSR
   bl    init_bcsr         # Board Control and Status Registers
#endif

   bl    init_60x_sdram    # Main 60x Bus SDRAM

   #-----------------------
   # return from init_memc
   #-----------------------

   mtspr    CYGARC_REG_LR,r30      # restore original Link Register value 

   bclr  20,0           # jump unconditionally to effective address in Link
                        # register



############################################################################
# Function: init_flash
#
# Description: This function programs Base Register 0 and Option Register 0
#              designating bank 0 for the 8Mbyte flash SIMM on the TS6
#              board. Programming these two registers describes how the 
#              MPC8260 will inter-operate with this memory space and thus 
#              this memory device.
#
#  History:
#
############################################################################        

init_flash:

   #-------------------------------------------------------------------------
   # Base Register 0 (BR0): Bank 0 is assigned to the 8Mbyte (2M X 32) 
   #                        flash that resides on the MPC8260 TS6 board. 
   #                        The particulars are defined here. 
   #
   # BA (Base Address) = 0xFF80+0b for a total of 17 address bits. This value
   #                     represents the upper 17 bits of the base address.
   #
   # Bits 17-18 reserved. = 00
   #
   # PS (Port Size) = 11b = 32 bit port size
   #
   # DECC (Data Error Correction and Checking) = 00 = Data errors checking
   #                                                  Disabled.
   #
   # WP (Write Protect) = 0 = both read and write accesses are allowed
   #
   # MS (Machine Select) = 000 = General Purpose Chip Select Machine (GPCM)
   #                             for 60x bus Selected
   #
   # EMEMC (External Memory Controller Enable) = 0 = Accesses are handled by
   #                                                 the memory controller
   #                                                 according to MSEL.
   #
   # ATOM (Atomic Operation) = 00 = The address space controlled by the 
   #                                memory controller bank is not used for
   #                                atomic operations.
   #
   # DR (Delayed Read) = 0 = Normal operation.
   #
   # V (Valid Bit) =  1 = Valid bit set
   #-------------------------------------------------------------------------
   addis    r3,0,0xFF80   # R3 holds the value temporarily
   ori      r3,r3,0x1801
   #-------------------------------------------------------------------------
   # Option Register 0 (OR0) for GPCM use: further flash definitions
   #
   # AM (Address Mask) = 0xFF80+0b = We have masked the upper 9 bits which 
   #                                 defines a 8 Mbyte memory block.
   #
   # Bits 17-19 Reserved - set to 000.
   #
   # CSNT (Chip Select Negation Time) = 1 = CS/|WE/ are negated a quarter
   #                                        of a clock early.
   #
   # ACS (Address To Chip-Select Setup) = 00 = CS/ is output the same time as
   #                                           the addr lines.
   #
   # Bit 23 Reserved - set to 0.
   #
   # SCY (Cycle Length In Clocks) = 0011 = Add a 3 clock cycle wait state
   #
   # SETA (External Transfer Acknowledge) = 0 = PSDVAL/ is generated 
   #                                            internally by the memory 
   #                                            controller unless GTA/ is 
   #                                            asserted earlier externally.
   #
   # TRLX (Timing Relaxed) = 1 = Relaxed timing is generated by the GPCM.
   #
   # EHTR (Extended Hold Time On Read Accesses) = 1 = 
   #
   #     Extended hold time is generated by the memory controller. An idle 
   #     clock cycle is inserted between a read access from the current bank
   #     and any write or read access to a different bank.
   #
   # Bit 31 Reserved - set to 0.
   #-------------------------------------------------------------------------

   addis    r5,0,0xFF80     # R5 holds the value temporarily
   ori      r5,r5,0x0836   

   #------------------------------------------------------------------------
   # It is important to note the order in which OR0 and BR0 are programmed.
   # When coming out of reset and CS0 is the global chip select, OR0 MUST be 
   # programmed AFTER BR0. In all other cases BRx would be programmed after
   # ORx.
   #------------------------------------------------------------------------

   #------------------
   # Write the values
   #------------------

   stw        r3,CYGARC_REG_IMM_BR0(r4)
   stw        r5,CYGARC_REG_IMM_OR0(r4)


   bclr  20,0           # jump unconditionally to effective address in Link
                        # register


#########################################################################
# Function: init_bcsr
#
# Description: This function programs Base Register 1 and Option Register 1
#              designating bank 1 for BCSR0, BCSR1, and BCSR2 on the TS6
#              board. BCSR stands for Board Control and Status Register.
#              This space is treated as general I/O. Programming the 
#              following 2 registers describes how the MPC8260 will inter-
#              operate with this memory space.
#
#  History:
#
#  Jan 9/99    jay      
#
#########################################################################        

init_bcsr:


   #-------------------------------------------------------------------------
   # Base Register 1 (BR1): Bank 1 is assigned to the Board Control and
   #                        Status Registers (BCSRs). There are 3 
   #                        that resides on the MPC8260 TS6 board. 
   #                        The particulars are defined here. 
   #
   # BA (Base Address) = 0x0450+0b for a total of 17 address bits. This value
   #                     represents the upper 17 bits of the base address.
   #
   # Bits 17-18 reserved. = 00
   #
   # PS (Port Size) = 11b = 32 bit port size
   #
   # DECC (Data Error Correction and Checking) = 00 = Data errors checking
   #                                                  Disabled.
   #
   # WP (Write Protect) = 0 = both read and write accesses are allowed
   #
   # MS (Machine Select) = 000 = General Purpose Chip Select Machine (GPCM)
   #                             for 60x bus Selected
   #
   # EMEMC (External Memory Controller Enable) = 0 = Accesses are handled by
   #                                                 the memory controller
   #                                                 according to MSEL.
   #
   # ATOM (Atomic Operation) = 00 = The address space controlled by the 
   #                                memory controller bank is not used for
   #                                atomic operations.
   #
   # DR (Delayed Read) = 0 = Normal operation.
   #
   # V (Valid Bit) =  1 = Valid bit set
   #-------------------------------------------------------------------------

   addis    r3,0,0x0450     # R3 holds the value temporarily
   ori      r3,r3,0x1801

   #-------------------------------------------------------------------------
   # Option Register 1 (OR1) for GPCM use: further BCSR definitions
   #
   # AM (Address Mask) = 0xFFFF +1b = We have masked the upper 17 bits which 
   #                                  which defines a 32 Kbyte memory block.
   #
   # Bits 17-19 Reserved - set to 000.
   #
   # CSNT (Chip Select Negation Time) = 0 = CS/|WE/ are negated normally.
   #
   # ACS (Address To Chip-Select Setup) = 00 = CS/ is output at the same 
   #                                           time as the addr lines.
   #
   # Bit 23 Reserved - set to 0.
   #
   # SCY (Cycle Length In Clocks) = 0001 = Add a 1 clock cycle wait state
   #
   # SETA (External Transfer Acknowledge) = 0 = PSDVAL/ is generated 
   #                                            internally by the memory 
   #                                            controller unless GTA/ is 
   #                                            asserted earlier externally.
   #
   # TRLX (Timing Relaxed) = 0 = Normal timing is generated by the GPCM.
   #
   # EHTR (Extended Hold Time On Read Accesses) = 0 = Normal timing is 
   #                                                  generated by the memory
   #                                                  controller
   #
   # Bit 31 Reserved - set to 0.
   #-------------------------------------------------------------------------

   addis    r5,0,0xFFFF     # R5 holds the value temporarily
   ori      r5,r5,0x8010

   #------------------
   # Write the values
   #------------------

   stw        r5,CYGARC_REG_IMM_OR1(r4)
   stw        r3,CYGARC_REG_IMM_BR1(r4)


   bclr  20,0    # jump unconditionally to effective address in Link
                 # register


#########################################################################
# Function: init_fpga_mem
#
# Description: This function programs Base Register 4 and Option Register 4
#              designating bank 4 for the FPGA control space on the TS6
#              board. 
#
#  History:
#
#
#########################################################################        

init_fpga_mem:


   #-------------------------------------------------------------------------
   # Base Register 4 (BR4): Bank 4 is assigned to FPGA control memory map.
   #
   # BA (Base Address) = 0x0450+0b for a total of 17 address bits. This value
   #                     represents the upper 17 bits of the base address.
   #
   # Bits 17-18 reserved. = 00
   #
   # PS (Port Size) = 11b = 32 bit port size
   #
   # DECC (Data Error Correction and Checking) = 00 = Data errors checking
   #                                                  Disabled.
   #
   # WP (Write Protect) = 0 = both read and write accesses are allowed
   #
   # MS (Machine Select) = 000 = General Purpose Chip Select Machine (GPCM)
   #                             for 60x bus Selected
   #
   # EMEMC (External Memory Controller Enable) = 0 = Accesses are handled by
   #                                                 the memory controller
   #                                                 according to MSEL.
   #
   # ATOM (Atomic Operation) = 00 = The address space controlled by the 
   #                                memory controller bank is not used for
   #                                atomic operations.
   #
   # DR (Delayed Read) = 0 = Normal operation.
   #
   # V (Valid Bit) =  1 = Valid bit set
   #-------------------------------------------------------------------------

   addis    r3,0,0x0450     # R3 holds the value temporarily
   ori      r3,r3,0x1801

   #-------------------------------------------------------------------------
   # Option Register 1 (OR1) for GPCM use: further BCSR definitions
   #
   # AM (Address Mask) = 0xFFFF +0b = We have masked the upper 16 bits which 
   #                                  which defines a 64 Kbyte memory block.
   #
   # Bits 17-19 Reserved - set to 000.
   #
   # CSNT (Chip Select Negation Time) = 1 = CS/|WE/ are negated a quarter cycle
   #                                        early.  For now, put in all delay
   #                                        possible.
   #
   # ACS (Address To Chip-Select Setup) = 11 = CS/ is output one half clock 
   #                                           after the addr lines.
   #
   # Bit 23 Reserved - set to 0.
   #
   # SCY (Cycle Length In Clocks) = 0000 = The SETA bit is set, therefore
   #                                       SCY is not used.
   #
   # SETA (External Transfer Acknowledge) = 1 = PSDVAL/ is generated 
   #                                            by external logic, is this
   #                                            case, the FPGA.
   #
   # TRLX (Timing Relaxed) = 1 = Relaxed timing is generated by the GPCM.
   #
   # EHTR (Extended Hold Time On Read Accesses) = 1 = Eight idle cycle inserted
   #                                                  between a read access from
   #                                                  the current bank and
   #                                                  next access.
   #
   # Bit 31 Reserved - set to 0.
   #-------------------------------------------------------------------------

   addis    r5,0,0xFFFF     # R5 holds the value temporarily
   ori      r5,r5,0x0E0E

   #------------------
   # Write the values
   #------------------

   stw        r5,CYGARC_REG_IMM_OR4(r4)
   stw        r3,CYGARC_REG_IMM_BR4(r4)


   bclr  20,0    # jump unconditionally to effective address in Link
                 # register

#########################################################################
# Function: init_cluster_bus
#
# Description: This function programs Base Register 8 and Option Register 8
#              designating bank 8 for the Cluster Bus space on the TS6
#              board. 
#
#  History:
#
#
#########################################################################        

init_cluster_bus:


   #-------------------------------------------------------------------------
   # Base Register 8 (BR8): Bank 8 is assigned to Cluster Bus  memory map.
   #
   # BA (Base Address) = 0xe000+0b for a total of 17 address bits. This value
   #                     represents the upper 17 bits of the base address.
   #
   # Bits 17-18 reserved. = 00
   #
   # PS (Port Size) = 00b = 64 bit port size (use 64 bit port size)
   # PS (Port Size) = 11b = 32 bit port size
   #
   # DECC (Data Error Correction and Checking) = 00 = Data errors checking
   #                                                  Disabled.
   #
   # WP (Write Protect) = 0 = both read and write accesses are allowed
   #
   # MS (Machine Select) = 000 = General Purpose Chip Select Machine (GPCM)
   #                             for 60x bus Selected
   #
   # EMEMC (External Memory Controller Enable) = 0 = Accesses are handled by
   #                                                 the memory controller
   #                                                 according to MSEL.
   #
   # ATOM (Atomic Operation) = 00 = The address space controlled by the 
   #                                memory controller bank is not used for
   #                                atomic operations.
   #
   # DR (Delayed Read) = 0 = Normal operation.
   #
   # V (Valid Bit) =  1 = Valid bit set
   #-------------------------------------------------------------------------

   addis    r3,0,0xE000     # R3 holds the value temporarily
   ori      r3,r3,0x0001

   #-------------------------------------------------------------------------
   # Option Register 8 (OR8) for GPCM use: further BCSR definitions
   #
   # AM (Address Mask) = 0xF000 +0b = We have masked the upper 4 bits which 
   #                                  which defines a 256 Mbyte memory block.
   #
   # Bits 17-19 Reserved - set to 000.
   #
   # CSNT (Chip Select Negation Time) = 1 = CS/|WE/ are negated a quarter cycle
   #                                        early.  For now, put in all delay
   #                                        possible.
   #
   # ACS (Address To Chip-Select Setup) = 11 = CS/ is output one half clock 
   #                                           after the addr lines.
   #
   # Bit 23 Reserved - set to 0.
   #
   # SCY (Cycle Length In Clocks) = 0000 = The SETA bit is set, therefore
   #                                       SCY is not used.
   #
   # SETA (External Transfer Acknowledge) = 1 = PSDVAL/ is generated 
   #                                            by external logic, is this
   #                                            case, the FPGA.
   #
   # TRLX (Timing Relaxed) = 1 = Relaxed timing is generated by the GPCM.
   #
   # EHTR (Extended Hold Time On Read Accesses) = 1 = Eight idle cycle inserted
   #                                                  between a read access from
   #                                                  the current bank and
   #                                                  next access.
   #
   # Bit 31 Reserved - set to 0.
   #-------------------------------------------------------------------------

   addis    r5,0,0xF000     # R5 holds the value temporarily
   ori      r5,r5,0x0E0E

   #------------------
   # Write the values
   #------------------

   stw        r5,CYGARC_REG_IMM_OR8(r4)
   stw        r3,CYGARC_REG_IMM_BR8(r4)


   bclr  20,0    # jump unconditionally to effective address in Link
                 # register



#########################################################################
# Function: init_60x_sdram
#
# Description: This function programs the 64 Mbyte SDRAM DIMM on the TS6 
#              board. This memory functions as the RAM for the 603 core and
#              is connected to the 60x bus. Base and Option Register 5 are 
#              used as well as the SDRAM Machine #1 and Chip Select #5.
#
#  History:
#
#  Apr /10/02   pfine    Modified so that all 64 MBytes are addressable.
#
#  Feb /27/02   pfine    Modified the Motorola code to work for the TS6
#                        board.
#  Jan /9/99    jay      Initial Release
#
#########################################################################        

init_60x_sdram:


   # The Delphi MPC8260 TigerSHARC-6 Board is stocked with a 64 MByte
   # SDRAM DIMM module (WINTEC W9Q308647PA-222H).  The DIMM module consists
   # of 4 Micron Technology 128 Mbit SDRAM modules, each having 12 row
   # by 9 columns by 4 banks.  The part number for the Micron SDRAM
   # is 48LC8M16A2.  The Refresh Period is 64 milliseconds for all 4096
   # rows, or 15.625 usec per row.
   #
   # NOTE - The spec sheet for the WINTEC DIMM claims that the device
   #        has 12/11/2 rows/columns/banks, but the spec sheet for the Micron
   #        SDRAM chips says 12/9/4.  The 4 Banks seems to work.

   # The TS6 board has the 60x Bus SDRAM connected to CS5, so use BR5, OR5.
	
   #-------------------------------------------------------------------------
   # Program the 60x Bus Assigned SDRAM Refresh Timer (PSRT).
   # See SIU initialization programming for details of why 16 is the
   # value for the PSRT.
   #-------------------------------------------------------------------------

   addi     r5,0,0x0010      # load 0x10 or 16 
   stb      r5,CYGARC_REG_IMM_PSRT(r4)      # store byte - bits[24-31]


   #########################
   # Program Bank Registers
   #########################


   #-------------------------------------------------------------------------
   # Base Register 5 (BR5): Bank 5 is assigned to the 64 Mbyte 60x SDRAM DIMM
   #                        that resides on the MPC8260 TS6 board. The 
   #                        particulars are defined here. 
   #
   # - BA (Base Address) = 0x0000+0b for a total of 17 address bits. This 
   #                       value represents the upper 17 bits of the base 
   #                       address.
   #
   # - Bits 17-18 reserved. = 00 = cleared to 0.
   #
   # - PS (Port Size) = 00b = 64 bit port size
   #
   # - DECC (Data Error Correction and Checking) = 00 = Data errors checking
   #                                                    Disabled.
   #
   # - WP (Write Protect) = 0 = both read and write accesses are allowed
   #
   # - MS (Machine Select) = 010 = SDRAM Machine for 60x bus Selected
   #
   # - EMEMC (External Memory Controller Enable) = 0 = 
   #
   #    Accesses are handled by the memory controller according to MSEL.
   #
   # - ATOM (Atomic Operation) = 00 = The address space controlled by the 
   #                                  memory controller bank is not used for
   #                                  atomic operations.
   #
   # - DR (Delayed Read) = 0 = Normal operation.
   #
   # - V (Valid Bit) =  1 = Valid bit set
   #-------------------------------------------------------------------------

   addis    r3,0,0x0000     # R3 holds the value temporarily
   ori      r3,r3,0x0041

   #-------------------------------------------------------------------------
   # Option Register 5 (OR5) for SDRAM Machine use: further SDRAM definitions
   #
   # - SDAM (SDRAM Address Mask) = 1111 1100 0000b = 0xFC0
   #      As per the errata to the MPC8260 Users Manual, the SDAM value
   #      is a 12 bit field (0-11).  The 64 MByte block is specified.
   #
   # - LSDAM (Lower SDRAM Address Mask) = 00000b = Minimum of 1 Mbyte size.
   #
   # - BPD (Banks Per Device) = 01b = 4 internal banks per device.
   #
   #
   #
   # -- The SDRAM is set up to operate in Page Based Interleaving Mode.
   #
   # - ROWST (Row Start Address Bit) = 0110b = A6 being the row start address 
   #                                           bit.  See the PSDMR programming
   #                                           explanation below for details
   #                                           of how A6 was determined to be
   #                                           the starting row.
   #       As per the errata to the MPC8260 Users Manual, the ROWST field
   #       bits 19-22.
   #
   # - NUMR (Number of Row Address Lines) = 011b = 12 row address lines.
   #
   # - PMSEL (Page Mode Select) = 1
   #
   # - IBID (Internal bank interleaving within same device disable) = 1
   # 
   #    As per MPC8260 Users Manual, if the SDRAM device is not connected
   #    to the BNKSEL pins, then this bit should be set.  The BNKSEL pins
   #    are not connected on the TS6 platform.
   #
   # - Bits 28-31 Reserved - set to 0.
   #-------------------------------------------------------------------------

   addis    r5,0,0xFC00     # R5 holds the value temporarily
   ori      r5,r5,0x2CF0

   #------------------
   # Write the values
   #------------------
   stw        r5,CYGARC_REG_IMM_OR5(r4)
   stw        r3,CYGARC_REG_IMM_BR5(r4)

   ###########################################
   # Perform Initialization sequence to SDRAM
   ###########################################

   #-------------------------------------------------------------------------
   # Program the PowerPC SDRAM Mode Registr (PSDMR). This register is used
   # to configure operations pertaining to SDRAM. Program the PSDMR, turning 
   # off refresh services and changing the SDRAM operation to "Precharge all
   # banks". Then do a single write to an arbitrary location. Writing 0xFF 
   # to address 0 will do the trick.
   #
   # - PBI (Page Based Interleaving) = 1
   #
   # - RFEN (Refresh Enable) = 0 = Refresh services not required. This bit 
   #                               will be set later in this function as a 
   #                               last step.
   # 
   # - OP (SDRAM Operation) = xxx  Precharge all banks 101b
   #                               CBR Refresh         001b
   #                               Mode register write 011b
   #                               Normal operation    000b
   #
   # - SDAM (Address Multiplex Size = 011 = 
   #
   #   FIXME - This explanation needs to updated.
   #   The 64 MByte SDRAM DIMM used on the TS6 platform has only 12 address
   #   pins and 2 bank select pins.  The MPC8260 SDRAM memory controller 
   #   must multiplex the 23 MPC8260 address bits (64 MBytes = 2^26, but 
   #   the 3 lsbs are ignored due to the 64 bit depth of the SDRAM) onto 
   #   the available SDRAM pins.  
	
   #   The following explanation will explain (hopefully) how the 23 bits
   #   of the effective address (as seen from the MPC8260) will be remapped
   #   onto the physical pins of the SDRAM device.  It is expected that
   #   the MPC8260 Users Manual (Chapter 10) is necessary to understand
   #   this explanation.
   #
   #   Page Based Interleaving is chosen as the address multiplexing method.
   #   The SDRAM device architecture is 12 rows, 9 columns, 4 banks.
   #
   #   The effective address space as seen from the MPC8260 is partitioned 
   #   as follows to map to the architecture of the SDRAM device.
   #
   #      addresses ignored by SDRAM : A29 A30 A31
   #
   #      column addresses: A20 A21 A22 A23 A24 A25 A26 A27 A28
   #
   #      bank select addresses:  A18 A19
   #
   #      row addresses:  A6 A7 A8 A9 A10 A11 A12 A13 A14 A15 A16 A17
   #
   #   These address bits correspond to the effective address which the MPC8260
   #   is attempting to access.  The MPC8260 memory controller, through
   #   the settings of the BR5/OR5 and PSDMR registers, will remap these 
   #   effective addresses bits onto the physical address pins of the SDRAM
   #   DIMM.
   #
   #  -1) The SDRAM has a data width of 64 bits, which means that each
   #      read or write consists of 8 8-bit bytes aligned on an 8 byte
   #      boundary.  Therefore, the lowest three address bits (A29 A30 A31)
   #      are always ignored, and not physically connected to the SDRAM.
   #
   #  -2) The 9 column address bits select one column of 512 for
   #      SDRAM read or write.  These bits are mapped directly to the
   #      physical pins on the SDRAM DIMM.
   #
   #  -3) In Page Based Interleaving mode (PSDMR[PBI] = 1) the bank select
   #      lines are assigned the address lines between the column addresses
   #      and the row addresses.  By setting OR5[BPD] = 01, the memory
   #      controller knows that there are 4 banks, and therefore 2 bank
   #      select lines (A18 A19).  The TS6 platform has MPC8260 address
   #      lines A14 and A15 connected to the two bank select pins on the
   #      DIMM, and therefore PSDMR[BSMA] = 001.
   #      
   #      PSDMR[PBI] = 1
   #      OR5[BPD] = 01
   #      
   #      To clarify, selecting PSDMR[BSMA] = 001 tells the memory
   #      controller to put out the bits corresponding to the bank select
   #      address bits on the address lines named A14 and A15.
   #      Those bank select lines correspond to effective address bits
   #      A18 and A19.  So the effective address bits A18 and A19 actually
   #      are put out onto the A14 and A15 address lines, which are, in
   #      turn, connected to BA1 and BA0 on the SDRAM DIMM.
   #      
   #      PSDMR[BSMA] = 001
   #      
   #  -4) The remaining 12 address bits are the row addresses. Simple
   #      subtraction says that the starting row address bit 
   #      (OR5[ROWST]) is A6 (3 bits not connected, 9 column bits, 2
   #      bank select bits, 12 row bits).
   #
   #      OR5[NUMR]  = 011b  ==> 12 rows
   #      OR5[ROWST] = 0110b ==> A6 is row start address bit
   #
   #  -5) The memory controller must be told how to multiplex the row
   #      address bits onto the physical pins of the SDRAM device.  This
   #      is accomplished through the PSDMR[SDAM] field.
   #      Lining up the column and row addresses so that the
   #      lsb of the row addresses lines up with the lsb of the column
   #      addresses gives the following arrangemnt:	
   #
   #      column addresses:          A20 A21 A22 A23 A24 A25 A26 A27 A28
   #      row addresses:  A6  A7  A8  A9 A10 A11 A12 A13 A14 A15 A16 A17
   #
   #      The MPC8260 Users Manual, Table 10-20 shows that PSDMR[SDAM] = 011b
   #      provides the proper alignment for this multiplexing scheme.
   #
   #      PSDMR[SDAM] = 011b
   #
   #  -6) The final complication in the SDRAM configuration is the SDA10
   #      control.  SDRAM is accessed in a two stage process.  In the
   #      first stage, the row address and bank selects are loaded to the
   #      SDRAM device.  In the second stage, the column address is loaded.
   #      SDRAM devices multiplex the A10/AP pin for two purposes.  In the
   #      first stage, A10/AP is an address pin.  In the second stage, the
   #      pin is used to determine how precharging is done (this 
   #      explanation will not go into this aspect).  Because the A10/AP
   #      pin is special, the pin is connected physically to a special
   #      pin on the MPC8260, the PSDA10 pin.  The memory controller must
   #      be told how which address bit is being multiplexed on PSDA10.
   #      
   #      The row address bit which is multplexed onto the pin between 
   #      A9 and A11 physical pins on the SDRAM device is this special bit.
   #      Lining up the physical pins and the MPC8260 address lines (from
   #      sheet 15 of the TS6 platform schematics) and the line of Table 20
   #      corresponding to SDAM = 011b
   #      
   #      SDRAM physical pins:  A11 A10  A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
   #      MPC8260 address lines:A17 A18 A19 A20 A21 A22 A23 A24 A25 A26 A27 A28
   #      row address:           A6  A7  A8  A9 A10 A11 A12 A13 A14 A15 A16 A17
   #      
   #      It is seen that the row address bit which will appear on the A10
   #      physical pin is A7 and therefore
   #   
   #      PSDMR[SDA10] = 011b ==> Address bit A7 (for PBI = 1)
   #   
   # - BSMA (Bank Select Multiplexed Address line) = 001 = A13-A15
   #
   # - SDA10 ("A10 Control") = 011 = 
   #
   # FIXME - The following timing values are not optimized.  They are the
   #         same values as were used in the MPC8260 TS6 board, with
   #         the 16 MByte DIMM and a 66 MHz Bus Frequency.
   #
   # - RFRC (ReFresh ReCovery) = 101 = 
   #
   #   Once a  refresh request is granted, the memory controller begins 
   #   issuing auto-refresh commands to each device associated with the 
   #   refresh timer in one clock intervals. After the last REFRESH 
   #   command is issued, the memory controller waits for 7 clocks before 
   #   the ACTIVATE command and before normal read/write operations can 
   #   resume.
   #
   # - PRETOACT (PREcharge TO ACTivate interval) = 011 = 
   #
   #   Wait 3 clock cycles before an activate command. This parameter is
   #   determined by the requirements of the SDRAM at a particular clock 
   #   speed.
   #
   # - ACTTORW (ACTivate TO Read/Write interval) = 010 = 2 clock cycles.
   # 
   # - BL (Burst Length) = 0 = SDRAM burst length is 4. This is programmed 
   #                          into the SDRAM via the MRS commmand.
   #
   # - LDOTOPRE (Last Data Out TO PREcharge) = 01 = 
   #
   #   The delay required from the last data out to precharge state is 1 clock
   #   cycle. This parameter is determined by the requirements of the SDRAM at
   #   a particular clock speed.
   #                               
   # - WRC (Write Recovery Time) = 01 = 
   #
   #   Time needed to elapse before another operation is 1 clock cycle. This 
   #   parameter is determined by the requirements of the SDRAM at a 
   #   particular clock speed.
   #
   # - EAMUX (External Address MUltipleXing) = 0 No external address
   #                                             multiplexing.
   #
   # - BUFCMD (Buffer Control Lines) = 0 = Normal timing for the control
   #                                       lines.
   #
   # - CL (CAS Latency) = 10 = 
   #
   #    CAS latency is 2. Two cycles after column address is registered, data
   #    is valid. This parameter is determined by the requirements of the 
   #    SDRAM at a particular clock speed.
   #
   #-------------------------------------------------------------------------

   ##################
   # Precharge all banks
   ##################

   # Clear refresh enable bit (bit 1 = 0)
   # Set OP mode to Precharge (bits 2-4 = 101b)

   addis    r3,0,0x8B2E
   ori      r3,r3,0xB452
	
   stw      r3,CYGARC_REG_IMM_PSDMR(r4)
   addis    r0,0,0

   addi     r3,0,0x00FF      # Load 0x000000FF into r3
   stb      r3,0(r0)         # Write 0xFF to address 0 - bits [24-31]
   

   #-------------------------------------------------------------------------
   # Program the PSDMR keeping refresh services off and changing the
   # SDRAM operation to "CBR Refresh". This step is responsible for issuing
   # a minimum of 8 auto-refresh commands. This is done by the SDRAM machine
   # by issuing the CBR Refresh command by programming the OP field of the 
   # PSDMR register and writing 0xFF 8 times to an arbitrary address.
   #-------------------------------------------------------------------------
   
   # Clear refresh enable bit (bit 1 = 0)
   # Set OP mode to CBR Refresh (bits 2-4 = 001b)

   addis    r3,0,0x8B2E
   ori      r3,r3,0xB452
	
   stw      r3,CYGARC_REG_IMM_PSDMR(r4)

   #------------------------------------------
   # Loop 8 times, writing 0xFF to address 0
   #------------------------------------------

   addi  r6,0,0x0008
   mtspr CYGARC_REG_CTR,r6             # Load CTR with 8. The CTR special purpose
                            # is spr 9

   addi  r3,0,0x00FF      # Load 0x000000FF into r3

write_loop:

   stb   r3,0(r0)         # Write 0xFF to address 0 - bits [24-31]
    
   bc    16,0,write_loop  # Decrement CTR, then branch if the decremented CTR
                          # is not equal to 0      

   #-------------------------------------------------------------------------
   # Program the PSDMR again turning off refresh services and changing the
   # SDRAM operation to "Mode Register Write". Then do a single write to an
   # arbitrary location. The various fields that will be programmed in the 
   # mode register on the SDRAM were specified in fields of the PSDMR, like
   # the BR (burst length) and the CL (CAS Latency) field.
   #-------------------------------------------------------------------------
   
   # Clear refresh enable bit (bit 1 = 0)
   # Set OP mode to Mode Register Write (bits 2-4 = 011b)

   addis    r3,0,0x9B2E
   ori      r3,r3,0xB452
	
   stw      r3,CYGARC_REG_IMM_PSDMR(r4)

   addi     r3,0,0x00FF      # Load 0x000000FF into r3
   stb      r3,0(r0)         # Write 0xFF to address 0 - bits [24-31]
 
   #-------------------------------------------------------------------------
   # Program the PSDMR one last time turning on refresh services and changing 
   # the SDRAM operation to "Normal Operation". 
   #-------------------------------------------------------------------------
   
   # Set refresh enable bit (bit 1 = 1)
   # Set OP mode to Normal Operation (bits 2-4 = 000b)

   addis    r3,0,0xC32E
   ori      r3,r3,0xB452

   stw      r3,CYGARC_REG_IMM_PSDMR(r4)

   #----------------------------
   # return from init_60x_sdram
   #----------------------------
 
   bclr  20,0    # jump unconditionally to effective address in Link
                 # register
        
init_user_leds:	

   # Initialize Port A pins
   # Pin 30-31 GPIO wires to RF board
   # Pin 29 TS_RESET_L, output, Tiger Sharc Reset line, drive low on 
   #                    initialization.
   # Pin 28 LB_ENB_L, output, Link Port Buffer enable, drive high on
   #                  initialization.
   # Pin 26-27 GPIO wires to RF board
   # Pin 18-23 GPIO wires, one each to Tiger Sharc
   # Pin 12-17 Interrupt wires to Sharc, output, drive high on
   #                  initialization.

   # Set Port A pins 12-31 for general purpose, i.e., to 0
   # Set Port A pins 12-17, 28, 29 (add more later) for output (set to 1)
   #     for lines that we do not know, set to 0 (input)
   # Set Port A output pins High or Low, as specified
#define TS6_PPARA_INIT_MASK 0xFFF00000
#define TS6_PDIRA_INIT_MASK 0x3003F000
#define TS6_PDATA_INIT_MASK 0x1003F000
	
   # Initialize Port B Pins 4,5,6,7 general purpose IO
   # Pin 4 LED 18, Red
   # Pin 5 LED 18, Green
   # Pin 6 LED 17, Red
   # Pin 7 LED 17, Green
	
   mfspr    r30,CYGARC_REG_LR        # Save the Link Register value. The link registers
                          # value will be restored so that this function 
                          # can return to the calling address.

   addis    r4,0,0x0471   # IMMR base addr = 0x04700000+10000. We
                          # add 0x10000 because using relative addressing
                          # in load and store instructions only allow a
                          # offset from the base of +/-32767.
	
   addi     r6,0,0        # R6 = 0
   lwz      r3,0x0D24(r4) # R3 = PPARB
   rlwimi   r3,r6,0,4,7
#   and      r3,r3,r6      # Clear bits 4,5,6,7
   stw      r3,0x0D24(r4) # Store 0 to PPARB to set 4 bits to general
			  # purpose.

   addis    r6,0,0x0F00   # R6 = 0x0F000000
   lwz      r3,0x0D20(r4) # R3 = PDIRB
   or       r3,r3,r6      # Set bits 4,5,6,7
   stw      r3,0x0D20(r4) # Store r3 to PDIRB to set 4 bits as outputs.

   #-----------------------
   # return from init_user_leds
   #-----------------------
   mtspr    CYGARC_REG_LR,r30      # restore original Link Register value 

   bclr  20,0           # jump unconditionally to effective address in Link
                        # register

	
flash_leds_forever:
   # This function assumes that the Port B registers have been properly
   # initialized so the LEDs will function.  See init_user_leds.
	
   addis    r4,0,0x0471   # IMMR base addr = 0x04700000+10000. We
                          # add 0x10000 because using relative addressing
                          # in load and store instructions only allow a
                          # offset from the base of +/-32767.



   addi     r3,0,0x000F      # R3 will hold value of LEDs, start with all off
	
flash_again:
   #~~~~~~~~~~~~~~~~~~~~~~~~~~
   # Load count of 0x0040000
   #~~~~~~~~~~~~~~~~~~~~~~~~~~

	addis		r5,r0,4

   #~~~~~~~~~~~~~~~~~
   # Delay Loop here
   #~~~~~~~~~~~~~~~~~

check_count:

   #------------------------- 
   # while (count != 0)
   #------------------------- 

	cmpi	0,0,r5,0
	bc		12,2,count_done   # if zero branch to count_done

   #----------- 
   # count--;      
   #----------- 

	addi		r5,r5,-1
	b		check_count

count_done:
   #increment r3
	addi            r3,r3,1
	rlwinm          r6,r3,24,4,7 # left shift r3 by 24, mask r3 so
				     # so bits 4 -7 are all thats left,
	                             # and store result in r6
	stw         r6,0x0D30(r4)    # Store new value to PDATB Register
	b           flash_again
	

#ifdef CYG_HAL_STARTUP_ROM
   # This routine will test the memory from address 0x0 
   # thru 0x03ffffff (64MByte) by writing the address to the address
   # for all addresses, and then reading each of those values.
   #
   # Use two loops, the outer loop goes from 0 to 0x3ff and counts
   # by 1.  The inner loop goes from 0 to 0xfffc and counts by 4.
 .globl     memory_test
memory_test:	

   #Turn on Green Led
   # For TS6 board, LD17 will signal running status
   # LD17 == GREEN ==> writing data to memory
   # LD17 == RED   ==> reading data from memory and comparing
   # LD17 == OFF   ==> test completed
   # For TS6 board, LD18 will test status
   # LD18 == GREEN ==> all memory location contained expected values
   # LD18 == RED   ==> error reading memory location,
   #                   r6  contains address and expected value
   #                   r10 contains actual value read
   # LD18 == OFF   ==> test not completed
   #
   # r23 will hold the largest memory value to test
   # r29 will hold the smallest memory value (0x0 <= r29 < 0x10000)
   # NOTE read the code to find out exactly how the test is run.
   bl       init_user_leds
	
   addis     r23, 0, 0x0400   # Initialize r23 to 64 MBytes
   addis     r29, 0, 0x0      # Initialize r29 to 0
	
   addis    r4,0,0x0471   # IMMR base addr = 0x04700000+10000. We
                          # add 0x10000 because using relative addressing
                          # in load and store instructions only allow a
                          # offset from the base of +/-32767.
   # LD18 off, LED17 Green
   lwz      r3,0x0D30(r4) # r3 = PDATB value
   addis    r5,0,0x0d00   # LD18 off, LED17 Green
   rlwimi   r3,r5,0,4,7
   stw      r3,0x0D30(r4)

   #addis    r3,0,0x0200   # r3 will hold the outer counter, start at 0x3ff0000
   #addis    r9,0,0x0000   # r9 will hold the stop address

   mr        r3, r23       # r23 holds the start value, set with debugger
   mr        r9, r29       # r29 holds the stop value, set with debugger
	
memwrite_loop1:
   addis    r3,r3,-1      # subtract 0x10000 from r3
   addis    r5,0,0x1      # r5 will hold inner counter, start at 0xfffc

   # generate the value to store in the address.  It is the value in r3
   # shifted by 16 plus the vale in r5.
	
memwrite_loop2:
   addi     r5,r5,-4      # subtract 1 from r5
	
   add      r6,r3,r5
   stwx     r6,r5,r3      # Store the value at r6 in the memory addressed
	                  # by the sum of r3 and r5
   nop
   nop
   nop
#   lwzx     r10,r5,r3     # Read the value into r10 from the memory addressed
	                  # by the sum of r3 and r5
#   cmpw     r6,r10        # compare values to confirm write 
#   bne      mem_error
	
   cmpi	    0,0,r5,0
   bne      memwrite_loop2 # branch if r5 != 0

# Check if r3 is 0
   cmp      0,0,r3,r9
   bne      memwrite_loop1 # branch if r3 != 0

# Writing to memory complete, start reading and validating
   # LD18 off, LED17 Red
   lwz      r3,0x0D30(r4) # r3 = PDATB value
   addis    r5,0,0x0e00   # LD18 off, LED17 Red
   rlwimi   r3,r5,0,4,7
   stw      r3,0x0D30(r4)
	
   #addis    r3,0,0x0200   # r3 will hold the outer counter, start at 0x3ff0000
   #addis    r9,0,0x0000   # r9 will hold the stop address
   mr        r3, r23       # r23 holds the start value, set with debugger
   mr        r9, r29       # r29 holds the stop value, set with debugger
	
	
memread_loop1:
   addis    r3,r3,-1      # subtract 0x10000 from r3
   addis    r5,0,0x1      # r5 will hold inner counter, start at 0xfffc

   # generate the value to store in the address.  It is the value in r3
   # shifted by 16 plus the vale in r5.
	
memread_loop2:
   addi     r5,r5,-4      # subtract 1 from r5
	
   add      r16,r3,r5
   lwzx     r20,r5,r3     # Read the value into r20 from the memory addressed
	                  # by the sum of r3 and r5
   cmpw     r16,r20        # compare values
   bne      mem_error
	
   cmpi	    0,0,r5,0
   bne      memread_loop2 # branch if r5 != 0

# Check if r3 is 0
   cmp      0,0,r3,r9
   bne      memread_loop1 # branch if r3 != 0


   # We get here if memory test passes
   # LD18 Green, LED17 Off
   lwz      r3,0x0D30(r4) # r3 = PDATB value
   addis    r5,0,0x0700   # LD18 Green, LED17 Off
   rlwimi   r3,r5,0,4,7
   stw      r3,0x0D30(r4)
	
   addis    r7,0,0x0E00
   stw      r7,0(r8)
   b        branch_to_dot

mem_error:
   # We get here if a memory error was detected
   # LD18 Red, LED17 Off
   lwz      r3,0x0D30(r4) # r3 = PDATB value
   addis    r5,0,0x0b00   # LD18 off, LED17 Red
   rlwimi   r3,r5,0,4,7
   stw      r3,0x0D30(r4)
#endif	
branch_to_dot:
   b        branch_to_dot
	

#------------------------------------------------------------------------------
# end of ts6.S
