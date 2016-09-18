ALTER TABLE acknowledges MODIFY acknowledgeid DEFAULT NULL;
ALTER TABLE acknowledges MODIFY userid DEFAULT NULL;
ALTER TABLE acknowledges MODIFY eventid DEFAULT NULL;
DELETE FROM acknowledges WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM acknowledges WHERE NOT eventid IN (SELECT eventid FROM events);
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE actions MODIFY actionid DEFAULT NULL;
UPDATE actions SET esc_period=3600 WHERE eventsource=0 AND esc_period=0;
ALTER TABLE alerts MODIFY alertid DEFAULT NULL;
ALTER TABLE alerts MODIFY actionid DEFAULT NULL;
ALTER TABLE alerts MODIFY eventid DEFAULT NULL;
ALTER TABLE alerts MODIFY userid DEFAULT NULL;
ALTER TABLE alerts MODIFY userid NULL;
ALTER TABLE alerts MODIFY mediatypeid DEFAULT NULL;
ALTER TABLE alerts MODIFY mediatypeid NULL;
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
ALTER TABLE applications MODIFY applicationid DEFAULT NULL;
ALTER TABLE applications MODIFY hostid DEFAULT NULL;
ALTER TABLE applications MODIFY templateid DEFAULT NULL;
ALTER TABLE applications MODIFY templateid NULL;
DELETE FROM applications WHERE NOT hostid IN (SELECT hostid FROM hosts);
UPDATE applications SET templateid=NULL WHERE templateid=0;
UPDATE applications SET templateid=NULL WHERE NOT templateid IS NULL AND NOT templateid IN (SELECT applicationid FROM applications);
ALTER TABLE applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE applications ADD CONSTRAINT c_applications_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE auditlog_details MODIFY auditdetailid DEFAULT NULL;
ALTER TABLE auditlog_details MODIFY auditid DEFAULT NULL;
DELETE FROM auditlog_details WHERE NOT auditid IN (SELECT auditid FROM auditlog);
ALTER TABLE auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE;
ALTER TABLE auditlog MODIFY auditid DEFAULT NULL;
ALTER TABLE auditlog MODIFY userid DEFAULT NULL;
DELETE FROM auditlog WHERE NOT userid IN (SELECT userid FROM users);
ALTER TABLE auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
DROP INDEX autoreg_host_1;
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);
ALTER TABLE autoreg_host MODIFY autoreg_hostid DEFAULT NULL;
ALTER TABLE autoreg_host MODIFY proxy_hostid DEFAULT NULL;
ALTER TABLE autoreg_host MODIFY proxy_hostid NULL;
ALTER TABLE autoreg_host ADD listen_ip nvarchar2(39) DEFAULT '';
ALTER TABLE autoreg_host ADD listen_port number(10) DEFAULT '0' NOT NULL;
ALTER TABLE autoreg_host ADD listen_dns nvarchar2(64) DEFAULT '';
UPDATE autoreg_host SET proxy_hostid=NULL WHERE proxy_hostid=0;
DELETE FROM autoreg_host WHERE proxy_hostid IS NOT NULL AND proxy_hostid NOT IN (SELECT hostid FROM hosts);
ALTER TABLE autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE conditions MODIFY conditionid DEFAULT NULL;
ALTER TABLE conditions MODIFY actionid DEFAULT NULL;
DELETE FROM conditions WHERE NOT actionid IN (SELECT actionid FROM actions);
ALTER TABLE conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE config MODIFY configid DEFAULT NULL;
ALTER TABLE config MODIFY alert_usrgrpid DEFAULT NULL;
ALTER TABLE config MODIFY alert_usrgrpid NULL;
ALTER TABLE config MODIFY discovery_groupid DEFAULT NULL;
ALTER TABLE config MODIFY default_theme nvarchar2(128) DEFAULT 'originalblue' NOT NULL;
ALTER TABLE config ADD severity_color_0 nvarchar2(6) DEFAULT 'DBDBDB';
ALTER TABLE config ADD severity_color_1 nvarchar2(6) DEFAULT 'D6F6FF';
ALTER TABLE config ADD severity_color_2 nvarchar2(6) DEFAULT 'FFF6A5';
ALTER TABLE config ADD severity_color_3 nvarchar2(6) DEFAULT 'FFB689';
ALTER TABLE config ADD severity_color_4 nvarchar2(6) DEFAULT 'FF9999';
ALTER TABLE config ADD severity_color_5 nvarchar2(6) DEFAULT 'FF3838';
ALTER TABLE config ADD severity_name_0 nvarchar2(32) DEFAULT 'Not classified';
ALTER TABLE config ADD severity_name_1 nvarchar2(32) DEFAULT 'Information';
ALTER TABLE config ADD severity_name_2 nvarchar2(32) DEFAULT 'Warning';
ALTER TABLE config ADD severity_name_3 nvarchar2(32) DEFAULT 'Average';
ALTER TABLE config ADD severity_name_4 nvarchar2(32) DEFAULT 'High';
ALTER TABLE config ADD severity_name_5 nvarchar2(32) DEFAULT 'Disaster';
ALTER TABLE config ADD ok_period number(10) DEFAULT '1800' NOT NULL;
ALTER TABLE config ADD blink_period number(10) DEFAULT '1800' NOT NULL;
ALTER TABLE config ADD problem_unack_color nvarchar2(6) DEFAULT 'DC0000';
ALTER TABLE config ADD problem_ack_color nvarchar2(6) DEFAULT 'DC0000';
ALTER TABLE config ADD ok_unack_color nvarchar2(6) DEFAULT '00AA00';
ALTER TABLE config ADD ok_ack_color nvarchar2(6) DEFAULT '00AA00';
ALTER TABLE config ADD problem_unack_style number(10) DEFAULT '1' NOT NULL;
ALTER TABLE config ADD problem_ack_style number(10) DEFAULT '1' NOT NULL;
ALTER TABLE config ADD ok_unack_style number(10) DEFAULT '1' NOT NULL;
ALTER TABLE config ADD ok_ack_style number(10) DEFAULT '1' NOT NULL;
ALTER TABLE config ADD snmptrap_logging number(10) DEFAULT '1' NOT NULL;
ALTER TABLE config ADD server_check_interval number(10) DEFAULT '60' NOT NULL;
UPDATE config SET alert_usrgrpid=NULL WHERE NOT alert_usrgrpid IN (SELECT usrgrpid FROM usrgrp);
UPDATE config SET discovery_groupid=(SELECT MIN(groupid) FROM groups) WHERE NOT discovery_groupid IN (SELECT groupid FROM groups);

UPDATE config SET default_theme='darkblue' WHERE default_theme='css_bb.css';
UPDATE config SET default_theme='originalblue' WHERE default_theme IN ('css_ob.css', 'default.css');
UPDATE config SET default_theme='darkorange' WHERE default_theme='css_od.css';

ALTER TABLE config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid);
-- See drules.sql
ALTER TABLE dhosts MODIFY dhostid DEFAULT NULL;
ALTER TABLE dhosts MODIFY druleid DEFAULT NULL;
DELETE FROM dhosts WHERE NOT druleid IN (SELECT druleid FROM drules);
ALTER TABLE dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
ALTER TABLE dchecks MODIFY dcheckid DEFAULT NULL;
ALTER TABLE dchecks MODIFY druleid DEFAULT NULL;
ALTER TABLE dchecks MODIFY key_ DEFAULT '';
ALTER TABLE dchecks MODIFY snmp_community DEFAULT '';
ALTER TABLE dchecks ADD uniq number(10) DEFAULT '0' NOT NULL;
DELETE FROM dchecks WHERE NOT druleid IN (SELECT druleid FROM drules);
ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
UPDATE dchecks SET uniq=1 WHERE dcheckid IN (SELECT unique_dcheckid FROM drules);
ALTER TABLE drules MODIFY druleid DEFAULT NULL;
ALTER TABLE drules MODIFY proxy_hostid DEFAULT NULL;
ALTER TABLE drules MODIFY proxy_hostid NULL;
ALTER TABLE drules MODIFY delay DEFAULT '3600';
ALTER TABLE drules DROP COLUMN unique_dcheckid;
UPDATE drules SET proxy_hostid=NULL WHERE NOT proxy_hostid IN (SELECT hostid FROM hosts);
ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE dservices MODIFY dserviceid DEFAULT NULL;
ALTER TABLE dservices MODIFY dhostid DEFAULT NULL;
ALTER TABLE dservices MODIFY dcheckid DEFAULT NULL;
ALTER TABLE dservices MODIFY key_ DEFAULT '';
ALTER TABLE dservices MODIFY value DEFAULT '';
ALTER TABLE dservices ADD dns nvarchar2(64) DEFAULT '';
DELETE FROM dservices WHERE NOT dhostid IN (SELECT dhostid FROM dhosts);
DELETE FROM dservices WHERE NOT dcheckid IN (SELECT dcheckid FROM dchecks);
ALTER TABLE dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE;
ALTER TABLE dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE;
ALTER TABLE escalations MODIFY escalationid DEFAULT NULL;
ALTER TABLE escalations MODIFY actionid DEFAULT NULL;
ALTER TABLE escalations MODIFY triggerid DEFAULT NULL;
ALTER TABLE escalations MODIFY triggerid NULL;
ALTER TABLE escalations MODIFY eventid DEFAULT NULL;
ALTER TABLE escalations MODIFY eventid NULL;
ALTER TABLE escalations MODIFY r_eventid DEFAULT NULL;
ALTER TABLE escalations MODIFY r_eventid NULL;
DROP INDEX escalations_2;

-- 0: ESCALATION_STATUS_ACTIVE
-- 1: ESCALATION_STATUS_RECOVERY
-- 2: ESCALATION_STATUS_SLEEP
-- 4: ESCALATION_STATUS_SUPERSEDED_ACTIVE
-- 5: ESCALATION_STATUS_SUPERSEDED_RECOVERY
UPDATE escalations SET status=0 WHERE status in (1,4,5);

VARIABLE escalation_maxid number;
BEGIN
SELECT MAX(escalationid) INTO :escalation_maxid FROM escalations;
END;
/

CREATE SEQUENCE escalations_seq;

