//==========================================================================
//
//      dload.cxx
//
//      Dynamic loader API
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-11-03
// Purpose:             Loader class implementation
// Description:         This file contains the dynamic ELF loader API.
//                      This presents the standard dlxxx() calls by invoking
//                      the loader classes as appropriate.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/isoinfra.h>

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <string.h>

#include <cyg/loader/loader.hxx>        // Loader header

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>                      // API definitions

// =========================================================================
// Cyg_LoaderStream_File class

// =========================================================================
// API calls

cyg_uint32 global_symbol_object;

// =========================================================================
// Load and open object

__externC void *dlopen (const char *file, int mode)
{
    CYG_REPORT_FUNCTION();

    void *ret = NULL;

    if( file == NULL )
    {
        // Special case to allow access to all symbols.

        ret = (void *)&global_symbol_object;
    }
#ifdef CYGPKG_IO_FILEIO    
    else
    {
        int fd = open( file, O_RDONLY );

        if( fd < 0 )
            return NULL;
        
        Cyg_LoaderStream_File filestream( fd );

        Cyg_LoadObject *obj;

        cyg_code error = Cyg_Loader::loader->load( filestream, mode, &obj );

        if( error == 0)
        {
            ret = (void *)obj;
        }

        close( fd );
    }
#endif
    
    CYG_REPORT_RETVAL(ret);    
    return ret;
}

// =========================================================================

__externC void *dlopenmem(const void *addr, size_t size, int mode)
{
    CYG_REPORT_FUNCTION();    

    void *ret = NULL;

    Cyg_LoaderStream_Mem memstream( addr, size );

    Cyg_LoadObject *obj;

    cyg_code error = Cyg_Loader::loader->load( memstream, mode, &obj );

    if( error == 0)
        ret = (void *)obj;
    
    CYG_REPORT_RETVAL(ret);    
    return ret;
}

// =========================================================================

__externC int dlclose (void *handle)
{
    CYG_REPORT_FUNCTION();

    int ret = 0;

    if( handle == (void *)global_symbol_object )
    {
        // Nothing to do here...
    }
    else
    {
        Cyg_LoadObject *obj = (Cyg_LoadObject *)handle;

        Cyg_Loader::loader->close( obj );
    }
    
    CYG_REPORT_RETVAL(ret);    
    return ret;
}

// =========================================================================

__externC void *dlsym (void *handle, const char *name)
{
    CYG_REPORT_FUNCTION();

    void *ret = NULL;

    Cyg_LoadObject *obj = (Cyg_LoadObject *)handle;

    ret = obj->symbol( name );
    
    CYG_REPORT_RETVAL(ret);    
    return ret;
}

// =========================================================================

__externC const char *dlerror (void)
{
    CYG_REPORT_FUNCTION();

    const char *ret = Cyg_Loader::loader->error_string();

    CYG_REPORT_RETVAL(ret);
    return ret;
}


// =========================================================================
// EOF dload.cxx
