ALTER TABLE ONLY acknowledges ALTER acknowledgeid DROP DEFAULT,
			      ALTER userid DROP DEFAULT,
			      ALTER eventid DROP DEFAULT;
DELETE FROM acknowledges WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=acknowledges.userid);
DELETE FROM acknowledges WHERE NOT EXISTS (SELECT 1 FROM events WHERE events.eventid=acknowledges.eventid);
ALTER TABLE ONLY acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE ONLY acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE ONLY actions ALTER actionid DROP DEFAULT;
UPDATE actions SET esc_period=3600 WHERE eventsource=0 AND esc_period=0;
ALTER TABLE ONLY alerts ALTER alertid DROP DEFAULT,
			ALTER actionid DROP DEFAULT,
			ALTER eventid DROP DEFAULT,
			ALTER userid DROP DEFAULT,
			ALTER userid DROP NOT NULL,
			ALTER mediatypeid DROP DEFAULT,
			ALTER mediatypeid DROP NOT NULL;
UPDATE alerts SET userid=NULL WHERE userid=0;
UPDATE alerts SET mediatypeid=NULL WHERE mediatypeid=0;
DELETE FROM alerts WHERE NOT EXISTS (SELECT 1 FROM actions WHERE actions.actionid=alerts.actionid);
DELETE FROM alerts WHERE NOT EXISTS (SELECT 1 FROM events WHERE events.eventid=alerts.eventid);
DELETE FROM alerts WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=alerts.userid);
DELETE FROM alerts WHERE NOT EXISTS (SELECT 1 FROM media_type WHERE media_type.mediatypeid=alerts.mediatypeid);
ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE ONLY alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE ONLY applications ALTER applicationid DROP DEFAULT,
			      ALTER hostid DROP DEFAULT,
			      ALTER templateid DROP DEFAULT,
			      ALTER templateid DROP NOT NULL;
DELETE FROM applications WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=applications.hostid);
UPDATE applications SET templateid=NULL WHERE templateid=0;
UPDATE applications SET templateid=NULL WHERE templateid IS NOT NULL AND NOT EXISTS (SELECT 1 FROM applications a WHERE a.applicationid=applications.templateid);
ALTER TABLE ONLY applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY applications ADD CONSTRAINT c_applications_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE ONLY auditlog_details ALTER auditdetailid DROP DEFAULT,
				  ALTER auditid DROP DEFAULT;
DELETE FROM auditlog_details WHERE NOT EXISTS (SELECT 1 FROM auditlog WHERE auditlog.auditid=auditlog_details.auditid);
ALTER TABLE ONLY auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE;
ALTER TABLE ONLY auditlog ALTER auditid DROP DEFAULT,
			  ALTER userid DROP DEFAULT;
DELETE FROM auditlog WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=auditlog.userid);
ALTER TABLE ONLY auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
DROP INDEX autoreg_host_1;
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);
ALTER TABLE ONLY autoreg_host ALTER autoreg_hostid DROP DEFAULT,
			      ALTER proxy_hostid DROP DEFAULT,
			      ALTER proxy_hostid DROP NOT NULL,
			      ADD listen_ip varchar(39) DEFAULT '' NOT NULL,
			      ADD listen_port integer DEFAULT '0' NOT NULL,
			      ADD listen_dns varchar(64) DEFAULT '' NOT NULL;
UPDATE autoreg_host SET proxy_hostid=NULL WHERE proxy_hostid=0;
DELETE FROM autoreg_host WHERE proxy_hostid IS NOT NULL AND NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=autoreg_host.proxy_hostid);
ALTER TABLE ONLY autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY conditions ALTER conditionid DROP DEFAULT,
			    ALTER actionid DROP DEFAULT;
DELETE FROM conditions WHERE NOT EXISTS (SELECT 1 FROM actions WHERE actions.actionid=conditions.actionid);
ALTER TABLE ONLY conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE ONLY config
	ALTER configid DROP DEFAULT,
	ALTER alert_usrgrpid DROP DEFAULT,
	ALTER alert_usrgrpid DROP NOT NULL,
	ALTER discovery_groupid DROP DEFAULT,
	ALTER default_theme SET DEFAULT 'originalblue',
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

UPDATE config SET alert_usrgrpid=NULL WHERE NOT EXISTS (SELECT 1 FROM usrgrp WHERE usrgrp.usrgrpid=config.alert_usrgrpid);
UPDATE config SET discovery_groupid=(SELECT MIN(groupid) FROM groups) WHERE NOT EXISTS (SELECT 1 FROM groups WHERE groups.groupid=config.discovery_groupid);

UPDATE config SET default_theme='darkblue' WHERE default_theme='css_bb.css';
UPDATE config SET default_theme='originalblue' WHERE default_theme IN ('css_ob.css', 'default.css');
UPDATE config SET default_theme='darkorange' WHERE default_theme='css_od.css';

ALTER TABLE ONLY config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE ONLY config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid);
-- See drules.sql
ALTER TABLE ONLY dhosts ALTER dhostid DROP DEFAULT,
			ALTER druleid DROP DEFAULT;
DELETE FROM dhosts WHERE NOT EXISTS (SELECT 1 FROM drules WHERE drules.druleid=dhosts.druleid);
ALTER TABLE ONLY dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
ALTER TABLE ONLY dchecks ALTER dcheckid DROP DEFAULT,
			 ALTER druleid DROP DEFAULT,
			 ALTER key_ SET DEFAULT '',
			 ALTER snmp_community SET DEFAULT '',
			 ADD uniq integer DEFAULT '0' NOT NULL;
DELETE FROM dchecks WHERE NOT EXISTS (SELECT 1 FROM drules WHERE drules.druleid=dchecks.druleid);
ALTER TABLE ONLY dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
UPDATE dchecks SET uniq=1 WHERE EXISTS (SELECT 1 FROM drules WHERE drules.unique_dcheckid=dchecks.dcheckid);
ALTER TABLE ONLY drules ALTER druleid DROP DEFAULT,
			ALTER proxy_hostid DROP DEFAULT,
			ALTER proxy_hostid DROP NOT NULL,
			ALTER delay SET DEFAULT '3600',
			DROP unique_dcheckid;
UPDATE drules SET proxy_hostid=NULL WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=drules.proxy_hostid);
ALTER TABLE ONLY drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE ONLY dservices ALTER dserviceid DROP DEFAULT,
			   ALTER dhostid DROP DEFAULT,
			   ALTER dcheckid DROP DEFAULT,
			   ALTER key_ SET DEFAULT '',
			   ALTER value SET DEFAULT '',
			   ADD dns varchar(64) DEFAULT '' NOT NULL;
DELETE FROM dservices WHERE NOT EXISTS (SELECT 1 FROM dhosts WHERE dhosts.dhostid=dservices.dhostid);
DELETE FROM dservices WHERE NOT EXISTS (SELECT 1 FROM dchecks WHERE dchecks.dcheckid=dservices.dcheckid);
ALTER TABLE ONLY dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE;
ALTER TABLE ONLY dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE;
ALTER TABLE ONLY escalations
	ALTER escalationid DROP DEFAULT,
	ALTER actionid DROP DEFAULT,
	ALTER triggerid DROP DEFAULT,
	ALTER triggerid DROP NOT NULL,
	ALTER eventid DROP DEFAULT,
	ALTER eventid DROP NOT NULL,
	ALTER r_eventid DROP DEFAULT,
	ALTER r_eventid DROP NOT NULL;
DROP INDEX escalations_2;

-- 0: ESCALATION_STATUS_ACTIVE
-- 1: ESCALATION_STATUS_RECOVERY
-- 2: ESCALATION_STATUS_SLEEP
-- 4: ESCALATION_STATUS_SUPERSEDED_ACTIVE
-- 5: ESCALATION_STATUS_SUPERSEDED_RECOVERY
UPDATE escalations SET status=0 WHERE status in (1,4,5);

CREATE SEQUENCE escalations_seq;
SELECT setval('escalations_seq', max(escalationid)) FROM escalations;

INSERT INTO escalations (escalationid, actionid, triggerid, r_eventid)
	SELECT NEXTVAL('escalations_seq'), actionid, triggerid, r_eventid
		FROM escalations
		WHERE status = 0
			AND eventid IS NOT NULL
			AND r_eventid IS NOT NULL;
UPDATE escalations SET r_eventid = NULL WHERE eventid IS NOT NULL AND r_eventid IS NOT NULL;

DROP SEQUENCE escalations_seq;
-- See triggers.sql
ALTER TABLE ONLY expressions ALTER expressionid DROP DEFAULT,
			     ALTER regexpid DROP DEFAULT;
