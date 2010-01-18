/*
 * FILE NAME gemini_gpio.h
 *
 * BRIEF MODULE DESCRIPTION
 *	Generic Gemini GPIO
 *
 *  Author: Storlink Software [Device driver]
 *          Jason Lee <jason@storlink.com.tw>
 *
 * Copyright 2005 Storlink Inc.
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
 */

#ifndef __GEMINI_GPIO_H
#define __GEMINI_GPIO_H

#include <linux/ioctl.h>

#define STATUS_HIGH	1
#define STATUS_LOW	0
#define DIRECT_OUT	1
#define DIRECT_IN	0

#define EDGE_TRIG	0
#define RISING_EDGE	0
#define FALL_EDGE	1
#define SINGLE_EDGE	0
#define BOTH_EDGE	1

#define LEVEL_TRIG	1
#define HIGH_ACTIVE	0
#define LOW_ACTIVE	1

struct gemini_gpio_ioctl_data {
	__u32 pin;
	__u8 status;			// status or pin direction
					// 0: status low or Input
					// 1: status high or Output

	/* these member are used to config GPIO interrupt parameter */
	__u8	use_default;		// if not sure ,set this argument 1
	__u8	trig_type;		// 0/1:edge/level triger ?
	__u8	trig_polar;		// 0/1:rising/falling high/low active ?
	__u8	trig_both;		// 0/1:single/both detect both ?
};

#define GEMINI_GPIO_IOCTL_BASE	'Z'

#define GEMINI_SET_GPIO_PIN_DIR		_IOW (GEMINI_GPIO_IOCTL_BASE,16, struct gemini_gpio_ioctl_data)
#define	GEMINI_SET_GPIO_PIN_STATUS	_IOW (GEMINI_GPIO_IOCTL_BASE,17, struct gemini_gpio_ioctl_data)
#define	GEMINI_GET_GPIO_PIN_STATUS	_IOWR(GEMINI_GPIO_IOCTL_BASE,18, struct gemini_gpio_ioctl_data)
#define GEMINI_WAIT_GPIO_PIN_INT	_IOWR(GEMINI_GPIO_IOCTL_BASE,19, struct gemini_gpio_ioctl_data)


extern void init_gpio_int(__u32 pin,__u8 trig_type,__u8 trig_polar,__u8 trig_both);
extern int request_gpio_irq(int bit,void (*handler)(int),char level,char high,char both);
extern int free_gpio_irq(int bit);
#endif