INSERT INTO escalations (escalationid, actionid, triggerid, r_eventid)
	SELECT :escalation_maxid + escalations_seq.NEXTVAL, actionid, triggerid, r_eventid
		FROM escalations
		WHERE status = 0
			AND eventid IS NOT NULL
			AND r_eventid IS NOT NULL;
UPDATE escalations SET r_eventid = NULL WHERE eventid IS NOT NULL AND r_eventid IS NOT NULL;

DROP SEQUENCE escalations_seq;
-- See triggers.sql
ALTER TABLE expressions MODIFY expressionid DEFAULT NULL;
ALTER TABLE expressions MODIFY regexpid DEFAULT NULL;
DELETE FROM expressions WHERE NOT regexpid IN (SELECT regexpid FROM regexps);
ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE;
ALTER TABLE functions MODIFY functionid DEFAULT NULL;
ALTER TABLE functions MODIFY itemid DEFAULT NULL;
ALTER TABLE functions MODIFY triggerid DEFAULT NULL;
ALTER TABLE functions DROP COLUMN lastvalue;
DELETE FROM functions WHERE NOT itemid IN (SELECT itemid FROM items);
DELETE FROM functions WHERE NOT triggerid IN (SELECT triggerid FROM triggers);
ALTER TABLE functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE globalmacro MODIFY globalmacroid DEFAULT NULL;
CREATE TABLE globalvars (
	globalvarid              number(20)                                NOT NULL,
	snmp_lastsize            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
);
CREATE TABLE graph_discovery (
	graphdiscoveryid         number(20)                                NOT NULL,
	graphid                  number(20)                                NOT NULL,
	parent_graphid           number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	PRIMARY KEY (graphdiscoveryid)
);
CREATE UNIQUE INDEX graph_discovery_1 on graph_discovery (graphid,parent_graphid);
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs_items MODIFY gitemid DEFAULT NULL;
ALTER TABLE graphs_items MODIFY graphid DEFAULT NULL;
ALTER TABLE graphs_items MODIFY itemid DEFAULT NULL;
ALTER TABLE graphs_items DROP COLUMN periods_cnt;
UPDATE graphs_items SET type=0 WHERE type=1;
DELETE FROM graphs_items WHERE NOT graphid IN (SELECT graphid FROM graphs);
DELETE FROM graphs_items WHERE NOT itemid IN (SELECT itemid FROM items);
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE graphs MODIFY graphid DEFAULT NULL;
ALTER TABLE graphs MODIFY templateid DEFAULT NULL;
ALTER TABLE graphs MODIFY templateid NULL;
ALTER TABLE graphs MODIFY ymin_itemid DEFAULT NULL;
ALTER TABLE graphs MODIFY ymin_itemid NULL;
ALTER TABLE graphs MODIFY ymax_itemid DEFAULT NULL;
ALTER TABLE graphs MODIFY ymax_itemid NULL;
ALTER TABLE graphs MODIFY show_legend DEFAULT 1;
UPDATE graphs SET show_legend=1 WHERE graphtype=0 OR graphtype=1;
ALTER TABLE graphs ADD flags number(10) DEFAULT '0' NOT NULL;
UPDATE graphs SET templateid=NULL WHERE templateid=0;
UPDATE graphs SET templateid=NULL WHERE NOT templateid IS NULL AND NOT templateid IN (SELECT graphid FROM graphs);
UPDATE graphs SET ymin_itemid=NULL WHERE ymin_itemid=0 OR NOT ymin_itemid IN (SELECT itemid FROM items);
UPDATE graphs SET ymax_itemid=NULL WHERE ymax_itemid=0 OR NOT ymax_itemid IN (SELECT itemid FROM items);
UPDATE graphs SET ymin_type=0 WHERE ymin_type=2 AND ymin_itemid=NULL;
UPDATE graphs SET ymax_type=0 WHERE ymax_type=2 AND ymax_itemid=NULL;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid);
ALTER TABLE graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid);
ALTER TABLE graph_theme MODIFY graphthemeid DEFAULT NULL;
ALTER TABLE graph_theme MODIFY noneworktimecolor DEFAULT 'CCCCCC';
ALTER TABLE graph_theme RENAME COLUMN noneworktimecolor TO nonworktimecolor;

UPDATE graph_theme SET theme = 'darkblue' WHERE theme = 'css_bb.css';
UPDATE graph_theme SET theme = 'originalblue' WHERE theme = 'css_ob.css';

-- Insert new graph theme
INSERT INTO graph_theme (graphthemeid, description, theme, backgroundcolor, graphcolor, graphbordercolor, gridcolor, maingridcolor, gridbordercolor, textcolor, highlightcolor, leftpercentilecolor, rightpercentilecolor, nonworktimecolor, gridview, legendview)
VALUES ((SELECT MAX(graphthemeid) + 1 FROM graph_theme), 'Dark orange', 'darkorange', '333333', '0A0A0A', '888888', '222222', '4F4F4F', 'EFEFEF', 'DFDFDF', 'FF5500', 'FF5500', 'FF1111', '1F1F1F', 1, 1);
INSERT INTO graph_theme (graphthemeid, description, theme, backgroundcolor, graphcolor, graphbordercolor, gridcolor, maingridcolor, gridbordercolor, textcolor, highlightcolor, leftpercentilecolor, rightpercentilecolor, nonworktimecolor, gridview, legendview)
VALUES ((SELECT MAX(graphthemeid) + 1 FROM graph_theme), 'Classic', 'classic', 'F0F0F0', 'FFFFFF', '333333', 'CCCCCC', 'AAAAAA', '000000', '222222', 'AA4444', '11CC11', 'CC1111', 'E0E0E0', 1, 1);

DELETE FROM ids WHERE table_name = 'graph_theme';
ALTER TABLE groups MODIFY groupid DEFAULT NULL;
SET DEFINE OFF

drop table help_items;

CREATE TABLE help_items (
	itemtype	number(10)	DEFAULT '0'	NOT NULL,
	key_		nvarchar2(255)	DEFAULT ''	,
	description	nvarchar2(255)	DEFAULT ''	,
	PRIMARY KEY (itemtype,key_)
);

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
ALTER TABLE history_log MODIFY id DEFAULT NULL;
ALTER TABLE history_log MODIFY itemid DEFAULT NULL;
ALTER TABLE history_log ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history MODIFY itemid DEFAULT NULL;
ALTER TABLE history ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history_str MODIFY itemid DEFAULT NULL;
ALTER TABLE history_str ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history_str_sync MODIFY itemid DEFAULT NULL;
ALTER TABLE history_str_sync MODIFY nodeid DEFAULT NULL;
ALTER TABLE history_str_sync MODIFY nodeid number(10);
ALTER TABLE history_str_sync ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history_sync MODIFY itemid DEFAULT NULL;
ALTER TABLE history_sync MODIFY nodeid DEFAULT NULL;
ALTER TABLE history_sync MODIFY nodeid number(10);
ALTER TABLE history_sync ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history_text MODIFY id DEFAULT NULL;
ALTER TABLE history_text MODIFY itemid DEFAULT NULL;
ALTER TABLE history_text ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history_uint MODIFY itemid DEFAULT NULL;
ALTER TABLE history_uint ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE history_uint_sync MODIFY itemid DEFAULT NULL;
ALTER TABLE history_uint_sync MODIFY nodeid DEFAULT NULL;
ALTER TABLE history_uint_sync MODIFY nodeid number(10);
ALTER TABLE history_uint_sync ADD ns number(10) DEFAULT '0' NOT NULL;
DELETE FROM hosts_profiles WHERE NOT hostid IN (SELECT hostid FROM hosts);
DELETE FROM hosts_profiles_ext WHERE NOT hostid IN (SELECT hostid FROM hosts);

