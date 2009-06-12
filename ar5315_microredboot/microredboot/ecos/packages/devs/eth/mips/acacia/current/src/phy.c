#include <stdlib.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <string.h>

#include "ethacacia.h"
#include "phy.h"
#include "acacia_ecos.h"
#include "acacia_mac.h"

#define ERROR -1
#define OK 0
#define AUTO_NEGOTIATE    

/* microsecond timeouts for hardware operations */
#define PHY_RD_TIMEOUT       500         /* PHY operation timeout */
#define PHY_WR_TIMEOUT       20000       /* PHY operation timeout */

/***************************************************************************
 *
 * idt32438PhyInit - initialize PHY device
 *
 * Starts Auto Negotiation if enabled
 *
 * RETURNS: OK or ERROR.
 *
 */

void idt32438PhyInit(void) 
{
	int phyAddr;
	
#ifdef  AUTO_NEGOTIATE    
	/* Start the Autonegotiation process on PHY0*/

	phyAddr = (PHY0_ADDR << 8) & 0x1F00;
        /* Write the (phyAddr, PHY_CONTROL_REG) to MIIMADDR register and write
         * the PHY_AUTONEG_EN in the MIIWTD register
         */
        writel (( phyAddr | (PHY_CONTROL_REG & 0x1F)), IDT_MIIMADDR_REG);
        writel (((PHY_AUTONEG_EN | PHY_RESTART_AN) & 0xFFFF), IDT_MIIMWTD_REG);

        /* Start the Autonegotiation process on PHY1*/
        phyAddr = (PHY1_ADDR << 8) & 0x1F00;
        /* Write the (phyAddr, PHY_CONTROL_REG) to MIIMADDR register and write
         * the PHY_AUTONEG_EN in the MIIWTD register
         */
        writel (( phyAddr | (PHY_CONTROL_REG & 0x1F)), IDT_MIIMADDR_REG);
        writel (((PHY_AUTONEG_EN | PHY_RESTART_AN) & 0xFFFF), IDT_MIIMWTD_REG);
#endif
}

/***************************************************************************
 *
 * idt32438PhyAutoNegotiationComplete - initialize PHY device
 *
 * Completes Auto Negotiation process
 *
 * RETURNS: OK or ERROR.
 *
 */

int idt32438PhyAutoNegotiationComplete(acacia_MAC_t *MACInfo) 
{
	int     regVal;     /* PHY register value */
	int     regCtrl;    /* PHY control register value */    
	int  rtv;        /* function return value */
	U16  timeout;
	int phyAddr;
	char portName[4];
	
	if( MACInfo->unit == 0 ) {
		phyAddr = PHY0_ADDR;
		strcpy(portName, PORT0_NAME);
	}
	else {
		phyAddr = PHY1_ADDR;
		strcpy(portName, PORT1_NAME);
	}

       	/* Reset the Scan command */
       	writel (readl(IDT_MIIMCMD_REG) & ~MIIMCMD_SCN, IDT_MIIMCMD_REG);
	
	/* On reset PHY picks up default values set by
	 * Hardware Configuration: 10 Mbps Half Duplex
	 */
	/*
	 * Use the default settings because the link speed is incorrect
	 * after auto-negotation.
	 */

	idt32438PhyRegRead (phyAddr, PHY_CONTROL_REG, &regCtrl);
	idt32438PhyRegRead (phyAddr, PHY_STATUS1_REG, &regVal);

#ifdef  AUTO_NEGOTIATE    
	/* Wait 1s */
	for (timeout=20; timeout; timeout--) {
		/* wait for AN to complete */

		rtv = idt32438PhyRegRead (phyAddr, PHY_STATUS1_REG, 
					  &regVal);
		if (rtv == ERROR) {
			return (ERROR);
		}

		if ((regVal & (PHY_AUTONEG_DONE | PHY_AUTONEG_ABLE)) == 
		    (PHY_AUTONEG_DONE | PHY_AUTONEG_ABLE))
			break;
		sysMsDelay(50);

	}
	if (timeout == 0) {
		diag_printf("Phy Autonegotiation Failed on %s port\n",portName);
	}
#endif
	if (regVal & PHY_LINK_STATUS) {
#ifndef  AUTO_NEGOTIATE
		regVal = regCtrl;               /* Use the default values */
#endif
		if (regVal & (PHY_100BASEX_FULL |PHY_100BASEX_HALF) )
			MACInfo->board.phySpeed = PHY_100MBS;
		else
			MACInfo->board.phySpeed = PHY_10MBS;
		if (regVal & (PHY_100BASEX_FULL | PHY_10BASE_FULL)) {
			MACInfo->board.phyDpx = PHY_FULL_DPX;
			writel ( ((readl (IDT_ETH0MAC2_REG)) | IDT_ETHMAC2_FD),
				 IDT_ETH0MAC2_REG);
		} else {
			MACInfo->board.phyDpx = PHY_HALF_DPX;
			writel (((readl (IDT_ETH0MAC2_REG)) & ~IDT_ETHMAC2_FD),
					IDT_ETH0MAC2_REG);
		}
	}
	else {
		diag_printf("idt32438PhyInit(): PHY Link STATUS failed.\n");
		diag_printf("Please check the ethernet cable on %s port !\n",
					 			portName);
		return (ERROR);
	}

        MACInfo->board.phyAddr = phyAddr;
#if 0
        diag_printf("Phy Autonegotiaton Successful on %s port.\n", portName);
        diag_printf("RedBoot> ");   //prompt
#endif
	return (OK);
}