DELETE FROM expressions WHERE NOT EXISTS (SELECT 1 FROM regexps WHERE regexps.regexpid = expressions.regexpid);
ALTER TABLE ONLY expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE;
ALTER TABLE ONLY functions ALTER functionid DROP DEFAULT,
			   ALTER itemid DROP DEFAULT,
			   ALTER triggerid DROP DEFAULT,
			   DROP COLUMN lastvalue;
DELETE FROM functions WHERE NOT EXISTS (SELECT 1 FROM items WHERE items.itemid=functions.itemid);
DELETE FROM functions WHERE NOT EXISTS (SELECT 1 FROM triggers WHERE triggers.triggerid=functions.triggerid);
ALTER TABLE ONLY functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE ONLY globalmacro ALTER globalmacroid DROP DEFAULT;
CREATE TABLE globalvars (
	globalvarid              bigint                                    NOT NULL,
	snmp_lastsize            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
);
CREATE TABLE graph_discovery (
	graphdiscoveryid         bigint                                    NOT NULL,
	graphid                  bigint                                    NOT NULL,
	parent_graphid           bigint                                    NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (graphdiscoveryid)
);
CREATE UNIQUE INDEX graph_discovery_1 on graph_discovery (graphid,parent_graphid);
ALTER TABLE ONLY graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE ONLY graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE ONLY graphs_items
	ALTER gitemid DROP DEFAULT,
	ALTER graphid DROP DEFAULT,
	ALTER itemid DROP DEFAULT,
	DROP COLUMN periods_cnt;
UPDATE graphs_items SET type=0 WHERE type=1;
DELETE FROM graphs_items WHERE NOT EXISTS (SELECT 1 FROM graphs WHERE graphs.graphid=graphs_items.graphid);
DELETE FROM graphs_items WHERE NOT EXISTS (SELECT 1 FROM items WHERE items.itemid=graphs_items.itemid);
ALTER TABLE ONLY graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE ONLY graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY graphs ALTER graphid DROP DEFAULT,
			ALTER templateid DROP DEFAULT,
			ALTER templateid DROP NOT NULL,
			ALTER ymin_itemid DROP DEFAULT,
			ALTER ymin_itemid DROP NOT NULL,
			ALTER ymax_itemid DROP DEFAULT,
			ALTER ymax_itemid DROP NOT NULL,
			ALTER show_legend SET DEFAULT 1,
			ADD flags integer DEFAULT '0' NOT NULL;
UPDATE graphs SET show_legend=1 WHERE graphtype=0 OR graphtype=1;
UPDATE graphs SET templateid=NULL WHERE templateid=0;
UPDATE graphs SET templateid=NULL WHERE templateid IS NOT NULL AND NOT EXISTS (SELECT 1 FROM graphs g WHERE g.graphid=graphs.templateid);
UPDATE graphs SET ymin_itemid=NULL WHERE ymin_itemid=0 OR NOT EXISTS (SELECT itemid FROM items WHERE items.itemid=graphs.ymin_itemid);
UPDATE graphs SET ymax_itemid=NULL WHERE ymax_itemid=0 OR NOT EXISTS (SELECT itemid FROM items WHERE items.itemid=graphs.ymax_itemid);
UPDATE graphs SET ymin_type=0 WHERE ymin_type=2 AND ymin_itemid=NULL;
UPDATE graphs SET ymax_type=0 WHERE ymax_type=2 AND ymax_itemid=NULL;
ALTER TABLE ONLY graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE ONLY graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid);
ALTER TABLE ONLY graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid);
ALTER TABLE ONLY graph_theme ALTER graphthemeid DROP DEFAULT,
			     ALTER noneworktimecolor SET DEFAULT 'CCCCCC';
ALTER TABLE ONLY graph_theme RENAME noneworktimecolor TO nonworktimecolor;

UPDATE graph_theme SET theme = 'darkblue' WHERE theme = 'css_bb.css';
UPDATE graph_theme SET theme = 'originalblue' WHERE theme = 'css_ob.css';

-- Insert new graph theme
INSERT INTO graph_theme (graphthemeid, description, theme, backgroundcolor, graphcolor, graphbordercolor, gridcolor, maingridcolor, gridbordercolor, textcolor, highlightcolor, leftpercentilecolor, rightpercentilecolor, nonworktimecolor, gridview, legendview)
VALUES ((SELECT MAX(graphthemeid) + 1 FROM graph_theme), 'Dark orange', 'darkorange', '333333', '0A0A0A', '888888', '222222', '4F4F4F', 'EFEFEF', 'DFDFDF', 'FF5500', 'FF5500', 'FF1111', '1F1F1F', 1, 1);
INSERT INTO graph_theme (graphthemeid, description, theme, backgroundcolor, graphcolor, graphbordercolor, gridcolor, maingridcolor, gridbordercolor, textcolor, highlightcolor, leftpercentilecolor, rightpercentilecolor, nonworktimecolor, gridview, legendview)
VALUES ((SELECT MAX(graphthemeid) + 1 FROM graph_theme), 'Classic', 'classic', 'F0F0F0', 'FFFFFF', '333333', 'CCCCCC', 'AAAAAA', '000000', '222222', 'AA4444', '11CC11', 'CC1111', 'E0E0E0', 1, 1);

DELETE FROM ids WHERE table_name = 'graph_theme';
ALTER TABLE ONLY groups ALTER groupid DROP DEFAULT;
DROP TABLE help_items;

CREATE TABLE help_items (
	itemtype	integer		DEFAULT '0'	NOT NULL,
	key_		varchar(255)	DEFAULT ''	NOT NULL,
	description	varchar(255)	DEFAULT ''	NOT NULL,
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
ALTER TABLE ONLY history_log
	ALTER id DROP DEFAULT,
	ALTER itemid DROP DEFAULT,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history
	ALTER itemid DROP DEFAULT,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history_str
	ALTER itemid DROP DEFAULT,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history_str_sync
	ALTER itemid DROP DEFAULT,
	ALTER nodeid DROP DEFAULT,
	ALTER nodeid TYPE integer,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history_sync
	ALTER itemid DROP DEFAULT,
	ALTER nodeid DROP DEFAULT,
	ALTER nodeid TYPE integer,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history_text
	ALTER id DROP DEFAULT,
	ALTER itemid DROP DEFAULT,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history_uint
	ALTER itemid DROP DEFAULT,
	ADD ns integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY history_uint_sync
	ALTER itemid DROP DEFAULT,
	ALTER nodeid DROP DEFAULT,
	ALTER nodeid TYPE integer,
	ADD ns integer DEFAULT '0' NOT NULL;
DELETE FROM hosts_profiles WHERE NOT EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=hosts_profiles.hostid);
DELETE FROM hosts_profiles_ext WHERE NOT EXISTS (SELECT hostid FROM hosts WHERE hosts.hostid=hosts_profiles_ext.hostid);

CREATE TABLE host_inventory (
	hostid                   bigint                                    NOT NULL,
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
	hardware_full            text            DEFAULT ''                NOT NULL,
	software                 varchar(255)    DEFAULT ''                NOT NULL,
	software_full            text            DEFAULT ''                NOT NULL,
	software_app_a           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_b           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_c           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_d           varchar(64)     DEFAULT ''                NOT NULL,
	software_app_e           varchar(64)     DEFAULT ''                NOT NULL,
	contact                  text            DEFAULT ''                NOT NULL,
	location                 text            DEFAULT ''                NOT NULL,
	location_lat             varchar(16)     DEFAULT ''                NOT NULL,
	location_lon             varchar(16)     DEFAULT ''                NOT NULL,
	notes                    text            DEFAULT ''                NOT NULL,
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
	host_networks            text            DEFAULT ''                NOT NULL,
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
	site_notes               text            DEFAULT ''                NOT NULL,
	poc_1_name               varchar(128)    DEFAULT ''                NOT NULL,
	poc_1_email              varchar(128)    DEFAULT ''                NOT NULL,
	poc_1_phone_a            varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_phone_b            varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_cell               varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_screen             varchar(64)     DEFAULT ''                NOT NULL,
	poc_1_notes              text            DEFAULT ''                NOT NULL,
	poc_2_name               varchar(128)    DEFAULT ''                NOT NULL,
	poc_2_email              varchar(128)    DEFAULT ''                NOT NULL,
	poc_2_phone_a            varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_phone_b            varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_cell               varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_screen             varchar(64)     DEFAULT ''                NOT NULL,
	poc_2_notes              text            DEFAULT ''                NOT NULL,
	PRIMARY KEY (hostid)
);
ALTER TABLE ONLY host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;

