#include <Copyright.h>

/********************************************************************************
* msApiPrototype.h
*
* DESCRIPTION:
*       API Prototypes for QuarterDeck Device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __msApiPrototype_h
#define __msApiPrototype_h

#ifdef __cplusplus
extern "C" {
#endif


/* gtBrgFdb.c */

/*******************************************************************************
* gfdbSetAtuSize
*
* DESCRIPTION:
*       Sets the Mac address table size.
*
* INPUTS:
*       size    - Mac address table size.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbSetAtuSize
(
    IN GT_QD_DEV *dev,
    IN ATU_SIZE size
);


/*******************************************************************************
* gfdbGetAgingTimeRange
*
* DESCRIPTION:
*       Gets the maximal and minimum age times that the hardware can support.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       maxTimeout - max aging time in secounds.
*       minTimeout - min aging time in secounds.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAgingTimeRange
(
    IN GT_QD_DEV *dev,
    OUT GT_U32 *maxTimeout,
    OUT GT_U32 *minTimeout
);

/*******************************************************************************
* gfdbGetAgingTimeout
*
* DESCRIPTION:
*       Gets the timeout period in seconds for aging out dynamically learned
*       forwarding information. The returned value may not be the same as the value
*		programmed with <gfdbSetAgingTimeout>. Please refer to the description of
*		<gfdbSetAgingTimeout>.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       timeout - aging time in seconds.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAgingTimeout
(
    IN  GT_QD_DEV    *dev,
    OUT GT_U32       *timeout
);

/*******************************************************************************
* gfdbSetAgingTimeout
*
* DESCRIPTION:
*       Sets the timeout period in seconds for aging out dynamically learned
*       forwarding information. The standard recommends 300 sec.
*		Supported aging timeout values are multiple of time-base, where time-base
*		is either 15 or 16 seconds, depending on the Switch device. For example,
*		88E6063 uses time-base 16, and so supported aging timeouts are 0,16,32,
*		48,..., and 4080. If unsupported timeout value (bigger than 16) is used,
*		the value will be rounded to the nearest supported value smaller than the
*		given timeout. If the given timeout is less than 16, minimum timeout value
*		16 will be used instead. E.g.) 35 becomes 32 and 5 becomes 16.
*		<gfdbGetAgingTimeRange> function can be used to find the time-base.
*
* INPUTS:
*       timeout - aging time in seconds.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbSetAgingTimeout
(
    IN GT_QD_DEV *dev,
    IN GT_U32 timeout
);



/*******************************************************************************
* gfdbGetAtuDynamicCount
*
* DESCRIPTION:
*       Gets the current number of dynamic unicast entries in this
*       Filtering Database.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numDynEntries - number of dynamic entries.
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
GT_STATUS gfdbGetAtuDynamicCount
(
    IN GT_QD_DEV *dev,
    OUT GT_U32 *numDynEntries
);



/*******************************************************************************
* gfdbGetAtuEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic MAC address entry from the ATU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from Mac[00:00:00:00:00:00]
*
*		DBNum in atuEntry -
*			ATU MAC Address Database number. If multiple address
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryFirst
(
    IN GT_QD_DEV *dev,
    OUT GT_ATU_ENTRY    *atuEntry
);



/*******************************************************************************
* gfdbGetAtuEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic MAC address from the specified Mac Addr.
*
* INPUTS:
*       atuEntry - the Mac Address to start the search.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from atu.macAddr[xx:xx:xx:xx:xx:xx] specified by the
*       user.
*
*		DBNum in atuEntry -
*			ATU MAC Address Database number. If multiple address
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryNext
(
    IN GT_QD_DEV *dev,
    INOUT GT_ATU_ENTRY  *atuEntry
);



/*******************************************************************************
* gfdbFindAtuMacEntry
*
* DESCRIPTION:
*       Find FDB entry for specific MAC address from the ATU.
*
* INPUTS:
*       atuEntry - the Mac address to search.
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       atuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*		DBNum in atuEntry -
*			ATU MAC Address Database number. If multiple address
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbFindAtuMacEntry
(
    IN GT_QD_DEV *dev,
    INOUT GT_ATU_ENTRY  *atuEntry,
    OUT GT_BOOL         *found
);



/*******************************************************************************
* gfdbFlush
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the MAC Address
*       Table.
*
* INPUTS:
*       flushCmd - the flush operation type.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlush
(
    IN GT_QD_DEV *dev,
    IN GT_FLUSH_CMD flushCmd
);

/*******************************************************************************
* gfdbFlushInDB
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the particular
*       ATU Database (DBNum). If multiple address databases are being used, this
*		API can be used to flush entries in a particular DBNum database.
*
* INPUTS:
*       flushCmd - the flush operation type.
*		DBNum	 - ATU MAC Address Database Number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlushInDB
(
    IN GT_QD_DEV *dev,
    IN GT_FLUSH_CMD flushCmd,
	IN GT_U32 DBNum
);

/*******************************************************************************
* gfdbAddMacEntry
*
* DESCRIPTION:
*       Creates the new entry in MAC address table.
*
* INPUTS:
*       macEntry    - mac address entry to insert to the ATU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*       GT_NO_RESOURCE    - failed to allocate a t2c struct
*       GT_OUT_OF_CPU_MEM - oaMalloc failed
*
* COMMENTS:
*		DBNum in atuEntry -
*			ATU MAC Address Database number. If multiple address
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbAddMacEntry
(
    IN GT_QD_DEV *dev,
    IN GT_ATU_ENTRY *macEntry
);



/*******************************************************************************
* gfdbDelMacEntry
*
* DESCRIPTION:
*       Deletes MAC address entry.
*
* INPUTS:
*       macAddress - mac address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*       For SVL mode vlan Id is ignored.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbDelMacEntry
(
    IN GT_QD_DEV *dev,
    IN GT_ETHERADDR  *macAddress
);

/*******************************************************************************
* gfdbDelAtuEntry
*
* DESCRIPTION:
*       Deletes ATU entry.
*
* INPUTS:
*       atuEntry - the ATU entry to be deleted.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*		DBNum in atuEntry -
*			ATU MAC Address Database number. If multiple address
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbDelAtuEntry
(
    IN GT_QD_DEV *dev,
    IN GT_ATU_ENTRY  *atuEntry
);

/*******************************************************************************
* gfdbLearnEnable
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       en - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbLearnEnable
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL  en
);


/*******************************************************************************
* gfdbGetLearnEnable
*
* DESCRIPTION:
*       Get automatic learning status of new source MAC addresses on port ingress.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       en - GT_TRUE if enabled  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetLearnEnable
(
    IN GT_QD_DEV    *dev,
    OUT GT_BOOL  *en
);

/*******************************************************************************
* gstpSetMode
*
* DESCRIPTION:
*       This routine Enable the Spanning tree.
*
* INPUTS:
*       en - GT_TRUE for enable, GT_FALSE for disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       when enabled, this function sets all port to blocking state, and inserts
*       the BPDU MAC into the ATU to be captured to CPU, on disable all port are
*       being modified to be in forwarding state.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetMode
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL  en
);



/*******************************************************************************
* gstpSetPortState
*
* DESCRIPTION:
*       This routine set the port state.
*
* INPUTS:
*       port  - the logical port number.
*       state - the port state to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetPortState
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT           port,
    IN GT_PORT_STP_STATE  state
);



/*******************************************************************************
* gstpGetPortState
*
* DESCRIPTION:
*       This routine returns the port state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       state - the current port state.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpGetPortState
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT           port,
    OUT GT_PORT_STP_STATE  *state
);

/*******************************************************************************
* gprtSetEgressMode
*
* DESCRIPTION:
*       This routine set the egress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the egress mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetEgressMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT        port,
    IN GT_EGRESS_MODE  mode
);



/*******************************************************************************
* gprtGetEgressMode
*
* DESCRIPTION:
*       This routine get the egress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the egress mode.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetEgressMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT        port,
    OUT GT_EGRESS_MODE  *mode
);



/*******************************************************************************
* gprtSetVlanTunnel
*
* DESCRIPTION:
*       This routine sets the vlan tunnel mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the vlan tunnel mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetVlanTunnel
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);



/*******************************************************************************
* gprtGetVlanTunnel
*
* DESCRIPTION:
*       This routine get the vlan tunnel mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the vlan tunnel mode..
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetVlanTunnel
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);


/*******************************************************************************
* gprtSetIGMPSnoop
*
* DESCRIPTION:
* 		This routine set the IGMP Snoop. When set to one and this port receives
*		IGMP frame, the frame is switched to the CPU port, overriding all other
*		switching decisions, with exception for CPU's Trailer.
*		CPU port is determined by the Ingress Mode bits. A port is considered
*		the CPU port if its Ingress Mode are either GT_TRAILER_INGRESS or
*		GT_CPUPORT_INGRESS.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for IGMP Snoop or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIGMPSnoop
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* gprtGetIGMPSnoop
*
* DESCRIPTION:
*		This routine get the IGMP Snoop mode.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: IGMP Snoop enabled
*  			GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIGMPSnoop
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/* the following two APIs are added to support clippership */

/*******************************************************************************
* gprtSetHeaderMode
*
* DESCRIPTION:
*		This routine set ingress and egress header mode of a switch port.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for header mode  or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetHeaderMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* gprtGetHeaderMode
*
* DESCRIPTION:
*		This routine gets ingress and egress header mode of a switch port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: header mode enabled
*  			GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHeaderMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);


/*******************************************************************************
* gprtSetProtectedMode
*
* DESCRIPTION:
*		This routine set protected mode of a switch port.
*		When this mode is set to GT_TRUE, frames are allowed to egress port
*		defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*		by the port's VLANTable if 802.1Q is enabled on the port. Both must
*		allow the frame to Egress.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for protected mode or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetProtectedMode
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
);

/*******************************************************************************
* gprtGetProtectedMode
*
* DESCRIPTION:
*		This routine gets protected mode of a switch port.
*		When this mode is set to GT_TRUE, frames are allowed to egress port
*		defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*		by the port's VLANTable if 802.1Q is enabled on the port. Both must
*		allow the frame to Egress.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: header mode enabled
*  			GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetProtectedMode
(
    IN  GT_QD_DEV		*dev,
    IN  GT_LPORT		port,
    OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtSetForwardUnknown
*
* DESCRIPTION:
*		This routine set Forward Unknown mode of a switch port.
*		When this mode is set to GT_TRUE, normal switch operation occurs.
*		When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*		will not egress out this port.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for protected mode or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetForwardUnknown
(
    IN GT_QD_DEV	*dev,
    IN GT_LPORT	port,
    IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetForwardUnknown
*
* DESCRIPTION:
*		This routine gets Forward Unknown mode of a switch port.
*		When this mode is set to GT_TRUE, normal switch operation occurs.
*		When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*		will not egress out this port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: header mode enabled
*				GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetForwardUnknown
(
    IN  GT_QD_DEV		*dev,
    IN  GT_LPORT		port,
    OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtGetSwitchReg
*
* DESCRIPTION:
*       This routine reads Switch Port Registers.
*
* INPUTS:
*       port    - logical port number
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSwitchReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
);

/*******************************************************************************
* gprtSetSwitchReg
*
* DESCRIPTION:
*       This routine writes Switch Port Registers.
*
* INPUTS:
*       port    - logical port number
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetSwitchReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
    IN  GT_U32	     regAddr,
    IN  GT_U16	     data
);


/*******************************************************************************
* gprtGetGlobalReg
*
* DESCRIPTION:
*       This routine reads Switch Global Registers.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetGlobalReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
);

/*******************************************************************************
* gprtSetGlobalReg
*
* DESCRIPTION:
*       This routine writes Switch Global Registers.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetGlobalReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     regAddr,
    IN  GT_U16	     data
);



/*******************************************************************************
* gvlnSetPortVlanPorts
*
* DESCRIPTION:
*       This routine sets the port VLAN group port membership list.
*
* INPUTS:
*       port        - logical port number to set.
*       memPorts    - array of logical ports.
*       memPortsLen - number of members in memPorts array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanPorts
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_LPORT memPorts[],
    IN GT_U8    memPortsLen
);



/*******************************************************************************
* gvlnGetPortVlanPorts
*
* DESCRIPTION:
*       This routine gets the port VLAN group port membership list.
*
* INPUTS:
*       port        - logical port number to set.
*
* OUTPUTS:
*       memPorts    - array of logical ports.
*       memPortsLen - number of members in memPorts array
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanPorts
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_LPORT memPorts[],
    OUT GT_U8    *memPortsLen
);




/*******************************************************************************
* gvlnSetPortUserPriLsb
*
* DESCRIPTION:
*       This routine Set the user priority (VPT) LSB bit, to be added to the
*       user priority on the egress.
*
* INPUTS:
*       port       - logical port number to set.
*       userPriLsb - GT_TRUE for 1, GT_FALSE for 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortUserPriLsb
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  userPriLsb
);



/*******************************************************************************
* gvlnGetPortUserPriLsb
*
* DESCRIPTION:
*       This routine gets the user priority (VPT) LSB bit.
*
* INPUTS:
*       port       - logical port number to set.
*
* OUTPUTS:
*       userPriLsb - GT_TRUE for 1, GT_FALSE for 0.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortUserPriLsb
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *userPriLsb
);


/*******************************************************************************
* gvlnSetPortVid
*
* DESCRIPTION:
*       This routine Set the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*       vid  - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVid
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_U16   vid
);


/*******************************************************************************
* gvlnGetPortVid
*
* DESCRIPTION:
*       This routine Get the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*
* OUTPUTS:
*       vid  - the port vlan id.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVid
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U16   *vid
);

/*******************************************************************************
* gvlnSetPortVlanDBNum
*
* DESCRIPTION:
*       This routine sets the port VLAN database number (DBNum).
*
* INPUTS:
*       port	- logical port number to set.
*       DBNum 	- database number for this port
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanDBNum
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_U32   DBNum
);


/*******************************************************************************
* gvlnGetPortVlanDBNum
*
* DESCRIPTION:IN GT_INGRESS_MODE mode
*       This routine gets the port VLAN database number (DBNum).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       DBNum 	- database number for this port
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanDBNum
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U32    *DBNum
);

/********************************************************************
* gvlnSetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine sets the port 802.1q mode (11:10)
*
* INPUTS:
*       port	- logical port number to set.
*       mode 	- 802.1q mode for this port
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanDot1qMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT 	port,
    IN GT_DOT1Q_MODE	mode
);

/*******************************************************************************
* gvlnGetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine gets the port 802.1q mode (bit 11:10).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       mode 	- 802.1q mode for this port
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanDot1qMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_DOT1Q_MODE    *mode
);


/********************************************************************
* gvlnSetPortVlanForceDefaultVID
*
* DESCRIPTION:
*       This routine sets the port 802.1q mode (11:10)
*
* INPUTS:
*       port	- logical port number to set.
*       mode    - GT_TRUE, force to use default VID
*                 GT_FAULSE, otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanForceDefaultVID
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT 	port,
    IN GT_BOOL  	mode
);

/*******************************************************************************
* gvlnGetPortVlanForceDefaultVID
*
* DESCRIPTION:
*       This routine gets the port mode for ForceDefaultVID (bit 12).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       mode 	- ForceDefaultVID mode for this port
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanForceDefaultVID
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT 	 port,
    OUT GT_BOOL    	*mode
);

/*******************************************************************************
* eventSetActive
*
* DESCRIPTION:
*       This routine enables/disables the receive of an hardware driven event.
*
* INPUTS:
*       eventType - the event type. any combination of the folowing:
*       	GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,
*       	GT_ATU_DONE, GT_PHY_INTERRUPT, GT_EE_INTERRUPT, and GT_DEVICE_INT
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Each switch device has its own set of event Types. Please refer to the
*		device datasheet for the list of event types that the device supports.
*
*******************************************************************************/
GT_STATUS eventSetActive
(
    IN GT_QD_DEV 	*dev,
    IN GT_U32 		eventType
);

/*******************************************************************************
* eventGetIntStatus
*
* DESCRIPTION:
*       This routine reads an hardware driven event status.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       intCause -  It provides the source of interrupt of the following:
*       GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,
*       GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT.
*		For Gigabit Switch, GT_ATU_FULL is replaced with GT_ATU_FULL and
*		GT_PHY_INTERRUPT is not supported.
*				GT_DEVICE_INT may not be available on the devices, so please refer
*				to the datasheet for details.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS eventGetIntStatus
(
    IN  GT_QD_DEV 	*dev,
    OUT GT_U16		*intCause
);

/*******************************************************************************
* gvtuGetIntStatus
*
* DESCRIPTION:
* 		Check to see if a specific type of VTU interrupt occured
*
* INPUTS:
*       intType - the type of interrupt which causes an interrupt.
*			any combination of
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_FULL_VIOLATION
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK   - on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		FULL_VIOLATION is only for Fast Ethernet Switch (not for Gigabit Switch).
*
*******************************************************************************/

GT_STATUS gvtuGetIntStatus
(
    IN  GT_QD_DEV 			*dev,
    OUT GT_VTU_INT_STATUS 	*vtuIntStatus
);

/*******************************************************************************
* gvtuGetEntryCount
*
* DESCRIPTION:
*       Gets the current number of entries in the VTU table
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
	IN  GT_QD_DEV	*dev,
	OUT GT_U32 		*numEntries
);

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
	IN  GT_QD_DEV 		*dev,
	OUT GT_VTU_ENTRY	*vtuEntry
);

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
	IN  GT_QD_DEV 		*dev,
	INOUT GT_VTU_ENTRY  *vtuEntry
);

/*******************************************************************************
* gvtuFindVidEntry
*
* DESCRIPTION:
*       Find VTU entry for a specific VID, it will return the entry, if found,
*       along with its associated data
*
* INPUTS:
*       vtuEntry - contains the VID to search for.
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
	IN GT_QD_DEV 		*dev,
	INOUT GT_VTU_ENTRY  *vtuEntry,
	OUT GT_BOOL         *found
);

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
    IN GT_QD_DEV *dev
);

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
    IN GT_QD_DEV 	*dev,
    IN GT_VTU_ENTRY *vtuEntry
);

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
    IN GT_QD_DEV 	*dev,
    IN GT_VTU_ENTRY *vtuEntry
);

/* gtPhyCtrl.c */

/*******************************************************************************
* gprtPhyReset
*
* DESCRIPTION:
*		This routine preforms PHY reset.
*		After reset, phy will be in Autonegotiation mode.
*
* INPUTS:
* 		port - The logical port number
*
* OUTPUTS:
* 		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
* COMMENTS:
* 		data sheet register 0.15 - Reset
* 		data sheet register 0.13 - Speed
* 		data sheet register 0.12 - Autonegotiation
* 		data sheet register 0.8  - Duplex Mode
*
*******************************************************************************/

GT_STATUS gprtPhyReset
(
    IN GT_QD_DEV 	*dev,
    IN GT_LPORT 	port
);


/*******************************************************************************
* gprtSetPortLoopback
*
* DESCRIPTION:
* Enable/Disable Internal Port Loopback.
* For 10/100 Fast Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled, this routine disables Auto-Negotiation and
*   forces speed to be 10Mbps.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   Disabling Loopback simply clears bit 14 of control register(0.14). Therefore,
*   it is recommended to call gprtSetPortAutoMode for PHY configuration after
*   Loopback test.
* For 10/100/1000 Gigagbit Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled and Link is active, the current speed is used.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   All other cases, default MAC Interface speed is used. Please refer to the data
*   sheet for the information of the default MAC Interface speed.
*
* INPUTS:
* 		port - logical port number
* 		enable - If GT_TRUE, enable loopback mode
* 					If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.14 - Loop_back
*
*******************************************************************************/

GT_STATUS gprtSetPortLoopback
(
	IN GT_QD_DEV 	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL 		enable
);


/*******************************************************************************
* gprtSetPortSpeed
*
* DESCRIPTION:
* 		Sets speed for a specific logical port. This function will keep the duplex
*		mode and loopback mode to the previous value, but disable others, such as
*		Autonegotiation.
*
* INPUTS:
* 		port  - logical port number
* 		speed - port speed.
*				PHY_SPEED_10_MBPS for 10Mbps
*				PHY_SPEED_100_MBPS for 100Mbps
*				PHY_SPEED_1000_MBPS for 1000Mbps
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.13 - Speed Selection (LSB)
* data sheet register 0.6  - Speed Selection (MSB)
*
*******************************************************************************/

GT_STATUS gprtSetPortSpeed
(
	IN GT_QD_DEV 	*dev,
	IN GT_LPORT 	port,
	IN GT_PHY_SPEED	speed
);


/*******************************************************************************
* gprtPortAutoNegEnable
*
* DESCRIPTION:
* 		Enable/disable an Auto-Negotiation for duplex mode on specific
* 		logical port. When Autonegotiation is disabled, phy will be in 10Mbps Half
*		Duplex mode. Enabling Autonegotiation will set 100BASE-TX Full Duplex,
*		100BASE-TX Full Duplex, 100BASE-TX Full Duplex, and 100BASE-TX Full Duplex
*		in AutoNegotiation Advertisement register.
*
* INPUTS:
* 		port - logical port number
* 		state - GT_TRUE for enable Auto-Negotiation for duplex mode,
* 					GT_FALSE otherwise
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.12 - Auto-Negotiation Enable
* 		data sheet register 4.8, 4.7, 4.6, 4.5 - Auto-Negotiation Advertisement
*
*******************************************************************************/

GT_STATUS gprtPortAutoNegEnable
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL 		state
);


/*******************************************************************************
* gprtPortPowerDown
*
* DESCRIPTION:
* 		Enable/disable (power down) on specific logical port. When this function
*		is called with normal operation request, phy will set to Autonegotiation
*		mode.
*
* INPUTS:
* 		port	- logical port number
* 		state	-  GT_TRUE: power down
* 					GT_FALSE: normal operation
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.11 - Power Down
*
*******************************************************************************/

GT_STATUS gprtPortPowerDown
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		state
);


/*******************************************************************************
* gprtPortRestartAutoNeg
*
* DESCRIPTION:
* 		Restart AutoNegotiation. If AutoNegotiation is not enabled, it'll enable
*		it. Loopback and Power Down will be disabled by this routine.
*
* INPUTS:
* 		port - logical port number
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.9 - Restart Auto-Negotiation
*
*******************************************************************************/

GT_STATUS gprtPortRestartAutoNeg
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
);


/*******************************************************************************
* gprtSetPortDuplexMode
*
* DESCRIPTION:
* 		Sets duplex mode for a specific logical port. This function will keep
*		the speed and loopback mode to the previous value, but disable others,
*		such as Autonegotiation.
*
* INPUTS:
* 		port 	- logical port number
* 		dMode	- dulpex mode
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/

GT_STATUS gprtSetPortDuplexMode
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		dMode
);


/*******************************************************************************
* gprtSetPortAutoMode
*
* DESCRIPTION:
* 		This routine sets up the port with given Auto Mode.
*		Supported mode is as follows:
*		- Auto for both speed and duplex.
*		- Auto for speed only and Full duplex.
*		- Auto for speed only and Half duplex.
*		- Auto for duplex only and speed 1000Mbps.
*		- Auto for duplex only and speed 100Mbps.
*		- Auto for duplex only and speed 10Mbps.
*		- Speed 1000Mbps and Full duplex.
*		- Speed 1000Mbps and Half duplex.
*		- Speed 100Mbps and Full duplex.
*		- Speed 100Mbps and Half duplex.
*		- Speed 10Mbps and Full duplex.
*		- Speed 10Mbps and Half duplex.
*
*
* INPUTS:
* 		port - The logical port number
* 		mode - Auto Mode to be written
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
* COMMENTS:
* 		data sheet register 4.8, 4.7, 4.6, and 4.5 Autonegotiation Advertisement
* 		data sheet register 4.6, 4.5 Autonegotiation Advertisement for 1000BX
* 		data sheet register 9.9, 9.8 Autonegotiation Advertisement for 1000BT
*******************************************************************************/

GT_STATUS gprtSetPortAutoMode
(
	IN GT_QD_DEV 	*dev,
	IN GT_LPORT 	port,
	IN GT_PHY_AUTO_MODE mode
);

/*******************************************************************************
* gprtSetPause
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*		Register. And restart the autonegotiation.
*
* INPUTS:
* port - The logical port number
* state - GT_PHY_PAUSE_MODE enum value.
*			GT_PHY_NO_PAUSE		- disable pause
* 			GT_PHY_PAUSE		- support pause
*			GT_PHY_ASYMMETRIC_PAUSE	- support asymmetric pause
*			GT_PHY_BOTH_PAUSE	- support both pause and asymmetric pause
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtSetPause
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_PHY_PAUSE_MODE state
);

/*******************************************************************************
* gprtGetPhyReg
*
* DESCRIPTION:
*       This routine reads Phy Registers.
*
* INPUTS:
*       port    - logical port number
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
);

/*******************************************************************************
* gprtSetPhyReg
*
* DESCRIPTION:
*       This routine writes Phy Registers.
*
* INPUTS:
*       port    - logical port number
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPhyReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
    IN  GT_U32	     regAddr,
    IN  GT_U16	     data
);


/*******************************************************************************
* gprtPhyIntEnable
*
* DESCRIPTION:
* Enable/Disable one PHY Interrupt
* This register determines whether the INT# pin is asserted when an interrupt
* event occurs. When an interrupt occurs, the corresponding bit is set and
* remains set until register 19 is read via the SMI. When interrupt enable
* bits are not set in register 18, interrupt status bits in register 19 are
* still set when the corresponding interrupt events occur. However, the INT#
* is not asserted.
*
* INPUTS:
* port    - logical port number
* intType - the type of interrupt to enable/disable. any combination of
*			GT_SPEED_CHANGED,
*			GT_DUPLEX_CHANGED,
*			GT_PAGE_RECEIVED,
*			GT_AUTO_NEG_COMPLETED,
*			GT_LINK_STATUS_CHANGED,
*			GT_SYMBOL_ERROR,
*			GT_FALSE_CARRIER,
*			GT_FIFO_FLOW,
*			GT_CROSSOVER_CHANGED,
*			GT_POLARITY_CHANGED, and
*			GT_JABBER
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 18
*
*******************************************************************************/

