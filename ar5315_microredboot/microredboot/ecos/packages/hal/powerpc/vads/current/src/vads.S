#==============================================================================
##
##      vads.S
##
##      MPC8260 VADS board hardware setup
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
## Author(s):    nickg
## Contributors: wpd, gthomas
## Date:         2002-12-09
## Purpose:      VADS MPC8260 board hardware setup
## Description:  This file contains any code needed to initialize the
##               hardware on the Motorola VADS MPC8260 board.
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
//#ifdef DCSPRI_HAL_VADS_ROM_MLT_RAM
	// Today, we will totally skip over all of this if we are using
	// the RAM memory layout
//	b	hardware_init_done
//#endif

   mfspr  r22,CYGARC_REG_HID0    # Get contents of HID0
   rlwinm r23,r22,0,15,15 #shift r0 by 0, mask HID0 with 0x10000, store in r23
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
   #     The memory controller inserts one wait state between the assertion 
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
   # - APPC (Address Parity Pins Configuration) = 10 = 
   #
   #    Gives the following pin meanings: MODCK1|AP(1)|TC(0)  = BNKSEL(0)
   #                                      MODCK2|AP(2)|TC(1)  = BNKSEL(1)
   #                                      MODCK3|AP(3)|TC(2)  = BNKSEL(2)
   #                                      IRQ7/|APE/|INT_OUT/ = IRQ7/+INT_OUT/
   #                                      CS11/|AP(0)         = CS11/
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

   addis    r3,0,0x0000
   ori      r3,r3,0x4000
   stw      r3,CYGARC_REG_IMM_LTESCR1(r4)


   #-------------------------------------------------------------------------
   # First, program the Memory Periodic Timer Prescaler Register (MPTPR).
   # Finding the value to this ties in with the desired SDRAM Refresh
   # Timer (PSRT) value and the required Refresh Command interval for 
   # refreshing each row. The Refresh Command interval is found by 
   # determining the number of rows on the SDRAM device. In this case
   # its 2048. The "total time between refresh" value in the AC characteristic 
   # section of the data sheet is 32.8 msec. This value is divided by the 
   # number of rows to give the number of Refresh commands that needs to be
   # sent in a 32.8 msec interval. This value is 32.8 Msec/2048 = 16.02 usec.
   # Due to probable contention from time to time with other memory 
   # controller bus requests, lets make the refresh command interval to be
   # around 15.5 usec.
   #
   # Now let us divide the input VADs system clock by 64 (somewhat arbitrary).
   # If this is our prescaler value then the input frequency to the Refresh
   # timer is 1.031 Mhz. Using the formula:
   #
   #        Timer Period = PSRT
   #                       ----
   #                       F(MPTC)
   #
   #        - Where F(MPTC) is the output from the MPT Prescaler block
   #          or 66 Mhz/64 = 1.031 Mhz.
   #
   #        - Timer Period = 15.5 usec
   #
   # The PSRT value would be approximately 16.
   #
   # The clock distribution block diagram looks like this:
   #
   #   ----------      -------------         -----------    -------------
   #   | Clock   |     |   MPT     |         | SDRAM   |    | Refresh   |
   #   | Gen.    |-----| Prescaler |---------| Refresh |----| Command   |
   #   | 66 Mhz  |     -------------         | Timer   |    | Logic in  |
   #   | on Vads |                           -----------    | Mem.Cont. |
   #   | Board   |                                          -------------
   #   -----------
   #      
   #-------------------------------------------------------------------------

   addi     r5,0,0x4000      # load 0x40 or 64 into the PTP field of MPTPR
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

   bl    init_flash        # 8 Mbyte of flash memory

   bl    init_bcsr         # Board Control and Status Registers

   bl    init_local_sdram  # Local Bus SDRAM

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
#              designating bank 0 for the 8Mbyte flash SIMM on the VADS
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
   #                        flash that resides on the MPC8260 VADS board. 
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
#define nPFHACK
#ifdef PF_HACK // move Flash from 0xFF800000 to 0xFE000000
   addis    r3,0,0xFE00   # R3 holds the value temporarily