CREATE TABLE host_inventory (
	hostid                   number(20)                                NOT NULL,
	inventory_mode           number(10)      DEFAULT '0'               NOT NULL,
	type                     nvarchar2(64)   DEFAULT ''                ,
	type_full                nvarchar2(64)   DEFAULT ''                ,
	name                     nvarchar2(64)   DEFAULT ''                ,
	alias                    nvarchar2(64)   DEFAULT ''                ,
	os                       nvarchar2(64)   DEFAULT ''                ,
	os_full                  nvarchar2(255)  DEFAULT ''                ,
	os_short                 nvarchar2(64)   DEFAULT ''                ,
	serialno_a               nvarchar2(64)   DEFAULT ''                ,
	serialno_b               nvarchar2(64)   DEFAULT ''                ,
	tag                      nvarchar2(64)   DEFAULT ''                ,
	asset_tag                nvarchar2(64)   DEFAULT ''                ,
	macaddress_a             nvarchar2(64)   DEFAULT ''                ,
	macaddress_b             nvarchar2(64)   DEFAULT ''                ,
	hardware                 nvarchar2(255)  DEFAULT ''                ,
	hardware_full            nvarchar2(2048) DEFAULT ''                ,
	software                 nvarchar2(255)  DEFAULT ''                ,
	software_full            nvarchar2(2048) DEFAULT ''                ,
	software_app_a           nvarchar2(64)   DEFAULT ''                ,
	software_app_b           nvarchar2(64)   DEFAULT ''                ,
	software_app_c           nvarchar2(64)   DEFAULT ''                ,
	software_app_d           nvarchar2(64)   DEFAULT ''                ,
	software_app_e           nvarchar2(64)   DEFAULT ''                ,
	contact                  nvarchar2(2048) DEFAULT ''                ,
	location                 nvarchar2(2048) DEFAULT ''                ,
	location_lat             nvarchar2(16)   DEFAULT ''                ,
	location_lon             nvarchar2(16)   DEFAULT ''                ,
	notes                    nvarchar2(2048) DEFAULT ''                ,
	chassis                  nvarchar2(64)   DEFAULT ''                ,
	model                    nvarchar2(64)   DEFAULT ''                ,
	hw_arch                  nvarchar2(32)   DEFAULT ''                ,
	vendor                   nvarchar2(64)   DEFAULT ''                ,
	contract_number          nvarchar2(64)   DEFAULT ''                ,
	installer_name           nvarchar2(64)   DEFAULT ''                ,
	deployment_status        nvarchar2(64)   DEFAULT ''                ,
	url_a                    nvarchar2(255)  DEFAULT ''                ,
	url_b                    nvarchar2(255)  DEFAULT ''                ,
	url_c                    nvarchar2(255)  DEFAULT ''                ,
	host_networks            nvarchar2(2048) DEFAULT ''                ,
	host_netmask             nvarchar2(39)   DEFAULT ''                ,
	host_router              nvarchar2(39)   DEFAULT ''                ,
	oob_ip                   nvarchar2(39)   DEFAULT ''                ,
	oob_netmask              nvarchar2(39)   DEFAULT ''                ,
	oob_router               nvarchar2(39)   DEFAULT ''                ,
	date_hw_purchase         nvarchar2(64)   DEFAULT ''                ,
	date_hw_install          nvarchar2(64)   DEFAULT ''                ,
	date_hw_expiry           nvarchar2(64)   DEFAULT ''                ,
	date_hw_decomm           nvarchar2(64)   DEFAULT ''                ,
	site_address_a           nvarchar2(128)  DEFAULT ''                ,
	site_address_b           nvarchar2(128)  DEFAULT ''                ,
	site_address_c           nvarchar2(128)  DEFAULT ''                ,
	site_city                nvarchar2(128)  DEFAULT ''                ,
	site_state               nvarchar2(64)   DEFAULT ''                ,
	site_country             nvarchar2(64)   DEFAULT ''                ,
	site_zip                 nvarchar2(64)   DEFAULT ''                ,
	site_rack                nvarchar2(128)  DEFAULT ''                ,
	site_notes               nvarchar2(2048) DEFAULT ''                ,
	poc_1_name               nvarchar2(128)  DEFAULT ''                ,
	poc_1_email              nvarchar2(128)  DEFAULT ''                ,
	poc_1_phone_a            nvarchar2(64)   DEFAULT ''                ,
	poc_1_phone_b            nvarchar2(64)   DEFAULT ''                ,
	poc_1_cell               nvarchar2(64)   DEFAULT ''                ,
	poc_1_screen             nvarchar2(64)   DEFAULT ''                ,
	poc_1_notes              nvarchar2(2048) DEFAULT ''                ,
	poc_2_name               nvarchar2(128)  DEFAULT ''                ,
	poc_2_email              nvarchar2(128)  DEFAULT ''                ,
	poc_2_phone_a            nvarchar2(64)   DEFAULT ''                ,
	poc_2_phone_b            nvarchar2(64)   DEFAULT ''                ,
	poc_2_cell               nvarchar2(64)   DEFAULT ''                ,
	poc_2_screen             nvarchar2(64)   DEFAULT ''                ,
	poc_2_notes              nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (hostid)
);
ALTER TABLE host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;

-- create temporary t_host_inventory table
CREATE TABLE t_host_inventory (
	hostid                   number(20),
	inventory_mode           number(10),
	type                     nvarchar2(64),
	type_full                nvarchar2(64),
	name                     nvarchar2(64),
	alias                    nvarchar2(64),
	os                       nvarchar2(64),
	os_full                  nvarchar2(255),
	os_short                 nvarchar2(64),
	serialno_a               nvarchar2(64),
	serialno_b               nvarchar2(64),
	tag                      nvarchar2(64),
	asset_tag                nvarchar2(64),
	macaddress_a             nvarchar2(64),
	macaddress_b             nvarchar2(64),
	hardware                 nvarchar2(255),
	hardware_full            nvarchar2(2048),
	software                 nvarchar2(255),
	software_full            nvarchar2(2048),
	software_app_a           nvarchar2(64),
	software_app_b           nvarchar2(64),
	software_app_c           nvarchar2(64),
	software_app_d           nvarchar2(64),
	software_app_e           nvarchar2(64),
	contact                  nvarchar2(2048),
	location                 nvarchar2(2048),
	location_lat             nvarchar2(16),
	location_lon             nvarchar2(16),
	notes                    nvarchar2(2048),
	chassis                  nvarchar2(64),
	model                    nvarchar2(64),
	hw_arch                  nvarchar2(32),
	vendor                   nvarchar2(64),
	contract_number          nvarchar2(64),
	installer_name           nvarchar2(64),
	deployment_status        nvarchar2(64),
	url_a                    nvarchar2(255),
	url_b                    nvarchar2(255),
	url_c                    nvarchar2(255),
	host_networks            nvarchar2(2048),
	host_netmask             nvarchar2(39),
	host_router              nvarchar2(39),
	oob_ip                   nvarchar2(39),
	oob_netmask              nvarchar2(39),
	oob_router               nvarchar2(39),
	date_hw_purchase         nvarchar2(64),
	date_hw_install          nvarchar2(64),
	date_hw_expiry           nvarchar2(64),
	date_hw_decomm           nvarchar2(64),
	site_address_a           nvarchar2(128),
	site_address_b           nvarchar2(128),
	site_address_c           nvarchar2(128),
	site_city                nvarchar2(128),
	site_state               nvarchar2(64),
	site_country             nvarchar2(64),
	site_zip                 nvarchar2(64),
	site_rack                nvarchar2(128),
	site_notes               nvarchar2(2048),
	poc_1_name               nvarchar2(128),
	poc_1_email              nvarchar2(128),
	poc_1_phone_a            nvarchar2(64),
	poc_1_phone_b            nvarchar2(64),
	poc_1_cell               nvarchar2(64),
	poc_1_screen             nvarchar2(64),
	poc_1_notes              nvarchar2(2048),
	poc_2_name               nvarchar2(128),
	poc_2_email              nvarchar2(128),
	poc_2_phone_a            nvarchar2(64),
	poc_2_phone_b            nvarchar2(64),
	poc_2_cell               nvarchar2(64),
	poc_2_screen             nvarchar2(64),
	poc_2_notes              nvarchar2(2048),
	notes_ext                nvarchar2(2048)
);

-- select all profiles into temporary table
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

-- merge notes field
UPDATE t_host_inventory SET notes = notes||CHR(13)||CHR(10)||notes_ext WHERE notes IS NOT NULL AND notes_ext IS NOT NULL;
UPDATE t_host_inventory SET notes = notes_ext WHERE notes IS NULL;
ALTER TABLE t_host_inventory DROP COLUMN notes_ext;

-- copy data from temporary table
INSERT INTO host_inventory SELECT * FROM t_host_inventory;

DROP TABLE t_host_inventory;
DROP TABLE hosts_profiles;
DROP TABLE hosts_profiles_ext;

DELETE FROM ids WHERE table_name IN ('hosts_profiles', 'hosts_profiles_ext');
ALTER TABLE hostmacro MODIFY hostmacroid DEFAULT NULL;
ALTER TABLE hostmacro MODIFY hostid DEFAULT NULL;
DELETE FROM hostmacro WHERE NOT hostid IN (SELECT hostid FROM hosts);
-- remove duplicates to allow unique index
DELETE FROM hostmacro
	WHERE hostmacroid IN (
		SELECT hm1.hostmacroid
		FROM hostmacro hm1
		LEFT OUTER JOIN (
			SELECT MIN(hm2.hostmacroid) AS hostmacroid
			FROM hostmacro hm2
			GROUP BY hm2.hostid,hm2.macro
		) keep_rows ON
			hm1.hostmacroid=keep_rows.hostmacroid
		WHERE keep_rows.hostmacroid IS NULL
	);
DROP INDEX hostmacro_1;
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);
ALTER TABLE hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups MODIFY hostgroupid DEFAULT NULL;
ALTER TABLE hosts_groups MODIFY hostid DEFAULT NULL;
ALTER TABLE hosts_groups MODIFY groupid DEFAULT NULL;
DELETE FROM hosts_groups WHERE NOT hostid IN (SELECT hostid FROM hosts);
DELETE FROM hosts_groups WHERE NOT groupid IN (SELECT groupid FROM groups);
-- remove duplicates to allow unique index
DELETE FROM hosts_groups
	WHERE hostgroupid IN (
		SELECT hg1.hostgroupid
		FROM hosts_groups hg1
		LEFT OUTER JOIN (
			SELECT MIN(hg2.hostgroupid) AS hostgroupid
			FROM hosts_groups hg2
			GROUP BY hostid,groupid
		) keep_rows ON
			hg1.hostgroupid=keep_rows.hostgroupid
		WHERE keep_rows.hostgroupid IS NULL
	);
DROP INDEX hosts_groups_1;
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
-- See host_inventory.sql
-- See host_inventory.sql
---- Patching table `interfaces`

CREATE TABLE interface (
	interfaceid              number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	main                     number(10)      DEFAULT '0'               NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	useip                    number(10)      DEFAULT '1'               NOT NULL,
	ip                       nvarchar2(39)   DEFAULT '127.0.0.1'       ,
	dns                      nvarchar2(64)   DEFAULT ''                ,
	port                     nvarchar2(64)   DEFAULT '10050'           ,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_1 on interface (hostid,type);
CREATE INDEX interface_2 on interface (ip,dns);
ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;

-- Passive proxy interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - (trunc(hostid / 100000000000)*100000000000)) * 3 + (trunc(hostid / 100000000000)*100000000000),
		hostid,1,0,ip,dns,useip,port
	FROM hosts
	WHERE status IN (6));

-- Zabbix Agent interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - (trunc(hostid / 100000000000)*100000000000)) * 3 + (trunc(hostid / 100000000000)*100000000000),
		hostid,1,1,ip,dns,useip,port
	FROM hosts
	WHERE status IN (0,1));

-- SNMP interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - (trunc(hostid / 100000000000)*100000000000)) * 3 + (trunc(hostid / 100000000000)*100000000000) + 1,
		hostid,1,2,ip,dns,useip,'161'
	FROM hosts
	WHERE status IN (0,1)
		AND EXISTS (SELECT DISTINCT i.hostid FROM items i WHERE i.hostid=hosts.hostid and i.type IN (1,4,6)));

-- IPMI interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - (trunc(hostid / 100000000000)*100000000000)) * 3 + (trunc(hostid / 100000000000)*100000000000) + 2,
		hostid,1,3,'',ipmi_ip,0,ipmi_port
	FROM hosts
	WHERE status IN (0,1) AND useipmi=1);

---- Patching table `items`