GT_STATUS gprtPhyIntEnable
(
IN GT_QD_DEV *dev,
IN GT_LPORT   port,
IN GT_U16	intType
);


/*******************************************************************************
* gprtGetPhyIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of  interrupt occured
*
* INPUTS:
* port - logical port number
* intType - the type of interrupt which causes an interrupt.
*			any combination of
*			GT_SPEED_CHANGED,
*			GT_DUPLEX_CHANGED,
*			GT_PAGE_RECEIVED,
*			GT_AUTO_NEG_COMPLETED,
*			GT_LINK_STATUS_CHANGED,
*			GT_SYMBOL_ERROR,
*			GT_FALSE_CARRIER,
*			GT_FIFO_FLOW,
*			GT_CROSSOVER_CHANGED,
*			GT_POLARITY_CHANGED, and
*			GT_JABBER
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 19
*
*******************************************************************************/

GT_STATUS gprtGetPhyIntStatus
(
IN GT_QD_DEV *dev,
IN  GT_LPORT port,
OUT  GT_U16* intType
);

/*******************************************************************************
* gprtGetPhyIntPortSummary
*
* DESCRIPTION:
* Lists the ports that have active interrupts. It provides a quick way to
* isolate the interrupt so that the MAC or switch does not have to poll the
* interrupt status register (19) for all ports. Reading this register does not
* de-assert the INT# pin
*
* INPUTS:
* none
*
* OUTPUTS:
* GT_U8 *intPortMask - bit Mask with the bits set for the corresponding
* phys with active interrupt. E.g., the bit number 0 and 2 are set when
* port number 0 and 2 have active interrupt
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 20
*
*******************************************************************************/

GT_STATUS gprtGetPhyIntPortSummary
(
IN GT_QD_DEV *dev,
OUT GT_U16 *intPortMask
);



/*******************************************************************************
* gprtSetForceFc
*
* DESCRIPTION:
*       This routine set the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetForceFc
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  force
);



/*******************************************************************************
* gprtGetForceFc
*
* DESCRIPTION:
*       This routine get the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetForceFc
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *force
);



/*******************************************************************************
* gprtSetTrailerMode
*
* DESCRIPTION:
*       This routine set the egress trailer mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetTrailerMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);



/*******************************************************************************
* gprtGetTrailerMode
*
* DESCRIPTION:
*       This routine get the egress trailer mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetTrailerMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtSetIngressMode
*
* DESCRIPTION:
*       This routine set the ingress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the ingress mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIngressMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT        port,
    IN GT_INGRESS_MODE mode
);



/*******************************************************************************
* gprtGetIngressMode
*
* DESCRIPTION:
*       This routine get the ingress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the ingress mode.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIngressMode
(
    IN GT_QD_DEV        *dev,
    IN  GT_LPORT        port,
    OUT GT_INGRESS_MODE *mode
);



/*******************************************************************************
* gprtSetMcRateLimit
*
* DESCRIPTION:
*       This routine set the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetMcRateLimit
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT     port,
    IN GT_MC_RATE   rate
);



/*******************************************************************************
* gprtGetMcRateLimit
*
* DESCRIPTION:
*       This routine Get the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetMcRateLimit
(
    IN GT_QD_DEV    *dev,
    IN  GT_LPORT    port,
    OUT GT_MC_RATE  *rate
);



/*******************************************************************************
* gprtSetCtrMode
*
* DESCRIPTION:
*       This routine sets the port counters mode of operation.
*
* INPUTS:
*       mode  - the counter mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetCtrMode
(
    IN GT_QD_DEV *dev,
    IN GT_CTR_MODE  mode
);



/*******************************************************************************
* gprtClearAllCtr
*
* DESCRIPTION:
*       This routine clears all port counters.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtClearAllCtr
(
    IN GT_QD_DEV *dev
);


/*******************************************************************************
* gprtGetPortCtr
*
* DESCRIPTION:
*       This routine gets the port counters.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       ctr - the counters value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortCtr
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT        port,
    OUT GT_PORT_STAT    *ctr
);




/*******************************************************************************
* gprtGetPartnerLinkPause
*
* DESCRIPTION:
*       This routine retrives the link partner pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPartnerLinkPause
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetSelfLinkPause
*
* DESCRIPTION:
*       This routine retrives the link pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSelfLinkPause
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetResolve
*
* DESCRIPTION:
*       This routine retrives the resolve state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Done  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetResolve
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetLinkState
*
* DESCRIPTION:
*       This routine retrives the link state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Up  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetLinkState
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetPortMode
*
* DESCRIPTION:
*       This routine retrives the port mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtGetPhyMode
*
* DESCRIPTION:
*       This routine retrives the PHY mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII PHY  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtGetDuplex
*
* DESCRIPTION:
*       This routine retrives the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for Full  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDuplex
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtGetSpeed
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       speed - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for 100Mb/s  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSpeed
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *speed
);

/*******************************************************************************
* gprtSetDuplex
*
* DESCRIPTION:
*       This routine sets the duplex mode of MII/SNI/RMII ports.
*
* INPUTS:
*       port - 	the logical port number.
*				(for FullSail, it will be port 2, and for ClipperShip,
*				it could be either port 5 or port 6.)
*       mode -  GT_TRUE for Full Duplex,
*				GT_FALSE for Half Duplex.
*
* OUTPUTS: None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDuplex
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    IN  GT_BOOL  mode
);


/*******************************************************************************
* gqosSetPortDefaultTc
*
* DESCRIPTION:
*       Sets the default traffic class for a specific port.
*
* INPUTS:
*       port      - logical port number
*       trafClass - default traffic class of a port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gcosSetPortDefaultTc
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_U8    trafClass
);


/*******************************************************************************
* gcosGetPortDefaultTc
*
* DESCRIPTION:
*       Gets the default traffic class for a specific port.
*
* INPUTS:
*       port      - logical port number
*
* OUTPUTS:
*       trafClass - default traffic class of a port.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gcosGetPortDefaultTc
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    OUT GT_U8     *trafClass
);


/*******************************************************************************
* gqosSetPrioMapRule
*
* DESCRIPTION:
*       This routine sets priority mapping rule.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetPrioMapRule
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);



/*******************************************************************************
* gqosGetPrioMapRule
*
* DESCRIPTION:
*       This routine get the priority mapping rule.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetPrioMapRule
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gqosIpPrioMapEn
*
* DESCRIPTION:
*       This routine enables the IP priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosIpPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  en
);



/*******************************************************************************
* gqosGetIpPrioMapEn
*
* DESCRIPTION:
*       This routine return the IP priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetIpPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *en
);



/*******************************************************************************
* gqosUserPrioMapEn
*
* DESCRIPTION:
*       This routine enables the user priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosUserPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  en
);



/*******************************************************************************
* gqosGetUserPrioMapEn
*
* DESCRIPTION:
*       This routine return the user priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetUserPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *en
);



/*******************************************************************************
* gcosGetUserPrio2Tc
*
* DESCRIPTION:
*       Gets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosGetUserPrio2Tc
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    userPrior,
    OUT GT_U8   *trClass
);


/*******************************************************************************
* gcosSetUserPrio2Tc
*
* DESCRIPTION:
*       Sets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority of a port.
*       trClass   - the Traffic Class the received frame is assigned.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosSetUserPrio2Tc
(
    IN GT_QD_DEV *dev,
    IN GT_U8    userPrior,
    IN GT_U8    trClass
);


/*******************************************************************************
* gcosGetDscp2Tc
*
* DESCRIPTION:
*       This routine retrieves the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to query.
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosGetDscp2Tc
(
    IN GT_QD_DEV *dev,
    IN  GT_U8   dscp,
    OUT GT_U8   *trClass
);


/*******************************************************************************
* gcosSetDscp2Tc
*
* DESCRIPTION:
*       This routine sets the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to map.
*       trClass - the Traffic Class the received frame is assigned.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosSetDscp2Tc
(
    IN GT_QD_DEV *dev,
    IN GT_U8    dscp,
    IN GT_U8    trClass
);


/*******************************************************************************
* qdLoadDriver
*
* DESCRIPTION:
*       QuarterDeck Driver Initialization Routine.
*       This is the first routine that needs be called by system software.
*       It takes sysCfg from system software, and retures a pointer (*dev)
*       to a data structure which includes infomation related to this QuarterDeck
*       device. This pointer (*dev) is then used for all the API functions.
*
* INPUTS:
*       sysCfg      - Holds system configuration parameters.
*
* OUTPUTS:
*       dev         - Holds general system information.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_ALREADY_EXIST    - if device already started
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
* 	qdUnloadDriver is provided when the driver is not to be used anymore.
*
*******************************************************************************/
GT_STATUS qdLoadDriver
(
    IN  GT_SYS_CONFIG   *sysCfg,
    OUT GT_QD_DEV	*dev
);


/*******************************************************************************
* qdUnloadDriver
*
* DESCRIPTION:
*       This function unloads the QuaterDeck Driver.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           qdLoadDriver().
*
*******************************************************************************/
GT_STATUS qdUnloadDriver
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* sysEnable
*
* DESCRIPTION:
*       This function enables the system for full operation.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS sysEnable
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* gsysSwReset
*
* DESCRIPTION:
*       This routine preforms switch software reset.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSwReset
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* gsysSetDiscardExcessive
*
* DESCRIPTION:
*       This routine set the Discard Excessive state.
*
* INPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDiscardExcessive
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL en
);



/*******************************************************************************
* gsysGetDiscardExcessive
*
* DESCRIPTION:
*       This routine get the Discard Excessive state.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDiscardExcessive
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL *en
);



/*******************************************************************************
* gsysSetSchedulingMode
*
* DESCRIPTION:
*       This routine set the Scheduling Mode.
*
* INPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetSchedulingMode
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL mode
);



/*******************************************************************************
* gsysGetSchedulingMode
*
* DESCRIPTION:
*       This routine get the Scheduling Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetSchedulingMode
(
    IN GT_QD_DEV *dev,
    OUT GT_BOOL *mode
);



/*******************************************************************************
* gsysSetMaxFrameSize
*
* DESCRIPTION:
*       This routine Set the max frame size allowed.
*
* INPUTS:
*       mode - GT_TRUE max size 1522, GT_FALSE max size 1535.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetMaxFrameSize
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL mode
);



/*******************************************************************************
* gsysGetMaxFrameSize
*
* DESCRIPTION:
*       This routine Get the max frame size allowed.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE max size 1522, GT_FALSE max size 1535.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMaxFrameSize
(
    IN GT_QD_DEV *dev,
    OUT GT_BOOL *mode
);



/*******************************************************************************
* gsysReLoad
*
* DESCRIPTION:
*       This routine cause to the switch to reload the EEPROM.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysReLoad
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* gsysSetWatchDog
*
* DESCRIPTION:
*       This routine Set the the watch dog mode.
*
* INPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetWatchDog
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL en
);



/*******************************************************************************
* gsysGetWatchDog
*
* DESCRIPTION:
*       This routine Get the the watch dog mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetWatchDog
(
    IN GT_QD_DEV* dev,
    OUT GT_BOOL *en
);


/*******************************************************************************
* gsysSetDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets the full duplex pause src Mac Address.
*
* INPUTS:
*       mac - The Mac address to be set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    IN GT_ETHERADDR *mac
);


/*******************************************************************************
* gsysGetDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets the full duplex pause src Mac Address.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mac - the Mac address.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    OUT GT_ETHERADDR *mac
);



/*******************************************************************************
* gsysSetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetPerPortDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL en
);



/*******************************************************************************
* gsysGetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPerPortDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL *en
);


/*******************************************************************************
* gsysReadMiiRegister
*
* DESCRIPTION:
*       This routine reads QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know exactly which phy address(0 ~ 0x1F) will be read.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysReadMiiReg
(
    IN GT_QD_DEV* dev,
    IN  GT_U32	phyAddr,
    IN  GT_U32	regAddr,
    OUT GT_U32	*data
);

/*******************************************************************************
* gsysWriteMiiRegister
*
* DESCRIPTION:
*       This routine writes QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know exactly which phy address(0 ~ 0x1F) will be read.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*       data    - data to be written.
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysWriteMiiReg
(
    IN GT_QD_DEV* dev,
    IN  GT_U32	phyAddr,
    IN  GT_U32	regAddr,
    IN  GT_U16	data
);

/*******************************************************************************
* gsysGetSW_Mode
*
* DESCRIPTION:
*       This routine get the Switch mode. These two bits returen
*       the current value of the SW_MODE[1:0] pins.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 		This feature is for both clippership and fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetSW_Mode
(
    IN GT_QD_DEV* dev,
    IN GT_SW_MODE *mode
);

/*******************************************************************************
* gsysGetInitReady
*
* DESCRIPTION:
*       This routine get the InitReady bit. This bit is set to a one when the ATU,
*       the Queue Controller and the Statistics Controller are done with their
*       initialization and are ready to accept frames.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: switch is ready, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 		This feature is for both clippership and fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetInitReady
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL *mode
);


/*******************************************************************************
* gstatsFlushAll
*
* DESCRIPTION:
*       Flush All RMON counters for all ports.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsFlushAll
(
    IN GT_QD_DEV* dev
);

/*******************************************************************************
* gstatsFlushPort
*
* DESCRIPTION:
*       Flush All RMON counters for a given port.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsFlushPort
(
    IN GT_QD_DEV* dev,
    IN GT_LPORT	port
);

/*******************************************************************************
* gstatsGetPortCounter
*
* DESCRIPTION:
*		This routine gets a specific counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS	counter,
	OUT GT_U32			*statsData
);

/*******************************************************************************
* gstatsGetPortAllCounters
*
* DESCRIPTION:
*       This routine gets all RMON counters of the given port
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters
(
	IN  GT_QD_DEV* dev,
	IN  GT_LPORT		port,
	OUT GT_STATS_COUNTER_SET	*statsCounterSet
);


/*******************************************************************************
* grcSetLimitMode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*       mode 	- rate control ingress limit mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*******************************************************************************/
GT_STATUS grcSetLimitMode
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT 	     port,
    IN GT_RATE_LIMIT_MODE    mode
);

/*******************************************************************************
* grcGetLimitMode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       mode 	- rate control ingress limit mode.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetLimitMode
(
    IN GT_QD_DEV* dev,
    IN  GT_LPORT port,
    OUT GT_RATE_LIMIT_MODE    *mode
);

/*******************************************************************************
* grcSetPri3Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS grcSetPri3Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* grcGetPri3Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri3Rate
(
    IN GT_QD_DEV* dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* grcSetPri2Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS grcSetPri2Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* grcGetPri2Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri2Rate
(
    IN GT_QD_DEV*            dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* grcSetPri1Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS grcSetPri1Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* grcGetPri1Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri1Rate
(
    IN GT_QD_DEV*            dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* grcSetPri0Rate
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached
*       	  or exceeded.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcSetPri0Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT        port,
    IN GT_PRI0_RATE    rate
);

/*******************************************************************************
* grcGetPri0Rate
*
* DESCRIPTION:
*       This routine gets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number to set.
*
* OUTPUTS:
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached
*       	  or exceeded.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri0Rate
(
    IN GT_QD_DEV*            dev,
    IN  GT_LPORT port,
    OUT GT_PRI0_RATE    *rate
);

/*******************************************************************************
* grcSetBytesCount
*
* DESCRIPTION:
*       This routine sets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	  - logical port number to set.
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcSetBytesCount
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT        	port,
    IN GT_BOOL 		limitMGMT,
    IN GT_BOOL 		countIFG,
    IN GT_BOOL 		countPre
);

/*******************************************************************************
* grcGetBytesCount
*
* DESCRIPTION:
*       This routine gets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	- logical port number
*
* OUTPUTS:
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetBytesCount
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT        	port,
    IN GT_BOOL 		*limitMGMT,
    IN GT_BOOL 		*countIFG,
    IN GT_BOOL 		*countPre
);

/*******************************************************************************
* grcSetEgressRate
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*
*
* INPUTS:
*       port	- logical port number.
*       rateType  - egress data rate limit (GT_ERATE_TYPE union type).
*					union type is used to support multiple devices with the
*					different formats of egress rate.
*					GT_ERATE_TYPE has the following fields:
*						definedRate - GT_EGRESS_RATE enum type should used for the
*							following devices:
*							88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183,
*							88E6093, 88E6095, 88E6185, 88E6108, 88E6065, 88E6061,
*							and their variations
*						kbRate - rate in kbps that should used for the following
*							devices:
*							88E6097, 88E6096 with the GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_LAYER1,
*								GT_PIRL_ELIMIT_LAYER2, or
*								GT_PIRL_ELIMIT_LAYER3 (see grcSetELimitMode)
*							64kbps ~ 1Mbps    : increments of 64kbps,
*							1Mbps ~ 100Mbps   : increments of 1Mbps, and
*							100Mbps ~ 1000Mbps: increments of 10Mbps
*							Therefore, the valid values are:
*								64, 128, 192, 256, 320, 384,..., 960,
*								1000, 2000, 3000, 4000, ..., 100000,
*								110000, 120000, 130000, ..., 1000000.
*						fRate - frame per second that should used for the following
*							devices:
*							88E6097, 88E6096 with GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_FRAME
*							Valid values are between 7600 and 1488000
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_EGRESS_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
*******************************************************************************/
GT_STATUS grcSetEgressRate
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT        port,
    IN GT_ERATE_TYPE   *rateType
);

/*******************************************************************************
* grcGetEgressRate
*
* DESCRIPTION:
*       This routine gets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rateType  - egress data rate limit (GT_ERATE_TYPE union type).
*					union type is used to support multiple devices with the
*					different formats of egress rate.
*					GT_ERATE_TYPE has the following fields:
*						definedRate - GT_EGRESS_RATE enum type should used for the
*							following devices:
*							88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183,
*							88E6093, 88E6095, 88E6185, 88E6108, 88E6065, 88E6061,
*							and their variations
*						kbRate - rate in kbps that should used for the following
*							devices:
*							88E6097, 88E6096 with the GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_LAYER1,
*								GT_PIRL_ELIMIT_LAYER2, or
*								GT_PIRL_ELIMIT_LAYER3 (see grcSetELimitMode)
*							64kbps ~ 1Mbps    : increments of 64kbps,
*							1Mbps ~ 100Mbps   : increments of 1Mbps, and
*							100Mbps ~ 1000Mbps: increments of 10Mbps
*							Therefore, the valid values are:
*								64, 128, 192, 256, 320, 384,..., 960,
*								1000, 2000, 3000, 4000, ..., 100000,
*								110000, 120000, 130000, ..., 1000000.
*						fRate - frame per second that should used for the following
*							devices:
*							88E6097, 88E6096 with GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_FRAME
*							Valid values are between 7600 and 1488000
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_EGRESS_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
*******************************************************************************/
GT_STATUS grcGetEgressRate
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT port,
    OUT GT_ERATE_TYPE  *rateType
);


/*******************************************************************************
* gpavSetPAV
*
* DESCRIPTION:
*       This routine sets the Port Association Vector
*
* INPUTS:
*       port	- logical port number.
*       pav 	- Port Association Vector
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS gpavSetPAV
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT	port,
    IN GT_U16	pav
);

/*******************************************************************************
* gpavGetPAV
*
* DESCRIPTION:
*       This routine gets the Port Association Vector
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       pav 	- Port Association Vector
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS gpavGetPAV
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT port,
    OUT GT_U16    *pav
);

/*******************************************************************************
* gpavSetIngressMonitor
*
* DESCRIPTION:
*       This routine sets the Ingress Monitor bit in the PAV.
*
* INPUTS:
*       port - the logical port number.
*       mode - the ingress monitor bit in the PAV
*              GT_FALSE: Ingress Monitor enabled
*              GT_TRUE:  Ingress Monitor disabled
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*******************************************************************************/
GT_STATUS gpavSetIngressMonitor
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* gpavGetIngressMonitor
*
* DESCRIPTION:
*       This routine gets the Ingress Monitor bit in the PAV.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the ingress monitor bit in the PAV
*              GT_FALSE: Ingress Monitor enabled
*              GT_TRUE:  Ingress Monitor disabled
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS gpavGetIngressMonitor
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* gvctGetCableStatus
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the requested port,
*       and returns the the status per MDI pair.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
*******************************************************************************/
GT_STATUS gvctGetCableDiag
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT        port,
    OUT GT_CABLE_STATUS *cableStatus
);


/*******************************************************************************
* gvctGet1000BTExtendedStatus
*
* DESCRIPTION:
*       This routine retrieves extended cable status, such as Pair Poloarity,
*		Pair Swap, and Pair Skew. Note that this routine will be success only
*		if 1000Base-T Link is up.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       extendedStatus - the extended cable status.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
*******************************************************************************/
GT_STATUS gvctGet1000BTExtendedStatus
(
    IN  GT_QD_DEV 		*dev,
    IN  GT_LPORT        port,
    OUT GT_1000BT_EXTENDED_STATUS *extendedStatus
);


/*******************************************************************************
* gtMemSet
*
* DESCRIPTION:
*       Set a block of memory
*
* INPUTS:
*       start  - start address of memory block for setting
*       simbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * gtMemSet
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/*******************************************************************************
* gtMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * gtMemCpy
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
);


/*******************************************************************************
* gtMemCmp
*
* DESCRIPTION:
*       Compares given memories.
*
* INPUTS:
*       src1 - source 1
*       src2 - source 2
*       size - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0, if equal.
*		negative number, if src1 < src2.
*		positive number, if src1 > src2.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int gtMemCmp
(
    IN char src1[],
    IN char src2[],
    IN GT_U32 size
);

/*******************************************************************************
* gtStrlen
*
* DESCRIPTION:
*       Determine the length of a string
* INPUTS:
*       source  - string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       size    - number of characters in string, not including EOS.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 gtStrlen
(
    IN const void * source
);

/*******************************************************************************
* gtDelay
*
* DESCRIPTION:
*       Wait for the given uSec and return.
*		Current Switch devices with Gigabit Ethernet Support require 250 uSec
*		of delay time for PPU to be disabled.
*		Since this function is System and/or OS dependent, it should be provided
*		by each DSDT user.
*
* INPUTS:
*       delayTime - delay in uSec.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
void gtDelay
(
    IN const unsigned int delayTime
);


/*******************************************************************************
* gtVersion
*
* DESCRIPTION:
*       This function returns the version of the QuarterDeck SW suite.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       version     - QuarterDeck software version.
*
* RETURNS:
*       GT_OK on success,
*       GT_BAD_PARAM on bad parameters,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gtVersion
(
    OUT GT_VERSION   *version
);


/* Prototypes added for Gigabit Ethernet Switch Support */


/* gtBrgFdb.c */

/*******************************************************************************
* gfdbMove
*
* DESCRIPTION:
*		This routine moves all or unblocked addresses from a port to another.
*
* INPUTS:
*		moveCmd  - the move operation type.
*		moveFrom - port where moving from
*		moveTo   - port where moving to
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK           - on success
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbMove
(
	IN GT_QD_DEV 	*dev,
	IN GT_MOVE_CMD	moveCmd,
	IN GT_U32		moveFrom,
	IN GT_U32		moveTo
);

/*******************************************************************************
* gfdbMoveInDB
*
* DESCRIPTION:
* 		This routine move all or unblocked addresses which are in the particular
* 		ATU Database (DBNum) from a port to another.
*
* INPUTS:
* 		moveCmd  - the move operation type.
*		DBNum	 	- ATU MAC Address Database Number.
*		moveFrom - port where moving from
*		moveTo   - port where moving to
*
* OUTPUTS:
*     None
*
* RETURNS:
* 		GT_OK           - on success
* 		GT_FAIL         - on error
* 		GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbMoveInDB
(
	IN GT_QD_DEV   *dev,
	IN GT_MOVE_CMD moveCmd,
	IN GT_U32 		DBNum,
	IN GT_U32		moveFrom,
	IN GT_U32		moveTo
);

/* gtBrgStp.c */

/* gtBrgVlan.c */

/* gtBrgVtu.c */

/* gtEvents.c */

/*******************************************************************************
* gatuGetIntStatus
*
* DESCRIPTION:
*		Check to see if a specific type of ATU interrupt occured
*
* INPUTS:
*     intType - the type of interrupt which causes an interrupt.
*					GT_MEMEBER_VIOLATION, GT_MISS_VIOLATION, or GT_FULL_VIOLATION
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gatuGetIntStatus
(
	IN  GT_QD_DEV				*dev,
	OUT GT_ATU_INT_STATUS	*atuIntStatus
);


/* gtPhyCtrl.c */

/*******************************************************************************
* gprtSet1000TMasterMode
*
* DESCRIPTION:
*		This routine set the port multicast rate limit.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_1000T_MASTER_SLAVE structure
*				autoConfig   - GT_TRUE for auto, GT_FALSE for manual setup.
*				masterPrefer - GT_TRUE if Master configuration is preferred.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSet1000TMasterMode
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_1000T_MASTER_SLAVE	*mode
);

/*******************************************************************************
* gprtGet1000TMasterMode
*
* DESCRIPTION:
*		This routine set the port multicast rate limit.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_1000T_MASTER_SLAVE structure
*				autoConfig   - GT_TRUE for auto, GT_FALSE for manual setup.
*				masterPrefer - GT_TRUE if Master configuration is preferred.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGet1000TMasterMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_1000T_MASTER_SLAVE	*mode
);

/*******************************************************************************
* gprtGetPhyReg
*
* DESCRIPTION:
*		This routine reads Phy Registers.
*
* INPUTS:
*		port    - logical port number
*		regAddr - The register's address.
*
* OUTPUTS:
*		data    - The read register's data.
*
* RETURNS:
*		GT_OK           - on success
*		GT_FAIL         - on error
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyReg
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_U32	 	regAddr,
	OUT GT_U16	 	*data
);

/*******************************************************************************
* gprtSetPhyReg
*
* DESCRIPTION:
*		This routine writes Phy Registers.
*
* INPUTS:
*		port    - logical port number
*		regAddr - The register's address.
*
* OUTPUTS:
*		data    - The read register's data.
*
* RETURNS:
*		GT_OK           - on success
*		GT_FAIL         - on error
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPhyReg
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_U32	 	regAddr,
	IN  GT_U16	 	data
);


/* gtPortCtrl.c */

