/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/*
 * Generic setup routines for Broadcom 963xx MIPS boards
 */

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/console.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <asm/addrspace.h>
#include <asm/bcache.h>
#include <asm/irq.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/gdb-stub.h>

extern void brcm_time_init(void);

extern unsigned long getMemorySize(void);

#include <bcm_map_part.h>
#include <boardparms.h>

#if defined(CONFIG_PCI)
#include <linux/pci.h>
#include <bcmpci.h>
#endif

/* This function should be in a board specific directory.  For now,
 * assume that all boards that include this file use a Broadcom chip
 * with a soft reset bit in the PLL control register.
 */
static void brcm_machine_restart(char *command)
{
    PERF->pll_control |= SOFT_RESET;
}

static void brcm_machine_halt(void)
{
    printk("System halted\n");
    while (1);
}

#if defined(CONFIG_PCI)

void mpi_SetLocalPciConfigReg(uint32 reg, uint32 value)
{
    /* write index then value */
    MPI->pcicfgcntrl = PCI_CFG_REG_WRITE_EN + reg;;
    MPI->pcicfgdata = value;
}

uint32 mpi_GetLocalPciConfigReg(uint32 reg)
{
    /* write index then get value */
    MPI->pcicfgcntrl = PCI_CFG_REG_WRITE_EN + reg;;
    return MPI->pcicfgdata;
}

/*
 * mpi_ResetPcCard: Set/Reset the PcCard
 */
void mpi_ResetPcCard(int cardtype, BOOL bReset)
{
    if (cardtype == MPI_CARDTYPE_NONE) {
        return;
    }

    if (cardtype == MPI_CARDTYPE_CARDBUS) {
        bReset = ! bReset;
    }

    if (bReset) {
        MPI->pcmcia_cntl1 = (MPI->pcmcia_cntl1 & ~PCCARD_CARD_RESET);
    } else {
        MPI->pcmcia_cntl1 = (MPI->pcmcia_cntl1 | PCCARD_CARD_RESET);
    }
}

/*
 * mpi_ConfigCs: Configure an MPI/EBI chip select
 */
void mpi_ConfigCs(uint32 cs, uint32 base, uint32 size, uint32 flags)
{
    MPI->cs[cs].base = ((base & 0x1FFFFFFF) | size);
    MPI->cs[cs].config = flags;
}

/*
 * mpi_InitPcmciaSpace
 */
void mpi_InitPcmciaSpace(void)
{
    // ChipSelect 4 controls PCMCIA Memory accesses
    mpi_ConfigCs(PCMCIA_COMMON_BASE, pcmciaMem, EBI_SIZE_1M, (EBI_WORD_WIDE|EBI_ENABLE));
    // ChipSelect 5 controls PCMCIA Attribute accesses
    mpi_ConfigCs(PCMCIA_ATTRIBUTE_BASE, pcmciaAttr, EBI_SIZE_1M, (EBI_WORD_WIDE|EBI_ENABLE));
    // ChipSelect 6 controls PCMCIA I/O accesses
    mpi_ConfigCs(PCMCIA_IO_BASE, pcmciaIo, EBI_SIZE_64K, (EBI_WORD_WIDE|EBI_ENABLE));

    MPI->pcmcia_cntl2 = ((PCMCIA_ATTR_ACTIVE << RW_ACTIVE_CNT_BIT) | 
                         (PCMCIA_ATTR_INACTIVE << INACTIVE_CNT_BIT) | 
                         (PCMCIA_ATTR_CE_SETUP << CE_SETUP_CNT_BIT) | 
                         (PCMCIA_ATTR_CE_HOLD << CE_HOLD_CNT_BIT));

    MPI->pcmcia_cntl2 |= (PCMCIA_HALFWORD_EN | PCMCIA_BYTESWAP_DIS);
}

