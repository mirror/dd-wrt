#include <Copyright.h>
/********************************************************************************
* loadBalance.c
*
* DESCRIPTION:
*		This sample shows how to setup load balance among Trunk ports.
*		In this sample, port 0,1,2, and 3 will be in the Trunk group.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
* COMMENTS:
*******************************************************************************/

#include "msSample.h"


/*
   The following sample sets Trunk Mask Table as follows:
 
					10	9	8	7	6	5	4	3	2	1	0
   TrunkMask[0]		1	1	1	1	1	1	1	0	0	0	1
   TrunkMask[1]		1	1	1	1	1	1	1	0	0	1	0
   TrunkMask[2]		1	1	1	1	1	1	1	0	1	0	0
   TrunkMask[3]		1	1	1	1	1	1	1	1	0	0	0
   TrunkMask[4]		1	1	1	1	1	1	1	0	0	0	1
   TrunkMask[5]		1	1	1	1	1	1	1	0	0	1	0
   TrunkMask[6]		1	1	1	1	1	1	1	0	1	0	0
   TrunkMask[7]		1	1	1	1	1	1	1	1	0	0	0
*/

GT_STATUS sampleLoadBalance(GT_QD_DEV *dev)
{
	GT_STATUS status;
	int i;
	GT_U32 mask, baseMask;

	baseMask = 0xFFF0;	/* clear bits for port 0 ~ 3 */

	/*
	 *	Set the trunk mask table for load balancing.
	*/
	for(i=0; i<8; i++)
	{
		mask = baseMask | (1 << (i%4));

		if((status = gsysSetTrunkMaskTable(dev,i,mask)) != GT_OK)
		{
			MSG_PRINT(("gsysSetTrunkMaskTable return Failed\n"));
			return status;
		}
	}

	return GT_OK;
}

