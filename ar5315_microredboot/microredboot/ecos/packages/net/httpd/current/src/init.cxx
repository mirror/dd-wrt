/* =================================================================
 *
 *      init.cxx
 *
 *      Constructor for HTTPD
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating
 * System.
 * Copyright (C) 2002 Nick Garnett.
 * 
 * eCos is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or (at your option)
 * any later version.
 * 
 * eCos is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with eCos; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * As a special exception, if other files instantiate templates or
 * use macros or inline functions from this file, or you compile this
 * file and link it with other works to produce a work based on this
 * file, this file does not by itself cause the resulting work to be
 * covered by the GNU General Public License. However the source code
 * for this file must still be made available in accordance with
 * section (3) of the GNU General Public License.
 * 
 * This exception does not invalidate any other reasons why a work
 * based on this file might be covered by the GNU General Public
 * License.
 *
 * -------------------------------------------
 * ####ECOSGPLCOPYRIGHTEND####
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    nickg@calivar.com
 *  Contributors: nickg@calivar.com
 *  Date:         2002-10-14
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/httpd.h>

#ifdef CYGNUM_HTTPD_SERVER_AUTO_START

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

/* ================================================================= */

__externC void cyg_httpd_startup(void);

/* ================================================================= */
/* Initialization object
 */

class Cyg_Httpd_Init_Class
{
public:
    Cyg_Httpd_Init_Class();
};

/* ----------------------------------------------------------------- */
/* Static initialization object instance. The constructor is
 * prioritized to run at the same time as any filesystem constructors.
 */
static Cyg_Httpd_Init_Class httpd_initializer CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);

/* ----------------------------------------------------------------- */
/* Constructor, just calls the startup routine.
 */

Cyg_Httpd_Init_Class::Cyg_Httpd_Init_Class()
{
    cyg_httpd_startup();
}

#endif // CYGNUM_HTTPD_SERVER_AUTO_START

/* ----------------------------------------------------------------- */
/* end of httpd.c                                                    */