-- create temporary t_host_inventory table
CREATE TABLE t_host_inventory (
	hostid                   bigint,
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
UPDATE t_host_inventory SET notes=notes||E'\r\n'||notes_ext WHERE notes<>'' AND notes_ext<>'';
UPDATE t_host_inventory SET notes=notes_ext WHERE notes='';
ALTER TABLE ONLY t_host_inventory DROP COLUMN notes_ext;

-- copy data from temporary table
INSERT INTO host_inventory SELECT * FROM t_host_inventory;

DROP TABLE t_host_inventory;
DROP TABLE hosts_profiles;
DROP TABLE hosts_profiles_ext;

DELETE FROM ids WHERE table_name IN ('hosts_profiles', 'hosts_profiles_ext');
ALTER TABLE ONLY hostmacro ALTER hostmacroid DROP DEFAULT,
			   ALTER hostid DROP DEFAULT;
DELETE FROM hostmacro WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=hostmacro.hostid);
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
ALTER TABLE ONLY hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY hosts_groups ALTER hostgroupid DROP DEFAULT,
			      ALTER hostid DROP DEFAULT,
			      ALTER groupid DROP DEFAULT;
DELETE FROM hosts_groups WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=hosts_groups.hostid);
DELETE FROM hosts_groups WHERE NOT EXISTS (SELECT 1 FROM groups WHERE groups.groupid=hosts_groups.groupid);
-- remove duplicates to allow unique index
DELETE FROM hosts_groups
	WHERE hostgroupid IN (
		SELECT hg1.hostgroupid
		FROM hosts_groups hg1
		LEFT OUTER JOIN (
			SELECT MIN(hg2.hostgroupid) AS hostgroupid
			FROM hosts_groups hg2
			GROUP BY hg2.hostid,hg2.groupid
		) keep_rows ON
			hg1.hostgroupid=keep_rows.hostgroupid
		WHERE keep_rows.hostgroupid IS NULL
	);
DROP INDEX hosts_groups_1;
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);
ALTER TABLE ONLY hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
-- See host_inventory.sql
-- See host_inventory.sql
---- Patching table `interfaces`

CREATE TABLE interface (
	interfaceid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	main                     integer         DEFAULT '0'               NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	useip                    integer         DEFAULT '1'               NOT NULL,
	ip                       varchar(39)     DEFAULT '127.0.0.1'       NOT NULL,
	dns                      varchar(64)     DEFAULT ''                NOT NULL,
	port                     varchar(64)     DEFAULT '10050'           NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_1 on interface (hostid,type);
CREATE INDEX interface_2 on interface (ip,dns);
ALTER TABLE ONLY interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;

-- Passive proxy interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid / 100000000000)*100000000000)) * 3 + ((hostid / 100000000000)*100000000000),
		hostid,1,0,ip,dns,useip,port
	FROM hosts
	WHERE status IN (6));	-- HOST_STATUS_PROXY_PASSIVE

-- Zabbix Agent interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid / 100000000000)*100000000000)) * 3 + ((hostid / 100000000000)*100000000000),
		hostid,1,1,ip,dns,useip,port
	FROM hosts
	WHERE status IN (0,1));

-- SNMP interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid / 100000000000)*100000000000)) * 3 + ((hostid / 100000000000)*100000000000) + 1,
		hostid,1,2,ip,dns,useip,'161'
	FROM hosts
	WHERE status IN (0,1)
		AND EXISTS (SELECT DISTINCT i.hostid FROM items i WHERE i.hostid=hosts.hostid and i.type IN (1,4,6)));	-- SNMPv1, SNMPv2c, SNMPv3

-- IPMI interface
INSERT INTO interface (interfaceid,hostid,main,type,ip,dns,useip,port)
	(SELECT (hostid - ((hostid / 100000000000)*100000000000)) * 3 + ((hostid / 100000000000)*100000000000) + 2,
		hostid,1,3,'',ipmi_ip,0,ipmi_port
	FROM hosts
	WHERE status IN (0,1) AND useipmi=1);

---- Patching table `items`

ALTER TABLE ONLY items RENAME COLUMN description TO name;
ALTER TABLE ONLY items
	ALTER itemid DROP DEFAULT,
	ALTER hostid DROP DEFAULT,
	ALTER units TYPE varchar(255),
	ALTER lastlogsize TYPE numeric(20),
	ALTER templateid DROP DEFAULT,
	ALTER templateid DROP NOT NULL,
	ALTER valuemapid DROP DEFAULT,
	ALTER valuemapid DROP NOT NULL,
	ADD lastns integer NULL,
	ADD flags integer DEFAULT '0' NOT NULL,
	ADD filter varchar(255) DEFAULT '' NOT NULL,
	ADD interfaceid bigint NULL,
	ADD port varchar(64) DEFAULT '' NOT NULL,
	ADD description text DEFAULT '' NOT NULL,
	ADD inventory_link integer DEFAULT '0' NOT NULL,
	ADD lifetime varchar(64) DEFAULT '30' NOT NULL;
UPDATE items
	SET templateid=NULL
	WHERE templateid=0
		OR NOT EXISTS (SELECT 1 FROM items i WHERE i.itemid=items.templateid);
UPDATE items
	SET valuemapid=NULL
	WHERE valuemapid=0
		OR NOT EXISTS (SELECT 1 FROM valuemaps v WHERE v.valuemapid=items.valuemapid);
UPDATE items SET units='Bps' WHERE type=9 AND units='bps';
DELETE FROM items WHERE NOT hostid IN (SELECT hostid FROM hosts);
ALTER TABLE ONLY items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid);
ALTER TABLE ONLY items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid);

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
	SET key_ = SUBSTR(key_, 1, STRPOS(key_, '[')) || '"{HOST.CONN}",' || SUBSTR(key_, STRPOS(key_, '[') + 1)
	WHERE type IN (10)	-- EXTERNAL
		AND STRPOS(key_, '[') <> 0;

UPDATE items
	SET key_ = key_ || '["{HOST.CONN}"]'
	WHERE type IN (10)	-- EXTERNAL
		AND STRPOS(key_, '[') = 0;

-- convert simple check keys to a new form

CREATE LANGUAGE 'plpgsql';

CREATE FUNCTION zbx_convert_simple_checks(v_itemid bigint, v_hostid bigint, v_key varchar(255))
RETURNS varchar(255) AS $$
DECLARE old_key varchar(255);
	new_key varchar(255);
	pos integer;
BEGIN
	old_key := v_key;
	new_key := 'net.tcp.service';
	pos := STRPOS(old_key, '_perf');
	IF 0 <> pos THEN
		new_key := new_key || '.perf';
		old_key := SUBSTR(old_key, 1, pos - 1) || SUBSTR(old_key, pos + 5);
	END IF;
	new_key := new_key || '[';
	pos := STRPOS(old_key, ',');
	IF 0 <> pos THEN
		new_key := new_key || '"' || SUBSTR(old_key, 1, pos - 1) || '"';
		old_key := SUBSTR(old_key, pos + 1);
	ELSE
		new_key := new_key || '"' || old_key || '"';
		old_key := '';
	END IF;
	IF 0 <> LENGTH(old_key) THEN
		new_key := new_key || ',,"' || old_key || '"';
	END IF;

	WHILE 0 != (SELECT COUNT(*) FROM items WHERE hostid = v_hostid AND key_ = new_key || ']') LOOP
		new_key := new_key || ' ';
	END LOOP;

	RETURN new_key || ']';
END;
$$ LANGUAGE 'plpgsql';

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

DROP FUNCTION zbx_convert_simple_checks(v_itemid bigint, v_hostid bigint, v_key varchar(255));

DROP LANGUAGE 'plpgsql';

-- adding web.test.error[<web check>] items

CREATE LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION zbx_add_web_error_items()
RETURNS void AS $$
DECLARE
	httptest_nodeid INT;
	init_nodeid BIGINT;
	min_nodeid BIGINT;
	max_nodeid BIGINT;
	node_cursor CURSOR FOR (SELECT DISTINCT httptestid / 100000000000000 FROM httptest);
	res BIGINT;