/*
 * cardtype_vcc_detect: PC Card's card detect and voltage sense connection
 * 
 *   CD1#/      CD2#/     VS1#/     VS2#/    Card       Initial Vcc
 *  CCD1#      CCD2#     CVS1      CVS2      Type
 *
 *   GND        GND       open      open     16-bit     5 vdc
 *
 *   GND        GND       GND       open     16-bit     3.3 vdc
 *
 *   GND        GND       open      GND      16-bit     x.x vdc
 *
 *   GND        GND       GND       GND      16-bit     3.3 & x.x vdc
 *
 *====================================================================
 *
 *   CVS1       GND       CCD1#     open     CardBus    3.3 vdc
 *
 *   GND        CVS2      open      CCD2#    CardBus    x.x vdc
 *
 *   GND        CVS1      CCD2#     open     CardBus    y.y vdc
 *
 *   GND        CVS2      GND       CCD2#    CardBus    3.3 & x.x vdc
 *
 *   CVS2       GND       open      CCD1#    CardBus    x.x & y.y vdc
 *
 *   GND        CVS1      CCD2#     open     CardBus    3.3, x.x & y.y vdc
 *
 */
int cardtype_vcc_detect(void)
{
    uint32 data32;
    int cardtype;

    cardtype = MPI_CARDTYPE_NONE;
    MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE); // Turn on the output enables and drive
                                        // the CVS pins to 0.
    data32 = MPI->pcmcia_cntl1;
    switch (data32 & (CD2_IN|CD1_IN))  // Test CD1# and CD2#, see if card is plugged in.
    {
    case (CD2_IN|CD1_IN):  // No Card is in the slot.
        printk("MPI: No Card is in the PCMCIA slot\n");
        break;

    case CD2_IN:  // Partial insertion, No CD2#.
        printk("MPI: Card in the PCMCIA slot partial insertion, no CD2 signal\n");
        break;

    case CD1_IN:  // Partial insertion, No CD1#.
        printk("MPI: Card in the PCMCIA slot partial insertion, no CD1 signal\n");
        break;

    case 0x00000000:
        MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS2_OEN|VS1_OEN); 
                                        // Turn off the CVS output enables and
                                        // float the CVS pins.
        mdelay(1);
        data32 = MPI->pcmcia_cntl1;
        // Read the Register.
        switch (data32 & (VS2_IN|VS1_IN))  // See what is on the CVS pins.
        {
        case 0x00000000: // CVS1 and CVS2 are tied to ground, only 1 option.
            printk("MPI: Detected 3.3 & x.x 16-bit PCMCIA card\n");
            cardtype = MPI_CARDTYPE_PCMCIA;
            break;
          
        case VS1_IN: // CVS1 is open or tied to CCD1/CCD2 and CVS2 is tied to ground.
                         // 2 valid voltage options.
        switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
        {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n"); 
                break;
      
            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2. 
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS2_OEN); // Drive CVS1 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD2_IN) { // CCD2 is tied to CVS2, not valid.
                    printk("MPI: Unknown card plugged into slot\n"); 
                } else {                   // CCD2 is tied to CVS1.
                    printk("MPI: Detected 3.3, x.x and y.y Cardbus card\n");
                    cardtype = MPI_CARDTYPE_CARDBUS;
                }
                break;
                
            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n"); 
                break;
                
            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                printk("MPI: Detected x.x vdc 16-bit PCMCIA card\n");
                cardtype = MPI_CARDTYPE_PCMCIA;
                break;
            }
            break;
          
        case VS2_IN: // CVS2 is open or tied to CCD1/CCD2 and CVS1 is tied to ground.
                         // 2 valid voltage options.
            switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
            {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n"); 
                break;
      
            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD2_IN) { // CCD2 is tied to CVS1, not valid.
                    printk("MPI: Unknown card plugged into slot\n"); 
                } else {// CCD2 is tied to CVS2.
                    printk("MPI: Detected 3.3 and x.x Cardbus card\n");
                    cardtype = MPI_CARDTYPE_CARDBUS;
                }
                break;

            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n"); 
                break;

            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                cardtype = MPI_CARDTYPE_PCMCIA;
                printk("MPI: Detected 3.3 vdc 16-bit PCMCIA card\n");
                break;
            }
            break;
          
        case (VS2_IN|VS1_IN):  // CVS1 and CVS2 are open or tied to CCD1/CCD2.
                          // 5 valid voltage options.
      
            switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
            {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n"); 
                break;
      
            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                              // CCD1 is tied to ground.
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD2_IN) {  // CCD2 is tied to CVS1.
                    printk("MPI: Detected y.y vdc Cardbus card\n");
                } else {                    // CCD2 is tied to CVS2.
                    printk("MPI: Detected x.x vdc Cardbus card\n");
                }
                cardtype = MPI_CARDTYPE_CARDBUS;
                break;
      
            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // CCD2 is tied to ground.
      
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD1_IN) {// CCD1 is tied to CVS1.
                    printk("MPI: Detected 3.3 vdc Cardbus card\n");
                } else {                    // CCD1 is tied to CVS2.
                    printk("MPI: Detected x.x and y.y Cardbus card\n");
                }
                cardtype = MPI_CARDTYPE_CARDBUS;
                break;
      
            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                cardtype = MPI_CARDTYPE_PCMCIA;
                printk("MPI: Detected 5 vdc 16-bit PCMCIA card\n");
                break;
            }
            break;
      
        default:
            printk("MPI: Unknown card plugged into slot\n"); 
            break;
        
        }
    }
    return cardtype;
}

