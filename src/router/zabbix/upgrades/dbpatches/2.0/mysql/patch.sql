ALTER TABLE acknowledges MODIFY acknowledgeid bigint unsigned NOT NULL,
			 MODIFY userid bigint unsigned NOT NULL,
			 MODIFY eventid bigint unsigned NOT NULL;
DELETE FROM acknowledges WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM acknowledges WHERE NOT eventid IN (SELECT eventid FROM events);
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE actions
	MODIFY actionid bigint unsigned NOT NULL,
	MODIFY def_longdata text NOT NULL,
	MODIFY r_longdata text NOT NULL;
UPDATE actions SET esc_period=3600 WHERE eventsource=0 AND esc_period=0;
ALTER TABLE alerts
	MODIFY alertid bigint unsigned NOT NULL,
	MODIFY actionid bigint unsigned NOT NULL,
	MODIFY eventid bigint unsigned NOT NULL,
	MODIFY userid bigint unsigned NULL,
	MODIFY mediatypeid bigint unsigned NULL,
	MODIFY message text NOT NULL;
UPDATE alerts SET userid=NULL WHERE userid=0;
UPDATE alerts SET mediatypeid=NULL WHERE mediatypeid=0;
DELETE FROM alerts WHERE NOT actionid IN (SELECT actionid FROM actions);
DELETE FROM alerts WHERE NOT eventid IN (SELECT eventid FROM events);
DELETE FROM alerts WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM alerts WHERE NOT mediatypeid IN (SELECT mediatypeid FROM media_type);
ALTER TABLE alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE applications MODIFY applicationid bigint unsigned NOT NULL,
			 MODIFY hostid bigint unsigned NOT NULL,
			 MODIFY templateid bigint unsigned NULL;
DELETE FROM applications WHERE NOT hostid IN (SELECT hostid FROM hosts);
UPDATE applications SET templateid=NULL WHERE templateid=0;
CREATE TEMPORARY TABLE tmp_applications_applicationid (applicationid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_applications_applicationid (applicationid) (SELECT applicationid FROM applications);
UPDATE applications SET templateid=NULL WHERE NOT templateid IS NULL AND NOT templateid IN (SELECT applicationid FROM tmp_applications_applicationid);
DROP TABLE tmp_applications_applicationid;
ALTER TABLE applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE applications ADD CONSTRAINT c_applications_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE auditlog_details
	MODIFY auditdetailid bigint unsigned NOT NULL,
	MODIFY auditid bigint unsigned NOT NULL,
	MODIFY oldvalue text NOT NULL,
	MODIFY newvalue text NOT NULL;
DELETE FROM auditlog_details WHERE NOT auditid IN (SELECT auditid FROM auditlog);
ALTER TABLE auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE;
ALTER TABLE auditlog MODIFY auditid bigint unsigned NOT NULL,
		     MODIFY userid bigint unsigned NOT NULL;
DELETE FROM auditlog WHERE NOT userid IN (SELECT userid FROM users);
ALTER TABLE auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
DROP INDEX autoreg_host_1 ON autoreg_host;
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);
ALTER TABLE autoreg_host MODIFY autoreg_hostid bigint unsigned NOT NULL,
			 MODIFY proxy_hostid bigint unsigned NULL,
			 ADD listen_ip varchar(39) DEFAULT '' NOT NULL,
			 ADD listen_port integer DEFAULT '0' NOT NULL,
			 ADD listen_dns varchar(64) DEFAULT '' NOT NULL;
UPDATE autoreg_host SET proxy_hostid=NULL WHERE proxy_hostid=0;
DELETE FROM autoreg_host WHERE proxy_hostid IS NOT NULL AND proxy_hostid NOT IN (SELECT hostid FROM hosts);
ALTER TABLE autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE conditions MODIFY conditionid bigint unsigned NOT NULL,
		       MODIFY actionid bigint unsigned NOT NULL;
DELETE FROM conditions WHERE NOT actionid IN (SELECT actionid FROM actions);
ALTER TABLE conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE config
	MODIFY configid bigint unsigned NOT NULL,
	MODIFY alert_usrgrpid bigint unsigned NULL,
	MODIFY discovery_groupid bigint unsigned NOT NULL,
	MODIFY default_theme varchar(128) DEFAULT 'originalblue' NOT NULL,
	ADD severity_color_0 varchar(6) DEFAULT 'DBDBDB' NOT NULL,
	ADD severity_color_1 varchar(6) DEFAULT 'D6F6FF' NOT NULL,
	ADD severity_color_2 varchar(6) DEFAULT 'FFF6A5' NOT NULL,
	ADD severity_color_3 varchar(6) DEFAULT 'FFB689' NOT NULL,
	ADD severity_color_4 varchar(6) DEFAULT 'FF9999' NOT NULL,
	ADD severity_color_5 varchar(6) DEFAULT 'FF3838' NOT NULL,
	ADD severity_name_0 varchar(32) DEFAULT 'Not classified' NOT NULL,
	ADD severity_name_1 varchar(32) DEFAULT 'Information' NOT NULL,
	ADD severity_name_2 varchar(32) DEFAULT 'Warning' NOT NULL,
	ADD severity_name_3 varchar(32) DEFAULT 'Average' NOT NULL,
	ADD severity_name_4 varchar(32) DEFAULT 'High' NOT NULL,
	ADD severity_name_5 varchar(32) DEFAULT 'Disaster' NOT NULL,
	ADD ok_period integer DEFAULT '1800' NOT NULL,
	ADD blink_period integer DEFAULT '1800' NOT NULL,
	ADD problem_unack_color varchar(6) DEFAULT 'DC0000' NOT NULL,
	ADD problem_ack_color varchar(6) DEFAULT 'DC0000' NOT NULL,
	ADD ok_unack_color varchar(6) DEFAULT '00AA00' NOT NULL,
	ADD ok_ack_color varchar(6) DEFAULT '00AA00' NOT NULL,
	ADD problem_unack_style integer DEFAULT '1' NOT NULL,
	ADD problem_ack_style integer DEFAULT '1' NOT NULL,
	ADD ok_unack_style integer DEFAULT '1' NOT NULL,
	ADD ok_ack_style integer DEFAULT '1' NOT NULL,
	ADD snmptrap_logging integer DEFAULT '1' NOT NULL,
	ADD server_check_interval integer DEFAULT '60' NOT NULL;

UPDATE config SET alert_usrgrpid=NULL WHERE NOT alert_usrgrpid IN (SELECT usrgrpid FROM usrgrp);
UPDATE config SET discovery_groupid=(SELECT MIN(groupid) FROM groups) WHERE NOT discovery_groupid IN (SELECT groupid FROM groups);

UPDATE config SET default_theme='darkblue' WHERE default_theme='css_bb.css';
UPDATE config SET default_theme='originalblue' WHERE default_theme IN ('css_ob.css', 'default.css');
UPDATE config SET default_theme='darkorange' WHERE default_theme='css_od.css';

ALTER TABLE config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid);
-- See drules.sql
ALTER TABLE dhosts MODIFY dhostid bigint unsigned NOT NULL,
		   MODIFY druleid bigint unsigned NOT NULL;
DELETE FROM dhosts WHERE NOT druleid IN (SELECT druleid FROM drules);
ALTER TABLE dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
ALTER TABLE dchecks MODIFY dcheckid bigint unsigned NOT NULL,
		    MODIFY druleid bigint unsigned NOT NULL,
		    MODIFY key_ varchar(255) DEFAULT '' NOT NULL,
		    MODIFY snmp_community varchar(255) DEFAULT '' NOT NULL,
		    ADD uniq integer DEFAULT '0' NOT NULL;
DELETE FROM dchecks WHERE NOT druleid IN (SELECT druleid FROM drules);
ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
UPDATE dchecks SET uniq=1 WHERE dcheckid IN (SELECT unique_dcheckid FROM drules);
ALTER TABLE drules MODIFY druleid bigint unsigned NOT NULL,
		   MODIFY proxy_hostid bigint unsigned NULL,
		   MODIFY delay integer DEFAULT '3600' NOT NULL,
		   DROP unique_dcheckid;
UPDATE drules SET proxy_hostid=NULL WHERE NOT proxy_hostid IN (SELECT hostid FROM hosts);
ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE dservices MODIFY dserviceid bigint unsigned NOT NULL,
		      MODIFY dhostid bigint unsigned NOT NULL,
		      MODIFY dcheckid bigint unsigned NOT NULL,
		      MODIFY key_ varchar(255) DEFAULT '' NOT NULL,
		      MODIFY value varchar(255) DEFAULT '' NOT NULL,
		      ADD dns varchar(64) DEFAULT '' NOT NULL;
DELETE FROM dservices WHERE NOT dhostid IN (SELECT dhostid FROM dhosts);
DELETE FROM dservices WHERE NOT dcheckid IN (SELECT dcheckid FROM dchecks);
ALTER TABLE dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE;
ALTER TABLE dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE;
ALTER TABLE escalations
	MODIFY escalationid bigint unsigned NOT NULL,
	MODIFY actionid bigint unsigned NOT NULL,
	MODIFY triggerid bigint unsigned NULL,
	MODIFY eventid bigint unsigned NULL,
	MODIFY r_eventid bigint unsigned NULL;
DROP INDEX escalations_2 ON escalations;

-- 0: ESCALATION_STATUS_ACTIVE
-- 1: ESCALATION_STATUS_RECOVERY
-- 2: ESCALATION_STATUS_SLEEP
-- 4: ESCALATION_STATUS_SUPERSEDED_ACTIVE
-- 5: ESCALATION_STATUS_SUPERSEDED_RECOVERY
UPDATE escalations SET status=0 WHERE status in (1,4,5);

SET @escalationid = (SELECT MAX(escalationid) FROM escalations);
INSERT INTO escalations (escalationid, actionid, triggerid, r_eventid)
	SELECT @escalationid := @escalationid + 1, actionid, triggerid, r_eventid
		FROM escalations
		WHERE status = 0
			AND eventid IS NOT NULL
			AND r_eventid IS NOT NULL;
UPDATE escalations SET r_eventid = NULL WHERE eventid IS NOT NULL AND r_eventid IS NOT NULL;
-- See triggers.sql
ALTER TABLE expressions MODIFY expressionid bigint unsigned NOT NULL,
			MODIFY regexpid bigint unsigned NOT NULL;
DELETE FROM expressions WHERE NOT regexpid IN (SELECT regexpid FROM regexps);
ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE;
ALTER TABLE functions MODIFY functionid bigint unsigned NOT NULL,
		      MODIFY itemid bigint unsigned NOT NULL,
		      MODIFY triggerid bigint unsigned NOT NULL,
		      DROP COLUMN lastvalue;
