/**
 * @file IxAtmdPrivateSymbols.c
 *
 * @author Intel Corporation
 * @date 29-Jul-2003
 *
 * @brief This file declares additional exported symbols for linux kernel module builds.
 *
 * @par 
 *
 * The symbols exported by this file are internal functions, exported
 * only if the macro IX_PRIVATE_OFF is defined. They are exported for
 * Intel testing purposes only.
 *
 * @par
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifdef __linux

#include <linux/module.h>

#ifdef IX_PRIVATE_OFF

/* Macro to provide dummy declarations for the symbols. */
#define DECLARE_AND_EXPORT_SYMBOL(s) extern void s(void);\
                                     EXPORT_SYMBOL(s)

/* These symbols are referenced by the atmd integration test */
DECLARE_AND_EXPORT_SYMBOL(ixAtmdAccPortDescriptor);
DECLARE_AND_EXPORT_SYMBOL(ixAtmdAccRxCfgNpeAtmStatusRead);
DECLARE_AND_EXPORT_SYMBOL(ixAtmdAccTxDoneDispatcher);

#endif /* IX_PRIVATE_OFF */

#endif /* __linux */
