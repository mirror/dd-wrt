/*-
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: ah_osdep.h 2554 2007-07-06 04:20:15Z proski $
 */
#ifndef _ATH_AH_OSDEP_H_
#define _ATH_AH_OSDEP_H_

/*
 * Linux on i386 may pass parameters in registers.  This is an option
 * starting with Linux 2.6.4.  Starting with Linux 2.6.20, it's done
 * unconditionally.  However, the HAL uses standard ABI whereas the
 * parameters are passed on the stack (for maximum portability).
 * "asmlinkage" forces the standard ABI for the HAL calls.
 */
#ifdef __i386__
#define	__ahdecl	asmlinkage
#else
#define	__ahdecl
#endif
#ifndef __packed
#define	__packed	__attribute__((__packed__))
#endif

/* Replace void pointers from ah.h with safer specific types */
#define HAL_SOFTC struct ath_softc *
#define HAL_BUS_HANDLE void __iomem *
#define HAL_BUS_TAG struct ar531x_config *

#endif				/* _ATH_AH_OSDEP_H_ */
