#include <Copyright.h>

/*******************************************************************************
* gtBrgVtu.c
*
* DESCRIPTION:
*       API definitions for Vlan Translation Unit for 802.1Q.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/****************************************************************************/
/* Forward function declaration.                                            */
/****************************************************************************/
#define MEMBER_TAG_CONV_FOR_APP(_dev,_tag)	memberTagConversionForApp(_dev,_tag)
#define MEMBER_TAG_CONV_FOR_DEV(_dev,_tag)	memberTagConversionForDev(_dev,_tag)

static GT_U8 memberTagConversionForApp
(
    IN	GT_QD_DEV           *dev,
    IN	GT_U8               tag
)
{
	GT_U8 convTag;

	/* check if memberTag needs to be converted */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
		return tag;

	switch(tag)
	{
		case 0:
				convTag = MEMBER_EGRESS_UNMODIFIED;
				break;
		case 1:
				convTag = MEMBER_EGRESS_UNTAGGED;
				break;
		case 2:
				convTag = MEMBER_EGRESS_TAGGED;
				break;
		case 3:
				convTag = NOT_A_MEMBER;
				break;
		default:
				DBG_INFO(("Unknown Tag (%#x) from Device !!!.\n",tag));
				convTag = 0xFF;
				break;

	}

	return convTag;
}

static GT_U8 memberTagConversionForDev
(
    IN	GT_QD_DEV           *dev,
    IN	GT_U8               tag
)
{
	GT_U8 convTag;

	/* check if memberTag needs to be converted */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
		return tag;

	switch(tag)
	{
		case MEMBER_EGRESS_UNMODIFIED:
				convTag = 0;
				break;
		case NOT_A_MEMBER:
				convTag = 3;
				break;
		case MEMBER_EGRESS_UNTAGGED:
				convTag = 1;
				break;
		case MEMBER_EGRESS_TAGGED:
				convTag = 2;
				break;
		default:
				DBG_INFO(("Unknown Tag (%#x) from App. !!!.\n",tag));
				convTag = 0xFF;
				break;

	}

	return convTag;
}

static GT_STATUS vtuOperationPerform
(
    IN	    GT_QD_DEV           *dev,
    IN      GT_VTU_OPERATION    vtuOp,
    INOUT   GT_U8               *valid,
    INOUT 	GT_VTU_ENTRY    	*vtuEntry
);

