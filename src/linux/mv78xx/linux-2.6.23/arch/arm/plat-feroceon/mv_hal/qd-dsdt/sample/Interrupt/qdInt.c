#include <Copyright.h>
/********************************************************************************
* qdInt.c
*
* DESCRIPTION:
*		This sample shows how to call QuarterDeck Interrupt handler when QD INT
*		raised, and how to take care each Interrupt Cause.
*
* DEPENDENCIES:   NONE.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include "msSample.h"


/*
 *	To enable quarterDeck interrupt, you need to call eventSetActive() and
 *	gprtPhyIntEnable(), as following sample routine.
 *	sampleQDIntEnable will enable all interrupt causes.
 *	For Port, GT_ATU_FULL, GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
 *	are enabled.
 *
 *	In this sample, GT_SPEED_CHANGED, GT_DUPLEX_CHANGED, and 
 *  GT_LINK_STATUS_CHANGED are enabled for ports 0 ~ 2.
*/
GT_STATUS sampleQDIntEnable(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_LPORT port;
	GT_U16 data;

	/* 
	 *	Enable QuarterDeck interrupt for ATUFull, ATUDone, PHYInt, and EEInt.
	 *	If writing 0 into eventSetActive(), all port interrupt will be disabled.
	*/
	data = GT_STATS_DONE|GT_VTU_PROB|GT_VTU_DONE|
		   GT_ATU_FULL|GT_ATU_DONE|GT_PHY_INTERRUPT|GT_EE_INTERRUPT;
	if((status = eventSetActive(dev,data)) != GT_OK)
	{
		MSG_PRINT(("eventSetActive returned fail.\n"));
		return status;
	}

	/* 
	 *	Enable Phy interrupt for every possible interrupt cause.
	 *	If writing 0 into gprtPhyIntEnable(), all port interrupt will be disabled.
	*/
	data = 	GT_SPEED_CHANGED|GT_DUPLEX_CHANGED|GT_LINK_STATUS_CHANGED;

	for(port=0; port<3; port++)
	{
		if((status = gprtPhyIntEnable(dev,port,data)) != GT_OK)
		{
			MSG_PRINT(("gprtPhyIntEnable returned fail.\n"));
			return status;
		}
	}

	return GT_OK;
}

/*
 *	Disable QuarterDeck Interrupt.
*/
GT_STATUS sampleQDIntDisable(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_LPORT port;

	/* 
	 *	Writing 0 into eventSetActive(), all port interrupt will be disabled.
	*/
	if((status = eventSetActive(dev,0)) != GT_OK)
	{
		MSG_PRINT(("eventSetActive returned fail.\n"));
		return status;
	}

	/* 
	 *	Writing 0 into gprtPhyIntEnable(), all port interrupt will be disabled.
	*/
	for(port=0; port<3; port++)
	{
		if((status = gprtPhyIntEnable(dev,port,0)) != GT_OK)
		{
			MSG_PRINT(("gprtPhyIntEnable returned fail.\n"));
			return status;
		}
	}
	return GT_OK;
}


