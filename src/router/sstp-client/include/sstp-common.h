/*!
 * @brief Provide common declarations for the sstp project
 *
 * @file sstp-common.h
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __SSTP_COMMON_H__
#define __SSTP_COMMON_H__

/*!
 * @brief Common return values
 */
typedef enum
{
    /*!< Generic failure */
    SSTP_FAIL   = -1,

    /*!< General okay */
    SSTP_OKAY   =  0,
    
    /*!< Operation in progress */
    SSTP_INPROG = 1,

    /*!< Socket connected */
    SSTP_CONNECTED = 2,

    /*!< Buffer overflow */
    SSTP_OVERFLOW = 3,

    /*!< Not implemented (yet) */
    SSTP_NOTIMPL = 4,

    /*!< Operation timed out */
    SSTP_TIMEOUT = 5,

    /*!< Authentication required */
    SSTP_AUTHENTICATE = 6,

} status_t;

#endif	/* #ifndef __SSTP_COMMON_H__ */