BEGIN

	OPEN node_cursor;
	LOOP
		FETCH node_cursor INTO httptest_nodeid;
		IF NOT FOUND THEN
			EXIT;
		END IF;

		min_nodeid := httptest_nodeid * 100000000000000;
		max_nodeid := min_nodeid + 99999999999999;
		init_nodeid := (httptest_nodeid * 1000 + httptest_nodeid) * 100000000000;

		CREATE SEQUENCE items_seq;
		CREATE SEQUENCE httptestitem_seq;
		CREATE SEQUENCE items_applications_seq;

		SELECT setval('items_seq', GREATEST(MAX(itemid), init_nodeid)) INTO res FROM items WHERE itemid BETWEEN min_nodeid AND max_nodeid;
		SELECT setval('httptestitem_seq', GREATEST(MAX(httptestitemid), init_nodeid)) INTO res FROM httptestitem WHERE httptestitemid BETWEEN min_nodeid AND max_nodeid;
		SELECT setval('items_applications_seq', GREATEST(MAX(itemappid), init_nodeid)) INTO res FROM items_applications WHERE itemappid BETWEEN min_nodeid AND max_nodeid;

		INSERT INTO items (itemid, hostid, type, name, key_, value_type, units, delay, history, trends, status)
			SELECT NEXTVAL('items_seq'), hostid, type, 'Last error message of scenario ''$1''', 'web.test.error' || SUBSTR(key_, STRPOS(key_, '[')), 1, '', delay, history, 0, status
			FROM items
			WHERE type = 9
				AND key_ LIKE 'web.test.fail%'
				AND itemid BETWEEN min_nodeid AND max_nodeid;

		INSERT INTO httptestitem (httptestitemid, httptestid, itemid, type)
			SELECT NEXTVAL('httptestitem_seq'), ht.httptestid, i.itemid, 4
			FROM httptest ht,applications a,items i
			WHERE ht.applicationid=a.applicationid
				AND a.hostid=i.hostid
				AND 'web.test.error[' || ht.name || ']' = i.key_
				AND itemid BETWEEN min_nodeid AND max_nodeid;

		INSERT INTO items_applications (itemappid, applicationid, itemid)
			SELECT NEXTVAL('items_applications_seq'), ht.applicationid, hti.itemid
			FROM httptest ht, httptestitem hti
			WHERE ht.httptestid = hti.httptestid
				AND hti.type = 4
				AND itemid BETWEEN min_nodeid AND max_nodeid;

		DROP SEQUENCE items_applications_seq;
		DROP SEQUENCE httptestitem_seq;
		DROP SEQUENCE items_seq;

	END LOOP;

	CLOSE node_cursor;
END;
$$ LANGUAGE 'plpgsql';

SELECT zbx_add_web_error_items();

DROP FUNCTION zbx_add_web_error_items();

DROP LANGUAGE 'plpgsql';

DELETE FROM ids WHERE table_name IN ('items', 'httptestitem', 'items_applications');

---- Patching table `hosts`

ALTER TABLE ONLY hosts ALTER hostid DROP DEFAULT,
		       ALTER proxy_hostid DROP DEFAULT,
		       ALTER proxy_hostid DROP NOT NULL,
		       ALTER maintenanceid DROP DEFAULT,
		       ALTER maintenanceid DROP NOT NULL,
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
UPDATE hosts SET name=host WHERE status in (0,1,3);	-- MONITORED, NOT_MONITORED, TEMPLATE
CREATE INDEX hosts_4 on hosts (name);
ALTER TABLE ONLY hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE ONLY hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid);
DELETE FROM hosts_templates WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=hosts_templates.hostid);
DELETE FROM hosts_templates WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=hosts_templates.templateid);

CREATE TABLE t_hosts_templates (
	hosttemplateid           bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	templateid               bigint                                    NOT NULL
);

INSERT INTO t_hosts_templates (SELECT hosttemplateid, hostid, templateid FROM hosts_templates);

DROP TABLE hosts_templates;

CREATE TABLE hosts_templates (
	hosttemplateid           bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	templateid               bigint                                    NOT NULL,
	PRIMARY KEY (hosttemplateid)
);
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
ALTER TABLE ONLY hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;

INSERT INTO hosts_templates (SELECT hosttemplateid, hostid, templateid FROM t_hosts_templates);

DROP TABLE t_hosts_templates;
ALTER TABLE ONLY housekeeper ALTER housekeeperid DROP DEFAULT,
			     ALTER value DROP DEFAULT;
ALTER TABLE ONLY httpstepitem ALTER httpstepitemid DROP DEFAULT,
			      ALTER httpstepid DROP DEFAULT,
			      ALTER itemid DROP DEFAULT;
DELETE FROM httpstepitem WHERE NOT EXISTS (SELECT 1 FROM httpstep WHERE httpstep.httpstepid=httpstepitem.httpstepid);
DELETE FROM httpstepitem WHERE NOT EXISTS (SELECT 1 FROM items WHERE items.itemid=httpstepitem.itemid);
ALTER TABLE ONLY httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE;
ALTER TABLE ONLY httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY httpstep ALTER httpstepid DROP DEFAULT,
			  ALTER httptestid DROP DEFAULT;
DELETE FROM httpstep WHERE NOT EXISTS (SELECT 1 FROM httptest WHERE httptest.httptestid=httpstep.httptestid);
ALTER TABLE ONLY httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE ONLY httptestitem ALTER httptestitemid DROP DEFAULT,
			      ALTER httptestid DROP DEFAULT,
			      ALTER itemid DROP DEFAULT;
DELETE FROM httptestitem WHERE NOT EXISTS (SELECT 1 FROM httptest WHERE httptest.httptestid=httptestitem.httptestid);
DELETE FROM httptestitem WHERE NOT EXISTS (SELECT 1 FROM items WHERE items.itemid=httptestitem.itemid);
ALTER TABLE ONLY httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE ONLY httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY httptest
	ALTER httptestid DROP DEFAULT,
	ALTER applicationid DROP DEFAULT,
	DROP COLUMN lastcheck,
	DROP COLUMN curstate,
	DROP COLUMN curstep,
	DROP COLUMN lastfailedstep,
	DROP COLUMN time,
	DROP COLUMN error;
DELETE FROM httptest WHERE NOT EXISTS (SELECT 1 FROM applications WHERE applications.applicationid=httptest.applicationid);
ALTER TABLE ONLY httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
-- See icon_map.sql
CREATE TABLE icon_map (
	iconmapid                bigint                                    NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	default_iconid           bigint                                    NOT NULL,
	PRIMARY KEY (iconmapid)
);
CREATE INDEX icon_map_1 ON icon_map (name);
ALTER TABLE ONLY icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid);

CREATE TABLE icon_mapping (
	iconmappingid            bigint                                    NOT NULL,
	iconmapid                bigint                                    NOT NULL,
	iconid                   bigint                                    NOT NULL,
	inventory_link           integer         DEFAULT '0'               NOT NULL,
	expression               varchar(64)     DEFAULT ''                NOT NULL,
	sortorder                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (iconmappingid)
);
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);
ALTER TABLE ONLY icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE;
ALTER TABLE ONLY icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid);
ALTER TABLE ONLY ids ALTER nodeid DROP DEFAULT;
ALTER TABLE ONLY ids ALTER nextid DROP DEFAULT;
ALTER TABLE ONLY images ALTER imageid DROP DEFAULT;
-- See hosts.sql
CREATE TABLE item_discovery (
	itemdiscoveryid          bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	parent_itemid            bigint                                    NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	lastcheck                integer         DEFAULT '0'               NOT NULL,
	ts_delete                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
);
CREATE UNIQUE INDEX item_discovery_1 on item_discovery (itemid,parent_itemid);
ALTER TABLE ONLY item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE ONLY items_applications ALTER itemappid DROP DEFAULT,
				    ALTER applicationid DROP DEFAULT,
				    ALTER itemid DROP DEFAULT;
DROP INDEX items_applications_1;
DELETE FROM items_applications WHERE NOT EXISTS (SELECT 1 FROM applications WHERE applications.applicationid=items_applications.applicationid);
DELETE FROM items_applications WHERE NOT EXISTS (SELECT 1 FROM items WHERE items.itemid=items_applications.itemid);
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);
ALTER TABLE ONLY items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE ONLY items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
-- See hosts.sql
ALTER TABLE ONLY maintenances_groups ALTER maintenance_groupid DROP DEFAULT,
				     ALTER maintenanceid DROP DEFAULT,
				     ALTER groupid DROP DEFAULT;
DROP INDEX maintenances_groups_1;
DELETE FROM maintenances_groups WHERE NOT EXISTS (SELECT 1 FROM maintenances WHERE maintenances.maintenanceid=maintenances_groups.maintenanceid);
DELETE FROM maintenances_groups WHERE NOT EXISTS (SELECT 1 FROM groups WHERE groups.groupid=maintenances_groups.groupid);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
ALTER TABLE ONLY maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE ONLY maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE ONLY maintenances_hosts ALTER maintenance_hostid DROP DEFAULT,
				    ALTER maintenanceid DROP DEFAULT,
				    ALTER hostid DROP DEFAULT;