DELETE FROM functions WHERE NOT itemid IN (SELECT itemid FROM items);
DELETE FROM functions WHERE NOT triggerid IN (SELECT triggerid FROM triggers);
ALTER TABLE functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE globalmacro MODIFY globalmacroid bigint unsigned NOT NULL;
CREATE TABLE globalvars (
	globalvarid              bigint unsigned                           NOT NULL,
	snmp_lastsize            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
) ENGINE=InnoDB;
CREATE TABLE graph_discovery (
	graphdiscoveryid         bigint unsigned                           NOT NULL,
	graphid                  bigint unsigned                           NOT NULL,
	parent_graphid           bigint unsigned                           NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (graphdiscoveryid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX graph_discovery_1 on graph_discovery (graphid,parent_graphid);
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs_items
	MODIFY gitemid bigint unsigned NOT NULL,
	MODIFY graphid bigint unsigned NOT NULL,
	MODIFY itemid bigint unsigned NOT NULL,
	DROP COLUMN periods_cnt;
UPDATE graphs_items SET type=0 WHERE type=1;
DELETE FROM graphs_items WHERE NOT graphid IN (SELECT graphid FROM graphs);
DELETE FROM graphs_items WHERE NOT itemid IN (SELECT itemid FROM items);
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE graphs MODIFY graphid bigint unsigned NOT NULL,
		   MODIFY templateid bigint unsigned NULL,
		   MODIFY ymin_itemid bigint unsigned NULL,
		   MODIFY ymax_itemid bigint unsigned NULL,
		   MODIFY show_legend integer NOT NULL DEFAULT 1,
		   ADD flags integer DEFAULT '0' NOT NULL;
UPDATE graphs SET templateid=NULL WHERE templateid=0;
UPDATE graphs SET show_legend=1 WHERE graphtype=0 OR graphtype=1;
CREATE TEMPORARY TABLE tmp_graphs_graphid (graphid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_graphs_graphid (graphid) (SELECT graphid FROM graphs);
UPDATE graphs SET templateid=NULL WHERE NOT templateid IS NULL AND NOT templateid IN (SELECT graphid FROM tmp_graphs_graphid);
DROP TABLE tmp_graphs_graphid;
UPDATE graphs SET ymin_itemid=NULL WHERE ymin_itemid=0 OR NOT ymin_itemid IN (SELECT itemid FROM items);
UPDATE graphs SET ymax_itemid=NULL WHERE ymax_itemid=0 OR NOT ymax_itemid IN (SELECT itemid FROM items);
UPDATE graphs SET ymin_type=0 WHERE ymin_type=2 AND ymin_itemid=NULL;
UPDATE graphs SET ymax_type=0 WHERE ymax_type=2 AND ymax_itemid=NULL;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid);
ALTER TABLE graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid);
ALTER TABLE graph_theme MODIFY graphthemeid bigint unsigned NOT NULL,
			CHANGE noneworktimecolor nonworktimecolor varchar(6) DEFAULT 'CCCCCC' NOT NULL;

UPDATE graph_theme SET theme = 'darkblue' WHERE theme = 'css_bb.css';
UPDATE graph_theme SET theme = 'originalblue' WHERE theme = 'css_ob.css';

-- Insert new graph theme
SET @graphthemeid = (SELECT MAX(graphthemeid) FROM graph_theme);
INSERT INTO graph_theme (graphthemeid, description, theme, backgroundcolor, graphcolor, graphbordercolor, gridcolor, maingridcolor, gridbordercolor, textcolor, highlightcolor, leftpercentilecolor, rightpercentilecolor, nonworktimecolor, gridview, legendview)
VALUES
(@graphthemeid + 1, 'Dark orange', 'darkorange', '333333', '0A0A0A', '888888', '222222', '4F4F4F', 'EFEFEF', 'DFDFDF', 'FF5500', 'FF5500', 'FF1111', '1F1F1F', 1, 1),
(@graphthemeid + 2, 'Classic', 'classic', 'F0F0F0', 'FFFFFF', '333333', 'CCCCCC', 'AAAAAA', '000000', '222222', 'AA4444', '11CC11', 'CC1111', 'E0E0E0', 1, 1);
DELETE FROM ids WHERE table_name = 'graph_theme';
ALTER TABLE groups MODIFY groupid bigint unsigned NOT NULL;
DROP TABLE help_items;

CREATE TABLE help_items (
	itemtype	integer		DEFAULT '0'	NOT NULL,
	key_		varchar(255)	DEFAULT ''	NOT NULL,
	description	varchar(255)	DEFAULT ''	NOT NULL,
	PRIMARY KEY (itemtype,key_)
) ENGINE=InnoDB;

INSERT INTO help_items (itemtype,key_,description) values ('0','agent.ping','Check the agent usability. Always return 1. Can be used as a TCP ping.');
INSERT INTO help_items (itemtype,key_,description) values ('0','agent.version','Version of zabbix_agent(d) running on monitored host. String value. Example of returned value: 1.1');
INSERT INTO help_items (itemtype,key_,description) values ('0','kernel.maxfiles','Maximum number of opened files supported by OS.');
INSERT INTO help_items (itemtype,key_,description) values ('0','kernel.maxproc','Maximum number of processes supported by OS.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.dns.record[&lt;ip&gt;,zone,&lt;type&gt;,&lt;timeout&gt;,&lt;count&gt;]','Performs a DNS query. On success returns a character string with the required type of information.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.dns[&lt;ip&gt;,zone,&lt;type&gt;,&lt;timeout&gt;,&lt;count&gt;]','Checks if DNS service is up. 0 - DNS is down (server did not respond or DNS resolution failed), 1 - DNS is up.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.if.collisions[if]','Out-of-window collision. Collisions count.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.if.in[if,&lt;mode&gt;]','Network interface input statistic. Integer value. If mode is missing bytes is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.if.list','List of network interfaces. Text value.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.if.out[if,&lt;mode&gt;]','Network interface output statistic. Integer value. If mode is missing bytes is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.if.total[if,&lt;mode&gt;]','Sum of network interface incoming and outgoing statistics. Integer value. Mode - one of bytes (default), packets, errors or dropped');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.tcp.listen[port]','Checks if this port is in LISTEN state. 0 - it is not, 1 - it is in LISTEN state.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.tcp.port[&lt;ip&gt;,port]','Check, if it is possible to make TCP connection to the port number. 0 - cannot connect, 1 - can connect. IP address is optional. If ip is missing, 127.0.0.1 is used. Example: net.tcp.port[,80]');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.tcp.service.perf[service,&lt;ip&gt;,&lt;port&gt;]','Check performance of service &quot;service&quot;. 0 - service is down, sec - number of seconds spent on connection to the service. If ip is missing 127.0.0.1 is used.  If port number is missing, default service port is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','net.tcp.service[service,&lt;ip&gt;,&lt;port&gt;]','Check if service is available. 0 - service is down, 1 - service is running. If ip is missing 127.0.0.1 is used. If port number is missing, default service port is used. Example: net.tcp.service[ftp,,45].');
INSERT INTO help_items (itemtype,key_,description) values ('0','perf_counter[counter,&lt;interval&gt;]','Value of any performance counter, where "counter" parameter is the counter path and "interval" parameter is a number of last seconds, for which the agent returns an average value.');
INSERT INTO help_items (itemtype,key_,description) values ('0','proc.mem[&lt;name&gt;,&lt;user&gt;,&lt;mode&gt;,&lt;cmdline&gt;]','Memory used by process with name name running under user user. Memory used by processes. Process name, user and mode is optional. If name or user is missing all processes will be calculated. If mode is missing sum is used. Example: proc.mem[,root]');
INSERT INTO help_items (itemtype,key_,description) values ('0','proc.num[&lt;name&gt;,&lt;user&gt;,&lt;state&gt;,&lt;cmdline&gt;]','Number of processes with name name running under user user having state state. Process name, user and state are optional. Examples: proc.num[,mysql]; proc.num[apache2,www-data]; proc.num[,oracle,sleep,oracleZABBIX]');
INSERT INTO help_items (itemtype,key_,description) values ('0','proc_info[&lt;process&gt;,&lt;attribute&gt;,&lt;type&gt;]','Different information about specific process(es)');
INSERT INTO help_items (itemtype,key_,description) values ('0','service_state[service]','State of service. 0 - running, 1 - paused, 2 - start pending, 3 - pause pending, 4 - continue pending, 5 - stop pending, 6 - stopped, 7 - unknown, 255 - no such service');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.boottime','Timestamp of system boot.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.cpu.intr','Device interrupts.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.cpu.load[&lt;cpu&gt;,&lt;mode&gt;]','CPU(s) load. Processor load. The cpu and mode are optional. If cpu is missing all is used. If mode is missing avg1 is used. Note that this is not percentage.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.cpu.num','Number of available proccessors.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.cpu.switches','Context switches.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.cpu.util[&lt;cpu&gt;,&lt;type&gt;,&lt;mode&gt;]','CPU(s) utilisation. Processor load in percents. The cpu, type and mode are optional. If cpu is missing all is used.  If type is missing user is used. If mode is missing avg1 is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.hostname[&lt;type&gt;]','Returns hostname (or NetBIOS name (by default) on Windows). String value. Example of returned value: www.zabbix.com');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.hw.chassis[&lt;info&gt;]','Chassis info - returns full info by default');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.hw.cpu[&lt;cpu&gt;,&lt;info&gt;]','CPU info - lists full info for all CPUs by default');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.hw.devices[&lt;type&gt;]','Device list - lists PCI devices by default');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.hw.macaddr[&lt;interface&gt;,&lt;format&gt;]','MAC address - lists all MAC addresses with interface names by default');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.localtime','System local time. Time in seconds.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.run[command,&lt;mode&gt;]','Run specified command on the host.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.stat[resource,&lt;type&gt;]','Virtual memory statistics.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.sw.arch','Software architecture');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.sw.os[&lt;info&gt;]','Current OS - returns full info by default');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.sw.packages[&lt;package&gt;,&lt;manager&gt;,&lt;format&gt;]','Software package list - lists all packages for all supported package managers by default');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.swap.in[&lt;swap&gt;,&lt;type&gt;]','Swap in. If type is count - swapins is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.swap.out[&lt;swap&gt;,&lt;type&gt;]','Swap out. If type is count - swapouts is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.swap.size[&lt;swap&gt;,&lt;mode&gt;]','Swap space. Number of bytes. If swap is missing all is used. If mode is missing free is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.uname','Returns detailed host information. String value');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.uptime','System uptime in seconds.');
INSERT INTO help_items (itemtype,key_,description) values ('0','system.users.num','Number of users connected. Command who is used on agent side.');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.dev.read[device,&lt;type&gt;,&lt;mode&gt;]','Device read statistics.');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.dev.write[device,&lt;type&gt;,&lt;mode&gt;]','Device write statistics.');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.cksum[file]','Calculate check sum of a given file. Check sum of the file calculate by standard algorithm used by UNIX utility cksum. Example: vfs.file.cksum[/etc/passwd]');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.contents[file,&lt;encoding&gt;]','Get contents of a given file.');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.exists[file]','Check if file exists. 0 - file does not exist, 1 - file exists');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.md5sum[file]','Calculate MD5 check sum of a given file. String MD5 hash of the file. Can be used for files less than 64MB, unsupported otherwise. Example: vfs.file.md5sum[/usr/local/etc/zabbix_agentd.conf]');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.regexp[file,regexp,&lt;encoding&gt;]','Find string in a file. Matched string');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.regmatch[file,regexp,&lt;encoding&gt;]','Find string in a file. 0 - expression not found, 1 - found');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.size[file]','Size of a given file. Size in bytes. File must have read permissions for user zabbix. Example: vfs.file.size[/var/log/syslog]');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.file.time[file,&lt;mode&gt;]','File time information. Number of seconds.The mode is optional. If mode is missing modify is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.fs.inode[fs,&lt;mode&gt;]','Number of inodes for a given volume. If mode is missing total is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','vfs.fs.size[fs,&lt;mode&gt;]','Calculate disk space for a given volume. Disk space in KB. If mode is missing total is used.  In case of mounted volume, unused disk space for local file system is returned. Example: vfs.fs.size[/tmp,free].');
INSERT INTO help_items (itemtype,key_,description) values ('0','vm.memory.size[&lt;mode&gt;]','Amount of memory size in bytes. If mode is missing total is used.');
INSERT INTO help_items (itemtype,key_,description) values ('0','web.page.get[host,&lt;path&gt;,&lt;port&gt;]','Get content of web page. Default path is /');
INSERT INTO help_items (itemtype,key_,description) values ('0','web.page.perf[host,&lt;path&gt;,&lt;port&gt;]','Get timing of loading full web page. Default path is /');
INSERT INTO help_items (itemtype,key_,description) values ('0','web.page.regexp[host,&lt;path&gt;,&lt;port&gt;,&lt;regexp&gt;,&lt;length&gt;]','Get first occurrence of regexp in web page. Default path is /');
INSERT INTO help_items (itemtype,key_,description) values ('3','icmppingloss[&lt;target&gt;,&lt;packets&gt;,&lt;interval&gt;,&lt;size&gt;,&lt;timeout&gt;]','Returns percentage of lost ICMP ping packets.');
INSERT INTO help_items (itemtype,key_,description) values ('3','icmppingsec[&lt;target&gt;,&lt;packets&gt;,&lt;interval&gt;,&lt;size&gt;,&lt;timeout&gt;,&lt;mode&gt;]','Returns ICMP ping response time in seconds. Example: 0.02');
INSERT INTO help_items (itemtype,key_,description) values ('3','icmpping[&lt;target&gt;,&lt;packets&gt;,&lt;interval&gt;,&lt;size&gt;,&lt;timeout&gt;]','Checks if server is accessible by ICMP ping. 0 - ICMP ping fails. 1 - ICMP ping successful. One of zabbix_server processes performs ICMP pings once per PingerFrequency seconds.');
INSERT INTO help_items (itemtype,key_,description) values ('3','net.tcp.service.perf[service,&lt;ip&gt;,&lt;port&gt;]','Check performance of service. 0 - service is down, sec - number of seconds spent on connection to the service. If &lt;ip&gt; is missing, IP or DNS name is taken from host definition. If &lt;port&gt; is missing, default service port is used.');
INSERT INTO help_items (itemtype,key_,description) values ('3','net.tcp.service[service,&lt;ip&gt;,&lt;port&gt;]','Check if service is available. 0 - service is down, 1 - service is running. If &lt;ip&gt; is missing, IP or DNS name is taken from host definition. If &lt;port&gt; is missing, default service port is used.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[boottime]','Startup time of Zabbix server, Unix timestamp.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[history]','Number of values stored in table HISTORY.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[history_log]','Number of values stored in table HISTORY_LOG.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[history_str]','Number of values stored in table HISTORY_STR.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[history_text]','Number of values stored in table HISTORY_TEXT.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[history_uint]','Number of values stored in table HISTORY_UINT.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[host,&lt;type&gt;,available]','Returns availability of a particular type of checks on the host. Value of this item corresponds to availability icons in the host list. Valid types are: agent, snmp, ipmi, jmx.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[items]','Number of items in Zabbix database.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[items_unsupported]','Number of unsupported items in Zabbix database.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[java,,&lt;param&gt;]','Returns information associated with Zabbix Java gateway. Valid params are: ping, version.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[process,&lt;type&gt;,&lt;num&gt;,&lt;state&gt;]','Time a particular Zabbix process or a group of processes (identified by &lt;type&gt; and &lt;num&gt;) spent in &lt;state&gt; in percentage.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[proxy,&lt;name&gt;,&lt;param&gt;]','Time of proxy last access. Name - proxy name. Param - lastaccess. Unix timestamp.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[queue,&lt;from&gt;,&lt;to&gt;]','Number of items in the queue which are delayed by from to to seconds, inclusive.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[requiredperformance]','Required performance of the Zabbix server, in new values per second expected.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[rcache,&lt;cache&gt;,&lt;mode&gt;]','Configuration cache statistics. Cache - buffer (modes: pfree, total, used, free).');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[trends]','Number of values stored in table TRENDS.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[trends_uint]','Number of values stored in table TRENDS_UINT.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[triggers]','Number of triggers in Zabbix database.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[uptime]','Uptime of Zabbix server process in seconds.');
INSERT INTO help_items (itemtype,key_,description) values ('5','zabbix[wcache,&lt;cache&gt;,&lt;mode&gt;]','Data cache statistics. Cache - one of values (modes: all, float, uint, str, log, text), history (modes: pfree, total, used, free), trend (modes: pfree, total, used, free), text (modes: pfree, total, used, free).');
INSERT INTO help_items (itemtype,key_,description) values ('7','agent.ping','Check the agent usability. Always return 1. Can be used as a TCP ping.');
INSERT INTO help_items (itemtype,key_,description) values ('7','agent.version','Version of zabbix_agent(d) running on monitored host. String value. Example of returned value: 1.1');
INSERT INTO help_items (itemtype,key_,description) values ('7','eventlog[logtype,&lt;pattern&gt;,&lt;severity&gt;,&lt;source&gt;,&lt;eventid&gt;,&lt;maxlines&gt;,&lt;mode&gt;]','Monitoring of Windows event logs. pattern, severity, eventid - regular expressions');
INSERT INTO help_items (itemtype,key_,description) values ('7','kernel.maxfiles','Maximum number of opened files supported by OS.');
INSERT INTO help_items (itemtype,key_,description) values ('7','kernel.maxproc','Maximum number of processes supported by OS.');
INSERT INTO help_items (itemtype,key_,description) values ('7','logrt[file_format,&lt;pattern&gt;,&lt;encoding&gt;,&lt;maxlines&gt;,&lt;mode&gt;]','Monitoring of log file with rotation. fileformat - [path][regexp], pattern - regular expression');
INSERT INTO help_items (itemtype,key_,description) values ('7','log[file,&lt;pattern&gt;,&lt;encoding&gt;,&lt;maxlines&gt;,&lt;mode&gt;]','Monitoring of log file. pattern - regular expression');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.dns.record[&lt;ip&gt;,zone,&lt;type&gt;,&lt;timeout&gt;,&lt;count&gt;]','Performs a DNS query. On success returns a character string with the required type of information.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.dns[&lt;ip&gt;,zone,&lt;type&gt;,&lt;timeout&gt;,&lt;count&gt;]','Checks if DNS service is up. 0 - DNS is down (server did not respond or DNS resolution failed), 1 - DNS is up.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.if.collisions[if]','Out-of-window collision. Collisions count.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.if.in[if,&lt;mode&gt;]','Network interface input statistic. Integer value. If mode is missing bytes is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.if.list','List of network interfaces. Text value.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.if.out[if,&lt;mode&gt;]','Network interface output statistic. Integer value. If mode is missing bytes is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.if.total[if,&lt;mode&gt;]','Sum of network interface incoming and outgoing statistics. Integer value. Mode - one of bytes (default), packets, errors or dropped');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.tcp.listen[port]','Checks if this port is in LISTEN state. 0 - it is not, 1 - it is in LISTEN state.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.tcp.port[&lt;ip&gt;,port]','Check, if it is possible to make TCP connection to the port number. 0 - cannot connect, 1 - can connect. IP address is optional. If ip is missing, 127.0.0.1 is used. Example: net.tcp.port[,80]');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.tcp.service.perf[service,&lt;ip&gt;,&lt;port&gt;]','Check performance of service &quot;service&quot;. 0 - service is down, sec - number of seconds spent on connection to the service. If ip is missing 127.0.0.1 is used.  If port number is missing, default service port is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','net.tcp.service[service,&lt;ip&gt;,&lt;port&gt;]','Check if service is available. 0 - service is down, 1 - service is running. If ip is missing 127.0.0.1 is used. If port number is missing, default service port is used. Example: net.tcp.service[ftp,,45].');
INSERT INTO help_items (itemtype,key_,description) values ('7','perf_counter[counter,&lt;interval&gt;]','Value of any performance counter, where "counter" parameter is the counter path and "interval" parameter is a number of last seconds, for which the agent returns an average value.');
INSERT INTO help_items (itemtype,key_,description) values ('7','proc.mem[&lt;name&gt;,&lt;user&gt;,&lt;mode&gt;,&lt;cmdline&gt;]','Memory used by process with name name running under user user. Memory used by processes. Process name, user and mode is optional. If name or user is missing all processes will be calculated. If mode is missing sum is used. Example: proc.mem[,root]');
INSERT INTO help_items (itemtype,key_,description) values ('7','proc.num[&lt;name&gt;,&lt;user&gt;,&lt;state&gt;,&lt;cmdline&gt;]','Number of processes with name name running under user user having state state. Process name, user and state are optional. Examples: proc.num[,mysql]; proc.num[apache2,www-data]; proc.num[,oracle,sleep,oracleZABBIX]');
INSERT INTO help_items (itemtype,key_,description) values ('7','proc_info[&lt;process&gt;,&lt;attribute&gt;,&lt;type&gt;]','Different information about specific process(es)');
INSERT INTO help_items (itemtype,key_,description) values ('7','service_state[service]','State of service. 0 - running, 1 - paused, 2 - start pending, 3 - pause pending, 4 - continue pending, 5 - stop pending, 6 - stopped, 7 - unknown, 255 - no such service');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.boottime','Timestamp of system boot.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.cpu.intr','Device interrupts.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.cpu.load[&lt;cpu&gt;,&lt;mode&gt;]','CPU(s) load. Processor load. The cpu and mode are optional. If cpu is missing all is used. If mode is missing avg1 is used. Note that this is not percentage.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.cpu.num','Number of available proccessors.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.cpu.switches','Context switches.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.cpu.util[&lt;cpu&gt;,&lt;type&gt;,&lt;mode&gt;]','CPU(s) utilisation. Processor load in percents. The cpu, type and mode are optional. If cpu is missing all is used.  If type is missing user is used. If mode is missing avg1 is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.hostname[&lt;type&gt;]','Returns hostname (or NetBIOS name (by default) on Windows). String value. Example of returned value: www.zabbix.com');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.hw.chassis[&lt;info&gt;]','Chassis info - returns full info by default');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.hw.cpu[&lt;cpu&gt;,&lt;info&gt;]','CPU info - lists full info for all CPUs by default');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.hw.devices[&lt;type&gt;]','Device list - lists PCI devices by default');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.hw.macaddr[&lt;interface&gt;,&lt;format&gt;]','MAC address - lists all MAC addresses with interface names by default');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.localtime','System local time. Time in seconds.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.run[command,&lt;mode&gt;]','Run specified command on the host.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.stat[resource,&lt;type&gt;]','Virtual memory statistics.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.sw.arch','Software architecture');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.sw.os[&lt;info&gt;]','Current OS - returns full info by default');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.sw.packages[&lt;package&gt;,&lt;manager&gt;,&lt;format&gt;]','Software package list - lists all packages for all supported package managers by default');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.swap.in[&lt;swap&gt;,&lt;type&gt;]','Swap in. If type is count - swapins is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.swap.out[&lt;swap&gt;,&lt;type&gt;]','Swap out. If type is count - swapouts is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.swap.size[&lt;swap&gt;,&lt;mode&gt;]','Swap space. Number of bytes. If swap is missing all is used. If mode is missing free is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.uname','Returns detailed host information. String value');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.uptime','System uptime in seconds.');
INSERT INTO help_items (itemtype,key_,description) values ('7','system.users.num','Number of users connected. Command who is used on agent side.');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.dev.read[device,&lt;type&gt;,&lt;mode&gt;]','Device read statistics.');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.dev.write[device,&lt;type&gt;,&lt;mode&gt;]','Device write statistics.');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.cksum[file]','Calculate check sum of a given file. Check sum of the file calculate by standard algorithm used by UNIX utility cksum. Example: vfs.file.cksum[/etc/passwd]');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.contents[file,&lt;encoding&gt;]','Get contents of a given file.');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.exists[file]','Check if file exists. 0 - file does not exist, 1 - file exists');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.md5sum[file]','Calculate MD5 check sum of a given file. String MD5 hash of the file. Can be used for files less than 64MB, unsupported otherwise. Example: vfs.file.md5sum[/usr/local/etc/zabbix_agentd.conf]');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.regexp[file,regexp,&lt;encoding&gt;]','Find string in a file. Matched string');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.regmatch[file,regexp,&lt;encoding&gt;]','Find string in a file. 0 - expression not found, 1 - found');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.size[file]','Size of a given file. Size in bytes. File must have read permissions for user zabbix. Example: vfs.file.size[/var/log/syslog]');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.file.time[file,&lt;mode&gt;]','File time information. Number of seconds.The mode is optional. If mode is missing modify is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.fs.inode[fs,&lt;mode&gt;]','Number of inodes for a given volume. If mode is missing total is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','vfs.fs.size[fs,&lt;mode&gt;]','Calculate disk space for a given volume. Disk space in KB. If mode is missing total is used.  In case of mounted volume, unused disk space for local file system is returned. Example: vfs.fs.size[/tmp,free].');
INSERT INTO help_items (itemtype,key_,description) values ('7','vm.memory.size[&lt;mode&gt;]','Amount of memory size in bytes. If mode is missing total is used.');
INSERT INTO help_items (itemtype,key_,description) values ('7','web.page.get[host,&lt;path&gt;,&lt;port&gt;]','Get content of web page. Default path is /');
INSERT INTO help_items (itemtype,key_,description) values ('7','web.page.perf[host,&lt;path&gt;,&lt;port&gt;]','Get timing of loading full web page. Default path is /');
INSERT INTO help_items (itemtype,key_,description) values ('7','web.page.regexp[host,&lt;path&gt;,&lt;port&gt;,&lt;regexp&gt;,&lt;length&gt;]','Get first occurrence of regexp in web page. Default path is /');
INSERT INTO help_items (itemtype,key_,description) values ('8','grpfunc[&lt;group&gt;,&lt;key&gt;,&lt;func&gt;,&lt;param&gt;]','Aggregate checks do not require any agent running on a host being monitored. Zabbix server collects aggregate information by doing direct database queries. See Zabbix Manual.');
INSERT INTO help_items (itemtype,key_,description) values ('17','snmptrap.fallback','Catches all SNMP traps from a corresponding address that were not catched by any of the snmptrap[] items for that interface.');
INSERT INTO help_items (itemtype,key_,description) values ('17','snmptrap[&lt;regex&gt;]','Catches all SNMP traps from a corresponding address that match regex. Default regex is an empty string.');
ALTER TABLE history_log
	MODIFY id bigint unsigned NOT NULL,
	MODIFY itemid bigint unsigned NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history
	MODIFY itemid bigint unsigned NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history_str
	MODIFY itemid bigint unsigned NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history_str_sync
	MODIFY itemid bigint unsigned NOT NULL,
	MODIFY nodeid integer NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history_sync
	MODIFY itemid bigint unsigned NOT NULL,
	MODIFY nodeid integer NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history_text
	MODIFY id bigint unsigned NOT NULL,
	MODIFY itemid bigint unsigned NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history_uint
	MODIFY itemid bigint unsigned NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE history_uint_sync
	MODIFY itemid bigint unsigned NOT NULL,
	MODIFY nodeid integer NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL;
