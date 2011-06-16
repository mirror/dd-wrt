/*******************************************************************************
 * 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *             
 *****************************************************************************/

#if !defined(__IX_SYMBOLS_H__)
#define __IX_SYMBOLS_H__

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 * The IX_EXPORT_FUNCTION symbol will be used for compilation on different platforms.
 * We are planning to provide a simulation version of the library that should work
 * with the Transactor rather than the hardware. This implementation will be done on
 * WIN32 in the form of a DLL that will need to export functions and symbols.
 */
#if (_IX_OS_TYPE_ == _IX_OS_WIN32_)
#    if defined(_IX_LIB_INTERFACE_IMPLEMENTATION_)
#        define IX_EXPORT_FUNCTION __declspec( dllexport )
#        define IX_EXPORT_SYMBOL   extern __declspec( dllexport )
#    elif defined(_IX_LIB_INTERFACE_IMPORT_DLL_)
#        define IX_EXPORT_FUNCTION extern
#        define IX_EXPORT_SYMBOL   __declspec( dllimport )
#    else
#        define IX_EXPORT_FUNCTION extern 
#        define IX_EXPORT_SYMBOL   extern
#    endif
#else
#    define IX_EXPORT_FUNCTION  extern
#    define IX_EXPORT_SYMBOL    extern
#endif



#if defined(IX_PLATFORM_2400)

#define _IX_HARDWARE_TYPE_ 1	/* _IX_HW_2400_ */
#define _IX_BOARD_TYPE_    1	/* _IX_IXDP2400_ */

#elif defined(IX_PLATFORM_2401)

#define _IX_HARDWARE_TYPE_ 1	/* _IX_HW_2400_ */
#define _IX_BOARD_TYPE_    2	/* _IX_IXDP2401_ */

#elif defined(IX_PLATFORM_2800)

#define _IX_HARDWARE_TYPE_ 2    /* _IX_HW_2800_ */
#define _IX_BOARD_TYPE_    3	/* _IX_IXDP2800_ */

#elif defined(IX_PLATFORM_2801)

#define _IX_HARDWARE_TYPE_ 2	/* _IX_HW_2800_ */
#define _IX_BOARD_TYPE_    4	/* _IX_IXDP2801_ */

#elif defined(IX_PLATFORM_2350)

#define _IX_HARDWARE_TYPE_ 3	/* _IX_HW_23XX_ */
#define _IX_BOARD_TYPE_    5	/* _IX_IXDP2350_ */

#elif defined(IX_PLATFORM_2351)

#define _IX_HARDWARE_TYPE_ 3	/* _IX_HW_23XX_ */
#define _IX_BOARD_TYPE_    6	/* _IX_IXDP2351_ */

#endif

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_SYMBOLS_H__) */