DROP INDEX maintenances_hosts_1;
DELETE FROM maintenances_hosts WHERE NOT EXISTS (SELECT 1 FROM maintenances WHERE maintenances.maintenanceid=maintenances_hosts.maintenanceid);
DELETE FROM maintenances_hosts WHERE NOT EXISTS (SELECT 1 FROM hosts WHERE hosts.hostid=maintenances_hosts.hostid);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
ALTER TABLE ONLY maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE ONLY maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY maintenances ALTER maintenanceid DROP DEFAULT;
ALTER TABLE ONLY maintenances_windows ALTER maintenance_timeperiodid DROP DEFAULT,
				      ALTER maintenanceid DROP DEFAULT,
				      ALTER timeperiodid DROP DEFAULT;
DROP INDEX maintenances_windows_1;
DELETE FROM maintenances_windows WHERE NOT EXISTS (SELECT 1 FROM maintenances WHERE maintenances.maintenanceid=maintenances_windows.maintenanceid);
DELETE FROM maintenances_windows WHERE NOT EXISTS (SELECT 1 FROM timeperiods WHERE timeperiods.timeperiodid=maintenances_windows.timeperiodid);
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);
ALTER TABLE ONLY maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE ONLY maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE;
ALTER TABLE ONLY mappings ALTER mappingid DROP DEFAULT,
			  ALTER valuemapid DROP DEFAULT;
DELETE FROM mappings WHERE NOT EXISTS (SELECT 1 FROM valuemaps WHERE valuemaps.valuemapid=mappings.valuemapid);
ALTER TABLE ONLY mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE;
ALTER TABLE ONLY media ALTER mediaid DROP DEFAULT,
		       ALTER userid DROP DEFAULT,
		       ALTER mediatypeid DROP DEFAULT,
		       ALTER period SET DEFAULT '1-7,00:00-24:00';
DELETE FROM media WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=media.userid);
DELETE FROM media WHERE NOT EXISTS (SELECT 1 FROM media_type WHERE media_type.mediatypeid=media.mediatypeid);
ALTER TABLE ONLY media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE ONLY media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE ONLY media_type
	ALTER mediatypeid DROP DEFAULT,
	ADD status integer DEFAULT '0' NOT NULL;
CREATE LANGUAGE 'plpgsql';

CREATE or REPLACE FUNCTION zbx_drop_index(idx_name varchar)
RETURNS VOID
AS $$
DECLARE cnt integer;
BEGIN
	SELECT INTO cnt count(relname)
		FROM pg_class
		WHERE relname=idx_name
			AND oid IN (
				SELECT indexrelid
					FROM pg_index, pg_class
					WHERE pg_class.oid=pg_index.indrelid);
	IF cnt > 0 THEN
		EXECUTE 'DROP INDEX ' || idx_name;
	END IF;
END;
$$ LANGUAGE 'plpgsql';

SELECT zbx_drop_index('node_cksum_1');
SELECT zbx_drop_index('node_cksum_cksum_1');

DROP FUNCTION zbx_drop_index(idx_name varchar);

DROP LANGUAGE 'plpgsql';

ALTER TABLE ONLY node_cksum ALTER nodeid DROP DEFAULT,
			    ALTER recordid DROP DEFAULT;
DELETE FROM node_cksum WHERE NOT EXISTS (SELECT 1 FROM nodes WHERE nodes.nodeid=node_cksum.nodeid);
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);
ALTER TABLE ONLY node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE;
ALTER TABLE ONLY nodes
	ALTER nodeid DROP DEFAULT,
	ALTER masterid DROP DEFAULT,
	ALTER masterid DROP NOT NULL,
	DROP COLUMN timezone,
	DROP COLUMN slave_history,
	DROP COLUMN slave_trends;
UPDATE nodes SET masterid=NULL WHERE masterid=0;
ALTER TABLE ONLY nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid);
-- See operations.sql
-- See operations.sql
-- See operations.sql
CREATE TABLE t_operations (
	operationid		bigint,
	new_operationid		bigint,
	actionid		bigint,
	operationtype		integer,
	object			integer,
	objectid		bigint,
	shortdata		varchar(255),
	longdata		text,
	esc_period		integer,
	esc_step_from		integer,
	esc_step_to		integer,
	default_msg		integer,
	evaltype		integer,
	mediatypeid		bigint,
	is_host			integer,
	hostid			bigint,
	groupid			bigint
);

CREATE TABLE t_opconditions (
	operationid		bigint,
	conditiontype		integer,
	operator		integer,
	value			varchar(255)
);

CREATE OR REPLACE FUNCTION zbx_unnest(anyarray) RETURNS SETOF anyelement
LANGUAGE SQL AS $$
	SELECT $1[i]
		FROM generate_series(array_lower($1, 1), array_upper($1, 1)) as i;
$$;

CREATE SEQUENCE operations_seq;

INSERT INTO t_operations
	SELECT o.operationid, NEXTVAL('operations_seq'), o.actionid, o.operationtype, o.object, o.objectid, o.shortdata,
			CASE WHEN operationtype = 1 THEN zbx_unnest(string_to_array(o.longdata, CHR(10))) ELSE o.longdata END,
			o.esc_period, o.esc_step_from, o.esc_step_to, o.default_msg, o.evaltype, omt.mediatypeid,
			NULL, NULL, NULL
		FROM actions a, operations o
			LEFT JOIN opmediatypes omt ON omt.operationid=o.operationid
		WHERE a.actionid=o.actionid;

DROP SEQUENCE operations_seq;

DROP FUNCTION zbx_unnest(anyarray);

INSERT INTO t_opconditions
	SELECT operationid, conditiontype, operator, value FROM opconditions;

UPDATE t_operations
	SET new_operationid = (operationid / 100000000000) * 100000000000 + new_operationid
	WHERE operationid >= 100000000000;

UPDATE t_operations
	SET is_host = 1,
		shortdata = TRIM(BOTH ' ' FROM SUBSTRING(longdata FOR POSITION(':' IN longdata) - 1)),
		longdata = TRIM(BOTH ' ' FROM SUBSTRING(longdata FROM POSITION(':' IN longdata) + 1))
	WHERE operationtype IN (1)	-- OPERATION_TYPE_COMMAND
		AND POSITION(':' IN longdata) <> 0
		AND (POSITION('#' IN longdata) = 0 OR POSITION(':' IN longdata) < POSITION('#' IN longdata));

UPDATE t_operations
	SET is_host = 0,
		shortdata = TRIM(BOTH ' ' FROM SUBSTRING(longdata FOR POSITION('#' IN longdata) - 1)),
		longdata = TRIM(BOTH ' ' FROM SUBSTRING(longdata FROM POSITION('#' IN longdata) + 1))
	WHERE operationtype IN (1)	-- OPERATION_TYPE_COMMAND
		AND POSITION('#' IN longdata) <> 0
		AND (POSITION(':' IN longdata) = 0 OR POSITION('#' IN longdata) < POSITION(':' IN longdata));

UPDATE t_operations
	SET longdata = TRIM(TRAILING CHR(13) FROM longdata)
	WHERE operationtype IN (1);	-- OPERATION_TYPE_COMMAND

UPDATE t_operations
	SET hostid = (
		SELECT MIN(h.hostid)
			FROM hosts h
			WHERE h.host = t_operations.shortdata
				AND (h.hostid / 100000000000000) = (t_operations.operationid / 100000000000000))
	WHERE operationtype IN (1)	-- OPERATION_TYPE_COMMAND
		AND is_host = 1
		AND shortdata <> '{HOSTNAME}';

UPDATE t_operations
	SET groupid = (
		SELECT MIN(g.groupid)
			FROM groups g
			WHERE g.name = t_operations.shortdata
				AND (g.groupid / 100000000000000) = (t_operations.operationid / 100000000000000))
	WHERE operationtype IN (1)	-- OPERATION_TYPE_COMMAND
		AND is_host = 0;

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
	operationid              bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL,
	operationtype            integer         DEFAULT '0'               NOT NULL,
	esc_period               integer         DEFAULT '0'               NOT NULL,
	esc_step_from            integer         DEFAULT '1'               NOT NULL,
	esc_step_to              integer         DEFAULT '1'               NOT NULL,
	evaltype                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE INDEX operations_1 ON operations (actionid);
ALTER TABLE ONLY operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;

CREATE TABLE opmessage (
	operationid              bigint                                    NOT NULL,
	default_msg              integer         DEFAULT '0'               NOT NULL,
	subject                  varchar(255)    DEFAULT ''                NOT NULL,
	message                  text            DEFAULT ''                NOT NULL,
	mediatypeid              bigint                                    NULL,
	PRIMARY KEY (operationid)
);
ALTER TABLE ONLY opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid);

