#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL extended support for platform specific interrupts
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-02-24
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the SA1110/iPAQ are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include CYGBLD_HAL_VAR_INTS_H   // Generic interrupt support

// Define here extended support for this particular platform

// IRQ handler for extended interrupts
#define HAL_EXTENDED_IRQ_HANDLER(index)                                 \
{  /* Check for extended interrupt source.  If present, decode it into  \
      a valid eCos interrupt number and return.  Fall through if no     \
      extended interrupt is present. */                                 \
}

// Mask an extended interrupt
#define HAL_EXTENDED_INTERRUPT_MASK(vector)                             \
{  /* Mask an interrupt source.  If 'vector' corresponds to an extended \
      interrupt, handle it and return.  Otherwise, fall through for     \
      normal processing. */                                             \
}

// Unmask an extended interrupt
#define HAL_EXTENDED_INTERRUPT_UNMASK(vector)                             \
{  /* Unmask an interrupt source.  If 'vector' corresponds to an extended \
      interrupt, handle it and return.  Otherwise, fall through for       \
      normal processing. */                                               \
}

// Acknowledge an extended interrupt
#define HAL_EXTENDED_INTERRUPT_ACKNOWLEDGE(vector)                      \
{  /* Ack an interrupt source.  If 'vector' corresponds to an extended  \
      interrupt, handle it and return.  Otherwise, fall through for     \
      normal processing. */                                             \
}

// Configure an extended interrupt
#define HAL_EXTENDED_INTERRUPT_CONFIGURE(vector, level, up)                  \
{  /* Configure an interrupt source.  If 'vector' corresponds to an extended \
      interrupt, handle it and return.  Otherwise, fall through for          \
      normal processing. */                                                  \
}

// Configure an extended interrupt
#define HAL_EXTENDED_INTERRUPT_SET_LEVEL(vector, level)                      \
{  /* Configure an interrupt source.  If 'vector' corresponds to an extended \
      interrupt, handle it and return.  Otherwise, fall through for          \
      normal processing. */                                                  \
}

#endif // CYGONCE_HAL_PLATFORM_INTS_H