ALTER TABLE items RENAME COLUMN description to name;
ALTER TABLE items MODIFY (
	itemid DEFAULT NULL,
	hostid DEFAULT NULL,
	units nvarchar2(255),
	templateid DEFAULT NULL NULL,
	lastlogsize number(20),
	valuemapid DEFAULT NULL NULL
);
ALTER TABLE items ADD (
	lastns number(10) NULL,
	flags number(10) DEFAULT '0' NOT NULL,
	filter nvarchar2(255) DEFAULT '',
	interfaceid number(20) NULL,
	port nvarchar2(64) DEFAULT '',
	description nvarchar2(2048) DEFAULT '',
	inventory_link number(10) DEFAULT '0' NOT NULL,
	lifetime nvarchar2(64) DEFAULT '30'
);
UPDATE items
	SET templateid=NULL
	WHERE templateid=0
		OR templateid NOT IN (SELECT itemid FROM items);
UPDATE items
	SET valuemapid=NULL
	WHERE valuemapid=0
		OR valuemapid NOT IN (SELECT valuemapid from valuemaps);
UPDATE items SET units='Bps' WHERE type=9 AND units='bps';
DELETE FROM items WHERE NOT hostid IN (SELECT hostid FROM hosts);
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
		AND type IN (0,3,10,11,13,14)	-- ZABBIX, SIMPLE, EXTERNAL, DB_MONITOR, SSH, TELNET
/

-- host interface for SNMP and non templated items
UPDATE items
	SET interfaceid=(SELECT interfaceid FROM interface WHERE hostid=items.hostid AND main=1 AND type=2)
	WHERE EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=items.hostid AND hosts.status IN (0,1))
		AND type IN (1,4,6)		-- SNMPv1, SNMPv2c, SNMPv3
/

-- host interface for IPMI and non templated items
UPDATE items
	SET interfaceid=(SELECT interfaceid FROM interface WHERE hostid=items.hostid AND main=1 AND type=3)
	WHERE EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=items.hostid AND hosts.status IN (0,1))
		AND type IN (12)		-- IPMI
/

-- clear port number for non SNMP items
UPDATE items
	SET port=''
	WHERE type NOT IN (1,4,6)		-- SNMPv1, SNMPv2c, SNMPv3
/

-- add a first parameter {HOST.CONN} for external checks

UPDATE items
	SET key_ = SUBSTR(key_, 1, INSTR(key_, '[')) || '"{HOST.CONN}",' || SUBSTR(key_, INSTR(key_, '[') + 1)
	WHERE type IN (10)	-- EXTERNAL
		AND INSTR(key_, '[') <> 0;

UPDATE items
	SET key_ = key_ || '["{HOST.CONN}"]'
	WHERE type IN (10)	-- EXTERNAL
		AND INSTR(key_, '[') = 0;

-- convert simple check keys to a new form

CREATE FUNCTION zbx_key_exists(v_hostid IN number, new_key IN nvarchar2)
	RETURN number IS key_exists number(10);
	BEGIN
		SELECT COUNT(*) INTO key_exists FROM items WHERE hostid = v_hostid AND key_ = new_key;
		RETURN key_exists;
	END;
/

DECLARE
	v_itemid number(20);
	v_hostid number(20);
	v_key nvarchar2(255);
	new_key nvarchar2(255);
	pos number(10);

	CURSOR i_cur IS
		SELECT itemid,hostid,key_
			FROM items
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
BEGIN
	OPEN i_cur;

	LOOP
		FETCH i_cur INTO v_itemid, v_hostid, v_key;

		EXIT WHEN i_cur%NOTFOUND;

		new_key := 'net.tcp.service';
		pos := INSTR(v_key, '_perf');
		IF 0 <> pos THEN
			new_key := new_key || '.perf';
			v_key := SUBSTR(v_key, 1, pos - 1) || SUBSTR(v_key, pos + 5);
		END IF;
		new_key := new_key || '[';
		pos := INSTR(v_key, ',');
		IF 0 <> pos THEN
			new_key := new_key || '"' || SUBSTR(v_key, 1, pos - 1) || '"';
			v_key := SUBSTR(v_key, pos + 1);
		ELSE
			new_key := new_key || '"' || v_key || '"';
			v_key := '';
		END IF;
		IF 0 <> LENGTH(v_key) THEN
			new_key := new_key || ',,"' || v_key || '"';
		END IF;

		WHILE 0 <> zbx_key_exists(v_hostid, new_key || ']') LOOP
			new_key := new_key || ' ';
		END LOOP;

		new_key := new_key || ']';

		UPDATE items SET key_ = new_key WHERE itemid = v_itemid;
	END LOOP;

	CLOSE i_cur;
END;
/

DROP FUNCTION zbx_key_exists;

-- adding web.test.error[<web check>] items

DECLARE
	httptest_nodeid number(10);
	min_nodeid number(20);
	max_nodeid number(20);
	init_nodeid number(20);
	CURSOR node_cursor IS SELECT DISTINCT TRUNC(httptestid / 100000000000000) FROM httptest;

	max_itemid number(20);
	max_httptestitemid number(20);
	max_itemappid number(20);
BEGIN
	OPEN node_cursor;

	LOOP
		FETCH node_cursor INTO httptest_nodeid;
		EXIT WHEN node_cursor%NOTFOUND;

		min_nodeid := httptest_nodeid * 100000000000000;
		max_nodeid := min_nodeid + 99999999999999;
		init_nodeid := (httptest_nodeid * 1000 + httptest_nodeid) * 100000000000;

		SELECT MAX(itemid) INTO max_itemid FROM items WHERE itemid BETWEEN min_nodeid AND max_nodeid;
		IF max_itemid IS NULL THEN
			max_itemid := init_nodeid;
		END IF;
		EXECUTE IMMEDIATE 'CREATE SEQUENCE items_seq MINVALUE ' || (max_itemid + 1);

		SELECT MAX(httptestitemid) INTO max_httptestitemid FROM httptestitem WHERE httptestitemid BETWEEN min_nodeid AND max_nodeid;
		IF max_httptestitemid IS NULL THEN
			max_httptestitemid := init_nodeid;
		END IF;
		EXECUTE IMMEDIATE 'CREATE SEQUENCE httptestitem_seq MINVALUE ' || (max_httptestitemid + 1);

		SELECT MAX(itemappid) INTO max_itemappid FROM items_applications WHERE itemappid BETWEEN min_nodeid AND max_nodeid;
		IF max_itemappid IS NULL THEN
			max_itemappid := init_nodeid;
		END IF;
		EXECUTE IMMEDIATE 'CREATE SEQUENCE items_applications_seq MINVALUE ' || (max_itemappid + 1);

		EXECUTE IMMEDIATE 'INSERT INTO items (itemid, hostid, type, name, key_, value_type, units, delay, history, trends, status)
			SELECT items_seq.NEXTVAL, hostid, type, ''Last error message of scenario ''''$1'''''', ''web.test.error'' || SUBSTR(key_, INSTR(key_, ''['')), 1, '''', delay, history, 0, status
			FROM items
			WHERE type = 9
				AND key_ LIKE ''web.test.fail%''
				AND itemid BETWEEN ' || min_nodeid ||' AND ' || max_nodeid;

		EXECUTE IMMEDIATE 'INSERT INTO httptestitem (httptestitemid, httptestid, itemid, type)
			SELECT httptestitem_seq.NEXTVAL, ht.httptestid, i.itemid, 4
			FROM httptest ht,applications a,items i
			WHERE ht.applicationid=a.applicationid
				AND a.hostid=i.hostid
				AND ''web.test.error['' || ht.name || '']'' = i.key_
				AND itemid BETWEEN ' || min_nodeid ||' AND ' || max_nodeid;

		EXECUTE IMMEDIATE 'INSERT INTO items_applications (itemappid, applicationid, itemid)
			SELECT items_applications_seq.NEXTVAL, ht.applicationid, hti.itemid
			FROM httptest ht, httptestitem hti
			WHERE ht.httptestid = hti.httptestid
				AND hti.type = 4
				AND itemid BETWEEN ' || min_nodeid ||' AND ' || max_nodeid;

		EXECUTE IMMEDIATE 'DROP SEQUENCE items_seq';
		EXECUTE IMMEDIATE 'DROP SEQUENCE httptestitem_seq';
		EXECUTE IMMEDIATE 'DROP SEQUENCE items_applications_seq';

	END LOOP;

	CLOSE node_cursor;
END;
/

DELETE FROM ids WHERE table_name IN ('items', 'httptestitem', 'items_applications');

---- Patching table `hosts`

ALTER TABLE hosts MODIFY hostid DEFAULT NULL;
ALTER TABLE hosts MODIFY proxy_hostid DEFAULT NULL;
ALTER TABLE hosts MODIFY proxy_hostid NULL;
ALTER TABLE hosts MODIFY maintenanceid DEFAULT NULL;
ALTER TABLE hosts MODIFY maintenanceid NULL;
ALTER TABLE hosts DROP COLUMN ip;
ALTER TABLE hosts DROP COLUMN dns;
ALTER TABLE hosts DROP COLUMN port;
ALTER TABLE hosts DROP COLUMN useip;
ALTER TABLE hosts DROP COLUMN useipmi;
ALTER TABLE hosts DROP COLUMN ipmi_ip;
ALTER TABLE hosts DROP COLUMN ipmi_port;
ALTER TABLE hosts DROP COLUMN inbytes;
ALTER TABLE hosts DROP COLUMN outbytes;
ALTER TABLE hosts ADD jmx_disable_until number(10) DEFAULT '0' NOT NULL;
ALTER TABLE hosts ADD jmx_available number(10) DEFAULT '0' NOT NULL;
ALTER TABLE hosts ADD jmx_errors_from number(10) DEFAULT '0' NOT NULL;
ALTER TABLE hosts ADD jmx_error nvarchar2(128) DEFAULT '';
ALTER TABLE hosts ADD name nvarchar2(64) DEFAULT '';
UPDATE hosts
	SET proxy_hostid=NULL
	WHERE proxy_hostid=0
		OR NOT EXISTS (SELECT 1 FROM hosts h WHERE h.hostid=hosts.proxy_hostid);
