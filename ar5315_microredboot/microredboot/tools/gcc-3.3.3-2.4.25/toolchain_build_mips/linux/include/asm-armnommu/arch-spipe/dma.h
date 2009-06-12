/*
 * asm/arch-armnommu/arch-dsc21/dma.h:
 *         dsc21-specific macros for DMA support.
 * copyright:
 *         (C) 2001 RidgeRun, Inc. (http://www.ridgerun.com)
 * author: Gordon McNutt <gmcnutt@ridgerun.com>
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
 */

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

/*
 * MAX_DMA_ADDRESS -- max address we can program into the DMA address registers?
 * Not sure. Various drivers use this value as a test before setting up a DMA.
 * Also, the macros in linux/bootmem.h use this value as a goal for many of the
 * bootmem allocations.
 * Open questions:
 * a) the DMA address register is only 26 bits -- does this limit us to this
 *    section of memory, or can we only DMA in certain size chunks?
 * b) can the ARM processor even directly program the DMA?
 * I'm going to go out on a limb and assume that the DMA range is the first
 * addresseable 26 bits.
 * --gmcnutt
 */
#define MAX_DMA_ADDRESS		0x02000000

/*
 * MAX_DMA_CHANNELS -- # of DMA control register sets we possess. For the dsc21
 * we have one that can run in either direction and between two different
 * destinations.
 * --gmcnutt
 */
#define MAX_DMA_CHANNELS        1

/*
 * arch_dma_init -- called by arch/armnommu/kernel/dma.c init_dma.
 * Don't know what's needed here (if anything).
 * --gmcnutt
 */
#define arch_dma_init(dma_chan)

#endif /* _ASM_ARCH_DMA_H */
