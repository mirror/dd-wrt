/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012 Cavium Inc. 
 */
#ifndef __ASM_OCTEON_OCTEON_CRYPTO_H
#define __ASM_OCTEON_OCTEON_CRYPTO_H

struct octeon_cop2_state;
extern unsigned long octeon_crypto_enable(struct octeon_cop2_state *state);
extern void octeon_crypto_disable(struct octeon_cop2_state *state,
				  unsigned long flags);

#endif /* __ASM_OCTEON_OCTEON_CRYPTO_H */