CREATE TABLE opmessage_grp (
	opmessage_grpid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	PRIMARY KEY (opmessage_grpid)
);
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
ALTER TABLE ONLY opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);

CREATE TABLE opmessage_usr (
	opmessage_usrid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	PRIMARY KEY (opmessage_usrid)
);
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
ALTER TABLE ONLY opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid);

CREATE TABLE opcommand (
	operationid              bigint                                    NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	scriptid                 bigint                                    NULL,
	execute_on               integer         DEFAULT '0'               NOT NULL,
	port                     varchar(64)     DEFAULT ''                NOT NULL,
	authtype                 integer         DEFAULT '0'               NOT NULL,
	username                 varchar(64)     DEFAULT ''                NOT NULL,
	password                 varchar(64)     DEFAULT ''                NOT NULL,
	publickey                varchar(64)     DEFAULT ''                NOT NULL,
	privatekey               varchar(64)     DEFAULT ''                NOT NULL,
	command                  text            DEFAULT ''                NOT NULL,
	PRIMARY KEY (operationid)
);
ALTER TABLE ONLY opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid);

CREATE TABLE opcommand_hst (
	opcommand_hstid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	hostid                   bigint                                    NULL,
	PRIMARY KEY (opcommand_hstid)
);
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
ALTER TABLE ONLY opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid);

CREATE TABLE opcommand_grp (
	opcommand_grpid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (opcommand_grpid)
);
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
ALTER TABLE ONLY opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);

CREATE TABLE opgroup (
	opgroupid                bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (opgroupid)
);
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
ALTER TABLE ONLY opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);

CREATE TABLE optemplate (
	optemplateid             bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	templateid               bigint                                    NOT NULL,
	PRIMARY KEY (optemplateid)
);
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
ALTER TABLE ONLY optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE ONLY optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid);

CREATE TABLE opconditions (
	opconditionid            bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	conditiontype            integer         DEFAULT '0'               NOT NULL,
	operator                 integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (opconditionid)
);
CREATE INDEX opconditions_1 ON opconditions (operationid);
ALTER TABLE ONLY opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;

CREATE SEQUENCE opconditions_seq;
CREATE SEQUENCE opmessage_grp_seq;
CREATE SEQUENCE opmessage_usr_seq;
CREATE SEQUENCE opcommand_grp_seq;
CREATE SEQUENCE opcommand_hst_seq;
CREATE SEQUENCE opgroup_seq;
CREATE SEQUENCE optemplate_seq;

INSERT INTO operations (operationid, actionid, operationtype, esc_period, esc_step_from, esc_step_to, evaltype)
	SELECT new_operationid, actionid, operationtype, esc_period, esc_step_from, esc_step_to, evaltype
		FROM t_operations;

INSERT INTO opmessage (operationid, default_msg, subject, message, mediatypeid)
	SELECT new_operationid, default_msg, shortdata, longdata, mediatypeid
		FROM t_operations
		WHERE operationtype IN (0);		-- OPERATION_TYPE_MESSAGE

INSERT INTO opmessage_grp (opmessage_grpid, operationid, usrgrpid)
	SELECT NEXTVAL('opmessage_grp_seq'), o.new_operationid, o.objectid
		FROM t_operations o, usrgrp g
		WHERE o.objectid = g.usrgrpid
			AND o.operationtype IN (0)	-- OPERATION_TYPE_MESSAGE
			AND o.object IN (1);		-- OPERATION_OBJECT_GROUP

UPDATE opmessage_grp
	SET opmessage_grpid = (operationid / 100000000000) * 100000000000 + opmessage_grpid
	WHERE operationid >= 100000000000;

INSERT INTO opmessage_usr (opmessage_usrid, operationid, userid)
	SELECT NEXTVAL('opmessage_usr_seq'), o.new_operationid, o.objectid
		FROM t_operations o, users u
		WHERE o.objectid = u.userid
			AND o.operationtype IN (0)	-- OPERATION_TYPE_MESSAGE
			AND o.object IN (0);		-- OPERATION_OBJECT_USER

UPDATE opmessage_usr
	SET opmessage_usrid = (operationid / 100000000000) * 100000000000 + opmessage_usrid
	WHERE operationid >= 100000000000;

INSERT INTO opcommand (operationid, command)
	SELECT new_operationid, longdata
		FROM t_operations
		WHERE operationtype IN (1);		-- OPERATION_TYPE_COMMAND

UPDATE opcommand
	SET type = 1,
		command = TRIM(SUBSTRING(command, 5))
	WHERE SUBSTRING(command, 1, 4) = 'IPMI';

INSERT INTO opcommand_grp (opcommand_grpid, operationid, groupid)
	SELECT NEXTVAL('opcommand_grp_seq'), new_operationid, groupid
		FROM t_operations
		WHERE operationtype IN (1)		-- OPERATION_TYPE_COMMAND
			AND is_host = 0;

UPDATE opcommand_grp
	SET opcommand_grpid = (operationid / 100000000000) * 100000000000 + opcommand_grpid
	WHERE operationid >= 100000000000;

INSERT INTO opcommand_hst (opcommand_hstid, operationid, hostid)
	SELECT NEXTVAL('opcommand_hst_seq'), new_operationid, hostid
		FROM t_operations
		WHERE operationtype IN (1)		-- OPERATION_TYPE_COMMAND
			AND is_host = 1
			AND (hostid IS NOT NULL OR shortdata = '{HOSTNAME}');

UPDATE opcommand_hst
	SET opcommand_hstid = (operationid / 100000000000) * 100000000000 + opcommand_hstid
	WHERE operationid >= 100000000000;

INSERT INTO opgroup (opgroupid, operationid, groupid)
	SELECT NEXTVAL('opgroup_seq'), o.new_operationid, o.objectid
		FROM t_operations o, groups g
		WHERE o.objectid = g.groupid
			AND o.operationtype IN (4,5);	-- OPERATION_TYPE_GROUP_ADD, OPERATION_TYPE_GROUP_REMOVE

UPDATE opgroup
	SET opgroupid = (operationid / 100000000000) * 100000000000 + opgroupid
	WHERE operationid >= 100000000000;

INSERT INTO optemplate (optemplateid, operationid, templateid)
	SELECT NEXTVAL('optemplate_seq'), o.new_operationid, o.objectid
		FROM t_operations o, hosts h
		WHERE o.objectid = h.hostid
			AND o.operationtype IN (6,7);	-- OPERATION_TYPE_TEMPLATE_ADD, OPERATION_TYPE_TEMPLATE_REMOVE

UPDATE optemplate
	SET optemplateid = (operationid / 100000000000) * 100000000000 + optemplateid
	WHERE operationid >= 100000000000;

INSERT INTO opconditions
	SELECT NEXTVAL('opconditions_seq'), o.new_operationid, c.conditiontype, c.operator, c.value
		FROM t_opconditions c, t_operations o
		WHERE c.operationid = o.operationid;

UPDATE opconditions
	SET opconditionid = (operationid / 100000000000) * 100000000000 + opconditionid
	WHERE operationid >= 100000000000;

DROP SEQUENCE optemplate_seq;
DROP SEQUENCE opgroup_seq;
DROP SEQUENCE opcommand_hst_seq;
DROP SEQUENCE opcommand_grp_seq;
DROP SEQUENCE opmessage_usr_seq;
DROP SEQUENCE opmessage_grp_seq;
DROP SEQUENCE opconditions_seq;

DROP TABLE t_operations;
DROP TABLE t_opconditions;

DELETE FROM ids WHERE table_name IN ('operations', 'opconditions', 'opmediatypes');
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
-- See operations.sql
ALTER TABLE ONLY profiles
	ALTER profileid DROP DEFAULT,
	ALTER userid DROP DEFAULT;
DELETE FROM profiles WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=profiles.userid);
DELETE FROM profiles WHERE idx LIKE 'web.%.sort' OR idx LIKE 'web.%.sortorder';
ALTER TABLE ONLY profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;

UPDATE profiles SET idx = 'web.screens.period' WHERE idx = 'web.charts.period';
UPDATE profiles SET idx = 'web.screens.stime' WHERE idx = 'web.charts.stime';
UPDATE profiles SET idx = 'web.screens.timelinefixed' WHERE idx = 'web.charts.timelinefixed';
ALTER TABLE ONLY proxy_autoreg_host ADD listen_ip varchar(39) DEFAULT '' NOT NULL,
				    ADD listen_port integer DEFAULT '0' NOT NULL,
				    ADD listen_dns varchar(64) DEFAULT '' NOT NULL;