#else
   addis    r3,0,0xFF80   # R3 holds the value temporarily
#endif
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

#ifdef PF_HACK // move Flash from 0xFF800000 to 0xFE000000
   addis    r5,0,0xFE00     # R5 holds the value temporarily
#else
   addis    r5,0,0xFF80     # R5 holds the value temporarily
#endif
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
#              designating bank 1 for BCSR0, BCSR1, and BCSR2 on the VADS
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
   #                        that resides on the MPC8260 VADS board. 
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
# Function: init_60x_sdram
#
# Description: This function programs the 16 Mbyte SDRAM DIMM on the VADS 
#              board. This memory functions as the RAM for the 603 core and
#              is connected to the 60x bus. Base and Option Register 2 are 
#              used as well as the SDRAM Machine #1 and Chip Select #2.
#
#  History:
#
#  Jan /9/99    jay      Initial Release
#
#########################################################################        

init_60x_sdram:


   ##################
   # Program Refresh
   ##################


   #-------------------------------------------------------------------------
   # Program the 60x Bus Assigned SDRAM Refresh Timer (PSRT).
   #-------------------------------------------------------------------------

   addi     r5,0,0x0010      # load 0x10 or 16 
   stb      r5,CYGARC_REG_IMM_PSRT(r4)      # store byte - bits[24-31]


   #########################
   # Program Bank Registers
   #########################


   #-------------------------------------------------------------------------
   # Base Register 2 (BR2): Bank 2 is assigned to the 16 Mbyte 60x SDRAM DIMM
   #                        that resides on the MPC8260 VADS board. The 
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
   # Option Register 2 (OR2) for SDRAM Machine use: further SDRAM definitions
   #
   # - USDAM (Upper SDRAM Address Mask) = 11111b = 
   #
   #    We have masked all 5 bits which defines a maximum 128 Mbyte block.
   #    Note that since the maximum SDRAM block size is 128 Mbyte, this field
   #    should always be 11111b.
   #
   # - SDAM (SDRAM Address Mask) = 1110000b = 16 Mbyte block.
   #
   # - LSDAM (Lower SDRAM Address Mask) = 00000b = Minimum of 1 Mbyte size.
   #
   # - BPD (Banks Per Device) = 00 = 2 internal banks per device.
   #
   # - ROWST (Row Start Address Bit) = 011 = A9 being the row start address 
   #                                         bit.
   #
   # - Bit 22 Reserved - set to 0.
   #
   # - NUMR (Number of Row Address Lines) = 010 = 11 row address lines.
   #
   # - PMSEL (Page Mode Select) = 1 = Reserved
   #
   # - IBID (Internal bank interleaving within same device disable) = 0 =
   # 
   #    Bank interleaving allowed.
   #
   # - AACKR (AACK/ release) = 0 = Normal operation.
   #
   # - Bits 29-31 Reserved - set to 0.
   #-------------------------------------------------------------------------

#define nCYGHWR_HAL_POWERPC_VADS_64MB_DIMM
#ifdef CYGHWR_HAL_POWERPC_VADS_64MB_DIMM
   addis    r5,0,0xFC00     # R5 holds the value temporarily
   ori      r5,r5,0x28E0
#else
   addis    r5,0,0xFF00     # R5 holds the value temporarily
   ori      r5,r5,0x0CA0
