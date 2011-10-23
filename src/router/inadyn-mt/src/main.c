/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*
	Small cmd line program useful for maintaining an IP address in a Dynamic DNS system.
	
	Author: Narcis Ilisei
	Date: April 2003
	
	History:
		May 1 2003 - project begin.
			- intendend to be used only with dyndns.org service
		May 8 2003 - ver. 0.9
			- first version - hard coded parameters
		May 18 2003 : ver. 0.95
			- cmd line option parsing added.
        June 2003
            - pSOS support added.
            - no DNS lookup supported -> IPs of the server as parameters			
        November 2003 
            - makefile review
            - DBG print modified  
		Nov/Dec 2007
			- log FILE parameter added to main (Win32)
			- debugging output (Win32)
*/

#define MODULE_TAG "MAIN: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include "debug_if.h"
#include "errorcode.h"
#include "dyndns.h"

#ifdef _WIN32

#include "debug_service.h"

#endif

/* MAIN - Dyn DNS update entry point.*/

#ifndef _WIN32

int inadyn_main(int argc, char* argv[])

#else

int inadyn_main(int argc, char* argv[],FILE *pLOG_FILE)

#endif

{
	RC_TYPE        rc        = RC_OK;
	DYN_DNS_CLIENT *p_dyndns = NULL;

#ifdef _WIN32

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Entered inadyn_main...\n"));

#endif

	do
	{
		/* create DYN_DNS_CLIENT object	*/

#ifdef _WIN32

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Constructing dyndns structure...\n"));

#endif

		rc = dyn_dns_construct(&p_dyndns);
		if (rc != RC_OK)
		{
			break;
		}

#ifdef _WIN32

		DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Calling dyn_dns_main...\n"));

#endif

		rc = dyn_dns_main(p_dyndns, argc, argv);
	}
	while(0);

#ifdef _WIN32

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "Returned from dyn_dns_main...\n"));

#endif

	/* end of program */

	if (rc != 0)
	{
		print_help_page();

#ifndef _WIN32

		/* log error*/
		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Main: Error '%s' (0x%x).\n", errorcode_get_name(rc), rc));
#else

		DBG_SERVICE_PRINTF((pLOG_FILE,"W:" MODULE_TAG "Main: Error %s (0x%x).\n", errorcode_get_name(rc), rc));
#endif

	}

	/* destroy DYN_DNS_CLIENT object*/

	rc = dyn_dns_destruct(p_dyndns);

	if (rc != RC_OK)
	{

#ifndef _WIN32

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Main: Error '%s' (0x%x) in dyn_dns_destruct().\n", errorcode_get_name(rc), rc));

#else
		DBG_SERVICE_PRINTF((pLOG_FILE,"W:" MODULE_TAG "Main: Error '%s' (0x%x) in dyn_dns_destruct().\n", errorcode_get_name(rc), rc));

#endif

	}

#ifdef _WIN32

	DBG_SERVICE_PRINTF((pLOG_FILE,"I:" MODULE_TAG "inadyn_main, returning to service dispatcher...\n"));
#endif

	return (int) rc;
}
