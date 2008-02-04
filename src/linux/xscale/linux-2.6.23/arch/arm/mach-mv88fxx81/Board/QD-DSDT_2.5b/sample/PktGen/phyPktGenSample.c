#include <Copyright.h>
/********************************************************************************
* phyPktGenSample.c
*
* DESCRIPTION:
*       Packet Generator setup sample (startPktGenerator and stopPktGenerator).
*
* DEPENDENCIES:
*		Please check the phy device's spec. if the device supports this feature.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"


/*
 * Start Packet Generator.
 * Input:
 *      pktload - enum GT_PG_PAYLOAD (GT_PG_PAYLOAD_RANDOM or GT_PG_PAYLOAD_5AA5)
 *      length  - enum GT_PG_LENGTH  (GT_PG_LENGTH_64 or GT_PG_LENGTH_1514)
 *      tx      - enum GT_PG_TX      (GT_PG_TX_NORMAL or GT_PG_TX_ERROR)
*/
GT_STATUS startPktGenerator
(
    GT_QD_DEV      *dev,
    GT_LPORT       port,
    GT_PG_PAYLOAD  payload,
    GT_PG_LENGTH   length,
    GT_PG_TX       tx
)
{
    GT_STATUS status;
    GT_PG     pktInfo;

    if (dev == 0)
    {
        MSG_PRINT(("GT driver is not initialized\n"));
        return GT_FAIL;
    }

    MSG_PRINT(("Start Packet Generator for port %i\n",(int)port));

    pktInfo.payload = payload; /* Pseudo-random, 5AA55AA5... */
    pktInfo.length = length;   /* 64 bytes, 1514 bytes */
    pktInfo.tx = tx;           /* normal packet, error packet */

    /*
     *	Start Packet Generator
    */
    if((status = gprtSetPktGenEnable(dev,port,GT_TRUE,&pktInfo)) != GT_OK)
    {
        MSG_PRINT(("mdDiagSetPktGenEnable return Failed\n"));
        return status;
    }

    return GT_OK;
}


/*
 * Stop Packet Generator.
 */
GT_STATUS stopPktGenerator(GT_QD_DEV *dev,GT_LPORT port)
{
    GT_STATUS status;

    if (dev == 0)
    {
        MSG_PRINT(("GT driver is not initialized\n"));
        return GT_FAIL;
    }

    MSG_PRINT(("Stopping Packet Generator for port %i\n",(int)port));

    /*
     *	Start Packet Generator
    */
    if((status = gprtSetPktGenEnable(dev,port,GT_FALSE,NULL)) != GT_OK)
    {
        MSG_PRINT(("mdDiagSetPktGenEnable return Failed\n"));
        return status;
    }

    return GT_OK;
}
