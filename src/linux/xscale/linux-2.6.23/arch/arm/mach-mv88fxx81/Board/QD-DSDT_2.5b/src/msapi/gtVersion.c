#include <Copyright.h>

/********************************************************************************
* gtVersion.h
*
* DESCRIPTION:
*       Includes software version information for the QuarterDeck software
*       suite.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <msApi.h>

char msApiCopyright[] = MSAPI_COPYRIGHT;
char msApiVersion[] = MSAPI_VERSION;

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
)
{
    if(version == NULL)
        return GT_BAD_PARAM;

    if(gtStrlen(msApiVersion) > VERSION_MAX_LEN)
    {
        return GT_FAIL;
    }

    gtMemCpy(version->version,msApiVersion,gtStrlen(msApiVersion));
    version->version[gtStrlen(msApiVersion)] = '\0';
    return GT_OK;
}

