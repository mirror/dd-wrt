#include <Copyright.h>

/********************************************************************************
* gtPriTable.c
*
* DESCRIPTION:
*       API definitions for Priority Override Table
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


/*******************************************************************************
* gsysSetQPriOverrideTable
*
* DESCRIPTION:
*       Queue Priority Override.
*		When a frame enters a port, its type is determined and the type is used 
*		to access the Queue Priority Table. If the type's qPriEn (in GT_QPRI_TBL_ENTRY
*		structure) is enabled, then the frame's Queue Priority will be overridden
*		with the value written in qPriority (in GT_QPRI_TBL_ENTRY structure).
*		Frame Types supported are:
*			FTYPE_DSA_TO_CPU_BPDU -
*				Used on multicast DSA To_CPU frames with a Code of 0x0 (BPDU/MGMT).
*				Not used on non-DSA Control frames.
*			FTYPE_DSA_TO_CPU_F2R -
*				Used on DSA To_CPU frames with a Code of 0x1 (Frame to Register
*				Reply). Not used on non-DSA Control frames.
*			FTYPE_DSA_TO_CPU_IGMP -
*				Used on DSA To_CPU frames with a Code of 0x2 (IGMP/MLD Trap)
*				and on non-DSA Control frames that are IGMP or MLD trapped
*			FTYPE_DSA_TO_CPU_TRAP -
*				Used on DSA To_CPU frames with a Code of 0x3 (Policy Trap) and
*				on non-DSA Control frames that are Policy Trapped
*			FTYPE_DSA_TO_CPU_ARP -
*				Used on DSA To_CPU frames with a Code of 0x4 (ARP Mirror) and
*				on non-DSA Control frames that are ARP Mirrored (see gprtSetARPtoCPU API).
*			FTYPE_DSA_TO_CPU_MIRROR -
*				Used on DSA To_CPU frames with a Code of 0x5 (Policy Mirror) and
*				on non-DSA Control frames that are Policy Mirrored (see gprtSetPolicy API).
*			FTYPE_DSA_TO_CPU_RESERVED -
*				Used on DSA To_CPU frames with a Code of 0x6 (Reserved). Not
*				used on non-DSA Control frames.
*			FTYPE_DSA_TO_CPU_UCAST_MGMT -
*				Used on unicast DSA To_CPU frames with a Code of 0x0 (unicast
*				MGMT). Not used on non-DSA Control frames.
*			FTYPE_DSA_FROM_CPU -
*				Used on DSA From_CPU frames. Not used on non-DSA Control frame
*			FTYPE_DSA_CROSS_CHIP_FC -
*				Used on DSA Cross Chip Flow Control frames (To_Sniffer Flow
*				Control). Not used on non-DSA Control frames.
*			FTYPE_DSA_CROSS_CHIP_EGRESS_MON -
*				Used on DSA Cross Chip Egress Monitor frames (To_Sniffer Tx).
*				Not used on non-DSA Control frames.
*			FTYPE_DSA_CROSS_CHIP_INGRESS_MON -
*				Used on DSA Cross Chip Ingress Monitor frames (To_Sniffer Rx).
*				Not used on non-DSA Control frames.
*			FTYPE_PORT_ETYPE_MATCH -
*				Used on normal network ports (see gprtSetFrameMode API)
*				on frames whose Ethertype matches the port's PortEType register.
*				Not used on non-DSA Control frames.
*			FTYPE_BCAST_NON_DSA_CONTROL -
*				Used on Non-DSA Control frames that contain a Broadcast
*				destination address. Not used on DSA Control frames.
*
* INPUTS:
*       fType - frame type (GT_PRI_OVERRIDE_FTYPE)
*       entry - Q Priority Override Table entry (GT_QPRI_TBL_ENTRY)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM     - on unknown frame type
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gsysSetQPriOverrideTable
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_PRI_OVERRIDE_FTYPE	fType,
    IN  GT_QPRI_TBL_ENTRY	*entry
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data,qPri;

    DBG_INFO(("gsysSetQPriOverrideTable Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE_TABLE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	switch (fType)
	{
		case FTYPE_DSA_TO_CPU_BPDU:
		case FTYPE_DSA_TO_CPU_F2R:
		case FTYPE_DSA_TO_CPU_IGMP:
		case FTYPE_DSA_TO_CPU_TRAP:
		case FTYPE_DSA_TO_CPU_ARP:
		case FTYPE_DSA_TO_CPU_MIRROR:
		case FTYPE_DSA_TO_CPU_RESERVED:
		case FTYPE_DSA_TO_CPU_UCAST_MGMT:
		case FTYPE_DSA_FROM_CPU:
		case FTYPE_DSA_CROSS_CHIP_FC:
		case FTYPE_DSA_CROSS_CHIP_EGRESS_MON:
		case FTYPE_DSA_CROSS_CHIP_INGRESS_MON:
		case FTYPE_PORT_ETYPE_MATCH:
		case FTYPE_BCAST_NON_DSA_CONTROL:
			break;
		default:
	        DBG_INFO(("GT_BAD_PARAM\n"));
			return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

    /* Wait until the Priority Override Table is ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE,15,1,&data);
        if(retVal != GT_OK)
        {
			gtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    }

	if (entry->qPriEn)
		qPri = (1 << 3) | (entry->qPriority & 0x3);
	else
		qPri = 0;

	data = (1 << 15) | (fType << 8) | qPri;

	retVal = hwWriteGlobal2Reg(dev, QD_REG_PRIORITY_OVERRIDE, data);

	gtSemGive(dev,dev->tblRegsSem);

    if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
        return retVal;
    }
	
	return GT_OK;
}


/*******************************************************************************
* gsysGetQPriOverrideTable
*
* DESCRIPTION:
*       Queue Priority Override.
*		When a frame enters a port, its type is determined and the type is used 
*		to access the Queue Priority Table. If the type's qPriEn (in GT_QPRI_TBL_ENTRY
*		structure) is enabled, then the frame's Queue Priority will be overridden
*		with the value written in qPriority (in GT_QPRI_TBL_ENTRY structure).
*		Frame Types supported are:
*			FTYPE_DSA_TO_CPU_BPDU -
*				Used on multicast DSA To_CPU frames with a Code of 0x0 (BPDU/MGMT).
*				Not used on non-DSA Control frames.
*			FTYPE_DSA_TO_CPU_F2R -
*				Used on DSA To_CPU frames with a Code of 0x1 (Frame to Register
*				Reply). Not used on non-DSA Control frames.
*			FTYPE_DSA_TO_CPU_IGMP -
*				Used on DSA To_CPU frames with a Code of 0x2 (IGMP/MLD Trap)
*				and on non-DSA Control frames that are IGMP or MLD trapped
*			FTYPE_DSA_TO_CPU_TRAP -
*				Used on DSA To_CPU frames with a Code of 0x3 (Policy Trap) and
*				on non-DSA Control frames that are Policy Trapped
*			FTYPE_DSA_TO_CPU_ARP -
*				Used on DSA To_CPU frames with a Code of 0x4 (ARP Mirror) and
*				on non-DSA Control frames that are ARP Mirrored (see gprtSetARPtoCPU API).
*			FTYPE_DSA_TO_CPU_MIRROR -
*				Used on DSA To_CPU frames with a Code of 0x5 (Policy Mirror) and
*				on non-DSA Control frames that are Policy Mirrored (see gprtSetPolicy API).
*			FTYPE_DSA_TO_CPU_RESERVED -
*				Used on DSA To_CPU frames with a Code of 0x6 (Reserved). Not
*				used on non-DSA Control frames.
*			FTYPE_DSA_TO_CPU_UCAST_MGMT -
*				Used on unicast DSA To_CPU frames with a Code of 0x0 (unicast
*				MGMT). Not used on non-DSA Control frames.
*			FTYPE_DSA_FROM_CPU -
*				Used on DSA From_CPU frames. Not used on non-DSA Control frame
*			FTYPE_DSA_CROSS_CHIP_FC -
*				Used on DSA Cross Chip Flow Control frames (To_Sniffer Flow
*				Control). Not used on non-DSA Control frames.
*			FTYPE_DSA_CROSS_CHIP_EGRESS_MON -
*				Used on DSA Cross Chip Egress Monitor frames (To_Sniffer Tx).
*				Not used on non-DSA Control frames.
*			FTYPE_DSA_CROSS_CHIP_INGRESS_MON -
*				Used on DSA Cross Chip Ingress Monitor frames (To_Sniffer Rx).
*				Not used on non-DSA Control frames.
*			FTYPE_PORT_ETYPE_MATCH -
*				Used on normal network ports (see gprtSetFrameMode API)
*				on frames whose Ethertype matches the port's PortEType register.
*				Not used on non-DSA Control frames.
*			FTYPE_BCAST_NON_DSA_CONTROL -
*				Used on Non-DSA Control frames that contain a Broadcast
*				destination address. Not used on DSA Control frames.
*
* INPUTS:
*       fType - frame type (GT_PRI_OVERRIDE_FTYPE)
*
* OUTPUTS:
*       entry - Q Priority Override Table entry (GT_QPRI_TBL_ENTRY)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM     - on unknown frame type
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gsysGetQPriOverrideTable
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_PRI_OVERRIDE_FTYPE	fType,
    OUT GT_QPRI_TBL_ENTRY	*entry
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysGetQPriOverrideTable Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE_TABLE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	switch (fType)
	{
		case FTYPE_DSA_TO_CPU_BPDU:
		case FTYPE_DSA_TO_CPU_F2R:
		case FTYPE_DSA_TO_CPU_IGMP:
		case FTYPE_DSA_TO_CPU_TRAP:
		case FTYPE_DSA_TO_CPU_ARP:
		case FTYPE_DSA_TO_CPU_MIRROR:
		case FTYPE_DSA_TO_CPU_RESERVED:
		case FTYPE_DSA_TO_CPU_UCAST_MGMT:
		case FTYPE_DSA_FROM_CPU:
		case FTYPE_DSA_CROSS_CHIP_FC:
		case FTYPE_DSA_CROSS_CHIP_EGRESS_MON:
		case FTYPE_DSA_CROSS_CHIP_INGRESS_MON:
		case FTYPE_PORT_ETYPE_MATCH:
		case FTYPE_BCAST_NON_DSA_CONTROL:
			break;
		default:
	        DBG_INFO(("GT_BAD_PARAM\n"));
			return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

    /* Wait until the Priority Override Table is ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE,15,1,&data);
        if(retVal != GT_OK)
        {
			gtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    }

	data = fType << 8;

	retVal = hwWriteGlobal2Reg(dev, QD_REG_PRIORITY_OVERRIDE, data);
    if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }
	
	retVal = hwReadGlobal2Reg(dev, QD_REG_PRIORITY_OVERRIDE, &data);
    if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

	if (data & (1 << 3))
	{
		entry->qPriEn = GT_TRUE;
		entry->qPriority = data & 0x3;
	}
	else
	{
		entry->qPriEn = GT_FALSE;
		entry->qPriority = data & 0x3; /* no meaning, but just in case */
	}

	gtSemGive(dev,dev->tblRegsSem);

	return GT_OK;
}

