#include <Copyright.h>

/********************************************************************************
* gtPortPav.c
*
* DESCRIPTION:
*       API definitions to handle Port Association Vector (0xB).
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

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
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpavSetPAV
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT	 port,
    IN GT_U16	 pav
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U16	    	hwPav;

    DBG_INFO(("gpavSetPAV Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_PORT_MONITORING)) != GT_OK )
      return retVal;

	/*
	 * translate Logical Port Vector to Physical Port Vector.
	 */
	hwPav = (GT_U16)GT_LPORTVEC_2_PORTVEC(pav);

	if(hwPav == (GT_U16)GT_INVALID_PORT_VEC)
	{
		return GT_BAD_PARAM;
	}

    /* there are 7 ports in the switch */
    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_ASSOCIATION,0,dev->maxPorts,hwPav);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



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
* GalTis:
*
*******************************************************************************/
GT_STATUS gpavGetPAV
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U16   *pav
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gpavGetPAV Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_PORT_MONITORING)) != GT_OK )
      return retVal;

    if(pav == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    retVal = hwGetPortRegField(dev,phyPort,(GT_U8)QD_REG_PORT_ASSOCIATION,0,dev->maxPorts,&data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	/*
	 * translate Physical Port Vector to Logical Port Vector.
	 */
    *pav = (GT_U16)GT_PORTVEC_2_LPORTVEC(data);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

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
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpavSetIngressMonitor
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gpavSetIngressMonitorCalled.\n"));

    phyPort = GT_LPORT_2_PORT(port);
    BOOL_2_BIT(mode,data);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENABLE_MONITORING))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_ASSOCIATION,15,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



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
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gpavGetIngressMonitor
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetIngressMonitor Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENABLE_MONITORING))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_ASSOCIATION,15,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