#endif

   #------------------
   # Write the values
   #------------------

   stw        r5,CYGARC_REG_IMM_OR2(r4)
   stw        r3,CYGARC_REG_IMM_BR2(r4)


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
   # - Bit 0 is reserved. Set to 0.
   #
   # - RFEN (Refresh Enable) = 0 = Refresh services not required. This bit 
   #                               will be set later in this function as a 
   #                               last step.
   # 
   # - OP (SDRAM Operation) = 000 = Precharge all banks.
   #
   # - SDAM (Address Multiplex Size = 001 = 
   #
   #   Coming up the value for this field is one of the most confusing
   #   and non-intuitive steps in the SDRAM programming process. This is how
   #   it works... The SDRAM device is a 16 Mbit DIMM that has a data width 
   #   of 64 bits or 8 bytes. The bank size is 64 bits so the SDRAM will 
   #   ignore the least significant 3 bits. Given this information and 
   #   knowing that the number of row address lines is 11 and the column 
   #   addresses is 9 and also knowing that the row addresses must be 
   #   multiplexed, write out the following:
   #
   #   addresses ignored by SDRAM : A29 A30 A31
   #
   #   column addresses: A20 A21 A22 A23 A24 A25 A26 A27 A28
   #
   #   row addresses:   A9 A10 A11 A12 A13 A14 A15 A16 A17 A18 A19
   #
   #   When the memory controller multiplexes the addresses it puts the 
   #   column addresses out on the actual physical pins that matches its
   #   corresponding effective address bits.
   #
   #   According to the SDRAM Address Multiplexing table in the memory 
   #   controller section of the manual, SDAM selection of 001 fits the 
   #   address layout given above. Because its the row that is multiplexed on
   #   designated column address pins, the row addresses are put out on the
   #   following column pins:
   #
   #   column ---------------------->  A20 A21 A22 A23 A24 A25 A26 A27 A28
   #                                    |   |   |   |   |   |   |   |   |
   #   row address -----------> A9 A10 A11 A12 A13 A14 A15 A16 A17 A18 A19
   #   on corresponding
   #   column pins.
   #
   #   Column pins A19, A18, and A17 were not connected to the DIMM device so
   #   A9 and A10 as well as the bank select address which is A8 are put out 
   #   on their respective pins because it was decided that these address 
   #   pins would be directly connected to the DIMM. That is, A10 of the real
   #   address is put out on the A10 pin, A9 on the A9 pin, etc.
   #
   #   As a final note on this subject, the row address is output on the 
   #   address bus first on the actual column address pins according to the 
   #   multiplexing scheme in the SDRAM address multiplexing table, then the 
   #   column address follows which makes up the most significant part of the
   #   real address which is connected to the memory device. 
   #
   #   Any entries in the multiplexing table that show a "-" means nothing is
   #   multiplexed on the physical pins shown on the top row of the table
   #   and therefore any part of the row address will also show up on its 
   #   respective physical pin if this pin is not used for multiplexing. For 
   #   example, for a SDAM of 001, the multiplexing table shows that A13 
   #   through A0 are not multiplexed because they have "-" in their 
   #   respective boxes. This means that A13 through A5 of the real address
   #   will show up multiplexed on address pins A22 through A14 and A13 
   #   through A0 of the real address will also show up on their actual 
   #   respective physical pins.
   #
   # - BSMA (Bank Select Multiplexed Address line) = 011 = A15-A17
   #
   #    BSMA determines how BNKSEL[0:2] will operate. This SDRAM has only 2 
   #    banks, so only BANKSEL2 is of concern because its the least 
   #    significant bit. BANKSEL2 should be programmed to operate as physical 
   #    pin A17. Because we are multiplexing 9 column addresses, it will 
   #    actually behave as A8. 
   #
   # - SDA10 ("A10 Control") = 011 = 
   #
   #    A9 is the address pin used for the activate command. The A10/AP pin
   #    the SDRAM needs to act as a command line during on phase of the 
   #    access and as an address line in the other phase. The SDA10 pin on 
   #    MPC8260 takes care of this function. Instead of connecting the 
   #    A10/AP pin to A9 on the address bus, connect it to SDA10 and program
   #    SDA10 to function as A9.
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

#ifdef CYGHWR_HAL_POWERPC_VADS_64MB_DIMM
   addis    r3,0,0x294E
   ori      r3,r3,0xB452
#else
   addis    r3,0,0x296E
   ori      r3,r3,0xB452
#endif
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
   
#ifdef CYGHWR_HAL_POWERPC_VADS_64MB_DIMM
   addis    r3,0,0x094E
   ori      r3,r3,0xB452
#else
   addis    r3,0,0x096E
   ori      r3,r3,0xB452
#endif
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
   
#ifdef CYGHWR_HAL_POWERPC_VADS_64MB_DIMM
   addis    r3,0,0x194E
   ori      r3,r3,0xB452