/*******************************************************************************
* gprtSetDropOnLock
*
* DESCRIPTION:
*		This routine set the Drop on Lock. When set to one, Ingress frames will
*		be discarded if their SA field is not in the ATU's address database.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for Unknown SA drop or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDropOnLock
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetDropOnLock
*
* DESCRIPTION:
*		This routine gets DropOnLock mode.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: DropOnLock enabled,
*				 GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDropOnLock
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtSetDoubleTag
*
* DESCRIPTION:
*		This routine set the Ingress Double Tag Mode. When set to one,
*		ingressing frames are examined to see if they contain an 802.3ac tag.
*		If they do, the tag is removed and then the frame is processed from
*		there (i.e., removed tag is ignored). Essentially, untagged frames
*		remain untagged, single tagged frames become untagged and double tagged
*		frames become single tagged.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for DoulbeTag mode or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDoubleTag
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetDoubleTag
*
* DESCRIPTION:
*		This routine gets DoubleTag mode.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: DoubleTag enabled,
*				 GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDoubleTag
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtSetInterswitchPort
*
* DESCRIPTION:
*		This routine set Interswitch Port. When set to one,
*		it indicates this port is a interswitch port used to communicated with
*		CPU or to cascade with another switch device.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for Interswitch port or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetInterswitchPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetInterswithPort
*
* DESCRIPTION:
*		This routine gets InterswitchPort.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: This port is interswitch port,
*				 GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetInterswitchPort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtSetLearnDisable
*
* DESCRIPTION:
*		This routine enables/disables automatic learning of new source MAC
*		addresses on the given port ingress
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for disable or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetLearnDisable
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);


/*******************************************************************************
* gprtGetLearnDisable
*
* DESCRIPTION:
*		This routine gets LearnDisable setup
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: Learning disabled on the given port ingress frames,
*				 GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetLearnDisable
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtSetIgnoreFCS
*
* DESCRIPTION:
*		This routine sets FCS Ignore mode. When this bit is set to a one,
*		the last four bytes of frames received on this port are overwritten with
*		a good CRC and the frames will be accepted by the switch.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for ignore FCS or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIgnoreFCS
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL 		mode
);

/*******************************************************************************
* gprtGetIgnoreFCS
*
* DESCRIPTION:
*		This routine gets Ignore FCS setup
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: Ignore FCS on the given port's ingress frames,
*				 GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIgnoreFCS
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gprtSetVTUPriOverride
*
* DESCRIPTION:
*		VTU Priority Override. The following modes are supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PRI_OVERRIDE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtSetVTUPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_PRI_OVERRIDE		mode
);

/*******************************************************************************
* gprtGetVTUPriOverride
*
* DESCRIPTION:
*		VTU Priority Override. The following modes are supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_PRI_OVERRIDE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtGetVTUPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_PRI_OVERRIDE		*mode
);

/*******************************************************************************
* gprtSetSAPriOverride
*
* DESCRIPTION:
*		SA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PRI_OVERRIDE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtSetSAPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetSAPriOverride
*
* DESCRIPTION:
*		SA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_PRI_OVERRIDE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtGetSAPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PRI_OVERRIDE		*mode
);

/*******************************************************************************
* gprtSetDAPriOverride
*
* DESCRIPTION:
*		DA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PRI_OVERRIDE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtSetDAPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_PRI_OVERRIDE		mode
);

/*******************************************************************************
* gprtGetDAPriOverride
*
* DESCRIPTION:
*		DA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_PRI_OVERRIDE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtGetDAPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PRI_OVERRIDE		*mode
);

/*******************************************************************************
* gprtSetCPUPort
*
* DESCRIPTION:
*		This routine sets CPU Port number. When Snooping is enabled on this port
*		or when this port is configured as an Interswitch Port and it receives a
*		To_CPU frame, the switch needs to know what port on this device the frame
*		should egress.
*
* INPUTS:
*		port - the logical port number.
*		cpuPort - CPU Port number or interswitch port where CPU Port is connected
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetCPUPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_LPORT 	cpuPort
);

/*******************************************************************************
* gprtGetCPUPort
*
* DESCRIPTION:
*		This routine gets CPU Logical Port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		cpuPort - CPU Port's logical number
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetCPUPort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_LPORT 	*cpuLPort
);

/*******************************************************************************
* gprtSetLockedPort
*
* DESCRIPTION:
*		This routine sets LockedPort. When it's set to one, CPU directed
*		learning for 802.1x MAC authentication is enabled on this port. In this
*		mode, an ATU Miss Violation interrupt will occur when a new SA address
*		is received in a frame on this port. Automatically SA learning and
*		refreshing is disabled in this mode.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for Locked Port, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetLockedPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetLockedPort
*
* DESCRIPTION:
*		This routine gets Locked Port mode for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if LockedPort, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetLockedPort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
);

/*******************************************************************************
* gprtSetIgnoreWrongData
*
* DESCRIPTION:
*		This routine sets Ignore Wrong Data. If the frame's SA address is found
*		in the database and if the entry is 'static' or if the port is 'locked'
*		the source port's bit is checked to insure the SA has been assigned to
*		this port. If the SA is NOT assigned to this port, it is considered an
*		ATU Member Violation. If the IgnoreWrongData is set to GT_FALSE, an ATU
*		Member Violation interrupt will be generated. If it's set to GT_TRUE,
*		the ATU Member Violation error will be masked and ignored.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for IgnoreWrongData, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIgnoreWrongData
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);


/*******************************************************************************
* gprtGetIgnoreWrongData
*
* DESCRIPTION:
*		This routine gets Ignore Wrong Data mode for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if IgnoreWrongData, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIgnoreWrongData
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);


/* gtPortRateCtrl.c */

/* gtPortRmon.c */

/*******************************************************************************
* gstatsGetPortCounter2
*
* DESCRIPTION:
*		This routine gets a specific counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch only
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter2
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS2	counter,
	OUT GT_U32			*statsData
);


/*******************************************************************************
* gstatsGetPortAllCounters2
*
* DESCRIPTION:
*		This routine gets all counters of the given port
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch only
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters2
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	OUT GT_STATS_COUNTER_SET2	*statsCounterSet
);

/*******************************************************************************
* gstatsGetHistogramMode
*
* DESCRIPTION:
*		This routine gets the Histogram Counters Mode.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - Histogram Mode (GT_COUNT_RX_ONLY, GT_COUNT_TX_ONLY,
*					and GT_COUNT_RX_TX)
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		This function supports Gigabit Switch only
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetHistogramMode
(
	IN  GT_QD_DEV				*dev,
	OUT GT_HISTOGRAM_MODE	*mode
);

/*******************************************************************************
* gstatsSetHistogramMode
*
* DESCRIPTION:
*		This routine sets the Histogram Counters Mode.
*
* INPUTS:
*		mode - Histogram Mode (GT_COUNT_RX_ONLY, GT_COUNT_TX_ONLY,
*					and GT_COUNT_RX_TX)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsSetHistogramMode
(
	IN GT_QD_DEV 				*dev,
	IN GT_HISTOGRAM_MODE		mode
);


/* gtPortStatus.c */

/*******************************************************************************
* gprtGetPauseEn
*
* DESCRIPTION:
*		This routine retrives the link pause state.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for enable or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		If set MAC Pause (for Full Duplex flow control) is implemented in the
*		link partner and in MyPause
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPauseEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*state
);

/*******************************************************************************
* gprtGetHdFlow
*
* DESCRIPTION:
*		This routine retrives the half duplex flow control value.
*		If set, Half Duplex back pressure will be used on this port if this port
*		is in a half duplex mode.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for enable or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHdFlow
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL 	*state
);

/*******************************************************************************
* gprtGetPHYDetect
*
* DESCRIPTION:
*		This routine retrives the information regarding PHY detection.
*		If set, An 802.3 PHY is attached to this port.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if connected or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPHYDetect
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL 	*state
);

/*******************************************************************************
* gprtSetPHYDetect
*
* DESCRIPTION:
*		This routine sets PHYDetect bit which make PPU change its polling.
*		PPU's pool routine uses these bits to determine which port's to poll
*		PHYs on for Link, Duplex, Speed, and Flow Control.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE or GT_FALSE
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		This function should not be called if gsysGetPPUState returns
*		PPU_STATE_ACTIVE.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPHYDetect
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_BOOL  	state
);

/*******************************************************************************
* gprtGetSpeedMode
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_PORT_SPEED_MODE type.
*					(PORT_SPEED_1000_MBPS,PORT_SPEED_100_MBPS, or PORT_SPEED_10_MBPS)
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSpeedMode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_PORT_SPEED_MODE   *speed
);

/*******************************************************************************
* gprtGetHighErrorRate
*
* DESCRIPTION:
*		This routine retrives the PCS High Error Rate.
*		This routine returns GT_TRUE if the rate of invalid code groups seen by
*		PCS has exceeded 10 to the power of -11.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE or GT_FALSE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHighErrorRate
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtGetTxPaused
*
* DESCRIPTION:
*		This routine retrives Transmit Pause state.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Rx MAC receives a PAUSE frame with none-zero Puase Time
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetTxPaused
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);


/*******************************************************************************
* gprtGetFlowCtrl
*
* DESCRIPTION:
*		This routine retrives Flow control state.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Rx MAC determines that no more data should be
*					entering this port.
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetFlowCtrl
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtGetC_Duplex
*
* DESCRIPTION:
*		This routine retrives Port 9's duplex configuration mode determined
*		at reset.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if configured as Full duplex operation
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Return value is valid only if the given port is 9.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetC_Duplex
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtGetC_Mode
*
* DESCRIPTION:
*		This routine retrives port's interface type configuration mode
*		determined at reset.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - one of value in GT_PORT_CONFIG_MODE enum type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Return value is valid only if the given port is 9.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetC_Mode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PORT_CONFIG_MODE   *state
);


/* gtSysCtrl.c */

/*******************************************************************************
* gsysSetPPUEn
*
* DESCRIPTION:
*		This routine enables/disables Phy Polling Unit.
*
* INPUTS:
*		en - GT_TRUE to enable PPU, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetPPUEn
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL 		en
);

/*******************************************************************************
* gsysGetPPUEn
*
* DESCRIPTION:
*		This routine get the PPU state.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if PPU is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPPUEn
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetCascadePort
*
* DESCRIPTION:
*		This routine sets Cascade Port number.
*		In multichip systems frames coming from a CPU need to know when they
*		have reached their destination chip.
*
* INPUTS:
*		port - Cascade Port
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetCascadePort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port
);

/*******************************************************************************
* gsysGetCascadePort
*
* DESCRIPTION:
*		This routine gets Cascade Port number.
*		In multichip systems frames coming from a CPU need to know when they
*		have reached their destination chip.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port - Cascade Port
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetCascadePort
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT 	*port
);

/*******************************************************************************
* gsysSetDeviceNumber
*
* DESCRIPTION:
*		This routine sets Device Number.
*		In multichip systems frames coming from a CPU need to know when they
*		have reached their destination chip. From CPU frames whose Dev_Num
*		fieldmatches these bits have reachedtheir destination chip and are sent
*		out this chip using the port number indicated in the frame's Trg_Port
*		field.
*
* INPUTS:
*		devNum - Device Number (0 ~ 31)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDeviceNumber
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		devNum
);

/*******************************************************************************
* gsysGetDeviceNumber
*
* DESCRIPTION:
*		This routine gets Device Number.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		devNum - Device Number (0 ~ 31)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDeviceNumber
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U32  	*devNum
);


/* gtPCSCtrl.c */


/*******************************************************************************
* gpcsGetCommaDet
*
* DESCRIPTION:
*		This routine retrieves Comma Detection status in PCS
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for Comma Detected or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetCommaDet
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsGetSyncOK
*
* DESCRIPTION:
*		This routine retrieves SynOK bit. It is set to a one when the PCS has
*		detected a few comma patterns and is synchronized with its peer PCS
*		layer.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if synchronized or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetSyncOK
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsGetSyncFail
*
* DESCRIPTION:
*		This routine retrieves SynFail bit.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if synchronizaion failed or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetSyncFail
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsGetAnBypassed
*
* DESCRIPTION:
*		This routine retrieves Inband Auto-Negotiation bypass status.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if AN is bypassed or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetAnBypassed
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsGetAnBypassMode
*
* DESCRIPTION:
*		This routine retrieves Enable mode of Inband Auto-Negotiation bypass.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE if AN bypass is enabled or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetAnBypassMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*mode
);

/*******************************************************************************
* gpcsSetAnBypassMode
*
* DESCRIPTION:
*		This routine retrieves Enable mode of Inband Auto-Negotiation bypass.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable AN bypass mode or GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetAnBypassMode
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL  	mode
);

/*******************************************************************************
* gpcsGetPCSAnEn
*
* DESCRIPTION:
*		This routine retrieves Enable mode of PCS Inband Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE if PCS AN is enabled or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetPCSAnEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*mode
);

/*******************************************************************************
* gpcsSetPCSAnEn
*
* DESCRIPTION:
*		This routine sets Enable mode of PCS Inband Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable PCS AN mode or GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetPCSAnEn
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL  	mode
);

/*******************************************************************************
* gpcsSetRestartPCSAn
*
* DESCRIPTION:
*		This routine restarts PCS Inband Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetRestartPCSAn
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port
);

/*******************************************************************************
* gpcsGetPCSAnDone
*
* DESCRIPTION:
*		This routine retrieves completion information of PCS Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE if PCS AN is done or never done
*			    GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetPCSAnDone
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*mode
);

/*******************************************************************************
* gpcsSetLinkValue
*
* DESCRIPTION:
*		This routine sets Link's force value
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force link up, GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetLinkValue
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
);

/*******************************************************************************
* gpcsGetLinkValue
*
* DESCRIPTION:
*		This routine retrieves Link Value which will be used for Forcing Link
*		up or down.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Link Force value is one (link up)
*			     GT_FALSE otherwise (link down)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetLinkValue
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsSetForcedLink
*
* DESCRIPTION:
*		This routine forces Link. If LinkValue is set to one, calling this
*		routine with GT_TRUE will force Link to be up.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force link (up or down), GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetForcedLink
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
);

/*******************************************************************************
* gpcsGetForcedLink
*
* DESCRIPTION:
*		This routine retrieves Forced Link bit
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if ForcedLink bit is one,
*			     GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetForcedLink
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsSetDpxValue
*
* DESCRIPTION:
*		This routine sets Duplex's Forced value. This function needs to be
*		called prior to gpcsSetForcedDpx.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force full duplex, GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetDpxValue
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
);

/*******************************************************************************
* gpcsGetDpxValue
*
* DESCRIPTION:
*		This routine retrieves Duplex's Forced value
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Duplex's Forced value is set to Full duplex,
*			     GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetDpxValue
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsSetForcedDpx
*
* DESCRIPTION:
*		This routine forces duplex mode. If DpxValue is set to one, calling this
*		routine with GT_TRUE will force duplex mode to be full duplex.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force duplex mode, GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetForcedDpx
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
);

/*******************************************************************************
* gpcsGetForcedDpx
*
* DESCRIPTION:
*		This routine retrieves Forced Duplex.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if ForcedDpx bit is one,
*			     GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetForcedDpx
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsSetForceSpeed
*
* DESCRIPTION:
*		This routine forces speed.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PORT_FORCED_SPEED_MODE (10, 100, 1000, or No Speed Force)
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetForceSpeed
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_PORT_FORCED_SPEED_MODE  mode
);

/*******************************************************************************
* gpcsGetForceSpeed
*
* DESCRIPTION:
*		This routine retrieves Force Speed value
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_PORT_FORCED_SPEED_MODE (10, 100, 1000, or no force speed)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetForceSpeed
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PORT_FORCED_SPEED_MODE   *mode
);



/* gtQosMap.c */

/*******************************************************************************
* gqosGetTagRemap
*
* DESCRIPTION:
*		Gets the remapped priority value for a specific 802.1p priority on a
*		given port.
*
* INPUTS:
*		port  - the logical port number.
*		pri   - 802.1p priority
*
* OUTPUTS:
*		remappedPri - remapped Priority
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetTagRemap
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_U8    	pri,
	OUT GT_U8   	*remappedPri
);

/*******************************************************************************
* gqosSetTagRemap
*
* DESCRIPTION:
*		Sets the remapped priority value for a specific 802.1p priority on a
*		given port.
*
* INPUTS:
*		port  - the logical port number.
*		pri   - 802.1p priority
*		remappedPri - remapped Priority
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetTagRemap
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_U8    	pri,
	IN GT_U8    	remappedPri
);


/* gtSysConfig.c */

/* gtSysStatus.c */

/*******************************************************************************
* gsysGetPPUState
*
* DESCRIPTION:
*		This routine get the PPU State. These two bits return
*		the current value of the PPU.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - GT_PPU_STATE
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPPUState
(
	IN  GT_QD_DEV   	*dev,
	OUT GT_PPU_STATE	*mode
);


/* Prototypes added for 88E6093 */


/* gtBrgFdb.c */

/*******************************************************************************
* gfdbGetLearn2All
*
* DESCRIPTION:
*		When more than one Marvell device is used to form a single 'switch', it
*		may be desirable for all devices in the 'switch' to learn any address this
*		device learns. When this bit is set to a one all other devices in the
*		'switch' learn the same addresses this device learns. When this bit is
*		cleared to a zero, only the devices that actually receive frames will learn
*		from those frames. This mode typically supports more active MAC addresses
*		at one time as each device in the switch does not need to learn addresses
*		it may nerver use.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode  - GT_TRUE if Learn2All is enabled, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK           - on success
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetLearn2All
(
	IN  GT_QD_DEV    *dev,
	OUT GT_BOOL 	*mode
);

/*******************************************************************************
* gfdbSetLearn2All
*
* DESCRIPTION:
*		Enable or disable Learn2All mode.
*
* INPUTS:
*		mode - GT_TRUE to set Learn2All, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbSetLearn2All
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gfdbRemovePort
*
* DESCRIPTION:
*       This routine deassociages all or unblocked addresses from a port.
*
* INPUTS:
*       moveCmd - the move operation type.
*       port - the logical port number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbRemovePort
(
	IN GT_QD_DEV    *dev,
    IN GT_MOVE_CMD 	moveCmd,
    IN GT_LPORT		port
);

/*******************************************************************************
* gfdbRemovePortInDB
*
* DESCRIPTION:
*       This routine deassociages all or unblocked addresses from a port in the
*       particular ATU Database (DBNum).
*
* INPUTS:
*       moveCmd  - the move operation type.
*       port - the logical port number.
*		DBNum	 - ATU MAC Address Database Number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbRemovePortInDB
(
	IN GT_QD_DEV    *dev,
    IN GT_MOVE_CMD 	moveCmd,
    IN GT_LPORT		port,
	IN GT_U32 		DBNum
);



/* gtBrgStp.c */

/* gtBrgVlan.c */

/* gtBrgVtu.c */

/* gtEvents.c */

/* gtPCSCtrl.c */

/*******************************************************************************
* gpcsGetPCSLink
*
* DESCRIPTION:
*		This routine retrieves Link up status in PCS
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for Comma Detected or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetPCSLink
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsSetFCValue
*
* DESCRIPTION:
*		This routine sets Flow Control's force value
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force flow control enabled, GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetFCValue
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
);

/*******************************************************************************
* gpcsGetFCValue
*
* DESCRIPTION:
*		This routine retrieves Flow Control Value which will be used for Forcing
*		Flow Control enabled or disabled.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if FC Force value is one (flow control enabled)
*			     GT_FALSE otherwise (flow control disabled)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetFCValue
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gpcsSetForcedFC
*
* DESCRIPTION:
*		This routine forces Flow Control. If FCValue is set to one, calling this
*		routine with GT_TRUE will force Flow Control to be enabled.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force flow control (enable or disable), GT_FALSE otherwise
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsSetForcedFC
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
);

/*******************************************************************************
* gpcsGetForcedFC
*
* DESCRIPTION:
*		This routine retrieves Forced Flow Control bit
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if ForcedFC bit is one,
*			     GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpcsGetForcedFC
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);



/* gtPhyCtrl.c */

/*******************************************************************************
* gprtGetPagedPhyReg
*
* DESCRIPTION:
*       This routine reads phy register of the given page
*
* INPUTS:
*		port 	- port to be read
*		regAddr	- register offset to be read
*		page	- page number to be read
*
* OUTPUTS:
*		data	- value of the read register
*
* RETURNS:
*       GT_OK   			- if read successed
*       GT_FAIL   			- if read failed
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gprtGetPagedPhyReg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U32  port,
	IN	GT_U32  regAddr,
	IN	GT_U32  page,
    OUT GT_U16* data
);

/*******************************************************************************
* gprtSetPagedPhyReg
*
* DESCRIPTION:
*       This routine writes a value to phy register of the given page
*
* INPUTS:
*		port 	- port to be read
*		regAddr	- register offset to be read
*		page	- page number to be read
*		data	- value of the read register
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK   			- if read successed
*       GT_FAIL   			- if read failed
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gprtSetPagedPhyReg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U32 port,
	IN	GT_U32 regAddr,
	IN	GT_U32 page,
    IN  GT_U16 data
);


/* gtPortCtrl.c */

/*******************************************************************************
* gprtSetUseCoreTag
*
* DESCRIPTION:
*       This routine set the UseCoreTag bit in Port Control Register.
*			When this bit is cleared to a zero, ingressing frames are considered
*			Tagged if the 16-bits following the frame's Source Address is 0x8100.
*			When this bit is set to a one, ingressing frames are considered Tagged
*			if the 16-bits following the frame's Source Address is equal to the
*			CoreTag register value.
*
* INPUTS:
*       port  - the logical port number.
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetUseCoreTag
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    force
);

/*******************************************************************************
* gprtGetUseCoreTag
*
* DESCRIPTION:
*       This routine get the Use Core Tag state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       force - GT_TRUE for using core tag register  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetUseCoreTag
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *force
);

/*******************************************************************************
* gprtSetDiscardTagged
*
* DESCRIPTION:
*		When this bit is set to a one, all non-MGMT frames that are processed as
*		Tagged will be discarded as they enter this switch port. Priority only
*		tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to discard tagged frame, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDiscardTagged
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetDiscardTagged
*
* DESCRIPTION:
*		This routine gets DiscardTagged bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if DiscardTagged bit is set, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDiscardTagged
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);

/*******************************************************************************
* gprtSetDiscardUntagged
*
* DESCRIPTION:
*		When this bit is set to a one, all non-MGMT frames that are processed as
*		Untagged will be discarded as they enter this switch port. Priority only
*		tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to discard untagged frame, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDiscardUntagged
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetDiscardUntagged
*
* DESCRIPTION:
*		This routine gets DiscardUntagged bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if DiscardUntagged bit is set, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDiscardUntagged
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);

/*******************************************************************************
* gprtSetMapDA
*
* DESCRIPTION:
*		When this bit is set to a one, normal switch operation will occur where a
*		frame's DA address is used to direct the frame out the correct port.
*		When this be is cleared to a zero, the frame will be sent out the port(s)
*		defined by ForwardUnknown bits or the DefaultForward bits even if the DA
*		is ound in the address database.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to use MapDA, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetMapDA
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetMapDA
*
* DESCRIPTION:
*		This routine gets MapDA bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if MapDA bit is set, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetMapDA
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);

/*******************************************************************************
* gprtSetDefaultForward
*
* DESCRIPTION:
*		When this bit is set to a one, normal switch operation will occurs and
*		multicast frames with unknown DA addresses are allowed to egress out this
*		port (assuming the VLAN settings allow the frame to egress this port too).
*		When this be is cleared to a zero, multicast frames with unknown DA
*		addresses will not egress out this port.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to use DefaultForward, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDefaultForward
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetDefaultForward
*
* DESCRIPTION:
*		This routine gets DefaultForward bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if DefaultForward bit is set, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDefaultForward
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);

/*******************************************************************************
* gprtSetEgressMonitorSource
*
* DESCRIPTION:
*		When this be is cleared to a zero, normal network switching occurs.
*		When this bit is set to a one, any frame that egresses out this port will
*		also be sent to the EgressMonitorDest Port
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to set EgressMonitorSource, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetEgressMonitorSource
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetEgressMonitorSource
*
* DESCRIPTION:
*		This routine gets EgressMonitorSource bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if EgressMonitorSource bit is set, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetEgressMonitorSource
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);

/*******************************************************************************
* gprtSetIngressMonitorSource
*
* DESCRIPTION:
*		When this be is cleared to a zero, normal network switching occurs.
*		When this bit is set to a one, any frame that egresses out this port will
*		also be sent to the EgressMonitorDest Port
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to set EgressMonitorSource, GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIngressMonitorSource
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetIngressMonitorSource
*
* DESCRIPTION:
*		This routine gets IngressMonitorSource bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if IngressMonitorSource bit is set, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIngressMonitorSource
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);


/* gtPortPAV.c */

/* gtPortRateCtrl.c */

/* gtPortRmon.c */