DELETE FROM proxy_dhistory WHERE NOT EXISTS (SELECT 1 FROM drules WHERE drules.druleid=proxy_dhistory.druleid);
DELETE FROM proxy_dhistory WHERE dcheckid<>0 AND NOT EXISTS (SELECT 1 FROM dchecks WHERE dchecks.dcheckid=proxy_dhistory.dcheckid);
ALTER TABLE ONLY proxy_dhistory ALTER druleid DROP DEFAULT;
ALTER TABLE ONLY proxy_dhistory ALTER dcheckid DROP NOT NULL;
ALTER TABLE ONLY proxy_dhistory ALTER dcheckid DROP DEFAULT;
ALTER TABLE ONLY proxy_dhistory ADD dns varchar(64) DEFAULT '' NOT NULL;
UPDATE proxy_dhistory SET dcheckid=NULL WHERE dcheckid=0;
ALTER TABLE ONLY proxy_history
	ALTER itemid DROP DEFAULT,
	ADD ns integer DEFAULT '0' NOT NULL,
	ADD status integer DEFAULT '0' NOT NULL;
ALTER TABLE ONLY regexps ALTER regexpid DROP DEFAULT;
ALTER TABLE ONLY rights ALTER rightid DROP DEFAULT,
			ALTER groupid DROP DEFAULT,
			ALTER id SET NOT NULL;
DELETE FROM rights WHERE NOT EXISTS (SELECT 1 FROM usrgrp WHERE usrgrp.usrgrpid=rights.groupid);
DELETE FROM rights WHERE NOT EXISTS (SELECT 1 FROM groups WHERE groups.groupid=rights.id);
ALTER TABLE ONLY rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE ONLY rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE ONLY screens_items
	ALTER screenitemid DROP DEFAULT,
	ALTER screenid DROP DEFAULT,
	ADD sort_triggers integer DEFAULT '0' NOT NULL;
DELETE FROM screens_items WHERE NOT EXISTS (SELECT 1 FROM screens WHERE screens.screenid=screens_items.screenid);
ALTER TABLE ONLY screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE ONLY screens ALTER screenid DROP DEFAULT,
			 ALTER name DROP DEFAULT,
			 ADD templateid bigint NULL;
ALTER TABLE ONLY screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE ONLY scripts
	ALTER scriptid DROP DEFAULT,
	ALTER usrgrpid DROP DEFAULT,
	ALTER usrgrpid DROP NOT NULL,
	ALTER groupid DROP DEFAULT,
	ALTER groupid DROP NOT NULL,
	ADD description text DEFAULT '' NOT NULL,
	ADD confirmation varchar(255) DEFAULT '' NOT NULL,
	ADD type integer DEFAULT '0' NOT NULL,
	ADD execute_on integer DEFAULT '1' NOT NULL;
UPDATE scripts SET usrgrpid=NULL WHERE usrgrpid=0;
UPDATE scripts SET groupid=NULL WHERE groupid=0;
UPDATE scripts SET type=1,command=TRIM(SUBSTRING(command FROM 5)) WHERE SUBSTRING(command FROM 1 FOR 4)='IPMI';
DELETE FROM scripts WHERE usrgrpid IS NOT NULL AND NOT EXISTS (SELECT 1 FROM usrgrp WHERE usrgrp.usrgrpid=scripts.usrgrpid);
DELETE FROM scripts WHERE groupid IS NOT NULL AND NOT EXISTS (SELECT 1 FROM groups WHERE groups.groupid=scripts.groupid);
ALTER TABLE ONLY scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE ONLY scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE ONLY service_alarms ALTER servicealarmid DROP DEFAULT,
				ALTER serviceid DROP DEFAULT;
DELETE FROM service_alarms WHERE NOT EXISTS (SELECT 1 FROM services WHERE services.serviceid=service_alarms.serviceid);
ALTER TABLE ONLY service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE ONLY services_links ALTER linkid DROP DEFAULT,
				ALTER serviceupid DROP DEFAULT,
				ALTER servicedownid DROP DEFAULT;
DELETE FROM services_links WHERE NOT EXISTS (SELECT 1 FROM services WHERE services.serviceid=services_links.serviceupid);
DELETE FROM services_links WHERE NOT EXISTS (SELECT 1 FROM services WHERE services.serviceid=services_links.servicedownid);
ALTER TABLE ONLY services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE ONLY services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE;
UPDATE services SET triggerid = NULL WHERE NOT EXISTS (SELECT 1 FROM triggers t WHERE t.triggerid = services.triggerid);
ALTER TABLE ONLY services ALTER serviceid DROP DEFAULT;
ALTER TABLE ONLY services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE ONLY services_times ALTER timeid DROP DEFAULT,
				ALTER serviceid DROP DEFAULT;
DELETE FROM services_times WHERE NOT EXISTS (SELECT 1 FROM services WHERE services.serviceid=services_times.serviceid);
ALTER TABLE ONLY services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE ONLY sessions ALTER userid DROP DEFAULT;
DELETE FROM sessions WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=sessions.userid);
ALTER TABLE ONLY sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE ONLY slideshows ALTER slideshowid DROP DEFAULT;
ALTER TABLE ONLY slides ALTER slideid DROP DEFAULT,
			ALTER slideshowid DROP DEFAULT,
			ALTER screenid DROP DEFAULT;
DELETE FROM slides WHERE NOT EXISTS (SELECT 1 FROM slideshows WHERE slideshows.slideshowid=slides.slideshowid);
DELETE FROM slides WHERE NOT EXISTS (SELECT 1 FROM screens WHERE screens.screenid=slides.screenid);
ALTER TABLE ONLY slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE ONLY slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
-- See sysmaps_elements.sql
CREATE TABLE sysmap_element_url (
	sysmapelementurlid       bigint                                    NOT NULL,
	selementid               bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (sysmapelementurlid)
);
CREATE UNIQUE INDEX sysmap_element_url_1 on sysmap_element_url (selementid,name);
ALTER TABLE ONLY sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;

INSERT INTO sysmap_element_url (sysmapelementurlid,selementid,name,url)
	SELECT selementid,selementid,url,url FROM sysmaps_elements WHERE url<>'';

ALTER TABLE ONLY sysmaps_elements
	ALTER selementid DROP DEFAULT,
	ALTER sysmapid DROP DEFAULT,
	ALTER iconid_off DROP DEFAULT,
	ALTER iconid_off DROP NOT NULL,
	ALTER iconid_on DROP DEFAULT,
	ALTER iconid_on DROP NOT NULL,
	DROP COLUMN iconid_unknown,
	ALTER iconid_disabled DROP DEFAULT,
	ALTER iconid_disabled DROP NOT NULL,
	ALTER iconid_maintenance DROP DEFAULT,
	ALTER iconid_maintenance DROP NOT NULL,
	DROP COLUMN url,
	ADD elementsubtype integer DEFAULT '0' NOT NULL,
	ADD areatype integer DEFAULT '0' NOT NULL,
	ADD width integer DEFAULT '200' NOT NULL,
	ADD height integer DEFAULT '200' NOT NULL,
	ADD viewtype integer DEFAULT '0' NOT NULL,
	ADD use_iconmap integer DEFAULT '1' NOT NULL;

DELETE FROM sysmaps_elements WHERE NOT EXISTS (SELECT 1 FROM sysmaps WHERE sysmaps.sysmapid=sysmaps_elements.sysmapid);
UPDATE sysmaps_elements SET iconid_off=NULL WHERE iconid_off=0;
UPDATE sysmaps_elements SET iconid_on=NULL WHERE iconid_on=0;
UPDATE sysmaps_elements SET iconid_disabled=NULL WHERE iconid_disabled=0;
UPDATE sysmaps_elements SET iconid_maintenance=NULL WHERE iconid_maintenance=0;
UPDATE sysmaps_elements SET iconid_off=NULL WHERE iconid_off IS NOT NULL AND NOT EXISTS (SELECT imageid FROM images WHERE images.imagetype=1 and images.imageid=sysmaps_elements.iconid_off );
UPDATE sysmaps_elements SET iconid_on=NULL WHERE iconid_on IS NOT NULL AND NOT EXISTS (SELECT imageid FROM images WHERE images.imagetype=1 and images.imageid=sysmaps_elements.iconid_on);
UPDATE sysmaps_elements SET iconid_disabled=NULL WHERE iconid_disabled IS NOT NULL AND NOT EXISTS (SELECT imageid FROM images WHERE images.imagetype=1 and images.imageid=sysmaps_elements.iconid_disabled);
UPDATE sysmaps_elements SET iconid_maintenance=NULL WHERE iconid_maintenance IS NOT NULL AND NOT EXISTS (SELECT imageid FROM images WHERE images.imagetype=1 and images.imageid=sysmaps_elements.iconid_maintenance);
ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_2 FOREIGN KEY (iconid_off) REFERENCES images (imageid);
ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_3 FOREIGN KEY (iconid_on) REFERENCES images (imageid);
ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_4 FOREIGN KEY (iconid_disabled) REFERENCES images (imageid);
ALTER TABLE ONLY sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_5 FOREIGN KEY (iconid_maintenance) REFERENCES images (imageid);
ALTER TABLE ONLY sysmaps_links ALTER linkid DROP DEFAULT,
			       ALTER sysmapid DROP DEFAULT,
			       ALTER selementid1 DROP DEFAULT,
			       ALTER selementid2 DROP DEFAULT;