/*
 * mpi_DetectPcCard: Detect the plugged in PC-Card
 * Return: < 0 => Unknown card detected
 *         0 => No card detected
 *         1 => 16-bit card detected
 *         2 => 32-bit CardBus card detected
 */
int mpi_DetectPcCard(void)
{
    int cardtype;

    cardtype = cardtype_vcc_detect();
    switch(cardtype) {
        case MPI_CARDTYPE_PCMCIA:
            MPI->pcmcia_cntl1 &= ~(CARDBUS_ENABLE|PCMCIA_ENABLE|PCMCIA_GPIO_ENABLE); // disable enable bits
            MPI->pcmcia_cntl1 |= (PCMCIA_ENABLE | PCMCIA_GPIO_ENABLE);
            mpi_InitPcmciaSpace();
            mpi_ResetPcCard(cardtype, FALSE);
            // Hold card in reset for 10ms
            mdelay(10);
            mpi_ResetPcCard(cardtype, TRUE);
            // Let card come out of reset
            mdelay(100);
            break;
        case MPI_CARDTYPE_CARDBUS:
            // 8 => CardBus Enable
            // 1 => PCI Slot Number
            // C => Float VS1 & VS2
            MPI->pcmcia_cntl1 = (MPI->pcmcia_cntl1 & 0xFFFF0000) | 
                                CARDBUS_ENABLE | 
                                (CARDBUS_SLOT << 8)| 
                                VS2_OEN |
                                VS1_OEN | PCMCIA_GPIO_ENABLE;
            /* access to this memory window will be to/from CardBus */
            MPI->l2pmremap1 |= CARDBUS_MEM;

            // Need to reset the Cardbus Card. There's no CardManager to do this, 
            // and we need to be ready for PCI configuration. 
            mpi_ResetPcCard(cardtype, FALSE);
            // Hold card in reset for 10ms
            mdelay(10);
            mpi_ResetPcCard(cardtype, TRUE);
            // Let card come out of reset
            mdelay(100);
            break;
        default:
            break;
    }
    return cardtype;
}

