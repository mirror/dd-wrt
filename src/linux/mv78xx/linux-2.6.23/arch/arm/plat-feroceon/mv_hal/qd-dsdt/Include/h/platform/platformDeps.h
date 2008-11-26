#include <Copyright.h>

/********************************************************************************
* platformDeps.h
*
* DESCRIPTION:
*       platform dependent definitions
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __platformDepsh
#define __platformDepsh

#include <msApi.h>
#ifdef __cplusplus
extern "C" {
#endif

GT_BOOL defaultMiiRead (unsigned int portNumber , unsigned int miiReg, unsigned int* value);
GT_BOOL defaultMiiWrite (unsigned int portNumber , unsigned int miiReg, unsigned int value);

#ifdef __cplusplus
}
#endif

#endif   /* platformDepsh */