UPDATE hosts
	SET maintenanceid=NULL,
		maintenance_status=0,
		maintenance_type=0,
		maintenance_from=0
	WHERE maintenanceid=0
		OR NOT EXISTS (SELECT 1 FROM maintenances m WHERE m.maintenanceid=hosts.maintenanceid);
UPDATE hosts SET name=host WHERE status in (0,1,3)	-- MONITORED, NOT_MONITORED, TEMPLATE
/
CREATE INDEX hosts_4 on hosts (name);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid);
DELETE FROM hosts_templates WHERE hostid NOT IN (SELECT hostid FROM hosts);
DELETE FROM hosts_templates WHERE templateid NOT IN (SELECT hostid FROM hosts);

CREATE TABLE t_hosts_templates (
	hosttemplateid           number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL
);

INSERT INTO t_hosts_templates (SELECT hosttemplateid, hostid, templateid FROM hosts_templates);

DROP TABLE hosts_templates;

CREATE TABLE hosts_templates (
	hosttemplateid           number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (hosttemplateid)
);
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;

INSERT INTO hosts_templates (SELECT hosttemplateid, hostid, templateid FROM t_hosts_templates);

DROP TABLE t_hosts_templates;
ALTER TABLE housekeeper MODIFY housekeeperid DEFAULT NULL;
ALTER TABLE housekeeper MODIFY value DEFAULT NULL;
ALTER TABLE httpstepitem MODIFY httpstepitemid DEFAULT NULL;
ALTER TABLE httpstepitem MODIFY httpstepid DEFAULT NULL;
ALTER TABLE httpstepitem MODIFY itemid DEFAULT NULL;
DELETE FROM httpstepitem WHERE NOT httpstepid IN (SELECT httpstepid FROM httpstep);
DELETE FROM httpstepitem WHERE NOT itemid IN (SELECT itemid FROM items);
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE;
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE httpstep MODIFY httpstepid DEFAULT NULL;
ALTER TABLE httpstep MODIFY httptestid DEFAULT NULL;
DELETE FROM httpstep WHERE NOT httptestid IN (SELECT httptestid FROM httptest);
ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httptestitem MODIFY httptestitemid DEFAULT NULL;
ALTER TABLE httptestitem MODIFY httptestid DEFAULT NULL;
ALTER TABLE httptestitem MODIFY itemid DEFAULT NULL;
DELETE FROM httptestitem WHERE NOT httptestid IN (SELECT httptestid FROM httptest);
DELETE FROM httptestitem WHERE NOT itemid IN (SELECT itemid FROM items);
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE httptest MODIFY httptestid DEFAULT NULL;
ALTER TABLE httptest MODIFY applicationid DEFAULT NULL;
ALTER TABLE httptest DROP COLUMN lastcheck;
ALTER TABLE httptest DROP COLUMN curstate;
ALTER TABLE httptest DROP COLUMN curstep;
ALTER TABLE httptest DROP COLUMN lastfailedstep;
ALTER TABLE httptest DROP COLUMN time;
ALTER TABLE httptest DROP COLUMN error;
DELETE FROM httptest WHERE applicationid NOT IN (SELECT applicationid FROM applications);
ALTER TABLE httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
-- See icon_map.sql
CREATE TABLE icon_map (
	iconmapid                number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	default_iconid           number(20)                                NOT NULL,
	PRIMARY KEY (iconmapid)
);
CREATE INDEX icon_map_1 ON icon_map (name);
ALTER TABLE icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid);

CREATE TABLE icon_mapping (
	iconmappingid            number(20)                                NOT NULL,
	iconmapid                number(20)                                NOT NULL,
	iconid                   number(20)                                NOT NULL,
	inventory_link           number(10)      DEFAULT '0'               NOT NULL,
	expression               nvarchar2(64)   DEFAULT ''                ,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (iconmappingid)
);
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);
ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE;
ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid);
ALTER TABLE ids MODIFY nodeid DEFAULT NULL;
ALTER TABLE ids MODIFY nextid DEFAULT NULL;
ALTER TABLE images MODIFY imageid DEFAULT NULL;
-- See hosts.sql
CREATE TABLE item_discovery (
	itemdiscoveryid          number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	parent_itemid            number(20)                                NOT NULL,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
);
CREATE UNIQUE INDEX item_discovery_1 on item_discovery (itemid,parent_itemid);
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE items_applications MODIFY itemappid DEFAULT NULL;
ALTER TABLE items_applications MODIFY applicationid DEFAULT NULL;
ALTER TABLE items_applications MODIFY itemid DEFAULT NULL;
DROP INDEX items_applications_1;
DELETE FROM items_applications WHERE applicationid NOT IN (SELECT applicationid FROM applications);
DELETE FROM items_applications WHERE itemid NOT IN (SELECT itemid FROM items);
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);
ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
-- See hosts.sql
ALTER TABLE maintenances_groups MODIFY maintenance_groupid DEFAULT NULL;
ALTER TABLE maintenances_groups MODIFY maintenanceid DEFAULT NULL;
ALTER TABLE maintenances_groups MODIFY groupid DEFAULT NULL;
DROP INDEX maintenances_groups_1;
DELETE FROM maintenances_groups WHERE maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
DELETE FROM maintenances_groups WHERE groupid NOT IN (SELECT groupid FROM groups);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts MODIFY maintenance_hostid DEFAULT NULL;
ALTER TABLE maintenances_hosts MODIFY maintenanceid DEFAULT NULL;
ALTER TABLE maintenances_hosts MODIFY hostid DEFAULT NULL;
DROP INDEX maintenances_hosts_1;
DELETE FROM maintenances_hosts WHERE maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
DELETE FROM maintenances_hosts WHERE hostid NOT IN (SELECT hostid FROM hosts);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE maintenances MODIFY maintenanceid DEFAULT NULL;
ALTER TABLE maintenances_windows MODIFY maintenance_timeperiodid DEFAULT NULL;
ALTER TABLE maintenances_windows MODIFY maintenanceid DEFAULT NULL;
ALTER TABLE maintenances_windows MODIFY timeperiodid DEFAULT NULL;
DROP INDEX maintenances_windows_1;
DELETE FROM maintenances_windows WHERE maintenanceid NOT IN (SELECT maintenanceid FROM maintenances);
DELETE FROM maintenances_windows WHERE timeperiodid NOT IN (SELECT timeperiodid FROM timeperiods);
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE;
ALTER TABLE mappings MODIFY mappingid DEFAULT NULL;
ALTER TABLE mappings MODIFY valuemapid DEFAULT NULL;
DELETE FROM mappings WHERE NOT valuemapid IN (SELECT valuemapid FROM valuemaps);
ALTER TABLE mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE;
ALTER TABLE media MODIFY mediaid DEFAULT NULL;
ALTER TABLE media MODIFY userid DEFAULT NULL;
ALTER TABLE media MODIFY mediatypeid DEFAULT NULL;
ALTER TABLE media MODIFY period DEFAULT '1-7,00:00-24:00';
DELETE FROM media WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM media WHERE NOT mediatypeid IN (SELECT mediatypeid FROM media_type);
ALTER TABLE media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE media_type MODIFY mediatypeid DEFAULT NULL;
ALTER TABLE media_type ADD status number(10) DEFAULT '0' NOT NULL;
DECLARE index_not_exists EXCEPTION;
PRAGMA EXCEPTION_INIT(index_not_exists, -1418);
BEGIN
	EXECUTE IMMEDIATE 'DROP INDEX NODE_CKSUM_1';
EXCEPTION
	WHEN index_not_exists THEN NULL;
END;
/
DECLARE index_not_exists EXCEPTION;
PRAGMA EXCEPTION_INIT(index_not_exists, -1418);
BEGIN
	EXECUTE IMMEDIATE 'DROP INDEX NODE_CKSUM_CKSUM_1';
EXCEPTION
	WHEN index_not_exists THEN NULL;
END;
/
ALTER TABLE node_cksum MODIFY nodeid DEFAULT NULL;
ALTER TABLE node_cksum MODIFY recordid DEFAULT NULL;
DELETE FROM node_cksum WHERE NOT nodeid IN (SELECT nodeid FROM nodes);
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);
ALTER TABLE node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE;
ALTER TABLE nodes MODIFY nodeid DEFAULT NULL;
ALTER TABLE nodes MODIFY masterid DEFAULT NULL;
ALTER TABLE nodes MODIFY masterid NULL;
ALTER TABLE nodes DROP COLUMN timezone;
ALTER TABLE nodes DROP COLUMN slave_history;
ALTER TABLE nodes DROP COLUMN slave_trends;
UPDATE nodes SET masterid=NULL WHERE masterid=0;
ALTER TABLE nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid);
-- See operations.sql
-- See operations.sql
-- See operations.sql
CREATE TABLE t_operations (
	operationid		number(20),
	actionid		number(20),
	operationtype		number(10),
	object			number(10),
	objectid		number(20),
	shortdata		nvarchar2(255),
	longdata		nvarchar2(2048),
	esc_period		number(10),
	esc_step_from		number(10),
	esc_step_to		number(10),
	default_msg		number(10),
	evaltype		number(10),
	mediatypeid		number(20)
);

CREATE TABLE t_opconditions (
	operationid		number(20),
	conditiontype		number(10),
	operator		number(10),
	value			nvarchar2(255)
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
	operationid              number(20)                                NOT NULL,
	actionid                 number(20)                                NOT NULL,
	operationtype            number(10)      DEFAULT '0'               NOT NULL,
	esc_period               number(10)      DEFAULT '0'               NOT NULL,
	esc_step_from            number(10)      DEFAULT '1'               NOT NULL,
	esc_step_to              number(10)      DEFAULT '1'               NOT NULL,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE INDEX operations_1 ON operations (actionid);
ALTER TABLE operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;

CREATE TABLE opmessage (
	operationid              number(20)                                NOT NULL,
	default_msg              number(10)      DEFAULT '0'               NOT NULL,
	subject                  nvarchar2(255)  DEFAULT ''                ,
	message                  nvarchar2(2048) DEFAULT ''                ,
	mediatypeid              number(20)                                NULL,
	PRIMARY KEY (operationid)
);
ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid);

CREATE TABLE opmessage_grp (
	opmessage_grpid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	PRIMARY KEY (opmessage_grpid)
);
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);

