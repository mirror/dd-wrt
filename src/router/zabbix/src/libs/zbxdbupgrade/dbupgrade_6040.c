/*
** Zabbix
** Copyright (C) 2001-2023 Zabbix SIA
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

#include "zbxdbhigh.h"
#include "dbupgrade.h"

/*
 * 6.4 maintenance database patches
 */

#ifndef HAVE_SQLITE3

static int	DBpatch_6040000(void)
{
	return SUCCEED;
}

static int	DBpatch_6040001(void)
{
	if (FAIL == zbx_db_index_exists("problem", "problem_4"))
		return DBcreate_index("problem", "problem_4", "cause_eventid", 0);

	return SUCCEED;
}

static int	DBpatch_6040002(void)
{
	if (FAIL == zbx_db_index_exists("dashboard_user", "dashboard_user_2"))
		return DBcreate_index("dashboard_user", "dashboard_user_2", "userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040003(void)
{
	if (FAIL == zbx_db_index_exists("dashboard_usrgrp", "dashboard_usrgrp_2"))
		return DBcreate_index("dashboard_usrgrp", "dashboard_usrgrp_2", "usrgrpid", 0);

	return SUCCEED;
}

static int	DBpatch_6040004(void)
{
	return DBcreate_index("event_suppress", "event_suppress_4", "userid", 0);
}

static int	DBpatch_6040005(void)
{
	if (FAIL == zbx_db_index_exists("group_discovery", "group_discovery_1"))
		return DBcreate_index("group_discovery", "group_discovery_1", "parent_group_prototypeid", 0);

	return SUCCEED;
}

static int	DBpatch_6040006(void)
{
	if (FAIL == zbx_db_index_exists("group_prototype", "group_prototype_2"))
		return DBcreate_index("group_prototype", "group_prototype_2", "groupid", 0);

	return SUCCEED;
}

static int	DBpatch_6040007(void)
{
	if (FAIL == zbx_db_index_exists("group_prototype", "group_prototype_3"))
		return DBcreate_index("group_prototype", "group_prototype_3", "templateid", 0);

	return SUCCEED;
}

static int	DBpatch_6040008(void)
{
	if (FAIL == zbx_db_index_exists("host_discovery", "host_discovery_1"))
		return DBcreate_index("host_discovery", "host_discovery_1", "parent_hostid", 0);

	return SUCCEED;
}

static int	DBpatch_6040009(void)
{
	if (FAIL == zbx_db_index_exists("host_discovery", "host_discovery_2"))
		return DBcreate_index("host_discovery", "host_discovery_2", "parent_itemid", 0);

	return SUCCEED;
}

static int	DBpatch_6040010(void)
{
	if (FAIL == zbx_db_index_exists("hosts", "hosts_7"))
		return DBcreate_index("hosts", "hosts_7", "templateid", 0);

	return SUCCEED;
}

static int	DBpatch_6040011(void)
{
	if (FAIL == zbx_db_index_exists("interface_discovery", "interface_discovery_1"))
		return DBcreate_index("interface_discovery", "interface_discovery_1", "parent_interfaceid", 0);

	return SUCCEED;
}

static int	DBpatch_6040012(void)
{
	if (FAIL == zbx_db_index_exists("report", "report_2"))
		return DBcreate_index("report", "report_2", "userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040013(void)
{
	if (FAIL == zbx_db_index_exists("report", "report_3"))
		return DBcreate_index("report", "report_3", "dashboardid", 0);

	return SUCCEED;
}

static int	DBpatch_6040014(void)
{
	if (FAIL == zbx_db_index_exists("report_user", "report_user_2"))
		return DBcreate_index("report_user", "report_user_2", "userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040015(void)
{
	if (FAIL == zbx_db_index_exists("report_user", "report_user_3"))
		return DBcreate_index("report_user", "report_user_3", "access_userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040016(void)
{
	if (FAIL == zbx_db_index_exists("report_usrgrp", "report_usrgrp_2"))
		return DBcreate_index("report_usrgrp", "report_usrgrp_2", "usrgrpid", 0);

	return SUCCEED;
}

static int	DBpatch_6040017(void)
{
	if (FAIL == zbx_db_index_exists("report_usrgrp", "report_usrgrp_3"))
		return DBcreate_index("report_usrgrp", "report_usrgrp_3", "access_userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040018(void)
{
	if (FAIL == zbx_db_index_exists("sysmaps", "sysmaps_4"))
		return DBcreate_index("sysmaps", "sysmaps_4", "userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040019(void)
{
	if (FAIL == zbx_db_index_exists("sysmap_element_trigger", "sysmap_element_trigger_2"))
		return DBcreate_index("sysmap_element_trigger", "sysmap_element_trigger_2", "triggerid", 0);

	return SUCCEED;
}

static int	DBpatch_6040020(void)
{
	if (FAIL == zbx_db_index_exists("sysmap_user", "sysmap_user_2"))
		return DBcreate_index("sysmap_user", "sysmap_user_2", "userid", 0);

	return SUCCEED;
}

static int	DBpatch_6040021(void)
{
	if (FAIL == zbx_db_index_exists("sysmap_usrgrp", "sysmap_usrgrp_2"))
		return DBcreate_index("sysmap_usrgrp", "sysmap_usrgrp_2", "usrgrpid", 0);

	return SUCCEED;
}

static int	DBpatch_6040022(void)
{
	if (FAIL == zbx_db_index_exists("tag_filter", "tag_filter_1"))
		return DBcreate_index("tag_filter", "tag_filter_1", "usrgrpid", 0);

	return SUCCEED;
}

static int	DBpatch_6040023(void)
{
	if (FAIL == zbx_db_index_exists("tag_filter", "tag_filter_2"))
		return DBcreate_index("tag_filter", "tag_filter_2", "groupid", 0);

	return SUCCEED;
}

static int	DBpatch_6040024(void)
{
	if (FAIL == zbx_db_index_exists("task", "task_2"))
		return DBcreate_index("task", "task_2", "proxy_hostid", 0);

	return SUCCEED;
}

static int	DBpatch_6040025(void)
{
	if (FAIL == zbx_db_index_exists("users", "users_3"))
		return DBcreate_index("users", "users_3", "roleid", 0);

	return SUCCEED;
}

#endif

DBPATCH_START(6040)

/* version, duplicates flag, mandatory flag */

DBPATCH_ADD(6040000, 0, 1)
DBPATCH_ADD(6040001, 0, 0)
DBPATCH_ADD(6040002, 0, 0)
DBPATCH_ADD(6040003, 0, 0)
DBPATCH_ADD(6040004, 0, 0)
DBPATCH_ADD(6040005, 0, 0)
DBPATCH_ADD(6040006, 0, 0)
DBPATCH_ADD(6040007, 0, 0)
DBPATCH_ADD(6040008, 0, 0)
DBPATCH_ADD(6040009, 0, 0)
DBPATCH_ADD(6040010, 0, 0)
DBPATCH_ADD(6040011, 0, 0)
DBPATCH_ADD(6040012, 0, 0)
DBPATCH_ADD(6040013, 0, 0)
DBPATCH_ADD(6040014, 0, 0)
DBPATCH_ADD(6040015, 0, 0)
DBPATCH_ADD(6040016, 0, 0)
DBPATCH_ADD(6040017, 0, 0)
DBPATCH_ADD(6040018, 0, 0)
DBPATCH_ADD(6040019, 0, 0)
DBPATCH_ADD(6040020, 0, 0)
DBPATCH_ADD(6040021, 0, 0)
DBPATCH_ADD(6040022, 0, 0)
DBPATCH_ADD(6040023, 0, 0)
DBPATCH_ADD(6040024, 0, 0)
DBPATCH_ADD(6040025, 0, 0)

DBPATCH_END()
