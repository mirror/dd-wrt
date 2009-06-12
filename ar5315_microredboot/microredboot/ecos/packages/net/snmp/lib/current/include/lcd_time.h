//==========================================================================
//
//      ./lib/current/include/lcd_time.h
//
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
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/*
 * lcd_time.h
 */

#ifndef _LCD_TIME_H
#define _LCD_TIME_H

#ifdef __cplusplus
extern "C" {
#endif


/* undefine to enable time synchronization only on authenticated packets */
#define LCD_TIME_SYNC_OPT 1

/*
 * Macros and definitions.
 */
#define ETIMELIST_SIZE	23



typedef struct enginetime_struct {
	u_char		*engineID;	
	u_int		 engineID_len;

	u_int		 engineTime;	
	u_int		 engineBoot;
		/* Time & boots values received from last authenticated
		 *   message within the previous time window.
		 */

	time_t		 lastReceivedEngineTime;
		/* Timestamp made when engineTime/engineBoots was last
		 *   updated.  Measured in seconds.
		 */

#ifdef LCD_TIME_SYNC_OPT	
        u_int  authenticatedFlag;
#endif      
	struct enginetime_struct	*next;
} enginetime, *Enginetime;




/*
 * Macros for streamlined engineID existence checks --
 *
 *	e	is char *engineID,
 *	e_l	is u_int engineID_len.
 *
 *
 *  ISENGINEKNOWN(e, e_l)
 *	Returns:
 *		TRUE	If engineID is recoreded in the EngineID List;
 *		FALSE	Otherwise.
 *
 *  ENSURE_ENGINE_RECORD(e, e_l)
 *	Adds the given engineID to the EngineID List if it does not exist
 *		already.  engineID is added with a <enginetime, engineboots>
 *		tuple of <0,0>.  ALWAYS succeeds -- except in case of a
 *		fatal internal error.
 *	Returns:
 *		SNMPERR_SUCCESS	On success;
 *		SNMPERR_GENERR	Otherwise.
 *
 *  MAKENEW_ENGINE_RECORD(e, e_l)
 *	Returns:
 *		SNMPERR_SUCCESS	If engineID already exists in the EngineID List;
 *		SNMPERR_GENERR	Otherwise -and- invokes ENSURE_ENGINE_RECORD()
 *					to add an entry to the EngineID List.
 *
 * XXX  Requres the following declaration in modules calling ISENGINEKNOWN():
 *	static u_int	dummy_etime, dummy_eboot;
 */
#define ISENGINEKNOWN(e, e_l)					\
	( (get_enginetime(e, e_l,				\
		&dummy_eboot, &dummy_etime, TRUE) == SNMPERR_SUCCESS)	\
		? TRUE						\
		: FALSE )

#define ENSURE_ENGINE_RECORD(e, e_l)				\
	( (set_enginetime(e, e_l, 0, 0, FALSE) == SNMPERR_SUCCESS)	\
		? SNMPERR_SUCCESS				\
		: SNMPERR_GENERR )

#define MAKENEW_ENGINE_RECORD(e, e_l)				\
	( (ISENGINEKNOWN(e, e_l) == TRUE)			\
		? SNMPERR_SUCCESS				\
		: (ENSURE_ENGINE_RECORD(e, e_l), SNMPERR_GENERR) )



/*
 * Prototypes.
 */
int	 get_enginetime (u_char *engineID, u_int  engineID_len,
			     u_int *engine_boot, u_int *engine_time,
			     u_int authenticated);

int	 get_enginetime_ex (u_char *engineID, u_int  engineID_len,
			    u_int *engine_boot, u_int *engine_time,
			    u_int *last_engine_time,
			    u_int authenticated);

int	 set_enginetime (u_char *engineID, u_int engineID_len,
			     u_int   engine_boot, u_int engine_time,
			     u_int authenticated);

Enginetime
	 search_enginetime_list (	u_char		*engineID,
					u_int		 engineID_len);

int	 hash_engineID (u_char *engineID, u_int engineID_len);

void	 dump_etimelist_entry (Enginetime e, int count);
void	 dump_etimelist (void);

#ifdef __cplusplus
}
#endif

#endif /* _LCD_TIME_H */
