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

#ifdef QD_DEBUG
#ifdef _VXWORKS
#define INT_MSG_PRINT	logMsg
#else
#define INT_MSG_PRINT	printf
#endif
#else	/* QD_DEBUG */
#define INT_MSG_PRINT(_x,_a0,_a1,_a2,_a3,_a4,_a5)	while(0){}
#endif

GT_U32 QDIntMask = GT_VTU_PROB|GT_ATU_PROB|GT_PHY_INTERRUPT;

/*
 *	To enable quarterDeck interrupt, you need to call eventSetActive() and
 *	gprtPhyIntEnable(), as following sample routine.
 *	sampleQDIntEnable will enable all interrupt causes.
 *	For Port, GT_ATU_FULL, GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
 *	are enabled.
 *	For every Phy (0 ~ 4), GT_SPEED_CHANGED, GT_DUPLEX_CHANGED, GT_PAGE_RECEIVED,
 *	GT_AUTO_NEG_COMPLETED, GT_LINK_STATUS_CHANGED, GT_SYMBOL_ERROR, 
 *	GT_FALSE_CARRIER, GT_FIFO_FLOW, GT_CROSSOVER_CHANGED, GT_POLARITY_CHANGED,
 *	and GT_JABBER are enabled.
*/
GT_STATUS qdIntEnable(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_U16 data;
	int port;

	/* 
	 *	Enable QuarterDeck interrupt for ATUFull, ATUDone, PHYInt, and EEInt.
	 *	If writing 0 into eventSetActive(), all port interrupt will be disabled.
	*/
	data = QDIntMask;
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
GT_STATUS qdIntDisable(GT_QD_DEV *dev)
{
	GT_STATUS status;
	int port;

	/* 
	 *	Writing 0 into eventSetActive(), all port interrupt will be disabled.
	*/
	if((status = eventSetActive(dev,0)) != GT_OK)
	{
		MSG_PRINT(("eventSetActive returned fail.\n"));
		return status;
	}

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
GT_STATUS qdIntVector(GT_QD_DEV *dev)
{
	GT_U16 intCause,data,phyIntCause;
    GT_VTU_INT_STATUS vtuIntStatus;
    GT_ATU_INT_STATUS atuIntStatus;
	int port;

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
	if(intCause & GT_VTU_PROB)
	{
		INT_MSG_PRINT("VTU PROB INT for Dev %#x.\n",(int)dev,0,0,0,0,0);
		/* 
		 *	VTU member violation, miss violation, or full violation.
		*/
		do
		{
			if(gvtuGetIntStatus(dev, &vtuIntStatus) != GT_OK)
			{
				/* reading VTU Int Status Failed */
				INT_MSG_PRINT("VTU INT Status read failure.\n",0,0,0,0,0,0);
				break;
			}
			INT_MSG_PRINT("VTU INT : Cause %#x, SPID %i, VID %i\n",vtuIntStatus.vtuIntCause,vtuIntStatus.spid,vtuIntStatus.vid,0,0,0);
		} while (vtuIntStatus.vtuIntCause);
	}

	if(intCause & GT_ATU_PROB)
	{
		/* 
		 *	ATU cannot load or learn a new mapping due to all the available
		 *	locations for an address being locked.
		 *	ToDo...
		*/
		INT_MSG_PRINT("ATU PROB INT for Dev %#x.\n",(int)dev,0,0,0,0,0);

		do
		{
			if(gatuGetIntStatus(dev, &atuIntStatus) != GT_OK)
			{
				/* reading ATU Int Status Failed */
				INT_MSG_PRINT("ATU INT Status read failure.\n",0,0,0,0,0,0);
				break;
			}
			INT_MSG_PRINT("ATU INT : Cause %#x, SPID %i, DBNum %i\n",atuIntStatus.atuIntCause,atuIntStatus.spid,atuIntStatus.dbNum,0,0,0);
			INT_MSG_PRINT("ATU INT : MAC %02x-%02x-%02x-%02x-%02x-%02x\n",
									atuIntStatus.macAddr.arEther[0],
									atuIntStatus.macAddr.arEther[1],
									atuIntStatus.macAddr.arEther[2],
									atuIntStatus.macAddr.arEther[3],
									atuIntStatus.macAddr.arEther[4],
									atuIntStatus.macAddr.arEther[5]);
		} while (atuIntStatus.atuIntCause);

	}

	if(intCause & GT_ATU_DONE)
	{
		/* 
		 *	There is a transitions from a one to a zero on ATUBusy bit
		 *	(Refer to ATU Operation Register.)
		 *	ToDo...
		*/
		INT_MSG_PRINT("ATU Done INT for Dev %#x.\n",(int)dev,0,0,0,0,0);
	}

#if 1

	if(intCause & GT_PHY_INTERRUPT)
	{
		/* 
		 *	At least one of the Phy generated interrupt.
		 *	We need to read Phy Interrupt Summary and go through each phy
		 *	based on the summary.
		*/

		if(gprtGetPhyIntPortSummary(dev,&data) != GT_OK)
		{
			return GT_FAIL;
		}

		INT_MSG_PRINT("Phy INT (Port Vector %#x).\n",(int)data,0,0,0,0,0);

		port = 0;
		while(data)
		{
			if(data & 0x01)
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

				INT_MSG_PRINT("Port %i: %#x.\n",port,phyIntCause,0,0,0,0);

				if(phyIntCause & GT_SPEED_CHANGED)
				{
					/* 
					 *	Speed has been changed.
					 *	ToDo...
					*/
					INT_MSG_PRINT("SPEED CHANGE\n",0,0,0,0,0,0);
				}

				if(phyIntCause & GT_DUPLEX_CHANGED)
				{
					/* 
					 *	Duplex mode has been changed.
					 *	ToDo...
					*/
					INT_MSG_PRINT("DUPLEX CHANGE\n",0,0,0,0,0,0);
				}

				if(phyIntCause & GT_PAGE_RECEIVED)
				{
					/* 
					 *	Page received.
					 *	ToDo...
					*/

					INT_MSG_PRINT("PAGE RECEIVED\n",0,0,0,0,0,0);
				}

				if(phyIntCause & GT_AUTO_NEG_COMPLETED)
				{
					/* 
					 *	AutoNegotiation completed.
					 *	ToDo...
					*/
					INT_MSG_PRINT("AUTO NEG COMPLETED\n",0,0,0,0,0,0);

				}

				if(phyIntCause & GT_LINK_STATUS_CHANGED)
				{
					/* 
					 *	Link Status changed.
					 *	ToDo...
					*/

					INT_MSG_PRINT("LINK CHANGE\n",0,0,0,0,0,0);
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

			data >>= 1;
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

#endif
	
	return GT_OK;

}
