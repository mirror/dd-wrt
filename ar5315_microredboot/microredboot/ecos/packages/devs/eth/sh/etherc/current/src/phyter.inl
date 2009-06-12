#ifndef CYGONCE_DEVS_ETH_PHYTER_PHYTER_INL
#define CYGONCE_DEVS_ETH_PHYTER_PHYTER_INL
//==========================================================================
//
//      phyther.inl
//
//      Generic Phyter definitions and helpers
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2002-02-19
// Purpose:      phy/MII definitions 
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Caller must define _MII_WRITE and _MII_READ macros
#ifndef _MII_WRITE
# error "_MII_WRITE(_priv_, _id_, _reg_, _val_) and _MII_READ(_priv_, _id_, _reg_) must be defined"
#endif

// Caller can define _MII_HAS_EXTENDED to add support for extended registers

//-----------------------------------------------------------------------------
// Helpers

// Renegotiate link
// This function will delay for up to 2 seconds waiting for a renegotiation
// of the link.
#define _MII_RENEGOTIATE(_priv_, _id_)                                                  \
  CYG_MACRO_START                                                                       \
  _MII_WRITE(_priv_, _id_, CYGARC_REG_MII_BMCR, CYGARC_REG_MII_BMCR_RENEGOTIATE);       \
  _MII_RENEGOTIATION_WAIT(_priv_, _id_);                                                \
  CYG_MACRO_END

// Wait for renegotiation to complete
// This function will delay for up to 2 seconds waiting for a renegotiation
// of the link.
#define _MII_RENEGOTIATION_WAIT(_priv_, _id_)                                           \
  CYG_MACRO_START                                                                       \
  int _i;                                                                               \
  cyg_uint16 _r;                                                                        \
  for (_i = 0; _i < 2000; _i++) {                                                       \
      HAL_DELAY_US(1000);                                                               \
      _r = _MII_READ(_priv_, _id_, CYGARC_REG_MII_BMSR);                                \
      if (_r & CYGARC_REG_MII_BMSR_AN_COMPLETE) break;                                  \
  }                                                                                     \
  CYG_MACRO_END

// Link state query
#define _MII_LINK_STATE(_priv_, _id_)                                           \
  ({ int _i;                                                                    \
  if (_MII_READ(_priv_, _id_, CYGARC_REG_MII_BMSR) & CYGARC_REG_MII_BMSR_LINK)  \
      (_i) = 1;                                                                 \
  else                                                                          \
      (_i) = 0;                                                                 \
  _i; })

// Clear 100MB capability advertisement bits
// Caller has to start a new renegoatiation
#define _MII_SPEED_FORCE_10MB(_priv_, _id_)                                         \
  CYG_MACRO_START                                                                   \
  cyg_uint16 _d;                                                                    \
  _d = _MII_READ(_priv_, _id_, CYGARC_REG_MII_ANAR);                                \
  _d &= ~(CYGARC_REG_MII_ANAR_T4|CYGARC_REG_MII_ANAR_TX_FD|CYGARC_REG_MII_ANAR_TX); \
  _MII_WRITE(_priv_, _id_, CYGARC_REG_MII_ANAR, _d);                                \
  CYG_MACRO_END

#ifdef _MII_HAS_EXTENDED

// Speed state query
// 0 = 10Mb, 1 = 100Mb
#define _MII_SPEED_STATE(_priv_, _id_)                                                  \
  ({ int _i;                                                                            \
  if (_MII_READ(_priv_, _id_, CYGARC_REG_MII_PHYSTS) & CYGARC_REG_MII_PHYSTS_SPEED)     \
      (_i) = 0;                                                                         \
  else                                                                                  \
      (_i) = 1;                                                                         \
  _i; })

// Duplex state query
// 0 = simplex, 1 = duplex
#define _MII_DUPLEX_STATE(_priv_, _id_)                                                 \
  ({ int _i;                                                                            \
  if (_MII_READ(_priv_, _id_, CYGARC_REG_MII_PHYSTS) & CYGARC_REG_MII_PHYSTS_DUPLEX)    \
      (_i) = 1;                                                                         \
  else                                                                                  \
      (_i) = 0;                                                                         \
  _i; })

#endif // _MII_HAS_EXTENDED

// Dump registers
// Useful for debugging
#define _MII_DUMP_REGS(_priv_, _id_)                                    \
  CYG_MACRO_START                                                       \
  int _i;                                                               \
  diag_printf("PHY registers:\n");                                      \
  for(_i = 0; _i <= CYGARC_REG_MII_LAST; _i++)                          \
      diag_printf(" %02d %04x\n", _i, _MII_READ(_priv_, _id_, _i));     \
  CYG_MACRO_END

//-----------------------------------------------------------------------------
// Phyter Registers

// Generic
#define CYGARC_REG_MII_BMCR              0x00
#define CYGARC_REG_MII_BMSR              0x01
#define CYGARC_REG_MII_PHYIDR1           0x02
#define CYGARC_REG_MII_PHYIDR2           0x03
#define CYGARC_REG_MII_ANAR              0x04
#define CYGARC_REG_MII_ANLPAR            0x05
#define CYGARC_REG_MII_ANER              0x06
#define CYGARC_REG_MII_ANNPTR            0x07

#define CYGARC_REG_MII_BMCR_RESET        0x8000
#define CYGARC_REG_MII_BMCR_LOOPBACK     0x4000
#define CYGARC_REG_MII_BMCR_RENEGOTIATE  0x3300

#define CYGARC_REG_MII_BMSR_AN_COMPLETE  0x0020
#define CYGARC_REG_MII_BMSR_LINK         0x0004

#define CYGARC_REG_MII_ANAR_T4           0x0200
#define CYGARC_REG_MII_ANAR_TX_FD        0x0100
#define CYGARC_REG_MII_ANAR_TX           0x0080
#define CYGARC_REG_MII_ANAR_10_FD        0x0040
#define CYGARC_REG_MII_ANAR_10           0x0020

// Extended registers
#ifndef _MII_HAS_EXTENDED
# define CYGARC_REG_MII_LAST             0x0f
#else
# define CYGARC_REG_MII_PHYSTS           0x10
# define CYGARC_REG_MII_FCSCR            0x14
# define CYGARC_REG_MII_RECR             0x15
# define CYGARC_REG_MII_PCSR             0x16
# define CYGARC_REG_MII_PHYCTRL          0x19
# define CYGARC_REG_MII_10BTSCR          0x1a
# define CYGARC_REG_MII_CDCTRL           0x1b
# define CYGARC_REG_MII_LAST             0x1f


# define CYGARC_REG_MII_PHYSTS_SPEED     0x00000002
# define CYGARC_REG_MII_PHYSTS_DUPLEX    0x00000004

#endif

#endif // CYGONCE_DEVS_ETH_PHYTER_PHYTER_INL