/*******************************************************************************
* gstatsGetPortCounter3
*
* DESCRIPTION:
*		This routine gets a specific counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch only
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter3
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS3	counter,
	OUT GT_U32			*statsData
);

/*******************************************************************************
* gstatsGetPortAllCounters3
*
* DESCRIPTION:
*		This routine gets all counters of the given port
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch only
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters3
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	OUT GT_STATS_COUNTER_SET3	*statsCounterSet
);


/* gtPortStat.c */

/*******************************************************************************
* gprtGetPortCtr2
*
* DESCRIPTION:
*       This routine gets the port InDiscards, InFiltered, and OutFiltered counters.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       ctr - the counters value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortCtr2
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_PORT_STAT2   *ctr
);

/* gtPortStatus.c */

/*******************************************************************************
* gprtGetMGMII
*
* DESCRIPTION:
*		SERDES Interface mode. When this bit is cleared to a zero and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be SGMII.  When this bit is set toa one and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be MGMII. When no PHY is detected on this port and the
*		SERDES interface is being used, it will be configured in 1000Base-X mode.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE or GT_FALSE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetMGMII
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtSetMGMII
*
* DESCRIPTION:
*		SERDES Interface mode. When this bit is cleared to a zero and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be SGMII.  When this bit is set toa one and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be MGMII. When no PHY is detected on this port and the
*		SERDES interface is being used, it will be configured in 1000Base-X mode.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE or GT_FALSE
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetMGMII
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_BOOL  	state
);


/* gtQosMap.c */

/* gtSysCtrl.c */

/*******************************************************************************
* gsysSetCoreTagType
*
* DESCRIPTION:
*		This routine sets Ether Core Tag Type.
*		This Ether Type is added to frames that egress the switch as Double Tagged
*		frames. It is also the Ether Type expected during Ingress to determine if
*		a frame is Tagged or not on ports configured as UseCoreTag mode.
*
* INPUTS:
*		etherType - Core Tag Type (2 bytes)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetCoreTagType
(
	IN GT_QD_DEV	*dev,
	IN GT_U16  		etherType
);

/*******************************************************************************
* gsysGetCoreTagType
*
* DESCRIPTION:
*		This routine gets CoreTagType
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		etherType - Core Tag Type (2 bytes)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetCoreTagType
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*etherType
);

/*******************************************************************************
* gsysSetIngressMonitorDest
*
* DESCRIPTION:
*		This routine sets Ingress Monitor Destination Port. Frames that are
*		targeted toward an Ingress Monitor Destination go out the port number
*		indicated in these bits. This includes frames received on a Marvell Tag port
*		with the Ingress Monitor type, and frames received on a Network port that
*		is enabled to be the Ingress Monitor Source Port.
*		If the Ingress Monitor Destination Port resides in this device these bits
*		should point to the Network port where these frames are to egress. If the
*		Ingress Monitor Destination Port resides in another device these bits
*		should point to the Marvell Tag port in this device that is used to get
*		to the device that contains the Ingress Monitor Destination Port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetIngressMonitorDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
);

/*******************************************************************************
* gsysGetIngressMonitorDest
*
* DESCRIPTION:
*		This routine gets Ingress Monitor Destination Port.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetIngressMonitorDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
);

/*******************************************************************************
* gsysSetEgressMonitorDest
*
* DESCRIPTION:
*		This routine sets Egress Monitor Destination Port. Frames that are
*		targeted toward an Egress Monitor Destination go out the port number
*		indicated in these bits. This includes frames received on a Marvell Tag port
*		with the Egress Monitor type, and frames transmitted on a Network port that
*		is enabled to be the Egress Monitor Source Port.
*		If the Egress Monitor Destination Port resides in this device these bits
*		should point to the Network port where these frames are to egress. If the
*		Egress Monitor Destination Port resides in another device these bits
*		should point to the Marvell Tag port in this device that is used to get
*		to the device that contains the Egress Monitor Destination Port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetEgressMonitorDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
);

/*******************************************************************************
* gsysGetEgressMonitorDest
*
* DESCRIPTION:
*		This routine gets Egress Monitor Destination Port.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetEgressMonitorDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
);


/* gtSysConfig.c */

/* gtSysStatus.c */


/* functions added on rev 2.2 */

/* gtPortCtrl.c */

/*******************************************************************************
* gprtSetMessagePort
*
* DESCRIPTION:
*		When the Learn2All bit is set to one, learning message frames are
*		generated. These frames will be sent out all ports whose Message Port is
*		set to one.
* 		If this feature is used, it is recommended that all Marvell Tag ports,
*		except for the CPU's port, have their MessagePort bit set to one.
*		Ports that are not Marvell Tag ports should not have their Message Port
*		bit set to one.
*
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to make this port a Message Port. GT_FALSE, otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetMessagePort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetMessagePort
*
* DESCRIPTION:
*		When the Learn2All bit is set to one, learning message frames are
*		generated. These frames will be sent out all ports whose Message Port is
*		set to one.
* 		If this feature is used, it is recommended that all Marvell Tag ports,
*		except for the CPU's port, have their MessagePort bit set to one.
*		Ports that are not Marvell Tag ports should not have their Message Port
*		bit set to one.
*
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to make this port a Message Port. GT_FALSE, otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetMessagePort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
);


/*******************************************************************************
* gprtSetTrunkPort
*
* DESCRIPTION:
*		This function enables/disables and sets the trunk ID.
*
* INPUTS:
*		port - the logical port number.
*		en - GT_TRUE to make the port be a member of a trunk with the given trunkId.
*			 GT_FALSE, otherwise.
*		trunkId - valid ID is 0 ~ 15.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkId is neither valid nor INVALID_TRUNK_ID
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetTrunkPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL 		en,
	IN GT_U32		trunkId
);


/*******************************************************************************
* gprtGetTrunkPort
*
* DESCRIPTION:
*		This function returns trunk state of the port.
*		When trunk is disabled, trunkId field won't have valid value.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		en - GT_TRUE, if the port is a member of a trunk,
*			 GT_FALSE, otherwise.
*		trunkId - 0 ~ 15, valid only if en is GT_TRUE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetTrunkPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	OUT GT_BOOL 	*en,
	OUT GT_U32		*trunkId
);




/*******************************************************************************
* gprtGetGlobal2Reg
*
* DESCRIPTION:
*       This routine reads Switch Global 2 Registers.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetGlobal2Reg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
);

/*******************************************************************************
* gprtSetGlobal2Reg
*
* DESCRIPTION:
*       This routine writes Switch Global2 Registers.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetGlobal2Reg
(
    IN  GT_QD_DEV		*dev,
    IN  GT_U32			regAddr,
    IN  GT_U16			data
);

/* gtSysCtrl.c */
/*******************************************************************************
* gsysSetARPDest
*
* DESCRIPTION:
*		This routine sets ARP Monitor Destination Port. Tagged or untagged
*		frames that ingress Network ports with the Broadcast Destination Address
*		and with an Ethertype of 0x0806 are mirrored to this port. The ARPDest
*		should point to the port that directs these frames to the switch's CPU
*		that will process ARPs. This target port should be a Marvell Tag port so
*		that frames will egress with a To_CPU Marvell Tag with a CPU Code of ARP.
*		To_CPU Marvell Tag frames with a CPU Code off ARP that ingress a Marvell
*		Tag port will be sent to the port number definded in ARPDest.
*
*		If ARPDest =  0xF, ARP Monitoring is disabled and ingressing To_CPU ARP
*		frames will be discarded.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetARPDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
);

/*******************************************************************************
* gsysGetARPDest
*
* DESCRIPTION:
*		This routine gets ARP Monitor Destination Port. Tagged or untagged
*		frames that ingress Network ports with the Broadcast Destination Address
*		and with an Ethertype of 0x0806 are mirrored to this port. The ARPDest
*		should point to the port that directs these frames to the switch's CPU
*		that will process ARPs. This target port should be a Marvell Tag port so
*		that frames will egress with a To_CPU Marvell Tag with a CPU Code of ARP.
*		To_CPU Marvell Tag frames with a CPU Code off ARP that ingress a Marvell
*		Tag port will be sent to the port number definded in ARPDest.
*
*		If ARPDest =  0xF, ARP Monitoring is disabled and ingressing To_CPU ARP
*		frames will be discarded.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetARPDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
);

/*******************************************************************************
* gsysSetRsvd2CpuEnables
*
* DESCRIPTION:
*		Reserved DA Enables. When the function, gsysSetRsvd2Cpu, is called with
*		en = GT_TRUE, the 16 reserved multicast DA addresses, whose bit in this
*		enBits(or register) are also set to a one, are treated as MGMT frames.
*		All the reserved DA's take the form 01:80:C2:00:00:0x. When x = 0x0,
*		bit 0 of this register is tested. When x = 0x2, bit 2 of this field is
*		tested and so on.
*		If the tested bit in this register is cleared to a zero, the frame will
*		be treated as a normal (non-MGMT) frame.
*
* INPUTS:
*		enBits - bit vector of enabled Reserved Multicast.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvd2CpuEnables
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		enBits
);

/*******************************************************************************
* gsysGetRsvd2CpuEnables
*
* DESCRIPTION:
*		Reserved DA Enables. When the function, gsysSetRsvd2Cpu, is called with
*		en = GT_TRUE, the 16 reserved multicast DA addresses, whose bit in this
*		enBits(or register) are also set to a one, are treated as MGMT frames.
*		All the reserved DA's take the form 01:80:C2:00:00:0x. When x = 0x0,
*		bit 0 of this register is tested. When x = 0x2, bit 2 of this field is
*		tested and so on.
*		If the tested bit in this register is cleared to a zero, the frame will
*		be treated as a normal (non-MGMT) frame.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		enBits - bit vector of enabled Reserved Multicast.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvd2CpuEnables
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*enBits
);

/*******************************************************************************
* gsysSetRsvd2Cpu
*
* DESCRIPTION:
*		When the Rsvd2Cpu is set to a one(GT_TRUE), frames with a Destination
*		Address in the range 01:80:C2:00:00:0x, regardless of their VLAN
*		membership, will be considered MGMT frames and sent to the port's CPU
*		Port as long as the associated Rsvd2CpuEnable bit (gsysSetRsvd2CpuEnable
*		function) for the frames's DA is also set to a one.
*
* INPUTS:
*		en - GT_TRUE if Rsvd2Cpu is set. GT_FALSE, otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvd2Cpu
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetRsvd2Cpu
*
* DESCRIPTION:
*		When the Rsvd2Cpu is set to a one(GT_TRUE), frames with a Destination
*		Address in the range 01:80:C2:00:00:0x, regardless of their VLAN
*		membership, will be considered MGMT frames and sent to the port's CPU
*		Port as long as the associated Rsvd2CpuEnable bit (gsysSetRsvd2CpuEnable
*		function) for the frames's DA is also set to a one.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Rsvd2Cpu is set. GT_FALSE, otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvd2Cpu
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetMGMTPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on Rsvd2CPU MGMT frames.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetMGMTPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
);

/*******************************************************************************
* gsysGetMGMTPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on Rsvd2CPU MGMT frames.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMGMTPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
);

/*******************************************************************************
* gsysSetUseDoubleTagData
*
* DESCRIPTION:
*		This bit is used to determine if Double Tag data that is removed from a
*		Double Tag frame is used or ignored when making switching decisions on
*		the frame.
*
* INPUTS:
*		en - GT_TRUE to use removed tag data, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetUseDoubleTagData
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetUseDoubleTagData
*
* DESCRIPTION:
*		This bit is used to determine if Double Tag data that is removed from a
*		Double Tag frame is used or ignored when making switching decisions on
*		the frame.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if removed tag data is used, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetUseDoubleTagData
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetPreventLoops
*
* DESCRIPTION:
*		When a Marvell Tag port receives a Forward Marvell Tag whose Src_Dev
*		field equals this device's Device Number, the following action will be
*		taken depending upon the value of this bit.
*		GT_TRUE (1) - The frame will be discarded.
*		GT_FALSE(0) - The frame will be prevented from going out its original
*						source port as defined by the frame's Src_Port field.
*
* INPUTS:
*		en - GT_TRUE to discard the frame as described above, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetPreventLoops
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetPreventLoops
*
* DESCRIPTION:
*		When a Marvell Tag port receives a Forward Marvell Tag whose Src_Dev
*		field equals this device's Device Number, the following action will be
*		taken depending upon the value of this bit.
*		GT_TRUE (1) - The frame will be discarded.
*		GT_FALSE(0) - The frame will be prevented from going out its original
*						source port as defined by the frame's Src_Port field.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to discard the frame as described above, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPreventLoops
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetFlowControlMessage
*
* DESCRIPTION:
*		When this bit is set to one, Marvell Tag Flow Control messages will be
*		generated when an output queue becomes congested and received Marvell Tag
*		Flow Control messages will pause MACs inside this device. When this bit
*		is cleared to a zero, Marvell Tag Flow Control messages will not be
*		generated and any received will be ignored at the target MAC.
*
* INPUTS:
*		en - GT_TRUE to use Marvell Tag Flow Control message, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetFlowControlMessage
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetFlowControlMessage
*
* DESCRIPTION:
*		When this bit is set to one, Marvell Tag Flow Control messages will be
*		generated when an output queue becomes congested and received Marvell Tag
*		Flow Control messages will pause MACs inside this device. When this bit
*		is cleared to a zero, Marvell Tag Flow Control messages will not be
*		generated and any received will be ignored at the target MAC.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use Marvell Tag Flow Control message, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFlowControlMessage
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetForceFlowControlPri
*
* DESCRIPTION:
*		When this bit is set to a one the PRI[2:0] bits of generated Marvell Tag
*		Flow Control frames will be set to the value of the FC Pri bits (set by
*		gsysSetFCPri function call). When this bit is cleared to a zero, generated
*		Marvell Tag Flow Control frames will retain the PRI[2:0] bits from the
*		frames that caused the congestion. This bit will have no effect if the
*		FlowControlMessage bit(gsysSetFlowControlMessage function call) is
*		cleared to a zero.
*
* INPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetForceFlowControlPri
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetForceFlowControlPri
*
* DESCRIPTION:
*		When this bit is set to a one the PRI[2:0] bits of generated Marvell Tag
*		Flow Control frames will be set to the value of the FC Pri bits (set by
*		gsysSetFCPri function call). When this bit is cleared to a zero, generated
*		Marvell Tag Flow Control frames will retain the PRI[2:0] bits from the
*		frames that caused the congestion. This bit will have no effect if the
*		FlowControlMessage bit(gsysSetFlowControlMessage function call) is
*		cleared to a zero.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetForceFlowControlPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetFCPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on generated Marvell Tag Flow
*		Control frames if the ForceFlowControlPri bit(gsysSetForceFlowControlPri)
*		is set to a one.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetFCPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
);

/*******************************************************************************
* gsysGetFCPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on generated Marvell Tag Flow
*		Control frames if the ForceFlowControlPri bit(gsysSetForceFlowControlPri)
*		is set to a one.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFCPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
);

/*******************************************************************************
* gsysSetFlowCtrlDelay
*
* DESCRIPTION:
*		This function sets Flow control delay time for 10Mbps, 100Mbps, and
*		1000Mbps.
*
* INPUTS:
*		sp - PORT_SPEED_10_MBPS, PORT_SPEED_100_MBPS, or PORT_SPEED_1000_MBPS
*		delayTime - actual delay time will be (this value x 2.048uS).
*					the value cannot exceed 0x1FFF (or 8191 in decimal).
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sp is not valid or delayTime is > 0x1FFF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetFlowCtrlDelay
(
	IN GT_QD_DEV			*dev,
	IN GT_PORT_SPEED_MODE	sp,
	IN GT_U32				delayTime
);

/*******************************************************************************
* gsysGetFlowCtrlDelay
*
* DESCRIPTION:
*		This function retrieves Flow control delay time for 10Mbps, 100Mbps, and
*		1000Mbps.
*
* INPUTS:
*		sp - PORT_SPEED_10_MBPS, PORT_SPEED_100_MBPS, or PORT_SPEED_1000_MBPS
*
* OUTPUTS:
*		delayTime - actual delay time will be (this value x 2.048uS).
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sp is not valid or delayTime is > 0x1FFF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFlowCtrlDelay
(
	IN  GT_QD_DEV	*dev,
	IN  GT_PORT_SPEED_MODE	sp,
	OUT GT_U32		*delayTime
);

/*******************************************************************************
* gsysSetDevRoutingTable
*
* DESCRIPTION:
*		This function sets Device to Port mapping (which device is connected to
*		which port of this device).
*
* INPUTS:
*		devNum - target device number.
*		portNum - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if devNum >= 32 or port >= total number of ports.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDevRoutingTable
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		devNum,
	IN GT_LPORT 	port
);

/*******************************************************************************
* gsysGetDevRoutingTable
*
* DESCRIPTION:
*		This function gets Device to Port mapping (which device is connected to
*		which port of this device).
*
* INPUTS:
*		devNum - target device number.
*
* OUTPUTS:
*		portNum - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if devNum >= 32
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDevRoutingTable
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		devNum,
	OUT GT_LPORT 	*port
);

/*******************************************************************************
* gsysSetTrunkMaskTable
*
* DESCRIPTION:
*		This function sets Trunk mask vector table for load balancing.
*		This vector will be AND'ed with where the frame was originally egressed to.
*		To insure all trunks are load balanced correctly, the data in this table
*		needs to be correctly configured.
*
* INPUTS:
*		trunkNum - one of the eight Trunk mask vectors.
*		trunkMask - Trunk Mask bits. Bit 0 controls trunk masking for port 0,
*					bit 1 for port 1 , etc.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkNum > 0x7 or trunMask > 0x7FF (or port vector).
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetTrunkMaskTable
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		trunkNum,
	IN GT_U32		trunkMask
);

/*******************************************************************************
* gsysGetTrunkMaskTable
*
* DESCRIPTION:
*		This function sets Trunk mask vector table for load balancing.
*		This vector will be AND'ed with where the frame was originally egressed to.
*		To insure all trunks are load balanced correctly, the data in this table
*		needs to be correctly configured.
*
* INPUTS:
*		trunkNum - one of the eight Trunk mask vectors.
*
* OUTPUTS:
*		trunkMask - Trunk Mask bits. Bit 0 controls trunk masking for port 0,
*					bit 1 for port 1 , etc.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkNum > 0x7.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetTrunkMaskTable
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		trunkNum,
	OUT GT_U32		*trunkMask
);

/*******************************************************************************
* gsysSetHashTrunk
*
* DESCRIPTION:
*		Hash DA & SA for TrunkMask selection. Trunk load balancing is accomplished
*		by using the frame's DA and SA fields to access one of eight Trunk Masks.
*		When this bit is set to a one, the hashed computed for address table
*		lookups is used for the TrunkMask selection. When this bit is cleared to
*		a zero the lower 3 bits of the frame's DA and SA are XOR'ed together to
*		select the TrunkMask to use.
*
* INPUTS:
*		en - GT_TRUE to use lookup table, GT_FALSE to use XOR.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetHashTrunk
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetHashTrunk
*
* DESCRIPTION:
*		Hash DA & SA for TrunkMask selection. Trunk load balancing is accomplished
*		by using the frame's DA and SA fields to access one of eight Trunk Masks.
*		When this bit is set to a one, the hashed computed for address table
*		lookups is used for the TrunkMask selection. When this bit is cleared to
*		a zero the lower 3 bits of the frame's DA and SA are XOR'ed together to
*		select the TrunkMask to use.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use lookup table, GT_FALSE to use XOR.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetHashTrunk
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetTrunkRouting
*
* DESCRIPTION:
*		This function sets routing information for the given Trunk ID.
*
* INPUTS:
*		trunkId - Trunk ID.
*		trunkRoute - Trunk route bits. Bit 0 controls trunk routing for port 0,
*					bit 1 for port 1 , etc.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkId > 0xF or trunkRoute > 0x7FF(or port vector).
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetTrunkRouting
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		trunkId,
	IN GT_U32		trunkRoute
);

/*******************************************************************************
* gsysGetTrunkRouting
*
* DESCRIPTION:
*		This function retrieves routing information for the given Trunk ID.
*
* INPUTS:
*		trunkId - Trunk ID.
*
* OUTPUTS:
*		trunkRoute - Trunk route bits. Bit 0 controls trunk routing for port 0,
*					bit 1 for port 1 , etc.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkId > 0xF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetTrunkRouting
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		trunkId,
	OUT GT_U32		*trunkRoute
);



/* Prototype added for 88E6095 Rev 1 or Rev 2 */

/* gtPortCtrl.c */
/*******************************************************************************
* gprtGetDiscardBCastMode
*
* DESCRIPTION:
*       This routine gets the Discard Broadcast Mode. If the mode is enabled,
*		all the broadcast frames to the given port will be discarded.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		en - GT_TRUE, if enabled,
*			 GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDiscardBCastMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	OUT GT_BOOL 	 *en
);

/*******************************************************************************
* gprtSetDiscardBCastMode
*
* DESCRIPTION:
*       This routine sets the Discard Broadcast mode.
*		If the mode is enabled, all the broadcast frames to the given port will
*		be discarded.
*
* INPUTS:
*       port - logical port number
*		en - GT_TRUE, to enable the mode,
*			 GT_FALSE, otherwise.
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDiscardBCastMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	IN  GT_BOOL 	 en
);

/*******************************************************************************
* gprtGetFCOnRateLimitMode
*
* DESCRIPTION:
*       This routine returns mode that tells if ingress rate limiting uses Flow
*		Control. When this mode is enabled and the port receives frames over the
*		limit, Ingress Rate Limiting will be performed by stalling the
*		link partner using flow control, instead of discarding frames.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		en - GT_TRUE, if the mode is enabled,
*			 GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetFCOnRateLimitMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	OUT GT_BOOL 	 *en
);

/*******************************************************************************
* gprtSetFCOnRateLimitMode
*
* DESCRIPTION:
*       This routine sets the mode that tells if ingress rate limiting uses Flow
*		Control. When this mode is enabled and the port receives frames over the
*		limit, Ingress Rate Limiting will be performed by stalling the
*		link partner using flow control, instead of discarding frames.
*
* INPUTS:
*       port - logical port number
*		en - GT_TRUE, to enable the mode,
*			 GT_FALSE, otherwise.
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetFCOnRateLimitMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	IN  GT_BOOL 	 en
);


/* gtPortRateCtrl.c */

/*******************************************************************************
* grcSetBurstRate
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*       bsize	- burst size.
*       rate    - ingress data rate limit. These frames will be discarded after
*				the ingress rate selected is reached or exceeded.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*								Minimum rate for Burst Size 24K byte is 128Kbps
*								Minimum rate for Burst Size 48K byte is 256Kbps
*								Minimum rate for Burst Size 96K byte is 512Kbps
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetBurstRate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_BURST_SIZE   bsize,
    IN GT_BURST_RATE   rate
);

/*******************************************************************************
* grcGetBurstRate
*
* DESCRIPTION:
*       This routine retrieves the port's ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       bsize	- burst size.
*       rate    - ingress data rate limit. These frames will be discarded after
*				the ingress rate selected is reached or exceeded.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetBurstRate
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_BURST_SIZE   *bsize,
    OUT GT_BURST_RATE   *rate
);


/*******************************************************************************
* grcSetTCPBurstRate
*
* DESCRIPTION:
*       This routine sets the port's TCP/IP ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit for TCP/IP packets. These frames will
*				be discarded after the ingress rate selected is reached or exceeded.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*								Valid rate is GT_BURST_NO_LIMIT, or between
*								64Kbps and 1500Kbps.
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetTCPBurstRate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_BURST_RATE   rate
);


/*******************************************************************************
* grcGetTCPBurstRate
*
* DESCRIPTION:
*       This routine sets the port's TCP/IP ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rate    - ingress data rate limit for TCP/IP packets. These frames will
*				be discarded after the ingress rate selected is reached or exceeded.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_VALUE        - register value is not known
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetTCPBurstRate
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_BURST_RATE   *rate
);


/* gtSysCtrl.c */
/*******************************************************************************
* gsysSetRateLimitMode
*
* DESCRIPTION:
*		Ingress Rate Limiting can be either Priority based or Burst Size based.
*		This routine sets which mode to use.
*
* INPUTS:
*		mode - either GT_RATE_PRI_BASE or GT_RATE_BURST_BASE
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if invalid mode is used.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRateLimitMode
(
	IN GT_QD_DEV	*dev,
	IN GT_INGRESS_RATE_MODE mode
);

/*******************************************************************************
* gsysGetRateLimitMode
*
* DESCRIPTION:
*		Ingress Rate Limiting can be either Priority based or Burst Size based.
*		This routine gets which mode is being used.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - either GT_RATE_PRI_BASE or GT_RATE_BURST_BASE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRateLimitMode
(
	IN  GT_QD_DEV	*dev,
	OUT GT_INGRESS_RATE_MODE *mode
);

/*******************************************************************************
* gsysSetAgeInt
*
* DESCRIPTION:
*		Enable/Disable Age Refresh Interrupt. If CPU Directed Learning is being
*		used (gprtSetLockedPort), it may be desirable to know when an address is
*		still being used before it totally ages out of the switch. This can be
*		accomplished by enabling Age Refresh Interrupt (or ATU Age Violation Int).
*		An ATU Age Violation looks identical to and reported the same as an ATU
*		Miss Violation. The only difference is when this reported. Normal ATU Miss
*		Violation only occur if a new SA arrives at a LockedPort. The Age version
*		of the ATU Miss Violation occurs if an SA arrives at a LockedPort, where
*		the address is contained in the ATU's database, but where its EntryState
*		is less than 0x4 (i.e., it has aged more than 1/2 way).
*		GT_ATU_PROB Interrupt should be enabled for this interrupt to occur.
*		Refer to eventSetActive routine to enable GT_ATU_PROB.
*
*
* INPUTS:
*		en - GT_TRUE, to enable,
*			 GT_FALSE, otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetAgeInt
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetAgeInt
*
* DESCRIPTION:
*		Get state of Age Refresh Interrupt mode. If CPU Directed Learning is being
*		used (gprtSetLockedPort), it may be desirable to know when an address is
*		still being used before it totally ages out of the switch. This can be
*		accomplished by enabling Age Refresh Interrupt (or ATU Age Violation Int).
*		An ATU Age Violation looks identical to and reported the same as an ATU
*		Miss Violation. The only difference is when this reported. Normal ATU Miss
*		Violation only occur if a new SA arrives at a LockedPort. The Age version
*		of the ATU Miss Violation occurs if an SA arrives at a LockedPort, where
*		the address is contained in the ATU's database, but where its EntryState
*		is less than 0x4 (i.e., it has aged more than 1/2 way).
*		GT_ATU_PROB Interrupt should be enabled for this interrupt to occur.
*		Refer to eventSetActive routine to enable GT_ATU_PROB.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE, if enabled,
*			 GT_FALSE, otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetAgeInt
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL		*en
);


/* For Zephyr */