/*******************************************************************************
* gvtuGetEntryCount
*
* DESCRIPTION:
*       Gets the current number of valid entries in the VTU table
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numEntries - number of VTU entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - vlan does not exist.
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryCount
(
    IN  GT_QD_DEV *dev,
    OUT GT_U32    *numEntries
)
{
    GT_U8               valid;
    GT_U32		numOfEntries;
    GT_STATUS       	retVal;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuGetEntryCount Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
      return retVal;

    entry.vid = 0xFFF;
    entry.DBNum = 0;

    numOfEntries = 0;
    while(1)
    {
		retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&valid,&entry);
		if(retVal != GT_OK)
		{
		    DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
	    	return retVal;
		}

		if( entry.vid==0xFFF )
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
* gvtuGetEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic entry from the VTU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuEntry - match VTU entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from vid of all one's
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryFirst
(
    IN  GT_QD_DEV       *dev,
    OUT GT_VTU_ENTRY    *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8       		port;
    GT_LPORT       		lport;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuGetEntryFirst Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
      return retVal;

    entry.vid = 0xFFF;
    entry.DBNum = 0;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */

    if((entry.vid == 0xFFF) && (valid == 0))
		return GT_NO_SUCH;

    vtuEntry->DBNum = entry.DBNum;
    vtuEntry->vid   = entry.vid;

	vtuEntry->vidPriOverride = entry.vidPriOverride;
	vtuEntry->vidPriority = entry.vidPriority;

	vtuEntry->vidPolicy = entry.vidPolicy;
	vtuEntry->sid = entry.sid;

	vtuEntry->vidExInfo.useVIDFPri = entry.vidExInfo.useVIDFPri;
	vtuEntry->vidExInfo.vidFPri = entry.vidExInfo.vidFPri;
	vtuEntry->vidExInfo.useVIDQPri = entry.vidExInfo.useVIDQPri;
	vtuEntry->vidExInfo.vidQPri = entry.vidExInfo.vidQPri;
	vtuEntry->vidExInfo.vidNRateLimit = entry.vidExInfo.vidNRateLimit;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
		port = GT_LPORT_2_PORT(lport);
		vtuEntry->vtuData.memberTagP[lport]=MEMBER_TAG_CONV_FOR_APP(dev,entry.vtuData.memberTagP[port]);
		vtuEntry->vtuData.portStateP[lport]=entry.vtuData.portStateP[port];
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gvtuGetEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic VTU entry from the specified VID.
*
* INPUTS:
*       vtuEntry - the VID to start the search.
*
* OUTPUTS:
*       vtuEntry - match VTU  entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from the VID specified by the user.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryNext
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_VTU_ENTRY  *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8       		port;
    GT_LPORT       		lport;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuGetEntryNext Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
      return retVal;

    entry.DBNum = vtuEntry->DBNum;
    entry.vid   = vtuEntry->vid;
    valid = 0;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrieve the value from the operation */

	if((entry.vid == 0xFFF) && (valid == 0))
		return GT_NO_SUCH;

    vtuEntry->DBNum = entry.DBNum;
    vtuEntry->vid   = entry.vid;

	vtuEntry->vidPriOverride = entry.vidPriOverride;
	vtuEntry->vidPriority = entry.vidPriority;

	vtuEntry->vidPolicy = entry.vidPolicy;
	vtuEntry->sid = entry.sid;

	vtuEntry->vidExInfo.useVIDFPri = entry.vidExInfo.useVIDFPri;
	vtuEntry->vidExInfo.vidFPri = entry.vidExInfo.vidFPri;
	vtuEntry->vidExInfo.useVIDQPri = entry.vidExInfo.useVIDQPri;
	vtuEntry->vidExInfo.vidQPri = entry.vidExInfo.vidQPri;
	vtuEntry->vidExInfo.vidNRateLimit = entry.vidExInfo.vidNRateLimit;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
		port = GT_LPORT_2_PORT(lport);
		vtuEntry->vtuData.memberTagP[lport]=MEMBER_TAG_CONV_FOR_APP(dev,entry.vtuData.memberTagP[port]);
		vtuEntry->vtuData.portStateP[lport]=entry.vtuData.portStateP[port];
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvtuFindVidEntry
*
* DESCRIPTION:
*       Find VTU entry for a specific VID, it will return the entry, if found,
*       along with its associated data
*
* INPUTS:
*       vtuEntry - contains the VID to searche for
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       vtuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuFindVidEntry
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_VTU_ENTRY  *vtuEntry,
    OUT GT_BOOL         *found
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8               port;
    GT_LPORT            lport;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuFindVidEntry Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
      return retVal;

    *found = GT_FALSE;

    /* Decrement 1 from vid    */
    entry.vid   = vtuEntry->vid-1;
    valid = 0; /* valid is not used as input in this operation */
    entry.DBNum = vtuEntry->DBNum;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */

    if( (entry.vid !=vtuEntry->vid) | (valid !=1) )
    {
          DBG_INFO(("Failed.\n"));
          return GT_NO_SUCH;
    }

    vtuEntry->DBNum = entry.DBNum;

	vtuEntry->vidPriOverride = entry.vidPriOverride;
	vtuEntry->vidPriority = entry.vidPriority;

	vtuEntry->vidPolicy = entry.vidPolicy;
	vtuEntry->sid = entry.sid;

	vtuEntry->vidExInfo.useVIDFPri = entry.vidExInfo.useVIDFPri;
	vtuEntry->vidExInfo.vidFPri = entry.vidExInfo.vidFPri;
	vtuEntry->vidExInfo.useVIDQPri = entry.vidExInfo.useVIDQPri;
	vtuEntry->vidExInfo.vidQPri = entry.vidExInfo.vidQPri;
	vtuEntry->vidExInfo.vidNRateLimit = entry.vidExInfo.vidNRateLimit;

    for(lport=0; lport<dev->numOfPorts; lport++)
    {
		port = GT_LPORT_2_PORT(lport);
		vtuEntry->vtuData.memberTagP[lport]=MEMBER_TAG_CONV_FOR_APP(dev,entry.vtuData.memberTagP[port]);
		vtuEntry->vtuData.portStateP[lport]=entry.vtuData.portStateP[port];
    }

    *found = GT_TRUE;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvtuFlush
