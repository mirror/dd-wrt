/*
 * Copyright (C) 2001 Nikos Mavroyanopoulos
 *
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef LIBDEFS_H
#define LIBDEFS_H
#include <libdefs.h>
#endif

#ifdef DLMUTEX

static void _mcrypt_mutex_seterror (const char *ERROR) {
 return; /* do nothing */
}
static const char * _mcrypt_mutex_geterror (void) {
 /* return no error */
 return NULL;
}

WIN32DLL_DEFINE
int mcrypt_mutex_register ( void (*mutex_lock)(void) , 
			void (*mutex_unlock)(void)) 
{

	return lt_dlmutex_register ( mutex_lock, mutex_unlock,
		_mcrypt_mutex_seterror, _mcrypt_mutex_geterror);

}
#else
WIN32DLL_DEFINE
int mcrypt_mutex_register ( void (*mutex_lock)(void) ,
                        void (*mutex_unlock)(void))   
                        {
                        return 0;
}
#endif /* NO_DLMUTEX */
