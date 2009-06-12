/*
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/hw_irq.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * BlackFin (ADI) assembler restricted values by Ted Ma <mated@sympatico.ca>
 * Copyright (c) 2002 Arcturus Networks Inc. (www.arcturusnetworks.com)
 * Copyright (c) 2002 Lineo, Inc <mattw@lineo.com>
 */
/*
 This is included from <linux/irq.h>.
	Include by assembler files which don't need the 'C' stuff
 */
#include <linux/config.h>

#ifdef CONFIG_BLACKFIN_EZKIT
  #include <asm/board/bf533_irq.h>
#endif
#ifdef CONFIG_PUB
  #include <asm/board/bf535_irq.h>
#endif
#ifdef CONFIG_HAWK
  #include <asm/board/bf535_irq.h>
#endif
#ifdef CONFIG_EAGLE
  #include <asm/board/bf535_irq.h>
#endif
