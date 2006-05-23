/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   NR_IRQS for IDT boards
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 P. Sadik.
 *
 * Initial Release
 *
 * June 2004 rkt 
 * Added support for EB355 & EB434
 * 
 **************************************************************************
 */

#ifndef __ASM_MACH_IDT_IRQ_H__
#define __ASM_MACH_IDT_IRQ_H__
#include <linux/config.h>

#ifdef CONFIG_IDT_EB365
#include <asm/idt-boards/rc32300/rc32365.h>
#define NR_IRQS RC32365_NR_IRQS
#endif

#ifdef CONFIG_IDT_EB434
#include <asm/idt-boards/rc32434/rc32434.h>
#define NR_IRQS RC32434_NR_IRQS
#endif

#ifdef CONFIG_IDT_EB438
#include <asm/idt-boards/rc32438/rc32438.h>
#define NR_IRQS RC32438_NR_IRQS
#endif

#ifdef CONFIG_IDT_S334
#include <asm/idt-boards/rc32300/rc32334.h>
#define NR_IRQS RC32334_NR_IRQS
#endif

#ifdef CONFIG_IDT_EB355
#include <asm/idt-boards/rc32300/rc32355.h>
#define NR_IRQS RC32355_NR_IRQS
#endif

#endif //__ASM_MACH_IDT_IRQ_H__
