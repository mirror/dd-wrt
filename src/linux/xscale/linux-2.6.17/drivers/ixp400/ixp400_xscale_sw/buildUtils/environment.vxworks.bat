:: IXP400 SW Release version  2.1
:: 
:: -- Intel Copyright Notice --
:: 
:: @par
:: Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
:: 
:: @par
:: The source code contained or described herein and all documents
:: related to the source code ("Material") are owned by Intel Corporation
:: or its suppliers or licensors.  Title to the Material remains with
:: Intel Corporation or its suppliers and licensors.
:: 
:: @par
:: The Material is protected by worldwide copyright and trade secret laws
:: and treaty provisions. No part of the Material may be used, copied,
:: reproduced, modified, published, uploaded, posted, transmitted,
:: distributed, or disclosed in any way except in accordance with the
:: applicable license agreement .
:: 
:: @par
:: No license under any patent, copyright, trade secret or other
:: intellectual property right is granted to or conferred upon you by
:: disclosure or delivery of the Materials, either expressly, by
:: implication, inducement, estoppel, except in accordance with the
:: applicable license agreement.
:: 
:: @par
:: Unless otherwise agreed by Intel in writing, you may not remove or
:: alter this notice or any other notice embedded in Materials by Intel
:: or Intel's suppliers or licensors in any way.
:: 
:: @par
:: For further details, please see the file README.TXT distributed with
:: this software.
:: 
:: @par
:: -- End Intel Copyright Notice --
rem Tornado environment settings. Edit these to reflect your installation.
set WIND_BASE=c:\tornado
set WIND_HOST_TYPE=x86-win32
set WIND_HOST_BASE=%WIND_BASE%

rem The location of the ixp400_xscale_sw directory (needed to build the vxWorks BSP)
set CSR_BASE=c:\change\this\to\your\ixp400_xscale_sw

rem Tool location for diab support
set DIABLIB=c:\path\to\the\diab\tool

set PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%DIABLIB%\WIN32\bin;%PATH%

rem NOTE: All flag definitions below are case sensitive

rem For IXDP425 platform, set IX_DEVICE to ixp42X
rem For IXDP465 platform, set IX_DEVICE to ixp42X or ixp46X
set IX_DEVICE=ixp42X

rem For IXDP425 platform, set IX_PLATFORM to ixdp42x 
rem For IXDP465 platform, set IX_PLATFORM to ixdp46x
set IX_PLATFORM=ixdp42x

rem For Big endian, set IX_TARGET to vxbe
rem For Little endian, set IX_TARGET to vxle
set IX_TARGET=vxbe