CREATE TABLE opmessage_usr (
	opmessage_usrid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	PRIMARY KEY (opmessage_usrid)
);
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid);

CREATE TABLE opcommand (
	operationid              number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	scriptid                 number(20)                                NULL,
	execute_on               number(10)      DEFAULT '0'               NOT NULL,
	port                     nvarchar2(64)   DEFAULT ''                ,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	publickey                nvarchar2(64)   DEFAULT ''                ,
	privatekey               nvarchar2(64)   DEFAULT ''                ,
	command                  nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (operationid)
);
ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid);

CREATE TABLE opcommand_hst (
	opcommand_hstid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	hostid                   number(20)                                NULL,
	PRIMARY KEY (opcommand_hstid)
);
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid);

CREATE TABLE opcommand_grp (
	opcommand_grpid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (opcommand_grpid)
);
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);

CREATE TABLE opgroup (
	opgroupid                number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (opgroupid)
);
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);

CREATE TABLE optemplate (
	optemplateid             number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (optemplateid)
);
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid);

CREATE TABLE opconditions (
	opconditionid            number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	conditiontype            number(10)      DEFAULT '0'               NOT NULL,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (opconditionid)
);
CREATE INDEX opconditions_1 ON opconditions (operationid);
ALTER TABLE opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;

CREATE SEQUENCE opconditions_seq;

DECLARE
	v_nodeid number(10);
	minid number(20);
	maxid number(20);
	new_operationid number(20);
	new_opmessage_grpid number(20);
	new_opmessage_usrid number(20);
	new_opgroupid number(20);
	new_optemplateid number(20);
	new_opcommand_hstid number(20);
	new_opcommand_grpid number(20);

	CURSOR n_cur IS SELECT DISTINCT TRUNC(operationid / 100000000000000) FROM t_operations;
BEGIN
	OPEN n_cur;

	LOOP
		FETCH n_cur INTO v_nodeid;

		EXIT WHEN n_cur%NOTFOUND;

		minid := v_nodeid * 100000000000000;
		maxid := minid + 99999999999999;
		new_operationid := minid;
		new_opmessage_grpid := minid;
		new_opmessage_usrid := minid;
		new_opgroupid := minid;
		new_optemplateid := minid;
		new_opcommand_hstid := minid;
		new_opcommand_grpid := minid;

		DECLARE
			v_operationid number(20);
			v_actionid number(20);
			v_operationtype number(10);
			v_esc_period number(10);
			v_esc_step_from number(10);
			v_esc_step_to number(10);
			v_evaltype number(10);
			v_default_msg number(10);
			v_shortdata nvarchar2(255);
			v_longdata nvarchar2(2048);
			v_mediatypeid number(20);
			v_object number(10);
			v_objectid number(20);
			l_pos number(10);
			r_pos number(10);
			h_pos number(10);
			g_pos number(10);
			cur_string nvarchar2(2048);
			v_host nvarchar2(64);
			v_group nvarchar2(64);
			v_hostid number(20);
			v_groupid number(20);
			CURSOR o_cur IS
				SELECT operationid, actionid, operationtype, esc_period, esc_step_from, esc_step_to,
						evaltype, default_msg, shortdata, longdata, mediatypeid, object, objectid
					FROM t_operations
					WHERE operationid BETWEEN minid AND maxid;
		BEGIN
			OPEN o_cur;

			LOOP
				FETCH o_cur INTO v_operationid, v_actionid, v_operationtype, v_esc_period, v_esc_step_from,
						v_esc_step_to, v_evaltype, v_default_msg, v_shortdata, v_longdata,
						v_mediatypeid, v_object, v_objectid;

				EXIT WHEN o_cur%NOTFOUND;

				IF v_operationtype IN (0) THEN			-- OPERATION_TYPE_MESSAGE
					new_operationid := new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype, esc_period,
							esc_step_from, esc_step_to, evaltype)
						VALUES (new_operationid, v_actionid, v_operationtype, v_esc_period,
							v_esc_step_from, v_esc_step_to, v_evaltype);

					INSERT INTO opmessage (operationid, default_msg, subject, message, mediatypeid)
						VALUES (new_operationid, v_default_msg, v_shortdata, v_longdata, v_mediatypeid);

					IF v_object = 0 AND v_objectid IS NOT NULL THEN	-- OPERATION_OBJECT_USER
						new_opmessage_usrid := new_opmessage_usrid + 1;

						INSERT INTO opmessage_usr (opmessage_usrid, operationid, userid)
							VALUES (new_opmessage_usrid, new_operationid, v_objectid);
					END IF;

					IF v_object = 1 AND v_objectid IS NOT NULL THEN	-- OPERATION_OBJECT_GROUP
						new_opmessage_grpid := new_opmessage_grpid + 1;

						INSERT INTO opmessage_grp (opmessage_grpid, operationid, usrgrpid)
							VALUES (new_opmessage_grpid, new_operationid, v_objectid);
					END IF;

					INSERT INTO opconditions
						SELECT minid + opconditions_seq.NEXTVAL, new_operationid, conditiontype,
								operator, value
							FROM t_opconditions
							WHERE operationid = v_operationid;
				ELSIF v_operationtype IN (1) THEN		-- OPERATION_TYPE_COMMAND
					r_pos := 1;
					l_pos := 1;

					WHILE r_pos > 0 LOOP
						r_pos := INSTR(v_longdata, CHR(10), l_pos);

						IF r_pos = 0 THEN
							cur_string := SUBSTR(v_longdata, l_pos);
						ELSE
							cur_string := SUBSTR(v_longdata, l_pos, r_pos - l_pos);
						END IF;

						cur_string := TRIM(RTRIM(cur_string, CHR(13)));

						IF LENGTH(cur_string) <> 0 THEN
							h_pos := INSTR(cur_string, ':');
							g_pos := INSTR(cur_string, '#');

							IF h_pos <> 0 OR g_pos <> 0 THEN
								new_operationid := new_operationid + 1;

								INSERT INTO operations (operationid, actionid, operationtype,
										esc_period, esc_step_from, esc_step_to, evaltype)
								VALUES (new_operationid, v_actionid, v_operationtype, v_esc_period,
										v_esc_step_from, v_esc_step_to, v_evaltype);

								INSERT INTO opconditions
									SELECT minid + opconditions_seq.NEXTVAL,
											new_operationid, conditiontype,
											operator, value
										FROM t_opconditions
										WHERE operationid = v_operationid;

								IF h_pos <> 0 AND (g_pos = 0 OR h_pos < g_pos) THEN
									INSERT INTO opcommand (operationid, command)
										VALUES (new_operationid, TRIM(SUBSTR(cur_string, h_pos + 1)));

									v_host := TRIM(SUBSTR(cur_string, 1, h_pos - 1));

									IF v_host = '{HOSTNAME}' THEN
										new_opcommand_hstid := new_opcommand_hstid + 1;

										INSERT INTO opcommand_hst
											VALUES (new_opcommand_hstid, new_operationid, NULL);
									ELSE
										SELECT MIN(hostid) INTO v_hostid
											FROM hosts
											WHERE host = v_host
												AND TRUNC(hostid / 100000000000000) = v_nodeid;

										IF v_hostid IS NOT NULL THEN
											new_opcommand_hstid := new_opcommand_hstid + 1;

											INSERT INTO opcommand_hst
												VALUES (new_opcommand_hstid, new_operationid, v_hostid);
										END IF;
									END IF;
								END IF;

								IF g_pos <> 0 AND (h_pos = 0 OR g_pos < h_pos) THEN
									INSERT INTO opcommand (operationid, command)
										VALUES (new_operationid, TRIM(SUBSTR(cur_string, g_pos + 1)));

									v_group := TRIM(SUBSTR(cur_string, 1, g_pos - 1));

									SELECT MIN(groupid) INTO v_groupid
										FROM groups
										WHERE name = v_group
											AND TRUNC(groupid / 100000000000000) = v_nodeid;

									IF v_groupid IS NOT NULL THEN
										new_opcommand_grpid := new_opcommand_grpid + 1;

										INSERT INTO opcommand_grp
											VALUES (new_opcommand_grpid, new_operationid, v_groupid);
									END IF;
								END IF;
							END IF;
						END IF;

						l_pos := r_pos + 1;
					END LOOP;
				ELSIF v_operationtype IN (2, 3, 8, 9) THEN	-- OPERATION_TYPE_HOST_(ADD, REMOVE, ENABLE, DISABLE)
					new_operationid := new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype)
						VALUES (new_operationid, v_actionid, v_operationtype);
				ELSIF v_operationtype IN (4, 5) THEN		-- OPERATION_TYPE_GROUP_(ADD, REMOVE)
					new_operationid := new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype)
						VALUES (new_operationid, v_actionid, v_operationtype);

					new_opgroupid := new_opgroupid + 1;

					INSERT INTO opgroup (opgroupid, operationid, groupid)
						VALUES (new_opgroupid, new_operationid, v_objectid);
				ELSIF v_operationtype IN (6, 7) THEN		-- OPERATION_TYPE_TEMPLATE_(ADD, REMOVE)
					new_operationid := new_operationid + 1;

					INSERT INTO operations (operationid, actionid, operationtype)
						VALUES (new_operationid, v_actionid, v_operationtype);

					new_optemplateid := new_optemplateid + 1;

					INSERT INTO optemplate (optemplateid, operationid, templateid)
						VALUES (new_optemplateid, new_operationid, v_objectid);
				END IF;
			END LOOP;

			CLOSE o_cur;
		END;
	END LOOP;

	CLOSE n_cur;
END;
/

DROP SEQUENCE opconditions_seq;

DROP TABLE t_operations;
DROP TABLE t_opconditions;

UPDATE opcommand
	SET type = 1, command = TRIM(SUBSTR(CAST(command AS nvarchar2(2048)), 5))
	WHERE SUBSTR(CAST(command AS nvarchar2(2048)), 1, 4) = 'IPMI';

DELETE FROM ids WHERE table_name IN ('operations', 'opconditions', 'opmediatypes');
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
ALTER TABLE profiles MODIFY profileid DEFAULT NULL;
ALTER TABLE profiles MODIFY userid DEFAULT NULL;
DELETE FROM profiles WHERE NOT userid IN (SELECT userid FROM users);
DELETE FROM profiles WHERE idx LIKE 'web.%.sort' OR idx LIKE 'web.%.sortorder';
ALTER TABLE profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;