*
* DESCRIPTION:
*       This routine removes all entries from VTU Table.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuFlush
(
    IN  GT_QD_DEV       *dev
)
{
    GT_STATUS       retVal;

    DBG_INFO(("gvtuFlush Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
	{
		return retVal;
	}

    retVal = vtuOperationPerform(dev,FLUSH_ALL,NULL,NULL);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvtuAddEntry
*
* DESCRIPTION:
*       Creates the new entry in VTU table based on user input.
*
* INPUTS:
*       vtuEntry    - vtu entry to insert to the VTU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*       GT_FULL			  - vtu table is full
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuAddEntry
(
    IN  GT_QD_DEV   *dev,
    IN GT_VTU_ENTRY *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_U8       	port;
    GT_LPORT       	lport;
    GT_VTU_ENTRY 	tmpVtuEntry;
	GT_BOOL		 	found;
	int				count = 5000;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuAddEntry Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
      return retVal;

    entry.DBNum = vtuEntry->DBNum;
    entry.vid   = vtuEntry->vid;

	if(IS_IN_DEV_GROUP(dev,DEV_VTU_EXT_INFO))
	{
		entry.vidPriOverride = 0;
		entry.vidPriority = 0;

		entry.vidPolicy = GT_FALSE;
		entry.sid = 0;

		if(IS_IN_DEV_GROUP(dev,DEV_FQPRI_IN_TABLE))
		{
			entry.vidExInfo.useVIDFPri = vtuEntry->vidExInfo.useVIDFPri;
			entry.vidExInfo.vidFPri = vtuEntry->vidExInfo.vidFPri;
			entry.vidExInfo.useVIDQPri = vtuEntry->vidExInfo.useVIDQPri;
			entry.vidExInfo.vidQPri = vtuEntry->vidExInfo.vidQPri;
			entry.vidExInfo.vidNRateLimit = vtuEntry->vidExInfo.vidNRateLimit;
		}
		else
		{
			entry.vidExInfo.useVIDFPri = 0;
			entry.vidExInfo.vidFPri = 0;
			entry.vidExInfo.useVIDQPri = 0;
			entry.vidExInfo.vidQPri = 0;
			entry.vidExInfo.vidNRateLimit = vtuEntry->vidExInfo.vidNRateLimit;
		}
	}
	else
	{
		entry.vidPriOverride = vtuEntry->vidPriOverride;
		entry.vidPriority = vtuEntry->vidPriority;

		if(IS_IN_DEV_GROUP(dev,DEV_POLICY))
		{
			entry.vidPolicy = vtuEntry->vidPolicy;
		}
		else
		{
			entry.vidPolicy = GT_FALSE;
		}

		if(IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
		{
			entry.sid = vtuEntry->sid;
		}
		else
		{
			entry.sid = 0;
		}

		entry.vidExInfo.useVIDFPri = 0;
		entry.vidExInfo.vidFPri = 0;
		entry.vidExInfo.useVIDQPri = 0;
		entry.vidExInfo.vidQPri = 0;
		entry.vidExInfo.vidNRateLimit = 0;
	}

    valid = 1; /* for load operation */

    for(port=0; port<dev->maxPorts; port++)
    {
		lport = GT_PORT_2_LPORT(port);
		if(lport == GT_INVALID_PORT)
		{
			entry.vtuData.memberTagP[port] = MEMBER_TAG_CONV_FOR_DEV(dev,NOT_A_MEMBER);
			entry.vtuData.portStateP[port] = 0;
		}
		else
		{
			entry.vtuData.memberTagP[port] = MEMBER_TAG_CONV_FOR_DEV(dev,vtuEntry->vtuData.memberTagP[lport]);
			if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
				entry.vtuData.portStateP[port] = vtuEntry->vtuData.portStateP[lport];
			else
				entry.vtuData.portStateP[port] = 0;
		}
    }

    retVal = vtuOperationPerform(dev,LOAD_PURGE_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
		DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	/* verify that the given entry has been added */
	tmpVtuEntry.vid = vtuEntry->vid;
	tmpVtuEntry.DBNum = vtuEntry->DBNum;

	if((retVal = gvtuFindVidEntry(dev,&tmpVtuEntry,&found)) != GT_OK)
	{
		while(count--);
		if((retVal = gvtuFindVidEntry(dev,&tmpVtuEntry,&found)) != GT_OK)
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
* gvtuDelEntry
*
* DESCRIPTION:
*       Deletes VTU entry specified by user.
*
* INPUTS:
*       vtuEntry - the VTU entry to be deleted
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuDelEntry
(
    IN  GT_QD_DEV   *dev,
    IN GT_VTU_ENTRY *vtuEntry
)
{
    GT_U8               valid;
    GT_STATUS       	retVal;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuDelEntry Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK)
      return retVal;

    entry.DBNum = vtuEntry->DBNum;
    entry.vid   = vtuEntry->vid;
    valid = 0; /* for delete operation */

    retVal = vtuOperationPerform(dev,LOAD_PURGE_ENTRY,&valid, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* gvtuGetViolation
*
* DESCRIPTION:
*       Get VTU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation
(
    IN  GT_QD_DEV         *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_U8               spid;
    GT_U16               vid;
    GT_U16               intCause;
    GT_STATUS       	retVal;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuGetViolation Called.\n"));

	/* check which Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,4,3,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

	if (intCause == 0)
	{
		/* No Violation occurred. */
		vtuIntStatus->vtuIntCause = 0;
		return GT_OK;
	}

    entry.DBNum = 0;

    retVal = vtuOperationPerform(dev,SERVICE_VIOLATIONS,NULL, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	spid = entry.DBNum & 0xF;
	vid = entry.vid;

	if(spid == 0xF)
	{
		vtuIntStatus->vtuIntCause = GT_VTU_FULL_VIOLATION;
		vtuIntStatus->spid = spid;
		vtuIntStatus->vid = 0;
	}
	else
	{
		vtuIntStatus->vtuIntCause = intCause & (GT_MEMBER_VIOLATION | GT_MISS_VIOLATION);
		vtuIntStatus->spid = spid;
		vtuIntStatus->vid = vid;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvtuGetViolation2
*
* DESCRIPTION:
*       Get VTU Violation data (for Gigabit Device)
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation2
(
    IN  GT_QD_DEV         *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_U16               intCause;
    GT_STATUS       	retVal;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuGetViolation2 Called.\n"));

	/* check if Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,5,1,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

	if (intCause == 0)
	{
		/* No Violation occurred. */
		vtuIntStatus->vtuIntCause = 0;
		return GT_OK;
	}

    entry.DBNum = 0;

    retVal = vtuOperationPerform(dev,SERVICE_VIOLATIONS,NULL, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	/* check which Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,5,2,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

	switch (intCause)
	{
		case 0:
			/* No Violation occurred. */
			vtuIntStatus->vtuIntCause = 0;
			return GT_OK;
		case 1:
			/* Miss Violation */
			vtuIntStatus->vtuIntCause = GT_MISS_VIOLATION;
			break;
		case 2:
			/* Member Violation */
			vtuIntStatus->vtuIntCause = GT_MEMBER_VIOLATION;
			break;
		default :
			return GT_FAIL;
	}

	vtuIntStatus->spid = entry.DBNum & 0xF;
	vtuIntStatus->vid = entry.vid;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvtuGetViolation3
*
* DESCRIPTION:
*       Get VTU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation3
(
    IN  GT_QD_DEV         *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_U16               intCause;
    GT_STATUS       	retVal;
    GT_VTU_ENTRY    	entry;

    DBG_INFO(("gvtuGetViolation3 Called.\n"));

	/* check if Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,5,1,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

	if (intCause == 0)
	{
		/* No Violation occurred. */
		vtuIntStatus->vtuIntCause = 0;
		return GT_OK;
	}

    entry.DBNum = 0;

    retVal = vtuOperationPerform(dev,SERVICE_VIOLATIONS,NULL, &entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	/* check which Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,4,3,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

	vtuIntStatus->vtuIntCause = 0;

	if(intCause & 0x1)
	{
		vtuIntStatus->vtuIntCause |= GT_VTU_FULL_VIOLATION;
	}

	if(intCause & 0x2)
	{
		vtuIntStatus->vtuIntCause |= GT_MISS_VIOLATION;
	}

	if(intCause & 0x4)
	{
		vtuIntStatus->vtuIntCause |= GT_MEMBER_VIOLATION;
	}

	vtuIntStatus->spid = entry.DBNum & 0xF;
	vtuIntStatus->vid = entry.vid;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* vtuOperationPerform
*
* DESCRIPTION:
*       This function is used by all VTU control functions, and is responsible
*       to write the required operation into the VTU registers.
*
* INPUTS:
*       vtuOp       - The VTU operation bits to be written into the VTU
*                     operation register.
*       DBNum       - DBNum where the given vid belongs to
*       vid         - vlan id
*       valid       - valid bit
*       vtuData     - VTU Data with memberTag information
*
* OUTPUTS:
*       DBNum       - DBNum where the given vid belongs to
*       vid         - vlan id
*       valid       - valid bit
*       vtuData     - VTU Data with memberTag information
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/

static GT_STATUS vtuOperationPerform
(
    IN	    GT_QD_DEV           *dev,
    IN      GT_VTU_OPERATION    vtuOp,
    INOUT   GT_U8               *valid,
	INOUT	GT_VTU_ENTRY    	*entry
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

	/* Set the VTU data register    */
	/* There is no need to setup data reg. on flush, get next, or service violation */
	if((vtuOp != FLUSH_ALL) && (vtuOp != GET_NEXT_ENTRY) && (vtuOp != SERVICE_VIOLATIONS))
	{

		/****************** VTU DATA 1 REG *******************/

		/* get data and wirte to QD_REG_VTU_DATA1_REG (ports 0 to 3) */

		data =  (entry->vtuData.memberTagP[0] & 3)     |
				((entry->vtuData.memberTagP[1] & 3)<<4) |
				((entry->vtuData.memberTagP[2] & 3)<<8);

		if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
			data |= ((entry->vtuData.portStateP[0] & 3)<<2)	|
					((entry->vtuData.portStateP[1] & 3)<<6) |
					((entry->vtuData.portStateP[2] & 3)<<10);

		if(dev->maxPorts > 3)
		{
			data |= ((entry->vtuData.memberTagP[3] & 3)<<12) ;
			if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
				data |= ((entry->vtuData.portStateP[3] & 3)<<14) ;
		}

		retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA1_REG,data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		/****************** VTU DATA 2 REG *******************/

		/* get data and wirte to QD_REG_VTU_DATA2_REG (ports 4 to 7) */

		if(dev->maxPorts > 4)
		{
			/* also need to set data register  ports 4 to 6 */

			data =  (entry->vtuData.memberTagP[4] & 3)   |
					((entry->vtuData.memberTagP[5] & 3) << 4);

			if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
				data |= ((entry->vtuData.portStateP[4] & 3) << 2) |
						((entry->vtuData.portStateP[5] & 3) << 6);

			if(dev->maxPorts > 6)
			{
				data |= ((entry->vtuData.memberTagP[6] & 3)<<8) ;
				if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
					data |= ((entry->vtuData.portStateP[6] & 3)<<10) ;
			}

			if(dev->maxPorts > 7)
			{
				data |= ((entry->vtuData.memberTagP[7] & 3)<<12) ;
				if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
					data |= ((entry->vtuData.portStateP[7] & 3)<<14) ;
			}

			if (IS_IN_DEV_GROUP(dev,DEV_VTU_EXT_INFO))
			{
				if(entry->vidExInfo.useVIDFPri == GT_TRUE)
					data |= ((1 << 15) | ((entry->vidExInfo.vidFPri & 0x7) << 12));
				if(entry->vidExInfo.useVIDQPri == GT_TRUE)
					data |= ((1 << 11) | ((entry->vidExInfo.vidQPri & 0x3) << 9));
				if(entry->vidExInfo.vidNRateLimit == GT_TRUE)
					data |= (1 << 8);
			}

			retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA2_REG,data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}
		}


		/****************** VTU DATA 3 REG *******************/

		/* get data and wirte to QD_REG_VTU_DATA3_REG (ports 8 to 10) */

		if(dev->maxPorts > 7)
		{
			/* also need to set data register  ports 8 to 9 */

			data =  (entry->vtuData.memberTagP[8] & 3)   |
					((entry->vtuData.memberTagP[9] & 3) << 4);

			if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
				data |= ((entry->vtuData.portStateP[8] & 3) << 2)	|
						((entry->vtuData.portStateP[9] & 3) << 6);

			if(dev->maxPorts > 10)
			{
				data |= (entry->vtuData.memberTagP[10] & 3) << 8;

				if (IS_IN_DEV_GROUP(dev,DEV_802_1S))
					data |= (entry->vtuData.portStateP[10] & 3) << 10;
			}

			if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
			{
				if(entry->vidPriOverride == GT_TRUE)
					data |= ((1 << 15) | ((entry->vidPriority & 0x7) << 12));
			}

			retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA3_REG,data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}
		}
		else if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
		{
			if(entry->vidPriOverride == GT_TRUE)
				data = ((1 << 15) | ((entry->vidPriority & 0x7) << 12));
			else
				data = 0;

			retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA3_REG,data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}
		}
    }

	/* Set the VID register (QD_REG_VTU_VID_REG) */
	/* There is no need to setup VID reg. on flush and service violation */
	if((vtuOp != FLUSH_ALL) && (vtuOp != SERVICE_VIOLATIONS) )
	{
    	data= ( (entry->vid) & 0xFFF ) | ( (*valid) << 12 );
	    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_VTU_VID_REG),data);
    	if(retVal != GT_OK)
	    {
    		gtSemGive(dev,dev->vtuRegsSem);
	    	return retVal;
    	}
	}

	/* Set SID, FID, VIDPolicy, if it's Load operation */
	if((vtuOp == LOAD_PURGE_ENTRY) && (*valid == 1))
	{
		if(IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
		{
	    	data= (entry->sid) & 0x3F;
		    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_STU_SID_REG),data);
    		if(retVal != GT_OK)
		    {
    			gtSemGive(dev,dev->vtuRegsSem);
	    		return retVal;
	    	}
		}

		data = 0;

		if(IS_IN_DEV_GROUP(dev,DEV_FID_REG))
		{
			if(IS_IN_DEV_GROUP(dev,DEV_POLICY))
			{
	    		data= entry->vidPolicy << 12;
			}

	    	data |= (entry->DBNum & 0xFFF);

		    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_VTU_FID_REG),data);
    		if(retVal != GT_OK)
		    {
    			gtSemGive(dev,dev->vtuRegsSem);
	    		return retVal;
	    	}
		}


	}

	/* Start the VTU Operation by defining the DBNum, vtuOp and VTUBusy    */
	/*
	 * Flush operation will skip the above two setup (for data and vid), and
	 * come to here directly
	 */

	if(vtuOp == FLUSH_ALL)
		data = (1 << 15) | (vtuOp << 12);
	else
	{
		if(IS_IN_DEV_GROUP(dev,DEV_FID_REG))
		{
			data = (1 << 15) | (vtuOp << 12);
		}
		else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
		{
			/* Since DBNum is defined as GT_U8, it cannot be >= 256. */
			#if 0
			if(entry->DBNum >= 256)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return GT_BAD_PARAM;
			}
			#endif
			data = (1 << 15) | (vtuOp << 12) | ((entry->DBNum & 0xF0) << 4) | (entry->DBNum & 0x0F);
		}
		else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
		{
			if(entry->DBNum >= 64)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return GT_BAD_PARAM;
			}
			data = (1 << 15) | (vtuOp << 12) | ((entry->DBNum & 0x30) << 4) | (entry->DBNum & 0x0F);
		}
		else
		{
			if(entry->DBNum >= 16)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return GT_BAD_PARAM;
			}
			data = (1 << 15) | (vtuOp << 12) | entry->DBNum;
		}
	}

	retVal = hwWriteGlobalReg(dev,QD_REG_VTU_OPERATION,data);
	if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->vtuRegsSem);
		return retVal;
	}

	/* only two operations need to go through the mess below to get some data
	 * after the operations -  service violation and get next entry
	 */

	/* If the operation is to service violation operation wait for the response   */
	if(vtuOp == SERVICE_VIOLATIONS)
	{
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

		/* get the Source Port ID that was involved in the violation */
		retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,0,4,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		entry->DBNum = (GT_U8)(data & 0xF);

		/* get the VID that was involved in the violation */

		retVal = hwReadGlobalReg(dev,QD_REG_VTU_VID_REG,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		/* Get the vid - bits 0-11 */
		entry->vid   = data & 0xFFF;


	} /* end of service violations */

	/* If the operation is a get next operation wait for the response   */
	if(vtuOp == GET_NEXT_ENTRY)
	{
		entry->vidExInfo.useVIDFPri = GT_FALSE;
		entry->vidExInfo.vidFPri = 0;

		entry->vidExInfo.useVIDQPri = GT_FALSE;
		entry->vidExInfo.vidQPri = 0;

		entry->vidExInfo.vidNRateLimit = GT_FALSE;

    	entry->sid = 0;
   		entry->vidPolicy = GT_FALSE;

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

		/****************** get the vid *******************/

		retVal = hwReadGlobalReg(dev,QD_REG_VTU_VID_REG,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		/* the vid is bits 0-11 */
		entry->vid   = data & 0xFFF;

		/* the vid valid is bits 12 */
		*valid   = (data >> 12) & 1;

		if (*valid == 0)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return GT_OK;
		}

		/****************** get the SID *******************/
		if(IS_IN_DEV_GROUP(dev,DEV_802_1S_STU))
		{
		    retVal = hwReadGlobalReg(dev,(GT_U8)(QD_REG_STU_SID_REG),&data);
    		if(retVal != GT_OK)
		    {
    			gtSemGive(dev,dev->vtuRegsSem);
	    		return retVal;
	    	}
	    	entry->sid = data & 0x3F;
		}

		/****************** get the DBNum *******************/
		if(IS_IN_DEV_GROUP(dev,DEV_FID_REG))
		{
		    retVal = hwReadGlobalReg(dev,(GT_U8)(QD_REG_VTU_FID_REG),&data);
    		if(retVal != GT_OK)
		    {
    			gtSemGive(dev,dev->vtuRegsSem);
	    		return retVal;
	    	}

			if(IS_IN_DEV_GROUP(dev,DEV_POLICY))
			{
	    		entry->vidPolicy = (data >> 12) & 0x1;
			}

	    	entry->DBNum = data & 0xFFF;

		}
		else
		{
		retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,0,4,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		entry->DBNum = data & 0xF;

		if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
		{
			retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,8,4,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}

			entry->DBNum |= ((data & 0xF) << 4);
		}
		else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
		{
			retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,8,2,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}

			entry->DBNum |= ((data & 0x3) << 4);
		}
		}


		/****************** get the MemberTagP *******************/
		retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA1_REG,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->vtuRegsSem);
			return retVal;
		}

		/* get data from data register for ports 0 to 2 */
		entry->vtuData.memberTagP[0]  =  data & 3 ;
		entry->vtuData.memberTagP[1]  = (data >> 4) & 3 ;
		entry->vtuData.memberTagP[2]  = (data >> 8) & 3 ;
		entry->vtuData.portStateP[0]  = (data >> 2) & 3 ;
		entry->vtuData.portStateP[1]  = (data >> 6) & 3 ;
		entry->vtuData.portStateP[2]  = (data >> 10) & 3 ;

		/****************** for the switch more than 3 ports *****************/

		if(dev->maxPorts > 3)
		{
			/* fullsail has 3 ports, clippership has 7 prots */
			entry->vtuData.memberTagP[3]  = (data >>12) & 3 ;
			entry->vtuData.portStateP[3]  = (data >>14) & 3 ;

			/* get data from data register for ports 4 to 6 */
			retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA2_REG,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}
			entry->vtuData.memberTagP[4]  = data & 3 ;
			entry->vtuData.memberTagP[5]  = (data >> 4) & 3 ;
			entry->vtuData.portStateP[4]  = (data >> 2) & 3 ;
			entry->vtuData.portStateP[5]  = (data >> 6) & 3 ;

			if(dev->maxPorts > 6)
			{
				entry->vtuData.memberTagP[6]  = (data >> 8) & 3 ;
				entry->vtuData.portStateP[6]  = (data >> 10) & 3 ;
			}

			if (IS_IN_DEV_GROUP(dev,DEV_VTU_EXT_INFO))
			{
				entry->vidPriOverride = 0;
				entry->vidPriority = 0;

				entry->vidExInfo.useVIDFPri = (data & 0x8000)?GT_TRUE:GT_FALSE;
				entry->vidExInfo.vidFPri = (data >> 12) & 0x7;

				entry->vidExInfo.useVIDQPri = (data & 0x0800)?GT_TRUE:GT_FALSE;
				entry->vidExInfo.vidQPri = (data >> 9) & 0x3;

				entry->vidExInfo.vidNRateLimit = (data & 0x0100)?GT_TRUE:GT_FALSE;
			}
		}
		/****************** upto 7 port switch *******************/

		/****************** for the switch more than 7 ports *****************/

		if(dev->maxPorts > 7)
		{
			/* fullsail has 3 ports, clippership has 7 prots */
			entry->vtuData.memberTagP[7]  = (data >>12) & 3 ;
			entry->vtuData.portStateP[7]  = (data >>14) & 3 ;

			/* get data from data register for ports 4 to 6 */
			retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA3_REG,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}
			entry->vtuData.memberTagP[8]  = data & 3 ;
			entry->vtuData.memberTagP[9]  = (data >> 4) & 3 ;
			entry->vtuData.portStateP[8]  = (data >> 2) & 3 ;
			entry->vtuData.portStateP[9]  = (data >> 6) & 3 ;

			if(dev->maxPorts > 10)
			{
				entry->vtuData.memberTagP[10]  = (data >> 8) & 3 ;
				entry->vtuData.portStateP[10]  = (data >> 10) & 3 ;
			}

			if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
			{
				if (data & 0x8000)
				{
					entry->vidPriOverride = GT_TRUE;
					entry->vidPriority = (data >> 12) & 0x7;
				}
				else
				{
					entry->vidPriOverride = GT_FALSE;
					entry->vidPriority = 0;
				}
			}

		}
		else if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
		{
			/* get data from data register for ports 4 to 6 */
			retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA3_REG,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->vtuRegsSem);
				return retVal;
			}

			if (data & 0x8000)
			{
				entry->vidPriOverride = GT_TRUE;
				entry->vidPriority = (data >> 12) & 0x7;
			}
			else
			{
				entry->vidPriOverride = GT_FALSE;
				entry->vidPriority = 0;
			}
		}

		/****************** upto 11 ports switch *******************/

	} /* end of get next entry */

	gtSemGive(dev,dev->vtuRegsSem);
	return GT_OK;
}