DELETE FROM hosts_profiles WHERE NOT hostid IN (SELECT hostid FROM hosts);
DELETE FROM hosts_profiles_ext WHERE NOT hostid IN (SELECT hostid FROM hosts);

CREATE TABLE host_inventory (
	hostid                   bigint unsigned                           NOT NULL,
	inventory_mode           integer         DEFAULT '0'               NOT NULL,
	type                     varchar(64)     DEFAULT ''                NOT NULL,
	type_full                varchar(64)     DEFAULT ''                NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	alias                    varchar(64)     DEFAULT ''                NOT NULL,
	os                       varchar(64)     DEFAULT ''                NOT NULL,
	os_full                  varchar(255)    DEFAULT ''                NOT NULL,
	os_short                 varchar(64)     DEFAULT ''                NOT NULL,
	serialno_a               varchar(64)     DEFAULT ''                NOT NULL,
	serialno_b               varchar(64)     DEFAULT ''                NOT NULL,
	tag                      varchar(64)     DEFAULT ''                NOT NULL,
	asset_tag                varchar(64)     DEFAULT ''                NOT NULL,
	macaddress_a             varchar(64)     DEFAULT ''                NOT NULL,
	macaddress_b             varchar(64)     DEFAULT ''                NOT NULL,
	hardware                 varchar(255)    DEFAULT ''                NOT NULL,
	hardware_full            text                                      NOT NULL,
	software                 varchar(255)    DEFAULT ''                NOT NULL,
	software_full            text                                      NOT NULL,
	software_app_a           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_b           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_c           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_d           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_e           varchar(64)     DEFAULT ''                NOT NULL,
	contact                  text                                      NOT NULL,
	location                 text                                      NOT NULL,
	location_lat             varchar(16)     DEFAULT ''                NOT NULL,
	location_lon             varchar(16)     DEFAULT ''                NOT NULL,
	notes                    text                                      NOT NULL,
	chassis                  varchar(64)     DEFAULT ''                NOT NULL,
	model                    varchar(64)     DEFAULT ''                NOT NULL,
	hw_arch                  varchar(32)     DEFAULT ''                NOT NULL,
	vendor                   varchar(64)     DEFAULT ''                NOT NULL,
	contract_number          varchar(64)     DEFAULT ''                NOT NULL,
	installer_name           varchar(64)     DEFAULT ''                NOT NULL,
	deployment_status        varchar(64)     DEFAULT ''                NOT NULL,
	url_a                    varchar(255)    DEFAULT ''                NOT NULL,
	url_b                    varchar(255)    DEFAULT ''                NOT NULL,
	url_c                    varchar(255)    DEFAULT ''                NOT NULL,
	host_networks            text                                      NOT NULL,
	host_netmask             varchar(39)     DEFAULT ''                NOT NULL,
	host_router              varchar(39)     DEFAULT ''                NOT NULL,
	oob_ip                   varchar(39)     DEFAULT ''                NOT NULL,
	oob_netmask              varchar(39)     DEFAULT ''                NOT NULL,
	oob_router               varchar(39)     DEFAULT ''                NOT NULL,
	date_hw_purchase         varchar(64)     DEFAULT ''                NOT NULL,
	date_hw_install          varchar(64)     DEFAULT ''                NOT NULL,
	date_hw_expiry           varchar(64)     DEFAULT ''                NOT NULL,
	date_hw_decomm           varchar(64)     DEFAULT ''                NOT NULL,
	site_address_a           varchar(128)    DEFAULT ''                NOT NULL,
	site_address_b           varchar(128)    DEFAULT ''                NOT NULL,
	site_address_c           varchar(128)    DEFAULT ''                NOT NULL,
	site_city                varchar(128)    DEFAULT ''                NOT NULL,
	site_state               varchar(64)     DEFAULT ''                NOT NULL,
	site_country             varchar(64)     DEFAULT ''                NOT NULL,
	site_zip                 varchar(64)     DEFAULT ''                NOT NULL,
	site_rack                varchar(128)    DEFAULT ''                NOT NULL,
	site_notes               text                                      NOT NULL,
	poc_1_name               varchar(128)    DEFAULT ''                NOT NULL,
	poc_1_email              varchar(128)    DEFAULT ''                NOT NULL,
	poc_1_phone_a            varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_phone_b            varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_cell               varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_screen             varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_notes              text                                      NOT NULL,
	poc_2_name               varchar(128)    DEFAULT ''                NOT NULL,
	poc_2_email              varchar(128)    DEFAULT ''                NOT NULL,
	poc_2_phone_a            varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_phone_b            varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_cell               varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_screen             varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_notes              text                                      NOT NULL,
	PRIMARY KEY (hostid)
) ENGINE=InnoDB;
ALTER TABLE host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;

