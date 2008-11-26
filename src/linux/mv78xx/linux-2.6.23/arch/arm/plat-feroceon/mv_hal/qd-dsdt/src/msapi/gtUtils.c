#include <Copyright.h>

/********************************************************************************
* gtUtils.c
*
* DESCRIPTION:
*       Collection of Utility functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <msApi.h>
#include "mvOs.h"

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
)
{
	GT_U32 i;
	char* buf;
	
	buf = (char*)start;
		
	for(i=0; i<size; i++)
	{
		*buf++ = (char)symbol;
	}

	return start;
}

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
)
{
	GT_U32 i;
	char* buf;
	char* src;
	
	buf = (char*)destination;
	src = (char*)source;
		
	for(i=0; i<size; i++)
	{
		*buf++ = *src++;
	}

	return destination;
}

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
)
{
	GT_U32 i;
	int value;

	for(i=0; i<size; i++)
	{
		if((value = (int)(src1[i] - src2[i])) != 0)
			return value; 
	}

	return 0;
}

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
)
{
	GT_U32 i = 0;
	char* src;
	
	src = (char*)source;
		
	while(*src++) i++;

    return i;
}


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
)
{
    mvOsUDelay(delayTime);
}



