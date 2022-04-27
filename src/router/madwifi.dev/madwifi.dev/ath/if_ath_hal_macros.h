/*-
 * Copyright (c) 2007 Michael Taylor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
	without modification.
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
 * $Id: foo mtaylor $
 */
/* 
 * This file provides some macros that are used by if_ath_hal.h,
 * if_ath_hal_wrappers.h, and if_ath_hal_wrappers.c.
 * 
 * The macros are provided for the HAL lock and for dynamically deciding whether
 * to define the HAL wrapper functions as inline or in the implementation file
 * at build time.  i.e. inline for speed, or non-inline for debugging visibility
 * into the HAL methods being called (despite obfuscation).
 * 
 * The if_ath_hal.h file is generated from hal/ah.h at build time by the build
 * system.
 */
#ifndef _IF_ATH_HAL_MACROS_H_
#define _IF_ATH_HAL_MACROS_H_

#define ATH_HAL_LOCK_INIT(_sc) 	spin_lock_init(&(_sc)->sc_hal_lock)
#define ATH_HAL_LOCK_DESTROY(_sc)
#define ATH_HAL_LOCK_IRQ(_sc) 	do { \
   unsigned long __sc_halLockflags; 				   \
   ATH_HAL_LOCK_CHECK(_sc); 					   \
   spin_lock_irqsave(&(_sc)->sc_hal_lock, __sc_halLockflags);
#define ATH_HAL_UNLOCK_IRQ(_sc) \
   ATH_HAL_LOCK_ASSERT(_sc); 					   \
   spin_unlock_irqrestore(&(_sc)->sc_hal_lock, __sc_halLockflags); \
   } while(0)
#define ATH_HAL_UNLOCK_IRQ_EARLY(_sc) \
   ATH_HAL_LOCK_ASSERT(_sc); 					   \
   spin_unlock_irqrestore(&(_sc)->sc_hal_lock, __sc_halLockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	ATH_HAL_LOCK_ASSERT(_sc) \
	KASSERT(spin_is_locked(&(_sc)->sc_hal_lock), ("HAL not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	ATH_HAL_LOCK_CHECK(_sc) do { \
	if (spin_is_locked(&(_sc)->sc_hal_lock)) \
		IPRINTF(_sc, "About to block on HAL lock! [%s:%d]\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	ATH_HAL_LOCK_CHECK(_sc)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */

#else
#define	ATH_HAL_LOCK_ASSERT(_sc)
#define	ATH_HAL_LOCK_CHECK(_sc)
#endif

#endif				/* #ifndef _IF_ATH_HAL_MACROS_H_ */