/* gtPhyCtrl.c */
/*******************************************************************************
* gprtGetPhyLinkStatus
*
* DESCRIPTION:
*       This routine retrieves the Link status.
*
* INPUTS:
* 		port 	- The logical port number
*
* OUTPUTS:
*       linkStatus - GT_FALSE if link is not established,
*				     GT_TRUE if link is established.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPhyLinkStatus
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
    IN GT_BOOL 	 *linkStatus
);


/*******************************************************************************
* gprtSetPktGenEnable
*
* DESCRIPTION:
*       This routine enables or disables Packet Generator.
*       Link should be established first prior to enabling the packet generator,
*       and generator will generate packets at the speed of the established link.
*		When enables packet generator, the following information should be
*       provided:
*           Payload Type:  either Random or 5AA55AA5
*           Packet Length: either 64 or 1514 bytes
*           Error Packet:  either Error packet or normal packet
*
* INPUTS:
* 		port 	- The logical port number
*       en      - GT_TRUE to enable, GT_FALSE to disable
*       pktInfo - packet information(GT_PG structure pointer), if en is GT_TRUE.
*                 ignored, if en is GT_FALSE
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPktGenEnable
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
    IN GT_BOOL   en,
    IN GT_PG     *pktInfo
);

/*******************************************************************************
* gprtGetSerdesMode
*
* DESCRIPTION:
*       This routine reads Serdes Interface Mode.
*
* INPUTS:
*       port    - logical port number
*
* OUTPUTS:
*       mode    - Serdes Interface Mode
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSerdesMode
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
	IN  GT_SERDES_MODE *mode
);

/*******************************************************************************
* gprtSetSerdesMode
*
* DESCRIPTION:
*       This routine sets Serdes Interface Mode.
*
* INPUTS:
*       port    - logical port number
*       mode    - Serdes Interface Mode
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetSerdesMode
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
	IN  GT_SERDES_MODE mode
);


/* gtSysCtrl.c */

/*******************************************************************************
* gsysSetForceSnoopPri
*
* DESCRIPTION:
*		Force Snooping Priority. The priority on IGMP or MLD Snoop frames are
*		set to the SnoopPri value (gsysSetSnoopPri API) when Force Snooping
*       Priority is enabled. When it's disabled, the priority on these frames
*		is not modified.
*
* INPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetForceSnoopPri
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetForceSnoopPri
*
* DESCRIPTION:
*		Force Snooping Priority. The priority on IGMP or MLD Snoop frames are
*		set to the SnoopPri value (gsysSetSnoopPri API) when Force Snooping
*       Priority is enabled. When it's disabled, the priority on these frames
*		is not modified.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetForceSnoopPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);


/*******************************************************************************
* gsysSetSnoopPri
*
* DESCRIPTION:
*		Snoop Priority. When ForceSnoopPri (gsysSetForceSnoopPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU Snoop frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on IGMP/MLD snoop frames.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetSnoopPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
);


/*******************************************************************************
* gsysGetSnoopPri
*
* DESCRIPTION:
*		Snoop Priority. When ForceSnoopPri (gsysSetForceSnoopPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU Snoop frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on IGMP/MLD snoop frames.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetSnoopPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
);


/*******************************************************************************
* gsysSetForceARPPri
*
* DESCRIPTION:
*		Force ARP Priority. The priority on ARP frames are set to the ARPPri
*       value (gsysSetARPPri API) when Force ARP Priority is enabled. When it's
*       disabled, the priority on these frames is not modified.
*
* INPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetForceARPPri
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetForceARPPri
*
* DESCRIPTION:
*		Force ARP Priority. The priority on ARP frames are set to the ARPPri
*       value (gsysSetARPPri API) when Force ARP Priority is enabled. When it's
*       disabled, the priority on these frames is not modified.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetForceARPPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);


/*******************************************************************************
* gsysSetARPPri
*
* DESCRIPTION:
*		ARP Priority. When ForceARPPri (gsysSetForceARPPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU ARP frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on ARP frames.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetARPPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
);


/*******************************************************************************
* gsysGetARPPri
*
* DESCRIPTION:
*		ARP Priority. When ForceARPPri (gsysSetForceARPPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU ARP frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on ARP frames.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetARPPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
);


/* added for 88E6065 */

/* gtBrgVlan.c */

/********************************************************************
* gvlnSetForceMap
*
* DESCRIPTION:
*       This routine enables/disables Force Map feature.
*		When Force Map feature is enabled, all received frames will be
*		considered MGMT and they are mapped to the port or ports defined
*		in the VLAN Table overriding the mapping from the address database.
*
* INPUTS:
*       port    - logical port number to set.
*       mode    - GT_TRUE, to enable force map feature
*                 GT_FAULSE, otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gvlnSetForceMap
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT 	port,
    IN  GT_BOOL  	mode
);


/********************************************************************
* gvlnGetForceMap
*
* DESCRIPTION:
*       This routine checks if Force Map feature is enabled.
*		When Force Map feature is enabled, all received frames will be
*		considered MGMT and they are mapped to the port or ports defined
*		in the VLAN Table overriding the mapping from the address database.
*
* INPUTS:
*       port    - logical port number to set.
*
* OUTPUTS:
*       mode    - GT_TRUE, to enable force map feature
*                 GT_FAULSE, otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gvlnGetForceMap
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT 	port,
    OUT GT_BOOL  	*mode
);

/* gtEvents.c */

/*******************************************************************************
* geventSetAgeIntEn
*
* DESCRIPTION:
*		This routine enables/disables Age Interrupt for a port.
*		When it's enabled, ATU Age Violation interrupts from this port are enabled.
*		An Age Violation will occur anytime a port is Locked(gprtSetLockedPort)
*		and the ingressing frame's SA is contained in the ATU as a non-Static
*		entry with a EntryState less than 0x4.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Interrupt,
*			   GT_FALUSE to disable
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventSetAgeIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* geventGetAgeIntEn
*
* DESCRIPTION:
*		This routine gets Age Interrupt Enable for the port.
*		When it's enabled, ATU Age Violation interrupts from this port are enabled.
*		An Age Violation will occur anytime a port is Locked(gprtSetLockedPort)
*		and the ingressing frame's SA is contained in the ATU as a non-Static
*		entry with a EntryState less than 0x4.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Interrupt,
*			   GT_FALUSE to disable
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventGetAgeIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);


/* gtPIRL.c */

/*******************************************************************************
* gpirlActivate
*
* DESCRIPTION:
*       This routine activates Ingress Rate Limiting for the given ports by
*		initializing a resource bucket, assigning ports, and configuring
*		Bucket Parameters.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*       portVec  - the list of ports that share the bucket.
*		pirlData - PIRL resource parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlActivate
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_U32		portVec,
	IN  GT_PIRL_DATA	*pirlData
);

/*******************************************************************************
* gpirlDeactivate
*
* DESCRIPTION:
*       This routine deactivates Ingress Rate Limiting for the given bucket.
*		It simply removes every ports from the Ingress Rate Resource.
*		It is assumed that gpirlActivate has been successfully called with
*		the irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be deactivated
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlDeactivate
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit
);

/*******************************************************************************
* gpirlUpdateParam
*
* DESCRIPTION:
*       This routine updates IRL Parameter.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11)
*		pirlData - PIRL resource parameters
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlUpdateParam
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_PIRL_DATA	*pirlData
);

/*******************************************************************************
* gpirlReadParam
*
* DESCRIPTION:
*       This routine retrieves IRL Parameter.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*
* OUTPUTS:
*		pirlData - PIRL resource parameters.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlReadParam
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	OUT GT_PIRL_DATA	*pirlData
);

/*******************************************************************************
* gpirlUpdatePortVec
*
* DESCRIPTION:
*       This routine updates port list that share the bucket.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*       portVec  - the list of ports that share the bucket.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlUpdatePortVec
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_U32		portVec
);

/*******************************************************************************
* gpirlReadPortVec
*
* DESCRIPTION:
*       This routine retrieves port list that share the bucket.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*
* OUTPUTS:
*       portVec  - the list of ports that share the bucket.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlReadPortVec
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	OUT GT_U32		*portVec
);

/*******************************************************************************
* grcGetPirlFcMode
*
* DESCRIPTION:
*       This routine gets Port Ingress Rate Limit Flow Control mode.
*		When EBSLimitAction is programmed to generate a flow control message,
*		the deassertion of flow control is controlled by this mode.
*			GT_PIRL_FC_DEASSERT_EMPTY:
*				De-assert when the ingress rate resource has become empty
*			GT_PIRL_FC_DEASSERT_CBS_LIMIT
*				De-assert when the ingress rate resource has enough room as
*				specified by the CBSLimit.
*		Please refer to GT_PIRL_RESOURCE structure for EBSLimitAction and
*		CBSLimit.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		mode - GT_PIRL_FC_DEASSERT enum type
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetPirlFcMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_PIRL_FC_DEASSERT		*mode
);

/*******************************************************************************
* gpirlGetIngressRateResource
*
* DESCRIPTION:
*       This routine gets Ingress Rate Limiting Resources assigned to the port.
*		This vector is used to attach specific counter resources to the physical
*		port. And the same counter resource can be attached to more than one port.
*
* INPUTS:
*       port   - logical port number
*
* OUTPUTS:
*		resVec - resource vector (bit 0 for irl unit 0, bit 1 for irl unit 1, etc.)
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gpirlGetIngressRateResource
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_U32		*resVec
);



/* gtPortStatus.c */

/*******************************************************************************
* gprtGetPxMode
*
* DESCRIPTION:
*		This routine retrives 4 bits of Px_MODE Configuration value.
*		If speed and duplex modes are forced, the returned mode value would be
*		different from the configuration pin values.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - Px_MODE configuration value
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPxMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_U32  	*mode
);

/*******************************************************************************
* gprtGetMiiInterface
*
* DESCRIPTION:
*		This routine retrives Mii Interface Mode.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Mii Interface is enabled,
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetMiiInterface
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtGetFdFlowDis
*
* DESCRIPTION:
*		This routine retrives the read time value of the Full Duplex Flow Disable.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Full Duplex Flow Disable.
*	   		    GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetFdFlowDis
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtGetHdFlowDis
*
* DESCRIPTION:
*		This routine retrives the read time value of the Half Duplex Flow Disable.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Half Duplex Flow Disable.
*	   		    GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetHdFlowDis
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
);

/*******************************************************************************
* gprtGetOutQSize
*
* DESCRIPTION:
*		This routine gets egress queue size counter value.
*		This counter reflects the current number of Egress buffers switched to
*		this port. This is the total number of buffers across all four priority
*		queues.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		count - egress queue size counter value
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetOutQSize
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*count
);


/* gtPortCtrl.c */

/*******************************************************************************
* gprtSetSAFiltering
*
* DESCRIPTION:
*		This routine set the Source Address(SA) fitering method.
*			GT_SA_FILTERING_DISABLE :
*				no frame will be filtered.
*			GT_SA_DROP_ON_LOCK :
*				discard if SA field is not in the ATU's address database.
*			GT_SA_DROP_ON_UNLOC :
*				discard if SA field is in the ATU's address database as Static
*				entry with a PortVec of all zeros.
*			GT_SA_DROP_TO_CPU :
*				Ingressing frames will be mapped to the CPU Port if their SA
*				field is in the ATU's address database as Static entry with a
*				PortVec of all zeros. Otherwise, the frames will be discarded
*				if their SA field is not in the ATU's address database or if this
*				port's bit is not set in the PortVec bits for the frame's SA.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_SA_FILTERING structure
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS gprtSetSAFiltering
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_SA_FILTERING    mode
);

/*******************************************************************************
* gprtGetSAFiltering
*
* DESCRIPTION:
*		This routine gets the Source Address(SA) fitering method.
*			GT_SA_FILTERING_DISABLE :
*				no frame will be filtered.
*			GT_SA_DROP_ON_LOCK :
*				discard if SA field is not in the ATU's address database.
*			GT_SA_DROP_ON_UNLOC :
*				discard if SA field is in the ATU's address database as Static
*				entry with a PortVec of all zeros.
*			GT_SA_DROP_TO_CPU :
*				Ingressing frames will be mapped to the CPU Port if their SA
*				field is in the ATU's address database as Static entry with a
*				PortVec of all zeros. Otherwise, the frames will be discarded
*				if their SA field is not in the ATU's address database or if this
*				port's bit is not set in the PortVec bits for the frame's SA.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_SA_FILTERING structure
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS gprtGetSAFiltering
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_SA_FILTERING    *mode
);


/*******************************************************************************
* gprtSetARPtoCPU
*
* DESCRIPTION:
*		When ARPtoCPU is set to GT_TRUE, ARP frames are mapped to the CPU port.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE, to map ARP frames to CPU Port,
*			   GT_FALSE, otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS gprtSetARPtoCPU
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    mode
);


/*******************************************************************************
* gprtGetARPtoCPU
*
* DESCRIPTION:
*		When ARPtoCPU is set to GT_TRUE, ARP frames are mapped to the CPU port.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE, to map ARP frames to CPU Port,
*			   GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS gprtGetARPtoCPU
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *mode
);

/*******************************************************************************
* gprtSetEgressFlood
*
* DESCRIPTION:
*       This routine set Egress Flooding Mode.
*		Frames with unknown DA (Destination Address that is not in ATU database)
*		generally flood out all the ports. This mode can be used to prevent
*		those frames from egressing this port as follows:
*			GT_BLOCK_EGRESS_UNKNOWN
*				do not egress frame with unknown DA (both unicast and multicast)
*			GT_BLOCK_EGRESS_UNKNOWN_MULTICAST
*				do not egress frame with unknown multicast DA
*			GT_BLOCK_EGRESS_UNKNOWN_UNICAST
*				do not egress frame with unknown unicast DA
*			GT_BLOCK_EGRESS_NONE
*				egress all frames with unknown DA
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_EGRESS_FLOOD structure
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetEgressFlood
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT     port,
    IN  GT_EGRESS_FLOOD      mode
);

/*******************************************************************************
* gprtGetEgressFlood
*
* DESCRIPTION:
*       This routine gets Egress Flooding Mode.
*		Frames with unknown DA (Destination Address that is not in ATU database)
*		generally flood out all the ports. This mode can be used to prevent
*		those frames from egressing this port as follows:
*			GT_BLOCK_EGRESS_UNKNOWN
*				do not egress frame with unknown DA (both unicast and multicast)
*			GT_BLOCK_EGRESS_UNKNOWN_MULTICAST
*				do not egress frame with unknown multicast DA
*			GT_BLOCK_EGRESS_UNKNOWN_UNICAST
*				do not egress frame with unknown unicast DA
*			GT_BLOCK_EGRESS_NONE
*				egress all frames with unknown DA
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_EGRESS_FLOOD structure
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetEgressFlood
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT     port,
    OUT GT_EGRESS_FLOOD      *mode
);

/*******************************************************************************
* gprtSetPortSched
*
* DESCRIPTION:
*		This routine sets Port Scheduling Mode.
*		When usePortSched is enablied, this mode is used to select the Queue
*		controller's scheduling on the port as follows:
*			GT_PORT_SCHED_WEIGHTED_RRB - use 8,4,2,1 weighted fair scheduling
*			GT_PORT_SCHED_STRICT_PRI - use a strict priority scheme
*
* INPUTS:
*		port - the logical port number
*		mode - GT_PORT_SCHED_MODE enum type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPortSched
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_PORT_SCHED_MODE		mode
);

/*******************************************************************************
* gprtGetPortSched
*
* DESCRIPTION:
*		This routine gets Port Scheduling Mode.
*		When usePortSched is enablied, this mode is used to select the Queue
*		controller's scheduling on the port as follows:
*			GT_PORT_SCHED_WEIGHTED_RRB - use 8,4,2,1 weighted fair scheduling
*			GT_PORT_SCHED_STRICT_PRI - use a strict priority scheme
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_PORT_SCHED_MODE enum type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPortSched
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_PORT_SCHED_MODE		*mode
);


/*******************************************************************************
* gprtSetProviderTag
*
* DESCRIPTION:
*		This routine sets Provider Tag which indicates the provider tag (Ether
*		Type) value that needs to be matched to in ingress to determine if a
*		frame is Provider tagged or not.
*
* INPUTS:
*		port - the logical port number
*		tag  - Provider Tag (Ether Type)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetProviderTag
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_U16		tag
);

/*******************************************************************************
* gprtGetProviderTag
*
* DESCRIPTION:
*		This routine gets Provider Tag which indicates the provider tag (Ether
*		Type) value that needs to be matched to in ingress to determine if a
*		frame is Provider tagged or not.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		tag  - Provider Tag (Ether Type)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetProviderTag
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*tag
);



/* gtPortRateCtrl.c */

/*******************************************************************************
* grcSetVidNrlEn
*
* DESCRIPTION:
*       This routine enables/disables VID None Rate Limit (NRL).
*		When VID NRL is enabled and the determined VID of a frame results in a VID
*		whose VIDNonRateLimit in the VTU Table is set to GT_TURE, then the frame
*		will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*		mode - GT_TRUE to enable VID None Rate Limit
*			   GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetVidNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* grcGetVidNrlEn
*
* DESCRIPTION:
*       This routine gets VID None Rate Limit (NRL) mode.
*		When VID NRL is enabled and the determined VID of a frame results in a VID
*		whose VIDNonRateLimit in the VTU Table is set to GT_TURE, then the frame
*		will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable VID None Rate Limit
*			   GT_FALSE otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetVidNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* grcSetSaNrlEn
*
* DESCRIPTION:
*       This routine enables/disables SA None Rate Limit (NRL).
*		When SA NRL is enabled and the source address of a frame results in a ATU
*		hit where the SA's MAC address returns an EntryState that indicates Non
*		Rate Limited, then the frame will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*		mode - GT_TRUE to enable SA None Rate Limit
*			   GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetSaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* grcGetSaNrlEn
*
* DESCRIPTION:
*       This routine gets SA None Rate Limit (NRL) mode.
*		When SA NRL is enabled and the source address of a frame results in a ATU
*		hit where the SA's MAC address returns an EntryState that indicates Non
*		Rate Limited, then the frame will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable SA None Rate Limit
*			   GT_FALSE otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetSaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* grcSetDaNrlEn
*
* DESCRIPTION:
*       This routine enables/disables DA None Rate Limit (NRL).
*		When DA NRL is enabled and the destination address of a frame results in
*		a ATU hit where the DA's MAC address returns an EntryState that indicates
*		Non Rate Limited, then the frame will not be ingress nor egress rate
*		limited.
*
* INPUTS:
*       port - logical port number.
*		mode - GT_TRUE to enable DA None Rate Limit
*			   GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetDaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* grcGetDaNrlEn
*
* DESCRIPTION:
*       This routine gets SA None Rate Limit (NRL) mode.
*		When DA NRL is enabled and the destination address of a frame results in
*		a ATU hit where the DA's MAC address returns an EntryState that indicates
*		Non Rate Limited, then the frame will not be ingress nor egress rate
*		limited.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable DA None Rate Limit
*			   GT_FALSE otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetDaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* grcSetELimitMode
*
* DESCRIPTION:
*       This routine sets Egress Rate Limit counting mode.
*		The supported modes are as follows:
*			GT_PIRL_ELIMIT_FRAME -
*				Count the number of frames
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*			GT_PIRL_ELIMIT_LAYER2 -
*				Count all Layer 2 bytes: Frame's DA to CRC
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*		mode - GT_PIRL_ELIMIT_MODE enum type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		GT_PIRL_ELIMIT_FRAME mode is supported by only a few devices.
*		Please refer to the device datasheet for details.
*
*******************************************************************************/
GT_STATUS grcSetELimitMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_PIRL_ELIMIT_MODE		mode
);

/*******************************************************************************
* grcGetELimitMode
*
* DESCRIPTION:
*       This routine gets Egress Rate Limit counting mode.
*		The supported modes are as follows:
*			GT_PIRL_ELIMIT_FRAME -
*				Count the number of frames
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*			GT_PIRL_ELIMIT_LAYER2 -
*				Count all Layer 2 bytes: Frame's DA to CRC
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		mode - GT_PIRL_ELIMIT_MODE enum type
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		GT_PIRL_ELIMIT_FRAME mode is supported by only a few devices.
*		Please refer to the device datasheet for details.
*
*******************************************************************************/
GT_STATUS grcGetELimitMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_PIRL_ELIMIT_MODE		*mode
);

/*******************************************************************************
* grcSetRsvdNrlEn
*
* DESCRIPTION:
*       This routine sets Reserved Non Rate Limit.
*		When this feature is enabled, frames that match the requirements of the
*		Rsvd2Cpu bit below will also be considered to be ingress and egress non
*		rate limited.
*
* INPUTS:
*       en - GT_TRUE to enable Reserved Non Rate Limit,
*			 GT_FALSE to disable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS grcSetRsvdNrlEn
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
);

/*******************************************************************************
* grcGetRsvdNrlEn
*
* DESCRIPTION:
*       This routine gets Reserved Non Rate Limit.
*		When this feature is enabled, frames that match the requirements of the
*		Rsvd2Cpu bit below will also be considered to be ingress and egress non
*		rate limited.
*
* INPUTS:
*       en - GT_TRUE to enable Reserved Non Rate Limit,
*			 GT_FALSE to disable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS grcGetRsvdNrlEn
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
);


/* gtPortRmon.c */

/*******************************************************************************
* gstatsGetRealtimePortCounter
*
* DESCRIPTION:
*		This routine gets a specific realtime counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstatsGetRealtimePortCounter
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS3	counter,
	OUT GT_U32			*statsData
);


/* gtQosMap.c */

