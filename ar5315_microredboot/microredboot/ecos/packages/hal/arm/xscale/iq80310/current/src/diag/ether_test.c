//=============================================================================
//
//      ether_test.c - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#include <redboot.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#include "pci_bios.h"
#include "iq80310.h"
#include "ether_test.h"

/* Forward declarations */
static int i557SelfTest (void);
static int i557Init (void);
static int i557Config (UINT8 loopBackMode);
static int i557AddrSet (void);
static int i557RUStart (void);
static void setUpPacket (char *p);
static int txPacket (char *p);
static char *malloc (int n);
static int waitForRxInt(void);
static int get_ether_addr(int unit, UINT8 *buf, int print_flag);

/* Externals */
extern long decIn(void);
extern void sgets(char *s);
extern int enable_external_interrupt (int int_id);
extern int isr_connect(int int_num, void (*handler)(int), int arg);
extern STATUS pci_isr_connect (int intline, int bus, int device, int (*handler)(int), int arg);
extern void delay_ms(int msecs);

extern int eeprom_read (UINT32 pci_base,/* PCI Base address */
                 int eeprom_addr,       /* word offset from start of eeprom */
                 UINT16 *p_data,/* where to put data in memory */
                 int nwords             /* number of 16bit words to read */
                 );
extern int eeprom_write (UINT32 pci_base,/* PCI Base address */
                 int eeprom_addr,       /* word offset from start of eeprom */
                 UINT16 *p_data,/* data location in memory */
                 int nwords             /* number of 16bit words to write */
                 );

/* Globals needed by both main program and irq handler */
static volatile struct SCBtype *pSCB;	/* Pointer to SCB in use */
static volatile UINT32 waitSem;	/* Used to block test until interrupt */
static volatile UINT32 rxSem;	/* Used to block test until rx sinterrupt */
static UINT16 i557Status;	/* Status code from SCB */
static volatile char *mem_pool; /* Ptr to malloc's free memory pool */
static UINT32 adapter[2];		/* Ptr to PCI Ethernet adapter */
static UINT8 node_address[6];
/*static long timer0_ticks = 0;*/
static char buf[4];
static int count = 0;
static int forever_flag = FALSE;
static UINT32 phy_id = 0;

/* 82557 required data structures which must be allocated */
static struct rfd *pRfd;
static union cmdBlock *pCmdBlock;
static char *pPacketBuf;

#define SPEED_NOLINK	0
#define SPEED_10M	10
#define SPEED_100M	100
static int link_speed = SPEED_NOLINK;

UINT8 unit_intpin;
int unit_devno, unit_busno, unit_funcno;

#define BUSY_WAIT_LIMIT	 0xf000	 /* the upper limit on a busy wait
				    for command completion, etc. */

static void mask_557_ints (void)
{
    pSCB->cmdStat.bits.m = 1;
}

static void unmask_557_ints (void)
{
    pSCB->cmdStat.bits.m = 0;
}