UPDATE profiles SET idx = 'web.screens.period' WHERE idx = 'web.charts.period';
UPDATE profiles SET idx = 'web.screens.stime' WHERE idx = 'web.charts.stime';
UPDATE profiles SET idx = 'web.screens.timelinefixed' WHERE idx = 'web.charts.timelinefixed';
ALTER TABLE proxy_autoreg_host ADD listen_ip nvarchar2(39) DEFAULT '';
ALTER TABLE proxy_autoreg_host ADD listen_port number(10) DEFAULT '0' NOT NULL;
ALTER TABLE proxy_autoreg_host ADD listen_dns nvarchar2(64) DEFAULT '';
DELETE FROM proxy_dhistory WHERE druleid NOT IN (SELECT druleid FROM drules);
DELETE FROM proxy_dhistory WHERE dcheckid<>0 AND dcheckid NOT IN (SELECT dcheckid FROM dchecks);
ALTER TABLE proxy_dhistory MODIFY druleid DEFAULT NULL;
ALTER TABLE proxy_dhistory MODIFY dcheckid NULL;
ALTER TABLE proxy_dhistory MODIFY dcheckid DEFAULT NULL;
ALTER TABLE proxy_dhistory ADD dns nvarchar2(64) DEFAULT '';
UPDATE proxy_dhistory SET dcheckid=NULL WHERE dcheckid=0;
ALTER TABLE proxy_history MODIFY itemid DEFAULT NULL;
ALTER TABLE proxy_history ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE proxy_history ADD status number(10) DEFAULT '0' NOT NULL;
ALTER TABLE regexps MODIFY regexpid DEFAULT NULL;
ALTER TABLE rights MODIFY rightid DEFAULT NULL;
ALTER TABLE rights MODIFY groupid DEFAULT NULL;
ALTER TABLE rights MODIFY id NOT NULL;
DELETE FROM rights WHERE NOT groupid IN (SELECT usrgrpid FROM usrgrp);
DELETE FROM rights WHERE NOT id IN (SELECT groupid FROM groups);
ALTER TABLE rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE screens_items MODIFY screenitemid DEFAULT NULL;
ALTER TABLE screens_items MODIFY screenid DEFAULT NULL;
ALTER TABLE screens_items ADD sort_triggers number(10) DEFAULT '0' NOT NULL;
DELETE FROM screens_items WHERE screenid NOT IN (SELECT screenid FROM screens);
ALTER TABLE screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE screens MODIFY screenid DEFAULT NULL;
ALTER TABLE screens MODIFY name DEFAULT NULL;
ALTER TABLE screens ADD templateid number(20) NULL;
ALTER TABLE screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE scripts MODIFY scriptid DEFAULT NULL;
ALTER TABLE scripts MODIFY usrgrpid DEFAULT NULL;
ALTER TABLE scripts MODIFY usrgrpid NULL;
ALTER TABLE scripts MODIFY groupid DEFAULT NULL;
ALTER TABLE scripts MODIFY groupid NULL;
ALTER TABLE scripts ADD description nvarchar2(2048) DEFAULT '';
ALTER TABLE scripts ADD confirmation nvarchar2(255) DEFAULT '';
ALTER TABLE scripts ADD type number(10) DEFAULT '0' NOT NULL;
ALTER TABLE scripts ADD execute_on number(10) DEFAULT '1' NOT NULL;
UPDATE scripts SET usrgrpid=NULL WHERE usrgrpid=0;
UPDATE scripts SET groupid=NULL WHERE groupid=0;
UPDATE scripts SET type=1,command=TRIM(SUBSTR(command, 5)) WHERE SUBSTR(command, 1, 4)='IPMI';
DELETE FROM scripts WHERE usrgrpid IS NOT NULL AND usrgrpid NOT IN (SELECT usrgrpid FROM usrgrp);
DELETE FROM scripts WHERE groupid IS NOT NULL AND groupid NOT IN (SELECT groupid FROM groups);
ALTER TABLE scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE service_alarms MODIFY servicealarmid DEFAULT NULL;
ALTER TABLE service_alarms MODIFY serviceid DEFAULT NULL;
DELETE FROM service_alarms WHERE NOT serviceid IN (SELECT serviceid FROM services);
ALTER TABLE service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_links MODIFY linkid DEFAULT NULL;
ALTER TABLE services_links MODIFY serviceupid DEFAULT NULL;
ALTER TABLE services_links MODIFY servicedownid DEFAULT NULL;
DELETE FROM services_links WHERE NOT serviceupid IN (SELECT serviceid FROM services);
DELETE FROM services_links WHERE NOT servicedownid IN (SELECT serviceid FROM services);
ALTER TABLE services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE;
UPDATE services SET triggerid = NULL WHERE NOT EXISTS (SELECT 1 FROM triggers t WHERE t.triggerid = services.triggerid);
ALTER TABLE services MODIFY serviceid DEFAULT NULL;
ALTER TABLE services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE services_times MODIFY timeid DEFAULT NULL;
ALTER TABLE services_times MODIFY serviceid DEFAULT NULL;
DELETE FROM services_times WHERE NOT serviceid IN (SELECT serviceid FROM services);
ALTER TABLE services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE sessions MODIFY userid DEFAULT NULL;
DELETE FROM sessions WHERE NOT userid IN (SELECT userid FROM users);
ALTER TABLE sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE slideshows MODIFY slideshowid DEFAULT NULL;
ALTER TABLE slides MODIFY slideid DEFAULT NULL;
ALTER TABLE slides MODIFY slideshowid DEFAULT NULL;
ALTER TABLE slides MODIFY screenid DEFAULT NULL;
DELETE FROM slides WHERE NOT slideshowid IN (SELECT slideshowid FROM slideshows);
DELETE FROM slides WHERE NOT screenid IN (SELECT screenid FROM screens);
ALTER TABLE slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
-- See sysmaps_elements.sql
CREATE TABLE sysmap_element_url (
	sysmapelementurlid       number(20)                                NOT NULL,
	selementid               number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (sysmapelementurlid)
);
CREATE UNIQUE INDEX sysmap_element_url_1 on sysmap_element_url (selementid,name);
ALTER TABLE sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;

INSERT INTO sysmap_element_url (sysmapelementurlid,selementid,name,url)
	SELECT selementid,selementid,url,url FROM sysmaps_elements WHERE url IS NOT NULL;

ALTER TABLE sysmaps_elements MODIFY selementid DEFAULT NULL;
ALTER TABLE sysmaps_elements MODIFY sysmapid DEFAULT NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_off DEFAULT NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_off NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_on DEFAULT NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_on NULL;
ALTER TABLE sysmaps_elements DROP COLUMN iconid_unknown;
ALTER TABLE sysmaps_elements MODIFY iconid_disabled DEFAULT NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_disabled NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_maintenance DEFAULT NULL;
ALTER TABLE sysmaps_elements MODIFY iconid_maintenance NULL;
ALTER TABLE sysmaps_elements DROP COLUMN url;
ALTER TABLE sysmaps_elements ADD elementsubtype number(10) DEFAULT '0' NOT NULL;
ALTER TABLE sysmaps_elements ADD areatype number(10) DEFAULT '0' NOT NULL;
ALTER TABLE sysmaps_elements ADD width number(10) DEFAULT '200' NOT NULL;
ALTER TABLE sysmaps_elements ADD height number(10) DEFAULT '200' NOT NULL;
ALTER TABLE sysmaps_elements ADD viewtype number(10) DEFAULT '0' NOT NULL;
ALTER TABLE sysmaps_elements ADD use_iconmap number(10) DEFAULT '1' NOT NULL;

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
ALTER TABLE sysmaps_links MODIFY linkid DEFAULT NULL;
ALTER TABLE sysmaps_links MODIFY sysmapid DEFAULT NULL;
ALTER TABLE sysmaps_links MODIFY selementid1 DEFAULT NULL;
ALTER TABLE sysmaps_links MODIFY selementid2 DEFAULT NULL;
DELETE FROM sysmaps_links WHERE sysmapid NOT IN (SELECT sysmapid FROM sysmaps);
DELETE FROM sysmaps_links WHERE selementid1 NOT IN (SELECT selementid FROM sysmaps_elements);
DELETE FROM sysmaps_links WHERE selementid2 NOT IN (SELECT selementid FROM sysmaps_elements);
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmaps_link_triggers MODIFY linktriggerid DEFAULT NULL;
ALTER TABLE sysmaps_link_triggers MODIFY linkid DEFAULT NULL;
ALTER TABLE sysmaps_link_triggers MODIFY triggerid DEFAULT NULL;
DELETE FROM sysmaps_link_triggers WHERE linkid NOT IN (SELECT linkid FROM sysmaps_links);
DELETE FROM sysmaps_link_triggers WHERE triggerid NOT IN (SELECT triggerid FROM triggers);
ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE;
ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE sysmaps MODIFY sysmapid DEFAULT NULL;
ALTER TABLE sysmaps MODIFY width DEFAULT '600';
ALTER TABLE sysmaps MODIFY height DEFAULT '400';
ALTER TABLE sysmaps MODIFY backgroundid DEFAULT NULL;
ALTER TABLE sysmaps MODIFY backgroundid NULL;
ALTER TABLE sysmaps MODIFY label_type DEFAULT '2';
ALTER TABLE sysmaps MODIFY label_location DEFAULT '3';
ALTER TABLE sysmaps ADD expandproblem number(10) DEFAULT '1' NOT NULL;
ALTER TABLE sysmaps ADD markelements number(10) DEFAULT '0' NOT NULL;
ALTER TABLE sysmaps ADD show_unack number(10) DEFAULT '0' NOT NULL;
ALTER TABLE sysmaps ADD grid_size number(10) DEFAULT '50' NOT NULL;
ALTER TABLE sysmaps ADD grid_show number(10) DEFAULT '1' NOT NULL;
ALTER TABLE sysmaps ADD grid_align number(10) DEFAULT '1' NOT NULL;
ALTER TABLE sysmaps ADD label_format number(10) DEFAULT '0' NOT NULL;
ALTER TABLE sysmaps ADD label_type_host number(10) DEFAULT '2' NOT NULL;
ALTER TABLE sysmaps ADD label_type_hostgroup number(10) DEFAULT '2' NOT NULL;
ALTER TABLE sysmaps ADD label_type_trigger number(10) DEFAULT '2' NOT NULL;
ALTER TABLE sysmaps ADD label_type_map number(10) DEFAULT '2' NOT NULL;
ALTER TABLE sysmaps ADD label_type_image number(10) DEFAULT '2' NOT NULL;
ALTER TABLE sysmaps ADD label_string_host nvarchar2(255) DEFAULT '';
ALTER TABLE sysmaps ADD label_string_hostgroup nvarchar2(255) DEFAULT '';
ALTER TABLE sysmaps ADD label_string_trigger nvarchar2(255) DEFAULT '';
ALTER TABLE sysmaps ADD label_string_map nvarchar2(255) DEFAULT '';
ALTER TABLE sysmaps ADD label_string_image nvarchar2(255) DEFAULT '';
ALTER TABLE sysmaps ADD iconmapid number(20) NULL;
ALTER TABLE sysmaps ADD expand_macros number(10) DEFAULT '0' NOT NULL;
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
	sysmapurlid              number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	elementtype              number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapurlid)
);
CREATE UNIQUE INDEX sysmap_url_1 on sysmap_url (sysmapid,name);
ALTER TABLE sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE timeperiods MODIFY timeperiodid DEFAULT NULL;
ALTER TABLE trends MODIFY itemid DEFAULT NULL;
ALTER TABLE trends_uint MODIFY itemid DEFAULT NULL;
ALTER TABLE trigger_depends MODIFY triggerdepid DEFAULT NULL;
ALTER TABLE trigger_depends MODIFY triggerid_down DEFAULT NULL;
ALTER TABLE trigger_depends MODIFY triggerid_up DEFAULT NULL;
DROP INDEX trigger_depends_1;
DELETE FROM trigger_depends WHERE triggerid_down NOT IN (SELECT triggerid FROM triggers);
DELETE FROM trigger_depends WHERE triggerid_up NOT IN (SELECT triggerid FROM triggers);
-- remove duplicates to allow unique index
DELETE FROM trigger_depends
	WHERE triggerdepid IN (
		SELECT td1.triggerdepid
		FROM trigger_depends td1
		LEFT OUTER JOIN (
			SELECT MIN(td2.triggerdepid) AS triggerdepid
			FROM trigger_depends td2
			GROUP BY td2.triggerid_down,td2.triggerid_up
		) keep_rows ON
			td1.triggerdepid=keep_rows.triggerdepid
		WHERE keep_rows.triggerdepid IS NULL
	);
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE;
CREATE TABLE trigger_discovery (
	triggerdiscoveryid       number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	parent_triggerid         number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (triggerdiscoveryid)
);
CREATE UNIQUE INDEX trigger_discovery_1 on trigger_discovery (triggerid,parent_triggerid);
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
----
---- Patching table `events`
----

