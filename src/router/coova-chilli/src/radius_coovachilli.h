/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RADIUS_COOVACHILLI_H
#define _RADIUS_COOVACHILLI_H

#define RADIUS_VENDOR_COOVACHILLI                      14559
#define	RADIUS_ATTR_COOVACHILLI_MAX_INPUT_OCTETS           1 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_MAX_OUTPUT_OCTETS          2 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_MAX_TOTAL_OCTETS           3 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_BANDWIDTH_MAX_UP	          4 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_BANDWIDTH_MAX_DOWN         5 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_CONFIG                     6 /* string */
#define RADIUS_ATTR_COOVACHILLI_LANG                       7 /* string */
#define RADIUS_ATTR_COOVACHILLI_VERSION                    8 /* string */
#define RADIUS_ATTR_COOVACHILLI_ORIGINALURL                9 /* string */
#define RADIUS_ATTR_COOVACHILLI_ACCT_VIEW_POINT           10 /* integer */
#define RADIUS_ATTR_COOVACHILLI_REQUIRE_UAM               11 /* string */
#define RADIUS_ATTR_COOVACHILLI_REQUIRE_SPLASH            12 /* string */
#define RADIUS_ATTR_COOVACHILLI_ROUTE_TO_INTERFACE        13 /* integer */
#define RADIUS_ATTR_COOVACHILLI_CONFIG_FILE               14 /* string */
#define RADIUS_ATTR_COOVACHILLI_SESSION_STATE             15 /* integer */
#define RADIUS_ATTR_COOVACHILLI_SESSION_ID                16 /* string */
#define RADIUS_ATTR_COOVACHILLI_AP_SESSION_ID             17 /* string */
#define RADIUS_ATTR_COOVACHILLI_USER_AGENT                18 /* string */
#define RADIUS_ATTR_COOVACHILLI_ACCEPT_LANGUAGE           19 /* string */

#define	RADIUS_ATTR_COOVACHILLI_MAX_INPUT_GIGAWORDS       21 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_MAX_OUTPUT_GIGAWORDS      22 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_MAX_TOTAL_GIGAWORDS       23 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_VLAN_ID                   24 /* integer */
#define	RADIUS_ATTR_COOVACHILLI_LOCATION                  25 /* string */
#define	RADIUS_ATTR_COOVACHILLI_OLD_LOCATION              26 /* string */
#define	RADIUS_ATTR_COOVACHILLI_LOCATION_CHANGE_COUNT     27 /* integer */

#define RADIUS_ATTR_COOVACHILLI_SYS_UPTIME                40 /* integer */
#define RADIUS_ATTR_COOVACHILLI_SYS_LOADAVG               41 /* string */
#define RADIUS_ATTR_COOVACHILLI_SYS_MEMORY                42 /* string */

#define RADIUS_ATTR_COOVACHILLI_DHCP_VENDOR_CLASS_ID      50 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_CLIENT_ID            51 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_OPTION               52 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_FILENAME             53 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_HOSTNAME             54 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_SERVER_NAME          55 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_CLIENT_FQDN          56 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_PARAMETER_REQUEST_LIST 57 /* string */

#define RADIUS_ATTR_COOVACHILLI_DHCP_IP_ADDRESS           60 /* ipaddr */
#define RADIUS_ATTR_COOVACHILLI_DHCP_IP_NETMASK           61 /* ipaddr */
#define RADIUS_ATTR_COOVACHILLI_DHCP_DNS1                 62 /* ipaddr */
#define RADIUS_ATTR_COOVACHILLI_DHCP_DNS2                 63 /* ipaddr */
#define RADIUS_ATTR_COOVACHILLI_DHCP_GATEWAY              64 /* ipaddr */
#define RADIUS_ATTR_COOVACHILLI_DHCP_DOMAIN               65 /* string */
#define RADIUS_ATTR_COOVACHILLI_DHCP_RELAY                66 /* ipaddr */

#define RADIUS_ATTR_COOVACHILLI_INJECT_URL                70 /* string */

#define RADIUS_ATTR_COOVACHILLI_POSTAUTHPROXY_ADDRESS     75 /* ipaddr */
#define RADIUS_ATTR_COOVACHILLI_POSTAUTHPROXY_PORT        76 /* integer */

#define RADIUS_ATTR_COOVACHILLI_GARDEN_INPUT_OCTETS       80 /* integer */
#define RADIUS_ATTR_COOVACHILLI_GARDEN_OUTPUT_OCTETS      81 /* integer */
#define RADIUS_ATTR_COOVACHILLI_GARDEN_INPUT_GIGAWORDS    82 /* integer */
#define RADIUS_ATTR_COOVACHILLI_GARDEN_OUTPUT_GIGAWORDS   83 /* integer */
#define RADIUS_ATTR_COOVACHILLI_OTHER_INPUT_OCTETS        84 /* integer */
#define RADIUS_ATTR_COOVACHILLI_OTHER_OUTPUT_OCTETS       85 /* integer */
#define RADIUS_ATTR_COOVACHILLI_OTHER_INPUT_GIGAWORDS     86 /* integer */
#define RADIUS_ATTR_COOVACHILLI_OTHER_OUTPUT_GIGAWORDS    87 /* integer */

#define RADIUS_VALUE_COOVACHILLI_NAS_VIEWPOINT             1
#define RADIUS_VALUE_COOVACHILLI_CLIENT_VIEWPOINT          2

#define RADIUS_VALUE_COOVACHILLI_SESSION_AUTH              1
#define RADIUS_VALUE_COOVACHILLI_SESSION_NOAUTH            2
#define RADIUS_VALUE_COOVACHILLI_SESSION_STARTED           3
#define RADIUS_VALUE_COOVACHILLI_SESSION_STOPPED           4
#define RADIUS_VALUE_COOVACHILLI_SESSION_USER_LOGOUT_URL          10
#define RADIUS_VALUE_COOVACHILLI_SESSION_IDLE_TIMEOUT_REACHED     11
#define RADIUS_VALUE_COOVACHILLI_SESSION_TIMEOUT_REACHED          12
#define RADIUS_VALUE_COOVACHILLI_SESSION_LOGOUT_TIME_REACHED      13
#define RADIUS_VALUE_COOVACHILLI_SESSION_IN_DATALIMIT_REACHED     14
#define RADIUS_VALUE_COOVACHILLI_SESSION_OUT_DATALIMIT_REACHED    15
#define RADIUS_VALUE_COOVACHILLI_SESSION_TOTAL_DATALIMIT_REACHED  16
#define RADIUS_VALUE_COOVACHILLI_SESSION_LOCATION_CHANGE          17

#endif	/* !_RADIUS_COOVACHILLI_H */
