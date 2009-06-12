#ifndef CYGONCE_ECOS_MW_APP_H_
#define CYGONCE_ECOS_MW_APP_H_
//==========================================================================
//
//      ecos_mw_app.h
//
//      API for plug-in MicroWindows applications
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas
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
// Date:         2003-08-30
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_tables.h>

#define STACKSIZE ( 65536 )
typedef void fun(CYG_ADDRWORD);
struct _mw_app_entry {
    char         *name;
    fun          *entry;
    int          prio;
    fun          *init;
    cyg_handle_t t;
    cyg_thread   t_obj;
    char         stack[STACKSIZE];
} CYG_HAL_TABLE_TYPE;

#define _mw_app(_name_,_id_,_pri_,_init_)               \
externC void _id_##_thread(CYG_ADDRWORD data);          \
struct _mw_app_entry _mw_app_##_pri_##_##_id_           \
   CYG_HAL_TABLE_QUALIFIED_ENTRY(_mw_apps,_pri_) =      \
     { _name_, _id_##_thread, _pri_, _init_}; 

#define ECOS_MW_STARTUP_PRIORITY 11
#define ECOS_MW_NANOX_PRIORITY   (ECOS_MW_STARTUP_PRIORITY+1)
#define ECOS_MW_KND_PRIORITY     (ECOS_MW_STARTUP_PRIORITY+2)
#define ECOS_MW_NANOWM_PRIORITY  (ECOS_MW_STARTUP_PRIORITY+4)
#define ECOS_MW_APP_PRIORITY     (ECOS_MW_STARTUP_PRIORITY+5)

#endif  // CYGONCE_ECOS_MW_APP_H_
// ------------------------------------------------------------------------