-- create temporary t_host_inventory table
CREATE TABLE t_host_inventory (
	hostid                   bigint unsigned,
	inventory_mode           integer,
	type                     varchar(64),
	type_full                varchar(64),
	name                     varchar(64),
	alias                    varchar(64),
	os                       varchar(64),
	os_full                  varchar(255),
	os_short                 varchar(64),
	serialno_a               varchar(64),
	serialno_b               varchar(64),
	tag                      varchar(64),
	asset_tag                varchar(64),
	macaddress_a             varchar(64),
	macaddress_b             varchar(64),
	hardware                 varchar(255),
	hardware_full            text,
	software                 varchar(255),
	software_full            text,
	software_app_a           varchar(64),
	software_app_b           varchar(64),
	software_app_c           varchar(64),
	software_app_d           varchar(64),
	software_app_e           varchar(64),
	contact                  text,
	location                 text,
	location_lat             varchar(16),
	location_lon             varchar(16),
	notes                    text,
	chassis                  varchar(64),
	model                    varchar(64),
	hw_arch                  varchar(32),
	vendor                   varchar(64),
	contract_number          varchar(64),
	installer_name           varchar(64),
	deployment_status        varchar(64),
	url_a                    varchar(255),
	url_b                    varchar(255),
	url_c                    varchar(255),
	host_networks            text,
	host_netmask             varchar(39),
	host_router              varchar(39),
	oob_ip                   varchar(39),
	oob_netmask              varchar(39),
	oob_router               varchar(39),
	date_hw_purchase         varchar(64),
	date_hw_install          varchar(64),
	date_hw_expiry           varchar(64),
	date_hw_decomm           varchar(64),
	site_address_a           varchar(128),
	site_address_b           varchar(128),
	site_address_c           varchar(128),
	site_city                varchar(128),
	site_state               varchar(64),
	site_country             varchar(64),
	site_zip                 varchar(64),
	site_rack                varchar(128),
	site_notes               text,
	poc_1_name               varchar(128),
	poc_1_email              varchar(128),
	poc_1_phone_a            varchar(64),
	poc_1_phone_b            varchar(64),
	poc_1_cell               varchar(64),
	poc_1_screen             varchar(64),
	poc_1_notes              text,
	poc_2_name               varchar(128),
	poc_2_email              varchar(128),
	poc_2_phone_a            varchar(64),
	poc_2_phone_b            varchar(64),
	poc_2_cell               varchar(64),
	poc_2_screen             varchar(64),
	poc_2_notes              text,
	notes_ext                text
);

-- select all inventories into temporary table
INSERT INTO t_host_inventory
	SELECT p.hostid,0,p.devicetype,ep.device_type,p.name,ep.device_alias,p.os,ep.device_os,ep.device_os_short,
		p.serialno,ep.device_serial,p.tag,ep.device_tag,p.macaddress,ep.ip_macaddress,ep.device_hardware,
		p.hardware,ep.device_software,p.software,ep.device_app_01,ep.device_app_02,ep.device_app_03,
		ep.device_app_04,ep.device_app_05,p.contact,p.location,'','',p.notes,ep.device_chassis,ep.device_model,
		ep.device_hw_arch,ep.device_vendor,ep.device_contract,ep.device_who,ep.device_status,ep.device_url_1,
		ep.device_url_2,ep.device_url_3,ep.device_networks,ep.ip_subnet_mask,ep.ip_router,ep.oob_ip,
		ep.oob_subnet_mask,ep.oob_router,ep.date_hw_buy,ep.date_hw_install,ep.date_hw_expiry,ep.date_hw_decomm,
		ep.site_street_1,ep.site_street_2,ep.site_street_3,ep.site_city,ep.site_state,ep.site_country,
		ep.site_zip,ep.site_rack,ep.site_notes,ep.poc_1_name,ep.poc_1_email,ep.poc_1_phone_1,ep.poc_1_phone_2,
		ep.poc_1_cell,ep.poc_1_screen,ep.poc_1_notes,ep.poc_2_name,ep.poc_2_email,ep.poc_2_phone_1,
		ep.poc_2_phone_2,ep.poc_2_cell,ep.poc_2_screen,ep.poc_2_notes,ep.device_notes
	FROM hosts_profiles p LEFT JOIN hosts_profiles_ext ep on p.hostid=ep.hostid
	UNION ALL
	SELECT ep.hostid,0,p.devicetype,ep.device_type,p.name,ep.device_alias,p.os,ep.device_os,ep.device_os_short,
		p.serialno,ep.device_serial,p.tag,ep.device_tag,p.macaddress,ep.ip_macaddress,ep.device_hardware,
		p.hardware,ep.device_software,p.software,ep.device_app_01,ep.device_app_02,ep.device_app_03,
		ep.device_app_04,ep.device_app_05,p.contact,p.location,'','',p.notes,ep.device_chassis,ep.device_model,
		ep.device_hw_arch,ep.device_vendor,ep.device_contract,ep.device_who,ep.device_status,ep.device_url_1,
		ep.device_url_2,ep.device_url_3,ep.device_networks,ep.ip_subnet_mask,ep.ip_router,ep.oob_ip,
		ep.oob_subnet_mask,ep.oob_router,ep.date_hw_buy,ep.date_hw_install,ep.date_hw_expiry,ep.date_hw_decomm,
		ep.site_street_1,ep.site_street_2,ep.site_street_3,ep.site_city,ep.site_state,ep.site_country,
		ep.site_zip,ep.site_rack,ep.site_notes,ep.poc_1_name,ep.poc_1_email,ep.poc_1_phone_1,ep.poc_1_phone_2,
		ep.poc_1_cell,ep.poc_1_screen,ep.poc_1_notes,ep.poc_2_name,ep.poc_2_email,ep.poc_2_phone_1,
		ep.poc_2_phone_2,ep.poc_2_cell,ep.poc_2_screen,ep.poc_2_notes,ep.device_notes
	FROM hosts_profiles p RIGHT JOIN hosts_profiles_ext ep on p.hostid=ep.hostid
	WHERE p.hostid IS NULL;

UPDATE t_host_inventory SET type='' WHERE type IS NULL;
UPDATE t_host_inventory SET type_full='' WHERE type_full IS NULL;
UPDATE t_host_inventory SET name='' WHERE name IS NULL;
UPDATE t_host_inventory SET alias='' WHERE alias IS NULL;
UPDATE t_host_inventory SET os='' WHERE os IS NULL;
UPDATE t_host_inventory SET os_full='' WHERE os_full IS NULL;
UPDATE t_host_inventory SET os_short='' WHERE os_short IS NULL;
UPDATE t_host_inventory SET serialno_a='' WHERE serialno_a IS NULL;
UPDATE t_host_inventory SET serialno_b='' WHERE serialno_b IS NULL;
UPDATE t_host_inventory SET tag='' WHERE tag IS NULL;
UPDATE t_host_inventory SET asset_tag='' WHERE asset_tag IS NULL;
UPDATE t_host_inventory SET macaddress_a='' WHERE macaddress_a IS NULL;
UPDATE t_host_inventory SET macaddress_b='' WHERE macaddress_b IS NULL;
UPDATE t_host_inventory SET hardware='' WHERE hardware IS NULL;
UPDATE t_host_inventory SET hardware_full='' WHERE hardware_full IS NULL;
UPDATE t_host_inventory SET software='' WHERE software IS NULL;
UPDATE t_host_inventory SET software_full='' WHERE software_full IS NULL;
UPDATE t_host_inventory SET software_app_a='' WHERE software_app_a IS NULL;
UPDATE t_host_inventory SET software_app_b='' WHERE software_app_b IS NULL;
UPDATE t_host_inventory SET software_app_c='' WHERE software_app_c IS NULL;
UPDATE t_host_inventory SET software_app_d='' WHERE software_app_d IS NULL;
UPDATE t_host_inventory SET software_app_e='' WHERE software_app_e IS NULL;
UPDATE t_host_inventory SET contact='' WHERE contact IS NULL;
UPDATE t_host_inventory SET location='' WHERE location IS NULL;
UPDATE t_host_inventory SET location_lat='' WHERE location_lat IS NULL;
UPDATE t_host_inventory SET location_lon='' WHERE location_lon IS NULL;
UPDATE t_host_inventory SET notes='' WHERE notes IS NULL;
UPDATE t_host_inventory SET chassis='' WHERE chassis IS NULL;
UPDATE t_host_inventory SET model='' WHERE model IS NULL;
UPDATE t_host_inventory SET hw_arch='' WHERE hw_arch IS NULL;
UPDATE t_host_inventory SET vendor='' WHERE vendor IS NULL;
UPDATE t_host_inventory SET contract_number='' WHERE contract_number IS NULL;
UPDATE t_host_inventory SET installer_name='' WHERE installer_name IS NULL;
UPDATE t_host_inventory SET deployment_status='' WHERE deployment_status IS NULL;
UPDATE t_host_inventory SET url_a='' WHERE url_a IS NULL;
UPDATE t_host_inventory SET url_b='' WHERE url_b IS NULL;
UPDATE t_host_inventory SET url_c='' WHERE url_c IS NULL;
UPDATE t_host_inventory SET host_networks='' WHERE host_networks IS NULL;
UPDATE t_host_inventory SET host_netmask='' WHERE host_netmask IS NULL;
UPDATE t_host_inventory SET host_router='' WHERE host_router IS NULL;
UPDATE t_host_inventory SET oob_ip='' WHERE oob_ip IS NULL;
UPDATE t_host_inventory SET oob_netmask='' WHERE oob_netmask IS NULL;
UPDATE t_host_inventory SET oob_router='' WHERE oob_router IS NULL;
UPDATE t_host_inventory SET date_hw_purchase='' WHERE date_hw_purchase IS NULL;
UPDATE t_host_inventory SET date_hw_install='' WHERE date_hw_install IS NULL;
UPDATE t_host_inventory SET date_hw_expiry='' WHERE date_hw_expiry IS NULL;
UPDATE t_host_inventory SET date_hw_decomm='' WHERE date_hw_decomm IS NULL;
UPDATE t_host_inventory SET site_address_a='' WHERE site_address_a IS NULL;
UPDATE t_host_inventory SET site_address_b='' WHERE site_address_b IS NULL;
UPDATE t_host_inventory SET site_address_c='' WHERE site_address_c IS NULL;
UPDATE t_host_inventory SET site_city='' WHERE site_city IS NULL;
UPDATE t_host_inventory SET site_state='' WHERE site_state IS NULL;
UPDATE t_host_inventory SET site_country='' WHERE site_country IS NULL;
UPDATE t_host_inventory SET site_zip='' WHERE site_zip IS NULL;
UPDATE t_host_inventory SET site_rack='' WHERE site_rack IS NULL;
UPDATE t_host_inventory SET site_notes='' WHERE site_notes IS NULL;
UPDATE t_host_inventory SET poc_1_name='' WHERE poc_1_name IS NULL;
UPDATE t_host_inventory SET poc_1_email='' WHERE poc_1_email IS NULL;
UPDATE t_host_inventory SET poc_1_phone_a='' WHERE poc_1_phone_a IS NULL;
UPDATE t_host_inventory SET poc_1_phone_b='' WHERE poc_1_phone_b IS NULL;
UPDATE t_host_inventory SET poc_1_cell='' WHERE poc_1_cell IS NULL;
UPDATE t_host_inventory SET poc_1_screen='' WHERE poc_1_screen IS NULL;
UPDATE t_host_inventory SET poc_1_notes='' WHERE poc_1_notes IS NULL;
UPDATE t_host_inventory SET poc_2_name='' WHERE poc_2_name IS NULL;
UPDATE t_host_inventory SET poc_2_email='' WHERE poc_2_email IS NULL;
UPDATE t_host_inventory SET poc_2_phone_a='' WHERE poc_2_phone_a IS NULL;
UPDATE t_host_inventory SET poc_2_phone_b='' WHERE poc_2_phone_b IS NULL;
UPDATE t_host_inventory SET poc_2_cell='' WHERE poc_2_cell IS NULL;
UPDATE t_host_inventory SET poc_2_screen='' WHERE poc_2_screen IS NULL;
UPDATE t_host_inventory SET poc_2_notes='' WHERE poc_2_notes IS NULL;

-- merge notes field
UPDATE t_host_inventory SET notes_ext='' WHERE notes_ext IS NULL;
UPDATE t_host_inventory SET notes=CONCAT(notes, '\r\n', notes_ext) WHERE notes<>'' AND notes_ext<>'';
UPDATE t_host_inventory SET notes=notes_ext WHERE notes='';
ALTER TABLE t_host_inventory DROP COLUMN notes_ext;

-- copy data from temporary table
INSERT INTO host_inventory SELECT * FROM t_host_inventory;

DROP TABLE t_host_inventory;
DROP TABLE hosts_profiles;
DROP TABLE hosts_profiles_ext;

DELETE FROM ids WHERE table_name IN ('hosts_profiles', 'hosts_profiles_ext');
ALTER TABLE hostmacro MODIFY hostmacroid bigint unsigned NOT NULL,
		      MODIFY hostid bigint unsigned NOT NULL;
DROP INDEX hostmacro_1 ON hostmacro;
DELETE FROM hostmacro WHERE NOT hostid IN (SELECT hostid FROM hosts);

-- remove duplicates to allow unique index
CREATE TEMPORARY TABLE tmp_hostmacro (hostmacroid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_hostmacro (hostmacroid) (
	SELECT MIN(hostmacroid)
		FROM hostmacro
		GROUP BY hostid,macro
);
DELETE FROM hostmacro WHERE hostmacroid NOT IN (SELECT hostmacroid FROM tmp_hostmacro);
DROP TABLE tmp_hostmacro;

CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);
ALTER TABLE hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups MODIFY hostgroupid bigint unsigned NOT NULL,
			 MODIFY hostid bigint unsigned NOT NULL,
			 MODIFY groupid bigint unsigned NOT NULL;
DROP INDEX hosts_groups_1 ON hosts_groups;
DELETE FROM hosts_groups WHERE NOT hostid IN (SELECT hostid FROM hosts);
DELETE FROM hosts_groups WHERE NOT groupid IN (SELECT groupid FROM groups);

-- remove duplicates to allow unique index
CREATE TEMPORARY TABLE tmp_hosts_groups (hostgroupid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_hosts_groups (hostgroupid) (
	SELECT MIN(hostgroupid)
		FROM hosts_groups
		GROUP BY hostid,groupid
);
DELETE FROM hosts_groups WHERE hostgroupid NOT IN (SELECT hostgroupid FROM tmp_hosts_groups);
DROP TABLE tmp_hosts_groups;

CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
-- See host_inventory.sql
-- See host_inventory.sql
-- Patching table `interfaces`

CREATE TABLE interface (
	interfaceid              bigint unsigned                           NOT NULL,
	hostid                   bigint unsigned                           NOT NULL,
	main                     integer         DEFAULT '0'               NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	useip                    integer         DEFAULT '1'               NOT NULL,
	ip                       varchar(39)     DEFAULT '127.0.0.1'       NOT NULL,
	dns                      varchar(64)     DEFAULT ''                NOT NULL,
	port                     varchar(64)     DEFAULT '10050'           NOT NULL,
	PRIMARY KEY (interfaceid)
) ENGINE=InnoDB;
CREATE INDEX interface_1 on interface (hostid,type);
CREATE INDEX interface_2 on interface (ip,dns);
ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;

-- Passive proxy interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid div 100000000000)*100000000000)) * 3 + ((hostid div 100000000000)*100000000000),
		hostid,1,0,ip,dns,useip,port
	FROM hosts
	WHERE status IN (6));	-- HOST_STATUS_PROXY_PASSIVE

