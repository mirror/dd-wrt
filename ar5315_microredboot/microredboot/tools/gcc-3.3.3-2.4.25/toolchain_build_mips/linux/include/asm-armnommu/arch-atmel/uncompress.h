/*
 * asm/arch/uncompress.c:
 *         Optional routines to aid in debugging the decompression phase
 *         of kernel boot.
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

/*
 * This is used by arch/armnommu/boot/compressed/misc.c to write progress info
 * out the serial port so that the user can see debug messages up to the point
 * where the kernel is decompressed. The STANDALONE_DEBUG macro chooses between
 * this and the standard printf. Punt.
 * --gmcnutt
 */

#include <asm/hardware.h>

#ifdef CONFIG_SERIAL_ATMEL_CONSOLE
#define putc(c) at91_usart_putc((volatile struct atmel_usart_regs *)AT91_USART0_BASE,c)
/*
 * If we need to do some setup prior to decompression (like initializing the
 * UART if we want to use puts() above) then we define it here. Punt.
 */
#define arch_decomp_setup() HW_AT91_USART_INIT \
	at91_usart_init((volatile struct atmel_usart_regs *)AT91_USART0_BASE,9600)
#else
#define puts(s)
#define arch_decomp_setup()
#endif

/*
 * Not sure what this is for. Probably an optional watchdog to check if the
 * decompress got hung so we can warn the user. Punt.
 */
#define arch_decomp_wdog()