int mpi_init(void)
{
    unsigned long data;
    unsigned int chipid;
    unsigned int chiprev;
    unsigned int sdramsize;
    unsigned int modesel;

    chipid  = (PERF->RevID & 0xFFFF0000) >> 16;
    chiprev = (PERF->RevID & 0xFF);
    sdramsize = getMemorySize();

    // UBUS to PCI address range
    // Memory Window 1. Used for devices in slot 0. Potentially can be CardBus
    MPI->l2pmrange1 = ~(BCM_PCI_MEM_SIZE_16MB-1);
    // UBUS to PCI Memory base address. This is akin to the ChipSelect base
    // register. 
    MPI->l2pmbase1 = BCM_CB_MEM_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI Remap Address. Replaces the masked address bits in the
    // range register with this setting. 
    // Also, enable direct I/O and direct Memory accesses
    MPI->l2pmremap1 = (BCM_PCI_MEM_BASE | MEM_WINDOW_EN);

    // Memory Window 2. Used for devices in other slots
    MPI->l2pmrange2 = ~(BCM_PCI_MEM_SIZE_16MB-1);
    // UBUS to PCI Memory base address. 
    MPI->l2pmbase2 = BCM_PCI_MEM_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI Remap Address
    MPI->l2pmremap2 = (BCM_PCI_MEM_BASE | MEM_WINDOW_EN);

    // Setup PCI I/O Window range. Give 64K to PCI I/O
    MPI->l2piorange = ~(BCM_PCI_IO_SIZE_64KB-1);
    // UBUS to PCI I/O base address 
    MPI->l2piobase = BCM_PCI_IO_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI I/O Window remap
    MPI->l2pioremap = (BCM_PCI_IO_BASE | MEM_WINDOW_EN);

    // enable PCI related GPIO pins and data swap between system and PCI bus
    MPI->locbuscntrl = (EN_PCI_GPIO | DIR_U2P_NOSWAP);

    /* Enable BusMaster and Memory access mode */
    data = mpi_GetLocalPciConfigReg(PCI_COMMAND);
    data |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
    mpi_SetLocalPciConfigReg(PCI_COMMAND, data);

    /* Configure two 16 MByte PCI to System memory regions. */
    /* These memory regions are used when PCI device is a bus master */
    /* Accesses to the SDRAM from PCI bus will be "byte swapped" for this region */
    mpi_SetLocalPciConfigReg(PCI_BASE_ADDRESS_3, BCM_HOST_MEM_SPACE1);

#if defined(CONFIG_BCM96348)
    MPI->sp0remap = 0x0;    
#else    
    MPI->sp0remap = MEM_WINDOW_EN;
#endif    

    /* Accesses to the SDRAM from PCI bus will be "byte swapped" for this region */
    mpi_SetLocalPciConfigReg(PCI_BASE_ADDRESS_4, BCM_HOST_MEM_SPACE2);
    
#if defined(CONFIG_BCM96348)
    MPI->sp1remap = 0x0;
#else
    MPI->sp1remap = MEM_WINDOW_EN;
#endif    
    
    modesel = MPI->pcimodesel;		
    modesel &= ~PCI_INT_BUS_RD_PREFETCH;
#if defined(CONFIG_BCM96348)
    modesel |= 0x80;
#else
    modesel |= 0x100;
#endif
    MPI->pcimodesel = modesel;

    if (!((chipid == 0x6348) && ((chiprev & 0xF0) == 0xa0))) {
        MPI->sp0range = ~(sdramsize-1);
        MPI->sp1range = ~(sdramsize-1);
    }
    /*
     * Change PCI Cfg Reg. offset 0x40 to PCI memory read retry count infinity
     * by set 0 in bit 8~15.  This resolve read Bcm4306 srom return 0xffff in
     * first read.
     */
    data = mpi_GetLocalPciConfigReg(BRCM_PCI_CONFIG_TIMER);
    data &= ~BRCM_PCI_CONFIG_TIMER_RETRY_MASK;
    data |= 0x00000080;
    mpi_SetLocalPciConfigReg(BRCM_PCI_CONFIG_TIMER, data);

    /* enable pci interrupt */
    MPI->locintstat |= (EXT_PCI_INT << 16);

    ioport_resource.start = BCM_PCI_IO_BASE;
    ioport_resource.end = BCM_PCI_IO_BASE + BCM_PCI_IO_SIZE_64KB;

    return 0;
}
#endif