#else
   addis    r3,0,0x196E
   ori      r3,r3,0xB452
#endif
   stw      r3,CYGARC_REG_IMM_PSDMR(r4)

   addi     r3,0,0x00FF      # Load 0x000000FF into r3
   stb      r3,0(r0)         # Write 0xFF to address 0 - bits [24-31]
 
   #-------------------------------------------------------------------------
   # Program the PSDMR one last time turning on refresh services and changing 
   # the SDRAM operation to "Normal Operation". 
   #-------------------------------------------------------------------------
   
#ifdef CYGHWR_HAL_POWERPC_VADS_64MB_DIMM
   addis    r3,0,0x414E
   ori      r3,r3,0xB452
#else
   addis    r3,0,0x416E
   ori      r3,r3,0xB452
#endif
   stw      r3,CYGARC_REG_IMM_PSDMR(r4)

   #----------------------------
   # return from init_60x_sdram
   #----------------------------
 
   bclr  20,0    # jump unconditionally to effective address in Link
                 # register


#########################################################################
# Function: init_local_sdram
#
# Description: This function programs the 4 Mbytes of SDRAM on the Local
#              Bus on the VADS board. This memory functions as storage for
#              connection tables and data buffers for the CPM peripherals.
#              Base and Option Register 4 are used. SDRAM Machine #2
#              and Chip Select #4 is used in the memory controller.
#
#  History:
#
#  Jan 9/99    jay      
#
#########################################################################        

