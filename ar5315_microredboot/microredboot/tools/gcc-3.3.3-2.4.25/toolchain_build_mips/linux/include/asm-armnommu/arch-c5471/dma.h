/***********************************************************************
 * asm/arch-armnommu/arch-c5471/dma.h:
 *
 *   Copyright (C) 2003 Cadenux, LLC. All rights reserved.
 *   todd.fischer@cadenux.com  <www.cadenux.com>
 *
 *   Copyright (C) 2001 RidgeRun, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 * WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 * USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***********************************************************************/

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

#include "linux/autoconf.h"

/* type dmamode_t is declared in include/asm-armnommu along with values.
 * Some additional DMA mode values are declared here.
 */

#define DMA_MODE_CARD_TO_SDRAM  0x08
#define DMA_MODE_SDRAM_TO_CARD  0x0c
#define DMA_MODE_FLASH_TO_SDRAM 0x10
#define DMA_MODE_SDRAM_TO_FLASH 0x14

/* MAX_DMA_ADDRESS -- max address we can program into the DMA address
 * registers? Not sure. Various drivers use this value as a test before
 * setting up a DMA. Also, the macros in linux/bootmem.h use this value
 * as a goal for many of the bootmem allocations.
 */

/* Configured memory is specified during configuration. */

#define MAX_DMA_ADDRESS (DRAM_BASE + DRAM_SIZE - 1)

/* MAX_DMA_CHANNELS -- # of DMA control register sets we possess.
 * We have one that can run in either direction and between two different
 * destinations.
 */

#define MAX_DMA_CHANNELS 1

#endif /* __ASM_ARCH_DMA_H */