/*******************************************************************************
* gqosSetVIDFPriOverride
*
* DESCRIPTION:
*		This routine sets VID Frame Priority Override. When this feature is enabled,
*		VID Frame priority overrides can occur on this port.
*		VID Frame priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDFPri override field is set to GT_TRUE.
*		When this occurs the VIDFPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new VIDFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for VID Frame Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetVIDFPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gqosGetVIDFPriOverride
*
* DESCRIPTION:
*		This routine gets VID Frame Priority Override. When this feature is enabled,
*		VID Frame priority overrides can occur on this port.
*		VID Frame priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDFPri override field is set to GT_TRUE.
*		When this occurs the VIDFPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new VIDFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for VID Frame Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetVIDFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gqosSetSAFPriOverride
*
* DESCRIPTION:
*		This routine sets Source Address(SA) Frame Priority Override.
*		When this feature is enabled, SA Frame priority overrides can occur on
*		this port.
*		SA ATU Frame priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for SA Frame Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetSAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* gqosGetSAFPriOverride
*
* DESCRIPTION:
*		This routine gets Source Address(SA) Frame Priority Override.
*		When this feature is enabled, SA Frame priority overrides can occur on
*		this port.
*		SA ATU Frame priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for SA Frame Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetSAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gqosSetDAFPriOverride
*
* DESCRIPTION:
*		This routine sets Destination Address(DA) Frame Priority Override.
*		When this feature is enabled, DA Frame priority overrides can occur on
*		this port.
*		DA ATU Frame priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for DA Frame Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetDAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* gqosGetDAFPriOverride
*
* DESCRIPTION:
*		This routine gets Destination Address(DA) Frame Priority Override.
*		When this feature is enabled, DA Frame priority overrides can occur on
*		this port.
*		DA ATU Frame priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for DA Frame Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetDAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gqosSetVIDQPriOverride
*
* DESCRIPTION:
*		This routine sets VID Queue Priority Override. When this feature is enabled,
*		VID Queue priority overrides can occur on this port.
*		VID Queue priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDQPri override field is set to GT_TRUE.
*		When this occurs the VIDQPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new VIDQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for VID Queue Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetVIDQPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gqosGetVIDQPriOverride
*
* DESCRIPTION:
*		This routine gets VID Queue Priority Override. When this feature is enabled,
*		VID Queue priority overrides can occur on this port.
*		VID Queue priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDQPri override field is set to GT_TRUE.
*		When this occurs the VIDQPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new VIDQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for VID Queue Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetVIDQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gqosSetSAQPriOverride
*
* DESCRIPTION:
*		This routine sets Source Address(SA) Queue Priority Override.
*		When this feature is enabled, SA Queue priority overrides can occur on
*		this port.
*		SA ATU Queue priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for SA Queue Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetSAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* gqosGetSAQPriOverride
*
* DESCRIPTION:
*		This routine gets Source Address(SA) Queue Priority Override.
*		When this feature is enabled, SA Queue priority overrides can occur on
*		this port.
*		SA ATU Queue priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for SA Queue Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetSAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gqosSetDAQPriOverride
*
* DESCRIPTION:
*		This routine sets Destination Address(DA) Queue Priority Override.
*		When this feature is enabled, DA Queue priority overrides can occur on
*		this port.
*		DA ATU Queue priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for DA Queue Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetDAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* gqosGetDAQPriOverride
*
* DESCRIPTION:
*		This routine sets Destination Address(DA) Queue Priority Override.
*		When this feature is enabled, DA Queue priority overrides can occur on
*		this port.
*		DA ATU Queue priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for DA Queue Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetDAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gqosSetARPQPriOverride
*
* DESCRIPTION:
*		This routine sets ARP Queue Priority Override.
*		When this feature is enabled, ARP Queue priority overrides can occur on
*		this port.
*		ARP Queue priority override occurs for all ARP frames.
*		When this occurs, the frame's previously determined egress queue priority
*		will be overwritten with ArpQPri.
*		If the frame egresses tagged the priority in the frame will not
*		be modified. When used, the two bits of the ArpQPri priority determine the
*		egress queue the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for ARP Queue Priority Override,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetARPQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* gqosGetARPQPriOverride
*
* DESCRIPTION:
*		This routine sets ARP Queue Priority Override.
*		When this feature is enabled, ARP Queue priority overrides can occur on
*		this port.
*		ARP Queue priority override occurs for all ARP frames.
*		When this occurs, the frame's previously determined egress queue priority
*		will be overwritten with ArpQPri.
*		If the frame egresses tagged the priority in the frame will not
*		be modified. When used, the two bits of the ArpQPri priority determine the
*		egress queue the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for ARP Queue Priority Override,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetARPQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);


/*******************************************************************************
* gqosSetQPriValue
*
* DESCRIPTION:
*       This routine sets Queue priority value to used when forced.
*		When ForceQPri is enabled (gqosSetForceQPri), all frames entering this port
*		are mapped to the priority queue defined in this value, unless a VTU, SA,
*		DA or ARP priority override occurs. The Frame's priority (FPri) is not
*		effected by this value.
*
* INPUTS:
*       port - the logical port number.
*       pri  - Queue priority value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if pri > 3
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosSetQPriValue
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    IN  GT_U8      pri
);

/*******************************************************************************
* gqosGetQPriValue
*
* DESCRIPTION:
*       This routine gets Queue priority value to used when forced.
*		When ForceQPri is enabled (gqosSetForceQPri), all frames entering this port
*		are mapped to the priority queue defined in this value, unless a VTU, SA,
*		DA or ARP priority override occurs. The Frame's priority (FPri) is not
*		effected by this value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       pri  - Queue priority value
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosGetQPriValue
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_U8      *pri
);

/*******************************************************************************
* gqosSetForceQPri
*
* DESCRIPTION:
*       This routine enables/disables forcing Queue priority.
*		When ForceQPri is disabled, normal priority queue mapping is used on all
*		ingressing frames entering this port. When it's enabled, all frames
*		entering this port are mapped to the QPriValue (gqosSetQPriValue), unless
*		a VTU, SA, DA or ARP priority override occurs. The frame's priorty (FPri)
*		is not effected by this feature.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE, to force Queue Priority,
*			   GT_FALSE, otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosSetForceQPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    IN  GT_BOOL    en
);

/*******************************************************************************
* gqosGetForceQPri
*
* DESCRIPTION:
*       This routine checks if forcing Queue priority is enabled.
*		When ForceQPri is disabled, normal priority queue mapping is used on all
*		ingressing frames entering this port. When it's enabled, all frames
*		entering this port are mapped to the QPriValue (gqosSetQPriValue), unless
*		a VTU, SA, DA or ARP priority override occurs. The frame's priorty (FPri)
*		is not effected by this feature.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       en   - GT_TRUE, to force Queue Priority,
*			   GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosGetForceQPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *en
);

/*******************************************************************************
* gqosSetDefFPri
*
* DESCRIPTION:
*       This routine sets the default frame priority (0 ~ 7).
*		This priority is used as the default frame priority (FPri) to use when
*		no other priority information is available.
*
* INPUTS:
*       port - the logical port number
*       pri  - default frame priority
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if pri > 7
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosSetDefFPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    IN  GT_U8      pri
);

/*******************************************************************************
* gqosGetDefFPri
*
* DESCRIPTION:
*       This routine gets the default frame priority (0 ~ 7).
*		This priority is used as the default frame priority (FPri) to use when
*		no other priority information is available.
*
* INPUTS:
*       port - the logical port number
*
* OUTPUTS:
*       pri  - default frame priority
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosGetDefFPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_U8      *pri
);


/*******************************************************************************
* gqosSetArpQPri
*
* DESCRIPTION:
*       This routine sets ARP queue Priority to use for ARP QPri Overridden
*		frames. When a ARP frame is received on a por tthat has its ARP
*		QPriOVerride is enabled, the QPri assigned to the frame comes from
*		this value
*
* INPUTS:
*       pri - ARP Queue Priority (0 ~ 3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if pri > 3
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gqosSetArpQPri
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     pri
);


/*******************************************************************************
* gqosGetArpQPri
*
* DESCRIPTION:
*       This routine gets ARP queue Priority to use for ARP QPri Overridden
*		frames. When a ARP frame is received on a por tthat has its ARP
*		QPriOVerride is enabled, the QPri assigned to the frame comes from
*		this value
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       pri - ARP Queue Priority (0 ~ 3)
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gqosGetArpQPri
(
    IN  GT_QD_DEV *dev,
    OUT GT_U8     *pri
);


/* gtSysCtrl.c */

/*******************************************************************************
* gsysSetUsePortSchedule
*
* DESCRIPTION:
*       This routine sets per port scheduling mode
*
* INPUTS:
*       en - GT_TRUE enables per port scheduling,
*			 GT_FALSE disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetUsePortSchedule
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
);

/*******************************************************************************
* gsysGetUsePortSchedule
*
* DESCRIPTION:
*       This routine gets per port scheduling mode
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE enables per port scheduling,
*			 GT_FALSE disable.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetUsePortSchedule
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
);

/*******************************************************************************
* gsysSetOldHader
*
* DESCRIPTION:
*       This routine sets Egress Old Header.
*		When this feature is enabled and frames are egressed with a Marvell Header,
*		the format of the Header is slightly modified to be backwards compatible
*		with previous devices that used the original Header. Specifically, bit 3
*		of the Header's 2nd octet is cleared to a zero such that only FPri[2:1]
*		is available in the Header.
*
* INPUTS:
*       en - GT_TRUE to enable Old Header Mode,
*			 GT_FALSE to disable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetOldHader
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
);

/*******************************************************************************
* gsysGetOldHader
*
* DESCRIPTION:
*       This routine gets Egress Old Header.
*		When this feature is enabled and frames are egressed with a Marvell Header,
*		the format of the Header is slightly modified to be backwards compatible
*		with previous devices that used the original Header. Specifically, bit 3
*		of the Header's 2nd octet is cleared to a zero such that only FPri[2:1]
*		is available in the Header.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE to enable Old Header Mode,
*			 GT_FALSE to disable
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetOldHader
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
);

/*******************************************************************************
* gsysSetRecursiveStrippingDisable
*
* DESCRIPTION:
*       This routine determines if recursive tag stripping feature needs to be
*		disabled.
*
* INPUTS:
*       en - GT_TRUE to disable Recursive Tag Stripping,
*			 GT_FALSE to enable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetRecursiveStrippingDisable
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
);

/*******************************************************************************
* gsysGetRecursiveStrippingDisable
*
* DESCRIPTION:
*       This routine checks if recursive tag stripping feature is disabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE, if Recursive Tag Stripping is disabled,
*			 GT_FALSE, otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetRecursiveStrippingDisable
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
);

/*******************************************************************************
* gsysSetCPUPort
*
* DESCRIPTION:
*       This routine sets CPU Port where Rsvd2Cpu frames and IGMP/MLD Snooped
*		frames are destined.
*
* INPUTS:
*       cpuPort - CPU Port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetCPUPort
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  cpuPort
);

/*******************************************************************************
* gsysGetCPUPort
*
* DESCRIPTION:
*       This routine gets CPU Port where Rsvd2Cpu frames and IGMP/MLD Snooped
*		frames are destined.
*
* INPUTS:
*       cpuPort - CPU Port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetCPUPort
(
    IN  GT_QD_DEV *dev,
    OUT GT_LPORT  *cpuPort
);



/* gtSysStatus.c */

/*******************************************************************************
* gsysGetFreeQSize
*
* DESCRIPTION:
*       This routine gets Free Queue Counter. This counter reflects the
*		current number of unalllocated buffers available for all the ports.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       count - Free Queue Counter
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFreeQSize
(
    IN  GT_QD_DEV	*dev,
    OUT GT_U16 		*count
);

/*******************************************************************************
				New APIs in DSDT 2.6
*******************************************************************************/

/* gtBrgFdb.c */

/*******************************************************************************
* gfdbSetPortAtuLearnLimit
*
* DESCRIPTION:
*       Port's auto learning limit. When the limit is non-zero value, the number
*		of MAC addresses that can be learned on this port are limited to the value
*		specified in this API. When the learn limit has been reached any frame
*		that ingresses this port with a source MAC address not already in the
*		address database that is associated with this port will be discarded.
*		Normal auto-learning will resume on the port as soon as the number of
*		active unicast MAC addresses associated to this port is less than the
*		learn limit.
*		CPU directed ATU Load, Purge, or Move will not have any effect on the
*		learn limit.
*		This feature is disabled when the limit is zero.
*		The following care is needed when enabling this feature:
*			1) disable learning on the ports
*			2) flush all non-static addresses in the ATU
*			3) define the desired limit for the ports
*			4) re-enable learing on the ports
*
* INPUTS:
*       port  - logical port number
*       limit - auto learning limit ( 0 ~ 255 )
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbSetPortAtuLearnLimit
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_U32   	limit
);


/*******************************************************************************
* gfdbGetPortAtuLearnCnt
*
* DESCRIPTION:
*       Read the current number of active unicast MAC addresses associated with
*		the given port. This counter (LearnCnt) is held at zero if learn limit
*		(gfdbSetPortAtuLearnLimit API) is set to zero.
*
* INPUTS:
*       port  - logical port number
*
* OUTPUTS:
*       count - current auto learning count
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetPortAtuLearnCnt
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_U32   	*count
);

/*******************************************************************************
* gfdbGetAtuAllCount
*
* DESCRIPTION:
*       Counts all entries in the Address Translation Unit.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       count - number of valid entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gfdbGetAtuAllCount
(
    IN  GT_QD_DEV 	*dev,
    OUT GT_U32 		*count
);

/*******************************************************************************
* gfdbGetAtuAllCountInDBNum
*
* DESCRIPTION:
*       Counts all entries in the defined FID (or DBNum).
*
* INPUTS:
*       dbNum - DBNum of FID
*
* OUTPUTS:
*       count - number of valid entries in FID (or DBNum).
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gfdbGetAtuAllCountInDBNum
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_U32 		dbNum,
    OUT GT_U32 		*count
);

/*******************************************************************************
* gfdbGetAtuDynamicCountInDBNum
*
* DESCRIPTION:
*       Counts all non-static entries in the defined FID (or DBNum).
*
* INPUTS:
*       dbNum - DBNum or FID
*
* OUTPUTS:
*       count - number of valid non-static entries in FID (or DBNum).
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gfdbGetAtuDynamicCountInDBNum
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_U32 		dbNum,
    OUT GT_U32 		*count
);


/* gtBrgStu.c */

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
);

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
);

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
);

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
*
*******************************************************************************/
GT_STATUS gstuFindSidEntry
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_STU_ENTRY  *stuEntry,
    OUT GT_BOOL         *found
);

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
*       None.
*
*******************************************************************************/
GT_STATUS gstuAddEntry
(
    IN  GT_QD_DEV   	*dev,
    IN  GT_STU_ENTRY	*stuEntry
);

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
*
*******************************************************************************/
GT_STATUS gstuDelEntry
(
    IN  GT_QD_DEV   	*dev,
    IN  GT_STU_ENTRY 	*stuEntry
);


/* gtCCPVT.c */

/*******************************************************************************
* gpvtInitialize
*
* DESCRIPTION:
*       This routine initializes the PVT Table to all one's (initial state)
*
* INPUTS:
*		None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpvtInitialize
(
	IN  GT_QD_DEV 	*dev
);

/*******************************************************************************
* gpvtWritePVTData
*
* DESCRIPTION:
*       This routine write Cross Chip Port Vlan Data.
*		Cross chip Port VLAN Data used as a bit mask to limit where cross chip
*		frames can egress (in chip Port VLANs are masked using gvlnSetPortVlanPorts
*		API). Cross chip frames are Forward frames that ingress a DSA or Ether
*		Type DSA port (see gprtSetFrameMode API). Bit 0 is a mask for port 0,
*		bit 1 for port 1, etc. When a port's mask bit is one, frames are allowed
*		to egress that port on this device. When a port's mask bit is zero,
*		frames are not allowed to egress that port on this device.
*
*		The Cross Chip Port VLAN Table is accessed by ingressing frames based
*		upon the original source port of the frame using the Forward frame's DSA tag
*		fields Src_Dev, Src_Port/Src_Trunk and Src_Is_Trunk. The 1 entry of the 512
*		that is accessed by the frame is:
*			If 5 Bit Port (in Global 2, offset 0x1D) = 0:
*				If Src_Is_Trunk = 0   Src_Dev[4:0], Src_Port[3:0]119
*				If Src_Is_Trunk = 1   Device Number (global offset 0x1C), Src_Trunk[3:0]
*			If 5 Bit Port (in Global 2, offset 0x1D) = 1:
*				If Src_Is_Trunk = 0   Src_Dev[3:0], Src_Port[4:0]120
*				If Src_Is_Trunk = 1   Device Number[3:0], Src_Trunk[4:0]
*
*		Cross chip port VLANs with Trunks are supported in the table where this
*		device's entries would be stored (defined by this device's Device Number).
*		This portion of the table is available for Trunk entries because this device's
*		port VLAN mappings to ports inside this device are masked by the port's
*		VLAN Table (see gvlnSetPortVlanPorts API).
*
*
* INPUTS:
*		pvtPointer - pointer to the desired entry of PVT (0 ~ 511)
*		pvtData    - Cross Chip Port Vlan Data
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpvtWritePVTData
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		pvtPointer,
	IN  GT_U32		pvtData
);

/*******************************************************************************
* gpvtReadPVTData
*
* DESCRIPTION:
*       This routine reads Cross Chip Port Vlan Data.
*		Cross chip Port VLAN Data used as a bit mask to limit where cross chip
*		frames can egress (in chip Port VLANs are masked using gvlnSetPortVlanPorts
*		API). Cross chip frames are Forward frames that ingress a DSA or Ether
*		Type DSA port (see gprtSetFrameMode API). Bit 0 is a mask for port 0,
*		bit 1 for port 1, etc. When a port's mask bit is one, frames are allowed
*		to egress that port on this device. When a port's mask bit is zero,
*		frames are not allowed to egress that port on this device.
*
*		The Cross Chip Port VLAN Table is accessed by ingressing frames based
*		upon the original source port of the frame using the Forward frame's DSA tag
*		fields Src_Dev, Src_Port/Src_Trunk and Src_Is_Trunk. The 1 entry of the 512
*		that is accessed by the frame is:
*			If 5 Bit Port (in Global 2, offset 0x1D) = 0:
*				If Src_Is_Trunk = 0   Src_Dev[4:0], Src_Port[3:0]119
*				If Src_Is_Trunk = 1   Device Number (global offset 0x1C), Src_Trunk[3:0]
*			If 5 Bit Port (in Global 2, offset 0x1D) = 1:
*				If Src_Is_Trunk = 0   Src_Dev[3:0], Src_Port[4:0]120
*				If Src_Is_Trunk = 1   Device Number[3:0], Src_Trunk[4:0]
*
*		Cross chip port VLANs with Trunks are supported in the table where this
*		device's entries would be stored (defined by this device's Device Number).
*		This portion of the table is available for Trunk entries because this device's
*		port VLAN mappings to ports inside this device are masked by the port's
*		VLAN Table (see gvlnSetPortVlanPorts API).
*
*
* INPUTS:
*		pvtPointer - pointer to the desired entry of PVT (0 ~ 511)
*
* OUTPUTS:
*		pvtData    - Cross Chip Port Vlan Data
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpvtReadPVTData
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		pvtPointer,
	OUT GT_U32		*pvtData
);


/* gtEvents.c */

/*******************************************************************************
* geventGetDevIntStatus
*
* DESCRIPTION:
* 		Check to see which device interrupts (WatchDog, JamLimit, Duplex Mismatch, and
*		SERDES Link Int) have occurred.
*
* INPUTS:
*       intType - the type of interrupt which causes an interrupt.
*				  any combination of
*					GT_DEV_INT_WATCHDOG,
*					GT_DEV_INT_JAMLIMIT,
*					GT_DEV_INT_DUPLEX_MISMATCH,
*					GT_DEV_INT_SERDES_LINK
*		port	- logical port where GT_DEV_INT_DUPLEX_MISMATCH occurred.
*				  valid only if GT_DEV_INT_DUPLEX_MISMATCH is set in intType.
*		linkInt - SERDES port list where GT_DEV_INT_SERDES_LINK interrupt is
*				  asserted. It's in vector format, Bit 10 is for port 10,
*				  Bit 9 is for port 9, etc.
*				  valid only if GT_DEV_INT_SERDES_LINK bit is set in intType.
*				  These bits are only valid of the port that is in 1000Base-X mode.
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK - on success
* 		GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventGetDevIntStatus
(
    IN  GT_QD_DEV 			*dev,
    OUT GT_DEV_INT_STATUS	*devIntStatus
);

/*******************************************************************************
* geventSetAgeOutIntEn
*
* DESCRIPTION:
*		Interrupt on Age Out. When aging is enabled, all non-static address
*		entries in the ATU's address database are periodically aged.
*		When this feature is set to GT_TRUE and an entry associated with this
*		port is aged out, an AgeOutViolation will be captured for that entry.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Out Interrupt,
*			   GT_FALUSE to disable
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventSetAgeOutIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* geventGetAgeOutIntEn
*
* DESCRIPTION:
*		Interrupt on Age Out. When aging is enabled, all non-static address
*		entries in the ATU's address database are periodically aged.
*		When this feature is set to GT_TRUE and an entry associated with this
*		port is aged out, an AgeOutViolation will be captured for that entry.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_TRUE, if Age Out Interrupt is enabled
*			   GT_FALUSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventGetAgeOutIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* geventSetOverLimitInt
*
* DESCRIPTION:
*		This routine enables/disables Over Limit Interrupt for a port.
*		If it's enabled, an ATU Miss violation will be generated when port auto
*		learn reached the limit(refer to gfdbGetPortAtuLimitReached API).
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Over Limit Interrupt,
*			   GT_FALUSE to disable
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventSetOverLimitInt
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
);

/*******************************************************************************
* geventGetOverLimitInt
*
* DESCRIPTION:
*		This routine enables/disables Over Limit Interrupt for a port.
*		If it's enabled, an ATU Miss violation will be generated when port auto
*		learn reached the limit(refer to gfdbSetPortAtuLearnLimit API).
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_TRUE to enable Over Limit Interrupt,
*			   GT_FALUSE to disable
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventGetOverLimitInt
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* geventGetPortAtuLimitReached
*
* DESCRIPTION:
*       This routine checks if learn limit has been reached.
*		When it reached, the port can no longer auto learn any more MAC addresses
*		because the address learn limit set on this port has been reached.
*
* INPUTS:
*       port  - logical port number
*
* OUTPUTS:
*       limit - GT_TRUE, if limit has been reached
*			    GT_FALSE, otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS geventGetPortAtuLimitReached
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_BOOL   	*limit
);

/*******************************************************************************
* eventSetDevInt
*
* DESCRIPTION:
*		Device Interrupt.
*		The following device interrupts are supported:
*			GT_DEV_INT_WATCHDOG	-
*				WatchDog event interrupt (WatchDog event can be configured with
*				gwdSetEvent API)
*			GT_DEV_INT_JAMLIMIT	-
*				any of the ports detect an Ingress Jam Limit violation
*				(see gprtSetPauseLimitIn API)
*			GT_DEV_INT_DUPLEX_MISMATCH -
*				any of the ports detect a duplex mismatch (i.e., the local port is
*				in half duplex mode while the link partner is in full duplex mode)
*			GT_DEV_INT_SERDES_LINK -
*				SERDES link chage interrupt.
*				An interrupt occurs when a SERDES port changes link status
*				(link up or link down)
*
*		If any of the above events is enabled, GT_DEVICE_INT interrupt will
*		be asserted by the enabled event when GT_DEV_INT is enabled with
*		eventSetActive API.
*
* INPUTS:
*		devInt - GT_DEV_INT
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS eventSetDevInt
(
	IN  GT_QD_DEV	*dev,
	IN  GT_DEV_EVENT    *devInt
);

/*******************************************************************************
* gwdSetEvent
*
* DESCRIPTION:
*		Watch Dog Event.
*		The following Watch Dog events are supported:
*			GT_WD_QC  - Queue Controller Watch Dog enable.
*						When enabled, the QC's watch dog circuit checks for link
*						list errors and any errors found in the QC.
*			GT_WD_EGRESS - Egress Watch Dog enable.
*						When enabled, each port's egress circuit checks for problems
*						between the port and the Queue Controller.
*			GT_WD_FORCE - Force a Watch Dog event.
*
*		If any of the above events is enabled, GT_DEVICE_INT interrupt will
*		be asserted by the enabled WatchDog event when GT_DEV_INT_WATCHDOG is
*		enabled with eventSetDevActive API and GT_DEV_INT is enabled with
*		eventSetActive API.
*
* INPUTS:
*		wdEvent - Watch Dog Events
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdSetEvent
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32	    wdEvent
);

/*******************************************************************************
* gwdSetSWResetOnWD
*
* DESCRIPTION:
*		SWReset on Watch Dog Event.
*		When this feature is enabled, any enabled watch dog event (gwdSetEvent API)
*		will automatically reset the switch core's datapath just as if gsysSwReset
*		API is called.
*
*		The Watch Dog History (gwdGetHistory API) won't be cleared by this
*		automatic SWReset. This allows the user to know if any watch dog event
*		ever occurred even if the swich is configured to automatically recover
*		from a watch dog.
*
*		When this feature is disabled, enabled watch dog events will not cause a
*		SWReset.
*
* INPUTS:
*		en   - GT_TRUE to enable SWReset on WD
*			   GT_FALUSE to disable
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdSetSWResetOnWD
(
	IN  GT_QD_DEV	*dev,
	IN  GT_BOOL	    en
);

/*******************************************************************************
* gwdGetSWResetOnWD
*
* DESCRIPTION:
*		SWReset on Watch Dog Event.
*		When this feature is enabled, any enabled watch dog event (gwdSetEvent API)
*		will automatically reset the switch core's datapath just as if gsysSwReset
*		API is called.
*
*		The Watch Dog History (gwdGetHistory API) won't be cleared by this
*		automatic SWReset. This allows the user to know if any watch dog event
*		ever occurred even if the swich is configured to automatically recover
*		from a watch dog.
*
*		When this feature is disabled, enabled watch dog events will not cause a
*		SWReset.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en   - GT_TRUE, if SWReset on WD is enabled
*			   GT_FALUSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetSWResetOnWD
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL	    *en
);

/*******************************************************************************
* gwdGetHistory
*
* DESCRIPTION:
*		This routine retrieves Watch Dog history. They are
*
*		wdEvent -
*			When it's set to GT_TRUE, some enabled Watch Dog event occurred.
*			The following events are possible:
*				QC WatchDog Event (GT_WD_QC)
*				Egress WatchDog Event (GT_WD_EGRESS)
*				Forced WatchDog Event (GT_WD_FORCE)
*		egressEvent -
*			If any port's egress logic detects an egress watch dog issue,
*			this field is set to GT_TRUE, regardless of the enabling GT_WD_EGRESS
*			event.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		history - GT_WD_EVENT_HISTORY structure
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetHistory
(
	IN  GT_QD_DEV			*dev,
	OUT GT_WD_EVENT_HISTORY	*history
);


/* gtPIRL2.c */

/*******************************************************************************
* gpirl2WriteResource
*
* DESCRIPTION:
*       This routine writes resource bucket parameters to the given resource
*		of the port.
*
* INPUTS:
*       port     - logical port number.
*		irlRes   - bucket to be used (0 ~ 4).
*		pirlData - PIRL resource parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirl2WriteResource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		irlRes,
	IN  GT_PIRL2_DATA	*pirlData
);

/*******************************************************************************
* gpirl2ReadResource
*
* DESCRIPTION:
*       This routine retrieves IRL Parameter.
*
* INPUTS:
*       port     - logical port number.
*		irlRes   - bucket to be used (0 ~ 4).
*
* OUTPUTS:
*		pirlData - PIRL resource parameters.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirl2ReadResource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		irlRes,
	OUT GT_PIRL2_DATA	*pirlData
);

/*******************************************************************************
* gpirl2DisableResource
*
* DESCRIPTION:
*       This routine disables Ingress Rate Limiting for the given bucket.
*
* INPUTS:
*       port     - logical port number.
*		irlRes   - bucket to be used (0 ~ 4).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirl2DisableResource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		irlRes
);


/* gtPolicy.c */

