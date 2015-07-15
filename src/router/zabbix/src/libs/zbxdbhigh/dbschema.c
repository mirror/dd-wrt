/*
** Zabbix
** Copyright (C) 2001-2015 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "common.h"
#include "dbschema.h"

const ZBX_TABLE	tables[] = {

#if defined(HAVE_IBM_DB2) || defined(HAVE_ORACLE)
#	define ZBX_TYPE_SHORTTEXT_LEN	2048
#else
#	define ZBX_TYPE_SHORTTEXT_LEN	65535
#endif

#if defined(HAVE_IBM_DB2)
#	define ZBX_TYPE_LONGTEXT_LEN	2048
#	define ZBX_TYPE_TEXT_LEN	2048
#else
#	define ZBX_TYPE_LONGTEXT_LEN	0
#	define ZBX_TYPE_TEXT_LEN	65535
#endif

	{"maintenances",	"maintenanceid",	ZBX_SYNC,
		{
		{"maintenanceid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"maintenance_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"description",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"active_since",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"active_till",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"hosts",	"hostid",	ZBX_SYNC,
		{
		{"hostid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"proxy_hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"host",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"disable_until",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"available",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"errors_from",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"lastaccess",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ipmi_authtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ipmi_privilege",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ipmi_username",	"",	NULL,	NULL,	16,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ipmi_password",	"",	NULL,	NULL,	20,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ipmi_disable_until",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ipmi_available",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"snmp_disable_until",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"snmp_available",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"maintenanceid",	NULL,	"maintenances",	"maintenanceid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"maintenance_status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"maintenance_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"maintenance_from",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ipmi_errors_from",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"snmp_errors_from",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ipmi_error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"snmp_error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"jmx_disable_until",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"jmx_available",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"jmx_errors_from",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"jmx_error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"flags",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"templateid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		NULL
	},
	{"groups",	"groupid",	ZBX_SYNC,
		{
		{"groupid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"internal",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"flags",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"group_prototype",	"group_prototypeid",	ZBX_SYNC,
		{
		{"group_prototypeid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"templateid",	NULL,	"group_prototype",	"group_prototypeid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		NULL
	},
	{"group_discovery",	"groupid",	ZBX_SYNC,
		{
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"parent_group_prototypeid",	NULL,	"group_prototype",	"group_prototypeid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"lastcheck",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ts_delete",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"screens",	"screenid",	ZBX_SYNC,
		{
		{"screenid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	NULL,	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hsize",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"vsize",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"templateid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		NULL
	},
	{"screens_items",	"screenitemid",	ZBX_SYNC,
		{
		{"screenitemid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"screenid",	NULL,	"screens",	"screenid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"resourcetype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"resourceid",	"0",	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"width",	"320",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"height",	"200",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"x",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"y",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"colspan",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"rowspan",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"elements",	"25",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"valign",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"halign",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"style",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"dynamic",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"sort_triggers",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"application",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"slideshows",	"slideshowid",	ZBX_SYNC,
		{
		{"slideshowid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"delay",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"slides",	"slideid",	ZBX_SYNC,
		{
		{"slideid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"slideshowid",	NULL,	"slideshows",	"slideshowid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"screenid",	NULL,	"screens",	"screenid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"step",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"delay",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"drules",	"druleid",	ZBX_SYNC,
		{
		{"druleid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"proxy_hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"iprange",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"delay",	"3600",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"nextcheck",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"dchecks",	"dcheckid",	ZBX_SYNC,
		{
		{"dcheckid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"druleid",	NULL,	"drules",	"druleid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"key_",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmp_community",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ports",	"0",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_securityname",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_securitylevel",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_authpassphrase",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_privpassphrase",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"uniq",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_authprotocol",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_privprotocol",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_contextname",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		NULL
	},
	{"applications",	"applicationid",	ZBX_SYNC,
		{
		{"applicationid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"hostid,name"
	},
	{"httptest",	"httptestid",	ZBX_SYNC,
		{
		{"httptestid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"applicationid",	NULL,	"applications",	"applicationid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"nextcheck",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"delay",	"60",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"variables",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"agent",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"authentication",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"http_user",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"http_password",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"templateid",	NULL,	"httptest",	"httptestid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"http_proxy",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"retries",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		"hostid,name"
	},
	{"httpstep",	"httpstepid",	ZBX_SYNC,
		{
		{"httpstepid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"httptestid",	NULL,	"httptest",	"httptestid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"no",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"url",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"timeout",	"30",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"posts",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"required",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"status_codes",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"variables",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		NULL
	},
	{"interface",	"interfaceid",	ZBX_SYNC,
		{
		{"interfaceid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"main",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"useip",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ip",	"127.0.0.1",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"dns",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"port",	"10050",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		NULL
	},
	{"valuemaps",	"valuemapid",	ZBX_SYNC,
		{
		{"valuemapid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"items",	"itemid",	ZBX_SYNC,
		{
		{"itemid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmp_community",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmp_oid",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"key_",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"delay",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"history",	"90",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"trends",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"value_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"trapper_hosts",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"units",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"multiplier",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"delta",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"snmpv3_securityname",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_securitylevel",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_authpassphrase",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_privpassphrase",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"formula",	"1",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lastlogsize",	"0",	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"logtimefmt",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"templateid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"valuemapid",	NULL,	"valuemaps",	"valuemapid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"delay_flex",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"params",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"ipmi_sensor",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"data_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"authtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"username",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"password",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"publickey",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"privatekey",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"mtime",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"flags",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"filter",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"interfaceid",	NULL,	"interface",	"interfaceid",	0,	ZBX_TYPE_ID,	ZBX_SYNC | ZBX_PROXY,	0},
		{"port",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"description",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"inventory_link",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lifetime",	"30",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"snmpv3_authprotocol",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"snmpv3_privprotocol",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"state",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"snmpv3_contextname",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		"hostid,key_"
	},
	{"httpstepitem",	"httpstepitemid",	ZBX_SYNC,
		{
		{"httpstepitemid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"httpstepid",	NULL,	"httpstep",	"httpstepid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		"httpstepid,itemid"
	},
	{"httptestitem",	"httptestitemid",	ZBX_SYNC,
		{
		{"httptestitemid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"httptestid",	NULL,	"httptest",	"httptestid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		"httptestid,itemid"
	},
	{"media_type",	"mediatypeid",	ZBX_SYNC,
		{
		{"mediatypeid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"description",	"",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"smtp_server",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"smtp_helo",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"smtp_email",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"exec_path",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"gsm_modem",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"username",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"passwd",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"users",	"userid",	ZBX_SYNC,
		{
		{"userid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"alias",	"",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"name",	"",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"surname",	"",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"passwd",	"",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"autologin",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"autologout",	"900",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lang",	"en_GB",	NULL,	NULL,	5,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"refresh",	"30",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"type",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"theme",	"default",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"attempt_failed",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"attempt_ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"attempt_clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"rows_per_page",	"50",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"usrgrp",	"usrgrpid",	ZBX_SYNC,
		{
		{"usrgrpid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"gui_access",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"users_status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"debug_mode",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"users_groups",	"id",	ZBX_SYNC,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"usrgrpid",	NULL,	"usrgrp",	"usrgrpid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"usrgrpid,userid"
	},
	{"scripts",	"scriptid",	ZBX_SYNC,
		{
		{"scriptid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"command",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"host_access",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"usrgrpid",	NULL,	"usrgrp",	"usrgrpid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"description",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"confirmation",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"execute_on",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"actions",	"actionid",	ZBX_SYNC,
		{
		{"actionid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"eventsource",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"evaltype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"esc_period",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"def_shortdata",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"def_longdata",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"recovery_msg",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"r_shortdata",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"r_longdata",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"operations",	"operationid",	ZBX_SYNC,
		{
		{"operationid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"actionid",	NULL,	"actions",	"actionid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"operationtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"esc_period",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"esc_step_from",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"esc_step_to",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"evaltype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"opmessage",	"operationid",	ZBX_SYNC,
		{
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"default_msg",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"subject",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"message",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"mediatypeid",	NULL,	"media_type",	"mediatypeid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"opmessage_grp",	"opmessage_grpid",	ZBX_SYNC,
		{
		{"opmessage_grpid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"usrgrpid",	NULL,	"usrgrp",	"usrgrpid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"operationid,usrgrpid"
	},
	{"opmessage_usr",	"opmessage_usrid",	ZBX_SYNC,
		{
		{"opmessage_usrid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"operationid,userid"
	},
	{"opcommand",	"operationid",	ZBX_SYNC,
		{
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"scriptid",	NULL,	"scripts",	"scriptid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"execute_on",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"port",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"authtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"username",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"password",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"publickey",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"privatekey",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"command",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"opcommand_hst",	"opcommand_hstid",	ZBX_SYNC,
		{
		{"opcommand_hstid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"opcommand_grp",	"opcommand_grpid",	ZBX_SYNC,
		{
		{"opcommand_grpid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"opgroup",	"opgroupid",	ZBX_SYNC,
		{
		{"opgroupid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"operationid,groupid"
	},
	{"optemplate",	"optemplateid",	ZBX_SYNC,
		{
		{"optemplateid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"templateid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"operationid,templateid"
	},
	{"opconditions",	"opconditionid",	ZBX_SYNC,
		{
		{"opconditionid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"operationid",	NULL,	"operations",	"operationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"conditiontype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"operator",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"conditions",	"conditionid",	ZBX_SYNC,
		{
		{"conditionid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"actionid",	NULL,	"actions",	"actionid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"conditiontype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"operator",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"config",	"configid",	ZBX_SYNC,
		{
		{"configid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"refresh_unsupported",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"work_period",	"1-5,00:00-24:00",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"alert_usrgrpid",	NULL,	"usrgrp",	"usrgrpid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"event_ack_enable",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"event_expire",	"7",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"event_show_max",	"100",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"default_theme",	"originalblue",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"authentication_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ldap_host",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ldap_port",	"389",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ldap_base_dn",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ldap_bind_dn",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ldap_bind_password",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ldap_search_attribute",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"dropdown_first_entry",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"dropdown_first_remember",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"discovery_groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"max_in_table",	"50",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"search_limit",	"1000",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_color_0",	"DBDBDB",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_color_1",	"D6F6FF",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_color_2",	"FFF6A5",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_color_3",	"FFB689",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_color_4",	"FF9999",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_color_5",	"FF3838",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_name_0",	"Not classified",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_name_1",	"Information",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_name_2",	"Warning",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_name_3",	"Average",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_name_4",	"High",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_name_5",	"Disaster",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ok_period",	"1800",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"blink_period",	"1800",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"problem_unack_color",	"DC0000",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"problem_ack_color",	"DC0000",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ok_unack_color",	"00AA00",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ok_ack_color",	"00AA00",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"problem_unack_style",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"problem_ack_style",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ok_unack_style",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ok_ack_style",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"snmptrap_logging",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"server_check_interval",	"10",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_events_mode",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_events_trigger",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_events_internal",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_events_discovery",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_events_autoreg",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_services_mode",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_services",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_audit_mode",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_audit",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_sessions_mode",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_sessions",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_history_mode",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_history_global",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_history",	"90",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_trends_mode",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_trends_global",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hk_trends",	"365",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"triggers",	"triggerid",	ZBX_SYNC,
		{
		{"triggerid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"expression",	"",	NULL,	NULL,	2048,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"description",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"value",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"priority",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lastchange",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"comments",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"templateid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"state",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"flags",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"trigger_depends",	"triggerdepid",	ZBX_SYNC,
		{
		{"triggerdepid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"triggerid_down",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"triggerid_up",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"triggerid_down,triggerid_up"
	},
	{"functions",	"functionid",	ZBX_SYNC,
		{
		{"functionid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"triggerid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"function",	"",	NULL,	NULL,	12,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"parameter",	"0",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"graphs",	"graphid",	ZBX_SYNC,
		{
		{"graphid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"width",	"900",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"height",	"200",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"yaxismin",	"0",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"yaxismax",	"100",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"templateid",	NULL,	"graphs",	"graphid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"show_work_period",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"show_triggers",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"graphtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"show_legend",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"show_3d",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"percent_left",	"0",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"percent_right",	"0",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ymin_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ymax_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ymin_itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"ymax_itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"flags",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"graphs_items",	"gitemid",	ZBX_SYNC,
		{
		{"gitemid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"graphid",	NULL,	"graphs",	"graphid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"drawtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"sortorder",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"color",	"009600",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"yaxisside",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"calc_fnc",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"graph_theme",	"graphthemeid",	0,
		{
		{"graphthemeid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"description",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"theme",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"backgroundcolor",	"F0F0F0",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"graphcolor",	"FFFFFF",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"graphbordercolor",	"222222",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"gridcolor",	"CCCCCC",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"maingridcolor",	"AAAAAA",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"gridbordercolor",	"000000",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"textcolor",	"202020",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"highlightcolor",	"AA4444",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"leftpercentilecolor",	"11CC11",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"rightpercentilecolor",	"CC1111",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"nonworktimecolor",	"CCCCCC",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"gridview",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"legendview",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"globalmacro",	"globalmacroid",	ZBX_SYNC,
		{
		{"globalmacroid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"macro",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		NULL
	},
	{"hostmacro",	"hostmacroid",	ZBX_SYNC,
		{
		{"hostmacroid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"macro",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		"hostid,macro"
	},
	{"hosts_groups",	"hostgroupid",	ZBX_SYNC,
		{
		{"hostgroupid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"hostid,groupid"
	},
	{"hosts_templates",	"hosttemplateid",	ZBX_SYNC,
		{
		{"hosttemplateid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"templateid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"hostid,templateid"
	},
	{"items_applications",	"itemappid",	ZBX_SYNC,
		{
		{"itemappid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"applicationid",	NULL,	"applications",	"applicationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"applicationid,itemid"
	},
	{"mappings",	"mappingid",	ZBX_SYNC,
		{
		{"mappingid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"valuemapid",	NULL,	"valuemaps",	"valuemapid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"value",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"newvalue",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"media",	"mediaid",	ZBX_SYNC,
		{
		{"mediaid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"mediatypeid",	NULL,	"media_type",	"mediatypeid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"sendto",	"",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"active",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity",	"63",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"period",	"1-7,00:00-24:00",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"rights",	"rightid",	ZBX_SYNC,
		{
		{"rightid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"groupid",	NULL,	"usrgrp",	"usrgrpid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"permission",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"id",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		NULL
	},
	{"services",	"serviceid",	ZBX_SYNC,
		{
		{"serviceid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"algorithm",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"triggerid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"showsla",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"goodsla",	"99.9",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"sortorder",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"services_links",	"linkid",	ZBX_SYNC,
		{
		{"linkid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"serviceupid",	NULL,	"services",	"serviceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"servicedownid",	NULL,	"services",	"serviceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"soft",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"serviceupid,servicedownid"
	},
	{"services_times",	"timeid",	ZBX_SYNC,
		{
		{"timeid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"serviceid",	NULL,	"services",	"serviceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ts_from",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"ts_to",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"note",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"icon_map",	"iconmapid",	ZBX_SYNC,
		{
		{"iconmapid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"default_iconid",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"icon_mapping",	"iconmappingid",	ZBX_SYNC,
		{
		{"iconmappingid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"iconmapid",	NULL,	"icon_map",	"iconmapid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"iconid",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"inventory_link",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"expression",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"sortorder",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"sysmaps",	"sysmapid",	ZBX_SYNC,
		{
		{"sysmapid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"width",	"600",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"height",	"400",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"backgroundid",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"label_type",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_location",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"highlight",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"expandproblem",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"markelements",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"show_unack",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"grid_size",	"50",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"grid_show",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"grid_align",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_format",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_type_host",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_type_hostgroup",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_type_trigger",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_type_map",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_type_image",	"2",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_string_host",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_string_hostgroup",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_string_trigger",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_string_map",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_string_image",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"iconmapid",	NULL,	"icon_map",	"iconmapid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"expand_macros",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"severity_min",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"sysmaps_elements",	"selementid",	ZBX_SYNC,
		{
		{"selementid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"sysmapid",	NULL,	"sysmaps",	"sysmapid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"elementid",	"0",	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"elementtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"iconid_off",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"iconid_on",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"label",	"",	NULL,	NULL,	2048,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label_location",	"-1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"x",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"y",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"iconid_disabled",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"iconid_maintenance",	NULL,	"images",	"imageid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"elementsubtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"areatype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"width",	"200",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"height",	"200",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"viewtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"use_iconmap",	"1",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"sysmaps_links",	"linkid",	ZBX_SYNC,
		{
		{"linkid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"sysmapid",	NULL,	"sysmaps",	"sysmapid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"selementid1",	NULL,	"sysmaps_elements",	"selementid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"selementid2",	NULL,	"sysmaps_elements",	"selementid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"drawtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"color",	"000000",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"label",	"",	NULL,	NULL,	2048,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"sysmaps_link_triggers",	"linktriggerid",	ZBX_SYNC,
		{
		{"linktriggerid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"linkid",	NULL,	"sysmaps_links",	"linkid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"triggerid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"drawtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"color",	"000000",	NULL,	NULL,	6,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"linkid,triggerid"
	},
	{"sysmap_element_url",	"sysmapelementurlid",	ZBX_SYNC,
		{
		{"sysmapelementurlid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"selementid",	NULL,	"sysmaps_elements",	"selementid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"name",	NULL,	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"selementid,name"
	},
	{"sysmap_url",	"sysmapurlid",	ZBX_SYNC,
		{
		{"sysmapurlid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"sysmapid",	NULL,	"sysmaps",	"sysmapid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"name",	NULL,	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"elementtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"sysmapid,name"
	},
	{"maintenances_hosts",	"maintenance_hostid",	ZBX_SYNC,
		{
		{"maintenance_hostid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"maintenanceid",	NULL,	"maintenances",	"maintenanceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"maintenanceid,hostid"
	},
	{"maintenances_groups",	"maintenance_groupid",	ZBX_SYNC,
		{
		{"maintenance_groupid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"maintenanceid",	NULL,	"maintenances",	"maintenanceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"groupid",	NULL,	"groups",	"groupid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"maintenanceid,groupid"
	},
	{"timeperiods",	"timeperiodid",	ZBX_SYNC,
		{
		{"timeperiodid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"timeperiod_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"every",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"month",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"dayofweek",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"day",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"start_time",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"period",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"start_date",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"maintenances_windows",	"maintenance_timeperiodid",	ZBX_SYNC,
		{
		{"maintenance_timeperiodid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"maintenanceid",	NULL,	"maintenances",	"maintenanceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"timeperiodid",	NULL,	"timeperiods",	"timeperiodid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"maintenanceid,timeperiodid"
	},
	{"regexps",	"regexpid",	ZBX_SYNC,
		{
		{"regexpid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"test_string",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"expressions",	"expressionid",	ZBX_SYNC,
		{
		{"expressionid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"regexpid",	NULL,	"regexps",	"regexpid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	ZBX_FK_CASCADE_DELETE},
		{"expression",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"expression_type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"exp_delimiter",	"",	NULL,	NULL,	1,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{"case_sensitive",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC | ZBX_PROXY,	0},
		{0}
		},
		NULL
	},
	{"nodes",	"nodeid",	0,
		{
		{"nodeid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"name",	"0",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"port",	"10051",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"nodetype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"masterid",	NULL,	"nodes",	"nodeid",	0,	ZBX_TYPE_INT,	0,	0},
		{0}
		},
		NULL
	},
	{"node_cksum",	"",	0,
		{
		{"nodeid",	NULL,	"nodes",	"nodeid",	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"tablename",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"recordid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"cksumtype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"cksum",	"",	NULL,	NULL,	ZBX_TYPE_TEXT_LEN,	ZBX_TYPE_TEXT,	ZBX_NOTNULL,	0},
		{"sync",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"ids",	"nodeid,table_name,field_name",	0,
		{
		{"nodeid",	NULL,	"nodes",	"nodeid",	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"table_name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"field_name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"nextid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"alerts",	"alertid",	ZBX_HISTORY,
		{
		{"alertid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"actionid",	NULL,	"actions",	"actionid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"eventid",	NULL,	"events",	"eventid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"mediatypeid",	NULL,	"media_type",	"mediatypeid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{"sendto",	"",	NULL,	NULL,	100,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"subject",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"message",	"",	NULL,	NULL,	ZBX_TYPE_TEXT_LEN,	ZBX_TYPE_TEXT,	ZBX_NOTNULL,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"retries",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"error",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"esc_step",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"alerttype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"history",	"",	0,
		{
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"0.0000",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"history_sync",	"id",	ZBX_HISTORY_SYNC,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"nodeid",	NULL,	"nodes",	"nodeid",	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{"value",	"0.0000",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{0}
		},
		NULL
	},
	{"history_uint",	"",	0,
		{
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"0",	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"history_uint_sync",	"id",	ZBX_HISTORY_SYNC,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"nodeid",	NULL,	"nodes",	"nodeid",	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{"value",	"0",	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{0}
		},
		NULL
	},
	{"history_str",	"",	0,
		{
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"history_str_sync",	"id",	ZBX_HISTORY_SYNC,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"nodeid",	NULL,	"nodes",	"nodeid",	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_HISTORY_SYNC,	0},
		{0}
		},
		NULL
	},
	{"history_log",	"id",	ZBX_HISTORY,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"timestamp",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"source",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"severity",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"",	NULL,	NULL,	ZBX_TYPE_TEXT_LEN,	ZBX_TYPE_TEXT,	ZBX_NOTNULL,	0},
		{"logeventid",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		"itemid,id"
	},
	{"history_text",	"id",	ZBX_HISTORY,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"",	NULL,	NULL,	ZBX_TYPE_TEXT_LEN,	ZBX_TYPE_TEXT,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		"itemid,id"
	},
	{"proxy_history",	"id",	0,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"timestamp",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"source",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"severity",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"",	NULL,	NULL,	ZBX_TYPE_LONGTEXT_LEN,	ZBX_TYPE_LONGTEXT,	ZBX_NOTNULL,	0},
		{"logeventid",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"state",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"proxy_dhistory",	"id",	0,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"druleid",	NULL,	"drules",	"druleid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"port",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"key_",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"dcheckid",	NULL,	"dchecks",	"dcheckid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{"dns",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"events",	"eventid",	ZBX_HISTORY,
		{
		{"eventid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"source",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"object",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"objectid",	"0",	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"acknowledged",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ns",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"trends",	"itemid,clock",	0,
		{
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"num",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value_min",	"0.0000",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL,	0},
		{"value_avg",	"0.0000",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL,	0},
		{"value_max",	"0.0000",	NULL,	NULL,	0,	ZBX_TYPE_FLOAT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"trends_uint",	"itemid,clock",	0,
		{
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"num",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value_min",	"0",	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"value_avg",	"0",	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"value_max",	"0",	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"acknowledges",	"acknowledgeid",	ZBX_HISTORY,
		{
		{"acknowledgeid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"eventid",	NULL,	"events",	"eventid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"message",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"auditlog",	"auditid",	ZBX_HISTORY,
		{
		{"auditid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"action",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"resourcetype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"details",	"0",	NULL,	NULL,	128 ,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"resourceid",	"0",	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"resourcename",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"auditlog_details",	"auditdetailid",	ZBX_HISTORY,
		{
		{"auditdetailid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"auditid",	NULL,	"auditlog",	"auditid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"table_name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"field_name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"oldvalue",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL,	0},
		{"newvalue",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"service_alarms",	"servicealarmid",	ZBX_HISTORY,
		{
		{"servicealarmid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"serviceid",	NULL,	"services",	"serviceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"autoreg_host",	"autoreg_hostid",	ZBX_SYNC,
		{
		{"autoreg_hostid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"proxy_hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"host",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"listen_ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"listen_port",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"listen_dns",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"host_metadata",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"proxy_autoreg_host",	"id",	0,
		{
		{"id",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_UINT,	ZBX_NOTNULL,	0},
		{"clock",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"host",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"listen_ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"listen_port",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"listen_dns",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"host_metadata",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"dhosts",	"dhostid",	ZBX_SYNC,
		{
		{"dhostid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"druleid",	NULL,	"drules",	"druleid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lastup",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lastdown",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"dservices",	"dserviceid",	ZBX_SYNC,
		{
		{"dserviceid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"dhostid",	NULL,	"dhosts",	"dhostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"key_",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"value",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"port",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lastup",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"lastdown",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"dcheckid",	NULL,	"dchecks",	"dcheckid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"dns",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		"dcheckid,type,key_,ip,port"
	},
	{"escalations",	"escalationid",	0,
		{
		{"escalationid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"actionid",	NULL,	"actions",	"actionid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"triggerid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{"eventid",	NULL,	"events",	"eventid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{"r_eventid",	NULL,	"events",	"eventid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{"nextcheck",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"esc_step",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	0,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"actionid,triggerid,itemid,escalationid"
	},
	{"globalvars",	"globalvarid",	0,
		{
		{"globalvarid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"snmp_lastsize",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"graph_discovery",	"graphdiscoveryid",	ZBX_SYNC,
		{
		{"graphdiscoveryid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"graphid",	NULL,	"graphs",	"graphid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"parent_graphid",	NULL,	"graphs",	"graphid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		"graphid,parent_graphid"
	},
	{"host_inventory",	"hostid",	ZBX_SYNC,
		{
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"inventory_mode",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"type",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"type_full",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"alias",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"os",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"os_full",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"os_short",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"serialno_a",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"serialno_b",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"tag",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"asset_tag",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"macaddress_a",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"macaddress_b",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hardware",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hardware_full",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software_full",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software_app_a",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software_app_b",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software_app_c",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software_app_d",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"software_app_e",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"contact",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"location",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"location_lat",	"",	NULL,	NULL,	16,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"location_lon",	"",	NULL,	NULL,	16,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"notes",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"chassis",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"model",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"hw_arch",	"",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"vendor",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"contract_number",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"installer_name",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"deployment_status",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url_a",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url_b",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"url_c",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"host_networks",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"host_netmask",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"host_router",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"oob_ip",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"oob_netmask",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"oob_router",	"",	NULL,	NULL,	39,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"date_hw_purchase",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"date_hw_install",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"date_hw_expiry",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"date_hw_decomm",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_address_a",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_address_b",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_address_c",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_city",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_state",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_country",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_zip",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_rack",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"site_notes",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_email",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_phone_a",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_phone_b",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_cell",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_screen",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_1_notes",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_name",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_email",	"",	NULL,	NULL,	128,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_phone_a",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_phone_b",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_cell",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_screen",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"poc_2_notes",	"",	NULL,	NULL,	ZBX_TYPE_SHORTTEXT_LEN,	ZBX_TYPE_SHORTTEXT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"housekeeper",	"housekeeperid",	0,
		{
		{"housekeeperid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"tablename",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"field",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"value",	NULL,	"items",	"value",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		NULL
	},
	{"images",	"imageid",	ZBX_SYNC,
		{
		{"imageid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"imagetype",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"name",	"0",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{"image",	"",	NULL,	NULL,	0,	ZBX_TYPE_BLOB,	ZBX_NOTNULL | ZBX_SYNC,	0},
		{0}
		},
		NULL
	},
	{"item_discovery",	"itemdiscoveryid",	ZBX_SYNC,
		{
		{"itemdiscoveryid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"parent_itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"key_",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"lastcheck",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ts_delete",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		"itemid,parent_itemid"
	},
	{"host_discovery",	"hostid",	ZBX_SYNC,
		{
		{"hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"parent_hostid",	NULL,	"hosts",	"hostid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"parent_itemid",	NULL,	"items",	"itemid",	0,	ZBX_TYPE_ID,	ZBX_SYNC,	0},
		{"host",	"",	NULL,	NULL,	64,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"lastcheck",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"ts_delete",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"interface_discovery",	"interfaceid",	ZBX_SYNC,
		{
		{"interfaceid",	NULL,	"interface",	"interfaceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"parent_interfaceid",	NULL,	"interface",	"interfaceid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		NULL
	},
	{"profiles",	"profileid",	0,
		{
		{"profileid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"idx",	"",	NULL,	NULL,	96,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"idx2",	"0",	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"value_id",	"0",	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"value_int",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"value_str",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"source",	"",	NULL,	NULL,	96,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"type",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"sessions",	"sessionid",	0,
		{
		{"sessionid",	"",	NULL,	NULL,	32,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"lastaccess",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"status",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{"trigger_discovery",	"triggerdiscoveryid",	ZBX_SYNC,
		{
		{"triggerdiscoveryid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"triggerid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"parent_triggerid",	NULL,	"triggers",	"triggerid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"name",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		"triggerid,parent_triggerid"
	},
	{"user_history",	"userhistoryid",	0,
		{
		{"userhistoryid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"userid",	NULL,	"users",	"userid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	ZBX_FK_CASCADE_DELETE},
		{"title1",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"url1",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"title2",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"url2",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"title3",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"url3",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"title4",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"url4",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"title5",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{"url5",	"",	NULL,	NULL,	255,	ZBX_TYPE_CHAR,	ZBX_NOTNULL,	0},
		{0}
		},
		"userid"
	},
	{"application_template",	"application_templateid",	ZBX_SYNC,
		{
		{"application_templateid",	NULL,	NULL,	NULL,	0,	ZBX_TYPE_ID,	ZBX_NOTNULL,	0},
		{"applicationid",	NULL,	"applications",	"applicationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{"templateid",	NULL,	"applications",	"applicationid",	0,	ZBX_TYPE_ID,	ZBX_NOTNULL | ZBX_SYNC,	ZBX_FK_CASCADE_DELETE},
		{0}
		},
		"applicationid,templateid"
	},
	{"dbversion",	"",	0,
		{
		{"mandatory",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{"optional",	"0",	NULL,	NULL,	0,	ZBX_TYPE_INT,	ZBX_NOTNULL,	0},
		{0}
		},
		NULL
	},
	{0}

#undef ZBX_TYPE_LONGTEXT_LEN
#undef ZBX_TYPE_SHORTTEXT_LEN

};

#if defined(HAVE_IBM_DB2)
const char	*const db_schema = "\
CREATE TABLE maintenances (\n\
maintenanceid bigint  NOT NULL,\n\
name varchar(128) WITH DEFAULT '' NOT NULL,\n\
maintenance_type integer WITH DEFAULT '0' NOT NULL,\n\
description varchar(2048) WITH DEFAULT '' NOT NULL,\n\
active_since integer WITH DEFAULT '0' NOT NULL,\n\
active_till integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (maintenanceid)\n\
);\n\
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);\n\
CREATE TABLE hosts (\n\
hostid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL,\n\
host varchar(64) WITH DEFAULT '' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
disable_until integer WITH DEFAULT '0' NOT NULL,\n\
error varchar(128) WITH DEFAULT '' NOT NULL,\n\
available integer WITH DEFAULT '0' NOT NULL,\n\
errors_from integer WITH DEFAULT '0' NOT NULL,\n\
lastaccess integer WITH DEFAULT '0' NOT NULL,\n\
ipmi_authtype integer WITH DEFAULT '0' NOT NULL,\n\
ipmi_privilege integer WITH DEFAULT '2' NOT NULL,\n\
ipmi_username varchar(16) WITH DEFAULT '' NOT NULL,\n\
ipmi_password varchar(20) WITH DEFAULT '' NOT NULL,\n\
ipmi_disable_until integer WITH DEFAULT '0' NOT NULL,\n\
ipmi_available integer WITH DEFAULT '0' NOT NULL,\n\
snmp_disable_until integer WITH DEFAULT '0' NOT NULL,\n\
snmp_available integer WITH DEFAULT '0' NOT NULL,\n\
maintenanceid bigint  NULL,\n\
maintenance_status integer WITH DEFAULT '0' NOT NULL,\n\
maintenance_type integer WITH DEFAULT '0' NOT NULL,\n\
maintenance_from integer WITH DEFAULT '0' NOT NULL,\n\
ipmi_errors_from integer WITH DEFAULT '0' NOT NULL,\n\
snmp_errors_from integer WITH DEFAULT '0' NOT NULL,\n\
ipmi_error varchar(128) WITH DEFAULT '' NOT NULL,\n\
snmp_error varchar(128) WITH DEFAULT '' NOT NULL,\n\
jmx_disable_until integer WITH DEFAULT '0' NOT NULL,\n\
jmx_available integer WITH DEFAULT '0' NOT NULL,\n\
jmx_errors_from integer WITH DEFAULT '0' NOT NULL,\n\
jmx_error varchar(128) WITH DEFAULT '' NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
flags integer WITH DEFAULT '0' NOT NULL,\n\
templateid bigint  NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE INDEX hosts_1 ON hosts (host);\n\
CREATE INDEX hosts_2 ON hosts (status);\n\
CREATE INDEX hosts_3 ON hosts (proxy_hostid);\n\
CREATE INDEX hosts_4 ON hosts (name);\n\
CREATE INDEX hosts_5 ON hosts (maintenanceid);\n\
CREATE TABLE groups (\n\
groupid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
internal integer WITH DEFAULT '0' NOT NULL,\n\
flags integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE INDEX groups_1 ON groups (name);\n\
CREATE TABLE group_prototype (\n\
group_prototypeid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
groupid bigint  NULL,\n\
templateid bigint  NULL,\n\
PRIMARY KEY (group_prototypeid)\n\
);\n\
CREATE INDEX group_prototype_1 ON group_prototype (hostid);\n\
CREATE TABLE group_discovery (\n\
groupid bigint  NOT NULL,\n\
parent_group_prototypeid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
lastcheck integer WITH DEFAULT '0' NOT NULL,\n\
ts_delete integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE TABLE screens (\n\
screenid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
hsize integer WITH DEFAULT '1' NOT NULL,\n\
vsize integer WITH DEFAULT '1' NOT NULL,\n\
templateid bigint  NULL,\n\
PRIMARY KEY (screenid)\n\
);\n\
CREATE INDEX screens_1 ON screens (templateid);\n\
CREATE TABLE screens_items (\n\
screenitemid bigint  NOT NULL,\n\
screenid bigint  NOT NULL,\n\
resourcetype integer WITH DEFAULT '0' NOT NULL,\n\
resourceid bigint WITH DEFAULT '0' NOT NULL,\n\
width integer WITH DEFAULT '320' NOT NULL,\n\
height integer WITH DEFAULT '200' NOT NULL,\n\
x integer WITH DEFAULT '0' NOT NULL,\n\
y integer WITH DEFAULT '0' NOT NULL,\n\
colspan integer WITH DEFAULT '0' NOT NULL,\n\
rowspan integer WITH DEFAULT '0' NOT NULL,\n\
elements integer WITH DEFAULT '25' NOT NULL,\n\
valign integer WITH DEFAULT '0' NOT NULL,\n\
halign integer WITH DEFAULT '0' NOT NULL,\n\
style integer WITH DEFAULT '0' NOT NULL,\n\
url varchar(255) WITH DEFAULT '' NOT NULL,\n\
dynamic integer WITH DEFAULT '0' NOT NULL,\n\
sort_triggers integer WITH DEFAULT '0' NOT NULL,\n\
application varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (screenitemid)\n\
);\n\
CREATE INDEX screens_items_1 ON screens_items (screenid);\n\
CREATE TABLE slideshows (\n\
slideshowid bigint  NOT NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
delay integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideshowid)\n\
);\n\
CREATE TABLE slides (\n\
slideid bigint  NOT NULL,\n\
slideshowid bigint  NOT NULL,\n\
screenid bigint  NOT NULL,\n\
step integer WITH DEFAULT '0' NOT NULL,\n\
delay integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideid)\n\
);\n\
CREATE INDEX slides_1 ON slides (slideshowid);\n\
CREATE INDEX slides_2 ON slides (screenid);\n\
CREATE TABLE drules (\n\
druleid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
iprange varchar(255) WITH DEFAULT '' NOT NULL,\n\
delay integer WITH DEFAULT '3600' NOT NULL,\n\
nextcheck integer WITH DEFAULT '0' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (druleid)\n\
);\n\
CREATE INDEX drules_1 ON drules (proxy_hostid);\n\
CREATE TABLE dchecks (\n\
dcheckid bigint  NOT NULL,\n\
druleid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
key_ varchar(255) WITH DEFAULT '' NOT NULL,\n\
snmp_community varchar(255) WITH DEFAULT '' NOT NULL,\n\
ports varchar(255) WITH DEFAULT '0' NOT NULL,\n\
snmpv3_securityname varchar(64) WITH DEFAULT '' NOT NULL,\n\
snmpv3_securitylevel integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase varchar(64) WITH DEFAULT '' NOT NULL,\n\
snmpv3_privpassphrase varchar(64) WITH DEFAULT '' NOT NULL,\n\
uniq integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_authprotocol integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_contextname varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dcheckid)\n\
);\n\
CREATE INDEX dchecks_1 ON dchecks (druleid);\n\
CREATE TABLE applications (\n\
applicationid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (applicationid)\n\
);\n\
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);\n\
CREATE TABLE httptest (\n\
httptestid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
applicationid bigint  NULL,\n\
nextcheck integer WITH DEFAULT '0' NOT NULL,\n\
delay integer WITH DEFAULT '60' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
variables varchar(2048) WITH DEFAULT '' NOT NULL,\n\
agent varchar(255) WITH DEFAULT '' NOT NULL,\n\
authentication integer WITH DEFAULT '0' NOT NULL,\n\
http_user varchar(64) WITH DEFAULT '' NOT NULL,\n\
http_password varchar(64) WITH DEFAULT '' NOT NULL,\n\
hostid bigint  NOT NULL,\n\
templateid bigint  NULL,\n\
http_proxy varchar(255) WITH DEFAULT '' NOT NULL,\n\
retries integer WITH DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (httptestid)\n\
);\n\
CREATE INDEX httptest_1 ON httptest (applicationid);\n\
CREATE UNIQUE INDEX httptest_2 ON httptest (hostid,name);\n\
CREATE INDEX httptest_3 ON httptest (status);\n\
CREATE INDEX httptest_4 ON httptest (templateid);\n\
CREATE TABLE httpstep (\n\
httpstepid bigint  NOT NULL,\n\
httptestid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
no integer WITH DEFAULT '0' NOT NULL,\n\
url varchar(255) WITH DEFAULT '' NOT NULL,\n\
timeout integer WITH DEFAULT '30' NOT NULL,\n\
posts varchar(2048) WITH DEFAULT '' NOT NULL,\n\
required varchar(255) WITH DEFAULT '' NOT NULL,\n\
status_codes varchar(255) WITH DEFAULT '' NOT NULL,\n\
variables varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (httpstepid)\n\
);\n\
CREATE INDEX httpstep_1 ON httpstep (httptestid);\n\
CREATE TABLE interface (\n\
interfaceid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
main integer WITH DEFAULT '0' NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
useip integer WITH DEFAULT '1' NOT NULL,\n\
ip varchar(64) WITH DEFAULT '127.0.0.1' NOT NULL,\n\
dns varchar(64) WITH DEFAULT '' NOT NULL,\n\
port varchar(64) WITH DEFAULT '10050' NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE INDEX interface_1 ON interface (hostid,type);\n\
CREATE INDEX interface_2 ON interface (ip,dns);\n\
CREATE TABLE valuemaps (\n\
valuemapid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (valuemapid)\n\
);\n\
CREATE INDEX valuemaps_1 ON valuemaps (name);\n\
CREATE TABLE items (\n\
itemid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
snmp_community varchar(64) WITH DEFAULT '' NOT NULL,\n\
snmp_oid varchar(255) WITH DEFAULT '' NOT NULL,\n\
hostid bigint  NOT NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
key_ varchar(255) WITH DEFAULT '' NOT NULL,\n\
delay integer WITH DEFAULT '0' NOT NULL,\n\
history integer WITH DEFAULT '90' NOT NULL,\n\
trends integer WITH DEFAULT '365' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
value_type integer WITH DEFAULT '0' NOT NULL,\n\
trapper_hosts varchar(255) WITH DEFAULT '' NOT NULL,\n\
units varchar(255) WITH DEFAULT '' NOT NULL,\n\
multiplier integer WITH DEFAULT '0' NOT NULL,\n\
delta integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_securityname varchar(64) WITH DEFAULT '' NOT NULL,\n\
snmpv3_securitylevel integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase varchar(64) WITH DEFAULT '' NOT NULL,\n\
snmpv3_privpassphrase varchar(64) WITH DEFAULT '' NOT NULL,\n\
formula varchar(255) WITH DEFAULT '1' NOT NULL,\n\
error varchar(128) WITH DEFAULT '' NOT NULL,\n\
lastlogsize bigint WITH DEFAULT '0' NOT NULL,\n\
logtimefmt varchar(64) WITH DEFAULT '' NOT NULL,\n\
templateid bigint  NULL,\n\
valuemapid bigint  NULL,\n\
delay_flex varchar(255) WITH DEFAULT '' NOT NULL,\n\
params varchar(2048) WITH DEFAULT '' NOT NULL,\n\
ipmi_sensor varchar(128) WITH DEFAULT '' NOT NULL,\n\
data_type integer WITH DEFAULT '0' NOT NULL,\n\
authtype integer WITH DEFAULT '0' NOT NULL,\n\
username varchar(64) WITH DEFAULT '' NOT NULL,\n\
password varchar(64) WITH DEFAULT '' NOT NULL,\n\
publickey varchar(64) WITH DEFAULT '' NOT NULL,\n\
privatekey varchar(64) WITH DEFAULT '' NOT NULL,\n\
mtime integer WITH DEFAULT '0' NOT NULL,\n\
flags integer WITH DEFAULT '0' NOT NULL,\n\
filter varchar(255) WITH DEFAULT '' NOT NULL,\n\
interfaceid bigint  NULL,\n\
port varchar(64) WITH DEFAULT '' NOT NULL,\n\
description varchar(2048) WITH DEFAULT '' NOT NULL,\n\
inventory_link integer WITH DEFAULT '0' NOT NULL,\n\
lifetime varchar(64) WITH DEFAULT '30' NOT NULL,\n\
snmpv3_authprotocol integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol integer WITH DEFAULT '0' NOT NULL,\n\
state integer WITH DEFAULT '0' NOT NULL,\n\
snmpv3_contextname varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (itemid)\n\
);\n\
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);\n\
CREATE INDEX items_3 ON items (status);\n\
CREATE INDEX items_4 ON items (templateid);\n\
CREATE INDEX items_5 ON items (valuemapid);\n\
CREATE INDEX items_6 ON items (interfaceid);\n\
CREATE TABLE httpstepitem (\n\
httpstepitemid bigint  NOT NULL,\n\
httpstepid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httpstepitemid)\n\
);\n\
CREATE UNIQUE INDEX httpstepitem_1 ON httpstepitem (httpstepid,itemid);\n\
CREATE INDEX httpstepitem_2 ON httpstepitem (itemid);\n\
CREATE TABLE httptestitem (\n\
httptestitemid bigint  NOT NULL,\n\
httptestid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httptestitemid)\n\
);\n\
CREATE UNIQUE INDEX httptestitem_1 ON httptestitem (httptestid,itemid);\n\
CREATE INDEX httptestitem_2 ON httptestitem (itemid);\n\
CREATE TABLE media_type (\n\
mediatypeid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
description varchar(100) WITH DEFAULT '' NOT NULL,\n\
smtp_server varchar(255) WITH DEFAULT '' NOT NULL,\n\
smtp_helo varchar(255) WITH DEFAULT '' NOT NULL,\n\
smtp_email varchar(255) WITH DEFAULT '' NOT NULL,\n\
exec_path varchar(255) WITH DEFAULT '' NOT NULL,\n\
gsm_modem varchar(255) WITH DEFAULT '' NOT NULL,\n\
username varchar(255) WITH DEFAULT '' NOT NULL,\n\
passwd varchar(255) WITH DEFAULT '' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (mediatypeid)\n\
);\n\
CREATE TABLE users (\n\
userid bigint  NOT NULL,\n\
alias varchar(100) WITH DEFAULT '' NOT NULL,\n\
name varchar(100) WITH DEFAULT '' NOT NULL,\n\
surname varchar(100) WITH DEFAULT '' NOT NULL,\n\
passwd varchar(32) WITH DEFAULT '' NOT NULL,\n\
url varchar(255) WITH DEFAULT '' NOT NULL,\n\
autologin integer WITH DEFAULT '0' NOT NULL,\n\
autologout integer WITH DEFAULT '900' NOT NULL,\n\
lang varchar(5) WITH DEFAULT 'en_GB' NOT NULL,\n\
refresh integer WITH DEFAULT '30' NOT NULL,\n\
type integer WITH DEFAULT '1' NOT NULL,\n\
theme varchar(128) WITH DEFAULT 'default' NOT NULL,\n\
attempt_failed integer WITH DEFAULT 0 NOT NULL,\n\
attempt_ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
attempt_clock integer WITH DEFAULT 0 NOT NULL,\n\
rows_per_page integer WITH DEFAULT 50 NOT NULL,\n\
PRIMARY KEY (userid)\n\
);\n\
CREATE INDEX users_1 ON users (alias);\n\
CREATE TABLE usrgrp (\n\
usrgrpid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
gui_access integer WITH DEFAULT '0' NOT NULL,\n\
users_status integer WITH DEFAULT '0' NOT NULL,\n\
debug_mode integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (usrgrpid)\n\
);\n\
CREATE INDEX usrgrp_1 ON usrgrp (name);\n\
CREATE TABLE users_groups (\n\
id bigint  NOT NULL,\n\
usrgrpid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);\n\
CREATE INDEX users_groups_2 ON users_groups (userid);\n\
CREATE TABLE scripts (\n\
scriptid bigint  NOT NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
command varchar(255) WITH DEFAULT '' NOT NULL,\n\
host_access integer WITH DEFAULT '2' NOT NULL,\n\
usrgrpid bigint  NULL,\n\
groupid bigint  NULL,\n\
description varchar(2048) WITH DEFAULT '' NOT NULL,\n\
confirmation varchar(255) WITH DEFAULT '' NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
execute_on integer WITH DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (scriptid)\n\
);\n\
CREATE INDEX scripts_1 ON scripts (usrgrpid);\n\
CREATE INDEX scripts_2 ON scripts (groupid);\n\
CREATE TABLE actions (\n\
actionid bigint  NOT NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
eventsource integer WITH DEFAULT '0' NOT NULL,\n\
evaltype integer WITH DEFAULT '0' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
esc_period integer WITH DEFAULT '0' NOT NULL,\n\
def_shortdata varchar(255) WITH DEFAULT '' NOT NULL,\n\
def_longdata varchar(2048) WITH DEFAULT '' NOT NULL,\n\
recovery_msg integer WITH DEFAULT '0' NOT NULL,\n\
r_shortdata varchar(255) WITH DEFAULT '' NOT NULL,\n\
r_longdata varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (actionid)\n\
);\n\
CREATE INDEX actions_1 ON actions (eventsource,status);\n\
CREATE TABLE operations (\n\
operationid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
operationtype integer WITH DEFAULT '0' NOT NULL,\n\
esc_period integer WITH DEFAULT '0' NOT NULL,\n\
esc_step_from integer WITH DEFAULT '1' NOT NULL,\n\
esc_step_to integer WITH DEFAULT '1' NOT NULL,\n\
evaltype integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX operations_1 ON operations (actionid);\n\
CREATE TABLE opmessage (\n\
operationid bigint  NOT NULL,\n\
default_msg integer WITH DEFAULT '0' NOT NULL,\n\
subject varchar(255) WITH DEFAULT '' NOT NULL,\n\
message varchar(2048) WITH DEFAULT '' NOT NULL,\n\
mediatypeid bigint  NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opmessage_1 ON opmessage (mediatypeid);\n\
CREATE TABLE opmessage_grp (\n\
opmessage_grpid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
usrgrpid bigint  NOT NULL,\n\
PRIMARY KEY (opmessage_grpid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);\n\
CREATE INDEX opmessage_grp_2 ON opmessage_grp (usrgrpid);\n\
CREATE TABLE opmessage_usr (\n\
opmessage_usrid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
PRIMARY KEY (opmessage_usrid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);\n\
CREATE INDEX opmessage_usr_2 ON opmessage_usr (userid);\n\
CREATE TABLE opcommand (\n\
operationid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
scriptid bigint  NULL,\n\
execute_on integer WITH DEFAULT '0' NOT NULL,\n\
port varchar(64) WITH DEFAULT '' NOT NULL,\n\
authtype integer WITH DEFAULT '0' NOT NULL,\n\
username varchar(64) WITH DEFAULT '' NOT NULL,\n\
password varchar(64) WITH DEFAULT '' NOT NULL,\n\
publickey varchar(64) WITH DEFAULT '' NOT NULL,\n\
privatekey varchar(64) WITH DEFAULT '' NOT NULL,\n\
command varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opcommand_1 ON opcommand (scriptid);\n\
CREATE TABLE opcommand_hst (\n\
opcommand_hstid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
hostid bigint  NULL,\n\
PRIMARY KEY (opcommand_hstid)\n\
);\n\
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);\n\
CREATE INDEX opcommand_hst_2 ON opcommand_hst (hostid);\n\
CREATE TABLE opcommand_grp (\n\
opcommand_grpid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (opcommand_grpid)\n\
);\n\
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);\n\
CREATE INDEX opcommand_grp_2 ON opcommand_grp (groupid);\n\
CREATE TABLE opgroup (\n\
opgroupid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (opgroupid)\n\
);\n\
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);\n\
CREATE INDEX opgroup_2 ON opgroup (groupid);\n\
CREATE TABLE optemplate (\n\
optemplateid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
templateid bigint  NOT NULL,\n\
PRIMARY KEY (optemplateid)\n\
);\n\
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);\n\
CREATE INDEX optemplate_2 ON optemplate (templateid);\n\
CREATE TABLE opconditions (\n\
opconditionid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
conditiontype integer WITH DEFAULT '0' NOT NULL,\n\
operator integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (opconditionid)\n\
);\n\
CREATE INDEX opconditions_1 ON opconditions (operationid);\n\
CREATE TABLE conditions (\n\
conditionid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
conditiontype integer WITH DEFAULT '0' NOT NULL,\n\
operator integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (conditionid)\n\
);\n\
CREATE INDEX conditions_1 ON conditions (actionid);\n\
CREATE TABLE config (\n\
configid bigint  NOT NULL,\n\
refresh_unsupported integer WITH DEFAULT '0' NOT NULL,\n\
work_period varchar(100) WITH DEFAULT '1-5,00:00-24:00' NOT NULL,\n\
alert_usrgrpid bigint  NULL,\n\
event_ack_enable integer WITH DEFAULT '1' NOT NULL,\n\
event_expire integer WITH DEFAULT '7' NOT NULL,\n\
event_show_max integer WITH DEFAULT '100' NOT NULL,\n\
default_theme varchar(128) WITH DEFAULT 'originalblue' NOT NULL,\n\
authentication_type integer WITH DEFAULT '0' NOT NULL,\n\
ldap_host varchar(255) WITH DEFAULT '' NOT NULL,\n\
ldap_port integer WITH DEFAULT 389 NOT NULL,\n\
ldap_base_dn varchar(255) WITH DEFAULT '' NOT NULL,\n\
ldap_bind_dn varchar(255) WITH DEFAULT '' NOT NULL,\n\
ldap_bind_password varchar(128) WITH DEFAULT '' NOT NULL,\n\
ldap_search_attribute varchar(128) WITH DEFAULT '' NOT NULL,\n\
dropdown_first_entry integer WITH DEFAULT '1' NOT NULL,\n\
dropdown_first_remember integer WITH DEFAULT '1' NOT NULL,\n\
discovery_groupid bigint  NOT NULL,\n\
max_in_table integer WITH DEFAULT '50' NOT NULL,\n\
search_limit integer WITH DEFAULT '1000' NOT NULL,\n\
severity_color_0 varchar(6) WITH DEFAULT 'DBDBDB' NOT NULL,\n\
severity_color_1 varchar(6) WITH DEFAULT 'D6F6FF' NOT NULL,\n\
severity_color_2 varchar(6) WITH DEFAULT 'FFF6A5' NOT NULL,\n\
severity_color_3 varchar(6) WITH DEFAULT 'FFB689' NOT NULL,\n\
severity_color_4 varchar(6) WITH DEFAULT 'FF9999' NOT NULL,\n\
severity_color_5 varchar(6) WITH DEFAULT 'FF3838' NOT NULL,\n\
severity_name_0 varchar(32) WITH DEFAULT 'Not classified' NOT NULL,\n\
severity_name_1 varchar(32) WITH DEFAULT 'Information' NOT NULL,\n\
severity_name_2 varchar(32) WITH DEFAULT 'Warning' NOT NULL,\n\
severity_name_3 varchar(32) WITH DEFAULT 'Average' NOT NULL,\n\
severity_name_4 varchar(32) WITH DEFAULT 'High' NOT NULL,\n\
severity_name_5 varchar(32) WITH DEFAULT 'Disaster' NOT NULL,\n\
ok_period integer WITH DEFAULT '1800' NOT NULL,\n\
blink_period integer WITH DEFAULT '1800' NOT NULL,\n\
problem_unack_color varchar(6) WITH DEFAULT 'DC0000' NOT NULL,\n\
problem_ack_color varchar(6) WITH DEFAULT 'DC0000' NOT NULL,\n\
ok_unack_color varchar(6) WITH DEFAULT '00AA00' NOT NULL,\n\
ok_ack_color varchar(6) WITH DEFAULT '00AA00' NOT NULL,\n\
problem_unack_style integer WITH DEFAULT '1' NOT NULL,\n\
problem_ack_style integer WITH DEFAULT '1' NOT NULL,\n\
ok_unack_style integer WITH DEFAULT '1' NOT NULL,\n\
ok_ack_style integer WITH DEFAULT '1' NOT NULL,\n\
snmptrap_logging integer WITH DEFAULT '1' NOT NULL,\n\
server_check_interval integer WITH DEFAULT '10' NOT NULL,\n\
hk_events_mode integer WITH DEFAULT '1' NOT NULL,\n\
hk_events_trigger integer WITH DEFAULT '365' NOT NULL,\n\
hk_events_internal integer WITH DEFAULT '365' NOT NULL,\n\
hk_events_discovery integer WITH DEFAULT '365' NOT NULL,\n\
hk_events_autoreg integer WITH DEFAULT '365' NOT NULL,\n\
hk_services_mode integer WITH DEFAULT '1' NOT NULL,\n\
hk_services integer WITH DEFAULT '365' NOT NULL,\n\
hk_audit_mode integer WITH DEFAULT '1' NOT NULL,\n\
hk_audit integer WITH DEFAULT '365' NOT NULL,\n\
hk_sessions_mode integer WITH DEFAULT '1' NOT NULL,\n\
hk_sessions integer WITH DEFAULT '365' NOT NULL,\n\
hk_history_mode integer WITH DEFAULT '1' NOT NULL,\n\
hk_history_global integer WITH DEFAULT '0' NOT NULL,\n\
hk_history integer WITH DEFAULT '90' NOT NULL,\n\
hk_trends_mode integer WITH DEFAULT '1' NOT NULL,\n\
hk_trends_global integer WITH DEFAULT '0' NOT NULL,\n\
hk_trends integer WITH DEFAULT '365' NOT NULL,\n\
PRIMARY KEY (configid)\n\
);\n\
CREATE INDEX config_1 ON config (alert_usrgrpid);\n\
CREATE INDEX config_2 ON config (discovery_groupid);\n\
CREATE TABLE triggers (\n\
triggerid bigint  NOT NULL,\n\
expression varchar(2048) WITH DEFAULT '' NOT NULL,\n\
description varchar(255) WITH DEFAULT '' NOT NULL,\n\
url varchar(255) WITH DEFAULT '' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
value integer WITH DEFAULT '0' NOT NULL,\n\
priority integer WITH DEFAULT '0' NOT NULL,\n\
lastchange integer WITH DEFAULT '0' NOT NULL,\n\
comments varchar(2048) WITH DEFAULT '' NOT NULL,\n\
error varchar(128) WITH DEFAULT '' NOT NULL,\n\
templateid bigint  NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
state integer WITH DEFAULT '0' NOT NULL,\n\
flags integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (triggerid)\n\
);\n\
CREATE INDEX triggers_1 ON triggers (status);\n\
CREATE INDEX triggers_2 ON triggers (value);\n\
CREATE INDEX triggers_3 ON triggers (templateid);\n\
CREATE TABLE trigger_depends (\n\
triggerdepid bigint  NOT NULL,\n\
triggerid_down bigint  NOT NULL,\n\
triggerid_up bigint  NOT NULL,\n\
PRIMARY KEY (triggerdepid)\n\
);\n\
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);\n\
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);\n\
CREATE TABLE functions (\n\
functionid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL,\n\
function varchar(12) WITH DEFAULT '' NOT NULL,\n\
parameter varchar(255) WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (functionid)\n\
);\n\
CREATE INDEX functions_1 ON functions (triggerid);\n\
CREATE INDEX functions_2 ON functions (itemid,function,parameter);\n\
CREATE TABLE graphs (\n\
graphid bigint  NOT NULL,\n\
name varchar(128) WITH DEFAULT '' NOT NULL,\n\
width integer WITH DEFAULT '900' NOT NULL,\n\
height integer WITH DEFAULT '200' NOT NULL,\n\
yaxismin decfloat(16) WITH DEFAULT '0' NOT NULL,\n\
yaxismax decfloat(16) WITH DEFAULT '100' NOT NULL,\n\
templateid bigint  NULL,\n\
show_work_period integer WITH DEFAULT '1' NOT NULL,\n\
show_triggers integer WITH DEFAULT '1' NOT NULL,\n\
graphtype integer WITH DEFAULT '0' NOT NULL,\n\
show_legend integer WITH DEFAULT '1' NOT NULL,\n\
show_3d integer WITH DEFAULT '0' NOT NULL,\n\
percent_left decfloat(16) WITH DEFAULT '0' NOT NULL,\n\
percent_right decfloat(16) WITH DEFAULT '0' NOT NULL,\n\
ymin_type integer WITH DEFAULT '0' NOT NULL,\n\
ymax_type integer WITH DEFAULT '0' NOT NULL,\n\
ymin_itemid bigint  NULL,\n\
ymax_itemid bigint  NULL,\n\
flags integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (graphid)\n\
);\n\
CREATE INDEX graphs_1 ON graphs (name);\n\
CREATE INDEX graphs_2 ON graphs (templateid);\n\
CREATE INDEX graphs_3 ON graphs (ymin_itemid);\n\
CREATE INDEX graphs_4 ON graphs (ymax_itemid);\n\
CREATE TABLE graphs_items (\n\
gitemid bigint  NOT NULL,\n\
graphid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
drawtype integer WITH DEFAULT '0' NOT NULL,\n\
sortorder integer WITH DEFAULT '0' NOT NULL,\n\
color varchar(6) WITH DEFAULT '009600' NOT NULL,\n\
yaxisside integer WITH DEFAULT '0' NOT NULL,\n\
calc_fnc integer WITH DEFAULT '2' NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (gitemid)\n\
);\n\
CREATE INDEX graphs_items_1 ON graphs_items (itemid);\n\
CREATE INDEX graphs_items_2 ON graphs_items (graphid);\n\
CREATE TABLE graph_theme (\n\
graphthemeid bigint  NOT NULL,\n\
description varchar(64) WITH DEFAULT '' NOT NULL,\n\
theme varchar(64) WITH DEFAULT '' NOT NULL,\n\
backgroundcolor varchar(6) WITH DEFAULT 'F0F0F0' NOT NULL,\n\
graphcolor varchar(6) WITH DEFAULT 'FFFFFF' NOT NULL,\n\
graphbordercolor varchar(6) WITH DEFAULT '222222' NOT NULL,\n\
gridcolor varchar(6) WITH DEFAULT 'CCCCCC' NOT NULL,\n\
maingridcolor varchar(6) WITH DEFAULT 'AAAAAA' NOT NULL,\n\
gridbordercolor varchar(6) WITH DEFAULT '000000' NOT NULL,\n\
textcolor varchar(6) WITH DEFAULT '202020' NOT NULL,\n\
highlightcolor varchar(6) WITH DEFAULT 'AA4444' NOT NULL,\n\
leftpercentilecolor varchar(6) WITH DEFAULT '11CC11' NOT NULL,\n\
rightpercentilecolor varchar(6) WITH DEFAULT 'CC1111' NOT NULL,\n\
nonworktimecolor varchar(6) WITH DEFAULT 'CCCCCC' NOT NULL,\n\
gridview integer WITH DEFAULT 1 NOT NULL,\n\
legendview integer WITH DEFAULT 1 NOT NULL,\n\
PRIMARY KEY (graphthemeid)\n\
);\n\
CREATE INDEX graph_theme_1 ON graph_theme (description);\n\
CREATE INDEX graph_theme_2 ON graph_theme (theme);\n\
CREATE TABLE globalmacro (\n\
globalmacroid bigint  NOT NULL,\n\
macro varchar(64) WITH DEFAULT '' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (globalmacroid)\n\
);\n\
CREATE INDEX globalmacro_1 ON globalmacro (macro);\n\
CREATE TABLE hostmacro (\n\
hostmacroid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
macro varchar(64) WITH DEFAULT '' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostmacroid)\n\
);\n\
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);\n\
CREATE TABLE hosts_groups (\n\
hostgroupid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (hostgroupid)\n\
);\n\
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);\n\
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);\n\
CREATE TABLE hosts_templates (\n\
hosttemplateid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
templateid bigint  NOT NULL,\n\
PRIMARY KEY (hosttemplateid)\n\
);\n\
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);\n\
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);\n\
CREATE TABLE items_applications (\n\
itemappid bigint  NOT NULL,\n\
applicationid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
PRIMARY KEY (itemappid)\n\
);\n\
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);\n\
CREATE INDEX items_applications_2 ON items_applications (itemid);\n\
CREATE TABLE mappings (\n\
mappingid bigint  NOT NULL,\n\
valuemapid bigint  NOT NULL,\n\
value varchar(64) WITH DEFAULT '' NOT NULL,\n\
newvalue varchar(64) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (mappingid)\n\
);\n\
CREATE INDEX mappings_1 ON mappings (valuemapid);\n\
CREATE TABLE media (\n\
mediaid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
mediatypeid bigint  NOT NULL,\n\
sendto varchar(100) WITH DEFAULT '' NOT NULL,\n\
active integer WITH DEFAULT '0' NOT NULL,\n\
severity integer WITH DEFAULT '63' NOT NULL,\n\
period varchar(100) WITH DEFAULT '1-7,00:00-24:00' NOT NULL,\n\
PRIMARY KEY (mediaid)\n\
);\n\
CREATE INDEX media_1 ON media (userid);\n\
CREATE INDEX media_2 ON media (mediatypeid);\n\
CREATE TABLE rights (\n\
rightid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
permission integer WITH DEFAULT '0' NOT NULL,\n\
id bigint  NOT NULL,\n\
PRIMARY KEY (rightid)\n\
);\n\
CREATE INDEX rights_1 ON rights (groupid);\n\
CREATE INDEX rights_2 ON rights (id);\n\
CREATE TABLE services (\n\
serviceid bigint  NOT NULL,\n\
name varchar(128) WITH DEFAULT '' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
algorithm integer WITH DEFAULT '0' NOT NULL,\n\
triggerid bigint  NULL,\n\
showsla integer WITH DEFAULT '0' NOT NULL,\n\
goodsla decfloat(16) WITH DEFAULT '99.9' NOT NULL,\n\
sortorder integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (serviceid)\n\
);\n\
CREATE INDEX services_1 ON services (triggerid);\n\
CREATE TABLE services_links (\n\
linkid bigint  NOT NULL,\n\
serviceupid bigint  NOT NULL,\n\
servicedownid bigint  NOT NULL,\n\
soft integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX services_links_1 ON services_links (servicedownid);\n\
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);\n\
CREATE TABLE services_times (\n\
timeid bigint  NOT NULL,\n\
serviceid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
ts_from integer WITH DEFAULT '0' NOT NULL,\n\
ts_to integer WITH DEFAULT '0' NOT NULL,\n\
note varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (timeid)\n\
);\n\
CREATE INDEX services_times_1 ON services_times (serviceid,type,ts_from,ts_to);\n\
CREATE TABLE icon_map (\n\
iconmapid bigint  NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
default_iconid bigint  NOT NULL,\n\
PRIMARY KEY (iconmapid)\n\
);\n\
CREATE INDEX icon_map_1 ON icon_map (name);\n\
CREATE INDEX icon_map_2 ON icon_map (default_iconid);\n\
CREATE TABLE icon_mapping (\n\
iconmappingid bigint  NOT NULL,\n\
iconmapid bigint  NOT NULL,\n\
iconid bigint  NOT NULL,\n\
inventory_link integer WITH DEFAULT '0' NOT NULL,\n\
expression varchar(64) WITH DEFAULT '' NOT NULL,\n\
sortorder integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (iconmappingid)\n\
);\n\
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);\n\
CREATE INDEX icon_mapping_2 ON icon_mapping (iconid);\n\
CREATE TABLE sysmaps (\n\
sysmapid bigint  NOT NULL,\n\
name varchar(128) WITH DEFAULT '' NOT NULL,\n\
width integer WITH DEFAULT '600' NOT NULL,\n\
height integer WITH DEFAULT '400' NOT NULL,\n\
backgroundid bigint  NULL,\n\
label_type integer WITH DEFAULT '2' NOT NULL,\n\
label_location integer WITH DEFAULT '0' NOT NULL,\n\
highlight integer WITH DEFAULT '1' NOT NULL,\n\
expandproblem integer WITH DEFAULT '1' NOT NULL,\n\
markelements integer WITH DEFAULT '0' NOT NULL,\n\
show_unack integer WITH DEFAULT '0' NOT NULL,\n\
grid_size integer WITH DEFAULT '50' NOT NULL,\n\
grid_show integer WITH DEFAULT '1' NOT NULL,\n\
grid_align integer WITH DEFAULT '1' NOT NULL,\n\
label_format integer WITH DEFAULT '0' NOT NULL,\n\
label_type_host integer WITH DEFAULT '2' NOT NULL,\n\
label_type_hostgroup integer WITH DEFAULT '2' NOT NULL,\n\
label_type_trigger integer WITH DEFAULT '2' NOT NULL,\n\
label_type_map integer WITH DEFAULT '2' NOT NULL,\n\
label_type_image integer WITH DEFAULT '2' NOT NULL,\n\
label_string_host varchar(255) WITH DEFAULT '' NOT NULL,\n\
label_string_hostgroup varchar(255) WITH DEFAULT '' NOT NULL,\n\
label_string_trigger varchar(255) WITH DEFAULT '' NOT NULL,\n\
label_string_map varchar(255) WITH DEFAULT '' NOT NULL,\n\
label_string_image varchar(255) WITH DEFAULT '' NOT NULL,\n\
iconmapid bigint  NULL,\n\
expand_macros integer WITH DEFAULT '0' NOT NULL,\n\
severity_min integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapid)\n\
);\n\
CREATE INDEX sysmaps_1 ON sysmaps (name);\n\
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);\n\
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);\n\
CREATE TABLE sysmaps_elements (\n\
selementid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL,\n\
elementid bigint WITH DEFAULT '0' NOT NULL,\n\
elementtype integer WITH DEFAULT '0' NOT NULL,\n\
iconid_off bigint  NULL,\n\
iconid_on bigint  NULL,\n\
label varchar(2048) WITH DEFAULT '' NOT NULL,\n\
label_location integer WITH DEFAULT '-1' NOT NULL,\n\
x integer WITH DEFAULT '0' NOT NULL,\n\
y integer WITH DEFAULT '0' NOT NULL,\n\
iconid_disabled bigint  NULL,\n\
iconid_maintenance bigint  NULL,\n\
elementsubtype integer WITH DEFAULT '0' NOT NULL,\n\
areatype integer WITH DEFAULT '0' NOT NULL,\n\
width integer WITH DEFAULT '200' NOT NULL,\n\
height integer WITH DEFAULT '200' NOT NULL,\n\
viewtype integer WITH DEFAULT '0' NOT NULL,\n\
use_iconmap integer WITH DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (selementid)\n\
);\n\
CREATE INDEX sysmaps_elements_1 ON sysmaps_elements (sysmapid);\n\
CREATE INDEX sysmaps_elements_2 ON sysmaps_elements (iconid_off);\n\
CREATE INDEX sysmaps_elements_3 ON sysmaps_elements (iconid_on);\n\
CREATE INDEX sysmaps_elements_4 ON sysmaps_elements (iconid_disabled);\n\
CREATE INDEX sysmaps_elements_5 ON sysmaps_elements (iconid_maintenance);\n\
CREATE TABLE sysmaps_links (\n\
linkid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL,\n\
selementid1 bigint  NOT NULL,\n\
selementid2 bigint  NOT NULL,\n\
drawtype integer WITH DEFAULT '0' NOT NULL,\n\
color varchar(6) WITH DEFAULT '000000' NOT NULL,\n\
label varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX sysmaps_links_1 ON sysmaps_links (sysmapid);\n\
CREATE INDEX sysmaps_links_2 ON sysmaps_links (selementid1);\n\
CREATE INDEX sysmaps_links_3 ON sysmaps_links (selementid2);\n\
CREATE TABLE sysmaps_link_triggers (\n\
linktriggerid bigint  NOT NULL,\n\
linkid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL,\n\
drawtype integer WITH DEFAULT '0' NOT NULL,\n\
color varchar(6) WITH DEFAULT '000000' NOT NULL,\n\
PRIMARY KEY (linktriggerid)\n\
);\n\
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);\n\
CREATE INDEX sysmaps_link_triggers_2 ON sysmaps_link_triggers (triggerid);\n\
CREATE TABLE sysmap_element_url (\n\
sysmapelementurlid bigint  NOT NULL,\n\
selementid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
url varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (sysmapelementurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);\n\
CREATE TABLE sysmap_url (\n\
sysmapurlid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
url varchar(255) WITH DEFAULT '' NOT NULL,\n\
elementtype integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);\n\
CREATE TABLE maintenances_hosts (\n\
maintenance_hostid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
PRIMARY KEY (maintenance_hostid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);\n\
CREATE INDEX maintenances_hosts_2 ON maintenances_hosts (hostid);\n\
CREATE TABLE maintenances_groups (\n\
maintenance_groupid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (maintenance_groupid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);\n\
CREATE INDEX maintenances_groups_2 ON maintenances_groups (groupid);\n\
CREATE TABLE timeperiods (\n\
timeperiodid bigint  NOT NULL,\n\
timeperiod_type integer WITH DEFAULT '0' NOT NULL,\n\
every integer WITH DEFAULT '0' NOT NULL,\n\
month integer WITH DEFAULT '0' NOT NULL,\n\
dayofweek integer WITH DEFAULT '0' NOT NULL,\n\
day integer WITH DEFAULT '0' NOT NULL,\n\
start_time integer WITH DEFAULT '0' NOT NULL,\n\
period integer WITH DEFAULT '0' NOT NULL,\n\
start_date integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (timeperiodid)\n\
);\n\
CREATE TABLE maintenances_windows (\n\
maintenance_timeperiodid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL,\n\
timeperiodid bigint  NOT NULL,\n\
PRIMARY KEY (maintenance_timeperiodid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);\n\
CREATE INDEX maintenances_windows_2 ON maintenances_windows (timeperiodid);\n\
CREATE TABLE regexps (\n\
regexpid bigint  NOT NULL,\n\
name varchar(128) WITH DEFAULT '' NOT NULL,\n\
test_string varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (regexpid)\n\
);\n\
CREATE INDEX regexps_1 ON regexps (name);\n\
CREATE TABLE expressions (\n\
expressionid bigint  NOT NULL,\n\
regexpid bigint  NOT NULL,\n\
expression varchar(255) WITH DEFAULT '' NOT NULL,\n\
expression_type integer WITH DEFAULT '0' NOT NULL,\n\
exp_delimiter varchar(1) WITH DEFAULT '' NOT NULL,\n\
case_sensitive integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (expressionid)\n\
);\n\
CREATE INDEX expressions_1 ON expressions (regexpid);\n\
CREATE TABLE nodes (\n\
nodeid integer  NOT NULL,\n\
name varchar(64) WITH DEFAULT '0' NOT NULL,\n\
ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
port integer WITH DEFAULT '10051' NOT NULL,\n\
nodetype integer WITH DEFAULT '0' NOT NULL,\n\
masterid integer  NULL,\n\
PRIMARY KEY (nodeid)\n\
);\n\
CREATE INDEX nodes_1 ON nodes (masterid);\n\
CREATE TABLE node_cksum (\n\
nodeid integer  NOT NULL,\n\
tablename varchar(64) WITH DEFAULT '' NOT NULL,\n\
recordid bigint  NOT NULL,\n\
cksumtype integer WITH DEFAULT '0' NOT NULL,\n\
cksum varchar(2048) WITH DEFAULT '' NOT NULL,\n\
sync varchar(128) WITH DEFAULT '' NOT NULL\n\
);\n\
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);\n\
CREATE TABLE ids (\n\
nodeid integer  NOT NULL,\n\
table_name varchar(64) WITH DEFAULT '' NOT NULL,\n\
field_name varchar(64) WITH DEFAULT '' NOT NULL,\n\
nextid bigint  NOT NULL,\n\
PRIMARY KEY (nodeid,table_name,field_name)\n\
);\n\
CREATE TABLE alerts (\n\
alertid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
eventid bigint  NOT NULL,\n\
userid bigint  NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
mediatypeid bigint  NULL,\n\
sendto varchar(100) WITH DEFAULT '' NOT NULL,\n\
subject varchar(255) WITH DEFAULT '' NOT NULL,\n\
message varchar(2048) WITH DEFAULT '' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
retries integer WITH DEFAULT '0' NOT NULL,\n\
error varchar(128) WITH DEFAULT '' NOT NULL,\n\
esc_step integer WITH DEFAULT '0' NOT NULL,\n\
alerttype integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (alertid)\n\
);\n\
CREATE INDEX alerts_1 ON alerts (actionid);\n\
CREATE INDEX alerts_2 ON alerts (clock);\n\
CREATE INDEX alerts_3 ON alerts (eventid);\n\
CREATE INDEX alerts_4 ON alerts (status,retries);\n\
CREATE INDEX alerts_5 ON alerts (mediatypeid);\n\
CREATE INDEX alerts_6 ON alerts (userid);\n\
CREATE TABLE history (\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value decfloat(16) WITH DEFAULT '0.0000' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_1 ON history (itemid,clock);\n\
CREATE TABLE history_sync (\n\
id bigint  NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value decfloat(16) WITH DEFAULT '0.0000' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_sync_1 ON history_sync (nodeid,id);\n\
CREATE TABLE history_uint (\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value bigint WITH DEFAULT '0' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);\n\
CREATE TABLE history_uint_sync (\n\
id bigint  NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value bigint WITH DEFAULT '0' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_uint_sync_1 ON history_uint_sync (nodeid,id);\n\
CREATE TABLE history_str (\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_str_1 ON history_str (itemid,clock);\n\
CREATE TABLE history_str_sync (\n\
id bigint  NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_str_sync_1 ON history_str_sync (nodeid,id);\n\
CREATE TABLE history_log (\n\
id bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
timestamp integer WITH DEFAULT '0' NOT NULL,\n\
source varchar(64) WITH DEFAULT '' NOT NULL,\n\
severity integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(2048) WITH DEFAULT '' NOT NULL,\n\
logeventid integer WITH DEFAULT '0' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_log_1 ON history_log (itemid,clock);\n\
CREATE UNIQUE INDEX history_log_2 ON history_log (itemid,id);\n\
CREATE TABLE history_text (\n\
id bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(2048) WITH DEFAULT '' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_text_1 ON history_text (itemid,clock);\n\
CREATE UNIQUE INDEX history_text_2 ON history_text (itemid,id);\n\
CREATE TABLE proxy_history (\n\
id bigint  NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
timestamp integer WITH DEFAULT '0' NOT NULL,\n\
source varchar(64) WITH DEFAULT '' NOT NULL,\n\
severity integer WITH DEFAULT '0' NOT NULL,\n\
value varchar(2048) WITH DEFAULT '' NOT NULL,\n\
logeventid integer WITH DEFAULT '0' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
state integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_history_1 ON proxy_history (clock);\n\
CREATE TABLE proxy_dhistory (\n\
id bigint  NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
druleid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
port integer WITH DEFAULT '0' NOT NULL,\n\
key_ varchar(255) WITH DEFAULT '' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
dcheckid bigint  NULL,\n\
dns varchar(64) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);\n\
CREATE TABLE events (\n\
eventid bigint  NOT NULL,\n\
source integer WITH DEFAULT '0' NOT NULL,\n\
object integer WITH DEFAULT '0' NOT NULL,\n\
objectid bigint WITH DEFAULT '0' NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value integer WITH DEFAULT '0' NOT NULL,\n\
acknowledged integer WITH DEFAULT '0' NOT NULL,\n\
ns integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (eventid)\n\
);\n\
CREATE INDEX events_1 ON events (source,object,objectid,clock);\n\
CREATE INDEX events_2 ON events (source,object,clock);\n\
CREATE TABLE trends (\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
num integer WITH DEFAULT '0' NOT NULL,\n\
value_min decfloat(16) WITH DEFAULT '0.0000' NOT NULL,\n\
value_avg decfloat(16) WITH DEFAULT '0.0000' NOT NULL,\n\
value_max decfloat(16) WITH DEFAULT '0.0000' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE trends_uint (\n\
itemid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
num integer WITH DEFAULT '0' NOT NULL,\n\
value_min bigint WITH DEFAULT '0' NOT NULL,\n\
value_avg bigint WITH DEFAULT '0' NOT NULL,\n\
value_max bigint WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE acknowledges (\n\
acknowledgeid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
eventid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
message varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (acknowledgeid)\n\
);\n\
CREATE INDEX acknowledges_1 ON acknowledges (userid);\n\
CREATE INDEX acknowledges_2 ON acknowledges (eventid);\n\
CREATE INDEX acknowledges_3 ON acknowledges (clock);\n\
CREATE TABLE auditlog (\n\
auditid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
action integer WITH DEFAULT '0' NOT NULL,\n\
resourcetype integer WITH DEFAULT '0' NOT NULL,\n\
details varchar(128)  WITH DEFAULT '0' NOT NULL,\n\
ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
resourceid bigint WITH DEFAULT '0' NOT NULL,\n\
resourcename varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditid)\n\
);\n\
CREATE INDEX auditlog_1 ON auditlog (userid,clock);\n\
CREATE INDEX auditlog_2 ON auditlog (clock);\n\
CREATE TABLE auditlog_details (\n\
auditdetailid bigint  NOT NULL,\n\
auditid bigint  NOT NULL,\n\
table_name varchar(64) WITH DEFAULT '' NOT NULL,\n\
field_name varchar(64) WITH DEFAULT '' NOT NULL,\n\
oldvalue varchar(2048) WITH DEFAULT '' NOT NULL,\n\
newvalue varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditdetailid)\n\
);\n\
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);\n\
CREATE TABLE service_alarms (\n\
servicealarmid bigint  NOT NULL,\n\
serviceid bigint  NOT NULL,\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
value integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (servicealarmid)\n\
);\n\
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);\n\
CREATE INDEX service_alarms_2 ON service_alarms (clock);\n\
CREATE TABLE autoreg_host (\n\
autoreg_hostid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL,\n\
host varchar(64) WITH DEFAULT '' NOT NULL,\n\
listen_ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
listen_port integer WITH DEFAULT '0' NOT NULL,\n\
listen_dns varchar(64) WITH DEFAULT '' NOT NULL,\n\
host_metadata varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (autoreg_hostid)\n\
);\n\
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);\n\
CREATE TABLE proxy_autoreg_host (\n\
id bigint  NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),\n\
clock integer WITH DEFAULT '0' NOT NULL,\n\
host varchar(64) WITH DEFAULT '' NOT NULL,\n\
listen_ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
listen_port integer WITH DEFAULT '0' NOT NULL,\n\
listen_dns varchar(64) WITH DEFAULT '' NOT NULL,\n\
host_metadata varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);\n\
CREATE TABLE dhosts (\n\
dhostid bigint  NOT NULL,\n\
druleid bigint  NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
lastup integer WITH DEFAULT '0' NOT NULL,\n\
lastdown integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (dhostid)\n\
);\n\
CREATE INDEX dhosts_1 ON dhosts (druleid);\n\
CREATE TABLE dservices (\n\
dserviceid bigint  NOT NULL,\n\
dhostid bigint  NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
key_ varchar(255) WITH DEFAULT '' NOT NULL,\n\
value varchar(255) WITH DEFAULT '' NOT NULL,\n\
port integer WITH DEFAULT '0' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
lastup integer WITH DEFAULT '0' NOT NULL,\n\
lastdown integer WITH DEFAULT '0' NOT NULL,\n\
dcheckid bigint  NOT NULL,\n\
ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
dns varchar(64) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dserviceid)\n\
);\n\
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,type,key_,ip,port);\n\
CREATE INDEX dservices_2 ON dservices (dhostid);\n\
CREATE TABLE escalations (\n\
escalationid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
triggerid bigint  NULL,\n\
eventid bigint  NULL,\n\
r_eventid bigint  NULL,\n\
nextcheck integer WITH DEFAULT '0' NOT NULL,\n\
esc_step integer WITH DEFAULT '0' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
itemid bigint  NULL,\n\
PRIMARY KEY (escalationid)\n\
);\n\
CREATE UNIQUE INDEX escalations_1 ON escalations (actionid,triggerid,itemid,escalationid);\n\
CREATE TABLE globalvars (\n\
globalvarid bigint  NOT NULL,\n\
snmp_lastsize integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (globalvarid)\n\
);\n\
CREATE TABLE graph_discovery (\n\
graphdiscoveryid bigint  NOT NULL,\n\
graphid bigint  NOT NULL,\n\
parent_graphid bigint  NOT NULL,\n\
name varchar(128) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (graphdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX graph_discovery_1 ON graph_discovery (graphid,parent_graphid);\n\
CREATE INDEX graph_discovery_2 ON graph_discovery (parent_graphid);\n\
CREATE TABLE host_inventory (\n\
hostid bigint  NOT NULL,\n\
inventory_mode integer WITH DEFAULT '0' NOT NULL,\n\
type varchar(64) WITH DEFAULT '' NOT NULL,\n\
type_full varchar(64) WITH DEFAULT '' NOT NULL,\n\
name varchar(64) WITH DEFAULT '' NOT NULL,\n\
alias varchar(64) WITH DEFAULT '' NOT NULL,\n\
os varchar(64) WITH DEFAULT '' NOT NULL,\n\
os_full varchar(255) WITH DEFAULT '' NOT NULL,\n\
os_short varchar(64) WITH DEFAULT '' NOT NULL,\n\
serialno_a varchar(64) WITH DEFAULT '' NOT NULL,\n\
serialno_b varchar(64) WITH DEFAULT '' NOT NULL,\n\
tag varchar(64) WITH DEFAULT '' NOT NULL,\n\
asset_tag varchar(64) WITH DEFAULT '' NOT NULL,\n\
macaddress_a varchar(64) WITH DEFAULT '' NOT NULL,\n\
macaddress_b varchar(64) WITH DEFAULT '' NOT NULL,\n\
hardware varchar(255) WITH DEFAULT '' NOT NULL,\n\
hardware_full varchar(2048) WITH DEFAULT '' NOT NULL,\n\
software varchar(255) WITH DEFAULT '' NOT NULL,\n\
software_full varchar(2048) WITH DEFAULT '' NOT NULL,\n\
software_app_a varchar(64) WITH DEFAULT '' NOT NULL,\n\
software_app_b varchar(64) WITH DEFAULT '' NOT NULL,\n\
software_app_c varchar(64) WITH DEFAULT '' NOT NULL,\n\
software_app_d varchar(64) WITH DEFAULT '' NOT NULL,\n\
software_app_e varchar(64) WITH DEFAULT '' NOT NULL,\n\
contact varchar(2048) WITH DEFAULT '' NOT NULL,\n\
location varchar(2048) WITH DEFAULT '' NOT NULL,\n\
location_lat varchar(16) WITH DEFAULT '' NOT NULL,\n\
location_lon varchar(16) WITH DEFAULT '' NOT NULL,\n\
notes varchar(2048) WITH DEFAULT '' NOT NULL,\n\
chassis varchar(64) WITH DEFAULT '' NOT NULL,\n\
model varchar(64) WITH DEFAULT '' NOT NULL,\n\
hw_arch varchar(32) WITH DEFAULT '' NOT NULL,\n\
vendor varchar(64) WITH DEFAULT '' NOT NULL,\n\
contract_number varchar(64) WITH DEFAULT '' NOT NULL,\n\
installer_name varchar(64) WITH DEFAULT '' NOT NULL,\n\
deployment_status varchar(64) WITH DEFAULT '' NOT NULL,\n\
url_a varchar(255) WITH DEFAULT '' NOT NULL,\n\
url_b varchar(255) WITH DEFAULT '' NOT NULL,\n\
url_c varchar(255) WITH DEFAULT '' NOT NULL,\n\
host_networks varchar(2048) WITH DEFAULT '' NOT NULL,\n\
host_netmask varchar(39) WITH DEFAULT '' NOT NULL,\n\
host_router varchar(39) WITH DEFAULT '' NOT NULL,\n\
oob_ip varchar(39) WITH DEFAULT '' NOT NULL,\n\
oob_netmask varchar(39) WITH DEFAULT '' NOT NULL,\n\
oob_router varchar(39) WITH DEFAULT '' NOT NULL,\n\
date_hw_purchase varchar(64) WITH DEFAULT '' NOT NULL,\n\
date_hw_install varchar(64) WITH DEFAULT '' NOT NULL,\n\
date_hw_expiry varchar(64) WITH DEFAULT '' NOT NULL,\n\
date_hw_decomm varchar(64) WITH DEFAULT '' NOT NULL,\n\
site_address_a varchar(128) WITH DEFAULT '' NOT NULL,\n\
site_address_b varchar(128) WITH DEFAULT '' NOT NULL,\n\
site_address_c varchar(128) WITH DEFAULT '' NOT NULL,\n\
site_city varchar(128) WITH DEFAULT '' NOT NULL,\n\
site_state varchar(64) WITH DEFAULT '' NOT NULL,\n\
site_country varchar(64) WITH DEFAULT '' NOT NULL,\n\
site_zip varchar(64) WITH DEFAULT '' NOT NULL,\n\
site_rack varchar(128) WITH DEFAULT '' NOT NULL,\n\
site_notes varchar(2048) WITH DEFAULT '' NOT NULL,\n\
poc_1_name varchar(128) WITH DEFAULT '' NOT NULL,\n\
poc_1_email varchar(128) WITH DEFAULT '' NOT NULL,\n\
poc_1_phone_a varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_1_phone_b varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_1_cell varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_1_screen varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_1_notes varchar(2048) WITH DEFAULT '' NOT NULL,\n\
poc_2_name varchar(128) WITH DEFAULT '' NOT NULL,\n\
poc_2_email varchar(128) WITH DEFAULT '' NOT NULL,\n\
poc_2_phone_a varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_2_phone_b varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_2_cell varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_2_screen varchar(64) WITH DEFAULT '' NOT NULL,\n\
poc_2_notes varchar(2048) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE housekeeper (\n\
housekeeperid bigint  NOT NULL,\n\
tablename varchar(64) WITH DEFAULT '' NOT NULL,\n\
field varchar(64) WITH DEFAULT '' NOT NULL,\n\
value bigint  NOT NULL,\n\
PRIMARY KEY (housekeeperid)\n\
);\n\
CREATE TABLE images (\n\
imageid bigint  NOT NULL,\n\
imagetype integer WITH DEFAULT '0' NOT NULL,\n\
name varchar(64) WITH DEFAULT '0' NOT NULL,\n\
image blob  NOT NULL,\n\
PRIMARY KEY (imageid)\n\
);\n\
CREATE INDEX images_1 ON images (imagetype,name);\n\
CREATE TABLE item_discovery (\n\
itemdiscoveryid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
parent_itemid bigint  NOT NULL,\n\
key_ varchar(255) WITH DEFAULT '' NOT NULL,\n\
lastcheck integer WITH DEFAULT '0' NOT NULL,\n\
ts_delete integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);\n\
CREATE INDEX item_discovery_2 ON item_discovery (parent_itemid);\n\
CREATE TABLE host_discovery (\n\
hostid bigint  NOT NULL,\n\
parent_hostid bigint  NULL,\n\
parent_itemid bigint  NULL,\n\
host varchar(64) WITH DEFAULT '' NOT NULL,\n\
lastcheck integer WITH DEFAULT '0' NOT NULL,\n\
ts_delete integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE interface_discovery (\n\
interfaceid bigint  NOT NULL,\n\
parent_interfaceid bigint  NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE TABLE profiles (\n\
profileid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
idx varchar(96) WITH DEFAULT '' NOT NULL,\n\
idx2 bigint WITH DEFAULT '0' NOT NULL,\n\
value_id bigint WITH DEFAULT '0' NOT NULL,\n\
value_int integer WITH DEFAULT '0' NOT NULL,\n\
value_str varchar(255) WITH DEFAULT '' NOT NULL,\n\
source varchar(96) WITH DEFAULT '' NOT NULL,\n\
type integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (profileid)\n\
);\n\
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);\n\
CREATE INDEX profiles_2 ON profiles (userid,profileid);\n\
CREATE TABLE sessions (\n\
sessionid varchar(32) WITH DEFAULT '' NOT NULL,\n\
userid bigint  NOT NULL,\n\
lastaccess integer WITH DEFAULT '0' NOT NULL,\n\
status integer WITH DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sessionid)\n\
);\n\
CREATE INDEX sessions_1 ON sessions (userid,status);\n\
CREATE TABLE trigger_discovery (\n\
triggerdiscoveryid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL,\n\
parent_triggerid bigint  NOT NULL,\n\
name varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (triggerdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX trigger_discovery_1 ON trigger_discovery (triggerid,parent_triggerid);\n\
CREATE INDEX trigger_discovery_2 ON trigger_discovery (parent_triggerid);\n\
CREATE TABLE user_history (\n\
userhistoryid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
title1 varchar(255) WITH DEFAULT '' NOT NULL,\n\
url1 varchar(255) WITH DEFAULT '' NOT NULL,\n\
title2 varchar(255) WITH DEFAULT '' NOT NULL,\n\
url2 varchar(255) WITH DEFAULT '' NOT NULL,\n\
title3 varchar(255) WITH DEFAULT '' NOT NULL,\n\
url3 varchar(255) WITH DEFAULT '' NOT NULL,\n\
title4 varchar(255) WITH DEFAULT '' NOT NULL,\n\
url4 varchar(255) WITH DEFAULT '' NOT NULL,\n\
title5 varchar(255) WITH DEFAULT '' NOT NULL,\n\
url5 varchar(255) WITH DEFAULT '' NOT NULL,\n\
PRIMARY KEY (userhistoryid)\n\
);\n\
CREATE UNIQUE INDEX user_history_1 ON user_history (userid);\n\
CREATE TABLE application_template (\n\
application_templateid bigint  NOT NULL,\n\
applicationid bigint  NOT NULL,\n\
templateid bigint  NOT NULL,\n\
PRIMARY KEY (application_templateid)\n\
);\n\
CREATE UNIQUE INDEX application_template_1 ON application_template (applicationid,templateid);\n\
CREATE INDEX application_template_2 ON application_template (templateid);\n\
CREATE TABLE dbversion (\n\
mandatory integer WITH DEFAULT '0' NOT NULL,\n\
optional integer WITH DEFAULT '0' NOT NULL\n\
);\n\
INSERT INTO dbversion VALUES ('2020000','2020001');\n\
";
const char	*const db_schema_fkeys[] = {
	"ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid)",
	"ALTER TABLE hosts ADD CONSTRAINT c_hosts_3 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_3 FOREIGN KEY (templateid) REFERENCES group_prototype (group_prototypeid) ON DELETE CASCADE",
	"ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_1 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_2 FOREIGN KEY (parent_group_prototypeid) REFERENCES group_prototype (group_prototypeid)",
	"ALTER TABLE screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE",
	"ALTER TABLE slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE",
	"ALTER TABLE slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE",
	"ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE",
	"ALTER TABLE applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid)",
	"ALTER TABLE httptest ADD CONSTRAINT c_httptest_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE httptest ADD CONSTRAINT c_httptest_3 FOREIGN KEY (templateid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid)",
	"ALTER TABLE items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid)",
	"ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE",
	"ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE",
	"ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid)",
	"ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid)",
	"ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid)",
	"ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid)",
	"ALTER TABLE opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid)",
	"ALTER TABLE triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid)",
	"ALTER TABLE graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid)",
	"ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	"ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE",
	"ALTER TABLE media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE",
	"ALTER TABLE rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE",
	"ALTER TABLE rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid)",
	"ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE",
	"ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_1 FOREIGN KEY (backgroundid) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_2 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_2 FOREIGN KEY (iconid_off) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_3 FOREIGN KEY (iconid_on) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_4 FOREIGN KEY (iconid_disabled) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_5 FOREIGN KEY (iconid_maintenance) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE",
	"ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE",
	"ALTER TABLE nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid)",
	"ALTER TABLE node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE",
	"ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE",
	"ALTER TABLE auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE",
	"ALTER TABLE service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE",
	"ALTER TABLE dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE",
	"ALTER TABLE dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE",
	"ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_2 FOREIGN KEY (parent_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_3 FOREIGN KEY (parent_itemid) REFERENCES items (itemid)",
	"ALTER TABLE interface_discovery ADD CONSTRAINT c_interface_discovery_1 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE",
	"ALTER TABLE interface_discovery ADD CONSTRAINT c_interface_discovery_2 FOREIGN KEY (parent_interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE",
	"ALTER TABLE profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE application_template ADD CONSTRAINT c_application_template_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	"ALTER TABLE application_template ADD CONSTRAINT c_application_template_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	NULL
};
const char	*const db_schema_fkeys_drop[] = {
	"ALTER TABLE hosts DROP CONSTRAINT c_hosts_1",
	"ALTER TABLE hosts DROP CONSTRAINT c_hosts_2",
	"ALTER TABLE hosts DROP CONSTRAINT c_hosts_3",
	"ALTER TABLE group_prototype DROP CONSTRAINT c_group_prototype_1",
	"ALTER TABLE group_prototype DROP CONSTRAINT c_group_prototype_2",
	"ALTER TABLE group_prototype DROP CONSTRAINT c_group_prototype_3",
	"ALTER TABLE group_discovery DROP CONSTRAINT c_group_discovery_1",
	"ALTER TABLE group_discovery DROP CONSTRAINT c_group_discovery_2",
	"ALTER TABLE screens DROP CONSTRAINT c_screens_1",
	"ALTER TABLE screens_items DROP CONSTRAINT c_screens_items_1",
	"ALTER TABLE slides DROP CONSTRAINT c_slides_1",
	"ALTER TABLE slides DROP CONSTRAINT c_slides_2",
	"ALTER TABLE drules DROP CONSTRAINT c_drules_1",
	"ALTER TABLE dchecks DROP CONSTRAINT c_dchecks_1",
	"ALTER TABLE applications DROP CONSTRAINT c_applications_1",
	"ALTER TABLE httptest DROP CONSTRAINT c_httptest_1",
	"ALTER TABLE httptest DROP CONSTRAINT c_httptest_2",
	"ALTER TABLE httptest DROP CONSTRAINT c_httptest_3",
	"ALTER TABLE httpstep DROP CONSTRAINT c_httpstep_1",
	"ALTER TABLE interface DROP CONSTRAINT c_interface_1",
	"ALTER TABLE items DROP CONSTRAINT c_items_1",
	"ALTER TABLE items DROP CONSTRAINT c_items_2",
	"ALTER TABLE items DROP CONSTRAINT c_items_3",
	"ALTER TABLE items DROP CONSTRAINT c_items_4",
	"ALTER TABLE httpstepitem DROP CONSTRAINT c_httpstepitem_1",
	"ALTER TABLE httpstepitem DROP CONSTRAINT c_httpstepitem_2",
	"ALTER TABLE httptestitem DROP CONSTRAINT c_httptestitem_1",
	"ALTER TABLE httptestitem DROP CONSTRAINT c_httptestitem_2",
	"ALTER TABLE users_groups DROP CONSTRAINT c_users_groups_1",
	"ALTER TABLE users_groups DROP CONSTRAINT c_users_groups_2",
	"ALTER TABLE scripts DROP CONSTRAINT c_scripts_1",
	"ALTER TABLE scripts DROP CONSTRAINT c_scripts_2",
	"ALTER TABLE operations DROP CONSTRAINT c_operations_1",
	"ALTER TABLE opmessage DROP CONSTRAINT c_opmessage_1",
	"ALTER TABLE opmessage DROP CONSTRAINT c_opmessage_2",
	"ALTER TABLE opmessage_grp DROP CONSTRAINT c_opmessage_grp_1",
	"ALTER TABLE opmessage_grp DROP CONSTRAINT c_opmessage_grp_2",
	"ALTER TABLE opmessage_usr DROP CONSTRAINT c_opmessage_usr_1",
	"ALTER TABLE opmessage_usr DROP CONSTRAINT c_opmessage_usr_2",
	"ALTER TABLE opcommand DROP CONSTRAINT c_opcommand_1",
	"ALTER TABLE opcommand DROP CONSTRAINT c_opcommand_2",
	"ALTER TABLE opcommand_hst DROP CONSTRAINT c_opcommand_hst_1",
	"ALTER TABLE opcommand_hst DROP CONSTRAINT c_opcommand_hst_2",
	"ALTER TABLE opcommand_grp DROP CONSTRAINT c_opcommand_grp_1",
	"ALTER TABLE opcommand_grp DROP CONSTRAINT c_opcommand_grp_2",
	"ALTER TABLE opgroup DROP CONSTRAINT c_opgroup_1",
	"ALTER TABLE opgroup DROP CONSTRAINT c_opgroup_2",
	"ALTER TABLE optemplate DROP CONSTRAINT c_optemplate_1",
	"ALTER TABLE optemplate DROP CONSTRAINT c_optemplate_2",
	"ALTER TABLE opconditions DROP CONSTRAINT c_opconditions_1",
	"ALTER TABLE conditions DROP CONSTRAINT c_conditions_1",
	"ALTER TABLE config DROP CONSTRAINT c_config_1",
	"ALTER TABLE config DROP CONSTRAINT c_config_2",
	"ALTER TABLE triggers DROP CONSTRAINT c_triggers_1",
	"ALTER TABLE trigger_depends DROP CONSTRAINT c_trigger_depends_1",
	"ALTER TABLE trigger_depends DROP CONSTRAINT c_trigger_depends_2",
	"ALTER TABLE functions DROP CONSTRAINT c_functions_1",
	"ALTER TABLE functions DROP CONSTRAINT c_functions_2",
	"ALTER TABLE graphs DROP CONSTRAINT c_graphs_1",
	"ALTER TABLE graphs DROP CONSTRAINT c_graphs_2",
	"ALTER TABLE graphs DROP CONSTRAINT c_graphs_3",
	"ALTER TABLE graphs_items DROP CONSTRAINT c_graphs_items_1",
	"ALTER TABLE graphs_items DROP CONSTRAINT c_graphs_items_2",
	"ALTER TABLE hostmacro DROP CONSTRAINT c_hostmacro_1",
	"ALTER TABLE hosts_groups DROP CONSTRAINT c_hosts_groups_1",
	"ALTER TABLE hosts_groups DROP CONSTRAINT c_hosts_groups_2",
	"ALTER TABLE hosts_templates DROP CONSTRAINT c_hosts_templates_1",
	"ALTER TABLE hosts_templates DROP CONSTRAINT c_hosts_templates_2",
	"ALTER TABLE items_applications DROP CONSTRAINT c_items_applications_1",
	"ALTER TABLE items_applications DROP CONSTRAINT c_items_applications_2",
	"ALTER TABLE mappings DROP CONSTRAINT c_mappings_1",
	"ALTER TABLE media DROP CONSTRAINT c_media_1",
	"ALTER TABLE media DROP CONSTRAINT c_media_2",
	"ALTER TABLE rights DROP CONSTRAINT c_rights_1",
	"ALTER TABLE rights DROP CONSTRAINT c_rights_2",
	"ALTER TABLE services DROP CONSTRAINT c_services_1",
	"ALTER TABLE services_links DROP CONSTRAINT c_services_links_1",
	"ALTER TABLE services_links DROP CONSTRAINT c_services_links_2",
	"ALTER TABLE services_times DROP CONSTRAINT c_services_times_1",
	"ALTER TABLE icon_map DROP CONSTRAINT c_icon_map_1",
	"ALTER TABLE icon_mapping DROP CONSTRAINT c_icon_mapping_1",
	"ALTER TABLE icon_mapping DROP CONSTRAINT c_icon_mapping_2",
	"ALTER TABLE sysmaps DROP CONSTRAINT c_sysmaps_1",
	"ALTER TABLE sysmaps DROP CONSTRAINT c_sysmaps_2",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_1",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_2",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_3",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_4",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_5",
	"ALTER TABLE sysmaps_links DROP CONSTRAINT c_sysmaps_links_1",
	"ALTER TABLE sysmaps_links DROP CONSTRAINT c_sysmaps_links_2",
	"ALTER TABLE sysmaps_links DROP CONSTRAINT c_sysmaps_links_3",
	"ALTER TABLE sysmaps_link_triggers DROP CONSTRAINT c_sysmaps_link_triggers_1",
	"ALTER TABLE sysmaps_link_triggers DROP CONSTRAINT c_sysmaps_link_triggers_2",
	"ALTER TABLE sysmap_element_url DROP CONSTRAINT c_sysmap_element_url_1",
	"ALTER TABLE sysmap_url DROP CONSTRAINT c_sysmap_url_1",
	"ALTER TABLE maintenances_hosts DROP CONSTRAINT c_maintenances_hosts_1",
	"ALTER TABLE maintenances_hosts DROP CONSTRAINT c_maintenances_hosts_2",
	"ALTER TABLE maintenances_groups DROP CONSTRAINT c_maintenances_groups_1",
	"ALTER TABLE maintenances_groups DROP CONSTRAINT c_maintenances_groups_2",
	"ALTER TABLE maintenances_windows DROP CONSTRAINT c_maintenances_windows_1",
	"ALTER TABLE maintenances_windows DROP CONSTRAINT c_maintenances_windows_2",
	"ALTER TABLE expressions DROP CONSTRAINT c_expressions_1",
	"ALTER TABLE nodes DROP CONSTRAINT c_nodes_1",
	"ALTER TABLE node_cksum DROP CONSTRAINT c_node_cksum_1",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_1",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_2",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_3",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_4",
	"ALTER TABLE acknowledges DROP CONSTRAINT c_acknowledges_1",
	"ALTER TABLE acknowledges DROP CONSTRAINT c_acknowledges_2",
	"ALTER TABLE auditlog DROP CONSTRAINT c_auditlog_1",
	"ALTER TABLE auditlog_details DROP CONSTRAINT c_auditlog_details_1",
	"ALTER TABLE service_alarms DROP CONSTRAINT c_service_alarms_1",
	"ALTER TABLE autoreg_host DROP CONSTRAINT c_autoreg_host_1",
	"ALTER TABLE dhosts DROP CONSTRAINT c_dhosts_1",
	"ALTER TABLE dservices DROP CONSTRAINT c_dservices_1",
	"ALTER TABLE dservices DROP CONSTRAINT c_dservices_2",
	"ALTER TABLE graph_discovery DROP CONSTRAINT c_graph_discovery_1",
	"ALTER TABLE graph_discovery DROP CONSTRAINT c_graph_discovery_2",
	"ALTER TABLE host_inventory DROP CONSTRAINT c_host_inventory_1",
	"ALTER TABLE item_discovery DROP CONSTRAINT c_item_discovery_1",
	"ALTER TABLE item_discovery DROP CONSTRAINT c_item_discovery_2",
	"ALTER TABLE host_discovery DROP CONSTRAINT c_host_discovery_1",
	"ALTER TABLE host_discovery DROP CONSTRAINT c_host_discovery_2",
	"ALTER TABLE host_discovery DROP CONSTRAINT c_host_discovery_3",
	"ALTER TABLE interface_discovery DROP CONSTRAINT c_interface_discovery_1",
	"ALTER TABLE interface_discovery DROP CONSTRAINT c_interface_discovery_2",
	"ALTER TABLE profiles DROP CONSTRAINT c_profiles_1",
	"ALTER TABLE sessions DROP CONSTRAINT c_sessions_1",
	"ALTER TABLE trigger_discovery DROP CONSTRAINT c_trigger_discovery_1",
	"ALTER TABLE trigger_discovery DROP CONSTRAINT c_trigger_discovery_2",
	"ALTER TABLE user_history DROP CONSTRAINT c_user_history_1",
	"ALTER TABLE application_template DROP CONSTRAINT c_application_template_1",
	"ALTER TABLE application_template DROP CONSTRAINT c_application_template_2",
	NULL
};
#elif defined(HAVE_MYSQL)
const char	*const db_schema = "\
CREATE TABLE `maintenances` (\n\
`maintenanceid` bigint unsigned  NOT NULL,\n\
`name` varchar(128) DEFAULT '' NOT NULL,\n\
`maintenance_type` integer DEFAULT '0' NOT NULL,\n\
`description` text  NOT NULL,\n\
`active_since` integer DEFAULT '0' NOT NULL,\n\
`active_till` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (maintenanceid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `maintenances_1` ON `maintenances` (`active_since`,`active_till`);\n\
CREATE TABLE `hosts` (\n\
`hostid` bigint unsigned  NOT NULL,\n\
`proxy_hostid` bigint unsigned  NULL,\n\
`host` varchar(64) DEFAULT '' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`disable_until` integer DEFAULT '0' NOT NULL,\n\
`error` varchar(128) DEFAULT '' NOT NULL,\n\
`available` integer DEFAULT '0' NOT NULL,\n\
`errors_from` integer DEFAULT '0' NOT NULL,\n\
`lastaccess` integer DEFAULT '0' NOT NULL,\n\
`ipmi_authtype` integer DEFAULT '0' NOT NULL,\n\
`ipmi_privilege` integer DEFAULT '2' NOT NULL,\n\
`ipmi_username` varchar(16) DEFAULT '' NOT NULL,\n\
`ipmi_password` varchar(20) DEFAULT '' NOT NULL,\n\
`ipmi_disable_until` integer DEFAULT '0' NOT NULL,\n\
`ipmi_available` integer DEFAULT '0' NOT NULL,\n\
`snmp_disable_until` integer DEFAULT '0' NOT NULL,\n\
`snmp_available` integer DEFAULT '0' NOT NULL,\n\
`maintenanceid` bigint unsigned  NULL,\n\
`maintenance_status` integer DEFAULT '0' NOT NULL,\n\
`maintenance_type` integer DEFAULT '0' NOT NULL,\n\
`maintenance_from` integer DEFAULT '0' NOT NULL,\n\
`ipmi_errors_from` integer DEFAULT '0' NOT NULL,\n\
`snmp_errors_from` integer DEFAULT '0' NOT NULL,\n\
`ipmi_error` varchar(128) DEFAULT '' NOT NULL,\n\
`snmp_error` varchar(128) DEFAULT '' NOT NULL,\n\
`jmx_disable_until` integer DEFAULT '0' NOT NULL,\n\
`jmx_available` integer DEFAULT '0' NOT NULL,\n\
`jmx_errors_from` integer DEFAULT '0' NOT NULL,\n\
`jmx_error` varchar(128) DEFAULT '' NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`flags` integer DEFAULT '0' NOT NULL,\n\
`templateid` bigint unsigned  NULL,\n\
PRIMARY KEY (hostid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `hosts_1` ON `hosts` (`host`);\n\
CREATE INDEX `hosts_2` ON `hosts` (`status`);\n\
CREATE INDEX `hosts_3` ON `hosts` (`proxy_hostid`);\n\
CREATE INDEX `hosts_4` ON `hosts` (`name`);\n\
CREATE INDEX `hosts_5` ON `hosts` (`maintenanceid`);\n\
CREATE TABLE `groups` (\n\
`groupid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`internal` integer DEFAULT '0' NOT NULL,\n\
`flags` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `groups_1` ON `groups` (`name`);\n\
CREATE TABLE `group_prototype` (\n\
`group_prototypeid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`groupid` bigint unsigned  NULL,\n\
`templateid` bigint unsigned  NULL,\n\
PRIMARY KEY (group_prototypeid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `group_prototype_1` ON `group_prototype` (`hostid`);\n\
CREATE TABLE `group_discovery` (\n\
`groupid` bigint unsigned  NOT NULL,\n\
`parent_group_prototypeid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`lastcheck` integer DEFAULT '0' NOT NULL,\n\
`ts_delete` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `screens` (\n\
`screenid` bigint unsigned  NOT NULL,\n\
`name` varchar(255)  NOT NULL,\n\
`hsize` integer DEFAULT '1' NOT NULL,\n\
`vsize` integer DEFAULT '1' NOT NULL,\n\
`templateid` bigint unsigned  NULL,\n\
PRIMARY KEY (screenid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `screens_1` ON `screens` (`templateid`);\n\
CREATE TABLE `screens_items` (\n\
`screenitemid` bigint unsigned  NOT NULL,\n\
`screenid` bigint unsigned  NOT NULL,\n\
`resourcetype` integer DEFAULT '0' NOT NULL,\n\
`resourceid` bigint unsigned DEFAULT '0' NOT NULL,\n\
`width` integer DEFAULT '320' NOT NULL,\n\
`height` integer DEFAULT '200' NOT NULL,\n\
`x` integer DEFAULT '0' NOT NULL,\n\
`y` integer DEFAULT '0' NOT NULL,\n\
`colspan` integer DEFAULT '0' NOT NULL,\n\
`rowspan` integer DEFAULT '0' NOT NULL,\n\
`elements` integer DEFAULT '25' NOT NULL,\n\
`valign` integer DEFAULT '0' NOT NULL,\n\
`halign` integer DEFAULT '0' NOT NULL,\n\
`style` integer DEFAULT '0' NOT NULL,\n\
`url` varchar(255) DEFAULT '' NOT NULL,\n\
`dynamic` integer DEFAULT '0' NOT NULL,\n\
`sort_triggers` integer DEFAULT '0' NOT NULL,\n\
`application` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (screenitemid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `screens_items_1` ON `screens_items` (`screenid`);\n\
CREATE TABLE `slideshows` (\n\
`slideshowid` bigint unsigned  NOT NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
`delay` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideshowid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `slides` (\n\
`slideid` bigint unsigned  NOT NULL,\n\
`slideshowid` bigint unsigned  NOT NULL,\n\
`screenid` bigint unsigned  NOT NULL,\n\
`step` integer DEFAULT '0' NOT NULL,\n\
`delay` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `slides_1` ON `slides` (`slideshowid`);\n\
CREATE INDEX `slides_2` ON `slides` (`screenid`);\n\
CREATE TABLE `drules` (\n\
`druleid` bigint unsigned  NOT NULL,\n\
`proxy_hostid` bigint unsigned  NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
`iprange` varchar(255) DEFAULT '' NOT NULL,\n\
`delay` integer DEFAULT '3600' NOT NULL,\n\
`nextcheck` integer DEFAULT '0' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (druleid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `drules_1` ON `drules` (`proxy_hostid`);\n\
CREATE TABLE `dchecks` (\n\
`dcheckid` bigint unsigned  NOT NULL,\n\
`druleid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`key_` varchar(255) DEFAULT '' NOT NULL,\n\
`snmp_community` varchar(255) DEFAULT '' NOT NULL,\n\
`ports` varchar(255) DEFAULT '0' NOT NULL,\n\
`snmpv3_securityname` varchar(64) DEFAULT '' NOT NULL,\n\
`snmpv3_securitylevel` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_authpassphrase` varchar(64) DEFAULT '' NOT NULL,\n\
`snmpv3_privpassphrase` varchar(64) DEFAULT '' NOT NULL,\n\
`uniq` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_authprotocol` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_privprotocol` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_contextname` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dcheckid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `dchecks_1` ON `dchecks` (`druleid`);\n\
CREATE TABLE `applications` (\n\
`applicationid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (applicationid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `applications_2` ON `applications` (`hostid`,`name`);\n\
CREATE TABLE `httptest` (\n\
`httptestid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`applicationid` bigint unsigned  NULL,\n\
`nextcheck` integer DEFAULT '0' NOT NULL,\n\
`delay` integer DEFAULT '60' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`variables` text  NOT NULL,\n\
`agent` varchar(255) DEFAULT '' NOT NULL,\n\
`authentication` integer DEFAULT '0' NOT NULL,\n\
`http_user` varchar(64) DEFAULT '' NOT NULL,\n\
`http_password` varchar(64) DEFAULT '' NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`templateid` bigint unsigned  NULL,\n\
`http_proxy` varchar(255) DEFAULT '' NOT NULL,\n\
`retries` integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (httptestid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `httptest_1` ON `httptest` (`applicationid`);\n\
CREATE UNIQUE INDEX `httptest_2` ON `httptest` (`hostid`,`name`);\n\
CREATE INDEX `httptest_3` ON `httptest` (`status`);\n\
CREATE INDEX `httptest_4` ON `httptest` (`templateid`);\n\
CREATE TABLE `httpstep` (\n\
`httpstepid` bigint unsigned  NOT NULL,\n\
`httptestid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`no` integer DEFAULT '0' NOT NULL,\n\
`url` varchar(255) DEFAULT '' NOT NULL,\n\
`timeout` integer DEFAULT '30' NOT NULL,\n\
`posts` text  NOT NULL,\n\
`required` varchar(255) DEFAULT '' NOT NULL,\n\
`status_codes` varchar(255) DEFAULT '' NOT NULL,\n\
`variables` text  NOT NULL,\n\
PRIMARY KEY (httpstepid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `httpstep_1` ON `httpstep` (`httptestid`);\n\
CREATE TABLE `interface` (\n\
`interfaceid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`main` integer DEFAULT '0' NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`useip` integer DEFAULT '1' NOT NULL,\n\
`ip` varchar(64) DEFAULT '127.0.0.1' NOT NULL,\n\
`dns` varchar(64) DEFAULT '' NOT NULL,\n\
`port` varchar(64) DEFAULT '10050' NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `interface_1` ON `interface` (`hostid`,`type`);\n\
CREATE INDEX `interface_2` ON `interface` (`ip`,`dns`);\n\
CREATE TABLE `valuemaps` (\n\
`valuemapid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (valuemapid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `valuemaps_1` ON `valuemaps` (`name`);\n\
CREATE TABLE `items` (\n\
`itemid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`snmp_community` varchar(64) DEFAULT '' NOT NULL,\n\
`snmp_oid` varchar(255) DEFAULT '' NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
`key_` varchar(255) DEFAULT '' NOT NULL,\n\
`delay` integer DEFAULT '0' NOT NULL,\n\
`history` integer DEFAULT '90' NOT NULL,\n\
`trends` integer DEFAULT '365' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`value_type` integer DEFAULT '0' NOT NULL,\n\
`trapper_hosts` varchar(255) DEFAULT '' NOT NULL,\n\
`units` varchar(255) DEFAULT '' NOT NULL,\n\
`multiplier` integer DEFAULT '0' NOT NULL,\n\
`delta` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_securityname` varchar(64) DEFAULT '' NOT NULL,\n\
`snmpv3_securitylevel` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_authpassphrase` varchar(64) DEFAULT '' NOT NULL,\n\
`snmpv3_privpassphrase` varchar(64) DEFAULT '' NOT NULL,\n\
`formula` varchar(255) DEFAULT '1' NOT NULL,\n\
`error` varchar(128) DEFAULT '' NOT NULL,\n\
`lastlogsize` bigint unsigned DEFAULT '0' NOT NULL,\n\
`logtimefmt` varchar(64) DEFAULT '' NOT NULL,\n\
`templateid` bigint unsigned  NULL,\n\
`valuemapid` bigint unsigned  NULL,\n\
`delay_flex` varchar(255) DEFAULT '' NOT NULL,\n\
`params` text  NOT NULL,\n\
`ipmi_sensor` varchar(128) DEFAULT '' NOT NULL,\n\
`data_type` integer DEFAULT '0' NOT NULL,\n\
`authtype` integer DEFAULT '0' NOT NULL,\n\
`username` varchar(64) DEFAULT '' NOT NULL,\n\
`password` varchar(64) DEFAULT '' NOT NULL,\n\
`publickey` varchar(64) DEFAULT '' NOT NULL,\n\
`privatekey` varchar(64) DEFAULT '' NOT NULL,\n\
`mtime` integer DEFAULT '0' NOT NULL,\n\
`flags` integer DEFAULT '0' NOT NULL,\n\
`filter` varchar(255) DEFAULT '' NOT NULL,\n\
`interfaceid` bigint unsigned  NULL,\n\
`port` varchar(64) DEFAULT '' NOT NULL,\n\
`description` text  NOT NULL,\n\
`inventory_link` integer DEFAULT '0' NOT NULL,\n\
`lifetime` varchar(64) DEFAULT '30' NOT NULL,\n\
`snmpv3_authprotocol` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_privprotocol` integer DEFAULT '0' NOT NULL,\n\
`state` integer DEFAULT '0' NOT NULL,\n\
`snmpv3_contextname` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (itemid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `items_1` ON `items` (`hostid`,`key_`);\n\
CREATE INDEX `items_3` ON `items` (`status`);\n\
CREATE INDEX `items_4` ON `items` (`templateid`);\n\
CREATE INDEX `items_5` ON `items` (`valuemapid`);\n\
CREATE INDEX `items_6` ON `items` (`interfaceid`);\n\
CREATE TABLE `httpstepitem` (\n\
`httpstepitemid` bigint unsigned  NOT NULL,\n\
`httpstepid` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httpstepitemid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `httpstepitem_1` ON `httpstepitem` (`httpstepid`,`itemid`);\n\
CREATE INDEX `httpstepitem_2` ON `httpstepitem` (`itemid`);\n\
CREATE TABLE `httptestitem` (\n\
`httptestitemid` bigint unsigned  NOT NULL,\n\
`httptestid` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httptestitemid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `httptestitem_1` ON `httptestitem` (`httptestid`,`itemid`);\n\
CREATE INDEX `httptestitem_2` ON `httptestitem` (`itemid`);\n\
CREATE TABLE `media_type` (\n\
`mediatypeid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`description` varchar(100) DEFAULT '' NOT NULL,\n\
`smtp_server` varchar(255) DEFAULT '' NOT NULL,\n\
`smtp_helo` varchar(255) DEFAULT '' NOT NULL,\n\
`smtp_email` varchar(255) DEFAULT '' NOT NULL,\n\
`exec_path` varchar(255) DEFAULT '' NOT NULL,\n\
`gsm_modem` varchar(255) DEFAULT '' NOT NULL,\n\
`username` varchar(255) DEFAULT '' NOT NULL,\n\
`passwd` varchar(255) DEFAULT '' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (mediatypeid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `users` (\n\
`userid` bigint unsigned  NOT NULL,\n\
`alias` varchar(100) DEFAULT '' NOT NULL,\n\
`name` varchar(100) DEFAULT '' NOT NULL,\n\
`surname` varchar(100) DEFAULT '' NOT NULL,\n\
`passwd` char(32) DEFAULT '' NOT NULL,\n\
`url` varchar(255) DEFAULT '' NOT NULL,\n\
`autologin` integer DEFAULT '0' NOT NULL,\n\
`autologout` integer DEFAULT '900' NOT NULL,\n\
`lang` varchar(5) DEFAULT 'en_GB' NOT NULL,\n\
`refresh` integer DEFAULT '30' NOT NULL,\n\
`type` integer DEFAULT '1' NOT NULL,\n\
`theme` varchar(128) DEFAULT 'default' NOT NULL,\n\
`attempt_failed` integer DEFAULT 0 NOT NULL,\n\
`attempt_ip` varchar(39) DEFAULT '' NOT NULL,\n\
`attempt_clock` integer DEFAULT 0 NOT NULL,\n\
`rows_per_page` integer DEFAULT 50 NOT NULL,\n\
PRIMARY KEY (userid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `users_1` ON `users` (`alias`);\n\
CREATE TABLE `usrgrp` (\n\
`usrgrpid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`gui_access` integer DEFAULT '0' NOT NULL,\n\
`users_status` integer DEFAULT '0' NOT NULL,\n\
`debug_mode` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (usrgrpid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `usrgrp_1` ON `usrgrp` (`name`);\n\
CREATE TABLE `users_groups` (\n\
`id` bigint unsigned  NOT NULL,\n\
`usrgrpid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `users_groups_1` ON `users_groups` (`usrgrpid`,`userid`);\n\
CREATE INDEX `users_groups_2` ON `users_groups` (`userid`);\n\
CREATE TABLE `scripts` (\n\
`scriptid` bigint unsigned  NOT NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
`command` varchar(255) DEFAULT '' NOT NULL,\n\
`host_access` integer DEFAULT '2' NOT NULL,\n\
`usrgrpid` bigint unsigned  NULL,\n\
`groupid` bigint unsigned  NULL,\n\
`description` text  NOT NULL,\n\
`confirmation` varchar(255) DEFAULT '' NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`execute_on` integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (scriptid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `scripts_1` ON `scripts` (`usrgrpid`);\n\
CREATE INDEX `scripts_2` ON `scripts` (`groupid`);\n\
CREATE TABLE `actions` (\n\
`actionid` bigint unsigned  NOT NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
`eventsource` integer DEFAULT '0' NOT NULL,\n\
`evaltype` integer DEFAULT '0' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`esc_period` integer DEFAULT '0' NOT NULL,\n\
`def_shortdata` varchar(255) DEFAULT '' NOT NULL,\n\
`def_longdata` text  NOT NULL,\n\
`recovery_msg` integer DEFAULT '0' NOT NULL,\n\
`r_shortdata` varchar(255) DEFAULT '' NOT NULL,\n\
`r_longdata` text  NOT NULL,\n\
PRIMARY KEY (actionid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `actions_1` ON `actions` (`eventsource`,`status`);\n\
CREATE TABLE `operations` (\n\
`operationid` bigint unsigned  NOT NULL,\n\
`actionid` bigint unsigned  NOT NULL,\n\
`operationtype` integer DEFAULT '0' NOT NULL,\n\
`esc_period` integer DEFAULT '0' NOT NULL,\n\
`esc_step_from` integer DEFAULT '1' NOT NULL,\n\
`esc_step_to` integer DEFAULT '1' NOT NULL,\n\
`evaltype` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `operations_1` ON `operations` (`actionid`);\n\
CREATE TABLE `opmessage` (\n\
`operationid` bigint unsigned  NOT NULL,\n\
`default_msg` integer DEFAULT '0' NOT NULL,\n\
`subject` varchar(255) DEFAULT '' NOT NULL,\n\
`message` text  NOT NULL,\n\
`mediatypeid` bigint unsigned  NULL,\n\
PRIMARY KEY (operationid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `opmessage_1` ON `opmessage` (`mediatypeid`);\n\
CREATE TABLE `opmessage_grp` (\n\
`opmessage_grpid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`usrgrpid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (opmessage_grpid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `opmessage_grp_1` ON `opmessage_grp` (`operationid`,`usrgrpid`);\n\
CREATE INDEX `opmessage_grp_2` ON `opmessage_grp` (`usrgrpid`);\n\
CREATE TABLE `opmessage_usr` (\n\
`opmessage_usrid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (opmessage_usrid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `opmessage_usr_1` ON `opmessage_usr` (`operationid`,`userid`);\n\
CREATE INDEX `opmessage_usr_2` ON `opmessage_usr` (`userid`);\n\
CREATE TABLE `opcommand` (\n\
`operationid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`scriptid` bigint unsigned  NULL,\n\
`execute_on` integer DEFAULT '0' NOT NULL,\n\
`port` varchar(64) DEFAULT '' NOT NULL,\n\
`authtype` integer DEFAULT '0' NOT NULL,\n\
`username` varchar(64) DEFAULT '' NOT NULL,\n\
`password` varchar(64) DEFAULT '' NOT NULL,\n\
`publickey` varchar(64) DEFAULT '' NOT NULL,\n\
`privatekey` varchar(64) DEFAULT '' NOT NULL,\n\
`command` text  NOT NULL,\n\
PRIMARY KEY (operationid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `opcommand_1` ON `opcommand` (`scriptid`);\n\
CREATE TABLE `opcommand_hst` (\n\
`opcommand_hstid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NULL,\n\
PRIMARY KEY (opcommand_hstid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `opcommand_hst_1` ON `opcommand_hst` (`operationid`);\n\
CREATE INDEX `opcommand_hst_2` ON `opcommand_hst` (`hostid`);\n\
CREATE TABLE `opcommand_grp` (\n\
`opcommand_grpid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`groupid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (opcommand_grpid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `opcommand_grp_1` ON `opcommand_grp` (`operationid`);\n\
CREATE INDEX `opcommand_grp_2` ON `opcommand_grp` (`groupid`);\n\
CREATE TABLE `opgroup` (\n\
`opgroupid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`groupid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (opgroupid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `opgroup_1` ON `opgroup` (`operationid`,`groupid`);\n\
CREATE INDEX `opgroup_2` ON `opgroup` (`groupid`);\n\
CREATE TABLE `optemplate` (\n\
`optemplateid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`templateid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (optemplateid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `optemplate_1` ON `optemplate` (`operationid`,`templateid`);\n\
CREATE INDEX `optemplate_2` ON `optemplate` (`templateid`);\n\
CREATE TABLE `opconditions` (\n\
`opconditionid` bigint unsigned  NOT NULL,\n\
`operationid` bigint unsigned  NOT NULL,\n\
`conditiontype` integer DEFAULT '0' NOT NULL,\n\
`operator` integer DEFAULT '0' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (opconditionid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `opconditions_1` ON `opconditions` (`operationid`);\n\
CREATE TABLE `conditions` (\n\
`conditionid` bigint unsigned  NOT NULL,\n\
`actionid` bigint unsigned  NOT NULL,\n\
`conditiontype` integer DEFAULT '0' NOT NULL,\n\
`operator` integer DEFAULT '0' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (conditionid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `conditions_1` ON `conditions` (`actionid`);\n\
CREATE TABLE `config` (\n\
`configid` bigint unsigned  NOT NULL,\n\
`refresh_unsupported` integer DEFAULT '0' NOT NULL,\n\
`work_period` varchar(100) DEFAULT '1-5,00:00-24:00' NOT NULL,\n\
`alert_usrgrpid` bigint unsigned  NULL,\n\
`event_ack_enable` integer DEFAULT '1' NOT NULL,\n\
`event_expire` integer DEFAULT '7' NOT NULL,\n\
`event_show_max` integer DEFAULT '100' NOT NULL,\n\
`default_theme` varchar(128) DEFAULT 'originalblue' NOT NULL,\n\
`authentication_type` integer DEFAULT '0' NOT NULL,\n\
`ldap_host` varchar(255) DEFAULT '' NOT NULL,\n\
`ldap_port` integer DEFAULT 389 NOT NULL,\n\
`ldap_base_dn` varchar(255) DEFAULT '' NOT NULL,\n\
`ldap_bind_dn` varchar(255) DEFAULT '' NOT NULL,\n\
`ldap_bind_password` varchar(128) DEFAULT '' NOT NULL,\n\
`ldap_search_attribute` varchar(128) DEFAULT '' NOT NULL,\n\
`dropdown_first_entry` integer DEFAULT '1' NOT NULL,\n\
`dropdown_first_remember` integer DEFAULT '1' NOT NULL,\n\
`discovery_groupid` bigint unsigned  NOT NULL,\n\
`max_in_table` integer DEFAULT '50' NOT NULL,\n\
`search_limit` integer DEFAULT '1000' NOT NULL,\n\
`severity_color_0` varchar(6) DEFAULT 'DBDBDB' NOT NULL,\n\
`severity_color_1` varchar(6) DEFAULT 'D6F6FF' NOT NULL,\n\
`severity_color_2` varchar(6) DEFAULT 'FFF6A5' NOT NULL,\n\
`severity_color_3` varchar(6) DEFAULT 'FFB689' NOT NULL,\n\
`severity_color_4` varchar(6) DEFAULT 'FF9999' NOT NULL,\n\
`severity_color_5` varchar(6) DEFAULT 'FF3838' NOT NULL,\n\
`severity_name_0` varchar(32) DEFAULT 'Not classified' NOT NULL,\n\
`severity_name_1` varchar(32) DEFAULT 'Information' NOT NULL,\n\
`severity_name_2` varchar(32) DEFAULT 'Warning' NOT NULL,\n\
`severity_name_3` varchar(32) DEFAULT 'Average' NOT NULL,\n\
`severity_name_4` varchar(32) DEFAULT 'High' NOT NULL,\n\
`severity_name_5` varchar(32) DEFAULT 'Disaster' NOT NULL,\n\
`ok_period` integer DEFAULT '1800' NOT NULL,\n\
`blink_period` integer DEFAULT '1800' NOT NULL,\n\
`problem_unack_color` varchar(6) DEFAULT 'DC0000' NOT NULL,\n\
`problem_ack_color` varchar(6) DEFAULT 'DC0000' NOT NULL,\n\
`ok_unack_color` varchar(6) DEFAULT '00AA00' NOT NULL,\n\
`ok_ack_color` varchar(6) DEFAULT '00AA00' NOT NULL,\n\
`problem_unack_style` integer DEFAULT '1' NOT NULL,\n\
`problem_ack_style` integer DEFAULT '1' NOT NULL,\n\
`ok_unack_style` integer DEFAULT '1' NOT NULL,\n\
`ok_ack_style` integer DEFAULT '1' NOT NULL,\n\
`snmptrap_logging` integer DEFAULT '1' NOT NULL,\n\
`server_check_interval` integer DEFAULT '10' NOT NULL,\n\
`hk_events_mode` integer DEFAULT '1' NOT NULL,\n\
`hk_events_trigger` integer DEFAULT '365' NOT NULL,\n\
`hk_events_internal` integer DEFAULT '365' NOT NULL,\n\
`hk_events_discovery` integer DEFAULT '365' NOT NULL,\n\
`hk_events_autoreg` integer DEFAULT '365' NOT NULL,\n\
`hk_services_mode` integer DEFAULT '1' NOT NULL,\n\
`hk_services` integer DEFAULT '365' NOT NULL,\n\
`hk_audit_mode` integer DEFAULT '1' NOT NULL,\n\
`hk_audit` integer DEFAULT '365' NOT NULL,\n\
`hk_sessions_mode` integer DEFAULT '1' NOT NULL,\n\
`hk_sessions` integer DEFAULT '365' NOT NULL,\n\
`hk_history_mode` integer DEFAULT '1' NOT NULL,\n\
`hk_history_global` integer DEFAULT '0' NOT NULL,\n\
`hk_history` integer DEFAULT '90' NOT NULL,\n\
`hk_trends_mode` integer DEFAULT '1' NOT NULL,\n\
`hk_trends_global` integer DEFAULT '0' NOT NULL,\n\
`hk_trends` integer DEFAULT '365' NOT NULL,\n\
PRIMARY KEY (configid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `config_1` ON `config` (`alert_usrgrpid`);\n\
CREATE INDEX `config_2` ON `config` (`discovery_groupid`);\n\
CREATE TABLE `triggers` (\n\
`triggerid` bigint unsigned  NOT NULL,\n\
`expression` varchar(2048) DEFAULT '' NOT NULL,\n\
`description` varchar(255) DEFAULT '' NOT NULL,\n\
`url` varchar(255) DEFAULT '' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`value` integer DEFAULT '0' NOT NULL,\n\
`priority` integer DEFAULT '0' NOT NULL,\n\
`lastchange` integer DEFAULT '0' NOT NULL,\n\
`comments` text  NOT NULL,\n\
`error` varchar(128) DEFAULT '' NOT NULL,\n\
`templateid` bigint unsigned  NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`state` integer DEFAULT '0' NOT NULL,\n\
`flags` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (triggerid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `triggers_1` ON `triggers` (`status`);\n\
CREATE INDEX `triggers_2` ON `triggers` (`value`);\n\
CREATE INDEX `triggers_3` ON `triggers` (`templateid`);\n\
CREATE TABLE `trigger_depends` (\n\
`triggerdepid` bigint unsigned  NOT NULL,\n\
`triggerid_down` bigint unsigned  NOT NULL,\n\
`triggerid_up` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (triggerdepid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `trigger_depends_1` ON `trigger_depends` (`triggerid_down`,`triggerid_up`);\n\
CREATE INDEX `trigger_depends_2` ON `trigger_depends` (`triggerid_up`);\n\
CREATE TABLE `functions` (\n\
`functionid` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`triggerid` bigint unsigned  NOT NULL,\n\
`function` varchar(12) DEFAULT '' NOT NULL,\n\
`parameter` varchar(255) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (functionid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `functions_1` ON `functions` (`triggerid`);\n\
CREATE INDEX `functions_2` ON `functions` (`itemid`,`function`,`parameter`);\n\
CREATE TABLE `graphs` (\n\
`graphid` bigint unsigned  NOT NULL,\n\
`name` varchar(128) DEFAULT '' NOT NULL,\n\
`width` integer DEFAULT '900' NOT NULL,\n\
`height` integer DEFAULT '200' NOT NULL,\n\
`yaxismin` double(16,4) DEFAULT '0' NOT NULL,\n\
`yaxismax` double(16,4) DEFAULT '100' NOT NULL,\n\
`templateid` bigint unsigned  NULL,\n\
`show_work_period` integer DEFAULT '1' NOT NULL,\n\
`show_triggers` integer DEFAULT '1' NOT NULL,\n\
`graphtype` integer DEFAULT '0' NOT NULL,\n\
`show_legend` integer DEFAULT '1' NOT NULL,\n\
`show_3d` integer DEFAULT '0' NOT NULL,\n\
`percent_left` double(16,4) DEFAULT '0' NOT NULL,\n\
`percent_right` double(16,4) DEFAULT '0' NOT NULL,\n\
`ymin_type` integer DEFAULT '0' NOT NULL,\n\
`ymax_type` integer DEFAULT '0' NOT NULL,\n\
`ymin_itemid` bigint unsigned  NULL,\n\
`ymax_itemid` bigint unsigned  NULL,\n\
`flags` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (graphid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `graphs_1` ON `graphs` (`name`);\n\
CREATE INDEX `graphs_2` ON `graphs` (`templateid`);\n\
CREATE INDEX `graphs_3` ON `graphs` (`ymin_itemid`);\n\
CREATE INDEX `graphs_4` ON `graphs` (`ymax_itemid`);\n\
CREATE TABLE `graphs_items` (\n\
`gitemid` bigint unsigned  NOT NULL,\n\
`graphid` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`drawtype` integer DEFAULT '0' NOT NULL,\n\
`sortorder` integer DEFAULT '0' NOT NULL,\n\
`color` varchar(6) DEFAULT '009600' NOT NULL,\n\
`yaxisside` integer DEFAULT '0' NOT NULL,\n\
`calc_fnc` integer DEFAULT '2' NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (gitemid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `graphs_items_1` ON `graphs_items` (`itemid`);\n\
CREATE INDEX `graphs_items_2` ON `graphs_items` (`graphid`);\n\
CREATE TABLE `graph_theme` (\n\
`graphthemeid` bigint unsigned  NOT NULL,\n\
`description` varchar(64) DEFAULT '' NOT NULL,\n\
`theme` varchar(64) DEFAULT '' NOT NULL,\n\
`backgroundcolor` varchar(6) DEFAULT 'F0F0F0' NOT NULL,\n\
`graphcolor` varchar(6) DEFAULT 'FFFFFF' NOT NULL,\n\
`graphbordercolor` varchar(6) DEFAULT '222222' NOT NULL,\n\
`gridcolor` varchar(6) DEFAULT 'CCCCCC' NOT NULL,\n\
`maingridcolor` varchar(6) DEFAULT 'AAAAAA' NOT NULL,\n\
`gridbordercolor` varchar(6) DEFAULT '000000' NOT NULL,\n\
`textcolor` varchar(6) DEFAULT '202020' NOT NULL,\n\
`highlightcolor` varchar(6) DEFAULT 'AA4444' NOT NULL,\n\
`leftpercentilecolor` varchar(6) DEFAULT '11CC11' NOT NULL,\n\
`rightpercentilecolor` varchar(6) DEFAULT 'CC1111' NOT NULL,\n\
`nonworktimecolor` varchar(6) DEFAULT 'CCCCCC' NOT NULL,\n\
`gridview` integer DEFAULT 1 NOT NULL,\n\
`legendview` integer DEFAULT 1 NOT NULL,\n\
PRIMARY KEY (graphthemeid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `graph_theme_1` ON `graph_theme` (`description`);\n\
CREATE INDEX `graph_theme_2` ON `graph_theme` (`theme`);\n\
CREATE TABLE `globalmacro` (\n\
`globalmacroid` bigint unsigned  NOT NULL,\n\
`macro` varchar(64) DEFAULT '' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (globalmacroid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `globalmacro_1` ON `globalmacro` (`macro`);\n\
CREATE TABLE `hostmacro` (\n\
`hostmacroid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`macro` varchar(64) DEFAULT '' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostmacroid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `hostmacro_1` ON `hostmacro` (`hostid`,`macro`);\n\
CREATE TABLE `hosts_groups` (\n\
`hostgroupid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`groupid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (hostgroupid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `hosts_groups_1` ON `hosts_groups` (`hostid`,`groupid`);\n\
CREATE INDEX `hosts_groups_2` ON `hosts_groups` (`groupid`);\n\
CREATE TABLE `hosts_templates` (\n\
`hosttemplateid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
`templateid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (hosttemplateid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `hosts_templates_1` ON `hosts_templates` (`hostid`,`templateid`);\n\
CREATE INDEX `hosts_templates_2` ON `hosts_templates` (`templateid`);\n\
CREATE TABLE `items_applications` (\n\
`itemappid` bigint unsigned  NOT NULL,\n\
`applicationid` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (itemappid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `items_applications_1` ON `items_applications` (`applicationid`,`itemid`);\n\
CREATE INDEX `items_applications_2` ON `items_applications` (`itemid`);\n\
CREATE TABLE `mappings` (\n\
`mappingid` bigint unsigned  NOT NULL,\n\
`valuemapid` bigint unsigned  NOT NULL,\n\
`value` varchar(64) DEFAULT '' NOT NULL,\n\
`newvalue` varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (mappingid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `mappings_1` ON `mappings` (`valuemapid`);\n\
CREATE TABLE `media` (\n\
`mediaid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
`mediatypeid` bigint unsigned  NOT NULL,\n\
`sendto` varchar(100) DEFAULT '' NOT NULL,\n\
`active` integer DEFAULT '0' NOT NULL,\n\
`severity` integer DEFAULT '63' NOT NULL,\n\
`period` varchar(100) DEFAULT '1-7,00:00-24:00' NOT NULL,\n\
PRIMARY KEY (mediaid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `media_1` ON `media` (`userid`);\n\
CREATE INDEX `media_2` ON `media` (`mediatypeid`);\n\
CREATE TABLE `rights` (\n\
`rightid` bigint unsigned  NOT NULL,\n\
`groupid` bigint unsigned  NOT NULL,\n\
`permission` integer DEFAULT '0' NOT NULL,\n\
`id` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (rightid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `rights_1` ON `rights` (`groupid`);\n\
CREATE INDEX `rights_2` ON `rights` (`id`);\n\
CREATE TABLE `services` (\n\
`serviceid` bigint unsigned  NOT NULL,\n\
`name` varchar(128) DEFAULT '' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`algorithm` integer DEFAULT '0' NOT NULL,\n\
`triggerid` bigint unsigned  NULL,\n\
`showsla` integer DEFAULT '0' NOT NULL,\n\
`goodsla` double(16,4) DEFAULT '99.9' NOT NULL,\n\
`sortorder` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (serviceid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `services_1` ON `services` (`triggerid`);\n\
CREATE TABLE `services_links` (\n\
`linkid` bigint unsigned  NOT NULL,\n\
`serviceupid` bigint unsigned  NOT NULL,\n\
`servicedownid` bigint unsigned  NOT NULL,\n\
`soft` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `services_links_1` ON `services_links` (`servicedownid`);\n\
CREATE UNIQUE INDEX `services_links_2` ON `services_links` (`serviceupid`,`servicedownid`);\n\
CREATE TABLE `services_times` (\n\
`timeid` bigint unsigned  NOT NULL,\n\
`serviceid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`ts_from` integer DEFAULT '0' NOT NULL,\n\
`ts_to` integer DEFAULT '0' NOT NULL,\n\
`note` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (timeid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `services_times_1` ON `services_times` (`serviceid`,`type`,`ts_from`,`ts_to`);\n\
CREATE TABLE `icon_map` (\n\
`iconmapid` bigint unsigned  NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`default_iconid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (iconmapid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `icon_map_1` ON `icon_map` (`name`);\n\
CREATE INDEX `icon_map_2` ON `icon_map` (`default_iconid`);\n\
CREATE TABLE `icon_mapping` (\n\
`iconmappingid` bigint unsigned  NOT NULL,\n\
`iconmapid` bigint unsigned  NOT NULL,\n\
`iconid` bigint unsigned  NOT NULL,\n\
`inventory_link` integer DEFAULT '0' NOT NULL,\n\
`expression` varchar(64) DEFAULT '' NOT NULL,\n\
`sortorder` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (iconmappingid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `icon_mapping_1` ON `icon_mapping` (`iconmapid`);\n\
CREATE INDEX `icon_mapping_2` ON `icon_mapping` (`iconid`);\n\
CREATE TABLE `sysmaps` (\n\
`sysmapid` bigint unsigned  NOT NULL,\n\
`name` varchar(128) DEFAULT '' NOT NULL,\n\
`width` integer DEFAULT '600' NOT NULL,\n\
`height` integer DEFAULT '400' NOT NULL,\n\
`backgroundid` bigint unsigned  NULL,\n\
`label_type` integer DEFAULT '2' NOT NULL,\n\
`label_location` integer DEFAULT '0' NOT NULL,\n\
`highlight` integer DEFAULT '1' NOT NULL,\n\
`expandproblem` integer DEFAULT '1' NOT NULL,\n\
`markelements` integer DEFAULT '0' NOT NULL,\n\
`show_unack` integer DEFAULT '0' NOT NULL,\n\
`grid_size` integer DEFAULT '50' NOT NULL,\n\
`grid_show` integer DEFAULT '1' NOT NULL,\n\
`grid_align` integer DEFAULT '1' NOT NULL,\n\
`label_format` integer DEFAULT '0' NOT NULL,\n\
`label_type_host` integer DEFAULT '2' NOT NULL,\n\
`label_type_hostgroup` integer DEFAULT '2' NOT NULL,\n\
`label_type_trigger` integer DEFAULT '2' NOT NULL,\n\
`label_type_map` integer DEFAULT '2' NOT NULL,\n\
`label_type_image` integer DEFAULT '2' NOT NULL,\n\
`label_string_host` varchar(255) DEFAULT '' NOT NULL,\n\
`label_string_hostgroup` varchar(255) DEFAULT '' NOT NULL,\n\
`label_string_trigger` varchar(255) DEFAULT '' NOT NULL,\n\
`label_string_map` varchar(255) DEFAULT '' NOT NULL,\n\
`label_string_image` varchar(255) DEFAULT '' NOT NULL,\n\
`iconmapid` bigint unsigned  NULL,\n\
`expand_macros` integer DEFAULT '0' NOT NULL,\n\
`severity_min` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `sysmaps_1` ON `sysmaps` (`name`);\n\
CREATE INDEX `sysmaps_2` ON `sysmaps` (`backgroundid`);\n\
CREATE INDEX `sysmaps_3` ON `sysmaps` (`iconmapid`);\n\
CREATE TABLE `sysmaps_elements` (\n\
`selementid` bigint unsigned  NOT NULL,\n\
`sysmapid` bigint unsigned  NOT NULL,\n\
`elementid` bigint unsigned DEFAULT '0' NOT NULL,\n\
`elementtype` integer DEFAULT '0' NOT NULL,\n\
`iconid_off` bigint unsigned  NULL,\n\
`iconid_on` bigint unsigned  NULL,\n\
`label` varchar(2048) DEFAULT '' NOT NULL,\n\
`label_location` integer DEFAULT '-1' NOT NULL,\n\
`x` integer DEFAULT '0' NOT NULL,\n\
`y` integer DEFAULT '0' NOT NULL,\n\
`iconid_disabled` bigint unsigned  NULL,\n\
`iconid_maintenance` bigint unsigned  NULL,\n\
`elementsubtype` integer DEFAULT '0' NOT NULL,\n\
`areatype` integer DEFAULT '0' NOT NULL,\n\
`width` integer DEFAULT '200' NOT NULL,\n\
`height` integer DEFAULT '200' NOT NULL,\n\
`viewtype` integer DEFAULT '0' NOT NULL,\n\
`use_iconmap` integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (selementid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `sysmaps_elements_1` ON `sysmaps_elements` (`sysmapid`);\n\
CREATE INDEX `sysmaps_elements_2` ON `sysmaps_elements` (`iconid_off`);\n\
CREATE INDEX `sysmaps_elements_3` ON `sysmaps_elements` (`iconid_on`);\n\
CREATE INDEX `sysmaps_elements_4` ON `sysmaps_elements` (`iconid_disabled`);\n\
CREATE INDEX `sysmaps_elements_5` ON `sysmaps_elements` (`iconid_maintenance`);\n\
CREATE TABLE `sysmaps_links` (\n\
`linkid` bigint unsigned  NOT NULL,\n\
`sysmapid` bigint unsigned  NOT NULL,\n\
`selementid1` bigint unsigned  NOT NULL,\n\
`selementid2` bigint unsigned  NOT NULL,\n\
`drawtype` integer DEFAULT '0' NOT NULL,\n\
`color` varchar(6) DEFAULT '000000' NOT NULL,\n\
`label` varchar(2048) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `sysmaps_links_1` ON `sysmaps_links` (`sysmapid`);\n\
CREATE INDEX `sysmaps_links_2` ON `sysmaps_links` (`selementid1`);\n\
CREATE INDEX `sysmaps_links_3` ON `sysmaps_links` (`selementid2`);\n\
CREATE TABLE `sysmaps_link_triggers` (\n\
`linktriggerid` bigint unsigned  NOT NULL,\n\
`linkid` bigint unsigned  NOT NULL,\n\
`triggerid` bigint unsigned  NOT NULL,\n\
`drawtype` integer DEFAULT '0' NOT NULL,\n\
`color` varchar(6) DEFAULT '000000' NOT NULL,\n\
PRIMARY KEY (linktriggerid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `sysmaps_link_triggers_1` ON `sysmaps_link_triggers` (`linkid`,`triggerid`);\n\
CREATE INDEX `sysmaps_link_triggers_2` ON `sysmaps_link_triggers` (`triggerid`);\n\
CREATE TABLE `sysmap_element_url` (\n\
`sysmapelementurlid` bigint unsigned  NOT NULL,\n\
`selementid` bigint unsigned  NOT NULL,\n\
`name` varchar(255)  NOT NULL,\n\
`url` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (sysmapelementurlid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `sysmap_element_url_1` ON `sysmap_element_url` (`selementid`,`name`);\n\
CREATE TABLE `sysmap_url` (\n\
`sysmapurlid` bigint unsigned  NOT NULL,\n\
`sysmapid` bigint unsigned  NOT NULL,\n\
`name` varchar(255)  NOT NULL,\n\
`url` varchar(255) DEFAULT '' NOT NULL,\n\
`elementtype` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapurlid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `sysmap_url_1` ON `sysmap_url` (`sysmapid`,`name`);\n\
CREATE TABLE `maintenances_hosts` (\n\
`maintenance_hostid` bigint unsigned  NOT NULL,\n\
`maintenanceid` bigint unsigned  NOT NULL,\n\
`hostid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (maintenance_hostid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `maintenances_hosts_1` ON `maintenances_hosts` (`maintenanceid`,`hostid`);\n\
CREATE INDEX `maintenances_hosts_2` ON `maintenances_hosts` (`hostid`);\n\
CREATE TABLE `maintenances_groups` (\n\
`maintenance_groupid` bigint unsigned  NOT NULL,\n\
`maintenanceid` bigint unsigned  NOT NULL,\n\
`groupid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (maintenance_groupid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `maintenances_groups_1` ON `maintenances_groups` (`maintenanceid`,`groupid`);\n\
CREATE INDEX `maintenances_groups_2` ON `maintenances_groups` (`groupid`);\n\
CREATE TABLE `timeperiods` (\n\
`timeperiodid` bigint unsigned  NOT NULL,\n\
`timeperiod_type` integer DEFAULT '0' NOT NULL,\n\
`every` integer DEFAULT '0' NOT NULL,\n\
`month` integer DEFAULT '0' NOT NULL,\n\
`dayofweek` integer DEFAULT '0' NOT NULL,\n\
`day` integer DEFAULT '0' NOT NULL,\n\
`start_time` integer DEFAULT '0' NOT NULL,\n\
`period` integer DEFAULT '0' NOT NULL,\n\
`start_date` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (timeperiodid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `maintenances_windows` (\n\
`maintenance_timeperiodid` bigint unsigned  NOT NULL,\n\
`maintenanceid` bigint unsigned  NOT NULL,\n\
`timeperiodid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (maintenance_timeperiodid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `maintenances_windows_1` ON `maintenances_windows` (`maintenanceid`,`timeperiodid`);\n\
CREATE INDEX `maintenances_windows_2` ON `maintenances_windows` (`timeperiodid`);\n\
CREATE TABLE `regexps` (\n\
`regexpid` bigint unsigned  NOT NULL,\n\
`name` varchar(128) DEFAULT '' NOT NULL,\n\
`test_string` text  NOT NULL,\n\
PRIMARY KEY (regexpid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `regexps_1` ON `regexps` (`name`);\n\
CREATE TABLE `expressions` (\n\
`expressionid` bigint unsigned  NOT NULL,\n\
`regexpid` bigint unsigned  NOT NULL,\n\
`expression` varchar(255) DEFAULT '' NOT NULL,\n\
`expression_type` integer DEFAULT '0' NOT NULL,\n\
`exp_delimiter` varchar(1) DEFAULT '' NOT NULL,\n\
`case_sensitive` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (expressionid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `expressions_1` ON `expressions` (`regexpid`);\n\
CREATE TABLE `nodes` (\n\
`nodeid` integer  NOT NULL,\n\
`name` varchar(64) DEFAULT '0' NOT NULL,\n\
`ip` varchar(39) DEFAULT '' NOT NULL,\n\
`port` integer DEFAULT '10051' NOT NULL,\n\
`nodetype` integer DEFAULT '0' NOT NULL,\n\
`masterid` integer  NULL,\n\
PRIMARY KEY (nodeid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `nodes_1` ON `nodes` (`masterid`);\n\
CREATE TABLE `node_cksum` (\n\
`nodeid` integer  NOT NULL,\n\
`tablename` varchar(64) DEFAULT '' NOT NULL,\n\
`recordid` bigint unsigned  NOT NULL,\n\
`cksumtype` integer DEFAULT '0' NOT NULL,\n\
`cksum` text  NOT NULL,\n\
`sync` char(128) DEFAULT '' NOT NULL\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `node_cksum_1` ON `node_cksum` (`nodeid`,`cksumtype`,`tablename`,`recordid`);\n\
CREATE TABLE `ids` (\n\
`nodeid` integer  NOT NULL,\n\
`table_name` varchar(64) DEFAULT '' NOT NULL,\n\
`field_name` varchar(64) DEFAULT '' NOT NULL,\n\
`nextid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (nodeid,table_name,field_name)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `alerts` (\n\
`alertid` bigint unsigned  NOT NULL,\n\
`actionid` bigint unsigned  NOT NULL,\n\
`eventid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`mediatypeid` bigint unsigned  NULL,\n\
`sendto` varchar(100) DEFAULT '' NOT NULL,\n\
`subject` varchar(255) DEFAULT '' NOT NULL,\n\
`message` text  NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`retries` integer DEFAULT '0' NOT NULL,\n\
`error` varchar(128) DEFAULT '' NOT NULL,\n\
`esc_step` integer DEFAULT '0' NOT NULL,\n\
`alerttype` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (alertid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `alerts_1` ON `alerts` (`actionid`);\n\
CREATE INDEX `alerts_2` ON `alerts` (`clock`);\n\
CREATE INDEX `alerts_3` ON `alerts` (`eventid`);\n\
CREATE INDEX `alerts_4` ON `alerts` (`status`,`retries`);\n\
CREATE INDEX `alerts_5` ON `alerts` (`mediatypeid`);\n\
CREATE INDEX `alerts_6` ON `alerts` (`userid`);\n\
CREATE TABLE `history` (\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` double(16,4) DEFAULT '0.0000' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_1` ON `history` (`itemid`,`clock`);\n\
CREATE TABLE `history_sync` (\n\
`id` bigint unsigned  NOT NULL auto_increment,\n\
`nodeid` integer  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` double(16,4) DEFAULT '0.0000' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_sync_1` ON `history_sync` (`nodeid`,`id`);\n\
CREATE TABLE `history_uint` (\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` bigint unsigned DEFAULT '0' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_uint_1` ON `history_uint` (`itemid`,`clock`);\n\
CREATE TABLE `history_uint_sync` (\n\
`id` bigint unsigned  NOT NULL auto_increment,\n\
`nodeid` integer  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` bigint unsigned DEFAULT '0' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_uint_sync_1` ON `history_uint_sync` (`nodeid`,`id`);\n\
CREATE TABLE `history_str` (\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_str_1` ON `history_str` (`itemid`,`clock`);\n\
CREATE TABLE `history_str_sync` (\n\
`id` bigint unsigned  NOT NULL auto_increment,\n\
`nodeid` integer  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_str_sync_1` ON `history_str_sync` (`nodeid`,`id`);\n\
CREATE TABLE `history_log` (\n\
`id` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`timestamp` integer DEFAULT '0' NOT NULL,\n\
`source` varchar(64) DEFAULT '' NOT NULL,\n\
`severity` integer DEFAULT '0' NOT NULL,\n\
`value` text  NOT NULL,\n\
`logeventid` integer DEFAULT '0' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_log_1` ON `history_log` (`itemid`,`clock`);\n\
CREATE UNIQUE INDEX `history_log_2` ON `history_log` (`itemid`,`id`);\n\
CREATE TABLE `history_text` (\n\
`id` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` text  NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `history_text_1` ON `history_text` (`itemid`,`clock`);\n\
CREATE UNIQUE INDEX `history_text_2` ON `history_text` (`itemid`,`id`);\n\
CREATE TABLE `proxy_history` (\n\
`id` bigint unsigned  NOT NULL auto_increment,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`timestamp` integer DEFAULT '0' NOT NULL,\n\
`source` varchar(64) DEFAULT '' NOT NULL,\n\
`severity` integer DEFAULT '0' NOT NULL,\n\
`value` longtext  NOT NULL,\n\
`logeventid` integer DEFAULT '0' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
`state` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `proxy_history_1` ON `proxy_history` (`clock`);\n\
CREATE TABLE `proxy_dhistory` (\n\
`id` bigint unsigned  NOT NULL auto_increment,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`druleid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`ip` varchar(39) DEFAULT '' NOT NULL,\n\
`port` integer DEFAULT '0' NOT NULL,\n\
`key_` varchar(255) DEFAULT '' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`dcheckid` bigint unsigned  NULL,\n\
`dns` varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `proxy_dhistory_1` ON `proxy_dhistory` (`clock`);\n\
CREATE TABLE `events` (\n\
`eventid` bigint unsigned  NOT NULL,\n\
`source` integer DEFAULT '0' NOT NULL,\n\
`object` integer DEFAULT '0' NOT NULL,\n\
`objectid` bigint unsigned DEFAULT '0' NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` integer DEFAULT '0' NOT NULL,\n\
`acknowledged` integer DEFAULT '0' NOT NULL,\n\
`ns` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (eventid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `events_1` ON `events` (`source`,`object`,`objectid`,`clock`);\n\
CREATE INDEX `events_2` ON `events` (`source`,`object`,`clock`);\n\
CREATE TABLE `trends` (\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`num` integer DEFAULT '0' NOT NULL,\n\
`value_min` double(16,4) DEFAULT '0.0000' NOT NULL,\n\
`value_avg` double(16,4) DEFAULT '0.0000' NOT NULL,\n\
`value_max` double(16,4) DEFAULT '0.0000' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `trends_uint` (\n\
`itemid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`num` integer DEFAULT '0' NOT NULL,\n\
`value_min` bigint unsigned DEFAULT '0' NOT NULL,\n\
`value_avg` bigint unsigned DEFAULT '0' NOT NULL,\n\
`value_max` bigint unsigned DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `acknowledges` (\n\
`acknowledgeid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
`eventid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`message` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (acknowledgeid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `acknowledges_1` ON `acknowledges` (`userid`);\n\
CREATE INDEX `acknowledges_2` ON `acknowledges` (`eventid`);\n\
CREATE INDEX `acknowledges_3` ON `acknowledges` (`clock`);\n\
CREATE TABLE `auditlog` (\n\
`auditid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`action` integer DEFAULT '0' NOT NULL,\n\
`resourcetype` integer DEFAULT '0' NOT NULL,\n\
`details` varchar(128)  DEFAULT '0' NOT NULL,\n\
`ip` varchar(39) DEFAULT '' NOT NULL,\n\
`resourceid` bigint unsigned DEFAULT '0' NOT NULL,\n\
`resourcename` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `auditlog_1` ON `auditlog` (`userid`,`clock`);\n\
CREATE INDEX `auditlog_2` ON `auditlog` (`clock`);\n\
CREATE TABLE `auditlog_details` (\n\
`auditdetailid` bigint unsigned  NOT NULL,\n\
`auditid` bigint unsigned  NOT NULL,\n\
`table_name` varchar(64) DEFAULT '' NOT NULL,\n\
`field_name` varchar(64) DEFAULT '' NOT NULL,\n\
`oldvalue` text  NOT NULL,\n\
`newvalue` text  NOT NULL,\n\
PRIMARY KEY (auditdetailid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `auditlog_details_1` ON `auditlog_details` (`auditid`);\n\
CREATE TABLE `service_alarms` (\n\
`servicealarmid` bigint unsigned  NOT NULL,\n\
`serviceid` bigint unsigned  NOT NULL,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`value` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (servicealarmid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `service_alarms_1` ON `service_alarms` (`serviceid`,`clock`);\n\
CREATE INDEX `service_alarms_2` ON `service_alarms` (`clock`);\n\
CREATE TABLE `autoreg_host` (\n\
`autoreg_hostid` bigint unsigned  NOT NULL,\n\
`proxy_hostid` bigint unsigned  NULL,\n\
`host` varchar(64) DEFAULT '' NOT NULL,\n\
`listen_ip` varchar(39) DEFAULT '' NOT NULL,\n\
`listen_port` integer DEFAULT '0' NOT NULL,\n\
`listen_dns` varchar(64) DEFAULT '' NOT NULL,\n\
`host_metadata` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (autoreg_hostid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `autoreg_host_1` ON `autoreg_host` (`proxy_hostid`,`host`);\n\
CREATE TABLE `proxy_autoreg_host` (\n\
`id` bigint unsigned  NOT NULL auto_increment,\n\
`clock` integer DEFAULT '0' NOT NULL,\n\
`host` varchar(64) DEFAULT '' NOT NULL,\n\
`listen_ip` varchar(39) DEFAULT '' NOT NULL,\n\
`listen_port` integer DEFAULT '0' NOT NULL,\n\
`listen_dns` varchar(64) DEFAULT '' NOT NULL,\n\
`host_metadata` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (id)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `proxy_autoreg_host_1` ON `proxy_autoreg_host` (`clock`);\n\
CREATE TABLE `dhosts` (\n\
`dhostid` bigint unsigned  NOT NULL,\n\
`druleid` bigint unsigned  NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`lastup` integer DEFAULT '0' NOT NULL,\n\
`lastdown` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (dhostid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `dhosts_1` ON `dhosts` (`druleid`);\n\
CREATE TABLE `dservices` (\n\
`dserviceid` bigint unsigned  NOT NULL,\n\
`dhostid` bigint unsigned  NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
`key_` varchar(255) DEFAULT '' NOT NULL,\n\
`value` varchar(255) DEFAULT '' NOT NULL,\n\
`port` integer DEFAULT '0' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`lastup` integer DEFAULT '0' NOT NULL,\n\
`lastdown` integer DEFAULT '0' NOT NULL,\n\
`dcheckid` bigint unsigned  NOT NULL,\n\
`ip` varchar(39) DEFAULT '' NOT NULL,\n\
`dns` varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dserviceid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `dservices_1` ON `dservices` (`dcheckid`,`type`,`key_`,`ip`,`port`);\n\
CREATE INDEX `dservices_2` ON `dservices` (`dhostid`);\n\
CREATE TABLE `escalations` (\n\
`escalationid` bigint unsigned  NOT NULL,\n\
`actionid` bigint unsigned  NOT NULL,\n\
`triggerid` bigint unsigned  NULL,\n\
`eventid` bigint unsigned  NULL,\n\
`r_eventid` bigint unsigned  NULL,\n\
`nextcheck` integer DEFAULT '0' NOT NULL,\n\
`esc_step` integer DEFAULT '0' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
`itemid` bigint unsigned  NULL,\n\
PRIMARY KEY (escalationid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `escalations_1` ON `escalations` (`actionid`,`triggerid`,`itemid`,`escalationid`);\n\
CREATE TABLE `globalvars` (\n\
`globalvarid` bigint unsigned  NOT NULL,\n\
`snmp_lastsize` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (globalvarid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `graph_discovery` (\n\
`graphdiscoveryid` bigint unsigned  NOT NULL,\n\
`graphid` bigint unsigned  NOT NULL,\n\
`parent_graphid` bigint unsigned  NOT NULL,\n\
`name` varchar(128) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (graphdiscoveryid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `graph_discovery_1` ON `graph_discovery` (`graphid`,`parent_graphid`);\n\
CREATE INDEX `graph_discovery_2` ON `graph_discovery` (`parent_graphid`);\n\
CREATE TABLE `host_inventory` (\n\
`hostid` bigint unsigned  NOT NULL,\n\
`inventory_mode` integer DEFAULT '0' NOT NULL,\n\
`type` varchar(64) DEFAULT '' NOT NULL,\n\
`type_full` varchar(64) DEFAULT '' NOT NULL,\n\
`name` varchar(64) DEFAULT '' NOT NULL,\n\
`alias` varchar(64) DEFAULT '' NOT NULL,\n\
`os` varchar(64) DEFAULT '' NOT NULL,\n\
`os_full` varchar(255) DEFAULT '' NOT NULL,\n\
`os_short` varchar(64) DEFAULT '' NOT NULL,\n\
`serialno_a` varchar(64) DEFAULT '' NOT NULL,\n\
`serialno_b` varchar(64) DEFAULT '' NOT NULL,\n\
`tag` varchar(64) DEFAULT '' NOT NULL,\n\
`asset_tag` varchar(64) DEFAULT '' NOT NULL,\n\
`macaddress_a` varchar(64) DEFAULT '' NOT NULL,\n\
`macaddress_b` varchar(64) DEFAULT '' NOT NULL,\n\
`hardware` varchar(255) DEFAULT '' NOT NULL,\n\
`hardware_full` text  NOT NULL,\n\
`software` varchar(255) DEFAULT '' NOT NULL,\n\
`software_full` text  NOT NULL,\n\
`software_app_a` varchar(64) DEFAULT '' NOT NULL,\n\
`software_app_b` varchar(64) DEFAULT '' NOT NULL,\n\
`software_app_c` varchar(64) DEFAULT '' NOT NULL,\n\
`software_app_d` varchar(64) DEFAULT '' NOT NULL,\n\
`software_app_e` varchar(64) DEFAULT '' NOT NULL,\n\
`contact` text  NOT NULL,\n\
`location` text  NOT NULL,\n\
`location_lat` varchar(16) DEFAULT '' NOT NULL,\n\
`location_lon` varchar(16) DEFAULT '' NOT NULL,\n\
`notes` text  NOT NULL,\n\
`chassis` varchar(64) DEFAULT '' NOT NULL,\n\
`model` varchar(64) DEFAULT '' NOT NULL,\n\
`hw_arch` varchar(32) DEFAULT '' NOT NULL,\n\
`vendor` varchar(64) DEFAULT '' NOT NULL,\n\
`contract_number` varchar(64) DEFAULT '' NOT NULL,\n\
`installer_name` varchar(64) DEFAULT '' NOT NULL,\n\
`deployment_status` varchar(64) DEFAULT '' NOT NULL,\n\
`url_a` varchar(255) DEFAULT '' NOT NULL,\n\
`url_b` varchar(255) DEFAULT '' NOT NULL,\n\
`url_c` varchar(255) DEFAULT '' NOT NULL,\n\
`host_networks` text  NOT NULL,\n\
`host_netmask` varchar(39) DEFAULT '' NOT NULL,\n\
`host_router` varchar(39) DEFAULT '' NOT NULL,\n\
`oob_ip` varchar(39) DEFAULT '' NOT NULL,\n\
`oob_netmask` varchar(39) DEFAULT '' NOT NULL,\n\
`oob_router` varchar(39) DEFAULT '' NOT NULL,\n\
`date_hw_purchase` varchar(64) DEFAULT '' NOT NULL,\n\
`date_hw_install` varchar(64) DEFAULT '' NOT NULL,\n\
`date_hw_expiry` varchar(64) DEFAULT '' NOT NULL,\n\
`date_hw_decomm` varchar(64) DEFAULT '' NOT NULL,\n\
`site_address_a` varchar(128) DEFAULT '' NOT NULL,\n\
`site_address_b` varchar(128) DEFAULT '' NOT NULL,\n\
`site_address_c` varchar(128) DEFAULT '' NOT NULL,\n\
`site_city` varchar(128) DEFAULT '' NOT NULL,\n\
`site_state` varchar(64) DEFAULT '' NOT NULL,\n\
`site_country` varchar(64) DEFAULT '' NOT NULL,\n\
`site_zip` varchar(64) DEFAULT '' NOT NULL,\n\
`site_rack` varchar(128) DEFAULT '' NOT NULL,\n\
`site_notes` text  NOT NULL,\n\
`poc_1_name` varchar(128) DEFAULT '' NOT NULL,\n\
`poc_1_email` varchar(128) DEFAULT '' NOT NULL,\n\
`poc_1_phone_a` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_1_phone_b` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_1_cell` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_1_screen` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_1_notes` text  NOT NULL,\n\
`poc_2_name` varchar(128) DEFAULT '' NOT NULL,\n\
`poc_2_email` varchar(128) DEFAULT '' NOT NULL,\n\
`poc_2_phone_a` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_2_phone_b` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_2_cell` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_2_screen` varchar(64) DEFAULT '' NOT NULL,\n\
`poc_2_notes` text  NOT NULL,\n\
PRIMARY KEY (hostid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `housekeeper` (\n\
`housekeeperid` bigint unsigned  NOT NULL,\n\
`tablename` varchar(64) DEFAULT '' NOT NULL,\n\
`field` varchar(64) DEFAULT '' NOT NULL,\n\
`value` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (housekeeperid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `images` (\n\
`imageid` bigint unsigned  NOT NULL,\n\
`imagetype` integer DEFAULT '0' NOT NULL,\n\
`name` varchar(64) DEFAULT '0' NOT NULL,\n\
`image` longblob  NOT NULL,\n\
PRIMARY KEY (imageid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `images_1` ON `images` (`imagetype`,`name`);\n\
CREATE TABLE `item_discovery` (\n\
`itemdiscoveryid` bigint unsigned  NOT NULL,\n\
`itemid` bigint unsigned  NOT NULL,\n\
`parent_itemid` bigint unsigned  NOT NULL,\n\
`key_` varchar(255) DEFAULT '' NOT NULL,\n\
`lastcheck` integer DEFAULT '0' NOT NULL,\n\
`ts_delete` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemdiscoveryid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `item_discovery_1` ON `item_discovery` (`itemid`,`parent_itemid`);\n\
CREATE INDEX `item_discovery_2` ON `item_discovery` (`parent_itemid`);\n\
CREATE TABLE `host_discovery` (\n\
`hostid` bigint unsigned  NOT NULL,\n\
`parent_hostid` bigint unsigned  NULL,\n\
`parent_itemid` bigint unsigned  NULL,\n\
`host` varchar(64) DEFAULT '' NOT NULL,\n\
`lastcheck` integer DEFAULT '0' NOT NULL,\n\
`ts_delete` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `interface_discovery` (\n\
`interfaceid` bigint unsigned  NOT NULL,\n\
`parent_interfaceid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
) ENGINE=InnoDB;\n\
CREATE TABLE `profiles` (\n\
`profileid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
`idx` varchar(96) DEFAULT '' NOT NULL,\n\
`idx2` bigint unsigned DEFAULT '0' NOT NULL,\n\
`value_id` bigint unsigned DEFAULT '0' NOT NULL,\n\
`value_int` integer DEFAULT '0' NOT NULL,\n\
`value_str` varchar(255) DEFAULT '' NOT NULL,\n\
`source` varchar(96) DEFAULT '' NOT NULL,\n\
`type` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (profileid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `profiles_1` ON `profiles` (`userid`,`idx`,`idx2`);\n\
CREATE INDEX `profiles_2` ON `profiles` (`userid`,`profileid`);\n\
CREATE TABLE `sessions` (\n\
`sessionid` varchar(32) DEFAULT '' NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
`lastaccess` integer DEFAULT '0' NOT NULL,\n\
`status` integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sessionid)\n\
) ENGINE=InnoDB;\n\
CREATE INDEX `sessions_1` ON `sessions` (`userid`,`status`);\n\
CREATE TABLE `trigger_discovery` (\n\
`triggerdiscoveryid` bigint unsigned  NOT NULL,\n\
`triggerid` bigint unsigned  NOT NULL,\n\
`parent_triggerid` bigint unsigned  NOT NULL,\n\
`name` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (triggerdiscoveryid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `trigger_discovery_1` ON `trigger_discovery` (`triggerid`,`parent_triggerid`);\n\
CREATE INDEX `trigger_discovery_2` ON `trigger_discovery` (`parent_triggerid`);\n\
CREATE TABLE `user_history` (\n\
`userhistoryid` bigint unsigned  NOT NULL,\n\
`userid` bigint unsigned  NOT NULL,\n\
`title1` varchar(255) DEFAULT '' NOT NULL,\n\
`url1` varchar(255) DEFAULT '' NOT NULL,\n\
`title2` varchar(255) DEFAULT '' NOT NULL,\n\
`url2` varchar(255) DEFAULT '' NOT NULL,\n\
`title3` varchar(255) DEFAULT '' NOT NULL,\n\
`url3` varchar(255) DEFAULT '' NOT NULL,\n\
`title4` varchar(255) DEFAULT '' NOT NULL,\n\
`url4` varchar(255) DEFAULT '' NOT NULL,\n\
`title5` varchar(255) DEFAULT '' NOT NULL,\n\
`url5` varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (userhistoryid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `user_history_1` ON `user_history` (`userid`);\n\
CREATE TABLE `application_template` (\n\
`application_templateid` bigint unsigned  NOT NULL,\n\
`applicationid` bigint unsigned  NOT NULL,\n\
`templateid` bigint unsigned  NOT NULL,\n\
PRIMARY KEY (application_templateid)\n\
) ENGINE=InnoDB;\n\
CREATE UNIQUE INDEX `application_template_1` ON `application_template` (`applicationid`,`templateid`);\n\
CREATE INDEX `application_template_2` ON `application_template` (`templateid`);\n\
CREATE TABLE `dbversion` (\n\
`mandatory` integer DEFAULT '0' NOT NULL,\n\
`optional` integer DEFAULT '0' NOT NULL\n\
) ENGINE=InnoDB;\n\
INSERT INTO dbversion VALUES ('2020000','2020001');\n\
";
const char	*const db_schema_fkeys[] = {
	"ALTER TABLE `hosts` ADD CONSTRAINT `c_hosts_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`)",
	"ALTER TABLE `hosts` ADD CONSTRAINT `c_hosts_2` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`)",
	"ALTER TABLE `hosts` ADD CONSTRAINT `c_hosts_3` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `group_prototype` ADD CONSTRAINT `c_group_prototype_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `group_prototype` ADD CONSTRAINT `c_group_prototype_2` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`)",
	"ALTER TABLE `group_prototype` ADD CONSTRAINT `c_group_prototype_3` FOREIGN KEY (`templateid`) REFERENCES `group_prototype` (`group_prototypeid`) ON DELETE CASCADE",
	"ALTER TABLE `group_discovery` ADD CONSTRAINT `c_group_discovery_1` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`) ON DELETE CASCADE",
	"ALTER TABLE `group_discovery` ADD CONSTRAINT `c_group_discovery_2` FOREIGN KEY (`parent_group_prototypeid`) REFERENCES `group_prototype` (`group_prototypeid`)",
	"ALTER TABLE `screens` ADD CONSTRAINT `c_screens_1` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `screens_items` ADD CONSTRAINT `c_screens_items_1` FOREIGN KEY (`screenid`) REFERENCES `screens` (`screenid`) ON DELETE CASCADE",
	"ALTER TABLE `slides` ADD CONSTRAINT `c_slides_1` FOREIGN KEY (`slideshowid`) REFERENCES `slideshows` (`slideshowid`) ON DELETE CASCADE",
	"ALTER TABLE `slides` ADD CONSTRAINT `c_slides_2` FOREIGN KEY (`screenid`) REFERENCES `screens` (`screenid`) ON DELETE CASCADE",
	"ALTER TABLE `drules` ADD CONSTRAINT `c_drules_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`)",
	"ALTER TABLE `dchecks` ADD CONSTRAINT `c_dchecks_1` FOREIGN KEY (`druleid`) REFERENCES `drules` (`druleid`) ON DELETE CASCADE",
	"ALTER TABLE `applications` ADD CONSTRAINT `c_applications_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `httptest` ADD CONSTRAINT `c_httptest_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`)",
	"ALTER TABLE `httptest` ADD CONSTRAINT `c_httptest_2` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `httptest` ADD CONSTRAINT `c_httptest_3` FOREIGN KEY (`templateid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE",
	"ALTER TABLE `httpstep` ADD CONSTRAINT `c_httpstep_1` FOREIGN KEY (`httptestid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE",
	"ALTER TABLE `interface` ADD CONSTRAINT `c_interface_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `items` ADD CONSTRAINT `c_items_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `items` ADD CONSTRAINT `c_items_2` FOREIGN KEY (`templateid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `items` ADD CONSTRAINT `c_items_3` FOREIGN KEY (`valuemapid`) REFERENCES `valuemaps` (`valuemapid`)",
	"ALTER TABLE `items` ADD CONSTRAINT `c_items_4` FOREIGN KEY (`interfaceid`) REFERENCES `interface` (`interfaceid`)",
	"ALTER TABLE `httpstepitem` ADD CONSTRAINT `c_httpstepitem_1` FOREIGN KEY (`httpstepid`) REFERENCES `httpstep` (`httpstepid`) ON DELETE CASCADE",
	"ALTER TABLE `httpstepitem` ADD CONSTRAINT `c_httpstepitem_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `httptestitem` ADD CONSTRAINT `c_httptestitem_1` FOREIGN KEY (`httptestid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE",
	"ALTER TABLE `httptestitem` ADD CONSTRAINT `c_httptestitem_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `users_groups` ADD CONSTRAINT `c_users_groups_1` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE",
	"ALTER TABLE `users_groups` ADD CONSTRAINT `c_users_groups_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `scripts` ADD CONSTRAINT `c_scripts_1` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`)",
	"ALTER TABLE `scripts` ADD CONSTRAINT `c_scripts_2` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`)",
	"ALTER TABLE `operations` ADD CONSTRAINT `c_operations_1` FOREIGN KEY (`actionid`) REFERENCES `actions` (`actionid`) ON DELETE CASCADE",
	"ALTER TABLE `opmessage` ADD CONSTRAINT `c_opmessage_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opmessage` ADD CONSTRAINT `c_opmessage_2` FOREIGN KEY (`mediatypeid`) REFERENCES `media_type` (`mediatypeid`)",
	"ALTER TABLE `opmessage_grp` ADD CONSTRAINT `c_opmessage_grp_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opmessage_grp` ADD CONSTRAINT `c_opmessage_grp_2` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`)",
	"ALTER TABLE `opmessage_usr` ADD CONSTRAINT `c_opmessage_usr_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opmessage_usr` ADD CONSTRAINT `c_opmessage_usr_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`)",
	"ALTER TABLE `opcommand` ADD CONSTRAINT `c_opcommand_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opcommand` ADD CONSTRAINT `c_opcommand_2` FOREIGN KEY (`scriptid`) REFERENCES `scripts` (`scriptid`)",
	"ALTER TABLE `opcommand_hst` ADD CONSTRAINT `c_opcommand_hst_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opcommand_hst` ADD CONSTRAINT `c_opcommand_hst_2` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`)",
	"ALTER TABLE `opcommand_grp` ADD CONSTRAINT `c_opcommand_grp_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opcommand_grp` ADD CONSTRAINT `c_opcommand_grp_2` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`)",
	"ALTER TABLE `opgroup` ADD CONSTRAINT `c_opgroup_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `opgroup` ADD CONSTRAINT `c_opgroup_2` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`)",
	"ALTER TABLE `optemplate` ADD CONSTRAINT `c_optemplate_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `optemplate` ADD CONSTRAINT `c_optemplate_2` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`)",
	"ALTER TABLE `opconditions` ADD CONSTRAINT `c_opconditions_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE",
	"ALTER TABLE `conditions` ADD CONSTRAINT `c_conditions_1` FOREIGN KEY (`actionid`) REFERENCES `actions` (`actionid`) ON DELETE CASCADE",
	"ALTER TABLE `config` ADD CONSTRAINT `c_config_1` FOREIGN KEY (`alert_usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`)",
	"ALTER TABLE `config` ADD CONSTRAINT `c_config_2` FOREIGN KEY (`discovery_groupid`) REFERENCES `groups` (`groupid`)",
	"ALTER TABLE `triggers` ADD CONSTRAINT `c_triggers_1` FOREIGN KEY (`templateid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `trigger_depends` ADD CONSTRAINT `c_trigger_depends_1` FOREIGN KEY (`triggerid_down`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `trigger_depends` ADD CONSTRAINT `c_trigger_depends_2` FOREIGN KEY (`triggerid_up`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `functions` ADD CONSTRAINT `c_functions_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `functions` ADD CONSTRAINT `c_functions_2` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `graphs` ADD CONSTRAINT `c_graphs_1` FOREIGN KEY (`templateid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE",
	"ALTER TABLE `graphs` ADD CONSTRAINT `c_graphs_2` FOREIGN KEY (`ymin_itemid`) REFERENCES `items` (`itemid`)",
	"ALTER TABLE `graphs` ADD CONSTRAINT `c_graphs_3` FOREIGN KEY (`ymax_itemid`) REFERENCES `items` (`itemid`)",
	"ALTER TABLE `graphs_items` ADD CONSTRAINT `c_graphs_items_1` FOREIGN KEY (`graphid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE",
	"ALTER TABLE `graphs_items` ADD CONSTRAINT `c_graphs_items_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `hostmacro` ADD CONSTRAINT `c_hostmacro_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `hosts_groups` ADD CONSTRAINT `c_hosts_groups_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `hosts_groups` ADD CONSTRAINT `c_hosts_groups_2` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`) ON DELETE CASCADE",
	"ALTER TABLE `hosts_templates` ADD CONSTRAINT `c_hosts_templates_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `hosts_templates` ADD CONSTRAINT `c_hosts_templates_2` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `items_applications` ADD CONSTRAINT `c_items_applications_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE",
	"ALTER TABLE `items_applications` ADD CONSTRAINT `c_items_applications_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `mappings` ADD CONSTRAINT `c_mappings_1` FOREIGN KEY (`valuemapid`) REFERENCES `valuemaps` (`valuemapid`) ON DELETE CASCADE",
	"ALTER TABLE `media` ADD CONSTRAINT `c_media_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `media` ADD CONSTRAINT `c_media_2` FOREIGN KEY (`mediatypeid`) REFERENCES `media_type` (`mediatypeid`) ON DELETE CASCADE",
	"ALTER TABLE `rights` ADD CONSTRAINT `c_rights_1` FOREIGN KEY (`groupid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE",
	"ALTER TABLE `rights` ADD CONSTRAINT `c_rights_2` FOREIGN KEY (`id`) REFERENCES `groups` (`groupid`) ON DELETE CASCADE",
	"ALTER TABLE `services` ADD CONSTRAINT `c_services_1` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `services_links` ADD CONSTRAINT `c_services_links_1` FOREIGN KEY (`serviceupid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE",
	"ALTER TABLE `services_links` ADD CONSTRAINT `c_services_links_2` FOREIGN KEY (`servicedownid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE",
	"ALTER TABLE `services_times` ADD CONSTRAINT `c_services_times_1` FOREIGN KEY (`serviceid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE",
	"ALTER TABLE `icon_map` ADD CONSTRAINT `c_icon_map_1` FOREIGN KEY (`default_iconid`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `icon_mapping` ADD CONSTRAINT `c_icon_mapping_1` FOREIGN KEY (`iconmapid`) REFERENCES `icon_map` (`iconmapid`) ON DELETE CASCADE",
	"ALTER TABLE `icon_mapping` ADD CONSTRAINT `c_icon_mapping_2` FOREIGN KEY (`iconid`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `sysmaps` ADD CONSTRAINT `c_sysmaps_1` FOREIGN KEY (`backgroundid`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `sysmaps` ADD CONSTRAINT `c_sysmaps_2` FOREIGN KEY (`iconmapid`) REFERENCES `icon_map` (`iconmapid`)",
	"ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_2` FOREIGN KEY (`iconid_off`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_3` FOREIGN KEY (`iconid_on`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_4` FOREIGN KEY (`iconid_disabled`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_5` FOREIGN KEY (`iconid_maintenance`) REFERENCES `images` (`imageid`)",
	"ALTER TABLE `sysmaps_links` ADD CONSTRAINT `c_sysmaps_links_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmaps_links` ADD CONSTRAINT `c_sysmaps_links_2` FOREIGN KEY (`selementid1`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmaps_links` ADD CONSTRAINT `c_sysmaps_links_3` FOREIGN KEY (`selementid2`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmaps_link_triggers` ADD CONSTRAINT `c_sysmaps_link_triggers_1` FOREIGN KEY (`linkid`) REFERENCES `sysmaps_links` (`linkid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmaps_link_triggers` ADD CONSTRAINT `c_sysmaps_link_triggers_2` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmap_element_url` ADD CONSTRAINT `c_sysmap_element_url_1` FOREIGN KEY (`selementid`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE",
	"ALTER TABLE `sysmap_url` ADD CONSTRAINT `c_sysmap_url_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE",
	"ALTER TABLE `maintenances_hosts` ADD CONSTRAINT `c_maintenances_hosts_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE",
	"ALTER TABLE `maintenances_hosts` ADD CONSTRAINT `c_maintenances_hosts_2` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `maintenances_groups` ADD CONSTRAINT `c_maintenances_groups_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE",
	"ALTER TABLE `maintenances_groups` ADD CONSTRAINT `c_maintenances_groups_2` FOREIGN KEY (`groupid`) REFERENCES `groups` (`groupid`) ON DELETE CASCADE",
	"ALTER TABLE `maintenances_windows` ADD CONSTRAINT `c_maintenances_windows_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE",
	"ALTER TABLE `maintenances_windows` ADD CONSTRAINT `c_maintenances_windows_2` FOREIGN KEY (`timeperiodid`) REFERENCES `timeperiods` (`timeperiodid`) ON DELETE CASCADE",
	"ALTER TABLE `expressions` ADD CONSTRAINT `c_expressions_1` FOREIGN KEY (`regexpid`) REFERENCES `regexps` (`regexpid`) ON DELETE CASCADE",
	"ALTER TABLE `nodes` ADD CONSTRAINT `c_nodes_1` FOREIGN KEY (`masterid`) REFERENCES `nodes` (`nodeid`)",
	"ALTER TABLE `node_cksum` ADD CONSTRAINT `c_node_cksum_1` FOREIGN KEY (`nodeid`) REFERENCES `nodes` (`nodeid`) ON DELETE CASCADE",
	"ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_1` FOREIGN KEY (`actionid`) REFERENCES `actions` (`actionid`) ON DELETE CASCADE",
	"ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_2` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE",
	"ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_3` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_4` FOREIGN KEY (`mediatypeid`) REFERENCES `media_type` (`mediatypeid`) ON DELETE CASCADE",
	"ALTER TABLE `acknowledges` ADD CONSTRAINT `c_acknowledges_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `acknowledges` ADD CONSTRAINT `c_acknowledges_2` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE",
	"ALTER TABLE `auditlog` ADD CONSTRAINT `c_auditlog_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `auditlog_details` ADD CONSTRAINT `c_auditlog_details_1` FOREIGN KEY (`auditid`) REFERENCES `auditlog` (`auditid`) ON DELETE CASCADE",
	"ALTER TABLE `service_alarms` ADD CONSTRAINT `c_service_alarms_1` FOREIGN KEY (`serviceid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE",
	"ALTER TABLE `autoreg_host` ADD CONSTRAINT `c_autoreg_host_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `dhosts` ADD CONSTRAINT `c_dhosts_1` FOREIGN KEY (`druleid`) REFERENCES `drules` (`druleid`) ON DELETE CASCADE",
	"ALTER TABLE `dservices` ADD CONSTRAINT `c_dservices_1` FOREIGN KEY (`dhostid`) REFERENCES `dhosts` (`dhostid`) ON DELETE CASCADE",
	"ALTER TABLE `dservices` ADD CONSTRAINT `c_dservices_2` FOREIGN KEY (`dcheckid`) REFERENCES `dchecks` (`dcheckid`) ON DELETE CASCADE",
	"ALTER TABLE `graph_discovery` ADD CONSTRAINT `c_graph_discovery_1` FOREIGN KEY (`graphid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE",
	"ALTER TABLE `graph_discovery` ADD CONSTRAINT `c_graph_discovery_2` FOREIGN KEY (`parent_graphid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE",
	"ALTER TABLE `host_inventory` ADD CONSTRAINT `c_host_inventory_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `item_discovery` ADD CONSTRAINT `c_item_discovery_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `item_discovery` ADD CONSTRAINT `c_item_discovery_2` FOREIGN KEY (`parent_itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE",
	"ALTER TABLE `host_discovery` ADD CONSTRAINT `c_host_discovery_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE",
	"ALTER TABLE `host_discovery` ADD CONSTRAINT `c_host_discovery_2` FOREIGN KEY (`parent_hostid`) REFERENCES `hosts` (`hostid`)",
	"ALTER TABLE `host_discovery` ADD CONSTRAINT `c_host_discovery_3` FOREIGN KEY (`parent_itemid`) REFERENCES `items` (`itemid`)",
	"ALTER TABLE `interface_discovery` ADD CONSTRAINT `c_interface_discovery_1` FOREIGN KEY (`interfaceid`) REFERENCES `interface` (`interfaceid`) ON DELETE CASCADE",
	"ALTER TABLE `interface_discovery` ADD CONSTRAINT `c_interface_discovery_2` FOREIGN KEY (`parent_interfaceid`) REFERENCES `interface` (`interfaceid`) ON DELETE CASCADE",
	"ALTER TABLE `profiles` ADD CONSTRAINT `c_profiles_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `sessions` ADD CONSTRAINT `c_sessions_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `trigger_discovery` ADD CONSTRAINT `c_trigger_discovery_1` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `trigger_discovery` ADD CONSTRAINT `c_trigger_discovery_2` FOREIGN KEY (`parent_triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE",
	"ALTER TABLE `user_history` ADD CONSTRAINT `c_user_history_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE",
	"ALTER TABLE `application_template` ADD CONSTRAINT `c_application_template_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE",
	"ALTER TABLE `application_template` ADD CONSTRAINT `c_application_template_2` FOREIGN KEY (`templateid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE",
	NULL
};
const char	*const db_schema_fkeys_drop[] = {
	"ALTER TABLE `hosts` DROP FOREIGN KEY `c_hosts_1`",
	"ALTER TABLE `hosts` DROP FOREIGN KEY `c_hosts_2`",
	"ALTER TABLE `hosts` DROP FOREIGN KEY `c_hosts_3`",
	"ALTER TABLE `group_prototype` DROP FOREIGN KEY `c_group_prototype_1`",
	"ALTER TABLE `group_prototype` DROP FOREIGN KEY `c_group_prototype_2`",
	"ALTER TABLE `group_prototype` DROP FOREIGN KEY `c_group_prototype_3`",
	"ALTER TABLE `group_discovery` DROP FOREIGN KEY `c_group_discovery_1`",
	"ALTER TABLE `group_discovery` DROP FOREIGN KEY `c_group_discovery_2`",
	"ALTER TABLE `screens` DROP FOREIGN KEY `c_screens_1`",
	"ALTER TABLE `screens_items` DROP FOREIGN KEY `c_screens_items_1`",
	"ALTER TABLE `slides` DROP FOREIGN KEY `c_slides_1`",
	"ALTER TABLE `slides` DROP FOREIGN KEY `c_slides_2`",
	"ALTER TABLE `drules` DROP FOREIGN KEY `c_drules_1`",
	"ALTER TABLE `dchecks` DROP FOREIGN KEY `c_dchecks_1`",
	"ALTER TABLE `applications` DROP FOREIGN KEY `c_applications_1`",
	"ALTER TABLE `httptest` DROP FOREIGN KEY `c_httptest_1`",
	"ALTER TABLE `httptest` DROP FOREIGN KEY `c_httptest_2`",
	"ALTER TABLE `httptest` DROP FOREIGN KEY `c_httptest_3`",
	"ALTER TABLE `httpstep` DROP FOREIGN KEY `c_httpstep_1`",
	"ALTER TABLE `interface` DROP FOREIGN KEY `c_interface_1`",
	"ALTER TABLE `items` DROP FOREIGN KEY `c_items_1`",
	"ALTER TABLE `items` DROP FOREIGN KEY `c_items_2`",
	"ALTER TABLE `items` DROP FOREIGN KEY `c_items_3`",
	"ALTER TABLE `items` DROP FOREIGN KEY `c_items_4`",
	"ALTER TABLE `httpstepitem` DROP FOREIGN KEY `c_httpstepitem_1`",
	"ALTER TABLE `httpstepitem` DROP FOREIGN KEY `c_httpstepitem_2`",
	"ALTER TABLE `httptestitem` DROP FOREIGN KEY `c_httptestitem_1`",
	"ALTER TABLE `httptestitem` DROP FOREIGN KEY `c_httptestitem_2`",
	"ALTER TABLE `users_groups` DROP FOREIGN KEY `c_users_groups_1`",
	"ALTER TABLE `users_groups` DROP FOREIGN KEY `c_users_groups_2`",
	"ALTER TABLE `scripts` DROP FOREIGN KEY `c_scripts_1`",
	"ALTER TABLE `scripts` DROP FOREIGN KEY `c_scripts_2`",
	"ALTER TABLE `operations` DROP FOREIGN KEY `c_operations_1`",
	"ALTER TABLE `opmessage` DROP FOREIGN KEY `c_opmessage_1`",
	"ALTER TABLE `opmessage` DROP FOREIGN KEY `c_opmessage_2`",
	"ALTER TABLE `opmessage_grp` DROP FOREIGN KEY `c_opmessage_grp_1`",
	"ALTER TABLE `opmessage_grp` DROP FOREIGN KEY `c_opmessage_grp_2`",
	"ALTER TABLE `opmessage_usr` DROP FOREIGN KEY `c_opmessage_usr_1`",
	"ALTER TABLE `opmessage_usr` DROP FOREIGN KEY `c_opmessage_usr_2`",
	"ALTER TABLE `opcommand` DROP FOREIGN KEY `c_opcommand_1`",
	"ALTER TABLE `opcommand` DROP FOREIGN KEY `c_opcommand_2`",
	"ALTER TABLE `opcommand_hst` DROP FOREIGN KEY `c_opcommand_hst_1`",
	"ALTER TABLE `opcommand_hst` DROP FOREIGN KEY `c_opcommand_hst_2`",
	"ALTER TABLE `opcommand_grp` DROP FOREIGN KEY `c_opcommand_grp_1`",
	"ALTER TABLE `opcommand_grp` DROP FOREIGN KEY `c_opcommand_grp_2`",
	"ALTER TABLE `opgroup` DROP FOREIGN KEY `c_opgroup_1`",
	"ALTER TABLE `opgroup` DROP FOREIGN KEY `c_opgroup_2`",
	"ALTER TABLE `optemplate` DROP FOREIGN KEY `c_optemplate_1`",
	"ALTER TABLE `optemplate` DROP FOREIGN KEY `c_optemplate_2`",
	"ALTER TABLE `opconditions` DROP FOREIGN KEY `c_opconditions_1`",
	"ALTER TABLE `conditions` DROP FOREIGN KEY `c_conditions_1`",
	"ALTER TABLE `config` DROP FOREIGN KEY `c_config_1`",
	"ALTER TABLE `config` DROP FOREIGN KEY `c_config_2`",
	"ALTER TABLE `triggers` DROP FOREIGN KEY `c_triggers_1`",
	"ALTER TABLE `trigger_depends` DROP FOREIGN KEY `c_trigger_depends_1`",
	"ALTER TABLE `trigger_depends` DROP FOREIGN KEY `c_trigger_depends_2`",
	"ALTER TABLE `functions` DROP FOREIGN KEY `c_functions_1`",
	"ALTER TABLE `functions` DROP FOREIGN KEY `c_functions_2`",
	"ALTER TABLE `graphs` DROP FOREIGN KEY `c_graphs_1`",
	"ALTER TABLE `graphs` DROP FOREIGN KEY `c_graphs_2`",
	"ALTER TABLE `graphs` DROP FOREIGN KEY `c_graphs_3`",
	"ALTER TABLE `graphs_items` DROP FOREIGN KEY `c_graphs_items_1`",
	"ALTER TABLE `graphs_items` DROP FOREIGN KEY `c_graphs_items_2`",
	"ALTER TABLE `hostmacro` DROP FOREIGN KEY `c_hostmacro_1`",
	"ALTER TABLE `hosts_groups` DROP FOREIGN KEY `c_hosts_groups_1`",
	"ALTER TABLE `hosts_groups` DROP FOREIGN KEY `c_hosts_groups_2`",
	"ALTER TABLE `hosts_templates` DROP FOREIGN KEY `c_hosts_templates_1`",
	"ALTER TABLE `hosts_templates` DROP FOREIGN KEY `c_hosts_templates_2`",
	"ALTER TABLE `items_applications` DROP FOREIGN KEY `c_items_applications_1`",
	"ALTER TABLE `items_applications` DROP FOREIGN KEY `c_items_applications_2`",
	"ALTER TABLE `mappings` DROP FOREIGN KEY `c_mappings_1`",
	"ALTER TABLE `media` DROP FOREIGN KEY `c_media_1`",
	"ALTER TABLE `media` DROP FOREIGN KEY `c_media_2`",
	"ALTER TABLE `rights` DROP FOREIGN KEY `c_rights_1`",
	"ALTER TABLE `rights` DROP FOREIGN KEY `c_rights_2`",
	"ALTER TABLE `services` DROP FOREIGN KEY `c_services_1`",
	"ALTER TABLE `services_links` DROP FOREIGN KEY `c_services_links_1`",
	"ALTER TABLE `services_links` DROP FOREIGN KEY `c_services_links_2`",
	"ALTER TABLE `services_times` DROP FOREIGN KEY `c_services_times_1`",
	"ALTER TABLE `icon_map` DROP FOREIGN KEY `c_icon_map_1`",
	"ALTER TABLE `icon_mapping` DROP FOREIGN KEY `c_icon_mapping_1`",
	"ALTER TABLE `icon_mapping` DROP FOREIGN KEY `c_icon_mapping_2`",
	"ALTER TABLE `sysmaps` DROP FOREIGN KEY `c_sysmaps_1`",
	"ALTER TABLE `sysmaps` DROP FOREIGN KEY `c_sysmaps_2`",
	"ALTER TABLE `sysmaps_elements` DROP FOREIGN KEY `c_sysmaps_elements_1`",
	"ALTER TABLE `sysmaps_elements` DROP FOREIGN KEY `c_sysmaps_elements_2`",
	"ALTER TABLE `sysmaps_elements` DROP FOREIGN KEY `c_sysmaps_elements_3`",
	"ALTER TABLE `sysmaps_elements` DROP FOREIGN KEY `c_sysmaps_elements_4`",
	"ALTER TABLE `sysmaps_elements` DROP FOREIGN KEY `c_sysmaps_elements_5`",
	"ALTER TABLE `sysmaps_links` DROP FOREIGN KEY `c_sysmaps_links_1`",
	"ALTER TABLE `sysmaps_links` DROP FOREIGN KEY `c_sysmaps_links_2`",
	"ALTER TABLE `sysmaps_links` DROP FOREIGN KEY `c_sysmaps_links_3`",
	"ALTER TABLE `sysmaps_link_triggers` DROP FOREIGN KEY `c_sysmaps_link_triggers_1`",
	"ALTER TABLE `sysmaps_link_triggers` DROP FOREIGN KEY `c_sysmaps_link_triggers_2`",
	"ALTER TABLE `sysmap_element_url` DROP FOREIGN KEY `c_sysmap_element_url_1`",
	"ALTER TABLE `sysmap_url` DROP FOREIGN KEY `c_sysmap_url_1`",
	"ALTER TABLE `maintenances_hosts` DROP FOREIGN KEY `c_maintenances_hosts_1`",
	"ALTER TABLE `maintenances_hosts` DROP FOREIGN KEY `c_maintenances_hosts_2`",
	"ALTER TABLE `maintenances_groups` DROP FOREIGN KEY `c_maintenances_groups_1`",
	"ALTER TABLE `maintenances_groups` DROP FOREIGN KEY `c_maintenances_groups_2`",
	"ALTER TABLE `maintenances_windows` DROP FOREIGN KEY `c_maintenances_windows_1`",
	"ALTER TABLE `maintenances_windows` DROP FOREIGN KEY `c_maintenances_windows_2`",
	"ALTER TABLE `expressions` DROP FOREIGN KEY `c_expressions_1`",
	"ALTER TABLE `nodes` DROP FOREIGN KEY `c_nodes_1`",
	"ALTER TABLE `node_cksum` DROP FOREIGN KEY `c_node_cksum_1`",
	"ALTER TABLE `alerts` DROP FOREIGN KEY `c_alerts_1`",
	"ALTER TABLE `alerts` DROP FOREIGN KEY `c_alerts_2`",
	"ALTER TABLE `alerts` DROP FOREIGN KEY `c_alerts_3`",
	"ALTER TABLE `alerts` DROP FOREIGN KEY `c_alerts_4`",
	"ALTER TABLE `acknowledges` DROP FOREIGN KEY `c_acknowledges_1`",
	"ALTER TABLE `acknowledges` DROP FOREIGN KEY `c_acknowledges_2`",
	"ALTER TABLE `auditlog` DROP FOREIGN KEY `c_auditlog_1`",
	"ALTER TABLE `auditlog_details` DROP FOREIGN KEY `c_auditlog_details_1`",
	"ALTER TABLE `service_alarms` DROP FOREIGN KEY `c_service_alarms_1`",
	"ALTER TABLE `autoreg_host` DROP FOREIGN KEY `c_autoreg_host_1`",
	"ALTER TABLE `dhosts` DROP FOREIGN KEY `c_dhosts_1`",
	"ALTER TABLE `dservices` DROP FOREIGN KEY `c_dservices_1`",
	"ALTER TABLE `dservices` DROP FOREIGN KEY `c_dservices_2`",
	"ALTER TABLE `graph_discovery` DROP FOREIGN KEY `c_graph_discovery_1`",
	"ALTER TABLE `graph_discovery` DROP FOREIGN KEY `c_graph_discovery_2`",
	"ALTER TABLE `host_inventory` DROP FOREIGN KEY `c_host_inventory_1`",
	"ALTER TABLE `item_discovery` DROP FOREIGN KEY `c_item_discovery_1`",
	"ALTER TABLE `item_discovery` DROP FOREIGN KEY `c_item_discovery_2`",
	"ALTER TABLE `host_discovery` DROP FOREIGN KEY `c_host_discovery_1`",
	"ALTER TABLE `host_discovery` DROP FOREIGN KEY `c_host_discovery_2`",
	"ALTER TABLE `host_discovery` DROP FOREIGN KEY `c_host_discovery_3`",
	"ALTER TABLE `interface_discovery` DROP FOREIGN KEY `c_interface_discovery_1`",
	"ALTER TABLE `interface_discovery` DROP FOREIGN KEY `c_interface_discovery_2`",
	"ALTER TABLE `profiles` DROP FOREIGN KEY `c_profiles_1`",
	"ALTER TABLE `sessions` DROP FOREIGN KEY `c_sessions_1`",
	"ALTER TABLE `trigger_discovery` DROP FOREIGN KEY `c_trigger_discovery_1`",
	"ALTER TABLE `trigger_discovery` DROP FOREIGN KEY `c_trigger_discovery_2`",
	"ALTER TABLE `user_history` DROP FOREIGN KEY `c_user_history_1`",
	"ALTER TABLE `application_template` DROP FOREIGN KEY `c_application_template_1`",
	"ALTER TABLE `application_template` DROP FOREIGN KEY `c_application_template_2`",
	NULL
};
#elif defined(HAVE_ORACLE)
const char	*const db_schema = "\
CREATE TABLE maintenances (\n\
maintenanceid number(20)  NOT NULL,\n\
name nvarchar2(128) DEFAULT '' ,\n\
maintenance_type number(10) DEFAULT '0' NOT NULL,\n\
description nvarchar2(2048) DEFAULT '' ,\n\
active_since number(10) DEFAULT '0' NOT NULL,\n\
active_till number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (maintenanceid)\n\
);\n\
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);\n\
CREATE TABLE hosts (\n\
hostid number(20)  NOT NULL,\n\
proxy_hostid number(20)  NULL,\n\
host nvarchar2(64) DEFAULT '' ,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
disable_until number(10) DEFAULT '0' NOT NULL,\n\
error nvarchar2(128) DEFAULT '' ,\n\
available number(10) DEFAULT '0' NOT NULL,\n\
errors_from number(10) DEFAULT '0' NOT NULL,\n\
lastaccess number(10) DEFAULT '0' NOT NULL,\n\
ipmi_authtype number(10) DEFAULT '0' NOT NULL,\n\
ipmi_privilege number(10) DEFAULT '2' NOT NULL,\n\
ipmi_username nvarchar2(16) DEFAULT '' ,\n\
ipmi_password nvarchar2(20) DEFAULT '' ,\n\
ipmi_disable_until number(10) DEFAULT '0' NOT NULL,\n\
ipmi_available number(10) DEFAULT '0' NOT NULL,\n\
snmp_disable_until number(10) DEFAULT '0' NOT NULL,\n\
snmp_available number(10) DEFAULT '0' NOT NULL,\n\
maintenanceid number(20)  NULL,\n\
maintenance_status number(10) DEFAULT '0' NOT NULL,\n\
maintenance_type number(10) DEFAULT '0' NOT NULL,\n\
maintenance_from number(10) DEFAULT '0' NOT NULL,\n\
ipmi_errors_from number(10) DEFAULT '0' NOT NULL,\n\
snmp_errors_from number(10) DEFAULT '0' NOT NULL,\n\
ipmi_error nvarchar2(128) DEFAULT '' ,\n\
snmp_error nvarchar2(128) DEFAULT '' ,\n\
jmx_disable_until number(10) DEFAULT '0' NOT NULL,\n\
jmx_available number(10) DEFAULT '0' NOT NULL,\n\
jmx_errors_from number(10) DEFAULT '0' NOT NULL,\n\
jmx_error nvarchar2(128) DEFAULT '' ,\n\
name nvarchar2(64) DEFAULT '' ,\n\
flags number(10) DEFAULT '0' NOT NULL,\n\
templateid number(20)  NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE INDEX hosts_1 ON hosts (host);\n\
CREATE INDEX hosts_2 ON hosts (status);\n\
CREATE INDEX hosts_3 ON hosts (proxy_hostid);\n\
CREATE INDEX hosts_4 ON hosts (name);\n\
CREATE INDEX hosts_5 ON hosts (maintenanceid);\n\
CREATE TABLE groups (\n\
groupid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
internal number(10) DEFAULT '0' NOT NULL,\n\
flags number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE INDEX groups_1 ON groups (name);\n\
CREATE TABLE group_prototype (\n\
group_prototypeid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
groupid number(20)  NULL,\n\
templateid number(20)  NULL,\n\
PRIMARY KEY (group_prototypeid)\n\
);\n\
CREATE INDEX group_prototype_1 ON group_prototype (hostid);\n\
CREATE TABLE group_discovery (\n\
groupid number(20)  NOT NULL,\n\
parent_group_prototypeid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
lastcheck number(10) DEFAULT '0' NOT NULL,\n\
ts_delete number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE TABLE screens (\n\
screenid number(20)  NOT NULL,\n\
name nvarchar2(255)  ,\n\
hsize number(10) DEFAULT '1' NOT NULL,\n\
vsize number(10) DEFAULT '1' NOT NULL,\n\
templateid number(20)  NULL,\n\
PRIMARY KEY (screenid)\n\
);\n\
CREATE INDEX screens_1 ON screens (templateid);\n\
CREATE TABLE screens_items (\n\
screenitemid number(20)  NOT NULL,\n\
screenid number(20)  NOT NULL,\n\
resourcetype number(10) DEFAULT '0' NOT NULL,\n\
resourceid number(20) DEFAULT '0' NOT NULL,\n\
width number(10) DEFAULT '320' NOT NULL,\n\
height number(10) DEFAULT '200' NOT NULL,\n\
x number(10) DEFAULT '0' NOT NULL,\n\
y number(10) DEFAULT '0' NOT NULL,\n\
colspan number(10) DEFAULT '0' NOT NULL,\n\
rowspan number(10) DEFAULT '0' NOT NULL,\n\
elements number(10) DEFAULT '25' NOT NULL,\n\
valign number(10) DEFAULT '0' NOT NULL,\n\
halign number(10) DEFAULT '0' NOT NULL,\n\
style number(10) DEFAULT '0' NOT NULL,\n\
url nvarchar2(255) DEFAULT '' ,\n\
dynamic number(10) DEFAULT '0' NOT NULL,\n\
sort_triggers number(10) DEFAULT '0' NOT NULL,\n\
application nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (screenitemid)\n\
);\n\
CREATE INDEX screens_items_1 ON screens_items (screenid);\n\
CREATE TABLE slideshows (\n\
slideshowid number(20)  NOT NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
delay number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideshowid)\n\
);\n\
CREATE TABLE slides (\n\
slideid number(20)  NOT NULL,\n\
slideshowid number(20)  NOT NULL,\n\
screenid number(20)  NOT NULL,\n\
step number(10) DEFAULT '0' NOT NULL,\n\
delay number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideid)\n\
);\n\
CREATE INDEX slides_1 ON slides (slideshowid);\n\
CREATE INDEX slides_2 ON slides (screenid);\n\
CREATE TABLE drules (\n\
druleid number(20)  NOT NULL,\n\
proxy_hostid number(20)  NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
iprange nvarchar2(255) DEFAULT '' ,\n\
delay number(10) DEFAULT '3600' NOT NULL,\n\
nextcheck number(10) DEFAULT '0' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (druleid)\n\
);\n\
CREATE INDEX drules_1 ON drules (proxy_hostid);\n\
CREATE TABLE dchecks (\n\
dcheckid number(20)  NOT NULL,\n\
druleid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
key_ nvarchar2(255) DEFAULT '' ,\n\
snmp_community nvarchar2(255) DEFAULT '' ,\n\
ports nvarchar2(255) DEFAULT '0' ,\n\
snmpv3_securityname nvarchar2(64) DEFAULT '' ,\n\
snmpv3_securitylevel number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase nvarchar2(64) DEFAULT '' ,\n\
snmpv3_privpassphrase nvarchar2(64) DEFAULT '' ,\n\
uniq number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_authprotocol number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_contextname nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (dcheckid)\n\
);\n\
CREATE INDEX dchecks_1 ON dchecks (druleid);\n\
CREATE TABLE applications (\n\
applicationid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (applicationid)\n\
);\n\
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);\n\
CREATE TABLE httptest (\n\
httptestid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
applicationid number(20)  NULL,\n\
nextcheck number(10) DEFAULT '0' NOT NULL,\n\
delay number(10) DEFAULT '60' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
variables nvarchar2(2048) DEFAULT '' ,\n\
agent nvarchar2(255) DEFAULT '' ,\n\
authentication number(10) DEFAULT '0' NOT NULL,\n\
http_user nvarchar2(64) DEFAULT '' ,\n\
http_password nvarchar2(64) DEFAULT '' ,\n\
hostid number(20)  NOT NULL,\n\
templateid number(20)  NULL,\n\
http_proxy nvarchar2(255) DEFAULT '' ,\n\
retries number(10) DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (httptestid)\n\
);\n\
CREATE INDEX httptest_1 ON httptest (applicationid);\n\
CREATE UNIQUE INDEX httptest_2 ON httptest (hostid,name);\n\
CREATE INDEX httptest_3 ON httptest (status);\n\
CREATE INDEX httptest_4 ON httptest (templateid);\n\
CREATE TABLE httpstep (\n\
httpstepid number(20)  NOT NULL,\n\
httptestid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
no number(10) DEFAULT '0' NOT NULL,\n\
url nvarchar2(255) DEFAULT '' ,\n\
timeout number(10) DEFAULT '30' NOT NULL,\n\
posts nvarchar2(2048) DEFAULT '' ,\n\
required nvarchar2(255) DEFAULT '' ,\n\
status_codes nvarchar2(255) DEFAULT '' ,\n\
variables nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (httpstepid)\n\
);\n\
CREATE INDEX httpstep_1 ON httpstep (httptestid);\n\
CREATE TABLE interface (\n\
interfaceid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
main number(10) DEFAULT '0' NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
useip number(10) DEFAULT '1' NOT NULL,\n\
ip nvarchar2(64) DEFAULT '127.0.0.1' ,\n\
dns nvarchar2(64) DEFAULT '' ,\n\
port nvarchar2(64) DEFAULT '10050' ,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE INDEX interface_1 ON interface (hostid,type);\n\
CREATE INDEX interface_2 ON interface (ip,dns);\n\
CREATE TABLE valuemaps (\n\
valuemapid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
PRIMARY KEY (valuemapid)\n\
);\n\
CREATE INDEX valuemaps_1 ON valuemaps (name);\n\
CREATE TABLE items (\n\
itemid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
snmp_community nvarchar2(64) DEFAULT '' ,\n\
snmp_oid nvarchar2(255) DEFAULT '' ,\n\
hostid number(20)  NOT NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
key_ nvarchar2(255) DEFAULT '' ,\n\
delay number(10) DEFAULT '0' NOT NULL,\n\
history number(10) DEFAULT '90' NOT NULL,\n\
trends number(10) DEFAULT '365' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
value_type number(10) DEFAULT '0' NOT NULL,\n\
trapper_hosts nvarchar2(255) DEFAULT '' ,\n\
units nvarchar2(255) DEFAULT '' ,\n\
multiplier number(10) DEFAULT '0' NOT NULL,\n\
delta number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_securityname nvarchar2(64) DEFAULT '' ,\n\
snmpv3_securitylevel number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase nvarchar2(64) DEFAULT '' ,\n\
snmpv3_privpassphrase nvarchar2(64) DEFAULT '' ,\n\
formula nvarchar2(255) DEFAULT '1' ,\n\
error nvarchar2(128) DEFAULT '' ,\n\
lastlogsize number(20) DEFAULT '0' NOT NULL,\n\
logtimefmt nvarchar2(64) DEFAULT '' ,\n\
templateid number(20)  NULL,\n\
valuemapid number(20)  NULL,\n\
delay_flex nvarchar2(255) DEFAULT '' ,\n\
params nvarchar2(2048) DEFAULT '' ,\n\
ipmi_sensor nvarchar2(128) DEFAULT '' ,\n\
data_type number(10) DEFAULT '0' NOT NULL,\n\
authtype number(10) DEFAULT '0' NOT NULL,\n\
username nvarchar2(64) DEFAULT '' ,\n\
password nvarchar2(64) DEFAULT '' ,\n\
publickey nvarchar2(64) DEFAULT '' ,\n\
privatekey nvarchar2(64) DEFAULT '' ,\n\
mtime number(10) DEFAULT '0' NOT NULL,\n\
flags number(10) DEFAULT '0' NOT NULL,\n\
filter nvarchar2(255) DEFAULT '' ,\n\
interfaceid number(20)  NULL,\n\
port nvarchar2(64) DEFAULT '' ,\n\
description nvarchar2(2048) DEFAULT '' ,\n\
inventory_link number(10) DEFAULT '0' NOT NULL,\n\
lifetime nvarchar2(64) DEFAULT '30' ,\n\
snmpv3_authprotocol number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol number(10) DEFAULT '0' NOT NULL,\n\
state number(10) DEFAULT '0' NOT NULL,\n\
snmpv3_contextname nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (itemid)\n\
);\n\
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);\n\
CREATE INDEX items_3 ON items (status);\n\
CREATE INDEX items_4 ON items (templateid);\n\
CREATE INDEX items_5 ON items (valuemapid);\n\
CREATE INDEX items_6 ON items (interfaceid);\n\
CREATE TABLE httpstepitem (\n\
httpstepitemid number(20)  NOT NULL,\n\
httpstepid number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httpstepitemid)\n\
);\n\
CREATE UNIQUE INDEX httpstepitem_1 ON httpstepitem (httpstepid,itemid);\n\
CREATE INDEX httpstepitem_2 ON httpstepitem (itemid);\n\
CREATE TABLE httptestitem (\n\
httptestitemid number(20)  NOT NULL,\n\
httptestid number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httptestitemid)\n\
);\n\
CREATE UNIQUE INDEX httptestitem_1 ON httptestitem (httptestid,itemid);\n\
CREATE INDEX httptestitem_2 ON httptestitem (itemid);\n\
CREATE TABLE media_type (\n\
mediatypeid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
description nvarchar2(100) DEFAULT '' ,\n\
smtp_server nvarchar2(255) DEFAULT '' ,\n\
smtp_helo nvarchar2(255) DEFAULT '' ,\n\
smtp_email nvarchar2(255) DEFAULT '' ,\n\
exec_path nvarchar2(255) DEFAULT '' ,\n\
gsm_modem nvarchar2(255) DEFAULT '' ,\n\
username nvarchar2(255) DEFAULT '' ,\n\
passwd nvarchar2(255) DEFAULT '' ,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (mediatypeid)\n\
);\n\
CREATE TABLE users (\n\
userid number(20)  NOT NULL,\n\
alias nvarchar2(100) DEFAULT '' ,\n\
name nvarchar2(100) DEFAULT '' ,\n\
surname nvarchar2(100) DEFAULT '' ,\n\
passwd nvarchar2(32) DEFAULT '' ,\n\
url nvarchar2(255) DEFAULT '' ,\n\
autologin number(10) DEFAULT '0' NOT NULL,\n\
autologout number(10) DEFAULT '900' NOT NULL,\n\
lang nvarchar2(5) DEFAULT 'en_GB' ,\n\
refresh number(10) DEFAULT '30' NOT NULL,\n\
type number(10) DEFAULT '1' NOT NULL,\n\
theme nvarchar2(128) DEFAULT 'default' ,\n\
attempt_failed number(10) DEFAULT 0 NOT NULL,\n\
attempt_ip nvarchar2(39) DEFAULT '' ,\n\
attempt_clock number(10) DEFAULT 0 NOT NULL,\n\
rows_per_page number(10) DEFAULT 50 NOT NULL,\n\
PRIMARY KEY (userid)\n\
);\n\
CREATE INDEX users_1 ON users (alias);\n\
CREATE TABLE usrgrp (\n\
usrgrpid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
gui_access number(10) DEFAULT '0' NOT NULL,\n\
users_status number(10) DEFAULT '0' NOT NULL,\n\
debug_mode number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (usrgrpid)\n\
);\n\
CREATE INDEX usrgrp_1 ON usrgrp (name);\n\
CREATE TABLE users_groups (\n\
id number(20)  NOT NULL,\n\
usrgrpid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);\n\
CREATE INDEX users_groups_2 ON users_groups (userid);\n\
CREATE TABLE scripts (\n\
scriptid number(20)  NOT NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
command nvarchar2(255) DEFAULT '' ,\n\
host_access number(10) DEFAULT '2' NOT NULL,\n\
usrgrpid number(20)  NULL,\n\
groupid number(20)  NULL,\n\
description nvarchar2(2048) DEFAULT '' ,\n\
confirmation nvarchar2(255) DEFAULT '' ,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
execute_on number(10) DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (scriptid)\n\
);\n\
CREATE INDEX scripts_1 ON scripts (usrgrpid);\n\
CREATE INDEX scripts_2 ON scripts (groupid);\n\
CREATE TABLE actions (\n\
actionid number(20)  NOT NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
eventsource number(10) DEFAULT '0' NOT NULL,\n\
evaltype number(10) DEFAULT '0' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
esc_period number(10) DEFAULT '0' NOT NULL,\n\
def_shortdata nvarchar2(255) DEFAULT '' ,\n\
def_longdata nvarchar2(2048) DEFAULT '' ,\n\
recovery_msg number(10) DEFAULT '0' NOT NULL,\n\
r_shortdata nvarchar2(255) DEFAULT '' ,\n\
r_longdata nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (actionid)\n\
);\n\
CREATE INDEX actions_1 ON actions (eventsource,status);\n\
CREATE TABLE operations (\n\
operationid number(20)  NOT NULL,\n\
actionid number(20)  NOT NULL,\n\
operationtype number(10) DEFAULT '0' NOT NULL,\n\
esc_period number(10) DEFAULT '0' NOT NULL,\n\
esc_step_from number(10) DEFAULT '1' NOT NULL,\n\
esc_step_to number(10) DEFAULT '1' NOT NULL,\n\
evaltype number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX operations_1 ON operations (actionid);\n\
CREATE TABLE opmessage (\n\
operationid number(20)  NOT NULL,\n\
default_msg number(10) DEFAULT '0' NOT NULL,\n\
subject nvarchar2(255) DEFAULT '' ,\n\
message nvarchar2(2048) DEFAULT '' ,\n\
mediatypeid number(20)  NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opmessage_1 ON opmessage (mediatypeid);\n\
CREATE TABLE opmessage_grp (\n\
opmessage_grpid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
usrgrpid number(20)  NOT NULL,\n\
PRIMARY KEY (opmessage_grpid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);\n\
CREATE INDEX opmessage_grp_2 ON opmessage_grp (usrgrpid);\n\
CREATE TABLE opmessage_usr (\n\
opmessage_usrid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
PRIMARY KEY (opmessage_usrid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);\n\
CREATE INDEX opmessage_usr_2 ON opmessage_usr (userid);\n\
CREATE TABLE opcommand (\n\
operationid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
scriptid number(20)  NULL,\n\
execute_on number(10) DEFAULT '0' NOT NULL,\n\
port nvarchar2(64) DEFAULT '' ,\n\
authtype number(10) DEFAULT '0' NOT NULL,\n\
username nvarchar2(64) DEFAULT '' ,\n\
password nvarchar2(64) DEFAULT '' ,\n\
publickey nvarchar2(64) DEFAULT '' ,\n\
privatekey nvarchar2(64) DEFAULT '' ,\n\
command nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opcommand_1 ON opcommand (scriptid);\n\
CREATE TABLE opcommand_hst (\n\
opcommand_hstid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
hostid number(20)  NULL,\n\
PRIMARY KEY (opcommand_hstid)\n\
);\n\
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);\n\
CREATE INDEX opcommand_hst_2 ON opcommand_hst (hostid);\n\
CREATE TABLE opcommand_grp (\n\
opcommand_grpid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
groupid number(20)  NOT NULL,\n\
PRIMARY KEY (opcommand_grpid)\n\
);\n\
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);\n\
CREATE INDEX opcommand_grp_2 ON opcommand_grp (groupid);\n\
CREATE TABLE opgroup (\n\
opgroupid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
groupid number(20)  NOT NULL,\n\
PRIMARY KEY (opgroupid)\n\
);\n\
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);\n\
CREATE INDEX opgroup_2 ON opgroup (groupid);\n\
CREATE TABLE optemplate (\n\
optemplateid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
templateid number(20)  NOT NULL,\n\
PRIMARY KEY (optemplateid)\n\
);\n\
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);\n\
CREATE INDEX optemplate_2 ON optemplate (templateid);\n\
CREATE TABLE opconditions (\n\
opconditionid number(20)  NOT NULL,\n\
operationid number(20)  NOT NULL,\n\
conditiontype number(10) DEFAULT '0' NOT NULL,\n\
operator number(10) DEFAULT '0' NOT NULL,\n\
value nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (opconditionid)\n\
);\n\
CREATE INDEX opconditions_1 ON opconditions (operationid);\n\
CREATE TABLE conditions (\n\
conditionid number(20)  NOT NULL,\n\
actionid number(20)  NOT NULL,\n\
conditiontype number(10) DEFAULT '0' NOT NULL,\n\
operator number(10) DEFAULT '0' NOT NULL,\n\
value nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (conditionid)\n\
);\n\
CREATE INDEX conditions_1 ON conditions (actionid);\n\
CREATE TABLE config (\n\
configid number(20)  NOT NULL,\n\
refresh_unsupported number(10) DEFAULT '0' NOT NULL,\n\
work_period nvarchar2(100) DEFAULT '1-5,00:00-24:00' ,\n\
alert_usrgrpid number(20)  NULL,\n\
event_ack_enable number(10) DEFAULT '1' NOT NULL,\n\
event_expire number(10) DEFAULT '7' NOT NULL,\n\
event_show_max number(10) DEFAULT '100' NOT NULL,\n\
default_theme nvarchar2(128) DEFAULT 'originalblue' ,\n\
authentication_type number(10) DEFAULT '0' NOT NULL,\n\
ldap_host nvarchar2(255) DEFAULT '' ,\n\
ldap_port number(10) DEFAULT 389 NOT NULL,\n\
ldap_base_dn nvarchar2(255) DEFAULT '' ,\n\
ldap_bind_dn nvarchar2(255) DEFAULT '' ,\n\
ldap_bind_password nvarchar2(128) DEFAULT '' ,\n\
ldap_search_attribute nvarchar2(128) DEFAULT '' ,\n\
dropdown_first_entry number(10) DEFAULT '1' NOT NULL,\n\
dropdown_first_remember number(10) DEFAULT '1' NOT NULL,\n\
discovery_groupid number(20)  NOT NULL,\n\
max_in_table number(10) DEFAULT '50' NOT NULL,\n\
search_limit number(10) DEFAULT '1000' NOT NULL,\n\
severity_color_0 nvarchar2(6) DEFAULT 'DBDBDB' ,\n\
severity_color_1 nvarchar2(6) DEFAULT 'D6F6FF' ,\n\
severity_color_2 nvarchar2(6) DEFAULT 'FFF6A5' ,\n\
severity_color_3 nvarchar2(6) DEFAULT 'FFB689' ,\n\
severity_color_4 nvarchar2(6) DEFAULT 'FF9999' ,\n\
severity_color_5 nvarchar2(6) DEFAULT 'FF3838' ,\n\
severity_name_0 nvarchar2(32) DEFAULT 'Not classified' ,\n\
severity_name_1 nvarchar2(32) DEFAULT 'Information' ,\n\
severity_name_2 nvarchar2(32) DEFAULT 'Warning' ,\n\
severity_name_3 nvarchar2(32) DEFAULT 'Average' ,\n\
severity_name_4 nvarchar2(32) DEFAULT 'High' ,\n\
severity_name_5 nvarchar2(32) DEFAULT 'Disaster' ,\n\
ok_period number(10) DEFAULT '1800' NOT NULL,\n\
blink_period number(10) DEFAULT '1800' NOT NULL,\n\
problem_unack_color nvarchar2(6) DEFAULT 'DC0000' ,\n\
problem_ack_color nvarchar2(6) DEFAULT 'DC0000' ,\n\
ok_unack_color nvarchar2(6) DEFAULT '00AA00' ,\n\
ok_ack_color nvarchar2(6) DEFAULT '00AA00' ,\n\
problem_unack_style number(10) DEFAULT '1' NOT NULL,\n\
problem_ack_style number(10) DEFAULT '1' NOT NULL,\n\
ok_unack_style number(10) DEFAULT '1' NOT NULL,\n\
ok_ack_style number(10) DEFAULT '1' NOT NULL,\n\
snmptrap_logging number(10) DEFAULT '1' NOT NULL,\n\
server_check_interval number(10) DEFAULT '10' NOT NULL,\n\
hk_events_mode number(10) DEFAULT '1' NOT NULL,\n\
hk_events_trigger number(10) DEFAULT '365' NOT NULL,\n\
hk_events_internal number(10) DEFAULT '365' NOT NULL,\n\
hk_events_discovery number(10) DEFAULT '365' NOT NULL,\n\
hk_events_autoreg number(10) DEFAULT '365' NOT NULL,\n\
hk_services_mode number(10) DEFAULT '1' NOT NULL,\n\
hk_services number(10) DEFAULT '365' NOT NULL,\n\
hk_audit_mode number(10) DEFAULT '1' NOT NULL,\n\
hk_audit number(10) DEFAULT '365' NOT NULL,\n\
hk_sessions_mode number(10) DEFAULT '1' NOT NULL,\n\
hk_sessions number(10) DEFAULT '365' NOT NULL,\n\
hk_history_mode number(10) DEFAULT '1' NOT NULL,\n\
hk_history_global number(10) DEFAULT '0' NOT NULL,\n\
hk_history number(10) DEFAULT '90' NOT NULL,\n\
hk_trends_mode number(10) DEFAULT '1' NOT NULL,\n\
hk_trends_global number(10) DEFAULT '0' NOT NULL,\n\
hk_trends number(10) DEFAULT '365' NOT NULL,\n\
PRIMARY KEY (configid)\n\
);\n\
CREATE INDEX config_1 ON config (alert_usrgrpid);\n\
CREATE INDEX config_2 ON config (discovery_groupid);\n\
CREATE TABLE triggers (\n\
triggerid number(20)  NOT NULL,\n\
expression nvarchar2(2048) DEFAULT '' ,\n\
description nvarchar2(255) DEFAULT '' ,\n\
url nvarchar2(255) DEFAULT '' ,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
value number(10) DEFAULT '0' NOT NULL,\n\
priority number(10) DEFAULT '0' NOT NULL,\n\
lastchange number(10) DEFAULT '0' NOT NULL,\n\
comments nvarchar2(2048) DEFAULT '' ,\n\
error nvarchar2(128) DEFAULT '' ,\n\
templateid number(20)  NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
state number(10) DEFAULT '0' NOT NULL,\n\
flags number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (triggerid)\n\
);\n\
CREATE INDEX triggers_1 ON triggers (status);\n\
CREATE INDEX triggers_2 ON triggers (value);\n\
CREATE INDEX triggers_3 ON triggers (templateid);\n\
CREATE TABLE trigger_depends (\n\
triggerdepid number(20)  NOT NULL,\n\
triggerid_down number(20)  NOT NULL,\n\
triggerid_up number(20)  NOT NULL,\n\
PRIMARY KEY (triggerdepid)\n\
);\n\
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);\n\
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);\n\
CREATE TABLE functions (\n\
functionid number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
triggerid number(20)  NOT NULL,\n\
function nvarchar2(12) DEFAULT '' ,\n\
parameter nvarchar2(255) DEFAULT '0' ,\n\
PRIMARY KEY (functionid)\n\
);\n\
CREATE INDEX functions_1 ON functions (triggerid);\n\
CREATE INDEX functions_2 ON functions (itemid,function,parameter);\n\
CREATE TABLE graphs (\n\
graphid number(20)  NOT NULL,\n\
name nvarchar2(128) DEFAULT '' ,\n\
width number(10) DEFAULT '900' NOT NULL,\n\
height number(10) DEFAULT '200' NOT NULL,\n\
yaxismin number(20,4) DEFAULT '0' NOT NULL,\n\
yaxismax number(20,4) DEFAULT '100' NOT NULL,\n\
templateid number(20)  NULL,\n\
show_work_period number(10) DEFAULT '1' NOT NULL,\n\
show_triggers number(10) DEFAULT '1' NOT NULL,\n\
graphtype number(10) DEFAULT '0' NOT NULL,\n\
show_legend number(10) DEFAULT '1' NOT NULL,\n\
show_3d number(10) DEFAULT '0' NOT NULL,\n\
percent_left number(20,4) DEFAULT '0' NOT NULL,\n\
percent_right number(20,4) DEFAULT '0' NOT NULL,\n\
ymin_type number(10) DEFAULT '0' NOT NULL,\n\
ymax_type number(10) DEFAULT '0' NOT NULL,\n\
ymin_itemid number(20)  NULL,\n\
ymax_itemid number(20)  NULL,\n\
flags number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (graphid)\n\
);\n\
CREATE INDEX graphs_1 ON graphs (name);\n\
CREATE INDEX graphs_2 ON graphs (templateid);\n\
CREATE INDEX graphs_3 ON graphs (ymin_itemid);\n\
CREATE INDEX graphs_4 ON graphs (ymax_itemid);\n\
CREATE TABLE graphs_items (\n\
gitemid number(20)  NOT NULL,\n\
graphid number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
drawtype number(10) DEFAULT '0' NOT NULL,\n\
sortorder number(10) DEFAULT '0' NOT NULL,\n\
color nvarchar2(6) DEFAULT '009600' ,\n\
yaxisside number(10) DEFAULT '0' NOT NULL,\n\
calc_fnc number(10) DEFAULT '2' NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (gitemid)\n\
);\n\
CREATE INDEX graphs_items_1 ON graphs_items (itemid);\n\
CREATE INDEX graphs_items_2 ON graphs_items (graphid);\n\
CREATE TABLE graph_theme (\n\
graphthemeid number(20)  NOT NULL,\n\
description nvarchar2(64) DEFAULT '' ,\n\
theme nvarchar2(64) DEFAULT '' ,\n\
backgroundcolor nvarchar2(6) DEFAULT 'F0F0F0' ,\n\
graphcolor nvarchar2(6) DEFAULT 'FFFFFF' ,\n\
graphbordercolor nvarchar2(6) DEFAULT '222222' ,\n\
gridcolor nvarchar2(6) DEFAULT 'CCCCCC' ,\n\
maingridcolor nvarchar2(6) DEFAULT 'AAAAAA' ,\n\
gridbordercolor nvarchar2(6) DEFAULT '000000' ,\n\
textcolor nvarchar2(6) DEFAULT '202020' ,\n\
highlightcolor nvarchar2(6) DEFAULT 'AA4444' ,\n\
leftpercentilecolor nvarchar2(6) DEFAULT '11CC11' ,\n\
rightpercentilecolor nvarchar2(6) DEFAULT 'CC1111' ,\n\
nonworktimecolor nvarchar2(6) DEFAULT 'CCCCCC' ,\n\
gridview number(10) DEFAULT 1 NOT NULL,\n\
legendview number(10) DEFAULT 1 NOT NULL,\n\
PRIMARY KEY (graphthemeid)\n\
);\n\
CREATE INDEX graph_theme_1 ON graph_theme (description);\n\
CREATE INDEX graph_theme_2 ON graph_theme (theme);\n\
CREATE TABLE globalmacro (\n\
globalmacroid number(20)  NOT NULL,\n\
macro nvarchar2(64) DEFAULT '' ,\n\
value nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (globalmacroid)\n\
);\n\
CREATE INDEX globalmacro_1 ON globalmacro (macro);\n\
CREATE TABLE hostmacro (\n\
hostmacroid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
macro nvarchar2(64) DEFAULT '' ,\n\
value nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (hostmacroid)\n\
);\n\
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);\n\
CREATE TABLE hosts_groups (\n\
hostgroupid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
groupid number(20)  NOT NULL,\n\
PRIMARY KEY (hostgroupid)\n\
);\n\
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);\n\
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);\n\
CREATE TABLE hosts_templates (\n\
hosttemplateid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
templateid number(20)  NOT NULL,\n\
PRIMARY KEY (hosttemplateid)\n\
);\n\
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);\n\
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);\n\
CREATE TABLE items_applications (\n\
itemappid number(20)  NOT NULL,\n\
applicationid number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
PRIMARY KEY (itemappid)\n\
);\n\
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);\n\
CREATE INDEX items_applications_2 ON items_applications (itemid);\n\
CREATE TABLE mappings (\n\
mappingid number(20)  NOT NULL,\n\
valuemapid number(20)  NOT NULL,\n\
value nvarchar2(64) DEFAULT '' ,\n\
newvalue nvarchar2(64) DEFAULT '' ,\n\
PRIMARY KEY (mappingid)\n\
);\n\
CREATE INDEX mappings_1 ON mappings (valuemapid);\n\
CREATE TABLE media (\n\
mediaid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
mediatypeid number(20)  NOT NULL,\n\
sendto nvarchar2(100) DEFAULT '' ,\n\
active number(10) DEFAULT '0' NOT NULL,\n\
severity number(10) DEFAULT '63' NOT NULL,\n\
period nvarchar2(100) DEFAULT '1-7,00:00-24:00' ,\n\
PRIMARY KEY (mediaid)\n\
);\n\
CREATE INDEX media_1 ON media (userid);\n\
CREATE INDEX media_2 ON media (mediatypeid);\n\
CREATE TABLE rights (\n\
rightid number(20)  NOT NULL,\n\
groupid number(20)  NOT NULL,\n\
permission number(10) DEFAULT '0' NOT NULL,\n\
id number(20)  NOT NULL,\n\
PRIMARY KEY (rightid)\n\
);\n\
CREATE INDEX rights_1 ON rights (groupid);\n\
CREATE INDEX rights_2 ON rights (id);\n\
CREATE TABLE services (\n\
serviceid number(20)  NOT NULL,\n\
name nvarchar2(128) DEFAULT '' ,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
algorithm number(10) DEFAULT '0' NOT NULL,\n\
triggerid number(20)  NULL,\n\
showsla number(10) DEFAULT '0' NOT NULL,\n\
goodsla number(20,4) DEFAULT '99.9' NOT NULL,\n\
sortorder number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (serviceid)\n\
);\n\
CREATE INDEX services_1 ON services (triggerid);\n\
CREATE TABLE services_links (\n\
linkid number(20)  NOT NULL,\n\
serviceupid number(20)  NOT NULL,\n\
servicedownid number(20)  NOT NULL,\n\
soft number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX services_links_1 ON services_links (servicedownid);\n\
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);\n\
CREATE TABLE services_times (\n\
timeid number(20)  NOT NULL,\n\
serviceid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
ts_from number(10) DEFAULT '0' NOT NULL,\n\
ts_to number(10) DEFAULT '0' NOT NULL,\n\
note nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (timeid)\n\
);\n\
CREATE INDEX services_times_1 ON services_times (serviceid,type,ts_from,ts_to);\n\
CREATE TABLE icon_map (\n\
iconmapid number(20)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '' ,\n\
default_iconid number(20)  NOT NULL,\n\
PRIMARY KEY (iconmapid)\n\
);\n\
CREATE INDEX icon_map_1 ON icon_map (name);\n\
CREATE INDEX icon_map_2 ON icon_map (default_iconid);\n\
CREATE TABLE icon_mapping (\n\
iconmappingid number(20)  NOT NULL,\n\
iconmapid number(20)  NOT NULL,\n\
iconid number(20)  NOT NULL,\n\
inventory_link number(10) DEFAULT '0' NOT NULL,\n\
expression nvarchar2(64) DEFAULT '' ,\n\
sortorder number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (iconmappingid)\n\
);\n\
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);\n\
CREATE INDEX icon_mapping_2 ON icon_mapping (iconid);\n\
CREATE TABLE sysmaps (\n\
sysmapid number(20)  NOT NULL,\n\
name nvarchar2(128) DEFAULT '' ,\n\
width number(10) DEFAULT '600' NOT NULL,\n\
height number(10) DEFAULT '400' NOT NULL,\n\
backgroundid number(20)  NULL,\n\
label_type number(10) DEFAULT '2' NOT NULL,\n\
label_location number(10) DEFAULT '0' NOT NULL,\n\
highlight number(10) DEFAULT '1' NOT NULL,\n\
expandproblem number(10) DEFAULT '1' NOT NULL,\n\
markelements number(10) DEFAULT '0' NOT NULL,\n\
show_unack number(10) DEFAULT '0' NOT NULL,\n\
grid_size number(10) DEFAULT '50' NOT NULL,\n\
grid_show number(10) DEFAULT '1' NOT NULL,\n\
grid_align number(10) DEFAULT '1' NOT NULL,\n\
label_format number(10) DEFAULT '0' NOT NULL,\n\
label_type_host number(10) DEFAULT '2' NOT NULL,\n\
label_type_hostgroup number(10) DEFAULT '2' NOT NULL,\n\
label_type_trigger number(10) DEFAULT '2' NOT NULL,\n\
label_type_map number(10) DEFAULT '2' NOT NULL,\n\
label_type_image number(10) DEFAULT '2' NOT NULL,\n\
label_string_host nvarchar2(255) DEFAULT '' ,\n\
label_string_hostgroup nvarchar2(255) DEFAULT '' ,\n\
label_string_trigger nvarchar2(255) DEFAULT '' ,\n\
label_string_map nvarchar2(255) DEFAULT '' ,\n\
label_string_image nvarchar2(255) DEFAULT '' ,\n\
iconmapid number(20)  NULL,\n\
expand_macros number(10) DEFAULT '0' NOT NULL,\n\
severity_min number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapid)\n\
);\n\
CREATE INDEX sysmaps_1 ON sysmaps (name);\n\
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);\n\
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);\n\
CREATE TABLE sysmaps_elements (\n\
selementid number(20)  NOT NULL,\n\
sysmapid number(20)  NOT NULL,\n\
elementid number(20) DEFAULT '0' NOT NULL,\n\
elementtype number(10) DEFAULT '0' NOT NULL,\n\
iconid_off number(20)  NULL,\n\
iconid_on number(20)  NULL,\n\
label nvarchar2(2048) DEFAULT '' ,\n\
label_location number(10) DEFAULT '-1' NOT NULL,\n\
x number(10) DEFAULT '0' NOT NULL,\n\
y number(10) DEFAULT '0' NOT NULL,\n\
iconid_disabled number(20)  NULL,\n\
iconid_maintenance number(20)  NULL,\n\
elementsubtype number(10) DEFAULT '0' NOT NULL,\n\
areatype number(10) DEFAULT '0' NOT NULL,\n\
width number(10) DEFAULT '200' NOT NULL,\n\
height number(10) DEFAULT '200' NOT NULL,\n\
viewtype number(10) DEFAULT '0' NOT NULL,\n\
use_iconmap number(10) DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (selementid)\n\
);\n\
CREATE INDEX sysmaps_elements_1 ON sysmaps_elements (sysmapid);\n\
CREATE INDEX sysmaps_elements_2 ON sysmaps_elements (iconid_off);\n\
CREATE INDEX sysmaps_elements_3 ON sysmaps_elements (iconid_on);\n\
CREATE INDEX sysmaps_elements_4 ON sysmaps_elements (iconid_disabled);\n\
CREATE INDEX sysmaps_elements_5 ON sysmaps_elements (iconid_maintenance);\n\
CREATE TABLE sysmaps_links (\n\
linkid number(20)  NOT NULL,\n\
sysmapid number(20)  NOT NULL,\n\
selementid1 number(20)  NOT NULL,\n\
selementid2 number(20)  NOT NULL,\n\
drawtype number(10) DEFAULT '0' NOT NULL,\n\
color nvarchar2(6) DEFAULT '000000' ,\n\
label nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX sysmaps_links_1 ON sysmaps_links (sysmapid);\n\
CREATE INDEX sysmaps_links_2 ON sysmaps_links (selementid1);\n\
CREATE INDEX sysmaps_links_3 ON sysmaps_links (selementid2);\n\
CREATE TABLE sysmaps_link_triggers (\n\
linktriggerid number(20)  NOT NULL,\n\
linkid number(20)  NOT NULL,\n\
triggerid number(20)  NOT NULL,\n\
drawtype number(10) DEFAULT '0' NOT NULL,\n\
color nvarchar2(6) DEFAULT '000000' ,\n\
PRIMARY KEY (linktriggerid)\n\
);\n\
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);\n\
CREATE INDEX sysmaps_link_triggers_2 ON sysmaps_link_triggers (triggerid);\n\
CREATE TABLE sysmap_element_url (\n\
sysmapelementurlid number(20)  NOT NULL,\n\
selementid number(20)  NOT NULL,\n\
name nvarchar2(255)  ,\n\
url nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (sysmapelementurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);\n\
CREATE TABLE sysmap_url (\n\
sysmapurlid number(20)  NOT NULL,\n\
sysmapid number(20)  NOT NULL,\n\
name nvarchar2(255)  ,\n\
url nvarchar2(255) DEFAULT '' ,\n\
elementtype number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);\n\
CREATE TABLE maintenances_hosts (\n\
maintenance_hostid number(20)  NOT NULL,\n\
maintenanceid number(20)  NOT NULL,\n\
hostid number(20)  NOT NULL,\n\
PRIMARY KEY (maintenance_hostid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);\n\
CREATE INDEX maintenances_hosts_2 ON maintenances_hosts (hostid);\n\
CREATE TABLE maintenances_groups (\n\
maintenance_groupid number(20)  NOT NULL,\n\
maintenanceid number(20)  NOT NULL,\n\
groupid number(20)  NOT NULL,\n\
PRIMARY KEY (maintenance_groupid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);\n\
CREATE INDEX maintenances_groups_2 ON maintenances_groups (groupid);\n\
CREATE TABLE timeperiods (\n\
timeperiodid number(20)  NOT NULL,\n\
timeperiod_type number(10) DEFAULT '0' NOT NULL,\n\
every number(10) DEFAULT '0' NOT NULL,\n\
month number(10) DEFAULT '0' NOT NULL,\n\
dayofweek number(10) DEFAULT '0' NOT NULL,\n\
day number(10) DEFAULT '0' NOT NULL,\n\
start_time number(10) DEFAULT '0' NOT NULL,\n\
period number(10) DEFAULT '0' NOT NULL,\n\
start_date number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (timeperiodid)\n\
);\n\
CREATE TABLE maintenances_windows (\n\
maintenance_timeperiodid number(20)  NOT NULL,\n\
maintenanceid number(20)  NOT NULL,\n\
timeperiodid number(20)  NOT NULL,\n\
PRIMARY KEY (maintenance_timeperiodid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);\n\
CREATE INDEX maintenances_windows_2 ON maintenances_windows (timeperiodid);\n\
CREATE TABLE regexps (\n\
regexpid number(20)  NOT NULL,\n\
name nvarchar2(128) DEFAULT '' ,\n\
test_string nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (regexpid)\n\
);\n\
CREATE INDEX regexps_1 ON regexps (name);\n\
CREATE TABLE expressions (\n\
expressionid number(20)  NOT NULL,\n\
regexpid number(20)  NOT NULL,\n\
expression nvarchar2(255) DEFAULT '' ,\n\
expression_type number(10) DEFAULT '0' NOT NULL,\n\
exp_delimiter nvarchar2(1) DEFAULT '' ,\n\
case_sensitive number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (expressionid)\n\
);\n\
CREATE INDEX expressions_1 ON expressions (regexpid);\n\
CREATE TABLE nodes (\n\
nodeid number(10)  NOT NULL,\n\
name nvarchar2(64) DEFAULT '0' ,\n\
ip nvarchar2(39) DEFAULT '' ,\n\
port number(10) DEFAULT '10051' NOT NULL,\n\
nodetype number(10) DEFAULT '0' NOT NULL,\n\
masterid number(10)  NULL,\n\
PRIMARY KEY (nodeid)\n\
);\n\
CREATE INDEX nodes_1 ON nodes (masterid);\n\
CREATE TABLE node_cksum (\n\
nodeid number(10)  NOT NULL,\n\
tablename nvarchar2(64) DEFAULT '' ,\n\
recordid number(20)  NOT NULL,\n\
cksumtype number(10) DEFAULT '0' NOT NULL,\n\
cksum nclob DEFAULT '' ,\n\
sync nvarchar2(128) DEFAULT '' \n\
);\n\
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);\n\
CREATE TABLE ids (\n\
nodeid number(10)  NOT NULL,\n\
table_name nvarchar2(64) DEFAULT '' ,\n\
field_name nvarchar2(64) DEFAULT '' ,\n\
nextid number(20)  NOT NULL,\n\
PRIMARY KEY (nodeid,table_name,field_name)\n\
);\n\
CREATE TABLE alerts (\n\
alertid number(20)  NOT NULL,\n\
actionid number(20)  NOT NULL,\n\
eventid number(20)  NOT NULL,\n\
userid number(20)  NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
mediatypeid number(20)  NULL,\n\
sendto nvarchar2(100) DEFAULT '' ,\n\
subject nvarchar2(255) DEFAULT '' ,\n\
message nclob DEFAULT '' ,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
retries number(10) DEFAULT '0' NOT NULL,\n\
error nvarchar2(128) DEFAULT '' ,\n\
esc_step number(10) DEFAULT '0' NOT NULL,\n\
alerttype number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (alertid)\n\
);\n\
CREATE INDEX alerts_1 ON alerts (actionid);\n\
CREATE INDEX alerts_2 ON alerts (clock);\n\
CREATE INDEX alerts_3 ON alerts (eventid);\n\
CREATE INDEX alerts_4 ON alerts (status,retries);\n\
CREATE INDEX alerts_5 ON alerts (mediatypeid);\n\
CREATE INDEX alerts_6 ON alerts (userid);\n\
CREATE TABLE history (\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value number(20,4) DEFAULT '0.0000' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_1 ON history (itemid,clock);\n\
CREATE TABLE history_sync (\n\
id number(20)  NOT NULL,\n\
nodeid number(10)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value number(20,4) DEFAULT '0.0000' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_sync_1 ON history_sync (nodeid,id);\n\
CREATE TABLE history_uint (\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value number(20) DEFAULT '0' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);\n\
CREATE TABLE history_uint_sync (\n\
id number(20)  NOT NULL,\n\
nodeid number(10)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value number(20) DEFAULT '0' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_uint_sync_1 ON history_uint_sync (nodeid,id);\n\
CREATE TABLE history_str (\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value nvarchar2(255) DEFAULT '' ,\n\
ns number(10) DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_str_1 ON history_str (itemid,clock);\n\
CREATE TABLE history_str_sync (\n\
id number(20)  NOT NULL,\n\
nodeid number(10)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value nvarchar2(255) DEFAULT '' ,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_str_sync_1 ON history_str_sync (nodeid,id);\n\
CREATE TABLE history_log (\n\
id number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
timestamp number(10) DEFAULT '0' NOT NULL,\n\
source nvarchar2(64) DEFAULT '' ,\n\
severity number(10) DEFAULT '0' NOT NULL,\n\
value nclob DEFAULT '' ,\n\
logeventid number(10) DEFAULT '0' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_log_1 ON history_log (itemid,clock);\n\
CREATE UNIQUE INDEX history_log_2 ON history_log (itemid,id);\n\
CREATE TABLE history_text (\n\
id number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value nclob DEFAULT '' ,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_text_1 ON history_text (itemid,clock);\n\
CREATE UNIQUE INDEX history_text_2 ON history_text (itemid,id);\n\
CREATE TABLE proxy_history (\n\
id number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
timestamp number(10) DEFAULT '0' NOT NULL,\n\
source nvarchar2(64) DEFAULT '' ,\n\
severity number(10) DEFAULT '0' NOT NULL,\n\
value nclob DEFAULT '' ,\n\
logeventid number(10) DEFAULT '0' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
state number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_history_1 ON proxy_history (clock);\n\
CREATE TABLE proxy_dhistory (\n\
id number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
druleid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
ip nvarchar2(39) DEFAULT '' ,\n\
port number(10) DEFAULT '0' NOT NULL,\n\
key_ nvarchar2(255) DEFAULT '' ,\n\
value nvarchar2(255) DEFAULT '' ,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
dcheckid number(20)  NULL,\n\
dns nvarchar2(64) DEFAULT '' ,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);\n\
CREATE TABLE events (\n\
eventid number(20)  NOT NULL,\n\
source number(10) DEFAULT '0' NOT NULL,\n\
object number(10) DEFAULT '0' NOT NULL,\n\
objectid number(20) DEFAULT '0' NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value number(10) DEFAULT '0' NOT NULL,\n\
acknowledged number(10) DEFAULT '0' NOT NULL,\n\
ns number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (eventid)\n\
);\n\
CREATE INDEX events_1 ON events (source,object,objectid,clock);\n\
CREATE INDEX events_2 ON events (source,object,clock);\n\
CREATE TABLE trends (\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
num number(10) DEFAULT '0' NOT NULL,\n\
value_min number(20,4) DEFAULT '0.0000' NOT NULL,\n\
value_avg number(20,4) DEFAULT '0.0000' NOT NULL,\n\
value_max number(20,4) DEFAULT '0.0000' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE trends_uint (\n\
itemid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
num number(10) DEFAULT '0' NOT NULL,\n\
value_min number(20) DEFAULT '0' NOT NULL,\n\
value_avg number(20) DEFAULT '0' NOT NULL,\n\
value_max number(20) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE acknowledges (\n\
acknowledgeid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
eventid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
message nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (acknowledgeid)\n\
);\n\
CREATE INDEX acknowledges_1 ON acknowledges (userid);\n\
CREATE INDEX acknowledges_2 ON acknowledges (eventid);\n\
CREATE INDEX acknowledges_3 ON acknowledges (clock);\n\
CREATE TABLE auditlog (\n\
auditid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
action number(10) DEFAULT '0' NOT NULL,\n\
resourcetype number(10) DEFAULT '0' NOT NULL,\n\
details nvarchar2(128)  DEFAULT '0' ,\n\
ip nvarchar2(39) DEFAULT '' ,\n\
resourceid number(20) DEFAULT '0' NOT NULL,\n\
resourcename nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (auditid)\n\
);\n\
CREATE INDEX auditlog_1 ON auditlog (userid,clock);\n\
CREATE INDEX auditlog_2 ON auditlog (clock);\n\
CREATE TABLE auditlog_details (\n\
auditdetailid number(20)  NOT NULL,\n\
auditid number(20)  NOT NULL,\n\
table_name nvarchar2(64) DEFAULT '' ,\n\
field_name nvarchar2(64) DEFAULT '' ,\n\
oldvalue nvarchar2(2048) DEFAULT '' ,\n\
newvalue nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (auditdetailid)\n\
);\n\
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);\n\
CREATE TABLE service_alarms (\n\
servicealarmid number(20)  NOT NULL,\n\
serviceid number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
value number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (servicealarmid)\n\
);\n\
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);\n\
CREATE INDEX service_alarms_2 ON service_alarms (clock);\n\
CREATE TABLE autoreg_host (\n\
autoreg_hostid number(20)  NOT NULL,\n\
proxy_hostid number(20)  NULL,\n\
host nvarchar2(64) DEFAULT '' ,\n\
listen_ip nvarchar2(39) DEFAULT '' ,\n\
listen_port number(10) DEFAULT '0' NOT NULL,\n\
listen_dns nvarchar2(64) DEFAULT '' ,\n\
host_metadata nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (autoreg_hostid)\n\
);\n\
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);\n\
CREATE TABLE proxy_autoreg_host (\n\
id number(20)  NOT NULL,\n\
clock number(10) DEFAULT '0' NOT NULL,\n\
host nvarchar2(64) DEFAULT '' ,\n\
listen_ip nvarchar2(39) DEFAULT '' ,\n\
listen_port number(10) DEFAULT '0' NOT NULL,\n\
listen_dns nvarchar2(64) DEFAULT '' ,\n\
host_metadata nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);\n\
CREATE TABLE dhosts (\n\
dhostid number(20)  NOT NULL,\n\
druleid number(20)  NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
lastup number(10) DEFAULT '0' NOT NULL,\n\
lastdown number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (dhostid)\n\
);\n\
CREATE INDEX dhosts_1 ON dhosts (druleid);\n\
CREATE TABLE dservices (\n\
dserviceid number(20)  NOT NULL,\n\
dhostid number(20)  NOT NULL,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
key_ nvarchar2(255) DEFAULT '' ,\n\
value nvarchar2(255) DEFAULT '' ,\n\
port number(10) DEFAULT '0' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
lastup number(10) DEFAULT '0' NOT NULL,\n\
lastdown number(10) DEFAULT '0' NOT NULL,\n\
dcheckid number(20)  NOT NULL,\n\
ip nvarchar2(39) DEFAULT '' ,\n\
dns nvarchar2(64) DEFAULT '' ,\n\
PRIMARY KEY (dserviceid)\n\
);\n\
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,type,key_,ip,port);\n\
CREATE INDEX dservices_2 ON dservices (dhostid);\n\
CREATE TABLE escalations (\n\
escalationid number(20)  NOT NULL,\n\
actionid number(20)  NOT NULL,\n\
triggerid number(20)  NULL,\n\
eventid number(20)  NULL,\n\
r_eventid number(20)  NULL,\n\
nextcheck number(10) DEFAULT '0' NOT NULL,\n\
esc_step number(10) DEFAULT '0' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
itemid number(20)  NULL,\n\
PRIMARY KEY (escalationid)\n\
);\n\
CREATE UNIQUE INDEX escalations_1 ON escalations (actionid,triggerid,itemid,escalationid);\n\
CREATE TABLE globalvars (\n\
globalvarid number(20)  NOT NULL,\n\
snmp_lastsize number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (globalvarid)\n\
);\n\
CREATE TABLE graph_discovery (\n\
graphdiscoveryid number(20)  NOT NULL,\n\
graphid number(20)  NOT NULL,\n\
parent_graphid number(20)  NOT NULL,\n\
name nvarchar2(128) DEFAULT '' ,\n\
PRIMARY KEY (graphdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX graph_discovery_1 ON graph_discovery (graphid,parent_graphid);\n\
CREATE INDEX graph_discovery_2 ON graph_discovery (parent_graphid);\n\
CREATE TABLE host_inventory (\n\
hostid number(20)  NOT NULL,\n\
inventory_mode number(10) DEFAULT '0' NOT NULL,\n\
type nvarchar2(64) DEFAULT '' ,\n\
type_full nvarchar2(64) DEFAULT '' ,\n\
name nvarchar2(64) DEFAULT '' ,\n\
alias nvarchar2(64) DEFAULT '' ,\n\
os nvarchar2(64) DEFAULT '' ,\n\
os_full nvarchar2(255) DEFAULT '' ,\n\
os_short nvarchar2(64) DEFAULT '' ,\n\
serialno_a nvarchar2(64) DEFAULT '' ,\n\
serialno_b nvarchar2(64) DEFAULT '' ,\n\
tag nvarchar2(64) DEFAULT '' ,\n\
asset_tag nvarchar2(64) DEFAULT '' ,\n\
macaddress_a nvarchar2(64) DEFAULT '' ,\n\
macaddress_b nvarchar2(64) DEFAULT '' ,\n\
hardware nvarchar2(255) DEFAULT '' ,\n\
hardware_full nvarchar2(2048) DEFAULT '' ,\n\
software nvarchar2(255) DEFAULT '' ,\n\
software_full nvarchar2(2048) DEFAULT '' ,\n\
software_app_a nvarchar2(64) DEFAULT '' ,\n\
software_app_b nvarchar2(64) DEFAULT '' ,\n\
software_app_c nvarchar2(64) DEFAULT '' ,\n\
software_app_d nvarchar2(64) DEFAULT '' ,\n\
software_app_e nvarchar2(64) DEFAULT '' ,\n\
contact nvarchar2(2048) DEFAULT '' ,\n\
location nvarchar2(2048) DEFAULT '' ,\n\
location_lat nvarchar2(16) DEFAULT '' ,\n\
location_lon nvarchar2(16) DEFAULT '' ,\n\
notes nvarchar2(2048) DEFAULT '' ,\n\
chassis nvarchar2(64) DEFAULT '' ,\n\
model nvarchar2(64) DEFAULT '' ,\n\
hw_arch nvarchar2(32) DEFAULT '' ,\n\
vendor nvarchar2(64) DEFAULT '' ,\n\
contract_number nvarchar2(64) DEFAULT '' ,\n\
installer_name nvarchar2(64) DEFAULT '' ,\n\
deployment_status nvarchar2(64) DEFAULT '' ,\n\
url_a nvarchar2(255) DEFAULT '' ,\n\
url_b nvarchar2(255) DEFAULT '' ,\n\
url_c nvarchar2(255) DEFAULT '' ,\n\
host_networks nvarchar2(2048) DEFAULT '' ,\n\
host_netmask nvarchar2(39) DEFAULT '' ,\n\
host_router nvarchar2(39) DEFAULT '' ,\n\
oob_ip nvarchar2(39) DEFAULT '' ,\n\
oob_netmask nvarchar2(39) DEFAULT '' ,\n\
oob_router nvarchar2(39) DEFAULT '' ,\n\
date_hw_purchase nvarchar2(64) DEFAULT '' ,\n\
date_hw_install nvarchar2(64) DEFAULT '' ,\n\
date_hw_expiry nvarchar2(64) DEFAULT '' ,\n\
date_hw_decomm nvarchar2(64) DEFAULT '' ,\n\
site_address_a nvarchar2(128) DEFAULT '' ,\n\
site_address_b nvarchar2(128) DEFAULT '' ,\n\
site_address_c nvarchar2(128) DEFAULT '' ,\n\
site_city nvarchar2(128) DEFAULT '' ,\n\
site_state nvarchar2(64) DEFAULT '' ,\n\
site_country nvarchar2(64) DEFAULT '' ,\n\
site_zip nvarchar2(64) DEFAULT '' ,\n\
site_rack nvarchar2(128) DEFAULT '' ,\n\
site_notes nvarchar2(2048) DEFAULT '' ,\n\
poc_1_name nvarchar2(128) DEFAULT '' ,\n\
poc_1_email nvarchar2(128) DEFAULT '' ,\n\
poc_1_phone_a nvarchar2(64) DEFAULT '' ,\n\
poc_1_phone_b nvarchar2(64) DEFAULT '' ,\n\
poc_1_cell nvarchar2(64) DEFAULT '' ,\n\
poc_1_screen nvarchar2(64) DEFAULT '' ,\n\
poc_1_notes nvarchar2(2048) DEFAULT '' ,\n\
poc_2_name nvarchar2(128) DEFAULT '' ,\n\
poc_2_email nvarchar2(128) DEFAULT '' ,\n\
poc_2_phone_a nvarchar2(64) DEFAULT '' ,\n\
poc_2_phone_b nvarchar2(64) DEFAULT '' ,\n\
poc_2_cell nvarchar2(64) DEFAULT '' ,\n\
poc_2_screen nvarchar2(64) DEFAULT '' ,\n\
poc_2_notes nvarchar2(2048) DEFAULT '' ,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE housekeeper (\n\
housekeeperid number(20)  NOT NULL,\n\
tablename nvarchar2(64) DEFAULT '' ,\n\
field nvarchar2(64) DEFAULT '' ,\n\
value number(20)  NOT NULL,\n\
PRIMARY KEY (housekeeperid)\n\
);\n\
CREATE TABLE images (\n\
imageid number(20)  NOT NULL,\n\
imagetype number(10) DEFAULT '0' NOT NULL,\n\
name nvarchar2(64) DEFAULT '0' ,\n\
image blob DEFAULT '' NOT NULL,\n\
PRIMARY KEY (imageid)\n\
);\n\
CREATE INDEX images_1 ON images (imagetype,name);\n\
CREATE TABLE item_discovery (\n\
itemdiscoveryid number(20)  NOT NULL,\n\
itemid number(20)  NOT NULL,\n\
parent_itemid number(20)  NOT NULL,\n\
key_ nvarchar2(255) DEFAULT '' ,\n\
lastcheck number(10) DEFAULT '0' NOT NULL,\n\
ts_delete number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);\n\
CREATE INDEX item_discovery_2 ON item_discovery (parent_itemid);\n\
CREATE TABLE host_discovery (\n\
hostid number(20)  NOT NULL,\n\
parent_hostid number(20)  NULL,\n\
parent_itemid number(20)  NULL,\n\
host nvarchar2(64) DEFAULT '' ,\n\
lastcheck number(10) DEFAULT '0' NOT NULL,\n\
ts_delete number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE interface_discovery (\n\
interfaceid number(20)  NOT NULL,\n\
parent_interfaceid number(20)  NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE TABLE profiles (\n\
profileid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
idx nvarchar2(96) DEFAULT '' ,\n\
idx2 number(20) DEFAULT '0' NOT NULL,\n\
value_id number(20) DEFAULT '0' NOT NULL,\n\
value_int number(10) DEFAULT '0' NOT NULL,\n\
value_str nvarchar2(255) DEFAULT '' ,\n\
source nvarchar2(96) DEFAULT '' ,\n\
type number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (profileid)\n\
);\n\
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);\n\
CREATE INDEX profiles_2 ON profiles (userid,profileid);\n\
CREATE TABLE sessions (\n\
sessionid nvarchar2(32) DEFAULT '' ,\n\
userid number(20)  NOT NULL,\n\
lastaccess number(10) DEFAULT '0' NOT NULL,\n\
status number(10) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sessionid)\n\
);\n\
CREATE INDEX sessions_1 ON sessions (userid,status);\n\
CREATE TABLE trigger_discovery (\n\
triggerdiscoveryid number(20)  NOT NULL,\n\
triggerid number(20)  NOT NULL,\n\
parent_triggerid number(20)  NOT NULL,\n\
name nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (triggerdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX trigger_discovery_1 ON trigger_discovery (triggerid,parent_triggerid);\n\
CREATE INDEX trigger_discovery_2 ON trigger_discovery (parent_triggerid);\n\
CREATE TABLE user_history (\n\
userhistoryid number(20)  NOT NULL,\n\
userid number(20)  NOT NULL,\n\
title1 nvarchar2(255) DEFAULT '' ,\n\
url1 nvarchar2(255) DEFAULT '' ,\n\
title2 nvarchar2(255) DEFAULT '' ,\n\
url2 nvarchar2(255) DEFAULT '' ,\n\
title3 nvarchar2(255) DEFAULT '' ,\n\
url3 nvarchar2(255) DEFAULT '' ,\n\
title4 nvarchar2(255) DEFAULT '' ,\n\
url4 nvarchar2(255) DEFAULT '' ,\n\
title5 nvarchar2(255) DEFAULT '' ,\n\
url5 nvarchar2(255) DEFAULT '' ,\n\
PRIMARY KEY (userhistoryid)\n\
);\n\
CREATE UNIQUE INDEX user_history_1 ON user_history (userid);\n\
CREATE TABLE application_template (\n\
application_templateid number(20)  NOT NULL,\n\
applicationid number(20)  NOT NULL,\n\
templateid number(20)  NOT NULL,\n\
PRIMARY KEY (application_templateid)\n\
);\n\
CREATE UNIQUE INDEX application_template_1 ON application_template (applicationid,templateid);\n\
CREATE INDEX application_template_2 ON application_template (templateid);\n\
CREATE TABLE dbversion (\n\
mandatory number(10) DEFAULT '0' NOT NULL,\n\
optional number(10) DEFAULT '0' NOT NULL\n\
);\n\
INSERT INTO dbversion VALUES ('2020000','2020001');\n\
CREATE SEQUENCE history_sync_seq\n\
START WITH 1\n\
INCREMENT BY 1\n\
NOMAXVALUE\n\
/\n\
CREATE TRIGGER history_sync_tr\n\
BEFORE INSERT ON history_sync\n\
FOR EACH ROW\n\
BEGIN\n\
SELECT history_sync_seq.nextval INTO :new.id FROM dual;\n\
END;\n\
/\n\
CREATE SEQUENCE history_uint_sync_seq\n\
START WITH 1\n\
INCREMENT BY 1\n\
NOMAXVALUE\n\
/\n\
CREATE TRIGGER history_uint_sync_tr\n\
BEFORE INSERT ON history_uint_sync\n\
FOR EACH ROW\n\
BEGIN\n\
SELECT history_uint_sync_seq.nextval INTO :new.id FROM dual;\n\
END;\n\
/\n\
CREATE SEQUENCE history_str_sync_seq\n\
START WITH 1\n\
INCREMENT BY 1\n\
NOMAXVALUE\n\
/\n\
CREATE TRIGGER history_str_sync_tr\n\
BEFORE INSERT ON history_str_sync\n\
FOR EACH ROW\n\
BEGIN\n\
SELECT history_str_sync_seq.nextval INTO :new.id FROM dual;\n\
END;\n\
/\n\
CREATE SEQUENCE proxy_history_seq\n\
START WITH 1\n\
INCREMENT BY 1\n\
NOMAXVALUE\n\
/\n\
CREATE TRIGGER proxy_history_tr\n\
BEFORE INSERT ON proxy_history\n\
FOR EACH ROW\n\
BEGIN\n\
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;\n\
END;\n\
/\n\
CREATE SEQUENCE proxy_dhistory_seq\n\
START WITH 1\n\
INCREMENT BY 1\n\
NOMAXVALUE\n\
/\n\
CREATE TRIGGER proxy_dhistory_tr\n\
BEFORE INSERT ON proxy_dhistory\n\
FOR EACH ROW\n\
BEGIN\n\
SELECT proxy_dhistory_seq.nextval INTO :new.id FROM dual;\n\
END;\n\
/\n\
CREATE SEQUENCE proxy_autoreg_host_seq\n\
START WITH 1\n\
INCREMENT BY 1\n\
NOMAXVALUE\n\
/\n\
CREATE TRIGGER proxy_autoreg_host_tr\n\
BEFORE INSERT ON proxy_autoreg_host\n\
FOR EACH ROW\n\
BEGIN\n\
SELECT proxy_autoreg_host_seq.nextval INTO :new.id FROM dual;\n\
END;\n\
/\n\
";
const char	*const db_schema_fkeys[] = {
	"ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid)",
	"ALTER TABLE hosts ADD CONSTRAINT c_hosts_3 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_3 FOREIGN KEY (templateid) REFERENCES group_prototype (group_prototypeid) ON DELETE CASCADE",
	"ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_1 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_2 FOREIGN KEY (parent_group_prototypeid) REFERENCES group_prototype (group_prototypeid)",
	"ALTER TABLE screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE",
	"ALTER TABLE slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE",
	"ALTER TABLE slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE",
	"ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE",
	"ALTER TABLE applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid)",
	"ALTER TABLE httptest ADD CONSTRAINT c_httptest_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE httptest ADD CONSTRAINT c_httptest_3 FOREIGN KEY (templateid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid)",
	"ALTER TABLE items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid)",
	"ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE",
	"ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE",
	"ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid)",
	"ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid)",
	"ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid)",
	"ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid)",
	"ALTER TABLE opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid)",
	"ALTER TABLE triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid)",
	"ALTER TABLE graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid)",
	"ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	"ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE",
	"ALTER TABLE media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE",
	"ALTER TABLE rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE",
	"ALTER TABLE rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid)",
	"ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE",
	"ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_1 FOREIGN KEY (backgroundid) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_2 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_2 FOREIGN KEY (iconid_off) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_3 FOREIGN KEY (iconid_on) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_4 FOREIGN KEY (iconid_disabled) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_5 FOREIGN KEY (iconid_maintenance) REFERENCES images (imageid)",
	"ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE",
	"ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE",
	"ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE",
	"ALTER TABLE nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid)",
	"ALTER TABLE node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE",
	"ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE",
	"ALTER TABLE auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE",
	"ALTER TABLE service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE",
	"ALTER TABLE dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE",
	"ALTER TABLE dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE",
	"ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_2 FOREIGN KEY (parent_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_3 FOREIGN KEY (parent_itemid) REFERENCES items (itemid)",
	"ALTER TABLE interface_discovery ADD CONSTRAINT c_interface_discovery_1 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE",
	"ALTER TABLE interface_discovery ADD CONSTRAINT c_interface_discovery_2 FOREIGN KEY (parent_interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE",
	"ALTER TABLE profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE application_template ADD CONSTRAINT c_application_template_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	"ALTER TABLE application_template ADD CONSTRAINT c_application_template_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	NULL
};
const char	*const db_schema_fkeys_drop[] = {
	"ALTER TABLE hosts DROP CONSTRAINT c_hosts_1",
	"ALTER TABLE hosts DROP CONSTRAINT c_hosts_2",
	"ALTER TABLE hosts DROP CONSTRAINT c_hosts_3",
	"ALTER TABLE group_prototype DROP CONSTRAINT c_group_prototype_1",
	"ALTER TABLE group_prototype DROP CONSTRAINT c_group_prototype_2",
	"ALTER TABLE group_prototype DROP CONSTRAINT c_group_prototype_3",
	"ALTER TABLE group_discovery DROP CONSTRAINT c_group_discovery_1",
	"ALTER TABLE group_discovery DROP CONSTRAINT c_group_discovery_2",
	"ALTER TABLE screens DROP CONSTRAINT c_screens_1",
	"ALTER TABLE screens_items DROP CONSTRAINT c_screens_items_1",
	"ALTER TABLE slides DROP CONSTRAINT c_slides_1",
	"ALTER TABLE slides DROP CONSTRAINT c_slides_2",
	"ALTER TABLE drules DROP CONSTRAINT c_drules_1",
	"ALTER TABLE dchecks DROP CONSTRAINT c_dchecks_1",
	"ALTER TABLE applications DROP CONSTRAINT c_applications_1",
	"ALTER TABLE httptest DROP CONSTRAINT c_httptest_1",
	"ALTER TABLE httptest DROP CONSTRAINT c_httptest_2",
	"ALTER TABLE httptest DROP CONSTRAINT c_httptest_3",
	"ALTER TABLE httpstep DROP CONSTRAINT c_httpstep_1",
	"ALTER TABLE interface DROP CONSTRAINT c_interface_1",
	"ALTER TABLE items DROP CONSTRAINT c_items_1",
	"ALTER TABLE items DROP CONSTRAINT c_items_2",
	"ALTER TABLE items DROP CONSTRAINT c_items_3",
	"ALTER TABLE items DROP CONSTRAINT c_items_4",
	"ALTER TABLE httpstepitem DROP CONSTRAINT c_httpstepitem_1",
	"ALTER TABLE httpstepitem DROP CONSTRAINT c_httpstepitem_2",
	"ALTER TABLE httptestitem DROP CONSTRAINT c_httptestitem_1",
	"ALTER TABLE httptestitem DROP CONSTRAINT c_httptestitem_2",
	"ALTER TABLE users_groups DROP CONSTRAINT c_users_groups_1",
	"ALTER TABLE users_groups DROP CONSTRAINT c_users_groups_2",
	"ALTER TABLE scripts DROP CONSTRAINT c_scripts_1",
	"ALTER TABLE scripts DROP CONSTRAINT c_scripts_2",
	"ALTER TABLE operations DROP CONSTRAINT c_operations_1",
	"ALTER TABLE opmessage DROP CONSTRAINT c_opmessage_1",
	"ALTER TABLE opmessage DROP CONSTRAINT c_opmessage_2",
	"ALTER TABLE opmessage_grp DROP CONSTRAINT c_opmessage_grp_1",
	"ALTER TABLE opmessage_grp DROP CONSTRAINT c_opmessage_grp_2",
	"ALTER TABLE opmessage_usr DROP CONSTRAINT c_opmessage_usr_1",
	"ALTER TABLE opmessage_usr DROP CONSTRAINT c_opmessage_usr_2",
	"ALTER TABLE opcommand DROP CONSTRAINT c_opcommand_1",
	"ALTER TABLE opcommand DROP CONSTRAINT c_opcommand_2",
	"ALTER TABLE opcommand_hst DROP CONSTRAINT c_opcommand_hst_1",
	"ALTER TABLE opcommand_hst DROP CONSTRAINT c_opcommand_hst_2",
	"ALTER TABLE opcommand_grp DROP CONSTRAINT c_opcommand_grp_1",
	"ALTER TABLE opcommand_grp DROP CONSTRAINT c_opcommand_grp_2",
	"ALTER TABLE opgroup DROP CONSTRAINT c_opgroup_1",
	"ALTER TABLE opgroup DROP CONSTRAINT c_opgroup_2",
	"ALTER TABLE optemplate DROP CONSTRAINT c_optemplate_1",
	"ALTER TABLE optemplate DROP CONSTRAINT c_optemplate_2",
	"ALTER TABLE opconditions DROP CONSTRAINT c_opconditions_1",
	"ALTER TABLE conditions DROP CONSTRAINT c_conditions_1",
	"ALTER TABLE config DROP CONSTRAINT c_config_1",
	"ALTER TABLE config DROP CONSTRAINT c_config_2",
	"ALTER TABLE triggers DROP CONSTRAINT c_triggers_1",
	"ALTER TABLE trigger_depends DROP CONSTRAINT c_trigger_depends_1",
	"ALTER TABLE trigger_depends DROP CONSTRAINT c_trigger_depends_2",
	"ALTER TABLE functions DROP CONSTRAINT c_functions_1",
	"ALTER TABLE functions DROP CONSTRAINT c_functions_2",
	"ALTER TABLE graphs DROP CONSTRAINT c_graphs_1",
	"ALTER TABLE graphs DROP CONSTRAINT c_graphs_2",
	"ALTER TABLE graphs DROP CONSTRAINT c_graphs_3",
	"ALTER TABLE graphs_items DROP CONSTRAINT c_graphs_items_1",
	"ALTER TABLE graphs_items DROP CONSTRAINT c_graphs_items_2",
	"ALTER TABLE hostmacro DROP CONSTRAINT c_hostmacro_1",
	"ALTER TABLE hosts_groups DROP CONSTRAINT c_hosts_groups_1",
	"ALTER TABLE hosts_groups DROP CONSTRAINT c_hosts_groups_2",
	"ALTER TABLE hosts_templates DROP CONSTRAINT c_hosts_templates_1",
	"ALTER TABLE hosts_templates DROP CONSTRAINT c_hosts_templates_2",
	"ALTER TABLE items_applications DROP CONSTRAINT c_items_applications_1",
	"ALTER TABLE items_applications DROP CONSTRAINT c_items_applications_2",
	"ALTER TABLE mappings DROP CONSTRAINT c_mappings_1",
	"ALTER TABLE media DROP CONSTRAINT c_media_1",
	"ALTER TABLE media DROP CONSTRAINT c_media_2",
	"ALTER TABLE rights DROP CONSTRAINT c_rights_1",
	"ALTER TABLE rights DROP CONSTRAINT c_rights_2",
	"ALTER TABLE services DROP CONSTRAINT c_services_1",
	"ALTER TABLE services_links DROP CONSTRAINT c_services_links_1",
	"ALTER TABLE services_links DROP CONSTRAINT c_services_links_2",
	"ALTER TABLE services_times DROP CONSTRAINT c_services_times_1",
	"ALTER TABLE icon_map DROP CONSTRAINT c_icon_map_1",
	"ALTER TABLE icon_mapping DROP CONSTRAINT c_icon_mapping_1",
	"ALTER TABLE icon_mapping DROP CONSTRAINT c_icon_mapping_2",
	"ALTER TABLE sysmaps DROP CONSTRAINT c_sysmaps_1",
	"ALTER TABLE sysmaps DROP CONSTRAINT c_sysmaps_2",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_1",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_2",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_3",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_4",
	"ALTER TABLE sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_5",
	"ALTER TABLE sysmaps_links DROP CONSTRAINT c_sysmaps_links_1",
	"ALTER TABLE sysmaps_links DROP CONSTRAINT c_sysmaps_links_2",
	"ALTER TABLE sysmaps_links DROP CONSTRAINT c_sysmaps_links_3",
	"ALTER TABLE sysmaps_link_triggers DROP CONSTRAINT c_sysmaps_link_triggers_1",
	"ALTER TABLE sysmaps_link_triggers DROP CONSTRAINT c_sysmaps_link_triggers_2",
	"ALTER TABLE sysmap_element_url DROP CONSTRAINT c_sysmap_element_url_1",
	"ALTER TABLE sysmap_url DROP CONSTRAINT c_sysmap_url_1",
	"ALTER TABLE maintenances_hosts DROP CONSTRAINT c_maintenances_hosts_1",
	"ALTER TABLE maintenances_hosts DROP CONSTRAINT c_maintenances_hosts_2",
	"ALTER TABLE maintenances_groups DROP CONSTRAINT c_maintenances_groups_1",
	"ALTER TABLE maintenances_groups DROP CONSTRAINT c_maintenances_groups_2",
	"ALTER TABLE maintenances_windows DROP CONSTRAINT c_maintenances_windows_1",
	"ALTER TABLE maintenances_windows DROP CONSTRAINT c_maintenances_windows_2",
	"ALTER TABLE expressions DROP CONSTRAINT c_expressions_1",
	"ALTER TABLE nodes DROP CONSTRAINT c_nodes_1",
	"ALTER TABLE node_cksum DROP CONSTRAINT c_node_cksum_1",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_1",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_2",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_3",
	"ALTER TABLE alerts DROP CONSTRAINT c_alerts_4",
	"ALTER TABLE acknowledges DROP CONSTRAINT c_acknowledges_1",
	"ALTER TABLE acknowledges DROP CONSTRAINT c_acknowledges_2",
	"ALTER TABLE auditlog DROP CONSTRAINT c_auditlog_1",
	"ALTER TABLE auditlog_details DROP CONSTRAINT c_auditlog_details_1",
	"ALTER TABLE service_alarms DROP CONSTRAINT c_service_alarms_1",
	"ALTER TABLE autoreg_host DROP CONSTRAINT c_autoreg_host_1",
	"ALTER TABLE dhosts DROP CONSTRAINT c_dhosts_1",
	"ALTER TABLE dservices DROP CONSTRAINT c_dservices_1",
	"ALTER TABLE dservices DROP CONSTRAINT c_dservices_2",
	"ALTER TABLE graph_discovery DROP CONSTRAINT c_graph_discovery_1",
	"ALTER TABLE graph_discovery DROP CONSTRAINT c_graph_discovery_2",
	"ALTER TABLE host_inventory DROP CONSTRAINT c_host_inventory_1",
	"ALTER TABLE item_discovery DROP CONSTRAINT c_item_discovery_1",
	"ALTER TABLE item_discovery DROP CONSTRAINT c_item_discovery_2",
	"ALTER TABLE host_discovery DROP CONSTRAINT c_host_discovery_1",
	"ALTER TABLE host_discovery DROP CONSTRAINT c_host_discovery_2",
	"ALTER TABLE host_discovery DROP CONSTRAINT c_host_discovery_3",
	"ALTER TABLE interface_discovery DROP CONSTRAINT c_interface_discovery_1",
	"ALTER TABLE interface_discovery DROP CONSTRAINT c_interface_discovery_2",
	"ALTER TABLE profiles DROP CONSTRAINT c_profiles_1",
	"ALTER TABLE sessions DROP CONSTRAINT c_sessions_1",
	"ALTER TABLE trigger_discovery DROP CONSTRAINT c_trigger_discovery_1",
	"ALTER TABLE trigger_discovery DROP CONSTRAINT c_trigger_discovery_2",
	"ALTER TABLE user_history DROP CONSTRAINT c_user_history_1",
	"ALTER TABLE application_template DROP CONSTRAINT c_application_template_1",
	"ALTER TABLE application_template DROP CONSTRAINT c_application_template_2",
	NULL
};
#elif defined(HAVE_POSTGRESQL)
const char	*const db_schema = "\
CREATE TABLE maintenances (\n\
maintenanceid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
maintenance_type integer DEFAULT '0' NOT NULL,\n\
description text DEFAULT '' NOT NULL,\n\
active_since integer DEFAULT '0' NOT NULL,\n\
active_till integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (maintenanceid)\n\
);\n\
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);\n\
CREATE TABLE hosts (\n\
hostid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL,\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
disable_until integer DEFAULT '0' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
available integer DEFAULT '0' NOT NULL,\n\
errors_from integer DEFAULT '0' NOT NULL,\n\
lastaccess integer DEFAULT '0' NOT NULL,\n\
ipmi_authtype integer DEFAULT '0' NOT NULL,\n\
ipmi_privilege integer DEFAULT '2' NOT NULL,\n\
ipmi_username varchar(16) DEFAULT '' NOT NULL,\n\
ipmi_password varchar(20) DEFAULT '' NOT NULL,\n\
ipmi_disable_until integer DEFAULT '0' NOT NULL,\n\
ipmi_available integer DEFAULT '0' NOT NULL,\n\
snmp_disable_until integer DEFAULT '0' NOT NULL,\n\
snmp_available integer DEFAULT '0' NOT NULL,\n\
maintenanceid bigint  NULL,\n\
maintenance_status integer DEFAULT '0' NOT NULL,\n\
maintenance_type integer DEFAULT '0' NOT NULL,\n\
maintenance_from integer DEFAULT '0' NOT NULL,\n\
ipmi_errors_from integer DEFAULT '0' NOT NULL,\n\
snmp_errors_from integer DEFAULT '0' NOT NULL,\n\
ipmi_error varchar(128) DEFAULT '' NOT NULL,\n\
snmp_error varchar(128) DEFAULT '' NOT NULL,\n\
jmx_disable_until integer DEFAULT '0' NOT NULL,\n\
jmx_available integer DEFAULT '0' NOT NULL,\n\
jmx_errors_from integer DEFAULT '0' NOT NULL,\n\
jmx_error varchar(128) DEFAULT '' NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
templateid bigint  NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE INDEX hosts_1 ON hosts (host);\n\
CREATE INDEX hosts_2 ON hosts (status);\n\
CREATE INDEX hosts_3 ON hosts (proxy_hostid);\n\
CREATE INDEX hosts_4 ON hosts (name);\n\
CREATE INDEX hosts_5 ON hosts (maintenanceid);\n\
CREATE TABLE groups (\n\
groupid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
internal integer DEFAULT '0' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE INDEX groups_1 ON groups (name);\n\
CREATE TABLE group_prototype (\n\
group_prototypeid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
groupid bigint  NULL,\n\
templateid bigint  NULL,\n\
PRIMARY KEY (group_prototypeid)\n\
);\n\
CREATE INDEX group_prototype_1 ON group_prototype (hostid);\n\
CREATE TABLE group_discovery (\n\
groupid bigint  NOT NULL,\n\
parent_group_prototypeid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
lastcheck integer DEFAULT '0' NOT NULL,\n\
ts_delete integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE TABLE screens (\n\
screenid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
hsize integer DEFAULT '1' NOT NULL,\n\
vsize integer DEFAULT '1' NOT NULL,\n\
templateid bigint  NULL,\n\
PRIMARY KEY (screenid)\n\
);\n\
CREATE INDEX screens_1 ON screens (templateid);\n\
CREATE TABLE screens_items (\n\
screenitemid bigint  NOT NULL,\n\
screenid bigint  NOT NULL,\n\
resourcetype integer DEFAULT '0' NOT NULL,\n\
resourceid bigint DEFAULT '0' NOT NULL,\n\
width integer DEFAULT '320' NOT NULL,\n\
height integer DEFAULT '200' NOT NULL,\n\
x integer DEFAULT '0' NOT NULL,\n\
y integer DEFAULT '0' NOT NULL,\n\
colspan integer DEFAULT '0' NOT NULL,\n\
rowspan integer DEFAULT '0' NOT NULL,\n\
elements integer DEFAULT '25' NOT NULL,\n\
valign integer DEFAULT '0' NOT NULL,\n\
halign integer DEFAULT '0' NOT NULL,\n\
style integer DEFAULT '0' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
dynamic integer DEFAULT '0' NOT NULL,\n\
sort_triggers integer DEFAULT '0' NOT NULL,\n\
application varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (screenitemid)\n\
);\n\
CREATE INDEX screens_items_1 ON screens_items (screenid);\n\
CREATE TABLE slideshows (\n\
slideshowid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
delay integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideshowid)\n\
);\n\
CREATE TABLE slides (\n\
slideid bigint  NOT NULL,\n\
slideshowid bigint  NOT NULL,\n\
screenid bigint  NOT NULL,\n\
step integer DEFAULT '0' NOT NULL,\n\
delay integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideid)\n\
);\n\
CREATE INDEX slides_1 ON slides (slideshowid);\n\
CREATE INDEX slides_2 ON slides (screenid);\n\
CREATE TABLE drules (\n\
druleid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
iprange varchar(255) DEFAULT '' NOT NULL,\n\
delay integer DEFAULT '3600' NOT NULL,\n\
nextcheck integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (druleid)\n\
);\n\
CREATE INDEX drules_1 ON drules (proxy_hostid);\n\
CREATE TABLE dchecks (\n\
dcheckid bigint  NOT NULL,\n\
druleid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
snmp_community varchar(255) DEFAULT '' NOT NULL,\n\
ports varchar(255) DEFAULT '0' NOT NULL,\n\
snmpv3_securityname varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_securitylevel integer DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_privpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
uniq integer DEFAULT '0' NOT NULL,\n\
snmpv3_authprotocol integer DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol integer DEFAULT '0' NOT NULL,\n\
snmpv3_contextname varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dcheckid)\n\
);\n\
CREATE INDEX dchecks_1 ON dchecks (druleid);\n\
CREATE TABLE applications (\n\
applicationid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (applicationid)\n\
);\n\
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);\n\
CREATE TABLE httptest (\n\
httptestid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
applicationid bigint  NULL,\n\
nextcheck integer DEFAULT '0' NOT NULL,\n\
delay integer DEFAULT '60' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
variables text DEFAULT '' NOT NULL,\n\
agent varchar(255) DEFAULT '' NOT NULL,\n\
authentication integer DEFAULT '0' NOT NULL,\n\
http_user varchar(64) DEFAULT '' NOT NULL,\n\
http_password varchar(64) DEFAULT '' NOT NULL,\n\
hostid bigint  NOT NULL,\n\
templateid bigint  NULL,\n\
http_proxy varchar(255) DEFAULT '' NOT NULL,\n\
retries integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (httptestid)\n\
);\n\
CREATE INDEX httptest_1 ON httptest (applicationid);\n\
CREATE UNIQUE INDEX httptest_2 ON httptest (hostid,name);\n\
CREATE INDEX httptest_3 ON httptest (status);\n\
CREATE INDEX httptest_4 ON httptest (templateid);\n\
CREATE TABLE httpstep (\n\
httpstepid bigint  NOT NULL,\n\
httptestid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
no integer DEFAULT '0' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
timeout integer DEFAULT '30' NOT NULL,\n\
posts text DEFAULT '' NOT NULL,\n\
required varchar(255) DEFAULT '' NOT NULL,\n\
status_codes varchar(255) DEFAULT '' NOT NULL,\n\
variables text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (httpstepid)\n\
);\n\
CREATE INDEX httpstep_1 ON httpstep (httptestid);\n\
CREATE TABLE interface (\n\
interfaceid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
main integer DEFAULT '0' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
useip integer DEFAULT '1' NOT NULL,\n\
ip varchar(64) DEFAULT '127.0.0.1' NOT NULL,\n\
dns varchar(64) DEFAULT '' NOT NULL,\n\
port varchar(64) DEFAULT '10050' NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE INDEX interface_1 ON interface (hostid,type);\n\
CREATE INDEX interface_2 ON interface (ip,dns);\n\
CREATE TABLE valuemaps (\n\
valuemapid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (valuemapid)\n\
);\n\
CREATE INDEX valuemaps_1 ON valuemaps (name);\n\
CREATE TABLE items (\n\
itemid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
snmp_community varchar(64) DEFAULT '' NOT NULL,\n\
snmp_oid varchar(255) DEFAULT '' NOT NULL,\n\
hostid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
delay integer DEFAULT '0' NOT NULL,\n\
history integer DEFAULT '90' NOT NULL,\n\
trends integer DEFAULT '365' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
value_type integer DEFAULT '0' NOT NULL,\n\
trapper_hosts varchar(255) DEFAULT '' NOT NULL,\n\
units varchar(255) DEFAULT '' NOT NULL,\n\
multiplier integer DEFAULT '0' NOT NULL,\n\
delta integer DEFAULT '0' NOT NULL,\n\
snmpv3_securityname varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_securitylevel integer DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_privpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
formula varchar(255) DEFAULT '1' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
lastlogsize numeric(20) DEFAULT '0' NOT NULL,\n\
logtimefmt varchar(64) DEFAULT '' NOT NULL,\n\
templateid bigint  NULL,\n\
valuemapid bigint  NULL,\n\
delay_flex varchar(255) DEFAULT '' NOT NULL,\n\
params text DEFAULT '' NOT NULL,\n\
ipmi_sensor varchar(128) DEFAULT '' NOT NULL,\n\
data_type integer DEFAULT '0' NOT NULL,\n\
authtype integer DEFAULT '0' NOT NULL,\n\
username varchar(64) DEFAULT '' NOT NULL,\n\
password varchar(64) DEFAULT '' NOT NULL,\n\
publickey varchar(64) DEFAULT '' NOT NULL,\n\
privatekey varchar(64) DEFAULT '' NOT NULL,\n\
mtime integer DEFAULT '0' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
filter varchar(255) DEFAULT '' NOT NULL,\n\
interfaceid bigint  NULL,\n\
port varchar(64) DEFAULT '' NOT NULL,\n\
description text DEFAULT '' NOT NULL,\n\
inventory_link integer DEFAULT '0' NOT NULL,\n\
lifetime varchar(64) DEFAULT '30' NOT NULL,\n\
snmpv3_authprotocol integer DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol integer DEFAULT '0' NOT NULL,\n\
state integer DEFAULT '0' NOT NULL,\n\
snmpv3_contextname varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (itemid)\n\
);\n\
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);\n\
CREATE INDEX items_3 ON items (status);\n\
CREATE INDEX items_4 ON items (templateid);\n\
CREATE INDEX items_5 ON items (valuemapid);\n\
CREATE INDEX items_6 ON items (interfaceid);\n\
CREATE TABLE httpstepitem (\n\
httpstepitemid bigint  NOT NULL,\n\
httpstepid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httpstepitemid)\n\
);\n\
CREATE UNIQUE INDEX httpstepitem_1 ON httpstepitem (httpstepid,itemid);\n\
CREATE INDEX httpstepitem_2 ON httpstepitem (itemid);\n\
CREATE TABLE httptestitem (\n\
httptestitemid bigint  NOT NULL,\n\
httptestid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httptestitemid)\n\
);\n\
CREATE UNIQUE INDEX httptestitem_1 ON httptestitem (httptestid,itemid);\n\
CREATE INDEX httptestitem_2 ON httptestitem (itemid);\n\
CREATE TABLE media_type (\n\
mediatypeid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
description varchar(100) DEFAULT '' NOT NULL,\n\
smtp_server varchar(255) DEFAULT '' NOT NULL,\n\
smtp_helo varchar(255) DEFAULT '' NOT NULL,\n\
smtp_email varchar(255) DEFAULT '' NOT NULL,\n\
exec_path varchar(255) DEFAULT '' NOT NULL,\n\
gsm_modem varchar(255) DEFAULT '' NOT NULL,\n\
username varchar(255) DEFAULT '' NOT NULL,\n\
passwd varchar(255) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (mediatypeid)\n\
);\n\
CREATE TABLE users (\n\
userid bigint  NOT NULL,\n\
alias varchar(100) DEFAULT '' NOT NULL,\n\
name varchar(100) DEFAULT '' NOT NULL,\n\
surname varchar(100) DEFAULT '' NOT NULL,\n\
passwd char(32) DEFAULT '' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
autologin integer DEFAULT '0' NOT NULL,\n\
autologout integer DEFAULT '900' NOT NULL,\n\
lang varchar(5) DEFAULT 'en_GB' NOT NULL,\n\
refresh integer DEFAULT '30' NOT NULL,\n\
type integer DEFAULT '1' NOT NULL,\n\
theme varchar(128) DEFAULT 'default' NOT NULL,\n\
attempt_failed integer DEFAULT 0 NOT NULL,\n\
attempt_ip varchar(39) DEFAULT '' NOT NULL,\n\
attempt_clock integer DEFAULT 0 NOT NULL,\n\
rows_per_page integer DEFAULT 50 NOT NULL,\n\
PRIMARY KEY (userid)\n\
);\n\
CREATE INDEX users_1 ON users (alias);\n\
CREATE TABLE usrgrp (\n\
usrgrpid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
gui_access integer DEFAULT '0' NOT NULL,\n\
users_status integer DEFAULT '0' NOT NULL,\n\
debug_mode integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (usrgrpid)\n\
);\n\
CREATE INDEX usrgrp_1 ON usrgrp (name);\n\
CREATE TABLE users_groups (\n\
id bigint  NOT NULL,\n\
usrgrpid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);\n\
CREATE INDEX users_groups_2 ON users_groups (userid);\n\
CREATE TABLE scripts (\n\
scriptid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
command varchar(255) DEFAULT '' NOT NULL,\n\
host_access integer DEFAULT '2' NOT NULL,\n\
usrgrpid bigint  NULL,\n\
groupid bigint  NULL,\n\
description text DEFAULT '' NOT NULL,\n\
confirmation varchar(255) DEFAULT '' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
execute_on integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (scriptid)\n\
);\n\
CREATE INDEX scripts_1 ON scripts (usrgrpid);\n\
CREATE INDEX scripts_2 ON scripts (groupid);\n\
CREATE TABLE actions (\n\
actionid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
eventsource integer DEFAULT '0' NOT NULL,\n\
evaltype integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
esc_period integer DEFAULT '0' NOT NULL,\n\
def_shortdata varchar(255) DEFAULT '' NOT NULL,\n\
def_longdata text DEFAULT '' NOT NULL,\n\
recovery_msg integer DEFAULT '0' NOT NULL,\n\
r_shortdata varchar(255) DEFAULT '' NOT NULL,\n\
r_longdata text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (actionid)\n\
);\n\
CREATE INDEX actions_1 ON actions (eventsource,status);\n\
CREATE TABLE operations (\n\
operationid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
operationtype integer DEFAULT '0' NOT NULL,\n\
esc_period integer DEFAULT '0' NOT NULL,\n\
esc_step_from integer DEFAULT '1' NOT NULL,\n\
esc_step_to integer DEFAULT '1' NOT NULL,\n\
evaltype integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX operations_1 ON operations (actionid);\n\
CREATE TABLE opmessage (\n\
operationid bigint  NOT NULL,\n\
default_msg integer DEFAULT '0' NOT NULL,\n\
subject varchar(255) DEFAULT '' NOT NULL,\n\
message text DEFAULT '' NOT NULL,\n\
mediatypeid bigint  NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opmessage_1 ON opmessage (mediatypeid);\n\
CREATE TABLE opmessage_grp (\n\
opmessage_grpid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
usrgrpid bigint  NOT NULL,\n\
PRIMARY KEY (opmessage_grpid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);\n\
CREATE INDEX opmessage_grp_2 ON opmessage_grp (usrgrpid);\n\
CREATE TABLE opmessage_usr (\n\
opmessage_usrid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
PRIMARY KEY (opmessage_usrid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);\n\
CREATE INDEX opmessage_usr_2 ON opmessage_usr (userid);\n\
CREATE TABLE opcommand (\n\
operationid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
scriptid bigint  NULL,\n\
execute_on integer DEFAULT '0' NOT NULL,\n\
port varchar(64) DEFAULT '' NOT NULL,\n\
authtype integer DEFAULT '0' NOT NULL,\n\
username varchar(64) DEFAULT '' NOT NULL,\n\
password varchar(64) DEFAULT '' NOT NULL,\n\
publickey varchar(64) DEFAULT '' NOT NULL,\n\
privatekey varchar(64) DEFAULT '' NOT NULL,\n\
command text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opcommand_1 ON opcommand (scriptid);\n\
CREATE TABLE opcommand_hst (\n\
opcommand_hstid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
hostid bigint  NULL,\n\
PRIMARY KEY (opcommand_hstid)\n\
);\n\
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);\n\
CREATE INDEX opcommand_hst_2 ON opcommand_hst (hostid);\n\
CREATE TABLE opcommand_grp (\n\
opcommand_grpid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (opcommand_grpid)\n\
);\n\
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);\n\
CREATE INDEX opcommand_grp_2 ON opcommand_grp (groupid);\n\
CREATE TABLE opgroup (\n\
opgroupid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (opgroupid)\n\
);\n\
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);\n\
CREATE INDEX opgroup_2 ON opgroup (groupid);\n\
CREATE TABLE optemplate (\n\
optemplateid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
templateid bigint  NOT NULL,\n\
PRIMARY KEY (optemplateid)\n\
);\n\
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);\n\
CREATE INDEX optemplate_2 ON optemplate (templateid);\n\
CREATE TABLE opconditions (\n\
opconditionid bigint  NOT NULL,\n\
operationid bigint  NOT NULL,\n\
conditiontype integer DEFAULT '0' NOT NULL,\n\
operator integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (opconditionid)\n\
);\n\
CREATE INDEX opconditions_1 ON opconditions (operationid);\n\
CREATE TABLE conditions (\n\
conditionid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
conditiontype integer DEFAULT '0' NOT NULL,\n\
operator integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (conditionid)\n\
);\n\
CREATE INDEX conditions_1 ON conditions (actionid);\n\
CREATE TABLE config (\n\
configid bigint  NOT NULL,\n\
refresh_unsupported integer DEFAULT '0' NOT NULL,\n\
work_period varchar(100) DEFAULT '1-5,00:00-24:00' NOT NULL,\n\
alert_usrgrpid bigint  NULL,\n\
event_ack_enable integer DEFAULT '1' NOT NULL,\n\
event_expire integer DEFAULT '7' NOT NULL,\n\
event_show_max integer DEFAULT '100' NOT NULL,\n\
default_theme varchar(128) DEFAULT 'originalblue' NOT NULL,\n\
authentication_type integer DEFAULT '0' NOT NULL,\n\
ldap_host varchar(255) DEFAULT '' NOT NULL,\n\
ldap_port integer DEFAULT 389 NOT NULL,\n\
ldap_base_dn varchar(255) DEFAULT '' NOT NULL,\n\
ldap_bind_dn varchar(255) DEFAULT '' NOT NULL,\n\
ldap_bind_password varchar(128) DEFAULT '' NOT NULL,\n\
ldap_search_attribute varchar(128) DEFAULT '' NOT NULL,\n\
dropdown_first_entry integer DEFAULT '1' NOT NULL,\n\
dropdown_first_remember integer DEFAULT '1' NOT NULL,\n\
discovery_groupid bigint  NOT NULL,\n\
max_in_table integer DEFAULT '50' NOT NULL,\n\
search_limit integer DEFAULT '1000' NOT NULL,\n\
severity_color_0 varchar(6) DEFAULT 'DBDBDB' NOT NULL,\n\
severity_color_1 varchar(6) DEFAULT 'D6F6FF' NOT NULL,\n\
severity_color_2 varchar(6) DEFAULT 'FFF6A5' NOT NULL,\n\
severity_color_3 varchar(6) DEFAULT 'FFB689' NOT NULL,\n\
severity_color_4 varchar(6) DEFAULT 'FF9999' NOT NULL,\n\
severity_color_5 varchar(6) DEFAULT 'FF3838' NOT NULL,\n\
severity_name_0 varchar(32) DEFAULT 'Not classified' NOT NULL,\n\
severity_name_1 varchar(32) DEFAULT 'Information' NOT NULL,\n\
severity_name_2 varchar(32) DEFAULT 'Warning' NOT NULL,\n\
severity_name_3 varchar(32) DEFAULT 'Average' NOT NULL,\n\
severity_name_4 varchar(32) DEFAULT 'High' NOT NULL,\n\
severity_name_5 varchar(32) DEFAULT 'Disaster' NOT NULL,\n\
ok_period integer DEFAULT '1800' NOT NULL,\n\
blink_period integer DEFAULT '1800' NOT NULL,\n\
problem_unack_color varchar(6) DEFAULT 'DC0000' NOT NULL,\n\
problem_ack_color varchar(6) DEFAULT 'DC0000' NOT NULL,\n\
ok_unack_color varchar(6) DEFAULT '00AA00' NOT NULL,\n\
ok_ack_color varchar(6) DEFAULT '00AA00' NOT NULL,\n\
problem_unack_style integer DEFAULT '1' NOT NULL,\n\
problem_ack_style integer DEFAULT '1' NOT NULL,\n\
ok_unack_style integer DEFAULT '1' NOT NULL,\n\
ok_ack_style integer DEFAULT '1' NOT NULL,\n\
snmptrap_logging integer DEFAULT '1' NOT NULL,\n\
server_check_interval integer DEFAULT '10' NOT NULL,\n\
hk_events_mode integer DEFAULT '1' NOT NULL,\n\
hk_events_trigger integer DEFAULT '365' NOT NULL,\n\
hk_events_internal integer DEFAULT '365' NOT NULL,\n\
hk_events_discovery integer DEFAULT '365' NOT NULL,\n\
hk_events_autoreg integer DEFAULT '365' NOT NULL,\n\
hk_services_mode integer DEFAULT '1' NOT NULL,\n\
hk_services integer DEFAULT '365' NOT NULL,\n\
hk_audit_mode integer DEFAULT '1' NOT NULL,\n\
hk_audit integer DEFAULT '365' NOT NULL,\n\
hk_sessions_mode integer DEFAULT '1' NOT NULL,\n\
hk_sessions integer DEFAULT '365' NOT NULL,\n\
hk_history_mode integer DEFAULT '1' NOT NULL,\n\
hk_history_global integer DEFAULT '0' NOT NULL,\n\
hk_history integer DEFAULT '90' NOT NULL,\n\
hk_trends_mode integer DEFAULT '1' NOT NULL,\n\
hk_trends_global integer DEFAULT '0' NOT NULL,\n\
hk_trends integer DEFAULT '365' NOT NULL,\n\
PRIMARY KEY (configid)\n\
);\n\
CREATE INDEX config_1 ON config (alert_usrgrpid);\n\
CREATE INDEX config_2 ON config (discovery_groupid);\n\
CREATE TABLE triggers (\n\
triggerid bigint  NOT NULL,\n\
expression varchar(2048) DEFAULT '' NOT NULL,\n\
description varchar(255) DEFAULT '' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
value integer DEFAULT '0' NOT NULL,\n\
priority integer DEFAULT '0' NOT NULL,\n\
lastchange integer DEFAULT '0' NOT NULL,\n\
comments text DEFAULT '' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
templateid bigint  NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
state integer DEFAULT '0' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (triggerid)\n\
);\n\
CREATE INDEX triggers_1 ON triggers (status);\n\
CREATE INDEX triggers_2 ON triggers (value);\n\
CREATE INDEX triggers_3 ON triggers (templateid);\n\
CREATE TABLE trigger_depends (\n\
triggerdepid bigint  NOT NULL,\n\
triggerid_down bigint  NOT NULL,\n\
triggerid_up bigint  NOT NULL,\n\
PRIMARY KEY (triggerdepid)\n\
);\n\
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);\n\
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);\n\
CREATE TABLE functions (\n\
functionid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL,\n\
function varchar(12) DEFAULT '' NOT NULL,\n\
parameter varchar(255) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (functionid)\n\
);\n\
CREATE INDEX functions_1 ON functions (triggerid);\n\
CREATE INDEX functions_2 ON functions (itemid,function,parameter);\n\
CREATE TABLE graphs (\n\
graphid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
width integer DEFAULT '900' NOT NULL,\n\
height integer DEFAULT '200' NOT NULL,\n\
yaxismin numeric(16,4) DEFAULT '0' NOT NULL,\n\
yaxismax numeric(16,4) DEFAULT '100' NOT NULL,\n\
templateid bigint  NULL,\n\
show_work_period integer DEFAULT '1' NOT NULL,\n\
show_triggers integer DEFAULT '1' NOT NULL,\n\
graphtype integer DEFAULT '0' NOT NULL,\n\
show_legend integer DEFAULT '1' NOT NULL,\n\
show_3d integer DEFAULT '0' NOT NULL,\n\
percent_left numeric(16,4) DEFAULT '0' NOT NULL,\n\
percent_right numeric(16,4) DEFAULT '0' NOT NULL,\n\
ymin_type integer DEFAULT '0' NOT NULL,\n\
ymax_type integer DEFAULT '0' NOT NULL,\n\
ymin_itemid bigint  NULL,\n\
ymax_itemid bigint  NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (graphid)\n\
);\n\
CREATE INDEX graphs_1 ON graphs (name);\n\
CREATE INDEX graphs_2 ON graphs (templateid);\n\
CREATE INDEX graphs_3 ON graphs (ymin_itemid);\n\
CREATE INDEX graphs_4 ON graphs (ymax_itemid);\n\
CREATE TABLE graphs_items (\n\
gitemid bigint  NOT NULL,\n\
graphid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
drawtype integer DEFAULT '0' NOT NULL,\n\
sortorder integer DEFAULT '0' NOT NULL,\n\
color varchar(6) DEFAULT '009600' NOT NULL,\n\
yaxisside integer DEFAULT '0' NOT NULL,\n\
calc_fnc integer DEFAULT '2' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (gitemid)\n\
);\n\
CREATE INDEX graphs_items_1 ON graphs_items (itemid);\n\
CREATE INDEX graphs_items_2 ON graphs_items (graphid);\n\
CREATE TABLE graph_theme (\n\
graphthemeid bigint  NOT NULL,\n\
description varchar(64) DEFAULT '' NOT NULL,\n\
theme varchar(64) DEFAULT '' NOT NULL,\n\
backgroundcolor varchar(6) DEFAULT 'F0F0F0' NOT NULL,\n\
graphcolor varchar(6) DEFAULT 'FFFFFF' NOT NULL,\n\
graphbordercolor varchar(6) DEFAULT '222222' NOT NULL,\n\
gridcolor varchar(6) DEFAULT 'CCCCCC' NOT NULL,\n\
maingridcolor varchar(6) DEFAULT 'AAAAAA' NOT NULL,\n\
gridbordercolor varchar(6) DEFAULT '000000' NOT NULL,\n\
textcolor varchar(6) DEFAULT '202020' NOT NULL,\n\
highlightcolor varchar(6) DEFAULT 'AA4444' NOT NULL,\n\
leftpercentilecolor varchar(6) DEFAULT '11CC11' NOT NULL,\n\
rightpercentilecolor varchar(6) DEFAULT 'CC1111' NOT NULL,\n\
nonworktimecolor varchar(6) DEFAULT 'CCCCCC' NOT NULL,\n\
gridview integer DEFAULT 1 NOT NULL,\n\
legendview integer DEFAULT 1 NOT NULL,\n\
PRIMARY KEY (graphthemeid)\n\
);\n\
CREATE INDEX graph_theme_1 ON graph_theme (description);\n\
CREATE INDEX graph_theme_2 ON graph_theme (theme);\n\
CREATE TABLE globalmacro (\n\
globalmacroid bigint  NOT NULL,\n\
macro varchar(64) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (globalmacroid)\n\
);\n\
CREATE INDEX globalmacro_1 ON globalmacro (macro);\n\
CREATE TABLE hostmacro (\n\
hostmacroid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
macro varchar(64) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostmacroid)\n\
);\n\
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);\n\
CREATE TABLE hosts_groups (\n\
hostgroupid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (hostgroupid)\n\
);\n\
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);\n\
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);\n\
CREATE TABLE hosts_templates (\n\
hosttemplateid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
templateid bigint  NOT NULL,\n\
PRIMARY KEY (hosttemplateid)\n\
);\n\
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);\n\
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);\n\
CREATE TABLE items_applications (\n\
itemappid bigint  NOT NULL,\n\
applicationid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
PRIMARY KEY (itemappid)\n\
);\n\
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);\n\
CREATE INDEX items_applications_2 ON items_applications (itemid);\n\
CREATE TABLE mappings (\n\
mappingid bigint  NOT NULL,\n\
valuemapid bigint  NOT NULL,\n\
value varchar(64) DEFAULT '' NOT NULL,\n\
newvalue varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (mappingid)\n\
);\n\
CREATE INDEX mappings_1 ON mappings (valuemapid);\n\
CREATE TABLE media (\n\
mediaid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
mediatypeid bigint  NOT NULL,\n\
sendto varchar(100) DEFAULT '' NOT NULL,\n\
active integer DEFAULT '0' NOT NULL,\n\
severity integer DEFAULT '63' NOT NULL,\n\
period varchar(100) DEFAULT '1-7,00:00-24:00' NOT NULL,\n\
PRIMARY KEY (mediaid)\n\
);\n\
CREATE INDEX media_1 ON media (userid);\n\
CREATE INDEX media_2 ON media (mediatypeid);\n\
CREATE TABLE rights (\n\
rightid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
permission integer DEFAULT '0' NOT NULL,\n\
id bigint  NOT NULL,\n\
PRIMARY KEY (rightid)\n\
);\n\
CREATE INDEX rights_1 ON rights (groupid);\n\
CREATE INDEX rights_2 ON rights (id);\n\
CREATE TABLE services (\n\
serviceid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
algorithm integer DEFAULT '0' NOT NULL,\n\
triggerid bigint  NULL,\n\
showsla integer DEFAULT '0' NOT NULL,\n\
goodsla numeric(16,4) DEFAULT '99.9' NOT NULL,\n\
sortorder integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (serviceid)\n\
);\n\
CREATE INDEX services_1 ON services (triggerid);\n\
CREATE TABLE services_links (\n\
linkid bigint  NOT NULL,\n\
serviceupid bigint  NOT NULL,\n\
servicedownid bigint  NOT NULL,\n\
soft integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX services_links_1 ON services_links (servicedownid);\n\
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);\n\
CREATE TABLE services_times (\n\
timeid bigint  NOT NULL,\n\
serviceid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
ts_from integer DEFAULT '0' NOT NULL,\n\
ts_to integer DEFAULT '0' NOT NULL,\n\
note varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (timeid)\n\
);\n\
CREATE INDEX services_times_1 ON services_times (serviceid,type,ts_from,ts_to);\n\
CREATE TABLE icon_map (\n\
iconmapid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
default_iconid bigint  NOT NULL,\n\
PRIMARY KEY (iconmapid)\n\
);\n\
CREATE INDEX icon_map_1 ON icon_map (name);\n\
CREATE INDEX icon_map_2 ON icon_map (default_iconid);\n\
CREATE TABLE icon_mapping (\n\
iconmappingid bigint  NOT NULL,\n\
iconmapid bigint  NOT NULL,\n\
iconid bigint  NOT NULL,\n\
inventory_link integer DEFAULT '0' NOT NULL,\n\
expression varchar(64) DEFAULT '' NOT NULL,\n\
sortorder integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (iconmappingid)\n\
);\n\
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);\n\
CREATE INDEX icon_mapping_2 ON icon_mapping (iconid);\n\
CREATE TABLE sysmaps (\n\
sysmapid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
width integer DEFAULT '600' NOT NULL,\n\
height integer DEFAULT '400' NOT NULL,\n\
backgroundid bigint  NULL,\n\
label_type integer DEFAULT '2' NOT NULL,\n\
label_location integer DEFAULT '0' NOT NULL,\n\
highlight integer DEFAULT '1' NOT NULL,\n\
expandproblem integer DEFAULT '1' NOT NULL,\n\
markelements integer DEFAULT '0' NOT NULL,\n\
show_unack integer DEFAULT '0' NOT NULL,\n\
grid_size integer DEFAULT '50' NOT NULL,\n\
grid_show integer DEFAULT '1' NOT NULL,\n\
grid_align integer DEFAULT '1' NOT NULL,\n\
label_format integer DEFAULT '0' NOT NULL,\n\
label_type_host integer DEFAULT '2' NOT NULL,\n\
label_type_hostgroup integer DEFAULT '2' NOT NULL,\n\
label_type_trigger integer DEFAULT '2' NOT NULL,\n\
label_type_map integer DEFAULT '2' NOT NULL,\n\
label_type_image integer DEFAULT '2' NOT NULL,\n\
label_string_host varchar(255) DEFAULT '' NOT NULL,\n\
label_string_hostgroup varchar(255) DEFAULT '' NOT NULL,\n\
label_string_trigger varchar(255) DEFAULT '' NOT NULL,\n\
label_string_map varchar(255) DEFAULT '' NOT NULL,\n\
label_string_image varchar(255) DEFAULT '' NOT NULL,\n\
iconmapid bigint  NULL,\n\
expand_macros integer DEFAULT '0' NOT NULL,\n\
severity_min integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapid)\n\
);\n\
CREATE INDEX sysmaps_1 ON sysmaps (name);\n\
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);\n\
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);\n\
CREATE TABLE sysmaps_elements (\n\
selementid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL,\n\
elementid bigint DEFAULT '0' NOT NULL,\n\
elementtype integer DEFAULT '0' NOT NULL,\n\
iconid_off bigint  NULL,\n\
iconid_on bigint  NULL,\n\
label varchar(2048) DEFAULT '' NOT NULL,\n\
label_location integer DEFAULT '-1' NOT NULL,\n\
x integer DEFAULT '0' NOT NULL,\n\
y integer DEFAULT '0' NOT NULL,\n\
iconid_disabled bigint  NULL,\n\
iconid_maintenance bigint  NULL,\n\
elementsubtype integer DEFAULT '0' NOT NULL,\n\
areatype integer DEFAULT '0' NOT NULL,\n\
width integer DEFAULT '200' NOT NULL,\n\
height integer DEFAULT '200' NOT NULL,\n\
viewtype integer DEFAULT '0' NOT NULL,\n\
use_iconmap integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (selementid)\n\
);\n\
CREATE INDEX sysmaps_elements_1 ON sysmaps_elements (sysmapid);\n\
CREATE INDEX sysmaps_elements_2 ON sysmaps_elements (iconid_off);\n\
CREATE INDEX sysmaps_elements_3 ON sysmaps_elements (iconid_on);\n\
CREATE INDEX sysmaps_elements_4 ON sysmaps_elements (iconid_disabled);\n\
CREATE INDEX sysmaps_elements_5 ON sysmaps_elements (iconid_maintenance);\n\
CREATE TABLE sysmaps_links (\n\
linkid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL,\n\
selementid1 bigint  NOT NULL,\n\
selementid2 bigint  NOT NULL,\n\
drawtype integer DEFAULT '0' NOT NULL,\n\
color varchar(6) DEFAULT '000000' NOT NULL,\n\
label varchar(2048) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX sysmaps_links_1 ON sysmaps_links (sysmapid);\n\
CREATE INDEX sysmaps_links_2 ON sysmaps_links (selementid1);\n\
CREATE INDEX sysmaps_links_3 ON sysmaps_links (selementid2);\n\
CREATE TABLE sysmaps_link_triggers (\n\
linktriggerid bigint  NOT NULL,\n\
linkid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL,\n\
drawtype integer DEFAULT '0' NOT NULL,\n\
color varchar(6) DEFAULT '000000' NOT NULL,\n\
PRIMARY KEY (linktriggerid)\n\
);\n\
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);\n\
CREATE INDEX sysmaps_link_triggers_2 ON sysmaps_link_triggers (triggerid);\n\
CREATE TABLE sysmap_element_url (\n\
sysmapelementurlid bigint  NOT NULL,\n\
selementid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (sysmapelementurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);\n\
CREATE TABLE sysmap_url (\n\
sysmapurlid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
elementtype integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);\n\
CREATE TABLE maintenances_hosts (\n\
maintenance_hostid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL,\n\
hostid bigint  NOT NULL,\n\
PRIMARY KEY (maintenance_hostid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);\n\
CREATE INDEX maintenances_hosts_2 ON maintenances_hosts (hostid);\n\
CREATE TABLE maintenances_groups (\n\
maintenance_groupid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL,\n\
groupid bigint  NOT NULL,\n\
PRIMARY KEY (maintenance_groupid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);\n\
CREATE INDEX maintenances_groups_2 ON maintenances_groups (groupid);\n\
CREATE TABLE timeperiods (\n\
timeperiodid bigint  NOT NULL,\n\
timeperiod_type integer DEFAULT '0' NOT NULL,\n\
every integer DEFAULT '0' NOT NULL,\n\
month integer DEFAULT '0' NOT NULL,\n\
dayofweek integer DEFAULT '0' NOT NULL,\n\
day integer DEFAULT '0' NOT NULL,\n\
start_time integer DEFAULT '0' NOT NULL,\n\
period integer DEFAULT '0' NOT NULL,\n\
start_date integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (timeperiodid)\n\
);\n\
CREATE TABLE maintenances_windows (\n\
maintenance_timeperiodid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL,\n\
timeperiodid bigint  NOT NULL,\n\
PRIMARY KEY (maintenance_timeperiodid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);\n\
CREATE INDEX maintenances_windows_2 ON maintenances_windows (timeperiodid);\n\
CREATE TABLE regexps (\n\
regexpid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
test_string text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (regexpid)\n\
);\n\
CREATE INDEX regexps_1 ON regexps (name);\n\
CREATE TABLE expressions (\n\
expressionid bigint  NOT NULL,\n\
regexpid bigint  NOT NULL,\n\
expression varchar(255) DEFAULT '' NOT NULL,\n\
expression_type integer DEFAULT '0' NOT NULL,\n\
exp_delimiter varchar(1) DEFAULT '' NOT NULL,\n\
case_sensitive integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (expressionid)\n\
);\n\
CREATE INDEX expressions_1 ON expressions (regexpid);\n\
CREATE TABLE nodes (\n\
nodeid integer  NOT NULL,\n\
name varchar(64) DEFAULT '0' NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
port integer DEFAULT '10051' NOT NULL,\n\
nodetype integer DEFAULT '0' NOT NULL,\n\
masterid integer  NULL,\n\
PRIMARY KEY (nodeid)\n\
);\n\
CREATE INDEX nodes_1 ON nodes (masterid);\n\
CREATE TABLE node_cksum (\n\
nodeid integer  NOT NULL,\n\
tablename varchar(64) DEFAULT '' NOT NULL,\n\
recordid bigint  NOT NULL,\n\
cksumtype integer DEFAULT '0' NOT NULL,\n\
cksum text DEFAULT '' NOT NULL,\n\
sync char(128) DEFAULT '' NOT NULL\n\
);\n\
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);\n\
CREATE TABLE ids (\n\
nodeid integer  NOT NULL,\n\
table_name varchar(64) DEFAULT '' NOT NULL,\n\
field_name varchar(64) DEFAULT '' NOT NULL,\n\
nextid bigint  NOT NULL,\n\
PRIMARY KEY (nodeid,table_name,field_name)\n\
);\n\
CREATE TABLE alerts (\n\
alertid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
eventid bigint  NOT NULL,\n\
userid bigint  NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
mediatypeid bigint  NULL,\n\
sendto varchar(100) DEFAULT '' NOT NULL,\n\
subject varchar(255) DEFAULT '' NOT NULL,\n\
message text DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
retries integer DEFAULT '0' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
esc_step integer DEFAULT '0' NOT NULL,\n\
alerttype integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (alertid)\n\
);\n\
CREATE INDEX alerts_1 ON alerts (actionid);\n\
CREATE INDEX alerts_2 ON alerts (clock);\n\
CREATE INDEX alerts_3 ON alerts (eventid);\n\
CREATE INDEX alerts_4 ON alerts (status,retries);\n\
CREATE INDEX alerts_5 ON alerts (mediatypeid);\n\
CREATE INDEX alerts_6 ON alerts (userid);\n\
CREATE TABLE history (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value numeric(16,4) DEFAULT '0.0000' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_1 ON history (itemid,clock);\n\
CREATE TABLE history_sync (\n\
id bigserial  NOT NULL,\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value numeric(16,4) DEFAULT '0.0000' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_sync_1 ON history_sync (nodeid,id);\n\
CREATE TABLE history_uint (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value numeric(20) DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);\n\
CREATE TABLE history_uint_sync (\n\
id bigserial  NOT NULL,\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value numeric(20) DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_uint_sync_1 ON history_uint_sync (nodeid,id);\n\
CREATE TABLE history_str (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_str_1 ON history_str (itemid,clock);\n\
CREATE TABLE history_str_sync (\n\
id bigserial  NOT NULL,\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_str_sync_1 ON history_str_sync (nodeid,id);\n\
CREATE TABLE history_log (\n\
id bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
timestamp integer DEFAULT '0' NOT NULL,\n\
source varchar(64) DEFAULT '' NOT NULL,\n\
severity integer DEFAULT '0' NOT NULL,\n\
value text DEFAULT '' NOT NULL,\n\
logeventid integer DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_log_1 ON history_log (itemid,clock);\n\
CREATE UNIQUE INDEX history_log_2 ON history_log (itemid,id);\n\
CREATE TABLE history_text (\n\
id bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value text DEFAULT '' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_text_1 ON history_text (itemid,clock);\n\
CREATE UNIQUE INDEX history_text_2 ON history_text (itemid,id);\n\
CREATE TABLE proxy_history (\n\
id bigserial  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
timestamp integer DEFAULT '0' NOT NULL,\n\
source varchar(64) DEFAULT '' NOT NULL,\n\
severity integer DEFAULT '0' NOT NULL,\n\
value text DEFAULT '' NOT NULL,\n\
logeventid integer DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
state integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_history_1 ON proxy_history (clock);\n\
CREATE TABLE proxy_dhistory (\n\
id bigserial  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
druleid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
port integer DEFAULT '0' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
dcheckid bigint  NULL,\n\
dns varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);\n\
CREATE TABLE events (\n\
eventid bigint  NOT NULL,\n\
source integer DEFAULT '0' NOT NULL,\n\
object integer DEFAULT '0' NOT NULL,\n\
objectid bigint DEFAULT '0' NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value integer DEFAULT '0' NOT NULL,\n\
acknowledged integer DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (eventid)\n\
);\n\
CREATE INDEX events_1 ON events (source,object,objectid,clock);\n\
CREATE INDEX events_2 ON events (source,object,clock);\n\
CREATE TABLE trends (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
num integer DEFAULT '0' NOT NULL,\n\
value_min numeric(16,4) DEFAULT '0.0000' NOT NULL,\n\
value_avg numeric(16,4) DEFAULT '0.0000' NOT NULL,\n\
value_max numeric(16,4) DEFAULT '0.0000' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE trends_uint (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
num integer DEFAULT '0' NOT NULL,\n\
value_min numeric(20) DEFAULT '0' NOT NULL,\n\
value_avg numeric(20) DEFAULT '0' NOT NULL,\n\
value_max numeric(20) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE acknowledges (\n\
acknowledgeid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
eventid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
message varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (acknowledgeid)\n\
);\n\
CREATE INDEX acknowledges_1 ON acknowledges (userid);\n\
CREATE INDEX acknowledges_2 ON acknowledges (eventid);\n\
CREATE INDEX acknowledges_3 ON acknowledges (clock);\n\
CREATE TABLE auditlog (\n\
auditid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
action integer DEFAULT '0' NOT NULL,\n\
resourcetype integer DEFAULT '0' NOT NULL,\n\
details varchar(128)  DEFAULT '0' NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
resourceid bigint DEFAULT '0' NOT NULL,\n\
resourcename varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditid)\n\
);\n\
CREATE INDEX auditlog_1 ON auditlog (userid,clock);\n\
CREATE INDEX auditlog_2 ON auditlog (clock);\n\
CREATE TABLE auditlog_details (\n\
auditdetailid bigint  NOT NULL,\n\
auditid bigint  NOT NULL,\n\
table_name varchar(64) DEFAULT '' NOT NULL,\n\
field_name varchar(64) DEFAULT '' NOT NULL,\n\
oldvalue text DEFAULT '' NOT NULL,\n\
newvalue text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditdetailid)\n\
);\n\
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);\n\
CREATE TABLE service_alarms (\n\
servicealarmid bigint  NOT NULL,\n\
serviceid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (servicealarmid)\n\
);\n\
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);\n\
CREATE INDEX service_alarms_2 ON service_alarms (clock);\n\
CREATE TABLE autoreg_host (\n\
autoreg_hostid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL,\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
listen_ip varchar(39) DEFAULT '' NOT NULL,\n\
listen_port integer DEFAULT '0' NOT NULL,\n\
listen_dns varchar(64) DEFAULT '' NOT NULL,\n\
host_metadata varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (autoreg_hostid)\n\
);\n\
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);\n\
CREATE TABLE proxy_autoreg_host (\n\
id bigserial  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
listen_ip varchar(39) DEFAULT '' NOT NULL,\n\
listen_port integer DEFAULT '0' NOT NULL,\n\
listen_dns varchar(64) DEFAULT '' NOT NULL,\n\
host_metadata varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);\n\
CREATE TABLE dhosts (\n\
dhostid bigint  NOT NULL,\n\
druleid bigint  NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
lastup integer DEFAULT '0' NOT NULL,\n\
lastdown integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (dhostid)\n\
);\n\
CREATE INDEX dhosts_1 ON dhosts (druleid);\n\
CREATE TABLE dservices (\n\
dserviceid bigint  NOT NULL,\n\
dhostid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
port integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
lastup integer DEFAULT '0' NOT NULL,\n\
lastdown integer DEFAULT '0' NOT NULL,\n\
dcheckid bigint  NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
dns varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dserviceid)\n\
);\n\
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,type,key_,ip,port);\n\
CREATE INDEX dservices_2 ON dservices (dhostid);\n\
CREATE TABLE escalations (\n\
escalationid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
triggerid bigint  NULL,\n\
eventid bigint  NULL,\n\
r_eventid bigint  NULL,\n\
nextcheck integer DEFAULT '0' NOT NULL,\n\
esc_step integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
itemid bigint  NULL,\n\
PRIMARY KEY (escalationid)\n\
);\n\
CREATE UNIQUE INDEX escalations_1 ON escalations (actionid,triggerid,itemid,escalationid);\n\
CREATE TABLE globalvars (\n\
globalvarid bigint  NOT NULL,\n\
snmp_lastsize integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (globalvarid)\n\
);\n\
CREATE TABLE graph_discovery (\n\
graphdiscoveryid bigint  NOT NULL,\n\
graphid bigint  NOT NULL,\n\
parent_graphid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (graphdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX graph_discovery_1 ON graph_discovery (graphid,parent_graphid);\n\
CREATE INDEX graph_discovery_2 ON graph_discovery (parent_graphid);\n\
CREATE TABLE host_inventory (\n\
hostid bigint  NOT NULL,\n\
inventory_mode integer DEFAULT '0' NOT NULL,\n\
type varchar(64) DEFAULT '' NOT NULL,\n\
type_full varchar(64) DEFAULT '' NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
alias varchar(64) DEFAULT '' NOT NULL,\n\
os varchar(64) DEFAULT '' NOT NULL,\n\
os_full varchar(255) DEFAULT '' NOT NULL,\n\
os_short varchar(64) DEFAULT '' NOT NULL,\n\
serialno_a varchar(64) DEFAULT '' NOT NULL,\n\
serialno_b varchar(64) DEFAULT '' NOT NULL,\n\
tag varchar(64) DEFAULT '' NOT NULL,\n\
asset_tag varchar(64) DEFAULT '' NOT NULL,\n\
macaddress_a varchar(64) DEFAULT '' NOT NULL,\n\
macaddress_b varchar(64) DEFAULT '' NOT NULL,\n\
hardware varchar(255) DEFAULT '' NOT NULL,\n\
hardware_full text DEFAULT '' NOT NULL,\n\
software varchar(255) DEFAULT '' NOT NULL,\n\
software_full text DEFAULT '' NOT NULL,\n\
software_app_a varchar(64) DEFAULT '' NOT NULL,\n\
software_app_b varchar(64) DEFAULT '' NOT NULL,\n\
software_app_c varchar(64) DEFAULT '' NOT NULL,\n\
software_app_d varchar(64) DEFAULT '' NOT NULL,\n\
software_app_e varchar(64) DEFAULT '' NOT NULL,\n\
contact text DEFAULT '' NOT NULL,\n\
location text DEFAULT '' NOT NULL,\n\
location_lat varchar(16) DEFAULT '' NOT NULL,\n\
location_lon varchar(16) DEFAULT '' NOT NULL,\n\
notes text DEFAULT '' NOT NULL,\n\
chassis varchar(64) DEFAULT '' NOT NULL,\n\
model varchar(64) DEFAULT '' NOT NULL,\n\
hw_arch varchar(32) DEFAULT '' NOT NULL,\n\
vendor varchar(64) DEFAULT '' NOT NULL,\n\
contract_number varchar(64) DEFAULT '' NOT NULL,\n\
installer_name varchar(64) DEFAULT '' NOT NULL,\n\
deployment_status varchar(64) DEFAULT '' NOT NULL,\n\
url_a varchar(255) DEFAULT '' NOT NULL,\n\
url_b varchar(255) DEFAULT '' NOT NULL,\n\
url_c varchar(255) DEFAULT '' NOT NULL,\n\
host_networks text DEFAULT '' NOT NULL,\n\
host_netmask varchar(39) DEFAULT '' NOT NULL,\n\
host_router varchar(39) DEFAULT '' NOT NULL,\n\
oob_ip varchar(39) DEFAULT '' NOT NULL,\n\
oob_netmask varchar(39) DEFAULT '' NOT NULL,\n\
oob_router varchar(39) DEFAULT '' NOT NULL,\n\
date_hw_purchase varchar(64) DEFAULT '' NOT NULL,\n\
date_hw_install varchar(64) DEFAULT '' NOT NULL,\n\
date_hw_expiry varchar(64) DEFAULT '' NOT NULL,\n\
date_hw_decomm varchar(64) DEFAULT '' NOT NULL,\n\
site_address_a varchar(128) DEFAULT '' NOT NULL,\n\
site_address_b varchar(128) DEFAULT '' NOT NULL,\n\
site_address_c varchar(128) DEFAULT '' NOT NULL,\n\
site_city varchar(128) DEFAULT '' NOT NULL,\n\
site_state varchar(64) DEFAULT '' NOT NULL,\n\
site_country varchar(64) DEFAULT '' NOT NULL,\n\
site_zip varchar(64) DEFAULT '' NOT NULL,\n\
site_rack varchar(128) DEFAULT '' NOT NULL,\n\
site_notes text DEFAULT '' NOT NULL,\n\
poc_1_name varchar(128) DEFAULT '' NOT NULL,\n\
poc_1_email varchar(128) DEFAULT '' NOT NULL,\n\
poc_1_phone_a varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_phone_b varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_cell varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_screen varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_notes text DEFAULT '' NOT NULL,\n\
poc_2_name varchar(128) DEFAULT '' NOT NULL,\n\
poc_2_email varchar(128) DEFAULT '' NOT NULL,\n\
poc_2_phone_a varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_phone_b varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_cell varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_screen varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_notes text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE housekeeper (\n\
housekeeperid bigint  NOT NULL,\n\
tablename varchar(64) DEFAULT '' NOT NULL,\n\
field varchar(64) DEFAULT '' NOT NULL,\n\
value bigint  NOT NULL,\n\
PRIMARY KEY (housekeeperid)\n\
);\n\
CREATE TABLE images (\n\
imageid bigint  NOT NULL,\n\
imagetype integer DEFAULT '0' NOT NULL,\n\
name varchar(64) DEFAULT '0' NOT NULL,\n\
image bytea DEFAULT '' NOT NULL,\n\
PRIMARY KEY (imageid)\n\
);\n\
CREATE INDEX images_1 ON images (imagetype,name);\n\
CREATE TABLE item_discovery (\n\
itemdiscoveryid bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
parent_itemid bigint  NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
lastcheck integer DEFAULT '0' NOT NULL,\n\
ts_delete integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);\n\
CREATE INDEX item_discovery_2 ON item_discovery (parent_itemid);\n\
CREATE TABLE host_discovery (\n\
hostid bigint  NOT NULL,\n\
parent_hostid bigint  NULL,\n\
parent_itemid bigint  NULL,\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
lastcheck integer DEFAULT '0' NOT NULL,\n\
ts_delete integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE interface_discovery (\n\
interfaceid bigint  NOT NULL,\n\
parent_interfaceid bigint  NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE TABLE profiles (\n\
profileid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
idx varchar(96) DEFAULT '' NOT NULL,\n\
idx2 bigint DEFAULT '0' NOT NULL,\n\
value_id bigint DEFAULT '0' NOT NULL,\n\
value_int integer DEFAULT '0' NOT NULL,\n\
value_str varchar(255) DEFAULT '' NOT NULL,\n\
source varchar(96) DEFAULT '' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (profileid)\n\
);\n\
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);\n\
CREATE INDEX profiles_2 ON profiles (userid,profileid);\n\
CREATE TABLE sessions (\n\
sessionid varchar(32) DEFAULT '' NOT NULL,\n\
userid bigint  NOT NULL,\n\
lastaccess integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sessionid)\n\
);\n\
CREATE INDEX sessions_1 ON sessions (userid,status);\n\
CREATE TABLE trigger_discovery (\n\
triggerdiscoveryid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL,\n\
parent_triggerid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (triggerdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX trigger_discovery_1 ON trigger_discovery (triggerid,parent_triggerid);\n\
CREATE INDEX trigger_discovery_2 ON trigger_discovery (parent_triggerid);\n\
CREATE TABLE user_history (\n\
userhistoryid bigint  NOT NULL,\n\
userid bigint  NOT NULL,\n\
title1 varchar(255) DEFAULT '' NOT NULL,\n\
url1 varchar(255) DEFAULT '' NOT NULL,\n\
title2 varchar(255) DEFAULT '' NOT NULL,\n\
url2 varchar(255) DEFAULT '' NOT NULL,\n\
title3 varchar(255) DEFAULT '' NOT NULL,\n\
url3 varchar(255) DEFAULT '' NOT NULL,\n\
title4 varchar(255) DEFAULT '' NOT NULL,\n\
url4 varchar(255) DEFAULT '' NOT NULL,\n\
title5 varchar(255) DEFAULT '' NOT NULL,\n\
url5 varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (userhistoryid)\n\
);\n\
CREATE UNIQUE INDEX user_history_1 ON user_history (userid);\n\
CREATE TABLE application_template (\n\
application_templateid bigint  NOT NULL,\n\
applicationid bigint  NOT NULL,\n\
templateid bigint  NOT NULL,\n\
PRIMARY KEY (application_templateid)\n\
);\n\
CREATE UNIQUE INDEX application_template_1 ON application_template (applicationid,templateid);\n\
CREATE INDEX application_template_2 ON application_template (templateid);\n\
CREATE TABLE dbversion (\n\
mandatory integer DEFAULT '0' NOT NULL,\n\
optional integer DEFAULT '0' NOT NULL\n\
);\n\
INSERT INTO dbversion VALUES ('2020000','2020001');\n\
";
const char	*const db_schema_fkeys[] = {
	"ALTER TABLE ONLY hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE ONLY hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid)",
	"ALTER TABLE ONLY hosts ADD CONSTRAINT c_hosts_3 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY group_prototype ADD CONSTRAINT c_group_prototype_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY group_prototype ADD CONSTRAINT c_group_prototype_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE ONLY group_prototype ADD CONSTRAINT c_group_prototype_3 FOREIGN KEY (templateid) REFERENCES group_prototype (group_prototypeid) ON DELETE CASCADE",
	"ALTER TABLE ONLY group_discovery ADD CONSTRAINT c_group_discovery_1 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE ONLY group_discovery ADD CONSTRAINT c_group_discovery_2 FOREIGN KEY (parent_group_prototypeid) REFERENCES group_prototype (group_prototypeid)",
	"ALTER TABLE ONLY screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE",
	"ALTER TABLE ONLY slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE",
	"ALTER TABLE ONLY slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE",
	"ALTER TABLE ONLY drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE ONLY dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE",
	"ALTER TABLE ONLY applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid)",
	"ALTER TABLE ONLY httptest ADD CONSTRAINT c_httptest_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY httptest ADD CONSTRAINT c_httptest_3 FOREIGN KEY (templateid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE ONLY httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE ONLY interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid)",
	"ALTER TABLE ONLY items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid)",
	"ALTER TABLE ONLY httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE",
	"ALTER TABLE ONLY httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE",
	"ALTER TABLE ONLY httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE",
	"ALTER TABLE ONLY users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE ONLY scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE ONLY operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid)",
	"ALTER TABLE ONLY opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE ONLY opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid)",
	"ALTER TABLE ONLY opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid)",
	"ALTER TABLE ONLY opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE ONLY opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE ONLY opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid)",
	"ALTER TABLE ONLY optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid)",
	"ALTER TABLE ONLY opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE ONLY config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid)",
	"ALTER TABLE ONLY config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid)",
	"ALTER TABLE ONLY triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE ONLY graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid)",
	"ALTER TABLE ONLY graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid)",
	"ALTER TABLE ONLY graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE ONLY graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE ONLY hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE",
	"ALTER TABLE ONLY media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE",
	"ALTER TABLE ONLY rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE",
	"ALTER TABLE ONLY rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE ONLY services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid)",
	"ALTER TABLE ONLY icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE",
	"ALTER TABLE ONLY icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid)",
	"ALTER TABLE ONLY sysmaps ADD CONSTRAINT c_sysmaps_1 FOREIGN KEY (backgroundid) REFERENCES images (imageid)",
	"ALTER TABLE ONLY sysmaps ADD CONSTRAINT c_sysmaps_2 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid)",
	"ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_2 FOREIGN KEY (iconid_off) REFERENCES images (imageid)",
	"ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_3 FOREIGN KEY (iconid_on) REFERENCES images (imageid)",
	"ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_4 FOREIGN KEY (iconid_disabled) REFERENCES images (imageid)",
	"ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_5 FOREIGN KEY (iconid_maintenance) REFERENCES images (imageid)",
	"ALTER TABLE ONLY sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE",
	"ALTER TABLE ONLY maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE",
	"ALTER TABLE ONLY maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE",
	"ALTER TABLE ONLY expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE",
	"ALTER TABLE ONLY nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid)",
	"ALTER TABLE ONLY node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE",
	"ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE",
	"ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE",
	"ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE",
	"ALTER TABLE ONLY acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE",
	"ALTER TABLE ONLY auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE",
	"ALTER TABLE ONLY service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE",
	"ALTER TABLE ONLY dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE",
	"ALTER TABLE ONLY graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE ONLY graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE",
	"ALTER TABLE ONLY host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE",
	"ALTER TABLE ONLY host_discovery ADD CONSTRAINT c_host_discovery_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE",
	"ALTER TABLE ONLY host_discovery ADD CONSTRAINT c_host_discovery_2 FOREIGN KEY (parent_hostid) REFERENCES hosts (hostid)",
	"ALTER TABLE ONLY host_discovery ADD CONSTRAINT c_host_discovery_3 FOREIGN KEY (parent_itemid) REFERENCES items (itemid)",
	"ALTER TABLE ONLY interface_discovery ADD CONSTRAINT c_interface_discovery_1 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY interface_discovery ADD CONSTRAINT c_interface_discovery_2 FOREIGN KEY (parent_interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE",
	"ALTER TABLE ONLY profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE",
	"ALTER TABLE ONLY user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE",
	"ALTER TABLE ONLY application_template ADD CONSTRAINT c_application_template_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	"ALTER TABLE ONLY application_template ADD CONSTRAINT c_application_template_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE",
	NULL
};
const char	*const db_schema_fkeys_drop[] = {
	"ALTER TABLE ONLY hosts DROP CONSTRAINT c_hosts_1",
	"ALTER TABLE ONLY hosts DROP CONSTRAINT c_hosts_2",
	"ALTER TABLE ONLY hosts DROP CONSTRAINT c_hosts_3",
	"ALTER TABLE ONLY group_prototype DROP CONSTRAINT c_group_prototype_1",
	"ALTER TABLE ONLY group_prototype DROP CONSTRAINT c_group_prototype_2",
	"ALTER TABLE ONLY group_prototype DROP CONSTRAINT c_group_prototype_3",
	"ALTER TABLE ONLY group_discovery DROP CONSTRAINT c_group_discovery_1",
	"ALTER TABLE ONLY group_discovery DROP CONSTRAINT c_group_discovery_2",
	"ALTER TABLE ONLY screens DROP CONSTRAINT c_screens_1",
	"ALTER TABLE ONLY screens_items DROP CONSTRAINT c_screens_items_1",
	"ALTER TABLE ONLY slides DROP CONSTRAINT c_slides_1",
	"ALTER TABLE ONLY slides DROP CONSTRAINT c_slides_2",
	"ALTER TABLE ONLY drules DROP CONSTRAINT c_drules_1",
	"ALTER TABLE ONLY dchecks DROP CONSTRAINT c_dchecks_1",
	"ALTER TABLE ONLY applications DROP CONSTRAINT c_applications_1",
	"ALTER TABLE ONLY httptest DROP CONSTRAINT c_httptest_1",
	"ALTER TABLE ONLY httptest DROP CONSTRAINT c_httptest_2",
	"ALTER TABLE ONLY httptest DROP CONSTRAINT c_httptest_3",
	"ALTER TABLE ONLY httpstep DROP CONSTRAINT c_httpstep_1",
	"ALTER TABLE ONLY interface DROP CONSTRAINT c_interface_1",
	"ALTER TABLE ONLY items DROP CONSTRAINT c_items_1",
	"ALTER TABLE ONLY items DROP CONSTRAINT c_items_2",
	"ALTER TABLE ONLY items DROP CONSTRAINT c_items_3",
	"ALTER TABLE ONLY items DROP CONSTRAINT c_items_4",
	"ALTER TABLE ONLY httpstepitem DROP CONSTRAINT c_httpstepitem_1",
	"ALTER TABLE ONLY httpstepitem DROP CONSTRAINT c_httpstepitem_2",
	"ALTER TABLE ONLY httptestitem DROP CONSTRAINT c_httptestitem_1",
	"ALTER TABLE ONLY httptestitem DROP CONSTRAINT c_httptestitem_2",
	"ALTER TABLE ONLY users_groups DROP CONSTRAINT c_users_groups_1",
	"ALTER TABLE ONLY users_groups DROP CONSTRAINT c_users_groups_2",
	"ALTER TABLE ONLY scripts DROP CONSTRAINT c_scripts_1",
	"ALTER TABLE ONLY scripts DROP CONSTRAINT c_scripts_2",
	"ALTER TABLE ONLY operations DROP CONSTRAINT c_operations_1",
	"ALTER TABLE ONLY opmessage DROP CONSTRAINT c_opmessage_1",
	"ALTER TABLE ONLY opmessage DROP CONSTRAINT c_opmessage_2",
	"ALTER TABLE ONLY opmessage_grp DROP CONSTRAINT c_opmessage_grp_1",
	"ALTER TABLE ONLY opmessage_grp DROP CONSTRAINT c_opmessage_grp_2",
	"ALTER TABLE ONLY opmessage_usr DROP CONSTRAINT c_opmessage_usr_1",
	"ALTER TABLE ONLY opmessage_usr DROP CONSTRAINT c_opmessage_usr_2",
	"ALTER TABLE ONLY opcommand DROP CONSTRAINT c_opcommand_1",
	"ALTER TABLE ONLY opcommand DROP CONSTRAINT c_opcommand_2",
	"ALTER TABLE ONLY opcommand_hst DROP CONSTRAINT c_opcommand_hst_1",
	"ALTER TABLE ONLY opcommand_hst DROP CONSTRAINT c_opcommand_hst_2",
	"ALTER TABLE ONLY opcommand_grp DROP CONSTRAINT c_opcommand_grp_1",
	"ALTER TABLE ONLY opcommand_grp DROP CONSTRAINT c_opcommand_grp_2",
	"ALTER TABLE ONLY opgroup DROP CONSTRAINT c_opgroup_1",
	"ALTER TABLE ONLY opgroup DROP CONSTRAINT c_opgroup_2",
	"ALTER TABLE ONLY optemplate DROP CONSTRAINT c_optemplate_1",
	"ALTER TABLE ONLY optemplate DROP CONSTRAINT c_optemplate_2",
	"ALTER TABLE ONLY opconditions DROP CONSTRAINT c_opconditions_1",
	"ALTER TABLE ONLY conditions DROP CONSTRAINT c_conditions_1",
	"ALTER TABLE ONLY config DROP CONSTRAINT c_config_1",
	"ALTER TABLE ONLY config DROP CONSTRAINT c_config_2",
	"ALTER TABLE ONLY triggers DROP CONSTRAINT c_triggers_1",
	"ALTER TABLE ONLY trigger_depends DROP CONSTRAINT c_trigger_depends_1",
	"ALTER TABLE ONLY trigger_depends DROP CONSTRAINT c_trigger_depends_2",
	"ALTER TABLE ONLY functions DROP CONSTRAINT c_functions_1",
	"ALTER TABLE ONLY functions DROP CONSTRAINT c_functions_2",
	"ALTER TABLE ONLY graphs DROP CONSTRAINT c_graphs_1",
	"ALTER TABLE ONLY graphs DROP CONSTRAINT c_graphs_2",
	"ALTER TABLE ONLY graphs DROP CONSTRAINT c_graphs_3",
	"ALTER TABLE ONLY graphs_items DROP CONSTRAINT c_graphs_items_1",
	"ALTER TABLE ONLY graphs_items DROP CONSTRAINT c_graphs_items_2",
	"ALTER TABLE ONLY hostmacro DROP CONSTRAINT c_hostmacro_1",
	"ALTER TABLE ONLY hosts_groups DROP CONSTRAINT c_hosts_groups_1",
	"ALTER TABLE ONLY hosts_groups DROP CONSTRAINT c_hosts_groups_2",
	"ALTER TABLE ONLY hosts_templates DROP CONSTRAINT c_hosts_templates_1",
	"ALTER TABLE ONLY hosts_templates DROP CONSTRAINT c_hosts_templates_2",
	"ALTER TABLE ONLY items_applications DROP CONSTRAINT c_items_applications_1",
	"ALTER TABLE ONLY items_applications DROP CONSTRAINT c_items_applications_2",
	"ALTER TABLE ONLY mappings DROP CONSTRAINT c_mappings_1",
	"ALTER TABLE ONLY media DROP CONSTRAINT c_media_1",
	"ALTER TABLE ONLY media DROP CONSTRAINT c_media_2",
	"ALTER TABLE ONLY rights DROP CONSTRAINT c_rights_1",
	"ALTER TABLE ONLY rights DROP CONSTRAINT c_rights_2",
	"ALTER TABLE ONLY services DROP CONSTRAINT c_services_1",
	"ALTER TABLE ONLY services_links DROP CONSTRAINT c_services_links_1",
	"ALTER TABLE ONLY services_links DROP CONSTRAINT c_services_links_2",
	"ALTER TABLE ONLY services_times DROP CONSTRAINT c_services_times_1",
	"ALTER TABLE ONLY icon_map DROP CONSTRAINT c_icon_map_1",
	"ALTER TABLE ONLY icon_mapping DROP CONSTRAINT c_icon_mapping_1",
	"ALTER TABLE ONLY icon_mapping DROP CONSTRAINT c_icon_mapping_2",
	"ALTER TABLE ONLY sysmaps DROP CONSTRAINT c_sysmaps_1",
	"ALTER TABLE ONLY sysmaps DROP CONSTRAINT c_sysmaps_2",
	"ALTER TABLE ONLY sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_1",
	"ALTER TABLE ONLY sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_2",
	"ALTER TABLE ONLY sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_3",
	"ALTER TABLE ONLY sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_4",
	"ALTER TABLE ONLY sysmaps_elements DROP CONSTRAINT c_sysmaps_elements_5",
	"ALTER TABLE ONLY sysmaps_links DROP CONSTRAINT c_sysmaps_links_1",
	"ALTER TABLE ONLY sysmaps_links DROP CONSTRAINT c_sysmaps_links_2",
	"ALTER TABLE ONLY sysmaps_links DROP CONSTRAINT c_sysmaps_links_3",
	"ALTER TABLE ONLY sysmaps_link_triggers DROP CONSTRAINT c_sysmaps_link_triggers_1",
	"ALTER TABLE ONLY sysmaps_link_triggers DROP CONSTRAINT c_sysmaps_link_triggers_2",
	"ALTER TABLE ONLY sysmap_element_url DROP CONSTRAINT c_sysmap_element_url_1",
	"ALTER TABLE ONLY sysmap_url DROP CONSTRAINT c_sysmap_url_1",
	"ALTER TABLE ONLY maintenances_hosts DROP CONSTRAINT c_maintenances_hosts_1",
	"ALTER TABLE ONLY maintenances_hosts DROP CONSTRAINT c_maintenances_hosts_2",
	"ALTER TABLE ONLY maintenances_groups DROP CONSTRAINT c_maintenances_groups_1",
	"ALTER TABLE ONLY maintenances_groups DROP CONSTRAINT c_maintenances_groups_2",
	"ALTER TABLE ONLY maintenances_windows DROP CONSTRAINT c_maintenances_windows_1",
	"ALTER TABLE ONLY maintenances_windows DROP CONSTRAINT c_maintenances_windows_2",
	"ALTER TABLE ONLY expressions DROP CONSTRAINT c_expressions_1",
	"ALTER TABLE ONLY nodes DROP CONSTRAINT c_nodes_1",
	"ALTER TABLE ONLY node_cksum DROP CONSTRAINT c_node_cksum_1",
	"ALTER TABLE ONLY alerts DROP CONSTRAINT c_alerts_1",
	"ALTER TABLE ONLY alerts DROP CONSTRAINT c_alerts_2",
	"ALTER TABLE ONLY alerts DROP CONSTRAINT c_alerts_3",
	"ALTER TABLE ONLY alerts DROP CONSTRAINT c_alerts_4",
	"ALTER TABLE ONLY acknowledges DROP CONSTRAINT c_acknowledges_1",
	"ALTER TABLE ONLY acknowledges DROP CONSTRAINT c_acknowledges_2",
	"ALTER TABLE ONLY auditlog DROP CONSTRAINT c_auditlog_1",
	"ALTER TABLE ONLY auditlog_details DROP CONSTRAINT c_auditlog_details_1",
	"ALTER TABLE ONLY service_alarms DROP CONSTRAINT c_service_alarms_1",
	"ALTER TABLE ONLY autoreg_host DROP CONSTRAINT c_autoreg_host_1",
	"ALTER TABLE ONLY dhosts DROP CONSTRAINT c_dhosts_1",
	"ALTER TABLE ONLY dservices DROP CONSTRAINT c_dservices_1",
	"ALTER TABLE ONLY dservices DROP CONSTRAINT c_dservices_2",
	"ALTER TABLE ONLY graph_discovery DROP CONSTRAINT c_graph_discovery_1",
	"ALTER TABLE ONLY graph_discovery DROP CONSTRAINT c_graph_discovery_2",
	"ALTER TABLE ONLY host_inventory DROP CONSTRAINT c_host_inventory_1",
	"ALTER TABLE ONLY item_discovery DROP CONSTRAINT c_item_discovery_1",
	"ALTER TABLE ONLY item_discovery DROP CONSTRAINT c_item_discovery_2",
	"ALTER TABLE ONLY host_discovery DROP CONSTRAINT c_host_discovery_1",
	"ALTER TABLE ONLY host_discovery DROP CONSTRAINT c_host_discovery_2",
	"ALTER TABLE ONLY host_discovery DROP CONSTRAINT c_host_discovery_3",
	"ALTER TABLE ONLY interface_discovery DROP CONSTRAINT c_interface_discovery_1",
	"ALTER TABLE ONLY interface_discovery DROP CONSTRAINT c_interface_discovery_2",
	"ALTER TABLE ONLY profiles DROP CONSTRAINT c_profiles_1",
	"ALTER TABLE ONLY sessions DROP CONSTRAINT c_sessions_1",
	"ALTER TABLE ONLY trigger_discovery DROP CONSTRAINT c_trigger_discovery_1",
	"ALTER TABLE ONLY trigger_discovery DROP CONSTRAINT c_trigger_discovery_2",
	"ALTER TABLE ONLY user_history DROP CONSTRAINT c_user_history_1",
	"ALTER TABLE ONLY application_template DROP CONSTRAINT c_application_template_1",
	"ALTER TABLE ONLY application_template DROP CONSTRAINT c_application_template_2",
	NULL
};
#elif defined(HAVE_SQLITE3)
const char	*const db_schema = "\
CREATE TABLE maintenances (\n\
maintenanceid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
maintenance_type integer DEFAULT '0' NOT NULL,\n\
description text DEFAULT '' NOT NULL,\n\
active_since integer DEFAULT '0' NOT NULL,\n\
active_till integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (maintenanceid)\n\
);\n\
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);\n\
CREATE TABLE hosts (\n\
hostid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL REFERENCES hosts (hostid),\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
disable_until integer DEFAULT '0' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
available integer DEFAULT '0' NOT NULL,\n\
errors_from integer DEFAULT '0' NOT NULL,\n\
lastaccess integer DEFAULT '0' NOT NULL,\n\
ipmi_authtype integer DEFAULT '0' NOT NULL,\n\
ipmi_privilege integer DEFAULT '2' NOT NULL,\n\
ipmi_username varchar(16) DEFAULT '' NOT NULL,\n\
ipmi_password varchar(20) DEFAULT '' NOT NULL,\n\
ipmi_disable_until integer DEFAULT '0' NOT NULL,\n\
ipmi_available integer DEFAULT '0' NOT NULL,\n\
snmp_disable_until integer DEFAULT '0' NOT NULL,\n\
snmp_available integer DEFAULT '0' NOT NULL,\n\
maintenanceid bigint  NULL REFERENCES maintenances (maintenanceid),\n\
maintenance_status integer DEFAULT '0' NOT NULL,\n\
maintenance_type integer DEFAULT '0' NOT NULL,\n\
maintenance_from integer DEFAULT '0' NOT NULL,\n\
ipmi_errors_from integer DEFAULT '0' NOT NULL,\n\
snmp_errors_from integer DEFAULT '0' NOT NULL,\n\
ipmi_error varchar(128) DEFAULT '' NOT NULL,\n\
snmp_error varchar(128) DEFAULT '' NOT NULL,\n\
jmx_disable_until integer DEFAULT '0' NOT NULL,\n\
jmx_available integer DEFAULT '0' NOT NULL,\n\
jmx_errors_from integer DEFAULT '0' NOT NULL,\n\
jmx_error varchar(128) DEFAULT '' NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
templateid bigint  NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE INDEX hosts_1 ON hosts (host);\n\
CREATE INDEX hosts_2 ON hosts (status);\n\
CREATE INDEX hosts_3 ON hosts (proxy_hostid);\n\
CREATE INDEX hosts_4 ON hosts (name);\n\
CREATE INDEX hosts_5 ON hosts (maintenanceid);\n\
CREATE TABLE groups (\n\
groupid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
internal integer DEFAULT '0' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE INDEX groups_1 ON groups (name);\n\
CREATE TABLE group_prototype (\n\
group_prototypeid bigint  NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
groupid bigint  NULL REFERENCES groups (groupid),\n\
templateid bigint  NULL REFERENCES group_prototype (group_prototypeid) ON DELETE CASCADE,\n\
PRIMARY KEY (group_prototypeid)\n\
);\n\
CREATE INDEX group_prototype_1 ON group_prototype (hostid);\n\
CREATE TABLE group_discovery (\n\
groupid bigint  NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,\n\
parent_group_prototypeid bigint  NOT NULL REFERENCES group_prototype (group_prototypeid),\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
lastcheck integer DEFAULT '0' NOT NULL,\n\
ts_delete integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (groupid)\n\
);\n\
CREATE TABLE screens (\n\
screenid bigint  NOT NULL,\n\
name varchar(255)  NOT NULL,\n\
hsize integer DEFAULT '1' NOT NULL,\n\
vsize integer DEFAULT '1' NOT NULL,\n\
templateid bigint  NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
PRIMARY KEY (screenid)\n\
);\n\
CREATE INDEX screens_1 ON screens (templateid);\n\
CREATE TABLE screens_items (\n\
screenitemid bigint  NOT NULL,\n\
screenid bigint  NOT NULL REFERENCES screens (screenid) ON DELETE CASCADE,\n\
resourcetype integer DEFAULT '0' NOT NULL,\n\
resourceid bigint DEFAULT '0' NOT NULL,\n\
width integer DEFAULT '320' NOT NULL,\n\
height integer DEFAULT '200' NOT NULL,\n\
x integer DEFAULT '0' NOT NULL,\n\
y integer DEFAULT '0' NOT NULL,\n\
colspan integer DEFAULT '0' NOT NULL,\n\
rowspan integer DEFAULT '0' NOT NULL,\n\
elements integer DEFAULT '25' NOT NULL,\n\
valign integer DEFAULT '0' NOT NULL,\n\
halign integer DEFAULT '0' NOT NULL,\n\
style integer DEFAULT '0' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
dynamic integer DEFAULT '0' NOT NULL,\n\
sort_triggers integer DEFAULT '0' NOT NULL,\n\
application varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (screenitemid)\n\
);\n\
CREATE INDEX screens_items_1 ON screens_items (screenid);\n\
CREATE TABLE slideshows (\n\
slideshowid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
delay integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideshowid)\n\
);\n\
CREATE TABLE slides (\n\
slideid bigint  NOT NULL,\n\
slideshowid bigint  NOT NULL REFERENCES slideshows (slideshowid) ON DELETE CASCADE,\n\
screenid bigint  NOT NULL REFERENCES screens (screenid) ON DELETE CASCADE,\n\
step integer DEFAULT '0' NOT NULL,\n\
delay integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (slideid)\n\
);\n\
CREATE INDEX slides_1 ON slides (slideshowid);\n\
CREATE INDEX slides_2 ON slides (screenid);\n\
CREATE TABLE drules (\n\
druleid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL REFERENCES hosts (hostid),\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
iprange varchar(255) DEFAULT '' NOT NULL,\n\
delay integer DEFAULT '3600' NOT NULL,\n\
nextcheck integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (druleid)\n\
);\n\
CREATE INDEX drules_1 ON drules (proxy_hostid);\n\
CREATE TABLE dchecks (\n\
dcheckid bigint  NOT NULL,\n\
druleid bigint  NOT NULL REFERENCES drules (druleid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
snmp_community varchar(255) DEFAULT '' NOT NULL,\n\
ports varchar(255) DEFAULT '0' NOT NULL,\n\
snmpv3_securityname varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_securitylevel integer DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_privpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
uniq integer DEFAULT '0' NOT NULL,\n\
snmpv3_authprotocol integer DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol integer DEFAULT '0' NOT NULL,\n\
snmpv3_contextname varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dcheckid)\n\
);\n\
CREATE INDEX dchecks_1 ON dchecks (druleid);\n\
CREATE TABLE applications (\n\
applicationid bigint  NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (applicationid)\n\
);\n\
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);\n\
CREATE TABLE httptest (\n\
httptestid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
applicationid bigint  NULL REFERENCES applications (applicationid),\n\
nextcheck integer DEFAULT '0' NOT NULL,\n\
delay integer DEFAULT '60' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
variables text DEFAULT '' NOT NULL,\n\
agent varchar(255) DEFAULT '' NOT NULL,\n\
authentication integer DEFAULT '0' NOT NULL,\n\
http_user varchar(64) DEFAULT '' NOT NULL,\n\
http_password varchar(64) DEFAULT '' NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
templateid bigint  NULL REFERENCES httptest (httptestid) ON DELETE CASCADE,\n\
http_proxy varchar(255) DEFAULT '' NOT NULL,\n\
retries integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (httptestid)\n\
);\n\
CREATE INDEX httptest_1 ON httptest (applicationid);\n\
CREATE UNIQUE INDEX httptest_2 ON httptest (hostid,name);\n\
CREATE INDEX httptest_3 ON httptest (status);\n\
CREATE INDEX httptest_4 ON httptest (templateid);\n\
CREATE TABLE httpstep (\n\
httpstepid bigint  NOT NULL,\n\
httptestid bigint  NOT NULL REFERENCES httptest (httptestid) ON DELETE CASCADE,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
no integer DEFAULT '0' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
timeout integer DEFAULT '30' NOT NULL,\n\
posts text DEFAULT '' NOT NULL,\n\
required varchar(255) DEFAULT '' NOT NULL,\n\
status_codes varchar(255) DEFAULT '' NOT NULL,\n\
variables text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (httpstepid)\n\
);\n\
CREATE INDEX httpstep_1 ON httpstep (httptestid);\n\
CREATE TABLE interface (\n\
interfaceid bigint  NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
main integer DEFAULT '0' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
useip integer DEFAULT '1' NOT NULL,\n\
ip varchar(64) DEFAULT '127.0.0.1' NOT NULL,\n\
dns varchar(64) DEFAULT '' NOT NULL,\n\
port varchar(64) DEFAULT '10050' NOT NULL,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE INDEX interface_1 ON interface (hostid,type);\n\
CREATE INDEX interface_2 ON interface (ip,dns);\n\
CREATE TABLE valuemaps (\n\
valuemapid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (valuemapid)\n\
);\n\
CREATE INDEX valuemaps_1 ON valuemaps (name);\n\
CREATE TABLE items (\n\
itemid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
snmp_community varchar(64) DEFAULT '' NOT NULL,\n\
snmp_oid varchar(255) DEFAULT '' NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
delay integer DEFAULT '0' NOT NULL,\n\
history integer DEFAULT '90' NOT NULL,\n\
trends integer DEFAULT '365' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
value_type integer DEFAULT '0' NOT NULL,\n\
trapper_hosts varchar(255) DEFAULT '' NOT NULL,\n\
units varchar(255) DEFAULT '' NOT NULL,\n\
multiplier integer DEFAULT '0' NOT NULL,\n\
delta integer DEFAULT '0' NOT NULL,\n\
snmpv3_securityname varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_securitylevel integer DEFAULT '0' NOT NULL,\n\
snmpv3_authpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
snmpv3_privpassphrase varchar(64) DEFAULT '' NOT NULL,\n\
formula varchar(255) DEFAULT '1' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
lastlogsize bigint DEFAULT '0' NOT NULL,\n\
logtimefmt varchar(64) DEFAULT '' NOT NULL,\n\
templateid bigint  NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
valuemapid bigint  NULL REFERENCES valuemaps (valuemapid),\n\
delay_flex varchar(255) DEFAULT '' NOT NULL,\n\
params text DEFAULT '' NOT NULL,\n\
ipmi_sensor varchar(128) DEFAULT '' NOT NULL,\n\
data_type integer DEFAULT '0' NOT NULL,\n\
authtype integer DEFAULT '0' NOT NULL,\n\
username varchar(64) DEFAULT '' NOT NULL,\n\
password varchar(64) DEFAULT '' NOT NULL,\n\
publickey varchar(64) DEFAULT '' NOT NULL,\n\
privatekey varchar(64) DEFAULT '' NOT NULL,\n\
mtime integer DEFAULT '0' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
filter varchar(255) DEFAULT '' NOT NULL,\n\
interfaceid bigint  NULL REFERENCES interface (interfaceid),\n\
port varchar(64) DEFAULT '' NOT NULL,\n\
description text DEFAULT '' NOT NULL,\n\
inventory_link integer DEFAULT '0' NOT NULL,\n\
lifetime varchar(64) DEFAULT '30' NOT NULL,\n\
snmpv3_authprotocol integer DEFAULT '0' NOT NULL,\n\
snmpv3_privprotocol integer DEFAULT '0' NOT NULL,\n\
state integer DEFAULT '0' NOT NULL,\n\
snmpv3_contextname varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (itemid)\n\
);\n\
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);\n\
CREATE INDEX items_3 ON items (status);\n\
CREATE INDEX items_4 ON items (templateid);\n\
CREATE INDEX items_5 ON items (valuemapid);\n\
CREATE INDEX items_6 ON items (interfaceid);\n\
CREATE TABLE httpstepitem (\n\
httpstepitemid bigint  NOT NULL,\n\
httpstepid bigint  NOT NULL REFERENCES httpstep (httpstepid) ON DELETE CASCADE,\n\
itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httpstepitemid)\n\
);\n\
CREATE UNIQUE INDEX httpstepitem_1 ON httpstepitem (httpstepid,itemid);\n\
CREATE INDEX httpstepitem_2 ON httpstepitem (itemid);\n\
CREATE TABLE httptestitem (\n\
httptestitemid bigint  NOT NULL,\n\
httptestid bigint  NOT NULL REFERENCES httptest (httptestid) ON DELETE CASCADE,\n\
itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (httptestitemid)\n\
);\n\
CREATE UNIQUE INDEX httptestitem_1 ON httptestitem (httptestid,itemid);\n\
CREATE INDEX httptestitem_2 ON httptestitem (itemid);\n\
CREATE TABLE media_type (\n\
mediatypeid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
description varchar(100) DEFAULT '' NOT NULL,\n\
smtp_server varchar(255) DEFAULT '' NOT NULL,\n\
smtp_helo varchar(255) DEFAULT '' NOT NULL,\n\
smtp_email varchar(255) DEFAULT '' NOT NULL,\n\
exec_path varchar(255) DEFAULT '' NOT NULL,\n\
gsm_modem varchar(255) DEFAULT '' NOT NULL,\n\
username varchar(255) DEFAULT '' NOT NULL,\n\
passwd varchar(255) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (mediatypeid)\n\
);\n\
CREATE TABLE users (\n\
userid bigint  NOT NULL,\n\
alias varchar(100) DEFAULT '' NOT NULL,\n\
name varchar(100) DEFAULT '' NOT NULL,\n\
surname varchar(100) DEFAULT '' NOT NULL,\n\
passwd char(32) DEFAULT '' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
autologin integer DEFAULT '0' NOT NULL,\n\
autologout integer DEFAULT '900' NOT NULL,\n\
lang varchar(5) DEFAULT 'en_GB' NOT NULL,\n\
refresh integer DEFAULT '30' NOT NULL,\n\
type integer DEFAULT '1' NOT NULL,\n\
theme varchar(128) DEFAULT 'default' NOT NULL,\n\
attempt_failed integer DEFAULT 0 NOT NULL,\n\
attempt_ip varchar(39) DEFAULT '' NOT NULL,\n\
attempt_clock integer DEFAULT 0 NOT NULL,\n\
rows_per_page integer DEFAULT 50 NOT NULL,\n\
PRIMARY KEY (userid)\n\
);\n\
CREATE INDEX users_1 ON users (alias);\n\
CREATE TABLE usrgrp (\n\
usrgrpid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
gui_access integer DEFAULT '0' NOT NULL,\n\
users_status integer DEFAULT '0' NOT NULL,\n\
debug_mode integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (usrgrpid)\n\
);\n\
CREATE INDEX usrgrp_1 ON usrgrp (name);\n\
CREATE TABLE users_groups (\n\
id bigint  NOT NULL,\n\
usrgrpid bigint  NOT NULL REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);\n\
CREATE INDEX users_groups_2 ON users_groups (userid);\n\
CREATE TABLE scripts (\n\
scriptid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
command varchar(255) DEFAULT '' NOT NULL,\n\
host_access integer DEFAULT '2' NOT NULL,\n\
usrgrpid bigint  NULL REFERENCES usrgrp (usrgrpid),\n\
groupid bigint  NULL REFERENCES groups (groupid),\n\
description text DEFAULT '' NOT NULL,\n\
confirmation varchar(255) DEFAULT '' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
execute_on integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (scriptid)\n\
);\n\
CREATE INDEX scripts_1 ON scripts (usrgrpid);\n\
CREATE INDEX scripts_2 ON scripts (groupid);\n\
CREATE TABLE actions (\n\
actionid bigint  NOT NULL,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
eventsource integer DEFAULT '0' NOT NULL,\n\
evaltype integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
esc_period integer DEFAULT '0' NOT NULL,\n\
def_shortdata varchar(255) DEFAULT '' NOT NULL,\n\
def_longdata text DEFAULT '' NOT NULL,\n\
recovery_msg integer DEFAULT '0' NOT NULL,\n\
r_shortdata varchar(255) DEFAULT '' NOT NULL,\n\
r_longdata text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (actionid)\n\
);\n\
CREATE INDEX actions_1 ON actions (eventsource,status);\n\
CREATE TABLE operations (\n\
operationid bigint  NOT NULL,\n\
actionid bigint  NOT NULL REFERENCES actions (actionid) ON DELETE CASCADE,\n\
operationtype integer DEFAULT '0' NOT NULL,\n\
esc_period integer DEFAULT '0' NOT NULL,\n\
esc_step_from integer DEFAULT '1' NOT NULL,\n\
esc_step_to integer DEFAULT '1' NOT NULL,\n\
evaltype integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX operations_1 ON operations (actionid);\n\
CREATE TABLE opmessage (\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
default_msg integer DEFAULT '0' NOT NULL,\n\
subject varchar(255) DEFAULT '' NOT NULL,\n\
message text DEFAULT '' NOT NULL,\n\
mediatypeid bigint  NULL REFERENCES media_type (mediatypeid),\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opmessage_1 ON opmessage (mediatypeid);\n\
CREATE TABLE opmessage_grp (\n\
opmessage_grpid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
usrgrpid bigint  NOT NULL REFERENCES usrgrp (usrgrpid),\n\
PRIMARY KEY (opmessage_grpid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);\n\
CREATE INDEX opmessage_grp_2 ON opmessage_grp (usrgrpid);\n\
CREATE TABLE opmessage_usr (\n\
opmessage_usrid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
userid bigint  NOT NULL REFERENCES users (userid),\n\
PRIMARY KEY (opmessage_usrid)\n\
);\n\
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);\n\
CREATE INDEX opmessage_usr_2 ON opmessage_usr (userid);\n\
CREATE TABLE opcommand (\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
scriptid bigint  NULL REFERENCES scripts (scriptid),\n\
execute_on integer DEFAULT '0' NOT NULL,\n\
port varchar(64) DEFAULT '' NOT NULL,\n\
authtype integer DEFAULT '0' NOT NULL,\n\
username varchar(64) DEFAULT '' NOT NULL,\n\
password varchar(64) DEFAULT '' NOT NULL,\n\
publickey varchar(64) DEFAULT '' NOT NULL,\n\
privatekey varchar(64) DEFAULT '' NOT NULL,\n\
command text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (operationid)\n\
);\n\
CREATE INDEX opcommand_1 ON opcommand (scriptid);\n\
CREATE TABLE opcommand_hst (\n\
opcommand_hstid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
hostid bigint  NULL REFERENCES hosts (hostid),\n\
PRIMARY KEY (opcommand_hstid)\n\
);\n\
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);\n\
CREATE INDEX opcommand_hst_2 ON opcommand_hst (hostid);\n\
CREATE TABLE opcommand_grp (\n\
opcommand_grpid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
groupid bigint  NOT NULL REFERENCES groups (groupid),\n\
PRIMARY KEY (opcommand_grpid)\n\
);\n\
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);\n\
CREATE INDEX opcommand_grp_2 ON opcommand_grp (groupid);\n\
CREATE TABLE opgroup (\n\
opgroupid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
groupid bigint  NOT NULL REFERENCES groups (groupid),\n\
PRIMARY KEY (opgroupid)\n\
);\n\
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);\n\
CREATE INDEX opgroup_2 ON opgroup (groupid);\n\
CREATE TABLE optemplate (\n\
optemplateid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
templateid bigint  NOT NULL REFERENCES hosts (hostid),\n\
PRIMARY KEY (optemplateid)\n\
);\n\
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);\n\
CREATE INDEX optemplate_2 ON optemplate (templateid);\n\
CREATE TABLE opconditions (\n\
opconditionid bigint  NOT NULL,\n\
operationid bigint  NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,\n\
conditiontype integer DEFAULT '0' NOT NULL,\n\
operator integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (opconditionid)\n\
);\n\
CREATE INDEX opconditions_1 ON opconditions (operationid);\n\
CREATE TABLE conditions (\n\
conditionid bigint  NOT NULL,\n\
actionid bigint  NOT NULL REFERENCES actions (actionid) ON DELETE CASCADE,\n\
conditiontype integer DEFAULT '0' NOT NULL,\n\
operator integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (conditionid)\n\
);\n\
CREATE INDEX conditions_1 ON conditions (actionid);\n\
CREATE TABLE config (\n\
configid bigint  NOT NULL,\n\
refresh_unsupported integer DEFAULT '0' NOT NULL,\n\
work_period varchar(100) DEFAULT '1-5,00:00-24:00' NOT NULL,\n\
alert_usrgrpid bigint  NULL REFERENCES usrgrp (usrgrpid),\n\
event_ack_enable integer DEFAULT '1' NOT NULL,\n\
event_expire integer DEFAULT '7' NOT NULL,\n\
event_show_max integer DEFAULT '100' NOT NULL,\n\
default_theme varchar(128) DEFAULT 'originalblue' NOT NULL,\n\
authentication_type integer DEFAULT '0' NOT NULL,\n\
ldap_host varchar(255) DEFAULT '' NOT NULL,\n\
ldap_port integer DEFAULT 389 NOT NULL,\n\
ldap_base_dn varchar(255) DEFAULT '' NOT NULL,\n\
ldap_bind_dn varchar(255) DEFAULT '' NOT NULL,\n\
ldap_bind_password varchar(128) DEFAULT '' NOT NULL,\n\
ldap_search_attribute varchar(128) DEFAULT '' NOT NULL,\n\
dropdown_first_entry integer DEFAULT '1' NOT NULL,\n\
dropdown_first_remember integer DEFAULT '1' NOT NULL,\n\
discovery_groupid bigint  NOT NULL REFERENCES groups (groupid),\n\
max_in_table integer DEFAULT '50' NOT NULL,\n\
search_limit integer DEFAULT '1000' NOT NULL,\n\
severity_color_0 varchar(6) DEFAULT 'DBDBDB' NOT NULL,\n\
severity_color_1 varchar(6) DEFAULT 'D6F6FF' NOT NULL,\n\
severity_color_2 varchar(6) DEFAULT 'FFF6A5' NOT NULL,\n\
severity_color_3 varchar(6) DEFAULT 'FFB689' NOT NULL,\n\
severity_color_4 varchar(6) DEFAULT 'FF9999' NOT NULL,\n\
severity_color_5 varchar(6) DEFAULT 'FF3838' NOT NULL,\n\
severity_name_0 varchar(32) DEFAULT 'Not classified' NOT NULL,\n\
severity_name_1 varchar(32) DEFAULT 'Information' NOT NULL,\n\
severity_name_2 varchar(32) DEFAULT 'Warning' NOT NULL,\n\
severity_name_3 varchar(32) DEFAULT 'Average' NOT NULL,\n\
severity_name_4 varchar(32) DEFAULT 'High' NOT NULL,\n\
severity_name_5 varchar(32) DEFAULT 'Disaster' NOT NULL,\n\
ok_period integer DEFAULT '1800' NOT NULL,\n\
blink_period integer DEFAULT '1800' NOT NULL,\n\
problem_unack_color varchar(6) DEFAULT 'DC0000' NOT NULL,\n\
problem_ack_color varchar(6) DEFAULT 'DC0000' NOT NULL,\n\
ok_unack_color varchar(6) DEFAULT '00AA00' NOT NULL,\n\
ok_ack_color varchar(6) DEFAULT '00AA00' NOT NULL,\n\
problem_unack_style integer DEFAULT '1' NOT NULL,\n\
problem_ack_style integer DEFAULT '1' NOT NULL,\n\
ok_unack_style integer DEFAULT '1' NOT NULL,\n\
ok_ack_style integer DEFAULT '1' NOT NULL,\n\
snmptrap_logging integer DEFAULT '1' NOT NULL,\n\
server_check_interval integer DEFAULT '10' NOT NULL,\n\
hk_events_mode integer DEFAULT '1' NOT NULL,\n\
hk_events_trigger integer DEFAULT '365' NOT NULL,\n\
hk_events_internal integer DEFAULT '365' NOT NULL,\n\
hk_events_discovery integer DEFAULT '365' NOT NULL,\n\
hk_events_autoreg integer DEFAULT '365' NOT NULL,\n\
hk_services_mode integer DEFAULT '1' NOT NULL,\n\
hk_services integer DEFAULT '365' NOT NULL,\n\
hk_audit_mode integer DEFAULT '1' NOT NULL,\n\
hk_audit integer DEFAULT '365' NOT NULL,\n\
hk_sessions_mode integer DEFAULT '1' NOT NULL,\n\
hk_sessions integer DEFAULT '365' NOT NULL,\n\
hk_history_mode integer DEFAULT '1' NOT NULL,\n\
hk_history_global integer DEFAULT '0' NOT NULL,\n\
hk_history integer DEFAULT '90' NOT NULL,\n\
hk_trends_mode integer DEFAULT '1' NOT NULL,\n\
hk_trends_global integer DEFAULT '0' NOT NULL,\n\
hk_trends integer DEFAULT '365' NOT NULL,\n\
PRIMARY KEY (configid)\n\
);\n\
CREATE INDEX config_1 ON config (alert_usrgrpid);\n\
CREATE INDEX config_2 ON config (discovery_groupid);\n\
CREATE TABLE triggers (\n\
triggerid bigint  NOT NULL,\n\
expression varchar(2048) DEFAULT '' NOT NULL,\n\
description varchar(255) DEFAULT '' NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
value integer DEFAULT '0' NOT NULL,\n\
priority integer DEFAULT '0' NOT NULL,\n\
lastchange integer DEFAULT '0' NOT NULL,\n\
comments text DEFAULT '' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
templateid bigint  NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
state integer DEFAULT '0' NOT NULL,\n\
flags integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (triggerid)\n\
);\n\
CREATE INDEX triggers_1 ON triggers (status);\n\
CREATE INDEX triggers_2 ON triggers (value);\n\
CREATE INDEX triggers_3 ON triggers (templateid);\n\
CREATE TABLE trigger_depends (\n\
triggerdepid bigint  NOT NULL,\n\
triggerid_down bigint  NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
triggerid_up bigint  NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
PRIMARY KEY (triggerdepid)\n\
);\n\
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);\n\
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);\n\
CREATE TABLE functions (\n\
functionid bigint  NOT NULL,\n\
itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
triggerid bigint  NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
function varchar(12) DEFAULT '' NOT NULL,\n\
parameter varchar(255) DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (functionid)\n\
);\n\
CREATE INDEX functions_1 ON functions (triggerid);\n\
CREATE INDEX functions_2 ON functions (itemid,function,parameter);\n\
CREATE TABLE graphs (\n\
graphid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
width integer DEFAULT '900' NOT NULL,\n\
height integer DEFAULT '200' NOT NULL,\n\
yaxismin double(16,4) DEFAULT '0' NOT NULL,\n\
yaxismax double(16,4) DEFAULT '100' NOT NULL,\n\
templateid bigint  NULL REFERENCES graphs (graphid) ON DELETE CASCADE,\n\
show_work_period integer DEFAULT '1' NOT NULL,\n\
show_triggers integer DEFAULT '1' NOT NULL,\n\
graphtype integer DEFAULT '0' NOT NULL,\n\
show_legend integer DEFAULT '1' NOT NULL,\n\
show_3d integer DEFAULT '0' NOT NULL,\n\
percent_left double(16,4) DEFAULT '0' NOT NULL,\n\
percent_right double(16,4) DEFAULT '0' NOT NULL,\n\
ymin_type integer DEFAULT '0' NOT NULL,\n\
ymax_type integer DEFAULT '0' NOT NULL,\n\
ymin_itemid bigint  NULL REFERENCES items (itemid),\n\
ymax_itemid bigint  NULL REFERENCES items (itemid),\n\
flags integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (graphid)\n\
);\n\
CREATE INDEX graphs_1 ON graphs (name);\n\
CREATE INDEX graphs_2 ON graphs (templateid);\n\
CREATE INDEX graphs_3 ON graphs (ymin_itemid);\n\
CREATE INDEX graphs_4 ON graphs (ymax_itemid);\n\
CREATE TABLE graphs_items (\n\
gitemid bigint  NOT NULL,\n\
graphid bigint  NOT NULL REFERENCES graphs (graphid) ON DELETE CASCADE,\n\
itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
drawtype integer DEFAULT '0' NOT NULL,\n\
sortorder integer DEFAULT '0' NOT NULL,\n\
color varchar(6) DEFAULT '009600' NOT NULL,\n\
yaxisside integer DEFAULT '0' NOT NULL,\n\
calc_fnc integer DEFAULT '2' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (gitemid)\n\
);\n\
CREATE INDEX graphs_items_1 ON graphs_items (itemid);\n\
CREATE INDEX graphs_items_2 ON graphs_items (graphid);\n\
CREATE TABLE graph_theme (\n\
graphthemeid bigint  NOT NULL,\n\
description varchar(64) DEFAULT '' NOT NULL,\n\
theme varchar(64) DEFAULT '' NOT NULL,\n\
backgroundcolor varchar(6) DEFAULT 'F0F0F0' NOT NULL,\n\
graphcolor varchar(6) DEFAULT 'FFFFFF' NOT NULL,\n\
graphbordercolor varchar(6) DEFAULT '222222' NOT NULL,\n\
gridcolor varchar(6) DEFAULT 'CCCCCC' NOT NULL,\n\
maingridcolor varchar(6) DEFAULT 'AAAAAA' NOT NULL,\n\
gridbordercolor varchar(6) DEFAULT '000000' NOT NULL,\n\
textcolor varchar(6) DEFAULT '202020' NOT NULL,\n\
highlightcolor varchar(6) DEFAULT 'AA4444' NOT NULL,\n\
leftpercentilecolor varchar(6) DEFAULT '11CC11' NOT NULL,\n\
rightpercentilecolor varchar(6) DEFAULT 'CC1111' NOT NULL,\n\
nonworktimecolor varchar(6) DEFAULT 'CCCCCC' NOT NULL,\n\
gridview integer DEFAULT 1 NOT NULL,\n\
legendview integer DEFAULT 1 NOT NULL,\n\
PRIMARY KEY (graphthemeid)\n\
);\n\
CREATE INDEX graph_theme_1 ON graph_theme (description);\n\
CREATE INDEX graph_theme_2 ON graph_theme (theme);\n\
CREATE TABLE globalmacro (\n\
globalmacroid bigint  NOT NULL,\n\
macro varchar(64) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (globalmacroid)\n\
);\n\
CREATE INDEX globalmacro_1 ON globalmacro (macro);\n\
CREATE TABLE hostmacro (\n\
hostmacroid bigint  NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
macro varchar(64) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostmacroid)\n\
);\n\
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);\n\
CREATE TABLE hosts_groups (\n\
hostgroupid bigint  NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
groupid bigint  NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,\n\
PRIMARY KEY (hostgroupid)\n\
);\n\
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);\n\
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);\n\
CREATE TABLE hosts_templates (\n\
hosttemplateid bigint  NOT NULL,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
templateid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
PRIMARY KEY (hosttemplateid)\n\
);\n\
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);\n\
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);\n\
CREATE TABLE items_applications (\n\
itemappid bigint  NOT NULL,\n\
applicationid bigint  NOT NULL REFERENCES applications (applicationid) ON DELETE CASCADE,\n\
itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
PRIMARY KEY (itemappid)\n\
);\n\
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);\n\
CREATE INDEX items_applications_2 ON items_applications (itemid);\n\
CREATE TABLE mappings (\n\
mappingid bigint  NOT NULL,\n\
valuemapid bigint  NOT NULL REFERENCES valuemaps (valuemapid) ON DELETE CASCADE,\n\
value varchar(64) DEFAULT '' NOT NULL,\n\
newvalue varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (mappingid)\n\
);\n\
CREATE INDEX mappings_1 ON mappings (valuemapid);\n\
CREATE TABLE media (\n\
mediaid bigint  NOT NULL,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
mediatypeid bigint  NOT NULL REFERENCES media_type (mediatypeid) ON DELETE CASCADE,\n\
sendto varchar(100) DEFAULT '' NOT NULL,\n\
active integer DEFAULT '0' NOT NULL,\n\
severity integer DEFAULT '63' NOT NULL,\n\
period varchar(100) DEFAULT '1-7,00:00-24:00' NOT NULL,\n\
PRIMARY KEY (mediaid)\n\
);\n\
CREATE INDEX media_1 ON media (userid);\n\
CREATE INDEX media_2 ON media (mediatypeid);\n\
CREATE TABLE rights (\n\
rightid bigint  NOT NULL,\n\
groupid bigint  NOT NULL REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE,\n\
permission integer DEFAULT '0' NOT NULL,\n\
id bigint  NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,\n\
PRIMARY KEY (rightid)\n\
);\n\
CREATE INDEX rights_1 ON rights (groupid);\n\
CREATE INDEX rights_2 ON rights (id);\n\
CREATE TABLE services (\n\
serviceid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
algorithm integer DEFAULT '0' NOT NULL,\n\
triggerid bigint  NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
showsla integer DEFAULT '0' NOT NULL,\n\
goodsla double(16,4) DEFAULT '99.9' NOT NULL,\n\
sortorder integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (serviceid)\n\
);\n\
CREATE INDEX services_1 ON services (triggerid);\n\
CREATE TABLE services_links (\n\
linkid bigint  NOT NULL,\n\
serviceupid bigint  NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,\n\
servicedownid bigint  NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,\n\
soft integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX services_links_1 ON services_links (servicedownid);\n\
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);\n\
CREATE TABLE services_times (\n\
timeid bigint  NOT NULL,\n\
serviceid bigint  NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
ts_from integer DEFAULT '0' NOT NULL,\n\
ts_to integer DEFAULT '0' NOT NULL,\n\
note varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (timeid)\n\
);\n\
CREATE INDEX services_times_1 ON services_times (serviceid,type,ts_from,ts_to);\n\
CREATE TABLE icon_map (\n\
iconmapid bigint  NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
default_iconid bigint  NOT NULL REFERENCES images (imageid),\n\
PRIMARY KEY (iconmapid)\n\
);\n\
CREATE INDEX icon_map_1 ON icon_map (name);\n\
CREATE INDEX icon_map_2 ON icon_map (default_iconid);\n\
CREATE TABLE icon_mapping (\n\
iconmappingid bigint  NOT NULL,\n\
iconmapid bigint  NOT NULL REFERENCES icon_map (iconmapid) ON DELETE CASCADE,\n\
iconid bigint  NOT NULL REFERENCES images (imageid),\n\
inventory_link integer DEFAULT '0' NOT NULL,\n\
expression varchar(64) DEFAULT '' NOT NULL,\n\
sortorder integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (iconmappingid)\n\
);\n\
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);\n\
CREATE INDEX icon_mapping_2 ON icon_mapping (iconid);\n\
CREATE TABLE sysmaps (\n\
sysmapid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
width integer DEFAULT '600' NOT NULL,\n\
height integer DEFAULT '400' NOT NULL,\n\
backgroundid bigint  NULL REFERENCES images (imageid),\n\
label_type integer DEFAULT '2' NOT NULL,\n\
label_location integer DEFAULT '0' NOT NULL,\n\
highlight integer DEFAULT '1' NOT NULL,\n\
expandproblem integer DEFAULT '1' NOT NULL,\n\
markelements integer DEFAULT '0' NOT NULL,\n\
show_unack integer DEFAULT '0' NOT NULL,\n\
grid_size integer DEFAULT '50' NOT NULL,\n\
grid_show integer DEFAULT '1' NOT NULL,\n\
grid_align integer DEFAULT '1' NOT NULL,\n\
label_format integer DEFAULT '0' NOT NULL,\n\
label_type_host integer DEFAULT '2' NOT NULL,\n\
label_type_hostgroup integer DEFAULT '2' NOT NULL,\n\
label_type_trigger integer DEFAULT '2' NOT NULL,\n\
label_type_map integer DEFAULT '2' NOT NULL,\n\
label_type_image integer DEFAULT '2' NOT NULL,\n\
label_string_host varchar(255) DEFAULT '' NOT NULL,\n\
label_string_hostgroup varchar(255) DEFAULT '' NOT NULL,\n\
label_string_trigger varchar(255) DEFAULT '' NOT NULL,\n\
label_string_map varchar(255) DEFAULT '' NOT NULL,\n\
label_string_image varchar(255) DEFAULT '' NOT NULL,\n\
iconmapid bigint  NULL REFERENCES icon_map (iconmapid),\n\
expand_macros integer DEFAULT '0' NOT NULL,\n\
severity_min integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapid)\n\
);\n\
CREATE INDEX sysmaps_1 ON sysmaps (name);\n\
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);\n\
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);\n\
CREATE TABLE sysmaps_elements (\n\
selementid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL REFERENCES sysmaps (sysmapid) ON DELETE CASCADE,\n\
elementid bigint DEFAULT '0' NOT NULL,\n\
elementtype integer DEFAULT '0' NOT NULL,\n\
iconid_off bigint  NULL REFERENCES images (imageid),\n\
iconid_on bigint  NULL REFERENCES images (imageid),\n\
label varchar(2048) DEFAULT '' NOT NULL,\n\
label_location integer DEFAULT '-1' NOT NULL,\n\
x integer DEFAULT '0' NOT NULL,\n\
y integer DEFAULT '0' NOT NULL,\n\
iconid_disabled bigint  NULL REFERENCES images (imageid),\n\
iconid_maintenance bigint  NULL REFERENCES images (imageid),\n\
elementsubtype integer DEFAULT '0' NOT NULL,\n\
areatype integer DEFAULT '0' NOT NULL,\n\
width integer DEFAULT '200' NOT NULL,\n\
height integer DEFAULT '200' NOT NULL,\n\
viewtype integer DEFAULT '0' NOT NULL,\n\
use_iconmap integer DEFAULT '1' NOT NULL,\n\
PRIMARY KEY (selementid)\n\
);\n\
CREATE INDEX sysmaps_elements_1 ON sysmaps_elements (sysmapid);\n\
CREATE INDEX sysmaps_elements_2 ON sysmaps_elements (iconid_off);\n\
CREATE INDEX sysmaps_elements_3 ON sysmaps_elements (iconid_on);\n\
CREATE INDEX sysmaps_elements_4 ON sysmaps_elements (iconid_disabled);\n\
CREATE INDEX sysmaps_elements_5 ON sysmaps_elements (iconid_maintenance);\n\
CREATE TABLE sysmaps_links (\n\
linkid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL REFERENCES sysmaps (sysmapid) ON DELETE CASCADE,\n\
selementid1 bigint  NOT NULL REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE,\n\
selementid2 bigint  NOT NULL REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE,\n\
drawtype integer DEFAULT '0' NOT NULL,\n\
color varchar(6) DEFAULT '000000' NOT NULL,\n\
label varchar(2048) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (linkid)\n\
);\n\
CREATE INDEX sysmaps_links_1 ON sysmaps_links (sysmapid);\n\
CREATE INDEX sysmaps_links_2 ON sysmaps_links (selementid1);\n\
CREATE INDEX sysmaps_links_3 ON sysmaps_links (selementid2);\n\
CREATE TABLE sysmaps_link_triggers (\n\
linktriggerid bigint  NOT NULL,\n\
linkid bigint  NOT NULL REFERENCES sysmaps_links (linkid) ON DELETE CASCADE,\n\
triggerid bigint  NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
drawtype integer DEFAULT '0' NOT NULL,\n\
color varchar(6) DEFAULT '000000' NOT NULL,\n\
PRIMARY KEY (linktriggerid)\n\
);\n\
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);\n\
CREATE INDEX sysmaps_link_triggers_2 ON sysmaps_link_triggers (triggerid);\n\
CREATE TABLE sysmap_element_url (\n\
sysmapelementurlid bigint  NOT NULL,\n\
selementid bigint  NOT NULL REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE,\n\
name varchar(255)  NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (sysmapelementurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);\n\
CREATE TABLE sysmap_url (\n\
sysmapurlid bigint  NOT NULL,\n\
sysmapid bigint  NOT NULL REFERENCES sysmaps (sysmapid) ON DELETE CASCADE,\n\
name varchar(255)  NOT NULL,\n\
url varchar(255) DEFAULT '' NOT NULL,\n\
elementtype integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sysmapurlid)\n\
);\n\
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);\n\
CREATE TABLE maintenances_hosts (\n\
maintenance_hostid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL REFERENCES maintenances (maintenanceid) ON DELETE CASCADE,\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
PRIMARY KEY (maintenance_hostid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);\n\
CREATE INDEX maintenances_hosts_2 ON maintenances_hosts (hostid);\n\
CREATE TABLE maintenances_groups (\n\
maintenance_groupid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL REFERENCES maintenances (maintenanceid) ON DELETE CASCADE,\n\
groupid bigint  NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,\n\
PRIMARY KEY (maintenance_groupid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);\n\
CREATE INDEX maintenances_groups_2 ON maintenances_groups (groupid);\n\
CREATE TABLE timeperiods (\n\
timeperiodid bigint  NOT NULL,\n\
timeperiod_type integer DEFAULT '0' NOT NULL,\n\
every integer DEFAULT '0' NOT NULL,\n\
month integer DEFAULT '0' NOT NULL,\n\
dayofweek integer DEFAULT '0' NOT NULL,\n\
day integer DEFAULT '0' NOT NULL,\n\
start_time integer DEFAULT '0' NOT NULL,\n\
period integer DEFAULT '0' NOT NULL,\n\
start_date integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (timeperiodid)\n\
);\n\
CREATE TABLE maintenances_windows (\n\
maintenance_timeperiodid bigint  NOT NULL,\n\
maintenanceid bigint  NOT NULL REFERENCES maintenances (maintenanceid) ON DELETE CASCADE,\n\
timeperiodid bigint  NOT NULL REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE,\n\
PRIMARY KEY (maintenance_timeperiodid)\n\
);\n\
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);\n\
CREATE INDEX maintenances_windows_2 ON maintenances_windows (timeperiodid);\n\
CREATE TABLE regexps (\n\
regexpid bigint  NOT NULL,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
test_string text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (regexpid)\n\
);\n\
CREATE INDEX regexps_1 ON regexps (name);\n\
CREATE TABLE expressions (\n\
expressionid bigint  NOT NULL,\n\
regexpid bigint  NOT NULL REFERENCES regexps (regexpid) ON DELETE CASCADE,\n\
expression varchar(255) DEFAULT '' NOT NULL,\n\
expression_type integer DEFAULT '0' NOT NULL,\n\
exp_delimiter varchar(1) DEFAULT '' NOT NULL,\n\
case_sensitive integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (expressionid)\n\
);\n\
CREATE INDEX expressions_1 ON expressions (regexpid);\n\
CREATE TABLE nodes (\n\
nodeid integer  NOT NULL,\n\
name varchar(64) DEFAULT '0' NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
port integer DEFAULT '10051' NOT NULL,\n\
nodetype integer DEFAULT '0' NOT NULL,\n\
masterid integer  NULL REFERENCES nodes (nodeid),\n\
PRIMARY KEY (nodeid)\n\
);\n\
CREATE INDEX nodes_1 ON nodes (masterid);\n\
CREATE TABLE node_cksum (\n\
nodeid integer  NOT NULL REFERENCES nodes (nodeid) ON DELETE CASCADE,\n\
tablename varchar(64) DEFAULT '' NOT NULL,\n\
recordid bigint  NOT NULL,\n\
cksumtype integer DEFAULT '0' NOT NULL,\n\
cksum text DEFAULT '' NOT NULL,\n\
sync char(128) DEFAULT '' NOT NULL\n\
);\n\
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);\n\
CREATE TABLE ids (\n\
nodeid integer  NOT NULL,\n\
table_name varchar(64) DEFAULT '' NOT NULL,\n\
field_name varchar(64) DEFAULT '' NOT NULL,\n\
nextid bigint  NOT NULL,\n\
PRIMARY KEY (nodeid,table_name,field_name)\n\
);\n\
CREATE TABLE alerts (\n\
alertid bigint  NOT NULL,\n\
actionid bigint  NOT NULL REFERENCES actions (actionid) ON DELETE CASCADE,\n\
eventid bigint  NOT NULL REFERENCES events (eventid) ON DELETE CASCADE,\n\
userid bigint  NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
clock integer DEFAULT '0' NOT NULL,\n\
mediatypeid bigint  NULL REFERENCES media_type (mediatypeid) ON DELETE CASCADE,\n\
sendto varchar(100) DEFAULT '' NOT NULL,\n\
subject varchar(255) DEFAULT '' NOT NULL,\n\
message text DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
retries integer DEFAULT '0' NOT NULL,\n\
error varchar(128) DEFAULT '' NOT NULL,\n\
esc_step integer DEFAULT '0' NOT NULL,\n\
alerttype integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (alertid)\n\
);\n\
CREATE INDEX alerts_1 ON alerts (actionid);\n\
CREATE INDEX alerts_2 ON alerts (clock);\n\
CREATE INDEX alerts_3 ON alerts (eventid);\n\
CREATE INDEX alerts_4 ON alerts (status,retries);\n\
CREATE INDEX alerts_5 ON alerts (mediatypeid);\n\
CREATE INDEX alerts_6 ON alerts (userid);\n\
CREATE TABLE history (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value double(16,4) DEFAULT '0.0000' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_1 ON history (itemid,clock);\n\
CREATE TABLE history_sync (\n\
id integer  NOT NULL PRIMARY KEY AUTOINCREMENT,\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value double(16,4) DEFAULT '0.0000' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_sync_1 ON history_sync (nodeid,id);\n\
CREATE TABLE history_uint (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value bigint DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);\n\
CREATE TABLE history_uint_sync (\n\
id integer  NOT NULL PRIMARY KEY AUTOINCREMENT,\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value bigint DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_uint_sync_1 ON history_uint_sync (nodeid,id);\n\
CREATE TABLE history_str (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_str_1 ON history_str (itemid,clock);\n\
CREATE TABLE history_str_sync (\n\
id integer  NOT NULL PRIMARY KEY AUTOINCREMENT,\n\
nodeid integer  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX history_str_sync_1 ON history_str_sync (nodeid,id);\n\
CREATE TABLE history_log (\n\
id bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
timestamp integer DEFAULT '0' NOT NULL,\n\
source varchar(64) DEFAULT '' NOT NULL,\n\
severity integer DEFAULT '0' NOT NULL,\n\
value text DEFAULT '' NOT NULL,\n\
logeventid integer DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_log_1 ON history_log (itemid,clock);\n\
CREATE UNIQUE INDEX history_log_2 ON history_log (itemid,id);\n\
CREATE TABLE history_text (\n\
id bigint  NOT NULL,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value text DEFAULT '' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (id)\n\
);\n\
CREATE INDEX history_text_1 ON history_text (itemid,clock);\n\
CREATE UNIQUE INDEX history_text_2 ON history_text (itemid,id);\n\
CREATE TABLE proxy_history (\n\
id integer  NOT NULL PRIMARY KEY AUTOINCREMENT,\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
timestamp integer DEFAULT '0' NOT NULL,\n\
source varchar(64) DEFAULT '' NOT NULL,\n\
severity integer DEFAULT '0' NOT NULL,\n\
value text DEFAULT '' NOT NULL,\n\
logeventid integer DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
state integer DEFAULT '0' NOT NULL\n\
);\n\
CREATE INDEX proxy_history_1 ON proxy_history (clock);\n\
CREATE TABLE proxy_dhistory (\n\
id integer  NOT NULL PRIMARY KEY AUTOINCREMENT,\n\
clock integer DEFAULT '0' NOT NULL,\n\
druleid bigint  NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
port integer DEFAULT '0' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
dcheckid bigint  NULL,\n\
dns varchar(64) DEFAULT '' NOT NULL\n\
);\n\
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);\n\
CREATE TABLE events (\n\
eventid bigint  NOT NULL,\n\
source integer DEFAULT '0' NOT NULL,\n\
object integer DEFAULT '0' NOT NULL,\n\
objectid bigint DEFAULT '0' NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value integer DEFAULT '0' NOT NULL,\n\
acknowledged integer DEFAULT '0' NOT NULL,\n\
ns integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (eventid)\n\
);\n\
CREATE INDEX events_1 ON events (source,object,objectid,clock);\n\
CREATE INDEX events_2 ON events (source,object,clock);\n\
CREATE TABLE trends (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
num integer DEFAULT '0' NOT NULL,\n\
value_min double(16,4) DEFAULT '0.0000' NOT NULL,\n\
value_avg double(16,4) DEFAULT '0.0000' NOT NULL,\n\
value_max double(16,4) DEFAULT '0.0000' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE trends_uint (\n\
itemid bigint  NOT NULL,\n\
clock integer DEFAULT '0' NOT NULL,\n\
num integer DEFAULT '0' NOT NULL,\n\
value_min bigint DEFAULT '0' NOT NULL,\n\
value_avg bigint DEFAULT '0' NOT NULL,\n\
value_max bigint DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemid,clock)\n\
);\n\
CREATE TABLE acknowledges (\n\
acknowledgeid bigint  NOT NULL,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
eventid bigint  NOT NULL REFERENCES events (eventid) ON DELETE CASCADE,\n\
clock integer DEFAULT '0' NOT NULL,\n\
message varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (acknowledgeid)\n\
);\n\
CREATE INDEX acknowledges_1 ON acknowledges (userid);\n\
CREATE INDEX acknowledges_2 ON acknowledges (eventid);\n\
CREATE INDEX acknowledges_3 ON acknowledges (clock);\n\
CREATE TABLE auditlog (\n\
auditid bigint  NOT NULL,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
clock integer DEFAULT '0' NOT NULL,\n\
action integer DEFAULT '0' NOT NULL,\n\
resourcetype integer DEFAULT '0' NOT NULL,\n\
details varchar(128)  DEFAULT '0' NOT NULL,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
resourceid bigint DEFAULT '0' NOT NULL,\n\
resourcename varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditid)\n\
);\n\
CREATE INDEX auditlog_1 ON auditlog (userid,clock);\n\
CREATE INDEX auditlog_2 ON auditlog (clock);\n\
CREATE TABLE auditlog_details (\n\
auditdetailid bigint  NOT NULL,\n\
auditid bigint  NOT NULL REFERENCES auditlog (auditid) ON DELETE CASCADE,\n\
table_name varchar(64) DEFAULT '' NOT NULL,\n\
field_name varchar(64) DEFAULT '' NOT NULL,\n\
oldvalue text DEFAULT '' NOT NULL,\n\
newvalue text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (auditdetailid)\n\
);\n\
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);\n\
CREATE TABLE service_alarms (\n\
servicealarmid bigint  NOT NULL,\n\
serviceid bigint  NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,\n\
clock integer DEFAULT '0' NOT NULL,\n\
value integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (servicealarmid)\n\
);\n\
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);\n\
CREATE INDEX service_alarms_2 ON service_alarms (clock);\n\
CREATE TABLE autoreg_host (\n\
autoreg_hostid bigint  NOT NULL,\n\
proxy_hostid bigint  NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
listen_ip varchar(39) DEFAULT '' NOT NULL,\n\
listen_port integer DEFAULT '0' NOT NULL,\n\
listen_dns varchar(64) DEFAULT '' NOT NULL,\n\
host_metadata varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (autoreg_hostid)\n\
);\n\
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);\n\
CREATE TABLE proxy_autoreg_host (\n\
id integer  NOT NULL PRIMARY KEY AUTOINCREMENT,\n\
clock integer DEFAULT '0' NOT NULL,\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
listen_ip varchar(39) DEFAULT '' NOT NULL,\n\
listen_port integer DEFAULT '0' NOT NULL,\n\
listen_dns varchar(64) DEFAULT '' NOT NULL,\n\
host_metadata varchar(255) DEFAULT '' NOT NULL\n\
);\n\
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);\n\
CREATE TABLE dhosts (\n\
dhostid bigint  NOT NULL,\n\
druleid bigint  NOT NULL REFERENCES drules (druleid) ON DELETE CASCADE,\n\
status integer DEFAULT '0' NOT NULL,\n\
lastup integer DEFAULT '0' NOT NULL,\n\
lastdown integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (dhostid)\n\
);\n\
CREATE INDEX dhosts_1 ON dhosts (druleid);\n\
CREATE TABLE dservices (\n\
dserviceid bigint  NOT NULL,\n\
dhostid bigint  NOT NULL REFERENCES dhosts (dhostid) ON DELETE CASCADE,\n\
type integer DEFAULT '0' NOT NULL,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
value varchar(255) DEFAULT '' NOT NULL,\n\
port integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
lastup integer DEFAULT '0' NOT NULL,\n\
lastdown integer DEFAULT '0' NOT NULL,\n\
dcheckid bigint  NOT NULL REFERENCES dchecks (dcheckid) ON DELETE CASCADE,\n\
ip varchar(39) DEFAULT '' NOT NULL,\n\
dns varchar(64) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (dserviceid)\n\
);\n\
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,type,key_,ip,port);\n\
CREATE INDEX dservices_2 ON dservices (dhostid);\n\
CREATE TABLE escalations (\n\
escalationid bigint  NOT NULL,\n\
actionid bigint  NOT NULL,\n\
triggerid bigint  NULL,\n\
eventid bigint  NULL,\n\
r_eventid bigint  NULL,\n\
nextcheck integer DEFAULT '0' NOT NULL,\n\
esc_step integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
itemid bigint  NULL,\n\
PRIMARY KEY (escalationid)\n\
);\n\
CREATE UNIQUE INDEX escalations_1 ON escalations (actionid,triggerid,itemid,escalationid);\n\
CREATE TABLE globalvars (\n\
globalvarid bigint  NOT NULL,\n\
snmp_lastsize integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (globalvarid)\n\
);\n\
CREATE TABLE graph_discovery (\n\
graphdiscoveryid bigint  NOT NULL,\n\
graphid bigint  NOT NULL REFERENCES graphs (graphid) ON DELETE CASCADE,\n\
parent_graphid bigint  NOT NULL REFERENCES graphs (graphid) ON DELETE CASCADE,\n\
name varchar(128) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (graphdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX graph_discovery_1 ON graph_discovery (graphid,parent_graphid);\n\
CREATE INDEX graph_discovery_2 ON graph_discovery (parent_graphid);\n\
CREATE TABLE host_inventory (\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
inventory_mode integer DEFAULT '0' NOT NULL,\n\
type varchar(64) DEFAULT '' NOT NULL,\n\
type_full varchar(64) DEFAULT '' NOT NULL,\n\
name varchar(64) DEFAULT '' NOT NULL,\n\
alias varchar(64) DEFAULT '' NOT NULL,\n\
os varchar(64) DEFAULT '' NOT NULL,\n\
os_full varchar(255) DEFAULT '' NOT NULL,\n\
os_short varchar(64) DEFAULT '' NOT NULL,\n\
serialno_a varchar(64) DEFAULT '' NOT NULL,\n\
serialno_b varchar(64) DEFAULT '' NOT NULL,\n\
tag varchar(64) DEFAULT '' NOT NULL,\n\
asset_tag varchar(64) DEFAULT '' NOT NULL,\n\
macaddress_a varchar(64) DEFAULT '' NOT NULL,\n\
macaddress_b varchar(64) DEFAULT '' NOT NULL,\n\
hardware varchar(255) DEFAULT '' NOT NULL,\n\
hardware_full text DEFAULT '' NOT NULL,\n\
software varchar(255) DEFAULT '' NOT NULL,\n\
software_full text DEFAULT '' NOT NULL,\n\
software_app_a varchar(64) DEFAULT '' NOT NULL,\n\
software_app_b varchar(64) DEFAULT '' NOT NULL,\n\
software_app_c varchar(64) DEFAULT '' NOT NULL,\n\
software_app_d varchar(64) DEFAULT '' NOT NULL,\n\
software_app_e varchar(64) DEFAULT '' NOT NULL,\n\
contact text DEFAULT '' NOT NULL,\n\
location text DEFAULT '' NOT NULL,\n\
location_lat varchar(16) DEFAULT '' NOT NULL,\n\
location_lon varchar(16) DEFAULT '' NOT NULL,\n\
notes text DEFAULT '' NOT NULL,\n\
chassis varchar(64) DEFAULT '' NOT NULL,\n\
model varchar(64) DEFAULT '' NOT NULL,\n\
hw_arch varchar(32) DEFAULT '' NOT NULL,\n\
vendor varchar(64) DEFAULT '' NOT NULL,\n\
contract_number varchar(64) DEFAULT '' NOT NULL,\n\
installer_name varchar(64) DEFAULT '' NOT NULL,\n\
deployment_status varchar(64) DEFAULT '' NOT NULL,\n\
url_a varchar(255) DEFAULT '' NOT NULL,\n\
url_b varchar(255) DEFAULT '' NOT NULL,\n\
url_c varchar(255) DEFAULT '' NOT NULL,\n\
host_networks text DEFAULT '' NOT NULL,\n\
host_netmask varchar(39) DEFAULT '' NOT NULL,\n\
host_router varchar(39) DEFAULT '' NOT NULL,\n\
oob_ip varchar(39) DEFAULT '' NOT NULL,\n\
oob_netmask varchar(39) DEFAULT '' NOT NULL,\n\
oob_router varchar(39) DEFAULT '' NOT NULL,\n\
date_hw_purchase varchar(64) DEFAULT '' NOT NULL,\n\
date_hw_install varchar(64) DEFAULT '' NOT NULL,\n\
date_hw_expiry varchar(64) DEFAULT '' NOT NULL,\n\
date_hw_decomm varchar(64) DEFAULT '' NOT NULL,\n\
site_address_a varchar(128) DEFAULT '' NOT NULL,\n\
site_address_b varchar(128) DEFAULT '' NOT NULL,\n\
site_address_c varchar(128) DEFAULT '' NOT NULL,\n\
site_city varchar(128) DEFAULT '' NOT NULL,\n\
site_state varchar(64) DEFAULT '' NOT NULL,\n\
site_country varchar(64) DEFAULT '' NOT NULL,\n\
site_zip varchar(64) DEFAULT '' NOT NULL,\n\
site_rack varchar(128) DEFAULT '' NOT NULL,\n\
site_notes text DEFAULT '' NOT NULL,\n\
poc_1_name varchar(128) DEFAULT '' NOT NULL,\n\
poc_1_email varchar(128) DEFAULT '' NOT NULL,\n\
poc_1_phone_a varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_phone_b varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_cell varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_screen varchar(64) DEFAULT '' NOT NULL,\n\
poc_1_notes text DEFAULT '' NOT NULL,\n\
poc_2_name varchar(128) DEFAULT '' NOT NULL,\n\
poc_2_email varchar(128) DEFAULT '' NOT NULL,\n\
poc_2_phone_a varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_phone_b varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_cell varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_screen varchar(64) DEFAULT '' NOT NULL,\n\
poc_2_notes text DEFAULT '' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE housekeeper (\n\
housekeeperid bigint  NOT NULL,\n\
tablename varchar(64) DEFAULT '' NOT NULL,\n\
field varchar(64) DEFAULT '' NOT NULL,\n\
value bigint  NOT NULL,\n\
PRIMARY KEY (housekeeperid)\n\
);\n\
CREATE TABLE images (\n\
imageid bigint  NOT NULL,\n\
imagetype integer DEFAULT '0' NOT NULL,\n\
name varchar(64) DEFAULT '0' NOT NULL,\n\
image longblob DEFAULT '' NOT NULL,\n\
PRIMARY KEY (imageid)\n\
);\n\
CREATE INDEX images_1 ON images (imagetype,name);\n\
CREATE TABLE item_discovery (\n\
itemdiscoveryid bigint  NOT NULL,\n\
itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
parent_itemid bigint  NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,\n\
key_ varchar(255) DEFAULT '' NOT NULL,\n\
lastcheck integer DEFAULT '0' NOT NULL,\n\
ts_delete integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (itemdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);\n\
CREATE INDEX item_discovery_2 ON item_discovery (parent_itemid);\n\
CREATE TABLE host_discovery (\n\
hostid bigint  NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,\n\
parent_hostid bigint  NULL REFERENCES hosts (hostid),\n\
parent_itemid bigint  NULL REFERENCES items (itemid),\n\
host varchar(64) DEFAULT '' NOT NULL,\n\
lastcheck integer DEFAULT '0' NOT NULL,\n\
ts_delete integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (hostid)\n\
);\n\
CREATE TABLE interface_discovery (\n\
interfaceid bigint  NOT NULL REFERENCES interface (interfaceid) ON DELETE CASCADE,\n\
parent_interfaceid bigint  NOT NULL REFERENCES interface (interfaceid) ON DELETE CASCADE,\n\
PRIMARY KEY (interfaceid)\n\
);\n\
CREATE TABLE profiles (\n\
profileid bigint  NOT NULL,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
idx varchar(96) DEFAULT '' NOT NULL,\n\
idx2 bigint DEFAULT '0' NOT NULL,\n\
value_id bigint DEFAULT '0' NOT NULL,\n\
value_int integer DEFAULT '0' NOT NULL,\n\
value_str varchar(255) DEFAULT '' NOT NULL,\n\
source varchar(96) DEFAULT '' NOT NULL,\n\
type integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (profileid)\n\
);\n\
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);\n\
CREATE INDEX profiles_2 ON profiles (userid,profileid);\n\
CREATE TABLE sessions (\n\
sessionid varchar(32) DEFAULT '' NOT NULL,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
lastaccess integer DEFAULT '0' NOT NULL,\n\
status integer DEFAULT '0' NOT NULL,\n\
PRIMARY KEY (sessionid)\n\
);\n\
CREATE INDEX sessions_1 ON sessions (userid,status);\n\
CREATE TABLE trigger_discovery (\n\
triggerdiscoveryid bigint  NOT NULL,\n\
triggerid bigint  NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
parent_triggerid bigint  NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,\n\
name varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (triggerdiscoveryid)\n\
);\n\
CREATE UNIQUE INDEX trigger_discovery_1 ON trigger_discovery (triggerid,parent_triggerid);\n\
CREATE INDEX trigger_discovery_2 ON trigger_discovery (parent_triggerid);\n\
CREATE TABLE user_history (\n\
userhistoryid bigint  NOT NULL,\n\
userid bigint  NOT NULL REFERENCES users (userid) ON DELETE CASCADE,\n\
title1 varchar(255) DEFAULT '' NOT NULL,\n\
url1 varchar(255) DEFAULT '' NOT NULL,\n\
title2 varchar(255) DEFAULT '' NOT NULL,\n\
url2 varchar(255) DEFAULT '' NOT NULL,\n\
title3 varchar(255) DEFAULT '' NOT NULL,\n\
url3 varchar(255) DEFAULT '' NOT NULL,\n\
title4 varchar(255) DEFAULT '' NOT NULL,\n\
url4 varchar(255) DEFAULT '' NOT NULL,\n\
title5 varchar(255) DEFAULT '' NOT NULL,\n\
url5 varchar(255) DEFAULT '' NOT NULL,\n\
PRIMARY KEY (userhistoryid)\n\
);\n\
CREATE UNIQUE INDEX user_history_1 ON user_history (userid);\n\
CREATE TABLE application_template (\n\
application_templateid bigint  NOT NULL,\n\
applicationid bigint  NOT NULL REFERENCES applications (applicationid) ON DELETE CASCADE,\n\
templateid bigint  NOT NULL REFERENCES applications (applicationid) ON DELETE CASCADE,\n\
PRIMARY KEY (application_templateid)\n\
);\n\
CREATE UNIQUE INDEX application_template_1 ON application_template (applicationid,templateid);\n\
CREATE INDEX application_template_2 ON application_template (templateid);\n\
CREATE TABLE dbversion (\n\
mandatory integer DEFAULT '0' NOT NULL,\n\
optional integer DEFAULT '0' NOT NULL\n\
);\n\
INSERT INTO dbversion VALUES ('2020000','2020001');\n\
";
const char	*const db_schema_fkeys[] = {
	NULL
};
const char	*const db_schema_fkeys_drop[] = {
	NULL
};
#endif	/* HAVE_SQLITE3 */