init_local_sdram:

   ##################
   # Program Refresh
   ##################

   #-------------------------------------------------------------------------
   # Program the Local Bus Assigned SDRAM Refresh Timer (LSRT). Note that the
   # MPTPR register was programmed in the init_siu function which also
   # dictates the frequency to the LSRT block. Because the frequency rating
   # for the local bus SDRAM is same as the 60x SDRAM DIMM, only the LSRT
   # needs to be programmed and it is going to be the same as the PSRT
   # register.
   #-------------------------------------------------------------------------

   addi     r5,0,0x0010      # load 0x10 or 16 
   stb      r5,CYGARC_REG_IMM_LSRT(r4)      # store byte - bits[24-31]


   #########################
   # Program Bank Registers
   #########################


   #-------------------------------------------------------------------------
   # Base Register 4 (BR4): Bank 4 is assigned to the 4 Mbyte Local SDRAM 
   #                        bank that resides on the MPC8260 VADS board. The 
   #                        particulars are defined here. 
   #
   # - BA (Base Address) = 0x0400+0b for a total of 17 address bits. This 
   #                       value represents the upper 17 bits of the base 
   #                       address.
   #
   # - Bits 17-18 reserved. = 00 = cleared to 0.
   #
   # - PS (Port Size) = 11b = 32 bit port size
   #
   # - DECC (Data Error Correction and Checking) = 00 = Data errors checking
   #                                                    Disabled.
   #
   # - WP (Write Protect) = 0 = both read and write accesses are allowed
   #
   # - MS (Machine Select) = 011 = SDRAM Machine for Local bus Selected
   #
   # - EMEMC (External Memory Controller Enable) = 0 = 
   #
   #    Accesses are handled by the memory controller according to MSEL.
   #
   # - ATOM (Atomic Operation) = 00 = 
   #
   #    The address space controlled by the memory controller bank is not 
   #    used for atomic operations.
   #
   # - DR (Delayed Read) = 0 = Normal operation.
   #
   # - V (Valid Bit) =  1 = Valid bit set
   #-------------------------------------------------------------------------

   addis    r3,0,0x0400     # R3 holds the value temporarily
   ori      r3,r3,0x1861

   #-------------------------------------------------------------------------
   # Option Register 4 (OR4) for SDRAM Machine use: further SDRAM definitions
   #
   # - USDAM (Upper SDRAM Address Mask) = 11111b = 
   #
   #    We have masked all 5 bits which defines a maximum 128 Mbyte block Note
   #    that since the maximum SDRAM block size is 128 Mbyte, this field
   #    should always be 11111b.
   #
   # - SDAM (SDRAM Address Mask) = 1111100b = 4 Mbyte block.
   #
   # - LSDAM (Lower SDRAM Address Mask) = 00000b = Minimum of 1 Mbyte size.
   #
   # - BPD (Banks Per Device) = 00 = 2 internal banks per device.
   #
   # - ROWST (Row Start Address Bit) = 101 = A11 being the row start address
   #                                         bit.
   #
   # - Bit 22 Reserved - set to 0.
   #
   # - NUMR (Number of Row Address Lines) = 010 = 11 row address lines.
   #
   # - PMSEL (Page Mode Select) = 0 = back-to-back page mode (normal 
   #                                  operation).
   #
   # - IBID (Internal bank interleaving within same device disable) = 0 =
   # 
   #    Bank interleaving allowed.
   #
   # - AACKR (AACK/ release) = 0 = Normal operation.
   #
   # - Bits 29-31 Reserved - set to 0.
   #-------------------------------------------------------------------------

   addis    r5,0,0xFFC0     # R5 holds the value temporarily
   ori      r5,r5,0x1480

   #------------------
   # Write the values
   #------------------

   stw        r5,CYGARC_REG_IMM_OR4(r4)
   stw        r3,CYGARC_REG_IMM_BR4(r4)


   ###########################################
   # Perform Initialization sequence to SDRAM
   ###########################################

   #-------------------------------------------------------------------------
   # Program the Local SDRAM Mode Registr (LSDMR). This register is used
   # to configure operations pertaining to SDRAM on the Local bus. Turn off 
   # refresh services and change the SDRAM operation to "Precharge all banks".
   # Then do a single write to an arbitrary location. Writing 0 top
   # address 0 will do the trick.
   #
   # - Bit 0 is reserved. Set to 0.
   #
   # - RFEN (Refresh Enable) = 0 = Refresh services not required. This bit 
   #                               will be set later in this function as a 
   #                               last step.
   # 
   # - OP (SDRAM Operation) = 101 = Precharge all banks.
   #
   # - SDAM (Address Multiplex Size = 000 = 
   #
   #   Coming up the value for this field is one of the most confusing
   #   and non-intuitive steps in the SDRAM programming process. This is how
   #   it works... The SDRAM device is a 4 Mbit chip set that has a data  
   #   width of 32 bits or 4 bytes so the SDRAM will ignore the least 
   #   significant 2 bits. Given this information and knowing that the 
   #   number of row address lines is 11 and the column addresses is 8 and 
   #   also knowing that the row addresses must be multiplexed, write out the 
   #   following:
   #
   #   addresses ignored by SDRAM : A30 A31
   #
   #   column addresses: A22 A23 A24 A25 A26 A27 A28 A29
   #
   #   row addresses:   A11 A12 A13 A14 A15 A16 A17 A18 A19 A20 A21
   #
   #   When the memory controller multiplexes the addresses it puts the 
   #   column addresses out on the actual physical pins that matches its
   #   corresponding effective address bits.
   #
   #   According to the SDRAM Address Multiplexing table in the memory 
   #   controller section of the manual, SDAM selection of 000 fits the 
   #   address layout given above. Because the row is what is multiplexed 
   #   on designated column address pins, the row addresses are put out on the
   #   following column pins:
   #
   #.                                  8 column addresses
   #                                   |
   #   column ------------->A19 A20 A21|A22 A23 A24 A25 A26 A27 A28 A29
   #                         |   |   |   |   |   |   |   |   |   |   |
   #.  11 row addresses---->A11 A12 A13 A14 A15 A16 A17 A18 A19 A20 A21
   #   on corresponding
   #   column pins.
   #
   #   The row address is output on the address bus first, then the column 
   #   address.              
   #
   # - BSMA (Bank Select Multiplexed Address line) = 100 = A16-A18
   #
   #    For Local bus accesses, this field is a "do not care". BNKSEL[0:2]
   #    are associated with this field and these pins are only used for 
   #    60x compatible mode for 60x bus transactions.
   #
   # - SDA10 ("A10 Control") = 001 = A11 is the address pin used for the
   #                                 activate command.
   #
   # - RFRC (ReFresh ReCovery) = 101 = 
   #
   #   Once a refresh request is granted, the memory controller begins 
   #   issuing auto-refresh commands to each device associated with the 
   #   refresh timer in one clock intervals. After the last REFRESH 
   #   command is issued, the memory controller waits for 7 clocks before 
   #   the ACTIVATE command and before normal read/write operations can 
   #   resume.
   #
   # - PRETOACT (PREcharge TO ACTivate interval) = 010 = 
   #
   #   Wait 2 clock cycles before an activate command.
   #
   # - ACTTORW (ACTivate TO Read/Write interval) = 010 = 2 clock cycles.
   # 
   # - BL (Burst Length) = 1 = SDRAM burst length is 8. This is programmed 
   #                           into the SDRAM via the MRS commmand.
   #
   # - LDOTOPRE (Last Data Out TO PREcharge) = 00 = 
   #
   #   The delay required from the last data out to precharge state is 0 clock
   #   cycles.
   #                               
   # - WRC (Write Recovery Time) = 01 = 
   #
   #   Time needed to elapse before another operation is 1 clock cycle.
   #
   # - EAMUX (External Address MUltipleXing) = 0 No external address
   #                                             multiplexing.
   #
   # - BUFCMD (Buffer Control Lines) = 0 = Normal timing for the control
   #                                       lines.
   #
   # - CL (CAS Latency) = 10 = CAS latency is 2. Two cycles after column
   #                           address is registered, data is valid.
   #
   #-------------------------------------------------------------------------

   addis    r3,0,0x2886
   ori      r3,r3,0xA522
   stw      r3,CYGARC_REG_IMM_LSDMR(r4)

   addis    r2,0,0x0400
   addi     r3,0,-1
   stb      r3,0(r2)         # Write 0xFF to address 0x04000000
   

   #-------------------------------------------------------------------------
   # Program the LSDMR keeping refresh services off and changing the
   # SDRAM operation to "CBR Refresh". This step is responsible for issuing
   # a minimum of 8 auto-refresh commands. This is done by the SDRAM machine
   # by issuing the CBR Refresh command by programming the OP field of the 
   # PSDMR register and writing 0xFF 8 times to an arbitrary address.
   #-------------------------------------------------------------------------
   
   addis    r3,0,0x0886
   ori      r3,r3,0xA522
   stw      r3,CYGARC_REG_IMM_LSDMR(r4)

   #--------------------------------------------------
   # Loop 8 times, writing 0xFF to address 0x04000000
   #--------------------------------------------------

   addis r6,0,0
   ori   r6,r6,8
   mtspr CYGARC_REG_CTR,r6           # Load CTR with 8. 

   addi  r3,0,-1          # Load 0xFs into r3