int idt32438PhyRegPoll(U32 v, U32 timeout)
{
#define TIMEOUT_COUNT    10
	int i=0;

	while ((readl (IDT_MIIMIND_REG) & v) == v) {
		sysMsDelay(10);
		if (i++  == TIMEOUT_COUNT) {
			/*check a second time.a task switch could make this fail
			 *so a second check is needed to make sure a timeout has
			 *really happened
			 */
			if ((readl (IDT_MIIMIND_REG) & v) == v) {
				diag_printf( \
			   "idt32438PhyRegPoll: PHY register read failed.\n");
				return (ERROR);
			}
			/* its OK so break out and continue with read */
			break;
		}
	}
	return OK;
}

/***************************************************************************
 *
 * idt32438PhyRegRead - read the PHY register
 *
 * This routine reads the specific register in the PHY device
 * Valid PHY address is 0-31
 *
 * RETURNS: OK or ERROR.
 *
 */

int idt32438PhyRegRead ( int         phyAddr,  /* phy address */
			 int         regAddr,  /* reg address */
			 int *       retVal   /* return value */ )
{
	if (idt32438PhyRegPoll(MIIMIND_BSY, PHY_RD_TIMEOUT) != OK)
		return ERROR;

	/* Write the (phyAddr, regAddr) to MIIMADDR register. */

	writel ( (((phyAddr << 8) & 0x1F00) | (regAddr & 0x1F)),
		 IDT_MIIMADDR_REG );

	if (idt32438PhyRegPoll(MIIMIND_BSY, PHY_RD_TIMEOUT) != OK)
		return ERROR;
   
	/* Set the read bit in MIIMCMD to start the read of MII. */

	writel (MIIMCMD_RD, IDT_MIIMCMD_REG );
   
	if (idt32438PhyRegPoll(MIIMIND_BSY, PHY_RD_TIMEOUT) != OK)
		return ERROR;
   
	/*
	 * Check for MIIMIND.NV; if NV=0 data read done otherwise wait.
	 */

	if ((readl (IDT_MIIMIND_REG) & MIIMIND_NV) == MIIMIND_NV) {
		ACACIA_PRINT (ACACIA_DEBUG_ERROR,
			      ("idt32438PhyRegRead : PHY register read failed.\n"));
		return (ERROR);
	}

	/* Read the Data */

	*retVal = (readl (IDT_MIIMRDD_REG) & 0xffff);

	/* Reset the Read command */

	writel (readl (IDT_MIIMCMD_REG) & ~MIIMCMD_RD, IDT_MIIMCMD_REG );

	return (OK);
}



#ifdef  AUTO_NEGOTIATE
/***************************************************************************
 *
 * idt32438PhyRegWrite - write to the PHY register
 *
 * This routine writes the specific register in the PHY device
 * Valid PHY address is 0-31
 *
 * RETURNS: OK or ERROR.
 */

int idt32438PhyRegWrite ( int     phyAddr,      /* phy address */
			  int     regAddr,      /* reg address */
			  int     writeData     /* data to be written */ )
{

	if (idt32438PhyRegPoll(MIIMIND_BSY, PHY_WR_TIMEOUT) != OK)
		return ERROR;

	/* Write the (phyAddr, regAddr)
	 * to MIIMADDR register and write
	 * the data in the MIIWTD register
	 */

	writel((((phyAddr << 8) & 0x1F00) | (regAddr & 0x1F)),IDT_MIIMADDR_REG);

	if (idt32438PhyRegPoll(MIIMIND_BSY, PHY_WR_TIMEOUT) != OK)
		return ERROR;

	writel ((writeData & 0xFFFF), IDT_MIIMWTD_REG);

	/*
	 * Check for MIIMIND.BSY; if BSY=0 data write done, otherwise wait.
	 */

	if (idt32438PhyRegPoll(MIIMIND_BSY, PHY_WR_TIMEOUT) != OK) {
		return ERROR;
	}

	return (OK);
}

#endif