/*****************************************************************************
* pci_ether_test - i8255x PCI Ethernet test
*
* Main diagnostic routine for the Intel 8255x 10/100BaseT Ethernet Controller
* family.  Arguments include the PCI bus, device and function numbers of the
* controller that is to be tested.
*
*/
void pci_ether_test (UINT32 busno, UINT32 devno, UINT32 funcno)
{
    volatile int i;
    int ntimes;
    int broadcom_flag =  FALSE;
    UINT16 phy_addr_reg, temp1, temp2;
    cyg_pci_device_id  devid;

    count = 0;

    devid = CYG_PCI_DEV_MAKE_ID(busno, CYG_PCI_DEV_MAKE_DEVFN(devno,funcno));

    /* read the PCI BAR for the Ethernet controller */
    cyg_pci_read_config_uint32(devid, 0x10, &adapter[0]);

    /* strip off BAR indicator bits */
    adapter[0] &= 0xfffffff0;

    unit_devno	= devno;
    unit_busno	= busno;
    unit_funcno = funcno;

    /* pointer to on-chip SCB */
    pSCB = (struct SCBtype *)(adapter[0] + SCB_OFFSET);

    unit_intpin = INTA;

    printf ("PCI Base Address  = 0x%X\n", adapter[0]);
    printf ("PCI Interrupt Pin = 0x%02X\n", unit_intpin);

    /* Initialize malloc's memory pool pointer */
    mem_pool = (char *) ETHER_MEM_POOL;
	
    /* Start the timer for delay implementation 
       printf("Starting timer... ");
       StartTimer(); */
    printf("Done.\n Resetting chip... ");

    /* reset the 82557 to start with a clean slate */
    resetChip();
    printf("Done.\n");

    /* Get the UUT's ethernet address */
    if (get_ether_addr (0, node_address, TRUE) == ERROR) {
	printf("Error Reading Adapter Ethernet Address\n");
	return;
    }

    temp1 = readMDI(0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_ID_1);
    temp2 = readMDI(0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_ID_2);
    phy_id = ((temp1 << 16) | temp2);

    if ((phy_id & 0xfffffff0) == I82555_PHY_ID)	{
	printf ("Intel 82555/558 PHY detected...\n");

	/* dummy read for reliable status */
	(void)readMDI (0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_STAT);
    
	temp1 = readMDI (0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_STAT);
	printf ("Status Register Link Status is %s\n", (temp1 & MDI_STAT_LINK) ? "UP" : "DOWN");

	phy_addr_reg = readMDI (0, MDI_DEFAULT_PHY_ADDR, I82555_STATCTRL_REG);

	if (temp1 & MDI_STAT_LINK) {	/* speed only valid with good LNK */
	    printf ("Connect Speed is %s\n", (phy_addr_reg & I82555_100_MBPS) ? "100Mbps" : "10Mbps");
	    link_speed = (phy_addr_reg & I82555_100_MBPS) ? SPEED_100M : SPEED_10M;
	} else
	    printf ("Connect Speed is NOT VALID\n");
    }

    if ((phy_id & 0xfffffff0) == ICS1890_PHY_ID) {
	printf ("Integrated Circuit Systems ICS1890 PHY detected...\n");
	printf ("Revision = %c\n", 'A' + (phy_id & REVISION_MASK));

	/* dummy read for reliable status */
	(void)readMDI (0, MDI_DEFAULT_PHY_ADDR, ICS1890_QUICKPOLL_REG);
	temp1 = readMDI (0, MDI_DEFAULT_PHY_ADDR, ICS1890_QUICKPOLL_REG);
	printf ("Status Register Link Status is %s\n", (temp1 & QUICK_LINK_VALID) ? "UP" : "DOWN");

	if (temp1 & QUICK_LINK_VALID) { /* speed only valid with good LNK */
	    printf ("Connect Speed is %s\n", (temp1 & QUICK_100_MBPS) ? "100Mbps" : "10Mbps");
	    link_speed = (temp1 & QUICK_100_MBPS) ? 
		SPEED_100M : SPEED_10M;
	} else
	    printf ("Connect Speed is NOT VALID\n");
    }

    if ((phy_id & 0xfffffff0) == DP83840_PHY_ID) {
	printf ("National DP83840 PHY detected...\n");
	printf ("Revision = %c\n", 'A' + (phy_id & REVISION_MASK));

	/* dummy read for reliable status */
	(void)readMDI (0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_STAT);
	temp1 = readMDI (0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_STAT);
	printf ("Status Register Link Status is %s\n", (temp1 & MDI_STAT_LINK) ? "UP" : "DOWN");

	phy_addr_reg = readMDI (0 ,MDI_DEFAULT_PHY_ADDR, DP83840_PHY_ADDR_REG);

	if (temp1 & MDI_STAT_LINK) {	/* speed only valid with good LNK */
	    printf ("Connect Speed is %s\n", (phy_addr_reg & PHY_ADDR_SPEED_10_MBPS) ? "10Mbps" : "100Mbps");
	    link_speed = (phy_addr_reg & PHY_ADDR_SPEED_10_MBPS) ? SPEED_10M : SPEED_100M;
	}
	else printf ("Connect Speed is NOT VALID\n");
    }

    if ((phy_id & 0xfffffff0) == I82553_PHY_ID) {
	printf ("Intel 82553 PHY detected...\n");
	printf ("Revision = %c\n", 'A' + (phy_id & REVISION_MASK));
	broadcom_flag = TRUE;
    }

    if (phy_id == I82553_REVAB_PHY_ID) {
	printf ("Intel 82553 PHY detected...\n");
	printf ("Revision = B\n");
	broadcom_flag = TRUE;
    }

    if (broadcom_flag == TRUE) {
	temp2 = readMDI (0,MDI_DEFAULT_PHY_ADDR, I82553_PHY_EXT_REG0);
	printf ("Stepping = %02X\n", GET_REV_CNTR(temp2));

	/* dummy read for reliable status */
	(void)readMDI (0 ,MDI_DEFAULT_PHY_ADDR, MDI_PHY_STAT);
	temp1 = readMDI (0 ,MDI_DEFAULT_PHY_ADDR, MDI_PHY_STAT);
	printf ("Status Register Link Status is %s\n", (temp1 & MDI_STAT_LINK) ? "UP" : "DOWN");

	if (temp1 & MDI_STAT_LINK) {	/* speed only valid with good LNK */
	    printf ("Connect Speed is %s\n", (temp2 & EXT_REG0_100_MBPS) ? "100Mbps" : "10Mbps");
	    link_speed = (temp2 & EXT_REG0_100_MBPS) ? SPEED_100M : SPEED_10M;
	} else
	    printf ("Connect Speed is NOT VALID\n");
    }
    printf ("\n");

    /* Run the built-in self test through the port register */
    if (i557SelfTest () == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    /* Reset clears the interrupt mask */
    mask_557_ints();

    printf ("Press return to initialize ethernet controller.\n");
    sgets (buf);

    /* Initialize data structures */
    if (i557Init () == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    /* Set hardware address */
    if (i557AddrSet () == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    printf ("Press return to perform internal loopback test.\n");
    sgets (buf);

    /* Configure for internal loopback */
    if (i557Config (INT_LOOP_BACK) == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    delay_ms(100);

    /* Initialize receive buffer and enable receiver */
    if (i557RUStart () == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    /* Send a packet */
    setUpPacket (pPacketBuf);
    if (txPacket (pPacketBuf) == ERROR)	{
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    printf ("Press return to perform loopback through PHY.\n");
    sgets (buf);

    /* Configure for external loopback */
    if (i557Config (EXT_LOOP_BACK) == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    delay_ms(100);

    /* Initialize receive buffer and enable receiver */
    if (i557RUStart () == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    /* Send a packet */
    setUpPacket (pPacketBuf);
    if (txPacket (pPacketBuf) == ERROR)	{
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    printf ("Press return to perform external loopback through\n");
    printf ("10/100 Base T Hub.  NOTE: If test duration is not forever,\n");
    printf ("this test will work only if a properly functioning Hub\n"); 
    printf ("and Twisted Pair cable are attached to the network connector\n");
    printf ("on the front panel.\n");
    sgets (buf);

    printf ("Enter the number of times to run test (0 = forever): ");
    ntimes = decIn();
    printf ("\n\n");

    if (i557RUStart () == ERROR) {
	mask_557_ints ();      /* Disable 557 interrupt */
	return;
    }

    setUpPacket (pPacketBuf);

    if (ntimes == 0) {
	forever_flag = TRUE;

	while (1) {
	    if ((i557RUStart() == ERROR)||(txPacket (pPacketBuf) == ERROR)) {
		printf ("Double-check TP cable and 10/100 Base T Hub\n");
		printf ("Try testing them with another system\n");
		printf ("(such as a workstation) that is working correctly.\n");
		mask_557_ints ();      /* Disable 557 interrupt */
		return;
	    }

	    count++;
	    if (((count) % 1000) == 0) 
		printf("Loopback Cycle Count = %d\n", count);
	}
    } else {
	forever_flag = FALSE;

	for (i=0; i<ntimes; i++) {
	    if ((i557RUStart() == ERROR)||(txPacket (pPacketBuf) == ERROR)) {
		printf ("Double-check TP cable and 10/100 Base T Hub\n");
		printf ("Try testing them with another system\n");
		printf ("(such as a workstation) that is working correctly.\n");
		mask_557_ints ();      /* Disable 557 interrupt */
		return;
	    }
	  
	    count++;
	    printf("Loopback Cycle Count = %d\n", count);
	}

	/* It worked! */

	mask_557_ints ();       /* Disable 557 interrupt */

	printf ("\nEthernet controller passed.  Press return to continue.\n");

	sgets (buf);
    }
}


/* Perform internal self test - returns OK if sucessful, ERROR if not. */
static int i557SelfTest ()
{
    volatile struct selfTest *pSelfTestMem;
    UINT32 oldWord2;
    long delay;
    UINT32 temp;
    int	rtnVal;

    /* reset the 82557 to start with a clean slate */
    resetChip();

    /* Allocate some memory for the self test */
    pSelfTestMem = (struct selfTest *) malloc (sizeof(struct selfTest));

    if (pSelfTestMem == NULL) {
	printf ("Couldn't get memory for self test.\n");
	return (ERROR);
    }

    printf ("Sending PORT* self-test command...\n");
    printf ("Local Dump address = 0x%X\n", pSelfTestMem);

    /* Set all bits in  second word, wait until it changes or a timeout */
    pSelfTestMem->u.word2 = ~0;
    oldWord2 = pSelfTestMem->u.word2;

    temp = ((UINT32) pSelfTestMem) + PORT_SELF_TEST;

    portWrite (temp);

    /* Wait for test completion or for timeout */
    for (delay = 0; (delay < MAX_DELAY) && (pSelfTestMem->u.word2 == oldWord2); delay++)
	;	/* Wait... */

    /* Print results */
    printf ("Self test result: %s\n", (pSelfTestMem->u.bits.selfTest) ? "Fail" : "Pass");
    printf ("ROM content test: %s\n", (pSelfTestMem->u.bits.romTest) ? "Fail" : "Pass");
    printf ("Register test:    %s\n", (pSelfTestMem->u.bits.regTest) ? "Fail" : "Pass");
    printf ("Diagnose test:    %s\n", (pSelfTestMem->u.bits.diagnTest) ? "Fail" : "Pass");
    printf ("ROM signature:    0x%X\n", pSelfTestMem->romSig);

    rtnVal = pSelfTestMem->u.bits.selfTest ? ERROR : OK;

    return (rtnVal);
}


/* Initialize the 82557. */
static int i557Init (void)
{
    /* Get memory for system data structures */
    if ( ((pRfd = (struct rfd *) malloc (sizeof(struct rfd))) == NULL) || 
	 ((pPacketBuf = malloc(ETHERMTU + sizeof(UINT16) + 6)) == NULL) ||
	 ((pCmdBlock = (union cmdBlock *) malloc (sizeof(union cmdBlock))) == NULL) ) {
	printf ("Memory allocation failed.\n");
	return (ERROR);
    }

    /* Set EL bits in command block and rfd so we don't fall of the end */
    pCmdBlock->nop.el = END_OF_LIST;
    pRfd->el = END_OF_LIST;

    /* Reset chip and initialize */
    printf ("Initializing... ");

    /* Reset 82557 */
    resetChip ();
		
    /* set up the CU and RU base values to 0x0 */
    sendCommand (LOAD_CU_BASE, RU_NOP, 0);
    sendCommand (CU_NOP, LOAD_RU_BASE, 0);
	
    /* Initialize interrupts */
	
    /* if it is the onboard i82559, it does not use the conventional PCI
       interrupt routines because the interrupt is not multiplexed onto
       the PCI bus */
    if ((unit_busno == 2) && (unit_devno == 0) && (unit_funcno == 0)) {
	if (isr_connect (ENET_INT_ID, (VOIDFUNCPTR)i557IntHandler, 0xdeadbeef) != OK) {
	    printf ("Error connecting Ethernet interrupt!\n");
	    return (ERROR);
	}
	if (enable_external_interrupt (ENET_INT_ID) != OK) {
	    printf ("Error enabling Ethernet interrupt!\n");
	    return (ERROR);
	}
    } else {  /* use regular PCI int connect scheme */
	if (pci_isr_connect (unit_intpin, unit_busno, unit_devno, i557IntHandler, 0xdeadbeef) != OK) {
	    printf ("Error connecting Ethernet interrupt!\n");
	    return (ERROR);
	}
    }
    unmask_557_ints();
    printf ("Done\n");
    return (OK);
}


static int initPHY (UINT32 device_type, int loop_mode)
{
    UINT16 temp_reg;
    UINT8 revision;

    /* strip off revision and phy. id information */ 
    revision = (UINT8)(device_type & REVISION_MASK);
    device_type &= ~REVISION_MASK;

    switch (device_type) {
    case ICS1890_PHY_ID: 
	temp_reg = readMDI (0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_CTRL);  /* get ready for loopback setting */
	    
	switch (loop_mode) {
	case EXT_LOOP_BACK:  /* loopback on the MII interface */
	    temp_reg |= MDI_CTRL_LOOPBACK;	/* MII loopback */
	    break;

	case INT_LOOP_BACK:
	default:
	    break;
	}
    	    
	writeMDI(0, MDI_DEFAULT_PHY_ADDR, MDI_PHY_CTRL, temp_reg);
	break;
		
    case DP83840_PHY_ID:  /* set the Intel-specified "must set" bits */
	temp_reg = readMDI (0,MDI_DEFAULT_PHY_ADDR, DP83840_PCR_REG);
	temp_reg |= (PCR_TXREADY_SEL | PCR_FCONNECT);
	writeMDI (0,MDI_DEFAULT_PHY_ADDR, DP83840_PCR_REG, temp_reg);

	/* get ready for loopback setting */
	temp_reg = readMDI (0,MDI_DEFAULT_PHY_ADDR, DP83840_LOOPBACK_REG);
	temp_reg &= CLEAR_LOOP_BITS;

	switch (loop_mode) {
	case EXT_LOOP_BACK:
	    temp_reg |= TWISTER_LOOPBACK;
	    break;

	case INT_LOOP_BACK:
	default:
	    break;
	}

	writeMDI (0,MDI_DEFAULT_PHY_ADDR, DP83840_LOOPBACK_REG, temp_reg);
	break;
			
    case I82553_PHY_ID:
    case I82553_REVAB_PHY_ID:
    case I82555_PHY_ID:
	break;

    default:
	return (ERROR);
	break;
    }

    return (OK);
}


/* Set hardware address of the 82557. */
static int i557AddrSet ()
{
    printf ("Setting hardware ethernet address to ");
    printf ("%02X:%02X:%02X:", node_address[0], node_address[1], node_address[2]);
    printf ("%02X:%02X:%02X... ", node_address[3], node_address[4], node_address[5]);

    /* Set up iaSetup command block and execute */
    memset((char *) pCmdBlock, 0, sizeof(union cmdBlock));
    pCmdBlock->iaSetup.code = IA_SETUP;
    pCmdBlock->iaSetup.el = END_OF_LIST;
    memcpy(pCmdBlock->iaSetup.enetAddr, node_address, sizeof(node_address));

    sendCommand (CU_START, RU_NOP, ((UINT32)pCmdBlock));

    if ((waitForInt() == ERROR) || (pCmdBlock->iaSetup.ok != 1)) {
	printf ("failed.  Status: 0x%04X.\n", pSCB->cmdStat.words.status);
	printf ("C bit = %d\n",pCmdBlock->iaSetup.c);
	printf ("OK bit = %d\n",pCmdBlock->iaSetup.ok);
	return (ERROR);
    }
    
    printf ("done.\n");
    return (OK);
}


/* Configure the 82557. */
static int i557Config (UINT8 loopBackMode)	/* None, int, or ext 1, 2 (see etherTest.h) */
{
    printf ("\nConfiguring for ");

    switch (loopBackMode) {
    case INT_LOOP_BACK:
	printf ("internal loopback... ");
	break;
    case EXT_LOOP_BACK:
	printf ("external loopback, LPBK* active... ");
	break;
    default:
	printf ("Unknown loopback mode, exiting...\n");
	return (ERROR);
    }

    /* Set up configure command block and execute */
    memset ((char *) pCmdBlock, 0, sizeof(union cmdBlock));
    pCmdBlock->configure.code = CONFIGURE;
    pCmdBlock->configure.el = END_OF_LIST;
    pCmdBlock->configure.configData[ 0] = CONFIG_BYTE_00;
    pCmdBlock->configure.configData[ 1] = CONFIG_BYTE_01;
    pCmdBlock->configure.configData[ 2] = CONFIG_BYTE_02;
    pCmdBlock->configure.configData[ 3] = CONFIG_BYTE_03;
    pCmdBlock->configure.configData[ 4] = CONFIG_BYTE_04;
    pCmdBlock->configure.configData[ 5] = CONFIG_BYTE_05;
    pCmdBlock->configure.configData[ 6] = CONFIG_BYTE_06;
    pCmdBlock->configure.configData[ 7] = CONFIG_BYTE_07;
    pCmdBlock->configure.configData[ 8] = CONFIG_BYTE_08;
    pCmdBlock->configure.configData[ 9] = CONFIG_BYTE_09;
    pCmdBlock->configure.configData[10] = CONFIG_BYTE_10 | loopBackMode;
    pCmdBlock->configure.configData[11] = CONFIG_BYTE_11;
    pCmdBlock->configure.configData[12] = CONFIG_BYTE_12;
    pCmdBlock->configure.configData[13] = CONFIG_BYTE_13;
    pCmdBlock->configure.configData[14] = CONFIG_BYTE_14;
    pCmdBlock->configure.configData[15] = CONFIG_BYTE_15;
    pCmdBlock->configure.configData[16] = CONFIG_BYTE_16;
    pCmdBlock->configure.configData[17] = CONFIG_BYTE_17;
    pCmdBlock->configure.configData[18] = CONFIG_BYTE_18;

    if (link_speed == SPEED_100M) 
	pCmdBlock->configure.configData[19] = CONFIG_BYTE_19_100T;
    else {
	pCmdBlock->configure.configData[19] = CONFIG_BYTE_19_10T;
	pCmdBlock->configure.configData[20] = CONFIG_BYTE_20;
	pCmdBlock->configure.configData[21] = CONFIG_BYTE_21;
    }

    sendCommand (CU_START, RU_NOP, ((UINT32)pCmdBlock));

    if ((waitForInt() == ERROR) || (pCmdBlock->configure.ok != 1)) {
	printf ("failed.  Status: 0x%04X.\n", pSCB->cmdStat.words.status);
	return (ERROR);
    }

    initPHY (phy_id, loopBackMode);  /* set up the PHY interface appropriately */

    printf ("done.\n");
    return (OK);
}


static int i557RUStart (void)
{
    volatile long delay;

    memset((char *) pRfd, 0, sizeof(struct rfd));

    /* Set end-of-list bit in the rfd so we don't fall off the end */
    pRfd->el = END_OF_LIST;
    pRfd->s = 1;
    pRfd->sf = 0;				/* Simplified mode */
    pRfd->rbdAddr = (UINT8 *) 0xffffffff; 	/* No RBD */
    /* buffer size: */
    pRfd->size = sizeof (pRfd->rxData) + sizeof (pRfd->destAddr) + sizeof (pRfd->sourceAddr) + sizeof (pRfd->length); 

    sendCommand (CU_NOP, RU_START, ((UINT32)pRfd));

    /*
     * Poll, can't use waitForInt (), as this step doesn't generate interrupts.
     */

    i557Status = 0;

    /* Wait for timeout (i557Status changes) or RU_STAT is RU_READY */
    for (delay = 0; (delay < MAX_DELAY) && (pSCB->cmdStat.bits.rus != RU_READY); delay++)
	;	/* Wait... */

    if (pSCB->cmdStat.bits.rus != RU_READY) {
	printf ("failed.  Status: 0x%04X.\n", pSCB->cmdStat.words.status);
	return (ERROR);
    }

    return (OK);
}


/*
 * Get packet ready to send out over the network.  Buffer should be
 * ETHERMTU + sizeof(enet_addr) + sizeof(UINT16)
 */
static void setUpPacket (char *pBuf)/* Where to put it */
{
    memcpy (pBuf, node_address, sizeof(node_address));
    pBuf += sizeof(node_address); /* skip dest. address */
    
    *((UINT16 *) pBuf) = 0;
    pBuf += sizeof(UINT16);      /* skip length field */

    makePacket (pBuf, ETHERMTU);
}


/* Send and verify a packet using the current loopback mode. */
static int txPacket (char *pBuf) /* Dest addr, ethertype, buffer */
{
    int status = OK;

    /* Set up transmit command block and execute */
    memset((char *) pCmdBlock, 0, sizeof(union cmdBlock));
    pCmdBlock->transmit.code = TRANSMIT;
    pCmdBlock->transmit.el = END_OF_LIST;
    pCmdBlock->transmit.sf = 0;				/* Simplified mode */
    pCmdBlock->transmit.tbdAddr = (UINT8 *) 0xffffffff; /* No TBD */
    pCmdBlock->transmit.eof = 1;			/* Entire frame here */
    /* # bytes to tx: */
    pCmdBlock->transmit.tcbCount = sizeof (pCmdBlock->transmit.destAddr) + sizeof (pCmdBlock->transmit.length) +
	sizeof (pCmdBlock->transmit.txData);

#if 0
    printf ("destAddr size = %d\n", sizeof (pCmdBlock->transmit.destAddr));
    printf ("length size = %d\n", sizeof (pCmdBlock->transmit.length));
    printf ("Transmitting %d bytes\n", pCmdBlock->transmit.tcbCount);
#endif

    memcpy (pCmdBlock->transmit.destAddr, pBuf, sizeof(node_address) + sizeof(UINT16) + ETHERMTU);

    rxSem = 0;	/* no Receive interrupt */

    sendCommand (CU_START, RU_NOP, ((UINT32)pCmdBlock));

    if (waitForInt() == ERROR) {
	printf ("No Transmit Interrupt\n");
	status = ERROR;
    }

    if (pCmdBlock->transmit.ok != 1) {	
	printf ("tx failed.  Status: 0x%04X.\n",
		pSCB->cmdStat.words.status);
	status = ERROR;
    }

    if (status == ERROR) {
	printf ("Transmit OK = %d\n", pCmdBlock->transmit.ok);
	return (ERROR);
    }

#if 1
    if (waitForRxInt() == ERROR) {
	printf ("No Receive Interrupt\n");
	status = ERROR;
    }

    if (pRfd->ok != 1) {
	printf ("rx failed.  Status: 0x%04X.\n", pSCB->cmdStat.words.status);
	status = ERROR;
    }

#if 1
    /* If RU still ready, hang for receive interrupt */
    if (pSCB->cmdStat.bits.rus == RU_READY)	{
	if (waitForRxInt() == ERROR)  {
	    printf ("No Receive Interrupt\n");
	    status = ERROR;
	}

	if (pRfd->ok != 1) {
	    printf ("rx failed.  Status: 0x%04X.\n", pSCB->cmdStat.words.status);
	    status = ERROR;
	}
    }
#endif

    if (status == ERROR) {
	printf ("\nTransmit Stats:\n");
	printf ("---------------\n");
	printf ("Transmit OK = %d\n", pCmdBlock->transmit.ok);

	printf ("\nReceive Stats:\n");
	printf ("---------------\n\n");
	printf ("Receive OK = %d\n", pRfd->ok);
	printf ("CRC Error = %d\n", pRfd->crcErr);
	printf ("Alignment Error = %d\n", pRfd->alignErr);
	printf ("Resource Error = %d\n", pRfd->noRsrc);
	printf ("DMA Overrun Error = %d\n", pRfd->dmaOverrun);
	printf ("Frame Too Short Error = %d\n", pRfd->frameTooshort);
	printf ("Receive Collision Error = %d\n", pRfd->rxColl);
	return (ERROR);
    }

#if 0
    printf ("Packet Actual Size = %d\n", pRfd->actCount);
#endif

    if (checkPacket (pCmdBlock->transmit.txData, pRfd->rxData, ETHERMTU) == ERROR) {
	printf ("data verify error.\n");
	return (ERROR);
    }
    
    if (forever_flag == FALSE)
	printf ("data OK.\n");
#endif

    return (OK);
}


/*
 * "Poor Man's Malloc" - return a pointer to a block of memory at least
 * The block returned will have been zeroed.
 */
static char *malloc (int numBytes) /* number of bytes needed */
{
    volatile char *rtnPtr;	/* ptr to return to caller */
    long	new_mem_pool;	/* For figuring new pool base address */

    rtnPtr = mem_pool;	/* Return pointer to start of free pool */

    /* Now calculate new base of free memory pool (round to >= 16 bytes) */
    new_mem_pool = (UINT32) mem_pool;
    new_mem_pool = ((new_mem_pool + numBytes + 0x10) & (~((UINT32) 0x0f)));
    mem_pool = (volatile char *) new_mem_pool;

    memset(rtnPtr, 0, numBytes);

    return ((char *) rtnPtr);
}


/*
 * Write "value" to PORT register of PRO/100.
 */
static void portWrite (UINT32 value)
{
    *PORT_REG(adapter[0]) = value;
}


/******************************************************************************
*
* sendCommand - send a command to the 82557 via the on-chip SCB
*
* Send a command to the 82557.  On the 82557, the Channel Attention signal
* has been replaced by an on-chip SCB.  Accesses to the Command Word portion
* of the SCB automatically forces the '557 to look at the various data
* structures which make up its interface.
*/
static void sendCommand (UINT8 cuc, UINT8 ruc, UINT32 scb_general_ptr)
{
    register CMD_STAT_U temp_cmdStat;
    volatile union cmdBlock *pBlock = (union cmdBlock *)scb_general_ptr;
    volatile int loop_ctr;
	
    /* Mask adapter interrupts to prevent the interrupt handler from
       playing with the SCB */
    mask_557_ints();

    /* must wait for the Command Unit to become idle to prevent
       us from issueing a CU_START to an active Command Unit */
    for (loop_ctr = BUSY_WAIT_LIMIT; loop_ctr > 0; loop_ctr--) {
        if ((pSCB->cmdStat.words.status & SCB_S_CUMASK) == SCB_S_CUIDLE)
            break;
    }
    if (loop_ctr == 0) {
        printf("sendCommand: CU won't go idle, command ignored\n");
	unmask_557_ints();
        return;
    }

    /* when setting the command word, read the current word from
       the SCB and preserve the upper byte which contains the interrupt
       mask bit */
    temp_cmdStat.words.command = (pSCB->cmdStat.words.command & 0xff00);
    temp_cmdStat.words.status = 0;

    /* set up the Command and Receive unit commands */
    temp_cmdStat.bits.cuc = cuc & 0x07;
    temp_cmdStat.bits.ruc = ruc & 0x07;

    /* Clear flag */
    waitSem = 0;

    /* write the General Pointer portion of the SCB first */
    pSCB->scb_general_ptr = scb_general_ptr;

    /* write the Command Word of the SCB */
    pSCB->cmdStat.words.command = temp_cmdStat.words.command;

    /* only wait for command which will complete immediately */
    if ((scb_general_ptr != 0/* NULL*/) && (ruc != RU_START)) {
	/* wait for command acceptance and completion */
	for (loop_ctr = BUSY_WAIT_LIMIT; loop_ctr > 0; loop_ctr--) {
	    if ((pSCB->cmdStat.bits.cuc == 0) && (pBlock->nop.c == 1))
		break;
	}
	if (loop_ctr == 0) {
	    printf("sendCommand: Timeout on command complete\n");
	    printf("Cmd Complete bit = %02X\n", pBlock->nop.c);
	    printf("CU command       = 0x%02X\n", cuc);
	    printf("RU command       = 0x%02X\n", ruc);
	    printf("SCB Gen Ptr      = 0x%X\n", scb_general_ptr);
	    printf("scb status       = 0x%04X\n", pSCB->cmdStat.words.status);
	    printf("scb command      = 0x%04X\n", pSCB->cmdStat.words.command);
	}
    }

#if 0
    /* DEBUG */
    printf("scb command = 0x%04X\n", pSCB->cmdStat.words.command);
    printf("scb status  = 0x%04X\n", pSCB->cmdStat.words.status);
#endif

    unmask_557_ints();
    return;
}


/*
 * Do a port reset on 82557.
 */
static void resetChip (void)
{
    portWrite (PORT_RESET);	/* bits 4-31 not used for reset */

    /* wait 5 msec for device to stabilize */
    delay_ms(5);
}


/*
 * Setup contents of a packet.
 */
static void makePacket (UINT8 *pPacket, int length)
{
    int byteNum;	/* Current byte number */

    for (byteNum = 0; byteNum < length; byteNum++)
	*pPacket++ = byteNum + ' ';
}


/*
 * Verify contents of a received packet to what was transmitted.
 * Returns OK if they match, ERROR if not.
 */
static int checkPacket (UINT8 *pTxBuffer, UINT8 *pRxBuffer, int length)
{
    int byteNum;	/* Current byte number */

     for (byteNum = 0; byteNum < length; byteNum++) {
	 if (*pTxBuffer++ != *pRxBuffer++) {
	     printf("Error at byte 0x%x\n", byteNum);
	     printf("Expected 0x%02X, got 0x%02X\n", *(pTxBuffer - 1), *(pRxBuffer - 1));
	     return (ERROR);
	 }
     }
     return (OK);
}


/*
 * Interrupt handler for i82557.  It acknowledges the interrupt
 * by setting the ACK bits in the command word and issuing a
 * channel attention.  It then updates the global status variable
 * and gives the semaphore to wake up the main routine.
 */
int i557IntHandler (int arg)  /* should return int */
{
    register CMD_STAT_U temp_cmdStat;
    register int rxFlag = FALSE;

    temp_cmdStat.words.status = pSCB->cmdStat.words.status;

    /* check to see if it was the PRO/100 */
    if (temp_cmdStat.words.status & I557_INT) {
	/* Wait for command word to clear - indicates no pending commands */
	while (pSCB->cmdStat.words.command)
	    ;

	/* Update global status variable */
	i557Status = temp_cmdStat.words.status;

	/* If the interrupt was due to a received frame... */
	if (temp_cmdStat.bits.statack_fr)
	    rxFlag = TRUE;

	temp_cmdStat.words.status = temp_cmdStat.words.status & I557_INT;

	/* Acknowledge interrupt by setting ack bits */
	pSCB->cmdStat.words.status = temp_cmdStat.words.status;

	/* Wait for command word to clear - indicates IACK accepted */
	while (pSCB->cmdStat.words.command)
	    ;

	/* Update global status variable and unblock task */
	waitSem = 1;

	if (rxFlag == TRUE)
	    rxSem = 1;

	return(1);		/* serviced - return 1 */
    }
    return(0);			/* nothing serviced - return 0 */
}



/*
 * Take the semaphore and block until i557 interrupt or timeout.
 * Returns OK if an interrupt occured, ERROR if a timeout.
 */
static int waitForInt(void)
{
    int num_ms = 0; 

    while ((waitSem == 0) && (num_ms != 2000)) { /* wait max 2secs for the interrupt */
	delay_ms(1);
	num_ms++;
    }

    if (!waitSem) {
	printf("Wait error!\n"); 
	return (ERROR);
    } else
	return (OK);
}


static int waitForRxInt(void)
{
    int num_ms = 0; 

    while ((rxSem == 0) && (num_ms != 2000)) { /* wait max 2secs for the interrupt */
	delay_ms(1);
	num_ms++;
    }

    if (!rxSem) {
	printf("Rx Wait error!\n"); 
	return (ERROR);
    } else
	return (OK);
}


static UINT16 readMDI (int unit, UINT8	phyAdd, UINT8	regAdd)
{
    register MDI_CONTROL_U mdiCtrl;
    int num_ms = 0; 

    /* prepare for the MDI operation */
    mdiCtrl.bits.ready = MDI_NOT_READY;
    mdiCtrl.bits.intEnab = MDI_POLLED;	/* no interrupts */
    mdiCtrl.bits.op = MDI_READ_OP;
    mdiCtrl.bits.phyAdd = phyAdd & 0x1f;
    mdiCtrl.bits.regAdd = regAdd & 0x1f;

    /* start the operation */
    *MDI_CTL_REG(adapter[unit]) = mdiCtrl.word;

    /* delay a bit */
    delay_ms(1);

    /* poll for completion */
    mdiCtrl.word = *MDI_CTL_REG(adapter[unit]);

    while ((mdiCtrl.bits.ready == MDI_NOT_READY) && (num_ms != 2000)) { /* wait max 2secs */
	mdiCtrl.word = *MDI_CTL_REG(adapter[unit]);
	delay_ms(1);
	num_ms++;
    }	
	
    if (num_ms >= 2000)	{
	printf ("readMDI Timeout!\n");
	return (-1);
    } else 
	return ((UINT16)mdiCtrl.bits.data);
}


static void writeMDI (int unit, UINT8 phyAdd, UINT8 regAdd, UINT16 data)
{
    register MDI_CONTROL_U mdiCtrl;
    int num_ms = 0;

    /* prepare for the MDI operation */
    mdiCtrl.bits.ready = MDI_NOT_READY;
    mdiCtrl.bits.intEnab = MDI_POLLED;  /* no interrupts */
    mdiCtrl.bits.op = MDI_WRITE_OP;
    mdiCtrl.bits.phyAdd = phyAdd & 0x1f;
    mdiCtrl.bits.regAdd = regAdd & 0x1f;
    mdiCtrl.bits.data = data & 0xffff;

    /* start the operation */
    *MDI_CTL_REG(adapter[unit]) = mdiCtrl.word;

    /* delay a bit */
    delay_ms(1);

    /* poll for completion */
    mdiCtrl.word = *MDI_CTL_REG(adapter[unit]);

    while ((mdiCtrl.bits.ready == MDI_NOT_READY) && (num_ms != 2000)) {
	mdiCtrl.word = *MDI_CTL_REG(adapter[unit]);
	delay_ms(1);
	num_ms++;
    }
    if (num_ms >= 2000)
	printf ("writeMDI Timeout!\n");
 
    return;
}

static int get_ether_addr (int     unit,
			   UINT8   *buffer,
			   int     print_flag)  /* TRUE to print the information */
{
    UINT16 temp_node_addr[3] = {0,0,0};
    register int i;

    /* Get the adapter's node address */
    if (eeprom_read (adapter[unit],IA_OFFSET,temp_node_addr,3) != OK) {
	printf ("Error reading the IA address from Serial EEPROM.\n");
	return (ERROR);
    }
 
    buffer[0] = (UINT8)(temp_node_addr[0] & 0x00ff);
    buffer[1] = (UINT8)((temp_node_addr[0] & 0xff00)>>8);
    buffer[2] = (UINT8)(temp_node_addr[1] & 0x00ff);
    buffer[3] = (UINT8)((temp_node_addr[1] & 0xff00)>>8);
    buffer[4] = (UINT8)(temp_node_addr[2] & 0x00ff);
    buffer[5] = (UINT8)((temp_node_addr[2] & 0xff00)>>8);
 
    if (print_flag == TRUE) {
        printf("Ethernet Address = [ ");
        for (i=0; i<6; i++) {
            printf("0x%02X ", buffer[i]);
        }
        printf("]\n\n");
    }
    return (OK);
}