DELETE FROM sysmaps_links WHERE NOT EXISTS (SELECT 1 FROM sysmaps WHERE sysmaps.sysmapid=sysmaps_links.sysmapid);
DELETE FROM sysmaps_links WHERE NOT EXISTS (SELECT 1 FROM sysmaps_elements WHERE sysmaps_elements.selementid=sysmaps_links.selementid1);
DELETE FROM sysmaps_links WHERE NOT EXISTS (SELECT 1 FROM sysmaps_elements WHERE sysmaps_elements.selementid=sysmaps_links.selementid2);
ALTER TABLE ONLY sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE ONLY sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE ONLY sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE ONLY sysmaps_link_triggers ALTER linktriggerid DROP DEFAULT,
				       ALTER linkid DROP DEFAULT,
				       ALTER triggerid DROP DEFAULT;
DELETE FROM sysmaps_link_triggers WHERE NOT EXISTS (SELECT 1 FROM sysmaps_links WHERE sysmaps_links.linkid=sysmaps_link_triggers.linkid);
DELETE FROM sysmaps_link_triggers WHERE NOT EXISTS (SELECT 1 FROM triggers WHERE triggers.triggerid=sysmaps_link_triggers.triggerid);
ALTER TABLE ONLY sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE;
ALTER TABLE ONLY sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE ONLY sysmaps
	ALTER sysmapid DROP DEFAULT,
	ALTER width SET DEFAULT '600',
	ALTER height SET DEFAULT '400',
	ALTER backgroundid DROP DEFAULT,
	ALTER backgroundid DROP NOT NULL,
	ALTER label_type SET DEFAULT '2',
	ALTER label_location SET DEFAULT '3',
	ADD expandproblem integer DEFAULT '1' NOT NULL,
	ADD markelements integer DEFAULT '0' NOT NULL,
	ADD show_unack integer DEFAULT '0' NOT NULL,
	ADD grid_size integer DEFAULT '50' NOT NULL,
	ADD grid_show integer DEFAULT '1' NOT NULL,
	ADD grid_align integer DEFAULT '1' NOT NULL,
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
	ADD iconmapid bigint NULL,
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
ALTER TABLE ONLY sysmaps ADD CONSTRAINT c_sysmaps_1 FOREIGN KEY (backgroundid) REFERENCES images (imageid);
ALTER TABLE ONLY sysmaps ADD CONSTRAINT c_sysmaps_2 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid);
CREATE TABLE sysmap_url (
	sysmapurlid              bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	elementtype              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapurlid)
);
CREATE UNIQUE INDEX sysmap_url_1 on sysmap_url (sysmapid,name);
ALTER TABLE ONLY sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE ONLY timeperiods ALTER timeperiodid DROP DEFAULT;
ALTER TABLE ONLY trends ALTER itemid DROP DEFAULT;
ALTER TABLE ONLY trends_uint ALTER itemid DROP DEFAULT;
ALTER TABLE ONLY trigger_depends ALTER triggerdepid DROP DEFAULT,
				 ALTER triggerid_down DROP DEFAULT,
				 ALTER triggerid_up DROP DEFAULT;
DROP INDEX trigger_depends_1;
DELETE FROM trigger_depends WHERE NOT EXISTS (SELECT 1 FROM triggers WHERE triggers.triggerid=trigger_depends.triggerid_down);
DELETE FROM trigger_depends WHERE NOT EXISTS (SELECT 1 FROM triggers WHERE triggers.triggerid=trigger_depends.triggerid_up);
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
ALTER TABLE ONLY trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE ONLY trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE;
CREATE TABLE trigger_discovery (
	triggerdiscoveryid       bigint                                    NOT NULL,
	triggerid                bigint                                    NOT NULL,
	parent_triggerid         bigint                                    NOT NULL,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (triggerdiscoveryid)
);
CREATE UNIQUE INDEX trigger_discovery_1 on trigger_discovery (triggerid,parent_triggerid);
ALTER TABLE ONLY trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE ONLY trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
----
---- Patching table `events`
----

DROP INDEX events_2;
CREATE INDEX events_2 on events (clock);
ALTER TABLE ONLY events ALTER eventid DROP DEFAULT,
			ADD ns integer DEFAULT '0' NOT NULL,
			ADD value_changed integer DEFAULT '0' NOT NULL;

-- Begin event redesign patch

CREATE LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION zbx_convert_events() RETURNS BOOLEAN AS $$
	DECLARE prev_triggerid bigint;
	DECLARE prev_value integer;
	r RECORD;
BEGIN
	FOR r IN
		SELECT e.eventid, t.triggerid, e.value, t.type
		FROM events e
			JOIN triggers t ON t.triggerid = e.objectid
		WHERE e.source = 0
			AND e.object = 0
			AND e.value IN (0, 1)
		ORDER BY e.objectid, e.clock, e.eventid
	LOOP

	IF prev_triggerid IS NULL OR prev_triggerid <> r.triggerid THEN
		prev_value := NULL;
	END IF;

	IF r.value = 0 THEN
		IF prev_value IS NULL OR prev_value = 1 THEN
			UPDATE events set value_changed = 1 WHERE eventid = r.eventid;
		END IF;
	ELSE
		IF r.type = 1 OR prev_value IS NULL OR prev_value = 0 THEN
			UPDATE events set value_changed = 1 WHERE eventid = r.eventid;
		END IF;
	END IF;

	prev_value := r.value;
	prev_triggerid := r.triggerid;

	END LOOP;

	RETURN 1;
END;
$$ LANGUAGE plpgsql;

SELECT zbx_convert_events();

DROP FUNCTION zbx_convert_events();

-- End event redesign patch

----
---- Patching table `triggers`
----

ALTER TABLE ONLY triggers ALTER triggerid DROP DEFAULT,
			  ALTER templateid DROP DEFAULT,
			  ALTER templateid DROP NOT NULL,
			  DROP COLUMN dep_level,
			  ADD value_flags integer DEFAULT '0' NOT NULL,
			  ADD flags integer DEFAULT '0' NOT NULL;
UPDATE triggers SET templateid=NULL WHERE templateid=0;
UPDATE triggers SET templateid=NULL WHERE templateid IS NOT NULL AND NOT EXISTS (SELECT 1 FROM triggers t WHERE t.triggerid=triggers.templateid);
ALTER TABLE ONLY triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE;

-- Begin event redesign patch

CREATE TEMPORARY TABLE tmp_triggers (triggerid bigint PRIMARY KEY, eventid bigint);

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
	);

UPDATE triggers
	SET value=0,					-- TRIGGER_VALUE_FALSE
		value_flags=1
	WHERE value=2;					-- TRIGGER_VALUE_UNKNOWN

DROP TABLE tmp_triggers;

-- End event redesign patch
ALTER TABLE ONLY user_history ALTER userhistoryid DROP DEFAULT,
			      ALTER userid DROP DEFAULT;
DELETE FROM user_history WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=user_history.userid);
ALTER TABLE ONLY user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE ONLY users_groups ALTER id DROP DEFAULT,
			      ALTER usrgrpid DROP DEFAULT,
			      ALTER userid DROP DEFAULT;
DELETE FROM users_groups WHERE NOT EXISTS (SELECT 1 FROM usrgrp WHERE usrgrp.usrgrpid=users_groups.usrgrpid);
DELETE FROM users_groups WHERE NOT EXISTS (SELECT 1 FROM users WHERE users.userid=users_groups.userid);

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
ALTER TABLE ONLY users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE ONLY users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE ONLY users ALTER userid DROP DEFAULT,
	ALTER COLUMN lang SET DEFAULT 'en_GB',
	ALTER COLUMN theme SET DEFAULT 'default';
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
ALTER TABLE ONLY usrgrp ALTER usrgrpid DROP DEFAULT,
			DROP COLUMN api_access;
ALTER TABLE ONLY valuemaps ALTER valuemapid DROP DEFAULT;