DROP INDEX events_2;
CREATE INDEX events_2 on events (clock);
ALTER TABLE events MODIFY eventid DEFAULT NULL;
ALTER TABLE events ADD ns number(10) DEFAULT '0' NOT NULL;
ALTER TABLE events ADD value_changed number(10) DEFAULT '0' NOT NULL;

-- Begin event redesign patch

CREATE TABLE tmp_events_eventid (eventid number(20) PRIMARY KEY,prev_value number(10),value number(10));
CREATE INDEX tmp_events_index on events (source, object, objectid, clock, eventid, value);

CREATE OR REPLACE FUNCTION get_prev_value(eventid IN NUMBER, triggerid IN NUMBER, clock IN NUMBER)
RETURN NUMBER IS
prev_value NUMBER(10);
BEGIN
	SELECT value
		INTO prev_value
		FROM (
		SELECT value
			FROM events
			WHERE source=0			-- EVENT_SOURCE_TRIGGERS
				AND object=0		-- EVENT_OBJECT_TRIGGER
				AND objectid=get_prev_value.triggerid
				AND (clock<get_prev_value.clock
					OR (clock=get_prev_value.clock
						AND eventid<get_prev_value.eventid)
					)
				AND value IN (0,1)	-- TRIGGER_VALUE_FALSE (OK), TRIGGER_VALUE_TRUE (PROBLEM)
			ORDER BY source DESC,
				object DESC,
				objectid DESC,
				clock DESC,
				eventid DESC,
				value DESC
		) WHERE rownum = 1;
	RETURN prev_value;
END;
/

-- Which OK events should have value_changed flag set?
-- Those that have a PROBLEM event (or no event) before them.

INSERT INTO tmp_events_eventid (eventid, prev_value, value)
	SELECT eventid,get_prev_value(eventid, objectid, clock) AS prev_value, value
	FROM events
	WHERE source=0					-- EVENT_SOURCE_TRIGGERS
		AND object=0				-- EVENT_OBJECT_TRIGGER
		AND value=0				-- TRIGGER_VALUE_FALSE (OK)
/

-- Which PROBLEM events should have value_changed flag set?
-- (1) Those that have an OK event (or no event) before them.

INSERT INTO tmp_events_eventid (eventid, prev_value, value)
	SELECT e.eventid,get_prev_value(e.eventid, e.objectid, e.clock) AS prev_value, e.value
	FROM events e,triggers t
	WHERE e.source=0				-- EVENT_SOURCE_TRIGGERS
		AND e.object=0				-- EVENT_OBJECT_TRIGGER
		AND e.objectid=t.triggerid
		AND e.value=1				-- TRIGGER_VALUE_TRUE (PROBLEM)
		AND t.type=0
/

-- (2) Those that came from a "MULTIPLE PROBLEM" trigger.

INSERT INTO tmp_events_eventid (eventid, value)
	SELECT e.eventid, e.value
		FROM events e,triggers t
		WHERE e.source=0			-- EVENT_SOURCE_TRIGGERS
			AND e.object=0			-- EVENT_OBJECT_TRIGGER
			AND e.objectid=t.triggerid
			AND e.value=1			-- TRIGGER_VALUE_TRUE (PROBLEM)
			AND t.type=1
/

DELETE FROM tmp_events_eventid WHERE prev_value = value;

-- Update the value_changed flag.

DROP INDEX tmp_events_index;
DROP FUNCTION get_prev_value;

UPDATE events SET value_changed=1 WHERE eventid IN (SELECT eventid FROM tmp_events_eventid);

DROP TABLE tmp_events_eventid;

-- End event redesign patch

----
---- Patching table `triggers`
----

ALTER TABLE triggers MODIFY triggerid DEFAULT NULL;
ALTER TABLE triggers MODIFY templateid DEFAULT NULL;
ALTER TABLE triggers MODIFY templateid NULL;
ALTER TABLE triggers DROP COLUMN dep_level;
ALTER TABLE triggers ADD value_flags number(10) DEFAULT '0' NOT NULL;
ALTER TABLE triggers ADD flags number(10) DEFAULT '0' NOT NULL;
UPDATE triggers SET templateid=NULL WHERE templateid=0;
UPDATE triggers SET templateid=NULL WHERE NOT templateid IS NULL AND NOT templateid IN (SELECT triggerid FROM triggers);
ALTER TABLE triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE;

-- Begin event redesign patch

CREATE TABLE tmp_triggers (triggerid number(20) PRIMARY KEY, eventid number(20))
/

INSERT INTO tmp_triggers (triggerid, eventid)
(
	SELECT t.triggerid, MAX(e.eventid)
		FROM triggers t, events e
		WHERE t.value=2				-- TRIGGER_VALUE_UNKNOWN
			AND e.source=0			-- EVENT_SOURCE_TRIGGERS
			AND e.object=0			-- EVENT_OBJECT_TRIGGER
			AND e.objectid=t.triggerid
			AND e.value IN (0,1)		-- TRIGGER_VALUE_FALSE (OK), TRIGGER_VALUE_TRUE (PROBLEM)
		GROUP BY t.triggerid
)
/

UPDATE triggers
	SET value=(
		SELECT e.value
			FROM events e,tmp_triggers t
			WHERE e.eventid=t.eventid
				AND triggers.triggerid=t.triggerid
	)
	WHERE triggerid IN (
		SELECT triggerid
			FROM tmp_triggers
	)
/

UPDATE triggers
	SET value=0,					-- TRIGGER_VALUE_FALSE
		value_flags=1
	WHERE value NOT IN (0,1)			-- TRIGGER_VALUE_FALSE (OK), TRIGGER_VALUE_TRUE (PROBLEM)
/

DROP TABLE tmp_triggers
/

-- End event redesign patch
ALTER TABLE user_history MODIFY userhistoryid DEFAULT NULL;
ALTER TABLE user_history MODIFY userid DEFAULT NULL;
DELETE FROM user_history WHERE NOT userid IN (SELECT userid FROM users);
ALTER TABLE user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE users_groups MODIFY id DEFAULT NULL;
ALTER TABLE users_groups MODIFY usrgrpid DEFAULT NULL;
ALTER TABLE users_groups MODIFY userid DEFAULT NULL;
DELETE FROM users_groups WHERE usrgrpid NOT IN (SELECT usrgrpid FROM usrgrp);
DELETE FROM users_groups WHERE userid NOT IN (SELECT userid FROM users);

-- remove duplicates to allow unique index
DELETE FROM users_groups
	WHERE id IN (
		SELECT ug1.id
		FROM users_groups ug1
		LEFT OUTER JOIN (
			SELECT MIN(ug2.id) AS id
			FROM users_groups ug2
			GROUP BY ug2.usrgrpid,ug2.userid
		) keep_rows ON
			ug1.id=keep_rows.id
		WHERE keep_rows.id IS NULL
	);

DROP INDEX users_groups_1;
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE users MODIFY (
	userid DEFAULT NULL,
	lang DEFAULT 'en_GB',
	theme DEFAULT 'default'
);
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
ALTER TABLE usrgrp MODIFY usrgrpid DEFAULT NULL;
ALTER TABLE usrgrp DROP COLUMN api_access;
ALTER TABLE valuemaps MODIFY valuemapid DEFAULT NULL;
