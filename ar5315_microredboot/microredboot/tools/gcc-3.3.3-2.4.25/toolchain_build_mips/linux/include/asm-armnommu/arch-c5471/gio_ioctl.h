/************************************************************************
 * include/asm-armnommu/arch-c5471/gio_ioctl.h
 *
 * Definitions of the General Purpose I/O (GIO) for the C5471.
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
 ************************************************************************/

#ifndef __ASM_ARCH_GIO_IOCTL_H
#define __ASM_ARCH_GIO_IOCTL_H
 
/************************************************************************
 * Included Files
 ************************************************************************/

#include <asm/ioctl.h>
 
/************************************************************************
 * Definitions
 ************************************************************************/

#define GIO_MAGIC_NUMBER 'g'

/* Major numbers, (from Documentation/devices.txt local/experimental list)  */
 
#define GIO_MAJOR     125
 
/************************************************************************
 * GIO_DIRECTION: The GIO_DIRECTION can be either input (GIO_INPUT) or
 * output (GIO_OUTPUT).  If unspecified, the GIO will default to input.
 */

#define GIO_OUTPUT 0
#define GIO_INPUT 1

#define GIO_DIRECTION _IO(GIO_MAGIC_NUMBER, 0)
 
/************************************************************************
 * GIO_BITSET: If the direction is set to output, this ioctl will set the
 * output.  If the direction is input, this ioctl will return the value
 * of the GIO pin (0, 1, or -1 on error).
 */

#define GIO_BITSET _IO(GIO_MAGIC_NUMBER, 1)

/************************************************************************
 * GIO_BITCLR: If the direction is set to output, this ioctl will clear the
 * output.  If the direction is input, this ioctl will return the value
 * of the GIO pin (0, 1, or -1 on error).
 */

#define GIO_BITCLR _IO(GIO_MAGIC_NUMBER, 2)

/************************************************************************
 * GIO_IRQPORT: Configure a GIO as an interrupt.  Only GIO0-GIO3 can
 * be configured as an interrupting source.
 */

#define GIO_NOINTERRUPT 0
#define GIO_INTERRUPT 1

#define GIO_IRQPORT _IO(GIO_MAGIC_NUMBER, 3)

#define GIO_IOC_MAXNR 3
 
#endif /* __ASM_ARCH_GIO_IOCTL_H */
