/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Daniel Pittman and Christian Grothoff
     Copyright (C) 2015-2021 Karlson2k (Evgeny Grin)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file response.h
 * @brief  Methods for managing response objects
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef RESPONSE_H
#define RESPONSE_H

/**
 * Increments the reference counter for the @a response.
 *
 * @param response object to modify
 */
void
MHD_increment_response_rc (struct MHD_Response *response);


/**
 * We are done sending the header of a given response
 * to the client.  Now it is time to perform the upgrade
 * and hand over the connection to the application.
 * @remark To be called only from thread that process connection's
 * recv(), send() and response. Must be called right after sending
 * response headers.
 *
 * @param response the response that was created for an upgrade
 * @param connection the specific connection we are upgrading
 * @return #MHD_YES on success, #MHD_NO on failure (will cause
 *        connection to be closed)
 */
enum MHD_Result
MHD_response_execute_upgrade_ (struct MHD_Response *response,
                               struct MHD_Connection *connection);


/**
 * Get a particular header (or footer) element from the response.
 *
 * Function returns the first found element.
 * @param response response to query
 * @param kind the kind of element: header or footer
 * @param key the key which header to get
 * @param key_len the length of the @a key
 * @return NULL if header element does not exist
 * @ingroup response
 */
struct MHD_HTTP_Header *
MHD_get_response_element_n_ (struct MHD_Response *response,
                             enum MHD_ValueKind kind,
                             const char *key,
                             size_t key_len);

#endif