#if defined(CONFIG_BCM96338)
int __init bcm_hw_init(void)
{
    unsigned long ledSave = *WAN_DATA_LED;
    /* 
     *  ATM and SDIOH share the same reset bit
     *  Issue reset needed such that SDIIOH can work properly after power cycle
     */
    PERF->BlockSoftReset &= ~BSR_SAR;
    mdelay(10);
    PERF->BlockSoftReset |= BSR_SAR;
    mdelay(10);   
    *WAN_DATA_LED = ledSave;
	
    /* Enable SPI interface */
    PERF->blkEnables |= SPI_CLK_EN;

    return 0;
}

#elif defined(CONFIG_BCM96348)
int __init bcm_hw_init(void)
{
    unsigned long data;
    unsigned short GPIOOverlays;

    /* Set MPI clock to 33MHz and Utopia clock to 25MHz */
    data = PERF->pll_control;
    data &= ~MPI_CLK_MASK;
    data |= MPI_CLK_33MHZ;
    data &= ~MPI_UTOPIA_MASK;
    data |= MPI_UTOPIA_25MHZ; /* 6348 utopia frequency has to be 25MHZ */
    PERF->pll_control = data;

    /* Enable SPI interface */
    PERF->blkEnables |= SPI_CLK_EN;

    GPIO->GPIOMode = 0;

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {

        if (GPIOOverlays & BP_UTOPIA) {
            /* Enable UTOPIA interface */
            GPIO->GPIOMode |= GROUP4_UTOPIA | GROUP3_UTOPIA | GROUP1_UTOPIA;
            PERF->blkEnables |= SAR_CLK_EN;
        }

        if (GPIOOverlays & BP_MII2) {
            if (GPIOOverlays & BP_UTOPIA) {
                printk ("*************** ERROR ***************\n");
                printk ("Invalid GPIO configuration. External MII cannot be enabled with UTOPIA\n");
            }
            /* Enable external MII interface */
            GPIO->GPIOMode |= (GROUP3_EXT_MII|GROUP0_EXT_MII); /*  */
        }

        if (GPIOOverlays & BP_SPI_EXT_CS) {
            if (GPIOOverlays & BP_UTOPIA) {
                printk ("*************** ERROR ***************\n");
                printk ("Invalid GPIO configuration. SPI Extra CS cannot be enabled with UTOPIA\n");
            }
            /* Enable Extra SPI CS */
            GPIO->GPIOMode |= GROUP1_SPI_MASTER;
        }

#if defined(CONFIG_PCI)
        if (GPIOOverlays & BP_PCI) {
            /* Enable PCI interface */
            GPIO->GPIOMode |= GROUP2_PCI | GROUP1_MII_PCCARD;

            mpi_init();
            if (GPIOOverlays & BP_CB) {
                mpi_DetectPcCard();
            }
            else {
                /*
                 * CardBus support is defaulted to Slot 0 because there is no external
                 * IDSEL for CardBus.  To disable the CardBus and allow a standard PCI
                 * card in Slot 0 set the cbus_idsel field to 0x1f.
                */
                data = MPI->pcmcia_cntl1;
                data |= CARDBUS_IDSEL;
                MPI->pcmcia_cntl1 = data;
            }
        }
#endif
    }

#if defined(CONFIG_USB)
    PERF->blkEnables |= USBH_CLK_EN;
    mdelay(100);
    *USBH = USBH_BYTE_SWAP;
#endif

    return 0;
}

