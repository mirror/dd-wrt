#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: hmt, jskov, msalter
// Date:         2000-10-10
// Purpose:      Intel IOP310 PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint32 __dev = CYG_PCI_DEV_GET_DEV(__devfn);                          \
    cyg_uint32 __fn = CYG_PCI_DEV_GET_FN(__devfn);                            \
    __valid = false;                                                          \
    if (__bus == (*SBNR_REG + 1) && __dev == 0 && __fn == 0) {		      \
        __vec = CYGNUM_HAL_INTERRUPT_ETHERNET;                                \
        __valid = true;                                                       \
    } else {                                                                  \
        cyg_uint8 __req;                                                      \
        HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);   \
        switch (__dev % 4) {                                                  \
          case 0:                                                             \
            switch(__req) {                                                   \
              case 1: /* INTA */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTA; __valid=true; break;   \
              case 2: /* INTB */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTB; __valid=true; break;   \
              case 3: /* INTC */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTC; __valid=true; break;   \
              case 4: /* INTD */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTD; __valid=true; break;   \
            }                                                                 \
 	    break;                                                            \
          case 1:                                                             \
            switch(__req) {                                                   \
              case 1: /* INTA */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTB; __valid=true; break;   \
              case 2: /* INTB */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTC; __valid=true; break;   \
              case 3: /* INTC */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTD; __valid=true; break;   \
              case 4: /* INTD */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTA; __valid=true; break;   \
            }                                                                 \
 	    break;                                                            \
          case 2:                                                             \
            switch(__req) {                                                   \
              case 1: /* INTA */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTC; __valid=true; break;   \
              case 2: /* INTB */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTD; __valid=true; break;   \
              case 3: /* INTC */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTA; __valid=true; break;   \
              case 4: /* INTD */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTB; __valid=true; break;   \
            }                                                                 \
 	    break;                                                            \
          case 3:                                                             \
            switch(__req) {                                                   \
              case 1: /* INTA */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTD; __valid=true; break;   \
              case 2: /* INTB */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTA; __valid=true; break;   \
              case 3: /* INTC */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTB; __valid=true; break;   \
              case 4: /* INTD */                                              \
                __vec=CYGNUM_HAL_INTERRUPT_PCI_S_INTC; __valid=true; break;   \
            }                                                                 \
 	    break;                                                            \
        }                                                                     \
    }                                                                         \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
