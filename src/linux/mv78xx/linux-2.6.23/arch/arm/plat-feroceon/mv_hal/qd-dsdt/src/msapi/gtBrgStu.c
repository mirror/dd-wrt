#include <Copyright.h>

/*******************************************************************************
* gtBrgStu.c
*
* DESCRIPTION:
*       API definitions for SID (VTU 802.1s Port State Information Database) 
*		Translation Unit.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: $
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/****************************************************************************/
/* Forward function declaration.                                            */
/****************************************************************************/

static GT_STATUS stuOperationPerform
(
    IN	    GT_QD_DEV           *dev,
    IN      GT_STU_OPERATION    stuOp,
    INOUT   GT_U8               *valid,
    INOUT 	GT_STU_ENTRY    	*stuEntry
);

/*******************************************************************************
* gstuGetEntryCount
*
* DESCRIPTION:
*       Gets the current number of valid entries in the STU table
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numEntries - number of STU entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gstuGetEntryCount
(
    IN  GT_QD_DEV *dev,
    OUT GT_U32    *numEntries
)
{
    GT_U8               valid;
    GT_U32				numOfEntries;
    GT_STATUS       	retVal;
    GT_STU_ENTRY    	entry;

    DBG_INFO(("gstuGetEntryCount Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    entry.sid = 0;
    numOfEntries = 0;

    while(1)
    {
		retVal = stuOperationPerform(dev,GET_NEXT_STU_ENTRY,&valid,&entry);
		if(retVal != GT_OK)
		{
		    DBG_INFO(("Failed (stuOperationPerform returned GT_FAIL).\n"));
	    	return retVal;
		}

		if( entry.sid==0x3F )
		{
			if (valid==1) numOfEntries++;
			break;
		}

        numOfEntries++;
    }

    *numEntries = numOfEntries;

    DBG_INFO(("OK.\n"));
    return GT_OK;

}


/*******************************************************************************
* gstuGetEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic entry from the STU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       stuEntry - find the first valid STU entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstuGetEntryFirst
(
    IN  GT_QD_DEV       *dev,
    OUT GT_STU_ENTRY    *stuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8       		port; 
    GT_LPORT       		lport; 
    GT_STU_ENTRY    	entry;

    DBG_INFO(("gstuGetEntryFirst Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    entry.sid = 0;
    valid = 0;

    retVal = stuOperationPerform(dev,GET_NEXT_STU_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (stuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrieve the value from the operation */

    if((entry.sid == 0x3F) && (valid == 0))
		return GT_NO_SUCH;

    stuEntry->sid = entry.sid;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
		port = GT_LPORT_2_PORT(lport);
		stuEntry->portState[lport]=entry.portState[port];
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gstuGetEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic STU entry from the specified SID.
*
* INPUTS:
*       stuEntry - the SID to start the search.
*
* OUTPUTS:
*       stuEntry - next STU entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstuGetEntryNext
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_STU_ENTRY  *stuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8       		port; 
    GT_LPORT       		lport; 
    GT_STU_ENTRY    	entry;

    DBG_INFO(("gstuGetEntryNext Called.\n"));
    
    /* check if device supports this feature */

	if (!IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if(stuEntry->sid >= 0x3F)
	{
		return GT_NO_SUCH;
	}
	else
	{
	    entry.sid = stuEntry->sid;
	}
    valid = 0;

    retVal = stuOperationPerform(dev,GET_NEXT_STU_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (stuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrieve the value from the operation */ 

	if((entry.sid == 0x3F) && (valid == 0))
		return GT_NO_SUCH;

    stuEntry->sid = entry.sid;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
		port = GT_LPORT_2_PORT(lport);
		stuEntry->portState[lport]=entry.portState[port];
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gstuFindSidEntry
*
* DESCRIPTION:
*       Find STU entry for a specific SID, it will return the entry, if found, 
*       along with its associated data 
*
* INPUTS:
*       stuEntry - contains the SID to searche for 
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       stuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no such entry.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Valid SID is 1 ~ 63.
*
*******************************************************************************/
GT_STATUS gstuFindSidEntry
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_STU_ENTRY  *stuEntry,
    OUT GT_BOOL         *found
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8               port;
    GT_LPORT            lport;
    GT_STU_ENTRY    	entry;

    DBG_INFO(("gstuFindSidEntry Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if((stuEntry->sid == 0) && (stuEntry->sid > 0x3F))
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    *found = GT_FALSE;

    /* Decrement 1 from sid */
    entry.sid   = stuEntry->sid-1;
    valid = 0; /* valid is not used as input in this operation */

    retVal = stuOperationPerform(dev,GET_NEXT_STU_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (stuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */ 
    if ((entry.sid != stuEntry->sid) | (valid == 0))
		return GT_NO_SUCH;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
		port = GT_LPORT_2_PORT(lport);
		stuEntry->portState[lport]=entry.portState[port];
    }

    *found = GT_TRUE;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gstuAddEntry
*
* DESCRIPTION:
*       Creates or update the entry in STU table based on user input.
*
* INPUTS:
*       stuEntry    - stu entry to insert to the STU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Valid SID is 1 ~ 63.
*
*******************************************************************************/
GT_STATUS gstuAddEntry
(
    IN  GT_QD_DEV   	*dev,
    IN  GT_STU_ENTRY	*stuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8       	port; 
    GT_LPORT       	lport; 
    GT_STU_ENTRY 	tmpStuEntry;
	GT_BOOL		 	found;
	int				count = 50000;
    GT_STU_ENTRY    	entry;

    DBG_INFO(("gstuAddEntry Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if((stuEntry->sid == 0) && (stuEntry->sid > 0x3F))
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    entry.sid = stuEntry->sid;

    valid = 1; /* for load operation */

    for(port=0; port<dev->maxPorts; port++)
    {
		lport = GT_PORT_2_LPORT(port);
		if (lport == GT_INVALID_PORT)
			entry.portState[port] = 0;
		else
			entry.portState[port] = stuEntry->portState[lport];
    }

    retVal = stuOperationPerform(dev,LOAD_PURGE_STU_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
		DBG_INFO(("Failed (stuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	/* verify that the given entry has been added */
	tmpStuEntry.sid = stuEntry->sid;

	if((retVal = gstuFindSidEntry(dev,&tmpStuEntry,&found)) != GT_OK)
	{
		while(count--);
		if((retVal = gstuFindSidEntry(dev,&tmpStuEntry,&found)) != GT_OK)
		{
			DBG_INFO(("Added entry cannot be found\n"));
			return retVal;
		}
	}
	if(found == GT_FALSE)
	{
		DBG_INFO(("Added entry cannot be found\n"));
		return GT_FAIL;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gstuDelEntry
*
* DESCRIPTION:
*       Deletes STU entry specified by user.
*
* INPUTS:
*       stuEntry - the STU entry to be deleted 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Valid SID is 1 ~ 63.
*
*******************************************************************************/
GT_STATUS gstuDelEntry
(
    IN  GT_QD_DEV   	*dev,
    IN  GT_STU_ENTRY 	*stuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_STU_ENTRY    	entry;

    DBG_INFO(("gstuDelEntry Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
    
    if((stuEntry->sid == 0) && (stuEntry->sid > 0x3F))
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    entry.sid = stuEntry->sid;
    valid = 0; /* for delete operation */

    retVal = stuOperationPerform(dev,LOAD_PURGE_STU_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (stuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

static GT_STATUS stuSetSTUData
(
    IN	GT_QD_DEV           *dev,
	IN	GT_STU_ENTRY    	*entry
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16          data1,data2,data3;           /* Data to be set into the      */
	GT_U16			nStuData = 0;

	data1 = data2 = data3 = 0;

	switch (dev->maxPorts)
	{
		case 11:
			data3 |= (entry->portState[10] & 3) << 10;
			/* pass through */
		case 10:
			data3 |= (entry->portState[9] & 3) << 6;
			/* pass through */
		case 9:
			data3 |= (entry->portState[8] & 3) << 2;
			nStuData++;

			/* pass through */
		case 8:
			data2 |= (entry->portState[7] & 3) << 14;
			/* pass through */
		case 7:
			data2 |= (entry->portState[6] & 3) << 10;
			/* pass through */
		case 6:
			data2 |= (entry->portState[5] & 3) << 6;
			/* pass through */
		case 5:
			data2 |= (entry->portState[4] & 3) << 2;
			nStuData++;

			/* pass through */
		case 4:
			data1 |= (entry->portState[3] & 3) << 14;
			/* pass through */
		case 3:
			data1 |= (entry->portState[2] & 3) << 10;
			/* pass through */
		case 2:
			data1 |= (entry->portState[1] & 3) << 6;
			/* pass through */
		case 1:
			data1 |= (entry->portState[0] & 3) << 2;
			nStuData++;
			break;

		default:
			return GT_FAIL;
	}

	switch(nStuData)
	{
		case 3:
			retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA3_REG,data3);
			if(retVal != GT_OK)
			{
				return retVal;
			}
			/* pass through */
		case 2:
			retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA2_REG,data2);
			if(retVal != GT_OK)
			{
				return retVal;
			}
			/* pass through */
		case 1:
			retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA1_REG,data1);
			if(retVal != GT_OK)
			{
				return retVal;
			}
			break;
		default:
			return GT_FAIL;
	}
	
	return retVal;		
}

static GT_STATUS stuGetSTUData
(
    IN	GT_QD_DEV           *dev,
	OUT	GT_STU_ENTRY    	*entry
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16          data1,data2,data3;           /* Data to be set into the      */
	GT_U16			nStuData = 0;

	data1 = data2 = data3 = 0;

	gtMemSet((void*)entry->portState,0,sizeof(entry->portState));

	switch (dev->maxPorts)
	{
		case 11:
		case 10:
		case 9:
			nStuData = 3;
			break;

		case 8:
		case 7:
		case 6:
		case 5:
			nStuData = 2;
			break;

		case 4:
		case 3:
		case 2:
		case 1:
			nStuData = 1;
			break;

		default:
			return GT_FAIL;
	}

	switch(nStuData)
	{
		case 3:
			retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA3_REG,&data3);
			if(retVal != GT_OK)
			{
				return retVal;
			}
			/* pass through */
		case 2:
			retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA2_REG,&data2);
			if(retVal != GT_OK)
			{
				return retVal;
			}
			/* pass through */
		case 1:
			retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA1_REG,&data1);
			if(retVal != GT_OK)
			{
				return retVal;
			}
			break;
		default:
			return GT_FAIL;
	}
	
	switch (dev->maxPorts)
	{
		case 11:
			entry->portState[10]  = (data3 >> 10) & 3 ;
			/* pass through */
		case 10:
			entry->portState[9]  = (data3 >> 6) & 3 ;
			/* pass through */
		case 9:
			entry->portState[8]  = (data3 >> 2) & 3 ;
			/* pass through */
		case 8:
			entry->portState[7]  = (data2 >> 14) & 3 ;
			/* pass through */
		case 7:
			entry->portState[6]  = (data2 >> 10) & 3 ;
			/* pass through */
		case 6:
			entry->portState[5]  = (data2 >> 6) & 3 ;
			/* pass through */
		case 5:
			entry->portState[4]  = (data2 >> 2) & 3 ;
			/* pass through */
		case 4:
			entry->portState[3]  = (data1 >> 14) & 3 ;
			/* pass through */
		case 3:
			entry->portState[2]  = (data1 >> 10) & 3 ;
			/* pass through */
		case 2:
			entry->portState[1]  = (data1 >> 6) & 3 ;
			/* pass through */
		case 1:
			entry->portState[0]  = (data1 >> 2) & 3 ;
			break;

		default:
			return GT_FAIL;
	}

	return GT_OK;
}


/*******************************************************************************
* stuOperationPerform
*
* DESCRIPTION:
*       This function is used by all STU control functions, and is responsible
*       to write the required operation into the STU registers.
*
* INPUTS:
*       stuOp       - The STU operation bits to be written into the STU
*                     operation register.
*       sid         - sid
*       valid       - valid bit
*       stuData     - STU Data with port state information
*
* OUTPUTS:
*       sid         - sid
*       valid       - valid bit
*       stuData     - STU Data with port state information
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/

static GT_STATUS stuOperationPerform
(
    IN	    GT_QD_DEV           *dev,
    IN      GT_STU_OPERATION    stuOp,
    INOUT   GT_U8               *valid,
	INOUT	GT_STU_ENTRY    	*entry
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16          data;           /* Data to be set into the      */
                                /* register.                    */

	gtSemTake(dev,dev->vtuRegsSem,OS_WAIT_FOREVER);

	/* Wait until the VTU in ready. */
	data = 1;
    while(data == 1)
    {
		retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,15,1,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}
	}

	/* Set the VTU data register if Load operation is required. */
	if (stuOp == LOAD_PURGE_STU_ENTRY)
	{
		if (*valid == 1)
		{
			/* set the Port State for all the ports */
			retVal = stuSetSTUData(dev,entry);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}

			/* Set the valid bit (QD_REG_VTU_VID_REG) */
	   		data= *valid << 12 ;
    	    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_VTU_VID_REG),data);
	   		if(retVal != GT_OK)
    	    {
	   			gtSemGive(dev,dev->vtuRegsSem);
	    		return retVal;
   		   	}		
		}
		else
		{
			/* Clear the valid bit (QD_REG_VTU_VID_REG) */
	   		data= 0 ;
    	    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_VTU_VID_REG),data);
	   		if(retVal != GT_OK)
    	    {
	   			gtSemGive(dev,dev->vtuRegsSem);
	    		return retVal;
   		   	}		
		}
    }

	/* Set the SID register (QD_REG_STU_SID_REG) */
   	data= (entry->sid) & 0x3F;
    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_STU_SID_REG),data);
   	if(retVal != GT_OK)
    {
   		gtSemGive(dev,dev->vtuRegsSem);
    	return retVal;
   	}		

	/* Start the STU Operation by defining the stuOp and VTUBusy */
	data = (1 << 15) | (stuOp << 12);

	retVal = hwWriteGlobalReg(dev,QD_REG_VTU_OPERATION,data);
	if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->vtuRegsSem);
		return retVal;
	}

	/* If the operation is a get next operation wait for the response   */
	if(stuOp == GET_NEXT_STU_ENTRY)
	{
		/* Wait until the STU in ready. */
		data = 1;
		while(data == 1)
		{
			retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,15,1,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}
		}

		/****************** get the valid bit *******************/
		retVal = hwGetGlobalRegField(dev,QD_REG_VTU_VID_REG,12,1,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		*valid = (GT_U8)data;

		/****************** get the sid *******************/

		retVal = hwReadGlobalReg(dev,QD_REG_STU_SID_REG,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		/* the sid is bits 0-5 */
		entry->sid   = data & 0x3F;

		if (*valid)
		{
			/* get the Port State for all the ports */
			retVal = stuGetSTUData(dev,entry);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}

		} /* entry is valid */

	} /* end of get next entry */

	gtSemGive(dev,dev->vtuRegsSem);
	return GT_OK;
}