-- Zabbix Agent interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid div 100000000000)*100000000000)) * 3 + ((hostid div 100000000000)*100000000000),
		hostid,1,1,ip,dns,useip,port
	FROM hosts
	WHERE status IN (0,1));

-- SNMP interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid div 100000000000)*100000000000)) * 3 + ((hostid div 100000000000)*100000000000) + 1,
		hostid,1,2,ip,dns,useip,'161'
	FROM hosts
	WHERE status IN (0,1)
		AND EXISTS (SELECT DISTINCT i.hostid FROM items i WHERE i.hostid=hosts.hostid and i.type IN (1,4,6)));	-- SNMPv1, SNMPv2c, SNMPv3

-- IPMI interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid div 100000000000)*100000000000)) * 3 + ((hostid div 100000000000)*100000000000) + 2,
		hostid,1,3,'',ipmi_ip,0,ipmi_port
	FROM hosts
	WHERE status IN (0,1) AND useipmi=1);

-- Patching table `items`

ALTER TABLE items
	CHANGE COLUMN description name VARCHAR(255) NOT NULL DEFAULT '',
	MODIFY itemid bigint unsigned NOT NULL,
	MODIFY hostid bigint unsigned NOT NULL,
	MODIFY units varchar(255) DEFAULT '' NOT NULL,
	MODIFY lastlogsize bigint unsigned DEFAULT '0' NOT NULL,
	MODIFY templateid bigint unsigned NULL,
	MODIFY valuemapid bigint unsigned NULL,
	ADD lastns integer NULL,
	ADD flags integer DEFAULT '0' NOT NULL,
	ADD filter varchar(255) DEFAULT '' NOT NULL,
	ADD interfaceid bigint unsigned NULL,
	ADD port varchar(64) DEFAULT '' NOT NULL,
	ADD description text NOT NULL,
	ADD inventory_link integer DEFAULT '0' NOT NULL,
	ADD lifetime varchar(64) DEFAULT '30' NOT NULL;
CREATE TEMPORARY TABLE tmp_items_itemid (itemid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_items_itemid (itemid) (SELECT itemid FROM items);
UPDATE items
	SET templateid=NULL
	WHERE templateid=0
		OR templateid NOT IN (SELECT itemid FROM tmp_items_itemid);
DROP TABLE tmp_items_itemid;
UPDATE items
	SET valuemapid=NULL
	WHERE valuemapid=0
		OR valuemapid NOT IN (SELECT valuemapid FROM valuemaps);
UPDATE items SET units='Bps' WHERE type=9 AND units='bps';
DELETE FROM items WHERE hostid NOT IN (SELECT hostid FROM hosts);
ALTER TABLE items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid);
ALTER TABLE items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid);

UPDATE items SET port=snmp_port;
ALTER TABLE items DROP COLUMN snmp_port;

CREATE INDEX items_5 on items (valuemapid);

-- host interface for non IPMI, SNMP and non templated items
UPDATE items
	SET interfaceid=(SELECT interfaceid FROM interface WHERE hostid=items.hostid AND main=1 AND type=1)
	WHERE EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=items.hostid AND hosts.status IN (0,1))
		AND type IN (0,3,10,11,13,14);	-- ZABBIX, SIMPLE, EXTERNAL, DB_MONITOR, SSH, TELNET

-- host interface for SNMP and non templated items
UPDATE items
	SET interfaceid=(SELECT interfaceid FROM interface WHERE hostid=items.hostid AND main=1 AND type=2)
	WHERE EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=items.hostid AND hosts.status IN (0,1))
		AND type IN (1,4,6);		-- SNMPv1, SNMPv2c, SNMPv3

-- host interface for IPMI and non templated items
UPDATE items
	SET interfaceid=(SELECT interfaceid FROM interface WHERE hostid=items.hostid AND main=1 AND type=3)
	WHERE EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=items.hostid AND hosts.status IN (0,1))
		AND type IN (12);		-- IPMI

-- clear port number for non SNMP items
UPDATE items
	SET port=''
	WHERE type NOT IN (1,4,6);		-- SNMPv1, SNMPv2c, SNMPv3

-- add a first parameter {HOST.CONN} for external checks

UPDATE items
	SET key_ = CONCAT(SUBSTR(key_, 1, INSTR(key_, '[')), '"{HOST.CONN}",', SUBSTR(key_, INSTR(key_, '[') + 1))
	WHERE type IN (10)	-- EXTERNAL
		AND INSTR(key_, '[') <> 0;

UPDATE items
	SET key_ = CONCAT(key_, '["{HOST.CONN}"]')
	WHERE type IN (10)	-- EXTERNAL
		AND INSTR(key_, '[') = 0;

-- convert simple check keys to a new form

DELIMITER $
CREATE FUNCTION zbx_convert_simple_checks(v_itemid bigint unsigned, v_hostid bigint unsigned, v_key varchar(255))
RETURNS varchar(255)
LANGUAGE SQL
DETERMINISTIC
BEGIN
	DECLARE new_key varchar(255);
	DECLARE pos integer;

	SET new_key = 'net.tcp.service';
	SET pos = INSTR(v_key, '_perf');
	IF 0 <> pos THEN
		SET new_key = CONCAT(new_key, '.perf');
		SET v_key = CONCAT(SUBSTR(v_key, 1, pos - 1), SUBSTR(v_key, pos + 5));
	END IF;
	SET new_key = CONCAT(new_key, '[');
	SET pos = INSTR(v_key, ',');
	IF 0 <> pos THEN
		SET new_key = CONCAT(new_key, '"', SUBSTR(v_key, 1, pos - 1), '"');
		SET v_key = SUBSTR(v_key, pos + 1);
	ELSE
		SET new_key = CONCAT(new_key, '"', v_key, '"');
		SET v_key = '';
	END IF;
	IF 0 <> LENGTH(v_key) THEN
		SET new_key = CONCAT(new_key, ',,"', v_key, '"');
	END IF;

	WHILE 0 != (SELECT COUNT(*) FROM items WHERE hostid = v_hostid AND key_ = CONCAT(new_key, ']')) DO
		SET new_key = CONCAT(new_key, ' ');
	END WHILE;

	RETURN CONCAT(new_key, ']');
END$
DELIMITER ;

UPDATE items SET key_ = zbx_convert_simple_checks(itemid, hostid, key_)
	WHERE type IN (3)	-- SIMPLE
		AND (key_ IN ('ftp','http','imap','ldap','nntp','ntp','pop','smtp','ssh',
			'ftp_perf','http_perf', 'imap_perf','ldap_perf','nntp_perf','ntp_perf','pop_perf',
			'smtp_perf','ssh_perf')
			OR key_ LIKE 'ftp,%' OR key_ LIKE 'http,%' OR key_ LIKE 'imap,%' OR key_ LIKE 'ldap,%'
			OR key_ LIKE 'nntp,%' OR key_ LIKE 'ntp,%' OR key_ LIKE 'pop,%' OR key_ LIKE 'smtp,%'
			OR key_ LIKE 'ssh,%' OR key_ LIKE 'tcp,%'
			OR key_ LIKE 'ftp_perf,%' OR key_ LIKE 'http_perf,%' OR key_ LIKE 'imap_perf,%'
			OR key_ LIKE 'ldap_perf,%' OR key_ LIKE 'nntp_perf,%' OR key_ LIKE 'ntp_perf,%'
			OR key_ LIKE 'pop_perf,%' OR key_ LIKE 'smtp_perf,%' OR key_ LIKE 'ssh_perf,%'
			OR key_ LIKE 'tcp_perf,%');

DROP FUNCTION zbx_convert_simple_checks;

-- adding web.test.error[<web check>] items

DROP PROCEDURE IF EXISTS zbx_add_web_error_items;

DELIMITER $
CREATE PROCEDURE zbx_add_web_error_items()
LANGUAGE SQL
BEGIN
	DECLARE v_nodeid integer;
	DECLARE init_nodeid bigint unsigned;
	DECLARE minid, maxid bigint unsigned;
	DECLARE n_done integer DEFAULT 0;
	DECLARE n_cur CURSOR FOR (SELECT DISTINCT httptestid div 100000000000000 FROM httptest);
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET n_done = 1;

	OPEN n_cur;

	n_loop: LOOP
		FETCH n_cur INTO v_nodeid;

		IF n_done THEN
			LEAVE n_loop;
		END IF;

		SET minid = v_nodeid * 100000000000000;
		SET maxid = minid + 99999999999999;
		SET init_nodeid = (v_nodeid * 1000 + v_nodeid) * 100000000000;

		SET @itemid = (SELECT MAX(itemid) FROM items where itemid BETWEEN minid AND maxid);
		IF @itemid IS NULL THEN
			SET @itemid = init_nodeid;
		END IF;

		SET @httptestitemid = (SELECT MAX(httptestitemid) FROM httptestitem where httptestitemid BETWEEN minid AND maxid);
		IF @httptestitemid IS NULL THEN
			SET @httptestitemid = init_nodeid;
		END IF;

		SET @itemappid = (SELECT MAX(itemappid) FROM items_applications where itemappid BETWEEN minid AND maxid);
		IF @itemappid IS NULL THEN
			SET @itemappid = init_nodeid;
		END IF;

		INSERT INTO items (itemid, hostid, type, name, key_, value_type, units, delay, history, trends, status, params, description)
			SELECT @itemid := @itemid + 1, hostid, type, 'Last error message of scenario \'$1\'', CONCAT('web.test.error',
				SUBSTR(key_, LOCATE('[', key_))), 1, '', delay, history, 0, status, '', ''
				FROM items
				WHERE type = 9
					AND key_ LIKE 'web.test.fail%'
					AND itemid BETWEEN minid AND maxid;

		INSERT INTO httptestitem (httptestitemid, httptestid, itemid, type)
			SELECT @httptestitemid := @httptestitemid + 1, ht.httptestid, i.itemid, 4
				FROM httptest ht,applications a,items i
				WHERE ht.applicationid=a.applicationid
					AND a.hostid=i.hostid
					AND CONCAT('web.test.error[', ht.name, ']') = i.key_
					AND i.itemid BETWEEN minid AND maxid;

		INSERT INTO items_applications (itemappid, applicationid, itemid)
			SELECT @itemappid := @itemappid + 1, ht.applicationid, hti.itemid
				FROM httptest ht, httptestitem hti
				WHERE ht.httptestid = hti.httptestid
					AND hti.type = 4
					AND hti.itemid BETWEEN minid AND maxid;

	END LOOP n_loop;

	CLOSE n_cur;
END$
DELIMITER ;

CALL zbx_add_web_error_items();
DROP PROCEDURE zbx_add_web_error_items;
DELETE FROM ids WHERE table_name IN ('items', 'httptestitem', 'items_applications');

-- Patching table `hosts`

ALTER TABLE hosts MODIFY hostid bigint unsigned NOT NULL,
		  MODIFY proxy_hostid bigint unsigned NULL,
		  MODIFY maintenanceid bigint unsigned NULL,
		  DROP COLUMN ip,
		  DROP COLUMN dns,
		  DROP COLUMN port,
		  DROP COLUMN useip,
		  DROP COLUMN useipmi,
		  DROP COLUMN ipmi_ip,
		  DROP COLUMN ipmi_port,
		  DROP COLUMN inbytes,
		  DROP COLUMN outbytes,
		  ADD jmx_disable_until integer DEFAULT '0' NOT NULL,
		  ADD jmx_available integer DEFAULT '0' NOT NULL,
		  ADD jmx_errors_from integer DEFAULT '0' NOT NULL,
		  ADD jmx_error varchar(128) DEFAULT '' NOT NULL,
		  ADD name varchar(64) DEFAULT '' NOT NULL;
CREATE TEMPORARY TABLE tmp_hosts_hostid (hostid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_hosts_hostid (hostid) (SELECT hostid FROM hosts);
UPDATE hosts
	SET proxy_hostid=NULL
	WHERE proxy_hostid=0
		OR proxy_hostid NOT IN (SELECT hostid FROM tmp_hosts_hostid);
DROP TABLE tmp_hosts_hostid;
UPDATE hosts
	SET maintenanceid=NULL,
		maintenance_status=0,
		maintenance_type=0,
		maintenance_from=0
	WHERE maintenanceid=0
		OR maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
UPDATE hosts SET name=host WHERE status in (0,1,3);	-- MONITORED, NOT_MONITORED, TEMPLATE
CREATE INDEX hosts_4 on hosts (name);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid);
DELETE FROM hosts_templates WHERE hostid NOT IN (SELECT hostid FROM hosts);
DELETE FROM hosts_templates WHERE templateid NOT IN (SELECT hostid FROM hosts);

CREATE TABLE t_hosts_templates (
	hosttemplateid           bigint unsigned                           NOT NULL,
	hostid                   bigint unsigned                           NOT NULL,
	templateid               bigint unsigned                           NOT NULL
);

INSERT INTO t_hosts_templates (SELECT hosttemplateid, hostid, templateid FROM hosts_templates);

DROP TABLE hosts_templates;