/*******************************************************************************
* gprtSetPolicy
*
* DESCRIPTION:
*       This routine sets the Policy for ports.
*		Supported Policies are defined as GT_FRAME_POLICY as follows:
*			FRAME_POLICY_NONE    - normal frame switching
*			FRAME_POLICY_MIRROR  - mirror (copy) frame to MirrorDest port
*			FRAME_POLICY_TRAP    - trap(re-direct) frame to the CPUDest port
*			FRAME_POLICY_DISCARD - discard(filter) the frame
*		Supported Policy types are defined as GT_POLICY_TYPE:
*			POLICY_TYPE_DA - DA Policy Mapping
*				DA Policy Mapping occurs when the DA of a frame is contained in
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_SA - SA Policy Mapping
*				SA Policy Mapping occurs when the SA of a frame is contained in
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_VTU - VTU Policy Mapping
*				VTU Policy Mapping occurs when the VID of a frame is contained in
*				the VTU database with the VidPolicy is enabled.
*			POLICY_TYPE_ETYPE - EtherType Policy Mapping
*				EType Policy Mapping occurs when the EtherType of a frame matches
*				the PortEType (see gprtSetPortEType API)
*			POLICY_TYPE_PPPoE - PPPoE Policy Mapping
*				PPPoE Policy Mapping occurs when the EtherType of a frame matches 0x8863
*			POLICY_TYPE_VBAS - VBAS Policy Mapping
*				VBAS Policy Mapping occurs when the EtherType of a frame matches 0x8200
*			POLICY_TYPE_OPT82 - DHCP Option 82 Policy Mapping
*				DHCP Option 82 Policy Mapping occurs when the ingressing frame is an
*				IPv4 UDP with a UDP Destination port = 0x0043 or 0x0044, or an
*				IPv6 UDP with a UDP Destination port = 0x0223 or 0x0222
*			POLICY_TYPE_UDP - UDP Policy Mapping
*				UDP Policy Mapping occurs when the ingressing frame is
*				a Broadcast IPv4 UDP or a Multicast IPv6 UDP.
*
* INPUTS:
*       port	- logical port number.
*       type 	- policy type (GT_POLICY_TYPE)
*       policy 	- policy (GT_FRAME_POLICY)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_BAD_PARAM     - on bad parameters
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPolicy
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT 	port,
    IN  GT_POLICY_TYPE	type,
	IN	GT_FRAME_POLICY	policy
);

/*******************************************************************************
* gprtGetPolicy
*
* DESCRIPTION:
*       This routine gets the Policy of the given policy type.
*		Supported Policies are defined as GT_FRAME_POLICY as follows:
*			FRAME_POLICY_NONE    - normal frame switching
*			FRAME_POLICY_MIRROR  - mirror (copy) frame to MirrorDest port
*			FRAME_POLICY_TRAP    - trap(re-direct) frame to the CPUDest port
*			FRAME_POLICY_DISCARD - discard(filter) the frame
*		Supported Policy types are defined as GT_POLICY_TYPE:
*			POLICY_TYPE_DA - DA Policy Mapping
*				DA Policy Mapping occurs when the DA of a frame is contained in
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_SA - SA Policy Mapping
*				SA Policy Mapping occurs when the SA of a frame is contained in
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_VTU - VTU Policy Mapping
*				VTU Policy Mapping occurs when the VID of a frame is contained in
*				the VTU database with the VidPolicy is enabled.
*			POLICY_TYPE_ETYPE - EtherType Policy Mapping
*				EType Policy Mapping occurs when the EtherType of a frame matches
*				the PortEType (see gprtSetPortEType API)
*			POLICY_TYPE_PPPoE - PPPoE Policy Mapping
*				PPPoE Policy Mapping occurs when the EtherType of a frame matches 0x8863
*			POLICY_TYPE_VBAS - VBAS Policy Mapping
*				VBAS Policy Mapping occurs when the EtherType of a frame matches 0x8200
*			POLICY_TYPE_OPT82 - DHCP Option 82 Policy Mapping
*				DHCP Option 82 Policy Mapping occurs when the ingressing frame is an
*				IPv4 UDP with a UDP Destination port = 0x0043 or 0x0044, or an
*				IPv6 UDP with a UDP Destination port = 0x0223 or 0x0222
*			POLICY_TYPE_UDP - UDP Policy Mapping
*				UDP Policy Mapping occurs when the ingressing frame is
*				a Broadcast IPv4 UDP or a Multicast IPv6 UDP.
*
* INPUTS:
*       port	- logical port number.
*       type 	- policy type (GT_POLICY_TYPE)
*
* OUTPUTS:
*       policy 	- policy (GT_FRAME_POLICY)
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_BAD_PARAM     - on bad parameters
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPolicy
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT 	port,
    IN  GT_POLICY_TYPE	type,
	OUT GT_FRAME_POLICY	*policy
);


/* gtPortCtrl.c */

/*******************************************************************************
* gprtSetPauseLimitOut
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be transmitted
*		from this port. When full duplex Flow Control is enabled on this port,
*		these bits are used to limit the number of Pause refresh frames that can
*		be generated from this port to keep this port's link partner from sending
*		any data.
*		Setting this value to 0 will allow continuous Pause frame refreshes to
*		egress this port as long as this port remains congested.
*		Setting this value to 1 will allow 1 Pause frame to egress from this port
*		for each congestion situation.
*		Setting this value to 2 will allow 2 Pause frames to egress from this port
*		for each congestion situation, etc.
*
* INPUTS:
*		port - the logical port number
*		limit - the max number of Pause refresh frames for each congestion situation
*				( 0 ~ 0xFF)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPauseLimitOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_U16		limit
);

/*******************************************************************************
* gprtGetPauseLimitOut
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be transmitted
*		from this port. When full duplex Flow Control is enabled on this port,
*		these bits are used to limit the number of Pause refresh frames that can
*		be generated from this port to keep this port's link partner from sending
*		any data.
*		Setting this value to 0 will allow continuous Pause frame refreshes to
*		egress this port as long as this port remains congested.
*		Setting this value to 1 will allow 1 Pause frame to egress from this port
*		for each congestion situation.
*		Setting this value to 2 will allow 2 Pause frames to egress from this port
*		for each congestion situation, etc.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		limit - the max number of Pause refresh frames for each congestion situation
*				( 0 ~ 0xFF)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPauseLimitOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*limit
);

/*******************************************************************************
* gprtSetPauseLimitIn
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be received
*		on this port. When a port has flow Control enabled, this value can be
*		used to limit how long this port can be Paused off to prevent a port stall
*		through jamming.
*		When this value is in the range of 0x01 to 0xFF, and a frame is ready to
*		be transmitted out this port, but it cannot be transmitted due to the port
*		being jammed, this limit mechanism starts. The limit mechanism starts
*		counting new Pause refresh frames or counts of 16 consecutive collisions.
*		If the counter reaches the value set through this API, the following event
*		will occur:
*			1) Port's ForceFC is enabled,
*			2) Port's FCValue is cleared to a zero, and
*			3) Jam Limit Interrupt is asserted.
*		This effectively disables Flow Control on the port once the Pause timer
*		expires. If a frame gets transmitted out this port before the counter
*		reaches this limit, then this limit mechanism counter resets back to zero.
*
*		Setting this value to 0 will allow continuous jamming to be received on
*		this port without the Port's ForceFC and FCValue getting modified.
*
*		The modification of Port's ForceFC and FCValue is the only indication that
*		the limit was reached on this port.
*
* INPUTS:
*		port - the logical port number
*		limit - the max number of continuous Pause refresh frames for each trasmition
*				( 0 ~ 0xFF)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPauseLimitIn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_U16		limit
);

/*******************************************************************************
* gprtGetPauseLimitIn
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be received
*		on this port. When a port has flow Control enabled, this value can be
*		used to limit how long this port can be Paused off to prevent a port stall
*		through jamming.
*		When this value is in the range of 0x01 to 0xFF, and a frame is ready to
*		be transmitted out this port, but it cannot be transmitted due to the port
*		being jammed, this limit mechanism starts. The limit mechanism starts
*		counting new Pause refresh frames or counts of 16 consecutive collisions.
*		If the counter reaches the value set through this API, the following event
*		will occur:
*			1) Port's ForceFC is enabled,
*			2) Port's FCValue is cleared to a zero, and
*			3) Jam Limit Interrupt is asserted.
*		This effectively disables Flow Control on the port once the Pause timer
*		expires. If a frame gets transmitted out this port before the counter
*		reaches this limit, then this limit mechanism counter resets back to zero.
*
*		Setting this value to 0 will allow continuous jamming to be received on
*		this port without the Port's ForceFC and FCValue getting modified.
*
*		The modification of Port's ForceFC and FCValue is the only indication that
*		the limit was reached on this port.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		limit - the max number of continuous Pause refresh frames for each trasmition
*				( 0 ~ 0xFF)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPauseLimitIn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*limit
);

/*******************************************************************************
* gprtSetFrameMode
*
* DESCRIPTION:
*		Frmae Mode is used to define the expected Ingress and the generated Egress
*		tagging frame format for this port as follows:
*			GT_FRAME_MODE_NORMAL -
*				Normal Network mode uses industry standard IEEE 802.3ac Tagged or
*				Untagged frames. Tagged frames use an Ether Type of 0x8100.
*			GT_FRAME_MODE_DSA -
*				DSA mode uses a Marvell defined tagged frame format for
*				Chip-to-Chip and Chip-to-CPU connections.
*			GT_FRAME_MODE_PROVIDER -
*				Provider mode uses user definable Ether Types per port
*				(see gprtSetPortEType/gprtGetPortEType API).
*			GT_FRAME_MODE_ETHER_TYPE_DSA -
*				Ether Type DSA mode uses standard Marvell DSA Tagged frame info
*				flowing a user definable Ether Type. This mode allows the mixture
*				of Normal Network frames with DSA Tagged frames and is useful to
*				be used on ports that connect to a CPU.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_FRAME_MODE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is unknown
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetFrameMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_FRAME_MODE	mode
);

/*******************************************************************************
* gprtGetFrameMode
*
* DESCRIPTION:
*		Frmae Mode is used to define the expected Ingress and the generated Egress
*		tagging frame format for this port as follows:
*			GT_FRAME_MODE_NORMAL -
*				Normal Network mode uses industry standard IEEE 802.3ac Tagged or
*				Untagged frames. Tagged frames use an Ether Type of 0x8100.
*			GT_FRAME_MODE_DSA -
*				DSA mode uses a Marvell defined tagged frame format for
*				Chip-to-Chip and Chip-to-CPU connections.
*			GT_FRAME_MODE_PROVIDER -
*				Provider mode uses user definable Ether Types per port
*				(see gprtSetPortEType/gprtGetPortEType API).
*			GT_FRAME_MODE_ETHER_TYPE_DSA -
*				Ether Type DSA mode uses standard Marvell DSA Tagged frame info
*				flowing a user definable Ether Type. This mode allows the mixture
*				of Normal Network frames with DSA Tagged frames and is useful to
*				be used on ports that connect to a CPU.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_FRAME_MODE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetFrameMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_FRAME_MODE	*mode
);

/*******************************************************************************
* gprtSetHoldAt1
*
* DESCRIPTION:
*		Hold Aging ATU Entries at an Entry State value of 1. When this feature
*		is set to GT_TRUE, ATU entries associated with this port will age down
*		to an Entry State of 0x1, but will not go to 0x0 (0x0 would purge the
*		entry)
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to hold aging ATU entry with Entry State of 1,
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetHoldAt1
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetHoldAt1
*
* DESCRIPTION:
*		Hold Aging ATU Entries at an Entry State value of 1. When this feature
*		is set to GT_TRUE, ATU entries associated with this port will age down
*		to an Entry State of 0x1, but will not go to 0x0 (0x0 would purge the
*		entry)
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to hold aging ATU entry with Entry State of 1,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetHoldAt1
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
);

/*******************************************************************************
* gprtSetRefreshLocked
*
* DESCRIPTION:
*		Auto Refresh known addresses when port is Locked. Already known addresses
*		will be auto refreshed when this feature is enabled. When this feature
*		is disabled, auto refreshing will not occur on Locked ports.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable Auto Refresh known addresses on locked port
*			   GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetRefreshLocked
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
);

/*******************************************************************************
* gprtGetRefreshLocked
*
* DESCRIPTION:
*		Auto Refresh known addresses when port is Locked. Already known addresses
*		will be auto refreshed when this feature is enabled. When this feature
*		is disabled, auto refreshing will not occur on Locked ports.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable Auto Refresh known addresses on locked port
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetRefreshLocked
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
);

/*******************************************************************************
* gprtSetPortEType
*
* DESCRIPTION:
*		This routine sets the port's special Ether Type. This Ether Type is used
*		for Policy (see gprtSetPolicy API) and FrameMode (see gprtSetFrameMode API).
*
* INPUTS:
*		port  - the logical port number
*		etype - port's special ether type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPortEType
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_ETYPE		etype
);

/*******************************************************************************
* gprtGetPortEType
*
* DESCRIPTION:
*		This routine retrieves the port's special Ether Type. This Ether Type is used
*		for Policy (see gprtSetPolicy API) and FrameMode (see gprtSetFrameMode API).
*
* INPUTS:
*		port  - the logical port number
*
* OUTPUTS:
*		etype - port's special ether type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPortEType
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_ETYPE	*etype
);


/* gtPortRateCtr.c */

/*******************************************************************************
* grcSetFrameOverhead
*
* DESCRIPTION:
*       Egress rate frame overhead adjustment.
*		This field is used to adjust the number of bytes that need to be added to a
*		frame's IFG on a per frame basis.
*
*		The egress rate limiter multiplies the value programmed in this field by four
*		for computing the frame byte offset adjustment value (i.e., the amount the
*		IPG is increased for every frame). This adjustment, if enabled, is made to
*		every egressing frame's IPG and it is made in addition to any other IPG
*		adjustments due to other Egress Rate Control settings.
*
*		The egress overhead adjustment can add the following number of byte times
*		to each frame's IPG: 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52,
*		56 and 60.
*
*		Example:
*		If FrameOverhead = 11, the egress rate limiter would increase the IPG
*		between every frame by an additional 44 bytes.
*
*		Note: When the Count Mode (port offset 0x0A) is in Frame based egress rate
*		shaping mode, these Frame Overhead bits must be 0x0.
*
* INPUTS:
*       port	 - logical port number.
*       overhead - Frame overhead (0 ~ 15)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetFrameOverhead
(
    IN GT_QD_DEV		*dev,
    IN GT_LPORT			port,
    IN GT_32			overhead
);

/*******************************************************************************
* grcGetFrameOverhead
*
* DESCRIPTION:
*       Egress rate frame overhead adjustment.
*		This field is used to adjust the number of bytes that need to be added to a
*		frame's IFG on a per frame basis.
*
*		The egress rate limiter multiplies the value programmed in this field by four
*		for computing the frame byte offset adjustment value (i.e., the amount the
*		IPG is increased for every frame). This adjustment, if enabled, is made to
*		every egressing frame's IPG and it is made in addition to any other IPG
*		adjustments due to other Egress Rate Control settings.
*
*		The egress overhead adjustment can add the following number of byte times
*		to each frame's IPG: 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52,
*		56 and 60.
*
*		Example:
*		If FrameOverhead = 11, the egress rate limiter would increase the IPG
*		between every frame by an additional 44 bytes.
*
*		Note: When the Count Mode (port offset 0x0A) is in Frame based egress rate
*		shaping mode, these Frame Overhead bits must be 0x0.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       overhead - Frame overhead (0 ~ 15)
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
*******************************************************************************/
GT_STATUS grcGetFrameOverhead
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_32    *overhead
);


/* gtPortStatus.c */

/*******************************************************************************
* gprtGetBufHigh
*
* DESCRIPTION:
*		Output from QC telling the MAC that it should perform Flow Control.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		bufHigh - GT_TRUE, if Flow control required
*				  GT_FALSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetBufHigh
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*bufHigh
);

/*******************************************************************************
* gprtGetFcEn
*
* DESCRIPTION:
*		Input into the QC telling it that Flow Control is enabled on this port.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		fcEn - GT_TRUE, if Flow control is enabled
*			   GT_FALSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetFcEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*fcEn
);

/*******************************************************************************
* gprtGetRsvSize
*
* DESCRIPTION:
*		This routine gets Ingress reserved queue size counter.
*		This counter reflects the current number of reserved ingress buffers
*		assigned to this port.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		count - reserved ingress queue size counter value
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetRsvSize
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*count
);


/* gtPriTable.c */

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
);

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
);


/* gtSysCtrl.c */

/*******************************************************************************
* gsysSetCPUDest
*
* DESCRIPTION:
*		This routine sets CPU Destination Port. CPU Destination port indicates the
*		port number on this device where the CPU is connected (either directly or
*		indirectly through another Marvell switch device).
*
*		Many modes of frame processing need to know where the CPU is located.
*		These modes are:
*		1. When IGMP/MLD frame is received and Snooping is enabled
*		2. When the port is configured as a DSA port and it receives a To_CPU frame
*		3. When a Rsvd2CPU frame enters the port
*		4. When the port's SA Filtering mode is Drop to CPU
*		5. When any of the port's Policy Options trap the frame to the CPU
*		6. When the ingressing frame is an ARP and ARP mirroring is enabled in the
*		   device
*
*		In all cases, except for ARP, the frames that meet the enabled criteria
*		are mapped to the CPU Destination port, overriding where the frame would
*		normally go. In the case of ARP, the frame will be mapped normally and it
*		will also get copied to this port.
*		Frames that filtered or discarded will not be mapped to the CPU Destination
*		port with the exception of the Rsvd2CPU and DSA Tag cases.
*
*		If CPUDest = 0xF, the remapped frames will be discarded, no ARP mirroring
*		will occur and ingressing To_CPU frames will be discarded.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetCPUDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
);

/*******************************************************************************
* gsysGetCPUDest
*
* DESCRIPTION:
*		This routine gets CPU Destination Port. CPU Destination port indicates the
*		port number on this device where the CPU is connected (either directly or
*		indirectly through another Marvell switch device).
*
*		Many modes of frame processing need to know where the CPU is located.
*		These modes are:
*		1. When IGMP/MLD frame is received and Snooping is enabled
*		2. When the port is configured as a DSA port and it receives a To_CPU frame
*		3. When a Rsvd2CPU frame enters the port
*		4. When the port's SA Filtering mode is Drop to CPU
*		5. When any of the port's Policy Options trap the frame to the CPU
*		6. When the ingressing frame is an ARP and ARP mirroring is enabled in the
*		   device
*
*		In all cases, except for ARP, the frames that meet the enabled criteria
*		are mapped to the CPU Destination port, overriding where the frame would
*		normally go. In the case of ARP, the frame will be mapped normally and it
*		will also get copied to this port.
*		Frames that filtered or discarded will not be mapped to the CPU Destination
*		port with the exception of the Rsvd2CPU and DSA Tag cases.
*
*		If CPUDest = 0xF, the remapped frames will be discarded, no ARP mirroring
*		will occur and ingressing To_CPU frames will be discarded.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetCPUDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
);

/*******************************************************************************
* gsysSetMirrorDest
*
* DESCRIPTION:
*		This routine sets Mirror Destination Port. Frames that ingress a port
*		that trigger a policy mirror are mapped (copied) to this port as long as
*		the frame is not filtered or discarded.
*		The Mirror Destination port should point to the port that directs these
*		frames to the CPU that will process these frames. This target port should
*		be a DSA Tag port so the frames will egress with a To_CPU DSA Tag with a
*		CPU Code of Policy Mirror.
*		To_CPU DSA Tag frames with a CPU Code of Policy Mirror that ingress a DSA
*		Tag port will be sent to the port number defined in MirrorDest.
*
*		If MirrorDest = 0xF, Policy Mirroring is disabled and ingressing To_CPU
*		Policy Mirror frames will be discarded.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetMirrorDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
);

/*******************************************************************************
* gsysGetMirrorDest
*
* DESCRIPTION:
*		This routine gets Mirror Destination Port. Frames that ingress a port
*		that trigger a policy mirror are mapped (copied) to this port as long as
*		the frame is not filtered or discarded.
*		The Mirror Destination port should point to the port that directs these
*		frames to the CPU that will process these frames. This target port should
*		be a DSA Tag port so the frames will egress with a To_CPU DSA Tag with a
*		CPU Code of Policy Mirror.
*		To_CPU DSA Tag frames with a CPU Code of Policy Mirror that ingress a DSA
*		Tag port will be sent to the port number defined in MirrorDest.
*
*		If MirrorDest = 0xF, Policy Mirroring is disabled and ingressing To_CPU
*		Policy Mirror frames will be discarded.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetMirrorDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
);

/*******************************************************************************
* gsysSetRMPort
*
* DESCRIPTION:
*		Remote Management feature is enabled only on one port. Since not all ports
*		can be enabled for Remote Management feature, please refer to the device
*		datasheet for detailed information.
*		For example, 88E6097 device allows logical port 9 or 10, and 88E6047
*		device allows logical port 4 and 5.
*
* INPUTS:
*		port - Remote Management Port
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM     - on unallowable port
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRMPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port
);

/*******************************************************************************
* gsysGetRMPort
*
* DESCRIPTION:
*		Remote Management feature is enabled only on one port. Since not all ports
*		can be enabled for Remote Management feature, please refer to the device
*		datasheet for detailed information.
*		For example, 88E6097 device allows logical port 9 or 10, and 88E6047
*		device allows logical port 4 and 5.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port - Remote Management Port
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRMPort
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT 	*port
);

/*******************************************************************************
* gsysSetRMDACheck
*
* DESCRIPTION:
*		Check the DA on Remote Management frames.
*		When DA Check is enabled, the DA of Remote Management frames must be
*		contained in this device's address database (ATU) as a Static entry
*		(either unicast or multicast). If the DA of the frame is not contained
*		in this device's address database, the frame will be not be processed as
*		a Frame-to-Regter frame.
*		When DA Check is disabled, the DA of Remote Management frames is not
*		validated before processing the frame.
*
* INPUTS:
*		en - GT_TRUE to enable DA Check,
*			 GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRMDACheck
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL 		en
);

/*******************************************************************************
* gsysGetRMDACheck
*
* DESCRIPTION:
*		Check the DA on Remote Management frames.
*		When DA Check is enabled, the DA of Remote Management frames must be
*		contained in this device's address database (ATU) as a Static entry
*		(either unicast or multicast). If the DA of the frame is not contained
*		in this device's address database, the frame will be not be processed as
*		a Frame-to-Regter frame.
*		When DA Check is disabled, the DA of Remote Management frames is not
*		validated before processing the frame.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if DA Check is enabled,
*			 GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRMDACheck
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL 	*en
);

/*******************************************************************************
* gsysSetRMEnable
*
* DESCRIPTION:
*		Enable or disable Remote Management feature. This feature can be enabled
*		only on one port (see gsysSetRMPort API).
*
* INPUTS:
*		en - GT_TRUE to enable Remote Management feature,
*			 GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRMEnable
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL 		en
);

/*******************************************************************************
* gsysGetRMEnable
*
* DESCRIPTION:
*		Enable or disable Remote Management feature. This feature can be enabled
*		only on one port (see gsysSetRMPort API).
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Remote Management feature is enabled,
*			 GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRMEnable
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL 	*en
);

/*******************************************************************************
* gsysSetRsvd2CpuEnables2X
*
* DESCRIPTION:
*		Reserved DA Enables for the form of 01:80:C2:00:00:2x.
*		When the Rsvd2Cpu(gsysSetRsvd2Cpu) is set to a one, the 16 reserved
*		multicast DA addresses, whose bit in this register are also set to a one,
*		are treadted as MGMT frames. All the reserved DA's take the form
*		01:80:C2:00:00:2x. When x = 0x0, bit 0 of this register is tested.
*		When x = 0x2, bit 2 of this field is tested and so on.
*		If the tested bit in this register is cleared to a zero, the frame will
*		be treated as a normal (non-MGMT) frame.
*
* INPUTS:
*		enBits - bit vector of enabled Reserved Multicast.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRsvd2CpuEnables2X
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		enBits
);

/*******************************************************************************
* gsysGetRsvd2CpuEnables2X
*
* DESCRIPTION:
*		Reserved DA Enables for the form of 01:80:C2:00:00:2x.
*		When the Rsvd2Cpu(gsysSetRsvd2Cpu) is set to a one, the 16 reserved
*		multicast DA addresses, whose bit in this register are also set to a one,
*		are treadted as MGMT frames. All the reserved DA's take the form
*		01:80:C2:00:00:2x. When x = 0x0, bit 0 of this register is tested.
*		When x = 0x2, bit 2 of this field is tested and so on.
*		If the tested bit in this register is cleared to a zero, the frame will
*		be treated as a normal (non-MGMT) frame.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		enBits - bit vector of enabled Reserved Multicast.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRsvd2CpuEnables2X
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*enBits
);

/*******************************************************************************
* gsysSetFloodBC
*
* DESCRIPTION:
*		Flood Broadcast.
*		When Flood Broadcast is enabled, frames with the Broadcast destination
*		address will flood out all the ports regardless of the setting of the
*		port's Egress Floods mode (see gprtSetEgressFlood API). VLAN rules and
*		other switch policy still applies to these Broadcast frames.
*		When this feature is disabled, frames with the Broadcast destination
*		address are considered Multicast frames and will be affected by port's
*		Egress Floods mode.
*
* INPUTS:
*		en - GT_TRUE to enable Flood Broadcast, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetFloodBC
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetFloodBC
*
* DESCRIPTION:
*		Flood Broadcast.
*		When Flood Broadcast is enabled, frames with the Broadcast destination
*		address will flood out all the ports regardless of the setting of the
*		port's Egress Floods mode (see gprtSetEgressFlood API). VLAN rules and
*		other switch policy still applies to these Broadcast frames.
*		When this feature is disabled, frames with the Broadcast destination
*		address are considered Multicast frames and will be affected by port's
*		Egress Floods mode.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Flood Broadcast is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetFloodBC
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetRemove1PTag
*
* DESCRIPTION:
*		Remove One Provider Tag.
*		When this feature is enabled and a port is configured as a Provider Port
*		(see gprtSetFrameMode API), recursive Provider Tag stripping will NOT be
*		performed. Only the first Provider Tag found on the frame will be
*		extracted and removed. Its extracted data will be used for switching.
*		When it's disabled and a port is configured as a Provider Port, recursive
*		Provider Tag stripping will be performed. The first Provider Tag's data
*		will be extracted and used for switching, and then all subsequent Provider
*		Tags found in the frame will also be removed. This will only occur if the
*		port's PortEType (see gprtSetPortEType API) is not 0x8100.
*
* INPUTS:
*		en - GT_TRUE to enable Remove One Provider Tag, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRemove1PTag
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetRemove1PTag
*
* DESCRIPTION:
*		Remove One Provider Tag.
*		When this feature is enabled and a port is configured as a Provider Port
*		(see gprtSetFrameMode API), recursive Provider Tag stripping will NOT be
*		performed. Only the first Provider Tag found on the frame will be
*		extracted and removed. Its extracted data will be used for switching.
*		When it's disabled and a port is configured as a Provider Port, recursive
*		Provider Tag stripping will be performed. The first Provider Tag's data
*		will be extracted and used for switching, and then all subsequent Provider
*		Tags found in the frame will also be removed. This will only occur if the
*		port's PortEType (see gprtSetPortEType API) is not 0x8100.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Remove One Provider Tag is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRemove1PTag
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL		*en
);

/*******************************************************************************
* gsysSetTagFlowControl
*
* DESCRIPTION:
*		Use and generate source port Flow Control status for Cross-Chip Flow
*		Control.
*		When this feature is enabled, bit 17 of the DSA Tag Forward frames is
*		defined to be Src_FC and it is added to these frames when generated and
*		it is inspected on these frames when received. The QC will use the Src_FC
*		bit on DSA ports instead of the DSA port's Flow Control mode bit for the
*		QC Flow Control algorithm.
*		When it is disabled, bit 17 of the DSA Tag Forward frames is defined to
*		be Reserved and it will be zero on these frames when generated and it
*		will not be used on these frames when received (this is a backwards
*		compatibility mode).
*
* INPUTS:
*		en - GT_TRUE to enable Tag Flow Control, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetTagFlowControl
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetTagFlowControl
*
* DESCRIPTION:
*		Use and generate source port Flow Control status for Cross-Chip Flow
*		Control.
*		When this feature is enabled, bit 17 of the DSA Tag Forward frames is
*		defined to be Src_FC and it is added to these frames when generated and
*		it is inspected on these frames when received. The QC will use the Src_FC
*		bit on DSA ports instead of the DSA port's Flow Control mode bit for the
*		QC Flow Control algorithm.
*		When it is disabled, bit 17 of the DSA Tag Forward frames is defined to
*		be Reserved and it will be zero on these frames when generated and it
*		will not be used on these frames when received (this is a backwards
*		compatibility mode).
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Tag Flow Control is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetTagFlowControl
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetAlwaysUseVTU
*
* DESCRIPTION:
*		Always use VTU.
*		When this feature is enabled, VTU hit data will be used to map frames
*		even if 802.1Q is Disabled on the port.
*		When it's disabled, data will be ignored when mapping frames on ports
*		where 802.1Q is Disabled.
*
* INPUTS:
*		en - GT_TRUE to use VTU always, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetAlwaysUseVTU
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetAlwaysUseVTU
*
* DESCRIPTION:
*		Always use VTU.
*		When this feature is enabled, VTU hit data will be used to map frames
*		even if 802.1Q is Disabled on the port.
*		When it's disabled, data will be ignored when mapping frames on ports
*		where 802.1Q is Disabled.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if VTU is always used, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetAlwaysUseVTU
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetQVlansOnly
*
* DESCRIPTION:
*		802.1Q VLANs Only.
*		When this feature is disabled, the egress mapping of the frame is
*		limited by the frame's VID (using the MemberTag data found in the VTU)
*		together with the port based VLANs (using the source port's PortVLANTable,
*		gvlnSetPortVlanPorts API). The two methods are always used together in
*		this mode.
*		When this feature is enabled, the egress mapping of the frame is limitied
*		by the frame's VID only, if the VID was found in the VTU. If the frame's
*		VID was not found in the VTU the egress mapping of the frame is limited
*		by the source port's PortVLANTable only. The two methods are never
*		used together in this mode.
*
* INPUTS:
*		en - GT_TRUE to use 802.1Q Vlan Only feature, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetQVlansOnly
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGetQVlansOnly
*
* DESCRIPTION:
*		802.1Q VLANs Only.
*		When this feature is disabled, the egress mapping of the frame is
*		limited by the frame's VID (using the MemberTag data found in the VTU)
*		together with the port based VLANs (using the source port's PortVLANTable,
*		gvlnSetPortVlanPorts API). The two methods are always used together in
*		this mode.
*		When this feature is enabled, the egress mapping of the frame is limitied
*		by the frame's VID only, if the VID was found in the VTU. If the frame's
*		VID was not found in the VTU the egress mapping of the frame is limited
*		by the source port's PortVLANTable only. The two methods are never
*		used together in this mode.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if 802.1Q Vlan Only feature is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetQVlansOnly
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSet5BitPort
*
* DESCRIPTION:
*		Use 5 bits for Port data in the Port VLAN Table (PVT).
*		When this feature is enabled, the 9 bits used to access the PVT memory is:
*			Addr[8:5] = Source Device[3:0] or Device Number[3:0]
*			Addr[4:0] = Source Port/Trunk[4:0]
*		When it's disabled, the 9 bits used to access the PVT memory is:
*			Addr[8:4] = Source Device[4:0] or Device Number[4:0]
*			Addr[3:0] = Source Port/Trunk[3:0]
*
* INPUTS:
*		en - GT_TRUE to use 5 bit as a Source port in PVT, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSet5BitPort
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
);

/*******************************************************************************
* gsysGet5BitPort
*
* DESCRIPTION:
*		Use 5 bits for Port data in the Port VLAN Table (PVT).
*		When this feature is enabled, the 9 bits used to access the PVT memory is:
*			Addr[8:5] = Source Device[3:0] or Device Number[3:0]
*			Addr[4:0] = Source Port/Trunk[4:0]
*		When it's disabled, the 9 bits used to access the PVT memory is:
*			Addr[8:4] = Source Device[4:0] or Device Number[4:0]
*			Addr[3:0] = Source Port/Trunk[3:0]
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if 5 bit is used as a Source Port in PVT, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGet5BitPort
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
);

/*******************************************************************************
* gsysSetSDETPolarity
*
* DESCRIPTION:
*		SDET (Signal Detect) Polarity select bits for each port.
*		Bit 10 is for Port 10, bit 9 is for Port 9, etc. SDET is used to help
*		determine link on fiber ports. This bit affects the active level of a
*		port's SDET pins as follows:
*			0 = SDET is active low. A low level on the port's SDET pin is
*				required for link to occur.
*			1 = SDET is active high. A high level on the port’s SDET pin is
*				required for link to occur.
*		SDET is used when the port is configured as a fiber port. In all other
*		port modes the SDET pins are ignored and these bits have no effect.
*
* INPUTS:
*		sdetVec - SDET Polarity for each port in Vector format
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sdetVec is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetSDETPolarity
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		sdetVec
);

/*******************************************************************************
* gsysSetSDETPolarity
*
* DESCRIPTION:
*		SDET (Signal Detect) Polarity select bits for each port.
*		Bit 10 is for Port 10, bit 9 is for Port 9, etc. SDET is used to help
*		determine link on fiber ports. This bit affects the active level of a
*		port's SDET pins as follows:
*			0 = SDET is active low. A low level on the port's SDET pin is
*				required for link to occur.
*			1 = SDET is active high. A high level on the port’s SDET pin is
*				required for link to occur.
*		SDET is used when the port is configured as a fiber port. In all other
*		port modes the SDET pins are ignored and these bits have no effect.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		sdetVec - SDET Polarity for each port in Vector format
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetSDETPolarity
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U32  	*sdetVec
);


/* gtBrgVlan.c for 2.6 release */

