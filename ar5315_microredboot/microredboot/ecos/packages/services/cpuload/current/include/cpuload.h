//==========================================================================
//
//      cpuload.h
//
//      Interface for the cpuload code.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Andrew Lunn
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
// Author(s):    Andrew Lunn
// Contributors: Andrew Lunn
// Date:         2002-08-12
// Purpose:      
// Description:  
//      API for the cpuload measurement code
//        
// This code is part of eCos (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _SERVICES_CPULOAD_CPULOAD_H
#define _SERVICES_CPULOAD_CPULOAD_H

#ifdef __cplusplus
externC 
{
#endif  

typedef struct cyg_cpuload_s {
  cyg_alarm alarm_s;
  cyg_handle_t alarmH;
  cyg_uint32 last_idle_loops;
  cyg_uint32 average_point1s;
  cyg_uint32 average_1s;
  cyg_uint32 average_10s;
  cyg_uint32 calibration;
} cyg_cpuload_t;

/* Calibrate the CPU load measurement code */
void cyg_cpuload_calibrate(cyg_uint32  *calibration);

/* Create a measurement object and start the measurements */
void cyg_cpuload_create(cyg_cpuload_t *cpuload, 
                        cyg_uint32 calibrate,
                        cyg_handle_t *handle);

/* Stop the measurements so the object can be freed */
void cyg_cpuload_delete(cyg_handle_t handle);

/* Get the current CPU load */
void cyg_cpuload_get(cyg_handle_t handle,
		 cyg_uint32 *average_point1s, 	    
		 cyg_uint32 *average_1s, 	    
		 cyg_uint32 *average_10s);

#ifdef __cplusplus
}
#endif
#endif /* _SERVICES_CPULOAD_CPULOAD_H */