#elif defined(CONFIG_BCM96358)
int __init bcm_hw_init(void)
{
#if defined(CONFIG_PCI)
    unsigned long data;
#endif
    unsigned short GPIOOverlays;

    /* Enable SPI interface */
    PERF->blkEnables |= SPI_CLK_EN;

    GPIO->GPIOMode = 0;

    /* Enable LED Interface */
    GPIO->GPIOMode |= GPIO_MODE_LED_OVERLAY;
    GPIO->GPIODir |= 0x000f;

    /* Enable Serial LED Interface */
    GPIO->GPIOMode |= GPIO_MODE_SERIAL_LED_OVERLAY;
    GPIO->GPIODir |= 0x00c0;

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {

        if (GPIOOverlays & BP_UTOPIA) {
            /* Enable UTOPIA interface */
            GPIO->GPIOMode |= GPIO_MODE_UTOPIA_OVERLAY ;
            PERF->blkEnables |= SAR_CLK_EN;
        }

        if (GPIOOverlays & BP_UART1) {
            /* Enable secondary UART interface */
            GPIO->GPIOMode |= GPIO_MODE_UART1_OVERLAY;
            GPIO->GPIODir |= 0x20000000;
        }

        if (GPIOOverlays & BP_SPI_EXT_CS) {
            /* Enable Overlay for SPI SS Pins */
            GPIO->GPIOMode |= GPIO_MODE_SPI_SS_OVERLAY;
            /* Enable SPI Slave Select as Output Pins */
            /* GPIO 32 is SS2, GPIO 33 is SS3 */
            GPIO->GPIODir_high |= 0x0003;
        }

#if defined(CONFIG_PCI)
        if (GPIOOverlays & BP_PCI) {
            mpi_init();
            if (GPIOOverlays & BP_CB) {
                mpi_DetectPcCard();
            }
            else {
                /*
                 * CardBus support is defaulted to Slot 0 because there is no external
                 * IDSEL for CardBus.  To disable the CardBus and allow a standard PCI
                 * card in Slot 0 set the cbus_idsel field to 0x1f.
                */
                data = MPI->pcmcia_cntl1;
                data |= CARDBUS_IDSEL;
                MPI->pcmcia_cntl1 = data;
            }
        }
#endif
    }

#if defined(CONFIG_USB)
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->TestPortControl = 0x001c0020;
#endif

    return 0;
}
#endif

static int __init brcm63xx_setup(void)
{
    extern int panic_timeout;

    _machine_restart = brcm_machine_restart;
    _machine_halt = brcm_machine_halt;
    pm_power_off = brcm_machine_halt;
//    _machine_power_off = brcm_machine_halt;

    board_time_init = brcm_time_init;

    panic_timeout = 1;
    bcm_hw_init();
    return 0;
}
void __init plat_mem_setup(void)
{
    brcm63xx_setup();
}


/***************************************************************************
 * C++ New and delete operator functions
 ***************************************************************************/

/* void *operator new(unsigned int sz) */
void *_Znwj(unsigned int sz)
{
printk(KERN_EMERG "New. %d bytes\n",sz);
    return( kmalloc(sz, GFP_KERNEL) );
}

/* void *operator new[](unsigned int sz)*/
void *_Znaj(unsigned int sz)
{
printk(KERN_EMERG "New[] %d bytes\n",sz);
    return( kmalloc(sz, GFP_KERNEL) );
}

/* placement new operator */
/* void *operator new (unsigned int size, void *ptr) */
void *ZnwjPv(unsigned int size, void *ptr)
{
    return ptr;
}

/* void operator delete(void *m) */
void _ZdlPv(void *m)
{
    kfree(m);
}

/* void operator delete[](void *m) */
void _ZdaPv(void *m)
{
    kfree(m);
}

EXPORT_SYMBOL(_Znwj);
EXPORT_SYMBOL(_Znaj);
EXPORT_SYMBOL(ZnwjPv);
EXPORT_SYMBOL(_ZdlPv);
EXPORT_SYMBOL(_ZdaPv);

