/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


/*******************************************************************
 * -mon_server.h
 *
 * BL framework - remote shell support
 *
 * This module is a back-end of remote shell support.
 * - multiple servers
 * - domain and TCP-based connections
 * - session access level - per server
 *******************************************************************/

#ifndef BDMF_MON_SERVER_H_
#define BDMF_MON_SERVER_H_

#include <bdmf_system.h>
#include <bdmf_session.h>
#include <bdmf_shell.h>

/** Shell server transport type
 */
typedef enum {
    BDMFMONS_TRANSPORT_DOMAIN_SOCKET,
    BDMFMONS_TRANSPORT_TCP_SOCKET,

    BDMFMONS_TRANSPORT__NUMBER_OF
} bdmfmons_transport_type_t;

/** Shell server parameters
 */
typedef struct bdmfmons_parm
{
    bdmf_access_right_t access;           /**< Access rights */
    bdmfmons_transport_type_t transport;  /**< Transport type */
    char *address;                      /**< Address in string form: domain socket file in local FS; port for TCP socket */
    int max_clients;                    /**< Max number of clients */
} bdmfmons_parm_t;


/** Create shell server.
 * Immediately after creation server is ready to accept client connections
 * \param[in]   parms   Server parameters
 * \param[out]  hs      Server handle
 * \return  0 - OK\n
 *         <0 - error code
 */
bdmf_error_t bdmfmons_server_create(const bdmfmons_parm_t *parms, int *hs);

/** Destroy shell server.
 * All client connections if any are closed
 * \param[in]   hs      Server handle
 * \return  0 - OK\n
 *         <0 - error code
 */
bdmf_error_t bdmfmons_server_destroy(int hs);


/* Create shell_server directory in root_dir
   Returns the "shell_server" directory handle
*/
bdmfmon_handle_t bdmfmons_server_mon_init(bdmfmon_handle_t root_dir);


/* Destroy shell_server directory
*/
void bdmfmons_server_mon_destroy(void);

#endif /* BDMF_MON_SERVER_H_ */