CREATE TABLE hosts_templates (
	hosttemplateid           bigint unsigned                           NOT NULL,
	hostid                   bigint unsigned                           NOT NULL,
	templateid               bigint unsigned                           NOT NULL,
	PRIMARY KEY (hosttemplateid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;

INSERT INTO hosts_templates (SELECT hosttemplateid, hostid, templateid FROM t_hosts_templates);

DROP TABLE t_hosts_templates;
ALTER TABLE housekeeper MODIFY housekeeperid bigint unsigned NOT NULL,
			MODIFY value bigint unsigned NOT NULL;
ALTER TABLE httpstepitem MODIFY httpstepitemid bigint unsigned NOT NULL,
			 MODIFY httpstepid bigint unsigned NOT NULL,
			 MODIFY itemid bigint unsigned NOT NULL;
DELETE FROM httpstepitem WHERE NOT httpstepid IN (SELECT httpstepid FROM httpstep);
DELETE FROM httpstepitem WHERE NOT itemid IN (SELECT itemid FROM items);
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE;
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE httpstep
	MODIFY httpstepid bigint unsigned NOT NULL,
	MODIFY httptestid bigint unsigned NOT NULL,
	MODIFY posts text NOT NULL;
DELETE FROM httpstep WHERE NOT httptestid IN (SELECT httptestid FROM httptest);
ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httptestitem MODIFY httptestitemid bigint unsigned NOT NULL,
			 MODIFY httptestid bigint unsigned NOT NULL,
			 MODIFY itemid bigint unsigned NOT NULL;
DELETE FROM httptestitem WHERE NOT httptestid IN (SELECT httptestid FROM httptest);
DELETE FROM httptestitem WHERE NOT itemid IN (SELECT itemid FROM items);
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE httptest
	MODIFY httptestid bigint unsigned NOT NULL,
	MODIFY applicationid bigint unsigned NOT NULL,
	MODIFY macros text NOT NULL,
	DROP COLUMN lastcheck,
	DROP COLUMN curstate,
	DROP COLUMN curstep,
	DROP COLUMN lastfailedstep,
	DROP COLUMN time,
	DROP COLUMN error;
DELETE FROM httptest WHERE applicationid NOT IN (SELECT applicationid FROM applications);
ALTER TABLE httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
-- See icon_map.sql
CREATE TABLE icon_map (
	iconmapid                bigint unsigned                           NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	default_iconid           bigint unsigned                           NOT NULL,
	PRIMARY KEY (iconmapid)
) ENGINE=InnoDB;
CREATE INDEX icon_map_1 ON icon_map (name);
ALTER TABLE icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid);

CREATE TABLE icon_mapping (
	iconmappingid            bigint unsigned                           NOT NULL,
	iconmapid                bigint unsigned                           NOT NULL,
	iconid                   bigint unsigned                           NOT NULL,
	inventory_link           integer         DEFAULT '0'               NOT NULL,
	expression               varchar(64)     DEFAULT ''                NOT NULL,
	sortorder                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (iconmappingid)
) ENGINE=InnoDB;
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);
ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE;
ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid);
ALTER TABLE ids MODIFY nodeid integer NOT NULL;
ALTER TABLE ids MODIFY nextid bigint unsigned NOT NULL;
ALTER TABLE images MODIFY imageid bigint unsigned NOT NULL;
-- See hosts.sql
CREATE TABLE item_discovery (
	itemdiscoveryid          bigint unsigned                           NOT NULL,
	itemid                   bigint unsigned                           NOT NULL,
	parent_itemid            bigint unsigned                           NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	lastcheck                integer         DEFAULT '0'               NOT NULL,
	ts_delete                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX item_discovery_1 on item_discovery (itemid,parent_itemid);
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE items_applications MODIFY itemappid bigint unsigned NOT NULL,
			       MODIFY applicationid bigint unsigned NOT NULL,
			       MODIFY itemid bigint unsigned NOT NULL;
DROP INDEX items_applications_1 ON items_applications;
DELETE FROM items_applications WHERE applicationid NOT IN (SELECT applicationid FROM applications);
DELETE FROM items_applications WHERE itemid NOT IN (SELECT itemid FROM items);
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);
ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
-- See hosts.sql
ALTER TABLE maintenances_groups MODIFY maintenance_groupid bigint unsigned NOT NULL,
				MODIFY maintenanceid bigint unsigned NOT NULL,
				MODIFY groupid bigint unsigned NOT NULL;
DROP INDEX maintenances_groups_1 ON maintenances_groups;
DELETE FROM maintenances_groups WHERE maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
DELETE FROM maintenances_groups WHERE groupid NOT IN (SELECT groupid FROM groups);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts MODIFY maintenance_hostid bigint unsigned NOT NULL,
			       MODIFY maintenanceid bigint unsigned NOT NULL,
			       MODIFY hostid bigint unsigned NOT NULL;
DROP INDEX maintenances_hosts_1 ON maintenances_hosts;
DELETE FROM maintenances_hosts WHERE maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
DELETE FROM maintenances_hosts WHERE hostid NOT IN (SELECT hostid FROM hosts);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE maintenances
	MODIFY maintenanceid bigint unsigned NOT NULL,
	MODIFY description text NOT NULL;
ALTER TABLE maintenances_windows MODIFY maintenance_timeperiodid bigint unsigned NOT NULL,
				 MODIFY maintenanceid bigint unsigned NOT NULL,
				 MODIFY timeperiodid bigint unsigned NOT NULL;
DROP INDEX maintenances_windows_1 ON maintenances_windows;
DELETE FROM maintenances_windows WHERE maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
DELETE FROM maintenances_windows WHERE timeperiodid NOT IN (SELECT timeperiodid FROM timeperiods);
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE;
ALTER TABLE mappings MODIFY mappingid bigint unsigned NOT NULL,
		     MODIFY valuemapid bigint unsigned NOT NULL;
DELETE FROM mappings WHERE NOT valuemapid IN (SELECT valuemapid FROM valuemaps);
ALTER TABLE mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE;
ALTER TABLE media MODIFY mediaid bigint unsigned NOT NULL,
		  MODIFY userid bigint unsigned NOT NULL,
		  MODIFY mediatypeid bigint unsigned NOT NULL,
		  MODIFY period varchar(100) DEFAULT '1-7,00:00-24:00' NOT NULL;
DELETE FROM media WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM media WHERE NOT mediatypeid IN (SELECT mediatypeid FROM media_type);
ALTER TABLE media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE media_type
	MODIFY mediatypeid bigint unsigned NOT NULL,
	ADD status integer DEFAULT '0' NOT NULL;
DELIMITER $
CREATE PROCEDURE zbx_drop_indexes()
BEGIN
	DECLARE dummy INT DEFAULT 0;
	DECLARE CONTINUE HANDLER FOR SQLSTATE '42000' SET dummy = 1;

	DROP INDEX node_cksum_1 ON node_cksum;
	DROP INDEX node_cksum_cksum_1 ON node_cksum;
END$
DELIMITER ;

CALL zbx_drop_indexes();
DROP PROCEDURE zbx_drop_indexes;

ALTER TABLE node_cksum MODIFY nodeid integer NOT NULL,
		       MODIFY recordid bigint unsigned NOT NULL;
DELETE FROM node_cksum WHERE NOT nodeid IN (SELECT nodeid FROM nodes);
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);
ALTER TABLE node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE;
ALTER TABLE nodes
	MODIFY nodeid integer NOT NULL,
	MODIFY masterid integer NULL,
	DROP COLUMN timezone,
	DROP COLUMN slave_history,
	DROP COLUMN slave_trends;
UPDATE nodes SET masterid=NULL WHERE masterid=0;
ALTER TABLE nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid);
-- See operations.sql
-- See operations.sql
-- See operations.sql
CREATE TEMPORARY TABLE t_operations (
	operationid		bigint unsigned,
	actionid		bigint unsigned,
	operationtype		integer,
	object			integer,
	objectid		bigint unsigned,
	shortdata		varchar(255),
	longdata		blob,
	esc_period		integer,
	esc_step_from		integer,
	esc_step_to		integer,
	default_msg		integer,
	evaltype		integer,
	mediatypeid		bigint unsigned
);

CREATE TEMPORARY TABLE t_opconditions (
	operationid		bigint unsigned,
	conditiontype		integer,
	operator		integer,
	value			varchar(255)
);

INSERT INTO t_operations
	SELECT o.operationid, o.actionid, o.operationtype, o.object, o.objectid, o.shortdata, o.longdata,
			o.esc_period, o.esc_step_from, o.esc_step_to, o.default_msg, o.evaltype, omt.mediatypeid
		FROM actions a, operations o
			LEFT JOIN opmediatypes omt ON omt.operationid=o.operationid
		WHERE a.actionid=o.actionid;

INSERT INTO t_opconditions
	SELECT operationid, conditiontype, operator, value FROM opconditions;

UPDATE t_operations
	SET mediatypeid = NULL
	WHERE NOT EXISTS (SELECT 1 FROM media_type mt WHERE mt.mediatypeid = t_operations.mediatypeid);

UPDATE t_operations
	SET objectid = NULL
	WHERE operationtype = 0		-- OPERATION_TYPE_MESSAGE
		AND object = 0		-- OPERATION_OBJECT_USER
		AND NOT EXISTS (SELECT 1 FROM users u WHERE u.userid = t_operations.objectid);

UPDATE t_operations
	SET objectid = NULL
	WHERE operationtype = 0		-- OPERATION_TYPE_MESSAGE
		AND object = 1		-- OPERATION_OBJECT_GROUP
		AND NOT EXISTS (SELECT 1 FROM usrgrp g WHERE g.usrgrpid = t_operations.objectid);

DELETE FROM t_operations
	WHERE operationtype IN (4,5)	-- OPERATION_TYPE_GROUP_ADD, OPERATION_TYPE_GROUP_REMOVE
		AND NOT EXISTS (SELECT 1 FROM groups g WHERE g.groupid = t_operations.objectid);

DELETE FROM t_operations
	WHERE operationtype IN (6,7)	-- OPERATION_TYPE_TEMPLATE_ADD, OPERATION_TYPE_TEMPLATE_REMOVE
		AND NOT EXISTS (SELECT 1 FROM hosts h WHERE h.hostid = t_operations.objectid);

DROP TABLE operations;
DROP TABLE opmediatypes;
DROP TABLE opconditions;

