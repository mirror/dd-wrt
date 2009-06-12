//==========================================================================
//
//      flash_config.h
//
//      Flash configuration data tables for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Date:         2000-08-21
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _FLASH_CONFIG_H_
#define _FLASH_CONFIG_H_

#define MAX_SCRIPT_LENGTH CYGNUM_REDBOOT_FLASH_SCRIPT_SIZE
#define MAX_STRING_LENGTH CYGNUM_REDBOOT_FLASH_STRING_SIZE
#define MAX_CONFIG_DATA   CYGNUM_REDBOOT_FLASH_CONFIG_SIZE

#define CONFIG_EMPTY   0
#define CONFIG_BOOL    1
#define CONFIG_INT     2
#define CONFIG_STRING  3
#define CONFIG_SCRIPT  4
#ifdef CYGPKG_REDBOOT_NETWORKING
#define CONFIG_IP      5
#define CONFIG_ESA     6
#define CONFIG_NETPORT 7
#endif

struct config_option {
    char *key;
    char *title;
    char *enable;
    bool  enable_sense;
    int   type;
    unsigned long dflt;
} CYG_HAL_TABLE_TYPE;

#define ALWAYS_ENABLED (char *)0

#define RedBoot_config_option(_t_,_n_,_e_,_ie_,_type_,_dflt_)        \
struct config_option _config_option_##_n_                               \
CYG_HAL_TABLE_QUALIFIED_ENTRY(RedBoot_config_options,_n_) =             \
   {#_n_,_t_,_e_,_ie_,_type_,(unsigned long)_dflt_};

// Cause the in-memory configuration data to be written to flash
void flash_write_config(bool prompt);
// Fetch a data item from flash storage, returns 'false' if not found
bool flash_get_config(char *key, void *val, int type);
// Update a data item from flash storage, returns 'false' if not found
bool flash_set_config(char *key, void *val, int type);
// Enumerate keys from configuration
bool flash_next_key(char *key, int keylen, int *type, int *offset);
// Add a new data item to configuration data base.  Returns 'false'
// if no space is available.
bool flash_add_config(struct config_option *opt, bool update);

// Internal structure used to hold config data
struct _config {
    unsigned long len;
    unsigned long key1;
    unsigned char config_data[MAX_CONFIG_DATA-(4*4)];
    unsigned long key2;
    unsigned long cksum;
};

#endif // _FLASH_CONFIG_H_
