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

#if !defined(__IX_OS_TYPE_H__)
#define __IX_OS_TYPE_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 * Define symbols for each supported OS
 */
#define _IX_OS_VXWORKS_             1   /* VxWorks OS */
#define _IX_OS_WIN32_               2   /* Windows Win32 OS */
#define _IX_OS_LINUX_KERNEL_        3   /* Linux kernel OS */
#define _IX_OS_LINUX_USER_          4   /* Linux user OS */
#define _IX_OS_QNX_                 5   /* QNX OS */
#define _IX_OS_ECOS_                6   /* ECOS OS */
#define _IX_OS_DIAGNOSTIC_          7   /* DIAGNOSTIC OS */
#define _IX_OS_FREEBSD_KERNEL_      8   /* FreeBSD Kernel OS */
#define _IX_OS_FREEBSD_USER_        9   /* FreeBSD user OS */


#undef _IX_OS_TYPE_

#if !defined(_IX_OS_TYPE_)
#   if defined(__vxworks) || defined (VXWORKS)
#       define _IX_OS_TYPE_ _IX_OS_VXWORKS_
#   elif defined(WIN32)
#       define _IX_OS_TYPE_ _IX_OS_WIN32_
#   elif defined(__linux) || defined(__linux__)
#       if defined (__KERNEL__)
#           define _IX_OS_TYPE_ _IX_OS_LINUX_KERNEL_
#       else
#           define _IX_OS_TYPE_ _IX_OS_LINUX_USER_
#       endif
#   elif defined(__freebsd) || defined(__freebsd__)
#       if defined (__KERNEL__) || defined(_KERNEL)
#           define _IX_OS_TYPE_ _IX_OS_FREEBSD_KERNEL_
#       else
#           define _IX_OS_TYPE_ _IX_OS_FREEBSD_USER_
#       endif
#   elif defined(__qnx)
#       define _IX_OS_TYPE_ _IX_OS_QNX_
#   elif defined(__ECOS)
#       define _IX_OS_TYPE_ _IX_OS_ECOS_
#   elif defined(KI_DIAGNOSTIC)
#       define _IX_OS_TYPE_ _IX_OS_DIAGNOSTIC_
#   endif
#endif /* !defined(_IX_OS_TYPE_) */

/**
 * Check if the OS type is defined and if it is supported
 */
#if 0
#if !defined(_IX_OS_TYPE_) || \
        ((_IX_OS_TYPE_ != _IX_OS_VXWORKS_) && \
         (_IX_OS_TYPE_ != _IX_OS_WIN32_) && \
         (_IX_OS_TYPE_ != _IX_OS_QNX_)&& \
         (_IX_OS_TYPE_ != _IX_OS_ECOS_)&& \
         (_IX_OS_TYPE_ != _IX_OS_DIAGNOSTIC_)&& \
         (_IX_OS_TYPE_ != _IX_OS_LINUX_KERNEL_) && \
         (_IX_OS_TYPE_ != _IX_OS_FREEBSD_KERNEL_) && \
         (_IX_OS_TYPE_ != _IX_OS_FREEBSD_USER_) && \
         (_IX_OS_TYPE_ != _IX_OS_LINUX_USER_))
#   error You are trying to build on an unsupported OS!
#endif
#endif


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_OS_TYPE_H__) */