/*******************************************************************************
* gvlnSetNoEgrPolicy
*
* DESCRIPTION:
*		No Egress Policy. When this bit is set to a one Egress 802.1Q Secure and
*		Check discards are not performed. This mode allowsa non-802.1Q enabled
*		port to send a frame to an 802.1Q enabled port that is configured in the
*		Secure or Check 802.1Q mode. In this situation the frames will egress
*		even if the VID assigned to the frame is not found in the VTU.
*
* INPUTS:
*		mode - no egress policy mode
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gvlnSetNoEgrPolicy
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		mode
);


/*******************************************************************************
* gvlnGetNoEgrPolicy
*
* DESCRIPTION:
*		No Egress Policy. When this bit is set to a one Egress 802.1Q Secure and
*		Check discards are not performed. This mode allowsa non-802.1Q enabled
*		port to send a frame to an 802.1Q enabled port that is configured in the
*		Secure or Check 802.1Q mode. In this situation the frames will egress
*		even if the VID assigned to the frame is not found in the VTU.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - no egress policy mode
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gvlnGetNoEgrPolicy
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL		*mode
);

/*******************************************************************************
* gwdSetRMUTimeOut
*
* DESCRIPTION:
*		Remote Management Timeout. When this bit is set to a one the Remote
*		Management Unit(RMU) will timeout on Wait on Bit commands. If the bit that
*		is being tested has not gone to the specified value after 1 sec. has elapsed
*		the Wait on Bit command will be terminated and the Response frame will be
*		sent without any further processing.
*
*		When this bit is cleared to a zero the Wait on Bit command will wait
*		until the bit that is being tested has changed to the specified value.
*
* INPUTS:
*		en   - GT_TRUE to enable RMU Timeout
*			   GT_FALUSE to disable
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdSetRMUTimeOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_BOOL	    en
);

/*******************************************************************************
* gwdGetRMUTimeOut
*
* DESCRIPTION:
*		Remote Management Timeout. When this bit is set to a one the Remote
*		Management Unit(RMU) will timeout on Wait on Bit commands. If the bit that
*		is being tested has not gone to the specified value after 1 sec. has elapsed
*		the Wait on Bit command will be terminated and the Response frame will be
*		sent without any further processing.
*
*		When this bit is cleared to a zero the Wait on Bit command will wait
*		until the bit that is being tested has changed to the specified value.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en   - GT_TRUE to enable RMU Timeout
*			   GT_FALUSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetRMUTimeOut
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL	    *en
);

/*******************************************************************************
* gwdGetEgressWDEvent
*
* DESCRIPTION:
*		If any port's egress logic detects an egress watch dog issue, this bit
*		will be set to a one, regardless of the setting of the GT_WD_EGRESS in
*		gwdSetEvent function.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		event - GT_TRUE, if egress logic has detected any egress watch dog issue
*			    GT_FALUSE, otherwise
*
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetEgressWDEvent
(
	IN  GT_QD_DEV		*dev,
	OUT GT_BOOL			*event
);


/*******************************************************************************
* gsysSetQoSWeight
*
* DESCRIPTION:
*       Programmable Round Robin Weights.
*		Each port has 4 output Queues. Queue 3 has the highest priority and
*		Queue 0 has the lowest priority. When a scheduling mode of port is
*		configured as Weighted Round Robin queuing mode, the access sequece of the
*		Queue is 3,2,3,1,3,2,3,0,3,2,3,1,3,2,3 by default.
*		This sequence can be configured with this API.
*
* INPUTS:
*       weight - access sequence of the queue
*
* OUTPUTS:
*       None.
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
GT_STATUS gsysSetQoSWeight
(
    IN  GT_QD_DEV 		*dev,
    IN  GT_QoS_WEIGHT	*weight
);

/*******************************************************************************
* gsysGetQoSWeight
*
* DESCRIPTION:
*       Programmable Round Robin Weights.
*		Each port has 4 output Queues. Queue 3 has the highest priority and
*		Queue 0 has the lowest priority. When a scheduling mode of port is
*		configured as Weighted Round Robin queuing mode, the access sequece of the
*		Queue is 3,2,3,1,3,2,3,0,3,2,3,1,3,2,3 by default.
*		This routine retrieves the access sequence of the Queue.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       weight - access sequence of the queue
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
GT_STATUS gsysGetQoSWeight
(
    IN  GT_QD_DEV 		*dev,
    OUT GT_QoS_WEIGHT	*weight
);


/*
 *	gtPortCtrl.c
*/

/*******************************************************************************
* gsysSetJumboMode
*
* DESCRIPTION:
*       This routine Set the max frame size allowed to be received and transmitted
*		from or to a given port.
*
* INPUTS:
*		port - the logical port number
*       mode - GT_JUMBO_MODE (1522, 2048, or 10240)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gsysSetJumboMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_JUMBO_MODE   mode
);

/*******************************************************************************
* gsysGetJumboMode
*
* DESCRIPTION:
*       This routine gets the max frame size allowed to be received and transmitted
*		from or to a given port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_JUMBO_MODE (1522, 2048, or 10240)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gsysGetJumboMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_JUMBO_MODE   *mode
);

/*
 *  gtPhyCtrl.c
*/
/*******************************************************************************
* gprtGetEnergyDetect
*
* DESCRIPTION:
*       Energy Detect power down mode enables or disables the PHY to wake up on
*		its own by detecting activity on the CAT 5 cable.
*
* INPUTS:
* 		port - The logical port number
*
* OUTPUTS:
*       mode - GT_EDETECT_MODE type
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetEnergyDetect
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	OUT GT_EDETECT_MODE   *mode
);

/*******************************************************************************
* gprtSetEnergyDetect
*
* DESCRIPTION:
*       Energy Detect power down mode enables or disables the PHY to wake up on
*		its own by detecting activity on the CAT 5 cable.
*
* INPUTS:
* 		port - The logical port number
*       mode - GT_EDETECT_MODE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtSetEnergyDetect
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	IN  GT_EDETECT_MODE   mode
);


/*
 *	gtSysCtrl.c
*/

/*******************************************************************************
* gsysSetRMUMode
*
* DESCRIPTION:
*		Set Rmote Management Unit Mode: disable, enable on port 4 or 5, or enable
*		on port 9 or 10. Devices, such as 88E6097, support RMU on port 9 and 10,
*		while other devices, such as 88E6165, support RMU on port 4 and 5. So,
*		please refer to the device datasheet for detail.
*		When RMU is enabled and this device receives a Remote Management Request
*		frame directed to this device, the frame will be processed and a Remote
*		Management Response frame will be generated and sent out.
*
*		Note: enabling RMU has no effect if the Remote Management port is in half
*		duplex mode. The port's FrameMode must be DSA or EtherType DSA as well.
*
* INPUTS:
*		rmu - GT_RMU structure
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM     - on bad parameter
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRMUMode
(
	IN GT_QD_DEV	*dev,
	IN GT_RMU		*rmu
);

/*******************************************************************************
* gsysGetRMUMode
*
* DESCRIPTION:
*		Get Rmote Management Unit Mode: disable, enable on port 4 or 5, or enable
*		on port 9 or 10. Devices, such as 88E6097, support RMU on port 9 and 10,
*		while other devices, such as 88E6165, support RMU on port 4 and 5. So,
*		please refer to the device datasheet for detail.
*		When RMU is enabled and this device receives a Remote Management Request
*		frame directed to this device, the frame will be processed and a Remote
*		Management Response frame will be generated and sent out.
*
*		Note: enabling RMU has no effect if the Remote Management port is in half
*		duplex mode. The port's FrameMode must be DSA or EtherType DSA as well.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		rmu - GT_RMU structure
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRMUMode
(
	IN  GT_QD_DEV	*dev,
	OUT GT_RMU		*rmu
);

/*******************************************************************************
* gsysPort2Lport
*
* DESCRIPTION:
*		This routine converts physical port number to logical port number.
*
* INPUTS:
*		port - physical port number
*
* OUTPUTS:
*		lport - logical port number
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysPort2Lport
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		port,
	OUT GT_LPORT	*lport
);

/*******************************************************************************
* gsysLport2Port
*
* DESCRIPTION:
*		This routine converts logical port number to physical port number.
*
* INPUTS:
*		lport - logical port number
*
* OUTPUTS:
*		port - physical port number
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysLport2Port
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	lport,
	OUT GT_U32 		*port
);

/*******************************************************************************
* gsysPortvec2Lportvec
*
* DESCRIPTION:
*		This routine converts physical port vector to logical port vector.
*
* INPUTS:
*		portvec - physical port vector
*
* OUTPUTS:
*		lportvec - logical port vector
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysPortvec2Lportvec
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32		portvec,
	OUT GT_U32 		*lportvec
);

/*******************************************************************************
* gsysLportvec2Portvec
*
* DESCRIPTION:
*		This routine converts logical port vector to physical port vector.
*
* INPUTS:
*		lportvec - logical port vector
*
* OUTPUTS:
*		portvec - physical port vector
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysLportvec2Portvec
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32		lportvec,
	OUT GT_U32 		*portvec
);


/*
 * gtPIRL.c
 */

/*******************************************************************************
* gpirlSetCurTimeUpInt
*
* DESCRIPTION:
*       This function sets the current time update interval.
*		Please contact FAE for detailed information.
*
* INPUTS:
*       upInt - updata interval (0 ~ 7)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gpirlSetCurTimeUpInt
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				upInt
);


/*
 * gtPIRL2.c
 */

/*******************************************************************************
* gpirl2SetCurTimeUpInt
*
* DESCRIPTION:
*       This function sets the current time update interval.
*		Please contact FAE for detailed information.
*
* INPUTS:
*       upInt - updata interval (0 ~ 7)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gpirl2SetCurTimeUpInt
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				upInt
);


/*
 * gtPTP.c
 */

/*******************************************************************************
* gptpSetConfig
*
* DESCRIPTION:
*       This routine writes PTP configuration parameters.
*
* INPUTS:
*		ptpData  - PTP configuration parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetConfig
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_CONFIG	*ptpData
);

/*******************************************************************************
* gptpGetConfig
*
* DESCRIPTION:
*       This routine reads PTP configuration parameters.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpData  - PTP configuration parameters.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetConfig
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_CONFIG	*ptpData
);

/*******************************************************************************
* gptpSetPTPEn
*
* DESCRIPTION:
*       This routine enables or disables PTP.
*
* INPUTS:
*		en - GT_TRUE to enable PTP, GT_FALSE to disable PTP
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetPTPEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_BOOL		en
);

/*******************************************************************************
* gptpGetPTPEn
*
* DESCRIPTION:
*       This routine checks if PTP is enabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		en - GT_TRUE if enabled, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetPTPEn
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_BOOL		*en
);

/*******************************************************************************
* gptpGetPTPInt
*
* DESCRIPTION:
*       This routine gets PTP interrupt status for each port.
*		The PTP Interrupt bit gets set for a given port when an incoming PTP
*		frame is time stamped and PTPArrIntEn for that port is set to 0x1.
*		Similary PTP Interrupt bit gets set for a given port when an outgoing
*		PTP frame is time stamped and PTPDepIntEn for that port is set to 0x1.
*		This bit gets cleared upon software reading and clearing the corresponding
*		time counter valid bits that are valid for that port.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpInt 	- interrupt status for each port (bit 0 for port 0, bit 1 for port 1, etc.)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetPTPInt
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_U32		*ptpInt
);

/*******************************************************************************
* gptpGetPTPGlobalTime
*
* DESCRIPTION:
*       This routine gets the global timer value that is running off of the free
*		running switch core clock.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		ptpTime	- PTP global time
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetPTPGlobalTime
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_U32		*ptpTime
);

/*******************************************************************************
* gptpGetTimeStamped
*
* DESCRIPTION:
*		This routine retrieves the PTP port status that includes time stamp value
*		and sequce Id that are captured by PTP logic for a PTP frame that needs
*		to be time stamped.
*
* INPUTS:
*       port 		- logical port number.
*       timeToRead	- Arr0, Arr1, or Dep time (GT_PTP_TIME enum type)
*
* OUTPUTS:
*		ptpStatus	- PTP port status
*
* RETURNS:
*       GT_OK 		- on success
*       GT_FAIL 	- on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetTimeStamped
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN	GT_PTP_TIME	timeToRead,
	OUT GT_PTP_TS_STATUS	*ptpStatus
);

/*******************************************************************************
* gptpResetTimeStamp
*
* DESCRIPTION:
*		This routine resets PTP Time valid bit so that PTP logic can time stamp
*		a next PTP frame that needs to be time stamped.
*
* INPUTS:
*       port 		- logical port number.
*       timeToReset	- Arr0, Arr1, or Dep time (GT_PTP_TIME enum type)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK 		- on success
*       GT_FAIL 	- on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpResetTimeStamp
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN	GT_PTP_TIME	timeToReset
);

/*******************************************************************************
* gptpGetReg
*
* DESCRIPTION:
*       This routine reads PTP register.
*
* INPUTS:
*       port 		- logical port number.
*       regOffset	- register to read
*
* OUTPUTS:
*		data		- register data
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetReg
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		regOffset,
	OUT GT_U32		*data
);

/*******************************************************************************
* gptpSetReg
*
* DESCRIPTION:
*       This routine writes data to PTP register.
*
* INPUTS:
*       port 		- logical port number
*       regOffset	- register to be written
*		data		- data to be written
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetReg
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		regOffset,
	IN  GT_U32		data
);

#ifdef CONFIG_AVB_FPGA

/*******************************************************************************
* gptpSetFPGAIntStatus
*
* DESCRIPTION:
*       This routine sets interrupt status of PTP logic.
*
* INPUTS:
*		intStatus	- PTP Int Status
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetFPGAIntStatus
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32	intStatus
);


/*******************************************************************************
* gptpGetFPGAIntStatus
*
* DESCRIPTION:
*       This routine gets interrupt status of PTP logic.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		intStatus	- PTP Int Status
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetFPGAIntStatus
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32	*intStatus
);


/*******************************************************************************
* gptpSetFPGAIntEn
*
* DESCRIPTION:
*       This routine enables PTP interrupt.
*
* INPUTS:
*		intEn	- enable/disable PTP interrupt (1 to enable, 0 to disable)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetFPGAIntEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32	intEn
);

/*******************************************************************************
* gptpGetClockSource
*
* DESCRIPTION:
*       This routine gets PTP Clock source mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		clkSrc	- PTP clock source (A/D Device or FPGA)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetClockSource
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_CLOCK_SRC 	*clkSrc
);

/*******************************************************************************
* gptpSetClockSource
*
* DESCRIPTION:
*       This routine sets PTP Clock source mode.
*
* INPUTS:
*		clkSrc	- PTP clock source (A/D Device or FPGA)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetClockSource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_CLOCK_SRC 	clkSrc
);

/*******************************************************************************
* gptpGetP9Mode
*
* DESCRIPTION:
*       This routine gets Port 9 Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		mode - Port 9 mode (GT_PTP_P9_MODE enum type)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetP9Mode
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_P9_MODE 	*mode
);

/*******************************************************************************
* gptpSetP9Mode
*
* DESCRIPTION:
*       This routine sets Port 9 Mode.
*
* INPUTS:
*		mode - Port 9 mode (GT_PTP_P9_MODE enum type)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetP9Mode
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_P9_MODE 	mode
);

/*******************************************************************************
* gptpReset
*
* DESCRIPTION:
*       This routine performs software reset for PTP logic.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpReset
(
	IN  GT_QD_DEV 	*dev
);


/*******************************************************************************
* gptpGetCycleAdjustEn
*
* DESCRIPTION:
*       This routine checks if PTP Duty Cycle Adjustment is enabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		adjEn	- GT_TRUE if enabled, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetCycleAdjustEn
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_BOOL		*adjEn
);


/*******************************************************************************
* gptpSetCycleAdjustEn
*
* DESCRIPTION:
*       This routine enables/disables PTP Duty Cycle Adjustment.
*
* INPUTS:
*		adjEn	- GT_TRUE to enable, GT_FALSE to disable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetCycleAdjustEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_BOOL		adjEn
);


/*******************************************************************************
* gptpGetCycleAdjust
*
* DESCRIPTION:
*       This routine gets clock duty cycle adjustment value.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		adj	- adjustment value (GT_PTP_CLOCK_ADJUSTMENT structure)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpGetCycleAdjust
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_PTP_CLOCK_ADJUSTMENT		*adj
);

/*******************************************************************************
* gptpSetCycleAdjust
*
* DESCRIPTION:
*       This routine sets clock duty cycle adjustment value.
*
* INPUTS:
*		adj	- adjustment value (GT_PTP_CLOCK_ADJUSTMENT structure)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gptpSetCycleAdjust
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_PTP_CLOCK_ADJUSTMENT		*adj
);

/*******************************************************************************
* gptpGetPLLEn
*
* DESCRIPTION:
*       This routine checks if PLL is enabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*		en		- GT_TRUE if enabled, GT_FALSE otherwise
*		freqSel	- PLL Frequency Selection (default 0x3 - 22.368MHz)
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       PLL Frequence selection is based on the Clock Recovery PLL device.
*		IDT MK1575-01 is the default PLL device.
*
*******************************************************************************/
GT_STATUS gptpGetPLLEn
(
	IN  GT_QD_DEV 	*dev,
	OUT GT_BOOL		*en,
	OUT GT_U32		*freqSel
);

/*******************************************************************************
* gptpSetPLLEn
*
* DESCRIPTION:
*       This routine enables/disables PLL device.
*
* INPUTS:
*		en		- GT_TRUE to enable, GT_FALSE to disable
*		freqSel	- PLL Frequency Selection (default 0x3 - 22.368MHz)
*				  Meaningful only when enabling PLL device
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       PLL Frequence selection is based on the Clock Recovery PLL device.
*		IDT MK1575-01 is the default PLL device.
*
*******************************************************************************/
GT_STATUS gptpSetPLLEn
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_BOOL		en,
	IN  GT_U32		freqSel
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __msApi_h */