write_loop1:

   stb   r3,0(r2)         # Write 0 to address 0x04000000
    
   bc    16,0,write_loop1 # Decrement CTR, then branch if the decremented CTR
                          # is not equal to 0      

   #-------------------------------------------------------------------------
   # Program the LSDMR again turning off refresh services and changing the
   # SDRAM operation to "Mode Register Write". Then do a single write to an
   # arbitrary location. The various fields that will be programmed in the 
   # mode register on the SDRAM were specified in fields of the LSDMR, like
   # the BR (burst length) and the CL (CAS Latency) field.
   #-------------------------------------------------------------------------
   
   addis    r3,0,0x1886
   ori      r3,r3,0xA522
   stw      r3,CYGARC_REG_IMM_LSDMR(r4)

   addi     r3,0,-1           
   stb      r3,0(r2)         # Write 0xFF to address 0x04000000
  
   #-------------------------------------------------------------------------
   # Program the LSDMR one last time turning on refresh services and changing 
   # the SDRAM operation to "Normal Operation". 
   #-------------------------------------------------------------------------
   
   addis    r3,0,0x4086
   ori      r3,r3,0xA522
   stw      r3,CYGARC_REG_IMM_LSDMR(r4)

   #------------------------------
   # return from init_local_sdram
   #------------------------------

   bclr  20,0    # jump unconditionally to effective address in Link
                 # register
                
#------------------------------------------------------------------------------
# end of vads.S
