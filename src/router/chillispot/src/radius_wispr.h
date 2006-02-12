/* 
 * Radius client functions.
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */

#ifndef _RADIUS_WISPR_H
#define _RADIUS_WISPR_H

#define RADIUS_VENDOR_WISPR                         14122
#define	RADIUS_ATTR_WISPR_LOCATION_ID	                1
#define	RADIUS_ATTR_WISPR_LOCATION_NAME		        2 /* string */
#define	RADIUS_ATTR_WISPR_LOGOFF_URL		        3 /* string */
#define	RADIUS_ATTR_WISPR_REDIRECTION_URL		4 /* string */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MIN_UP		5 /* integer */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MIN_DOWN	        6 /* integer */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP		7 /* integer */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN	        8 /* integer */
#define	RADIUS_ATTR_WISPR_SESSION_TERMINATE_TIME	9 /* string */
#define	RADIUS_ATTR_WISPR_SESSION_TERMINATE_END_OF_DAY 10 /* string */
#define	RADIUS_ATTR_WISPR_BILLING_CLASS_OF_SERVICE     11 /* string */

#endif	/* !_RADIUS_WISPR_H */