/*
 *	Assume that the following function, sampleQDIntVector(), is registered 
 *	when BSP calls intConnect for QD Interrupt.
 *	This sample will show how to deal with QuarterDeck Interrupt.
*/
GT_STATUS sampleQDIntVector(GT_QD_DEV *dev)
{
	GT_U16 intCause, phyIntCause;
	GT_U16 portVec;
	GT_LPORT port;
	GT_VTU_INT_STATUS vtuInt;
	GT_ATU_INT_STATUS atuInt;

	/*
	 *	Disable QuarterDeck Interrupt in System Level.
	 *	ToDo...
	*/

	/*
	 *	Check if QD generated the interrupt.
	*/
	if(eventGetIntStatus(dev,&intCause) != GT_TRUE)
	{
		/* QD didn't generate the interrupt. */
		return GT_FAIL;
	}

	/*
	 *	QD generated interrupt with the reason in intCause.
	*/

	if(intCause & GT_STATS_DONE)
	{
		/* 
		 *	Statistics Done Interrupt
		 *	ToDo...
		*/

	}
	if(intCause & GT_VTU_DONE)
	{
		/* 
		 *	VTU Done Interrupt
		 *	ToDo...
		*/

	}

	if(intCause & GT_VTU_PROB)
	{
		/* 
		 *	Vlan Table Problem/Violation.
		 *	Need to read the cause.
		*/
		do {
			if(gvtuGetIntStatus(dev,&vtuInt) != GT_OK)
			{
				/* failed to retrieve VTU Interrupt cause */
				break;
			}

			if(vtuInt.vtuIntCause & GT_VTU_FULL_VIOLATION)
			{
				/* 
				 *	Vlan Table is Full
				 *	ToDo...
				*/
			}

			if(vtuInt.vtuIntCause & GT_MEMBER_VIOLATION)
			{
				/* 
				 *	Member Violation
				 *	ToDo...
				*/
			}

			if(vtuInt.vtuIntCause & GT_MISS_VIOLATION)
			{
				/* 
				 *	Miss Violation
				 *	ToDo...
				*/
			}
		} while(vtuInt.vtuIntCause != 0);
	}

	if(intCause & GT_ATU_PROB)
	{
		/* 
		 *	ATU cannot load or learn a new mapping due to all the available
		 *	locations for an address being locked.
		 *	ToDo...
		*/
		do {
			if(gatuGetIntStatus(dev,&atuInt) != GT_OK)
			{
				/* failed to retrieve VTU Interrupt cause */
				break;
			}

			if(atuInt.atuIntCause & GT_FULL_VIOLATION)
			{
				/* 
				 *	Table is Full
				 *	ToDo...
				*/
			}

			if(atuInt.atuIntCause & GT_MEMBER_VIOLATION)
			{
				/* 
				 *	Member Violation
				 *	ToDo...
				*/
			}

			if(atuInt.atuIntCause & GT_MISS_VIOLATION)
			{
				/* 
				 *	Miss Violation
				 *	ToDo...
				*/
			}
		} while(atuInt.atuIntCause != 0);

	}

	if(intCause & GT_ATU_DONE)
	{
		/* 
		 *	There is a transitions from a one to a zero on ATUBusy bit
		 *	(Refer to ATU Operation Register.)
		 *	ToDo...
		*/

	}

	if(intCause & GT_PHY_INTERRUPT)
	{
		/* 
		 *	At least one of the Phy generated interrupt.
		 *	We need to read Phy Interrupt Summary and go through each phy
		 *	based on the summary.
		*/

		if(gprtGetPhyIntPortSummary(dev,&portVec) != GT_OK)
		{
			return GT_FAIL;
		}

		port = 0;
		while(portVec)
		{
			if(portVec & 0x01)
			{
				/*
				 *	Call gprtGetPhyIntStatus to get intCause
				*/
				if(gprtGetPhyIntStatus(dev,port,&phyIntCause) != GT_OK)
				{
					/* 
					 *	Something wrong with the system. Need to do the 
					 *	necessary work. 
					 *	ToDo...
					*/
				}

				if(phyIntCause & GT_SPEED_CHANGED)
				{
					/* 
					 *	Speed has been changed.
					 *	ToDo...
					*/
				}

				if(phyIntCause & GT_DUPLEX_CHANGED)
				{
					/* 
					 *	Duplex mode has been changed.
					 *	ToDo...
					*/
				}

				if(phyIntCause & GT_PAGE_RECEIVED)
				{
					/* 
					 *	Page received.
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_AUTO_NEG_COMPLETED)
				{
					/* 
					 *	AutoNegotiation completed.
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_LINK_STATUS_CHANGED)
				{
					/* 
					 *	Link Status changed.
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_SYMBOL_ERROR)
				{
					/* 
					 *	Symbol error
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_FALSE_CARRIER)
				{
					/* 
					 *	False Carrier.
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_FIFO_FLOW)
				{
					/* 
					 *	Fifo Overflow/underflow error
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_CROSSOVER_CHANGED)
				{
					/* 
					 *	MDI/MDIX crossover changed.
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_POLARITY_CHANGED)
				{
					/* 
					 *	Polarity changed.
					 *	ToDo...
					*/

				}

				if(phyIntCause & GT_JABBER)
				{
					/* 
					 *	Jabber
					 *	ToDo...
					*/

				}
			}

			portVec >>= 1;
			port++;
		}
	}

	if(intCause & GT_EE_INTERRUPT)
	{
		/* 
		 *	EEPROM is done loading registers.
		 *	ToDo...
		*/

	}


	/*
	 *	Now, all the QuarterDeck related interrupt have been cleared,
	 *	so it's OK to enable QuarterDeck Interrupt in System Level.
	 *	ToDo...
	*/
	
	return GT_OK;

}