CREATE TABLE operations (
	operationid              bigint unsigned                           NOT NULL,
	actionid                 bigint unsigned                           NOT NULL,
	operationtype            integer         DEFAULT '0'               NOT NULL,
	esc_period               integer         DEFAULT '0'               NOT NULL,
	esc_step_from            integer         DEFAULT '1'               NOT NULL,
	esc_step_to              integer         DEFAULT '1'               NOT NULL,
	evaltype                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
CREATE INDEX operations_1 ON operations (actionid);
ALTER TABLE operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;

CREATE TABLE opmessage (
	operationid              bigint unsigned                           NOT NULL,
	default_msg              integer         DEFAULT '0'               NOT NULL,
	subject                  varchar(255)    DEFAULT ''                NOT NULL,
	message                  text                                      NOT NULL,
	mediatypeid              bigint unsigned                           NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid);

CREATE TABLE opmessage_grp (
	opmessage_grpid          bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	usrgrpid                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (opmessage_grpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);

CREATE TABLE opmessage_usr (
	opmessage_usrid          bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	userid                   bigint unsigned                           NOT NULL,
	PRIMARY KEY (opmessage_usrid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid);

CREATE TABLE opcommand (
	operationid              bigint unsigned                           NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	scriptid                 bigint unsigned                           NULL,
	execute_on               integer         DEFAULT '0'               NOT NULL,
	port                     varchar(64)     DEFAULT ''                NOT NULL,
	authtype                 integer         DEFAULT '0'               NOT NULL,
	username                 varchar(64)     DEFAULT ''                NOT NULL,
	password                 varchar(64)     DEFAULT ''                NOT NULL,
	publickey                varchar(64)     DEFAULT ''                NOT NULL,
	privatekey               varchar(64)     DEFAULT ''                NOT NULL,
	command                  text                                      NOT NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid);

CREATE TABLE opcommand_hst (
	opcommand_hstid          bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	hostid                   bigint unsigned                           NULL,
	PRIMARY KEY (opcommand_hstid)
) ENGINE=InnoDB;
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid);

CREATE TABLE opcommand_grp (
	opcommand_grpid          bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	groupid                  bigint unsigned                           NOT NULL,
	PRIMARY KEY (opcommand_grpid)
) ENGINE=InnoDB;
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);

CREATE TABLE opgroup (
	opgroupid                bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	groupid                  bigint unsigned                           NOT NULL,
	PRIMARY KEY (opgroupid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);

CREATE TABLE optemplate (
	optemplateid             bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	templateid               bigint unsigned                           NOT NULL,
	PRIMARY KEY (optemplateid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid);

CREATE TABLE opconditions (
	opconditionid            bigint unsigned                           NOT NULL,
	operationid              bigint unsigned                           NOT NULL,
	conditiontype            integer         DEFAULT '0'               NOT NULL,
	operator                 integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (opconditionid)
) ENGINE=InnoDB;
CREATE INDEX opconditions_1 ON opconditions (operationid);
ALTER TABLE opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;

DELIMITER $
CREATE PROCEDURE zbx_convert_operations()
LANGUAGE SQL
BEGIN
	DECLARE v_nodeid integer;
	DECLARE minid, maxid bigint unsigned;
	DECLARE new_operationid bigint unsigned;
	DECLARE new_opmessage_grpid bigint unsigned;
	DECLARE new_opmessage_usrid bigint unsigned;
	DECLARE new_opgroupid bigint unsigned;
	DECLARE new_optemplateid bigint unsigned;
	DECLARE new_opcommand_hstid bigint unsigned;
	DECLARE new_opcommand_grpid bigint unsigned;
	DECLARE n_done integer DEFAULT 0;
	DECLARE n_cur CURSOR FOR (SELECT DISTINCT operationid div 100000000000000 FROM t_operations);
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET n_done = 1;

	OPEN n_cur;

	n_loop: LOOP
		FETCH n_cur INTO v_nodeid;

		IF n_done THEN
			LEAVE n_loop;
		END IF;

		SET minid = v_nodeid * 100000000000000;
		SET maxid = minid + 99999999999999;
		SET new_operationid = minid;
		SET new_opmessage_grpid = minid;
		SET new_opmessage_usrid = minid;
		SET new_opgroupid = minid;
		SET new_optemplateid = minid;
		SET new_opcommand_hstid = minid;
		SET new_opcommand_grpid = minid;
		SET @new_opconditionid = minid;

		BEGIN
			DECLARE v_operationid bigint unsigned;
			DECLARE v_actionid bigint unsigned;
			DECLARE v_operationtype integer;
			DECLARE v_esc_period integer;
			DECLARE v_esc_step_from integer;
			DECLARE v_esc_step_to integer;
			DECLARE v_evaltype integer;
			DECLARE v_default_msg integer;
			DECLARE v_shortdata varchar(255);
			DECLARE v_longdata text;
			DECLARE v_mediatypeid bigint unsigned;
			DECLARE v_object integer;
			DECLARE v_objectid bigint unsigned;
			DECLARE l_pos, r_pos, h_pos, g_pos integer;
			DECLARE cur_string text;
			DECLARE v_host, v_group varchar(64);
			DECLARE v_hostid, v_groupid bigint unsigned;
			DECLARE o_done integer DEFAULT 0;
			DECLARE o_cur CURSOR FOR (
				SELECT operationid, actionid, operationtype, esc_period, esc_step_from, esc_step_to,
						evaltype, default_msg, shortdata, longdata, mediatypeid, object, objectid
					FROM t_operations
					WHERE operationid BETWEEN minid AND maxid);
			DECLARE CONTINUE HANDLER FOR NOT FOUND SET o_done = 1;

			OPEN o_cur;

			o_loop: LOOP
				FETCH o_cur INTO v_operationid, v_actionid, v_operationtype, v_esc_period, v_esc_step_from,
						v_esc_step_to, v_evaltype, v_default_msg, v_shortdata, v_longdata,
						v_mediatypeid, v_object, v_objectid;

				IF o_done THEN
					LEAVE o_loop;
				END IF;

				IF v_operationtype IN (0) THEN			-- OPERATION_TYPE_MESSAGE
					SET new_operationid = new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype, esc_period,
							esc_step_from, esc_step_to, evaltype)
						VALUES (new_operationid, v_actionid, v_operationtype, v_esc_period,
							v_esc_step_from, v_esc_step_to, v_evaltype);

					INSERT INTO opmessage (operationid, default_msg, subject, message, mediatypeid)
						VALUES (new_operationid, v_default_msg, v_shortdata, v_longdata, v_mediatypeid);

					IF v_object = 0 AND v_objectid IS NOT NULL THEN	-- OPERATION_OBJECT_USER
						SET new_opmessage_usrid = new_opmessage_usrid + 1;

						INSERT INTO opmessage_usr (opmessage_usrid, operationid, userid)
							VALUES (new_opmessage_usrid, new_operationid, v_objectid);
					END IF;

					IF v_object = 1 AND v_objectid IS NOT NULL THEN	-- OPERATION_OBJECT_GROUP
						SET new_opmessage_grpid = new_opmessage_grpid + 1;

						INSERT INTO opmessage_grp (opmessage_grpid, operationid, usrgrpid)
							VALUES (new_opmessage_grpid, new_operationid, v_objectid);
					END IF;

					INSERT INTO opconditions
						SELECT @new_opconditionid := @new_opconditionid + 1,
								new_operationid, conditiontype, operator, value
							FROM t_opconditions
							WHERE operationid = v_operationid;
				ELSEIF v_operationtype IN (1) THEN		-- OPERATION_TYPE_COMMAND
					SET r_pos = 1;
					SET l_pos = 1;

					WHILE r_pos > 0 DO
						SET r_pos = LOCATE('\n', v_longdata, l_pos);

						IF r_pos = 0 THEN
							SET cur_string = SUBSTRING(v_longdata, l_pos);
						ELSE
							SET cur_string = SUBSTRING(v_longdata, l_pos, r_pos - l_pos);
						END IF;

						SET cur_string = TRIM(TRAILING '\r' FROM cur_string);
						SET cur_string = TRIM(cur_string);

						IF CHAR_LENGTH(cur_string) <> 0 THEN
							SET h_pos = LOCATE(':', cur_string);
							SET g_pos = LOCATE('#', cur_string);

							IF h_pos <> 0 OR g_pos <> 0 THEN
								SET new_operationid = new_operationid + 1;

								INSERT INTO operations (operationid, actionid, operationtype,
										esc_period, esc_step_from, esc_step_to, evaltype)
								VALUES (new_operationid, v_actionid, v_operationtype, v_esc_period,
										v_esc_step_from, v_esc_step_to, v_evaltype);

								INSERT INTO opconditions
									SELECT @new_opconditionid := @new_opconditionid + 1, new_operationid, conditiontype,
											operator, value
										FROM t_opconditions
										WHERE operationid = v_operationid;

								IF h_pos <> 0 AND (g_pos = 0 OR h_pos < g_pos) THEN
									INSERT INTO opcommand (operationid, command)
										VALUES (new_operationid, TRIM(SUBSTRING(cur_string, h_pos + 1)));

									SET v_host = TRIM(SUBSTRING(cur_string, 1, h_pos - 1));

									IF v_host = '{HOSTNAME}' THEN
										SET new_opcommand_hstid = new_opcommand_hstid + 1;

										INSERT INTO opcommand_hst
											VALUES (new_opcommand_hstid, new_operationid, NULL);
									ELSE
										SET v_hostid = (
											SELECT MIN(hostid)
												FROM hosts
												WHERE host = v_host
													AND (hostid div 100000000000000) = v_nodeid);

										IF v_hostid IS NOT NULL THEN
											SET new_opcommand_hstid = new_opcommand_hstid + 1;

											INSERT INTO opcommand_hst
												VALUES (new_opcommand_hstid, new_operationid, v_hostid);
										END IF;
									END IF;
								END IF;

								IF g_pos <> 0 AND (h_pos = 0 OR g_pos < h_pos) THEN
									INSERT INTO opcommand (operationid, command)
										VALUES (new_operationid, TRIM(SUBSTRING(cur_string, g_pos + 1)));

									SET v_group = TRIM(SUBSTRING(cur_string, 1, g_pos - 1));

									SET v_groupid = (
										SELECT MIN(groupid)
											FROM groups
											WHERE name = v_group
												AND (groupid div 100000000000000) = v_nodeid);

									IF v_groupid IS NOT NULL THEN
										SET new_opcommand_grpid = new_opcommand_grpid + 1;

										INSERT INTO opcommand_grp
											VALUES (new_opcommand_grpid, new_operationid, v_groupid);
									END IF;
								END IF;
							END IF;
						END IF;

						SET l_pos = r_pos + 1;
					END WHILE;
				ELSEIF v_operationtype IN (2, 3, 8, 9) THEN	-- OPERATION_TYPE_HOST_(ADD, REMOVE, ENABLE, DISABLE)
					SET new_operationid = new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype)
						VALUES (new_operationid, v_actionid, v_operationtype);
				ELSEIF v_operationtype IN (4, 5) THEN		-- OPERATION_TYPE_GROUP_(ADD, REMOVE)
					SET new_operationid = new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype)
						VALUES (new_operationid, v_actionid, v_operationtype);

					SET new_opgroupid = new_opgroupid + 1;

					INSERT INTO opgroup (opgroupid, operationid, groupid)
						VALUES (new_opgroupid, new_operationid, v_objectid);
				ELSEIF v_operationtype IN (6, 7) THEN		-- OPERATION_TYPE_TEMPLATE_(ADD, REMOVE)
					SET new_operationid = new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype)
						VALUES (new_operationid, v_actionid, v_operationtype);

					SET new_optemplateid = new_optemplateid + 1;

					INSERT INTO optemplate (optemplateid, operationid, templateid)
						VALUES (new_optemplateid, new_operationid, v_objectid);
				END IF;
			END LOOP o_loop;

			CLOSE o_cur;
		END;
	END LOOP n_loop;

	CLOSE n_cur;
END$
DELIMITER ;

CALL zbx_convert_operations();

DROP TABLE t_operations;
DROP TABLE t_opconditions;
DROP PROCEDURE zbx_convert_operations;

UPDATE opcommand
	SET type = 1, command = TRIM(SUBSTRING(command, 5))
	WHERE SUBSTRING(command, 1, 4) = 'IPMI';

DELETE FROM ids WHERE table_name IN ('operations', 'opconditions', 'opmediatypes');
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
ALTER TABLE profiles
	MODIFY profileid bigint unsigned NOT NULL,
	MODIFY userid bigint unsigned NOT NULL;
DELETE FROM profiles WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM profiles WHERE idx LIKE 'web.%.sort' OR idx LIKE 'web.%.sortorder';
ALTER TABLE profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;

UPDATE profiles SET idx = 'web.screens.period' WHERE idx = 'web.charts.period';
UPDATE profiles SET idx = 'web.screens.stime' WHERE idx = 'web.charts.stime';
UPDATE profiles SET idx = 'web.screens.timelinefixed' WHERE idx = 'web.charts.timelinefixed';
ALTER TABLE proxy_autoreg_host ADD listen_ip varchar(39) DEFAULT '' NOT NULL,
			       ADD listen_port integer DEFAULT '0' NOT NULL,
			       ADD listen_dns varchar(64) DEFAULT '' NOT NULL;
DELETE FROM proxy_dhistory WHERE druleid NOT IN (SELECT druleid FROM drules);
DELETE FROM proxy_dhistory WHERE dcheckid<>0 AND dcheckid NOT IN (SELECT dcheckid FROM dchecks);
ALTER TABLE proxy_dhistory MODIFY druleid bigint unsigned NOT NULL;
ALTER TABLE proxy_dhistory MODIFY dcheckid bigint unsigned NULL;
ALTER TABLE proxy_dhistory ADD dns varchar(64) DEFAULT '' NOT NULL;
UPDATE proxy_dhistory SET dcheckid=NULL WHERE dcheckid=0;
ALTER TABLE proxy_history
	MODIFY itemid bigint unsigned NOT NULL,
	MODIFY value longtext NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL,
	ADD status integer DEFAULT '0' NOT NULL;
ALTER TABLE regexps
	MODIFY regexpid bigint unsigned NOT NULL,
	MODIFY test_string text NOT NULL;
ALTER TABLE rights MODIFY rightid bigint unsigned NOT NULL,
		   MODIFY groupid bigint unsigned NOT NULL,
		   MODIFY id bigint unsigned NOT NULL;
DELETE FROM rights WHERE NOT groupid IN (SELECT usrgrpid FROM usrgrp);
DELETE FROM rights WHERE NOT id IN (SELECT groupid FROM groups);
ALTER TABLE rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE screens_items
	MODIFY screenitemid bigint unsigned NOT NULL,
	MODIFY screenid bigint unsigned NOT NULL,
	ADD sort_triggers integer DEFAULT '0' NOT NULL;
DELETE FROM screens_items WHERE screenid NOT IN (SELECT screenid FROM screens);
ALTER TABLE screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE screens MODIFY screenid bigint unsigned NOT NULL,
		    MODIFY name varchar(255) NOT NULL,
		    ADD templateid bigint unsigned NULL;
ALTER TABLE screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE scripts
	MODIFY scriptid bigint unsigned NOT NULL,
	MODIFY usrgrpid bigint unsigned NULL,
	MODIFY groupid bigint unsigned NULL,
	ADD description text NOT NULL,
	ADD confirmation varchar(255) DEFAULT '' NOT NULL,
	ADD type integer DEFAULT '0' NOT NULL,
	ADD execute_on integer DEFAULT '1' NOT NULL;
UPDATE scripts SET usrgrpid=NULL WHERE usrgrpid=0;
UPDATE scripts SET groupid=NULL WHERE groupid=0;
UPDATE scripts SET type=1,command=TRIM(SUBSTRING(command, 5)) WHERE SUBSTRING(command, 1, 4)='IPMI';
DELETE FROM scripts WHERE usrgrpid IS NOT NULL AND usrgrpid NOT IN (SELECT usrgrpid FROM usrgrp);
DELETE FROM scripts WHERE groupid IS NOT NULL AND groupid NOT IN (SELECT groupid FROM groups);
ALTER TABLE scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE service_alarms MODIFY servicealarmid bigint unsigned NOT NULL,
			   MODIFY serviceid bigint unsigned NOT NULL;
DELETE FROM service_alarms WHERE NOT serviceid IN (SELECT serviceid FROM services);
ALTER TABLE service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_links MODIFY linkid bigint unsigned NOT NULL,
			   MODIFY serviceupid bigint unsigned NOT NULL,
			   MODIFY servicedownid bigint unsigned NOT NULL;
DELETE FROM services_links WHERE NOT serviceupid IN (SELECT serviceid FROM services);
DELETE FROM services_links WHERE NOT servicedownid IN (SELECT serviceid FROM services);
ALTER TABLE services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE;
UPDATE services SET triggerid = NULL WHERE NOT EXISTS (SELECT 1 FROM triggers t WHERE t.triggerid = services.triggerid);
ALTER TABLE services MODIFY serviceid bigint unsigned NOT NULL;
ALTER TABLE services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE services_times MODIFY timeid bigint unsigned NOT NULL,
			   MODIFY serviceid bigint unsigned NOT NULL;
DELETE FROM services_times WHERE NOT serviceid IN (SELECT serviceid FROM services);
ALTER TABLE services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE sessions MODIFY userid bigint unsigned NOT NULL;
DELETE FROM sessions WHERE NOT userid IN (SELECT userid FROM users);
ALTER TABLE sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE slideshows MODIFY slideshowid bigint unsigned NOT NULL;
ALTER TABLE slides MODIFY slideid bigint unsigned NOT NULL,
		   MODIFY slideshowid bigint unsigned NOT NULL,
		   MODIFY screenid bigint unsigned NOT NULL;
