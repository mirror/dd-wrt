#include <Copyright.h>

/********************************************************************************
* msApiMadConfig.h
*
* DESCRIPTION:
*       Marvell Alaska Device (MAD) Configuration header file
*       If DSDT does not need to load MAD driver for internal Gigabit Phy,
*       MAD_INCLUDE should be undefined.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __msApiMadConfig_h
#define __msApiMadConfig_h

#define MAD_INCLUDE

/*
 * uncomment the following #undef for DSDT not to include MAD
*/
/* #undef MAD_INCLUDE */

#ifdef MAD_INCLUDE
#include <madApi.h>
#endif

#endif /* __msApiMadConfig_h */