DELETE FROM slides WHERE NOT slideshowid IN (SELECT slideshowid FROM slideshows);
DELETE FROM slides WHERE NOT screenid IN (SELECT screenid FROM screens);
ALTER TABLE slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
-- See sysmaps_elements.sql
CREATE TABLE sysmap_element_url (
	sysmapelementurlid       bigint unsigned                           NOT NULL,
	selementid               bigint unsigned                           NOT NULL,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (sysmapelementurlid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX sysmap_element_url_1 on sysmap_element_url (selementid,name);
ALTER TABLE sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;

INSERT INTO sysmap_element_url (sysmapelementurlid,selementid,name,url)
	SELECT selementid,selementid,url,url FROM sysmaps_elements WHERE url<>'';

ALTER TABLE sysmaps_elements
	MODIFY selementid bigint unsigned NOT NULL,
	MODIFY sysmapid bigint unsigned NOT NULL,
	MODIFY iconid_off bigint unsigned NULL,
	MODIFY iconid_on bigint unsigned NULL,
	DROP COLUMN iconid_unknown,
	MODIFY iconid_disabled bigint unsigned NULL,
	MODIFY iconid_maintenance bigint unsigned NULL,
	DROP COLUMN url,
	ADD elementsubtype integer DEFAULT '0' NOT NULL,
	ADD areatype integer DEFAULT '0' NOT NULL,
	ADD width integer DEFAULT '200' NOT NULL,
	ADD height integer DEFAULT '200' NOT NULL,
	ADD viewtype integer DEFAULT '0' NOT NULL,
	ADD use_iconmap integer DEFAULT '1' NOT NULL;

DELETE FROM sysmaps_elements WHERE sysmapid NOT IN (SELECT sysmapid FROM sysmaps);
UPDATE sysmaps_elements SET iconid_off=NULL WHERE iconid_off=0;
UPDATE sysmaps_elements SET iconid_on=NULL WHERE iconid_on=0;
UPDATE sysmaps_elements SET iconid_disabled=NULL WHERE iconid_disabled=0;
UPDATE sysmaps_elements SET iconid_maintenance=NULL WHERE iconid_maintenance=0;
UPDATE sysmaps_elements SET iconid_off=NULL WHERE NOT iconid_off IS NULL AND NOT iconid_off IN (SELECT imageid FROM images WHERE imagetype=1);
UPDATE sysmaps_elements SET iconid_on=NULL WHERE NOT iconid_on IS NULL AND NOT iconid_on IN (SELECT imageid FROM images WHERE imagetype=1);
UPDATE sysmaps_elements SET iconid_disabled=NULL WHERE NOT iconid_disabled IS NULL AND NOT iconid_disabled IN (SELECT imageid FROM images WHERE imagetype=1);
UPDATE sysmaps_elements SET iconid_maintenance=NULL WHERE NOT iconid_maintenance IS NULL AND NOT iconid_maintenance IN (SELECT imageid FROM images WHERE imagetype=1);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_2 FOREIGN KEY (iconid_off) REFERENCES images (imageid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_3 FOREIGN KEY (iconid_on) REFERENCES images (imageid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_4 FOREIGN KEY (iconid_disabled) REFERENCES images (imageid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_5 FOREIGN KEY (iconid_maintenance) REFERENCES images (imageid);
ALTER TABLE sysmaps_links MODIFY linkid bigint unsigned NOT NULL,
			  MODIFY sysmapid bigint unsigned NOT NULL,
			  MODIFY selementid1 bigint unsigned NOT NULL,
			  MODIFY selementid2 bigint unsigned NOT NULL;
DELETE FROM sysmaps_links WHERE sysmapid NOT IN (SELECT sysmapid FROM sysmaps);
DELETE FROM sysmaps_links WHERE selementid1 NOT IN (SELECT selementid FROM sysmaps_elements);
DELETE FROM sysmaps_links WHERE selementid2 NOT IN (SELECT selementid FROM sysmaps_elements);
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmaps_link_triggers MODIFY linktriggerid bigint unsigned NOT NULL,
				  MODIFY linkid bigint unsigned NOT NULL,
				  MODIFY triggerid bigint unsigned NOT NULL;
DELETE FROM sysmaps_link_triggers WHERE linkid NOT IN (SELECT linkid FROM sysmaps_links);
DELETE FROM sysmaps_link_triggers WHERE triggerid NOT IN (SELECT triggerid FROM triggers);
ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE;
ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE sysmaps
	MODIFY sysmapid bigint unsigned NOT NULL,
	MODIFY width integer DEFAULT '600' NOT NULL,
	MODIFY height integer DEFAULT '400' NOT NULL,
	MODIFY backgroundid BIGINT unsigned NULL,
	MODIFY label_type integer DEFAULT '2' NOT NULL,
	MODIFY label_location integer DEFAULT '3' NOT NULL,
	ADD expandproblem integer DEFAULT '1' NOT NULL,
	ADD markelements integer DEFAULT '0' NOT NULL,
	ADD show_unack integer DEFAULT '0' NOT NULL,
	ADD grid_size integer DEFAULT '50' NOT NULL,
	ADD grid_show integer DEFAULT '1' NOT NULL,
	ADD grid_align  integer DEFAULT '1' NOT NULL,
	ADD label_format integer DEFAULT '0' NOT NULL,
	ADD label_type_host integer DEFAULT '2' NOT NULL,
	ADD label_type_hostgroup integer DEFAULT '2' NOT NULL,
	ADD label_type_trigger integer DEFAULT '2' NOT NULL,
	ADD label_type_map integer DEFAULT '2' NOT NULL,
	ADD label_type_image integer DEFAULT '2' NOT NULL,
	ADD label_string_host varchar(255) DEFAULT '' NOT NULL,
	ADD label_string_hostgroup varchar(255) DEFAULT '' NOT NULL,
	ADD label_string_trigger varchar(255) DEFAULT '' NOT NULL,
	ADD label_string_map varchar(255) DEFAULT '' NOT NULL,
	ADD label_string_image varchar(255) DEFAULT '' NOT NULL,
	ADD iconmapid bigint unsigned NULL,
	ADD expand_macros integer DEFAULT '0' NOT NULL;
UPDATE sysmaps SET backgroundid=NULL WHERE backgroundid=0;
UPDATE sysmaps SET show_unack=1 WHERE highlight>7 AND highlight<16;
UPDATE sysmaps SET show_unack=2 WHERE highlight>23;
UPDATE sysmaps SET highlight=(highlight-16) WHERE highlight>15;
UPDATE sysmaps SET highlight=(highlight-8) WHERE highlight>7;
UPDATE sysmaps SET markelements=1 WHERE highlight>3  AND highlight<8;
UPDATE sysmaps SET highlight=(highlight-4) WHERE highlight>3;
UPDATE sysmaps SET expandproblem=0 WHERE highlight>1 AND highlight<4;
UPDATE sysmaps SET highlight=(highlight-2) WHERE highlight>1;
ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_1 FOREIGN KEY (backgroundid) REFERENCES images (imageid);
ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_2 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid);
CREATE TABLE sysmap_url (
	sysmapurlid              bigint unsigned                           NOT NULL,
	sysmapid                 bigint unsigned                           NOT NULL,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	elementtype              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapurlid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX sysmap_url_1 on sysmap_url (sysmapid,name);
ALTER TABLE sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE timeperiods MODIFY timeperiodid bigint unsigned NOT NULL;
ALTER TABLE trends MODIFY itemid bigint unsigned NOT NULL;
ALTER TABLE trends_uint MODIFY itemid bigint unsigned NOT NULL;
ALTER TABLE trigger_depends MODIFY triggerdepid bigint unsigned NOT NULL,
			    MODIFY triggerid_down bigint unsigned NOT NULL,
			    MODIFY triggerid_up bigint unsigned NOT NULL;
DROP INDEX trigger_depends_1 ON trigger_depends;
DELETE FROM trigger_depends WHERE triggerid_down NOT IN (SELECT triggerid FROM triggers);
DELETE FROM trigger_depends WHERE triggerid_up NOT IN (SELECT triggerid FROM triggers);

-- remove duplicates to allow unique index
CREATE TEMPORARY TABLE tmp_trigger_depends (triggerdepid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_trigger_depends (triggerdepid) (
	SELECT MIN(triggerdepid)
		FROM trigger_depends
		GROUP BY triggerid_down,triggerid_up
);
DELETE FROM trigger_depends WHERE triggerdepid NOT IN (SELECT triggerdepid FROM tmp_trigger_depends);
DROP TABLE tmp_trigger_depends;

CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE;
CREATE TABLE trigger_discovery (
	triggerdiscoveryid       bigint unsigned                           NOT NULL,
	triggerid                bigint unsigned                           NOT NULL,
	parent_triggerid         bigint unsigned                           NOT NULL,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (triggerdiscoveryid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX trigger_discovery_1 on trigger_discovery (triggerid,parent_triggerid);
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
--
-- Patching table `events`
--

DROP INDEX events_2 ON events;
CREATE INDEX events_2 ON events (clock);
ALTER TABLE events
	MODIFY eventid bigint unsigned NOT NULL,
	ADD ns integer DEFAULT '0' NOT NULL,
	ADD value_changed integer DEFAULT '0' NOT NULL;

-- Begin event redesign patch

DELIMITER $
CREATE PROCEDURE zbx_convert_events()
LANGUAGE SQL
BEGIN
	DECLARE v_eventid bigint unsigned;
	DECLARE v_triggerid bigint unsigned;
	DECLARE v_value integer;
	DECLARE v_type integer;
	DECLARE prev_triggerid bigint unsigned;
	DECLARE prev_value integer;

	DECLARE n_done integer DEFAULT 0;
	DECLARE n_cur CURSOR FOR (
		SELECT e.eventid, e.objectid, e.value, t.type
			FROM events e
			JOIN triggers t ON t.triggerid = e.objectid
			WHERE e.source = 0		-- EVENT_SOURCE_TRIGGERS
				AND e.object = 0	-- EVENT_OBJECT_TRIGGER
				AND e.value IN (0,1)	-- TRIGGER_VALUE_FALSE (OK), TRIGGER_VALUE_TRUE (PROBLEM)
			ORDER BY e.objectid, e.clock, e.eventid
		);
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET n_done = 1;

	OPEN n_cur;

	n_loop: LOOP
		FETCH n_cur INTO v_eventid, v_triggerid, v_value, v_type;

		IF n_done THEN
			LEAVE n_loop;
		END IF;

		IF prev_triggerid IS NULL OR prev_triggerid <> v_triggerid THEN
			SET prev_value = NULL;
		END IF;

		IF v_value = 0 THEN	-- TRIGGER_VALUE_FALSE (OK)
			-- Which OK events should have value_changed flag set?
			-- (1) those that have a PROBLEM event (or no event) before them
			IF prev_value IS NULL OR prev_value = 1 THEN
				UPDATE events set value_changed = 1 WHERE eventid = v_eventid;
			END IF;
		ELSE			-- TRIGGER_VALUE_TRUE (PROBLEM)
			-- Which PROBLEM events should have value_changed flag set?
			-- (1) those that have an OK event (or no event) before them
			-- (2) those that came from a "MULTIPLE PROBLEM" trigger
			IF v_type = 1 OR prev_value IS NULL OR prev_value = 0 THEN
				UPDATE events set value_changed = 1 WHERE eventid = v_eventid;
			END IF;
		END IF;

		SET prev_value = v_value;
		SET prev_triggerid = v_triggerid;
	END LOOP n_loop;

	CLOSE n_cur;
END$
DELIMITER ;

CALL zbx_convert_events();

DROP PROCEDURE zbx_convert_events;

-- End event redesign patch

--
-- Patching table `triggers`
--

ALTER TABLE triggers
	MODIFY triggerid bigint unsigned NOT NULL,
	MODIFY templateid bigint unsigned NULL,
	MODIFY comments text NOT NULL,
	DROP COLUMN dep_level,
	ADD value_flags integer DEFAULT '0' NOT NULL,
	ADD flags integer DEFAULT '0' NOT NULL;
UPDATE triggers SET templateid=NULL WHERE templateid=0;
CREATE TEMPORARY TABLE tmp_triggers_triggerid (triggerid bigint unsigned PRIMARY KEY);
INSERT INTO tmp_triggers_triggerid (triggerid) (SELECT triggerid FROM triggers);
UPDATE triggers SET templateid=NULL WHERE NOT templateid IS NULL AND NOT templateid IN (SELECT triggerid FROM tmp_triggers_triggerid);
DROP TABLE tmp_triggers_triggerid;
ALTER TABLE triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE;

-- Begin event redesign patch

CREATE TEMPORARY TABLE tmp_triggers (triggerid bigint unsigned PRIMARY KEY, eventid bigint unsigned);

INSERT INTO tmp_triggers (triggerid, eventid)
(
	SELECT t.triggerid, MAX(e.eventid)
		FROM triggers t, events e
		WHERE t.value=2				-- TRIGGER_VALUE_UNKNOWN
			AND e.source=0			-- EVENT_SOURCE_TRIGGERS
			AND e.object=0			-- EVENT_OBJECT_TRIGGER
			AND e.objectid=t.triggerid
			AND e.value<>2			-- TRIGGER_VALUE_UNKNOWN
		GROUP BY t.triggerid
);

UPDATE triggers t1, tmp_triggers t2, events e
	SET t1.value=e.value,
		t1.value_flags=1
	WHERE t1.triggerid=t2.triggerid
		AND t2.eventid=e.eventid;

UPDATE triggers
	SET value=0,					-- TRIGGER_VALUE_FALSE
		value_flags=1
	WHERE value=2;					-- TRIGGER_VALUE_UNKNOWN

DROP TABLE tmp_triggers;

-- End event redesign patch
ALTER TABLE user_history MODIFY userhistoryid bigint unsigned NOT NULL,
			 MODIFY userid bigint unsigned NOT NULL;
DELETE FROM user_history WHERE NOT userid IN (SELECT userid FROM users);
ALTER TABLE user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE users_groups MODIFY id bigint unsigned NOT NULL,
			 MODIFY usrgrpid bigint unsigned NOT NULL,
			 MODIFY userid bigint unsigned NOT NULL;
DELETE FROM users_groups WHERE usrgrpid NOT IN (SELECT usrgrpid FROM usrgrp);
DELETE FROM users_groups WHERE userid NOT IN (SELECT userid FROM users);

-- remove duplicates to allow unique index
CREATE TEMPORARY TABLE tmp_users_groups (id bigint unsigned PRIMARY KEY);
INSERT INTO tmp_users_groups (id) (
	SELECT MIN(id)
		FROM users_groups
		GROUP BY usrgrpid,userid
);
DELETE FROM users_groups WHERE id NOT IN (SELECT id FROM tmp_users_groups);
DROP TABLE tmp_users_groups;

DROP INDEX users_groups_1 ON users_groups;
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE users MODIFY userid bigint unsigned NOT NULL,
	MODIFY lang VARCHAR(5) NOT NULL DEFAULT 'en_GB',
	ALTER theme SET DEFAULT 'default';
UPDATE users SET lang = 'zh_CN' WHERE lang = 'cn_zh';
UPDATE users SET lang = 'es_ES' WHERE lang = 'sp_sp';
UPDATE users SET lang = 'en_GB' WHERE lang = 'en_gb';
UPDATE users SET lang = 'cs_CZ' WHERE lang = 'cs_cz';
UPDATE users SET lang = 'nl_NL' WHERE lang = 'nl_nl';
UPDATE users SET lang = 'fr_FR' WHERE lang = 'fr_fr';
UPDATE users SET lang = 'de_DE' WHERE lang = 'de_de';
UPDATE users SET lang = 'hu_HU' WHERE lang = 'hu_hu';
UPDATE users SET lang = 'ko_KR' WHERE lang = 'ko_kr';
UPDATE users SET lang = 'ja_JP' WHERE lang = 'ja_jp';
UPDATE users SET lang = 'lv_LV' WHERE lang = 'lv_lv';
UPDATE users SET lang = 'pl_PL' WHERE lang = 'pl_pl';
UPDATE users SET lang = 'pt_BR' WHERE lang = 'pt_br';
UPDATE users SET lang = 'ru_RU' WHERE lang = 'ru_ru';
UPDATE users SET lang = 'sv_SE' WHERE lang = 'sv_se';
UPDATE users SET lang = 'uk_UA' WHERE lang = 'ua_ua';

UPDATE users SET theme = 'darkblue' WHERE theme = 'css_bb.css';
UPDATE users SET theme = 'originalblue' WHERE theme = 'css_ob.css';
UPDATE users SET theme = 'darkorange' WHERE theme = 'css_od.css';
UPDATE users SET theme = 'default' WHERE theme = 'default.css';
ALTER TABLE usrgrp MODIFY usrgrpid bigint unsigned NOT NULL,
		   DROP COLUMN api_access;
ALTER TABLE valuemaps MODIFY valuemapid bigint unsigned NOT NULL;
