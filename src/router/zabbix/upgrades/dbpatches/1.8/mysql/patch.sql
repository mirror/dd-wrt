CREATE INDEX actions_1 on actions (eventsource,status);
CREATE TABLE auditlog_details (
      auditdetailid           bigint unsigned         DEFAULT '0'     NOT NULL,
      auditid         bigint unsigned         DEFAULT '0'     NOT NULL,
      table_name              varchar(64)             DEFAULT ''      NOT NULL,
      field_name              varchar(64)             DEFAULT ''      NOT NULL,
      oldvalue                blob                    NOT NULL,
      newvalue                blob                    NOT NULL,
      PRIMARY KEY (auditdetailid)
) ENGINE=InnoDB;
CREATE INDEX auditlog_details_1 on auditlog_details (auditid);
alter table auditlog add ip              varchar(39)             DEFAULT ''      NOT NULL;
alter table auditlog add resourceid              bigint unsigned         DEFAULT '0'     NOT NULL;
alter table auditlog add resourcename            varchar(255)            DEFAULT ''      NOT NULL;
CREATE TABLE autoreg_host (
      autoreg_hostid          bigint unsigned         DEFAULT '0'     NOT NULL,
      proxy_hostid            bigint unsigned         DEFAULT '0'     NOT NULL,
      host            varchar(64)             DEFAULT ''      NOT NULL,
      PRIMARY KEY (autoreg_hostid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX autoreg_host_1 on autoreg_host (proxy_hostid,host);
alter table config add dropdown_first_entry integer DEFAULT '1' NOT NULL;
alter table config add dropdown_first_remember integer DEFAULT '1' NOT NULL;
alter table config add discovery_groupid bigint unsigned DEFAULT '0' NOT NULL;
alter table config add max_in_table integer DEFAULT '50' NOT NULL;
alter table config add search_limit integer DEFAULT '1000' NOT NULL;
alter table dchecks add snmpv3_securityname varchar(64) DEFAULT '' NOT NULL;
alter table dchecks add snmpv3_securitylevel integer DEFAULT '0' NOT NULL;
alter table dchecks add snmpv3_authpassphrase varchar(64) DEFAULT '' NOT NULL;
alter table dchecks add snmpv3_privpassphrase varchar(64) DEFAULT '' NOT NULL;

CREATE INDEX dchecks_1 on dchecks (druleid);
alter table dservices add dcheckid                bigint unsigned         DEFAULT '0'     NOT NULL;
alter table dservices add ip              varchar(39)             DEFAULT ''      NOT NULL;

update dservices set ip=(select dhosts.ip from dhosts where dservices.dhostid=dhosts.dhostid);

alter table dhosts drop ip;

CREATE INDEX dhosts_1 on dhosts (druleid);
alter table drules add unique_dcheckid bigint unsigned DEFAULT '0' NOT NULL;
-- See also dhosts.sql

CREATE UNIQUE INDEX dservices_1 on dservices (dcheckid,type,key_,ip,port);
CREATE INDEX dservices_2 on dservices (dhostid);
CREATE INDEX escalations_2 on escalations (status,nextcheck);
alter table events DROP INDEX events_2;
CREATE INDEX events_2 on events (clock, objectid);
CREATE TABLE expressions (
      expressionid            bigint unsigned         DEFAULT '0'     NOT NULL,
      regexpid                bigint unsigned         DEFAULT '0'     NOT NULL,
      expression              varchar(255)            DEFAULT ''      NOT NULL,
      expression_type         integer         DEFAULT '0'     NOT NULL,
      exp_delimiter           varchar(1)              DEFAULT ''      NOT NULL,
      case_sensitive          integer         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (expressionid)
) ENGINE=InnoDB;
CREATE INDEX expressions_1 on expressions (regexpid);
CREATE TABLE globalmacro (
      globalmacroid           bigint unsigned         DEFAULT '0'     NOT NULL,
      macro           varchar(64)             DEFAULT ''      NOT NULL,
      value           varchar(255)            DEFAULT ''      NOT NULL,
      PRIMARY KEY (globalmacroid)
) ENGINE=InnoDB;
CREATE INDEX globalmacro_1 on globalmacro (macro);
alter table graphs_items change color color varchar(6) DEFAULT '009600' NOT NULL;

CREATE INDEX graphs_items_1 on graphs_items (itemid);
CREATE INDEX graphs_items_2 on graphs_items (graphid);
alter table graphs add ymin_type               integer         DEFAULT '0'     NOT NULL;
alter table graphs add ymax_type               integer         DEFAULT '0'     NOT NULL;
alter table graphs add ymin_itemid             bigint unsigned         DEFAULT '0'     NOT NULL;
alter table graphs add ymax_itemid             bigint unsigned         DEFAULT '0'     NOT NULL;

update graphs set ymin_type=yaxistype;
update graphs set ymax_type=yaxistype;

alter table graphs drop yaxistype;
CREATE TABLE graph_theme (
      graphthemeid            bigint unsigned         DEFAULT '0'     NOT NULL,
      description             varchar(64)             DEFAULT ''      NOT NULL,
      theme           varchar(64)             DEFAULT ''      NOT NULL,
      backgroundcolor         varchar(6)              DEFAULT 'F0F0F0'        NOT NULL,
      graphcolor              varchar(6)              DEFAULT 'FFFFFF'        NOT NULL,
      graphbordercolor                varchar(6)              DEFAULT '222222'        NOT NULL,
      gridcolor               varchar(6)              DEFAULT 'CCCCCC'        NOT NULL,
      maingridcolor           varchar(6)              DEFAULT 'AAAAAA'        NOT NULL,
      gridbordercolor         varchar(6)              DEFAULT '000000'        NOT NULL,
      textcolor               varchar(6)              DEFAULT '202020'        NOT NULL,
      highlightcolor          varchar(6)              DEFAULT 'AA4444'        NOT NULL,
      leftpercentilecolor             varchar(6)              DEFAULT '11CC11'        NOT NULL,
      rightpercentilecolor            varchar(6)              DEFAULT 'CC1111'        NOT NULL,
      noneworktimecolor               varchar(6)              DEFAULT 'E0E0E0'        NOT NULL,
      gridview                integer         DEFAULT 1       NOT NULL,
      legendview              integer         DEFAULT 1       NOT NULL,
      PRIMARY KEY (graphthemeid)
) ENGINE=InnoDB;
CREATE INDEX graph_theme_1 on graph_theme (description);
CREATE INDEX graph_theme_2 on graph_theme (theme);

INSERT INTO graph_theme VALUES (1,'Original Blue','css_ob.css','F0F0F0','FFFFFF','333333','CCCCCC','AAAAAA','000000','222222','AA4444','11CC11','CC1111','E0E0E0',1,1);
INSERT INTO graph_theme VALUES (2,'Black & Blue','css_bb.css','333333','0A0A0A','888888','222222','4F4F4F','EFEFEF','0088FF','CC4444','1111FF','FF1111','1F1F1F',1,1);
alter table groups add internal                integer         DEFAULT '0'     NOT NULL;
drop table help_items;

CREATE TABLE help_items (
        itemtype                integer         DEFAULT '0'     NOT NULL,
        key_            varchar(255)            DEFAULT ''      NOT NULL,
        description             varchar(255)            DEFAULT ''      NOT NULL,
        PRIMARY KEY (itemtype,key_)
) ENGINE=InnoDB;

insert into help_items values (3,'icmpping[&lt;ip&gt;,&lt;count&gt;,&lt;interval&gt;,&lt;size&gt;,&lt;timeout&gt;]','Checks if server is accessible by ICMP ping. 0 - ICMP ping fails. 1 - ICMP ping successful. One of zabbix_server processes performs ICMP pings once per PingerFrequency seconds.');
insert into help_items values (3,'icmppingloss[&lt;ip&gt;,&lt;count&gt;,&lt;interval&gt;,&lt;size&gt;,&lt;timeout&gt;]','Returns percentage of lost ICMP ping packets.');
insert into help_items values (3,'icmppingsec[&lt;ip&gt;,&lt;count&gt;,&lt;interval&gt;,&lt;size&gt;,&lt;timeout&gt;,&lt;type&gt;]','Returns ICMP ping response time in seconds. Example: 0.02');
insert into help_items values (3,'ftp&lt;,port&gt;','Checks if FTP server is running and accepting connections. 0 - FTP server is down. 1 - FTP server is running.');
insert into help_items values (3,'http&lt;,port&gt;','Checks if HTTP (web) server is running and accepting connections. 0 - HTTP server is down. 1 - HTTP server is running.');
insert into help_items values (3,'imap&lt;,port&gt;','Checks if IMAP server is running and accepting connections. 0 - IMAP server is down. 1 - IMAP server is running.');
insert into help_items values (3,'ldap&lt;,port&gt;','Checks if LDAP server is running and accepting connections. 0 - LDAP server is down. 1 - LDAP server is running.');
insert into help_items values (3,'nntp&lt;,port&gt;','Checks if NNTP server is running and accepting connections. 0 - NNTP server is down. 1 - NNTP server is running.');
insert into help_items values (3,'ntp&lt;,port&gt;','Checks if NTP server is running and accepting connections. 0 - NTP server is down. 1 - NTP server is running.');
insert into help_items values (3,'pop&lt;,port&gt;','Checks if POP server is running and accepting connections. 0 - POP server is down. 1 - POP server is running.');
insert into help_items values (3,'smtp&lt;,port&gt;','Checks if SMTP server is running and accepting connections. 0 - SMTP server is down. 1 - SMTP server is running.');
insert into help_items values (3,'ssh&lt;,port&gt;','Checks if SSH server is running and accepting connections. 0 - SSH server is down. 1 - SSH server is running.');
insert into help_items values (3,'tcp,port','Checks if TCP service is running and accepting connections on port. 0 - the service on the port is down. 1 - the service is running.');
insert into help_items values (3,'ftp_perf&lt;,port&gt;','Checks if FTP server is running and accepting connections. 0 - FTP server is down. Otherwise, number of seconds spent connecting to FTP server.');
insert into help_items values (3,'http_perf&lt;,port&gt;','Checks if HTTP (web) server is running and accepting connections. 0 - HTTP server is down. Otherwise, number of seconds spent connecting to HTTP server.');
insert into help_items values (3,'imap_perf&lt;,port&gt;','Checks if IMAP server is running and accepting connections. 0 - IMAP server is down. Otherwise, number of seconds spent connecting to IMAP server.');
insert into help_items values (3,'ldap_perf&lt;,port&gt;','Checks if LDAP server is running and accepting connections. 0 - LDAP server is down. Otherwise, number of seconds spent connecting to LDAP server.');
insert into help_items values (3,'nntp_perf&lt;,port&gt;','Checks if NNTP server is running and accepting connections. 0 - NNTP server is down. Otherwise, number of seconds spent connecting to NNTP server.');
insert into help_items values (3,'ntp_perf&lt;,port&gt;','Checks if NTP server is running and accepting connections. 0 - NTP server is down. Otherwise, number of seconds spent connecting to NTP server.');
insert into help_items values (3,'pop_perf&lt;,port&gt;','Checks if POP server is running and accepting connections. 0 - POP server is down. Otherwise, number of milliseconds spent connecting to POP server.');
insert into help_items values (3,'smtp_perf&lt;,port&gt;','Checks if SMTP server is running and accepting connections. 0 - SMTP server is down. Otherwise, number of seconds spent connecting to SMTP server.');
insert into help_items values (3,'ssh_perf&lt;,port&gt;','Checks if SSH server is running and accepting connections. 0 - SSH server is down. Otherwise, number of seconds spent connecting to SSH server.');
insert into help_items values (3,'tcp_perf,port','Checks if TCP service is running and accepting connections on port. 0 - the service on the port is down. Otherwise, number of seconds spent connecting to TCP service.');

insert into help_items values (5,'zabbix[boottime]','Startup time of Zabbix server, Unix timestamp.');
insert into help_items values (5,'zabbix[history]','Number of values stored in table HISTORY.');
insert into help_items values (5,'zabbix[history_log]','Number of values stored in table HISTORY_LOG.');
insert into help_items values (5,'zabbix[history_str]','Number of values stored in table HISTORY_STR.');
insert into help_items values (5,'zabbix[history_text]','Number of values stored in table HISTORY_TEXT.');
insert into help_items values (5,'zabbix[history_uint]','Number of values stored in table HISTORY_UINT.');
insert into help_items values (5,'zabbix[items]','Number of items in Zabbix database.');
insert into help_items values (5,'zabbix[items_unsupported]','Number of unsupported items in Zabbix database.');
insert into help_items values (5,'zabbix[log]','Stores warning and error messages generated by Zabbix server.');
insert into help_items values (5,'zabbix[proxy,&lt;name&gt;,&lt;param&gt;]','Time of proxy last access. Name - proxy name. Param - lastaccess. Unix timestamp.');
insert into help_items values (5,'zabbix[queue&lt;,from&gt;&lt;,to&gt;]','Number of items in the queue which are delayed by from to to seconds, inclusive.');
insert into help_items values (5,'zabbix[rcache,&lt;cache&gt;,&lt;mode&gt;]','Configuration cache statistics. Cache - buffer (modes: pfree, total, used, free).');
insert into help_items values (5,'zabbix[trends]','Number of values stored in table TRENDS.');
insert into help_items values (5,'zabbix[trends_uint]','Number of values stored in table TRENDS_UINT.');
insert into help_items values (5,'zabbix[triggers]','Number of triggers in Zabbix database.');
insert into help_items values (5,'zabbix[uptime]','Uptime of Zabbix server process in seconds.');
insert into help_items values (5,'zabbix[wcache,&lt;cache&gt;,&lt;mode&gt;]','Data cache statistics. Cache - one of values (modes: all, float, uint, str, log, text), history (modes: pfree, total, used, free), trend (modes: pfree, total, used, free), text (modes: pfree, total, used, free).');
insert into help_items values (5,'zabbix[process,&lt;type&gt;,&lt;num&gt;,&lt;state&gt;]','Time a particular Zabbix process or a group of processes (identified by &lt;type&gt; and &lt;num&gt;) spent in &lt;state&gt; in percentage.');

insert into help_items values (8,'grpfunc[&lt;Group&gt;,&lt;Key&gt;,&lt;func&gt;,&lt;param&gt;]','Aggregate checks do not require any agent running on a host being monitored. Zabbix server collects aggregate information by doing direct database queries. See Zabbix Manual.');

insert into help_items values(0,'agent.ping','Check the agent usability. Always return 1. Can be used as a TCP ping.');
insert into help_items values(0,'agent.version','Version of zabbix_agent(d) running on monitored host. String value. Example of returned value: 1.1');
insert into help_items values(0,'kernel.maxfiles','Maximum number of opened files supported by OS.');
insert into help_items values(0,'kernel.maxproc','Maximum number of processes supported by OS.');
insert into help_items values(0,'net.if.collisions[if]','Out-of-window collision. Collisions count.');
insert into help_items values(0,'net.if.in[if &lt;,mode&gt;]','Network interface input statistic. Integer value. If mode is missing bytes is used.');
insert into help_items values(0,'net.if.out[if &lt;,mode&gt;]','Network interface output statistic. Integer value. If mode is missing bytes is used.');
insert into help_items values(0,'net.if.total[if &lt;,mode&gt;]','Sum of network interface incoming and outgoing statistics. Integer value. Mode - one of bytes (default), packets, errors or dropped');
insert into help_items values(0,'net.if.list','List of network interfaces. Text value.');
insert into help_items values(0,'net.tcp.dns[ip, zone]','Checks if DNS service is up. 0 - DNS is down, 1 - DNS is up.');
insert into help_items values(0,'net.tcp.dns.query[ip, zone, type]','Performs a query for the record type specified by the parameter type');
insert into help_items values(0,'net.tcp.listen[port]','Checks if this port is in LISTEN state. 0 - it is not, 1 - it is in LISTEN state.');
insert into help_items values(0,'net.tcp.port[&lt;ip&gt;, port]','Check, if it is possible to make TCP connection to the port number. 0 - cannot connect, 1 - can connect. IP address is optional. If ip is missing, 127.0.0.1 is used. Example: net.tcp.port[,80]');
insert into help_items values(0,'net.tcp.service[service &lt;,ip&gt; &lt;,port&gt;]','Check if service server is running and accepting connections. 0 - service is down, 1 - service is running. If ip is missing 127.0.0.1 is used. If port number is missing, default service port is used. Example: net.tcp.service[ftp,,45].');
insert into help_items values(0,'net.tcp.service.perf[service &lt;,ip&gt; &lt;,port&gt;]','Check performance of service &quot;service&quot;. 0 - service is down, sec - number of seconds spent on connection to the service. If ip is missing 127.0.0.1 is used.  If port number is missing, default service port is used.');
insert into help_items values(0,'proc.mem[&lt;name&gt; &lt;,user&gt; &lt;,mode&gt; &lt;,cmdline&gt;]','Memory used by process with name name running under user user. Memory used by processes. Process name, user and mode is optional. If name or user is missing all processes will be calculated. If mode is missing sum is used. Example: proc.mem[,root]');
insert into help_items values(0,'proc.num[&lt;name&gt; &lt;,user&gt; &lt;,state&gt; &lt;,cmdline&gt;]','Number of processes with name name running under user user having state state. Process name, user and state are optional. Examples: proc.num[,mysql]; proc.num[apache2,www-data]; proc.num[,oracle,sleep,oracleZABBIX]');
insert into help_items values(0,'system.cpu.intr','Device interrupts.');
insert into help_items values(0,'system.cpu.load[&lt;cpu&gt; &lt;,mode&gt;]','CPU(s) load. Processor load. The cpu and mode are optional. If cpu is missing all is used. If mode is missing avg1 is used. Note that this is not percentage.');
insert into help_items values(0,'system.cpu.switches','Context switches.');
insert into help_items values(0,'system.cpu.util[&lt;cpu&gt; &lt;,type&gt; &lt;,mode&gt;]','CPU(s) utilisation. Processor load in percents. The cpu, type and mode are optional. If cpu is missing all is used.  If type is missing user is used. If mode is missing avg1 is used.');
insert into help_items values(0,'system.boottime','Timestamp of system boot.');
insert into help_items values(0,'system.cpu.num','Number of available proccessors.');
insert into help_items values(0,'system.hostname[&lt;type&gt;]','Returns hostname (or NetBIOS name (by default) on Windows). String value. Example of returned value: www.zabbix.com');
insert into help_items values(0,'system.localtime','System local time. Time in seconds.');
insert into help_items values(0,'system.run[command,&lt;mode&gt]','Run specified command on the host.');
insert into help_items values(0,'system.swap.in[&lt;swap&gt; &lt;,type&gt;]','Swap in. If type is count - swapins is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
insert into help_items values(0,'system.swap.out[&lt;swap&gt; &lt;,type&gt;]','Swap out. If type is count - swapouts is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
insert into help_items values(0,'system.swap.size[&lt;swap&gt; &lt;,mode&gt;]','Swap space. Number of bytes. If swap is missing all is used. If mode is missing free is used.');
insert into help_items values(0,'system.uname','Returns detailed host information. String value');
insert into help_items values(0,'system.uptime','System uptime in seconds.');
insert into help_items values(0,'system.users.num','Number of users connected. Command who is used on agent side.');
insert into help_items values(0,'vfs.dev.read[device &lt;,type&gt; &lt;,mode&gt;]','Device read statistics.');
insert into help_items values(0,'vfs.dev.write[device &lt;,type&gt; &lt;,mode&gt;]','Device write statistics.');
insert into help_items values(0,'vfs.file.cksum[file]','Calculate check sum of a given file. Check sum of the file calculate by standard algorithm used by UNIX utility cksum. Example: vfs.file.cksum[/etc/passwd]');
insert into help_items values(0,'vfs.file.exists[file]','Check file existence. 0 - file does not exist, 1 - file exists');
insert into help_items values(0,'vfs.file.md5sum[file]','Calculate MD5 check sum of a given file. String MD5 hash of the file. Can be used for files less than 64MB, unsupported otherwise. Example: vfs.file.md5sum[/etc/zabbix/zabbix_agentd.conf]');
insert into help_items values(0,'vfs.file.regexp[file,regexp]','Find string in a file. Matched string');
insert into help_items values(0,'vfs.file.regmatch[file,regexp]','Find string in a file. 0 - expression not found, 1 - found');
insert into help_items values(0,'vfs.file.size[file]','Size of a given file. Size in bytes. File must have read permissions for user zabbix. Example: vfs.file.size[/var/log/syslog]');
insert into help_items values(0,'vfs.file.time[file &lt;,mode&gt;]','File time information. Number of seconds.	The mode is optional. If mode is missing modify is used.');
insert into help_items values(0,'vfs.fs.inode[fs &lt;,mode&gt;]','Number of inodes for a given volume. If mode is missing total is used.');
insert into help_items values(0,'vfs.fs.size[fs &lt;,mode&gt;]','Calculate disk space for a given volume. Disk space in KB. If mode is missing total is used.  In case of mounted volume, unused disk space for local file system is returned. Example: vfs.fs.size[/tmp,free].');
insert into help_items values(0,'vm.memory.size[&lt;mode&gt;]','Amount of memory size in bytes. If mode is missing total is used.');
insert into help_items values(0,'web.page.get[host,&lt;path&gt;,&lt;port&gt;]','Get content of web page. Default path is /');
insert into help_items values(0,'web.page.perf[host,&lt;path&gt;,&lt;port&gt;]','Get timing of loading full web page. Default path is /');
insert into help_items values(0,'web.page.regexp[host,&lt;path&gt;,&lt;port&gt;,&lt;regexp&gt;,&lt;length&gt;]','Get first occurrence of regexp in web page. Default path is /');
insert into help_items values(0,'perf_counter[counter, interval]','Value of any performance counter, where "counter" parameter is the counter path and "interval" parameter is a number of last seconds, for which the agent returns an average value.');
insert into help_items values(0,'service_state[service]','State of service. 0 - running, 1 - paused, 2 - start pending, 3 - pause pending, 4 - continue pending, 5 - stop pending, 6 - stopped, 7 - unknown, 255 - no such service');
insert into help_items values(0,'proc_info[&lt;process&gt;,&lt;attribute&gt;,&lt;type&gt;]','Different information about specific process(es)');
insert into help_items values(0,'system.stat[resource &lt;,type&gt;]','Virtual memory statistics.');

insert into help_items values(7,'agent.ping','Check the agent usability. Always return 1. Can be used as a TCP ping.');
insert into help_items values(7,'agent.version','Version of zabbix_agent(d) running on monitored host. String value. Example of returned value: 1.1');
insert into help_items values(7,'kernel.maxfiles','Maximum number of opened files supported by OS.');
insert into help_items values(7,'kernel.maxproc','Maximum number of processes supported by OS.');
insert into help_items values(7,'net.if.collisions[if]','Out-of-window collision. Collisions count.');
insert into help_items values(7,'net.if.in[if &lt;,mode&gt;]','Network interface input statistic. Integer value. If mode is missing bytes is used.');
insert into help_items values(7,'net.if.out[if &lt;,mode&gt;]','Network interface output statistic. Integer value. If mode is missing bytes is used.');
insert into help_items values(7,'net.if.total[if &lt;,mode&gt;]','Sum of network interface incoming and outgoing statistics. Integer value. Mode - one of bytes (default), packets, errors or dropped');
insert into help_items values(7,'net.if.list','List of network interfaces. Text value.');
insert into help_items values(7,'net.tcp.dns[ip, zone]','Checks if DNS service is up. 0 - DNS is down, 1 - DNS is up.');
insert into help_items values(7,'net.tcp.dns.query[ip, zone, type]','Performs a query for the record type specified by the parameter type');
insert into help_items values(7,'net.tcp.listen[port]','Checks if this port is in LISTEN state. 0 - it is not, 1 - it is in LISTEN state.');
insert into help_items values(7,'net.tcp.port[&lt;ip&gt;, port]','Check, if it is possible to make TCP connection to the port number. 0 - cannot connect, 1 - can connect. IP address is optional. If ip is missing, 127.0.0.1 is used. Example: net.tcp.port[,80]');
insert into help_items values(7,'net.tcp.service[service &lt;,ip&gt; &lt;,port&gt;]','Check if service server is running and accepting connections. 0 - service is down, 1 - service is running. If ip is missing 127.0.0.1 is used. If port number is missing, default service port is used. Example: net.tcp.service[ftp,,45].');
insert into help_items values(7,'net.tcp.service.perf[service &lt;,ip&gt; &lt;,port&gt;]','Check performance of service &quot;service&quot;. 0 - service is down, sec - number of seconds spent on connection to the service. If ip is missing 127.0.0.1 is used.  If port number is missing, default service port is used.');
insert into help_items values(7,'proc.mem[&lt;name&gt; &lt;,user&gt; &lt;,mode&gt; &lt;,cmdline&gt;]','Memory used by process with name name running under user user. Memory used by processes. Process name, user and mode is optional. If name or user is missing all processes will be calculated. If mode is missing sum is used. Example: proc.mem[,root]');
insert into help_items values(7,'proc.num[&lt;name&gt; &lt;,user&gt; &lt;,state&gt; &lt;,cmdline&gt;]','Number of processes with name name running under user user having state state. Process name, user and state are optional. Examples: proc.num[,mysql]; proc.num[apache2,www-data]; proc.num[,oracle,sleep,oracleZABBIX]');
insert into help_items values(7,'system.cpu.intr','Device interrupts.');
insert into help_items values(7,'system.cpu.load[&lt;cpu&gt; &lt;,mode&gt;]','CPU(s) load. Processor load. The cpu and mode are optional. If cpu is missing all is used. If mode is missing avg1 is used. Note that this is not percentage.');
insert into help_items values(7,'system.cpu.switches','Context switches.');
insert into help_items values(7,'system.cpu.util[&lt;cpu&gt; &lt;,type&gt; &lt;,mode&gt;]','CPU(s) utilisation. Processor load in percents. The cpu, type and mode are optional. If cpu is missing all is used.  If type is missing user is used. If mode is missing avg1 is used.');
insert into help_items values(7,'system.boottime','Timestamp of system boot.');
insert into help_items values(7,'system.cpu.num','Number of available proccessors.');
insert into help_items values(7,'system.hostname[&lt;type&gt;]','Returns hostname (or NetBIOS name (by default) on Windows). String value. Example of returned value: www.zabbix.com');
insert into help_items values(7,'system.localtime','System local time. Time in seconds.');
insert into help_items values(7,'system.run[command,&lt;mode&gt]','Run specified command on the host.');
insert into help_items values(7,'system.swap.in[&lt;swap&gt; &lt;,type&gt;]','Swap in. If type is count - swapins is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
insert into help_items values(7,'system.swap.out[&lt;swap&gt; &lt;,type&gt;]','Swap out. If type is count - swapouts is returned. If type is pages - pages swapped in is returned. If swap is missing all is used.');
insert into help_items values(7,'system.swap.size[&lt;swap&gt; &lt;,mode&gt;]','Swap space. Number of bytes. If swap is missing all is used. If mode is missing free is used.');
insert into help_items values(7,'system.uname','Returns detailed host information. String value');
insert into help_items values(7,'system.uptime','System uptime in seconds.');
insert into help_items values(7,'system.users.num','Number of users connected. Command who is used on agent side.');
insert into help_items values(7,'vfs.dev.read[device &lt;,type&gt; &lt;,mode&gt;]','Device read statistics.');
insert into help_items values(7,'vfs.dev.write[device &lt;,type&gt; &lt;,mode&gt;]','Device write statistics.');
insert into help_items values(7,'vfs.file.cksum[file]','Calculate check sum of a given file. Check sum of the file calculate by standard algorithm used by UNIX utility cksum. Example: vfs.file.cksum[/etc/passwd]');
insert into help_items values(7,'vfs.file.exists[file]','Check file existence. 0 - file does not exist, 1 - file exists');
insert into help_items values(7,'vfs.file.md5sum[file]','Calculate MD5 check sum of a given file. String MD5 hash of the file. Can be used for files less than 64MB, unsupported otherwise. Example: vfs.file.md5sum[/etc/zabbix/zabbix_agentd.conf]');
insert into help_items values(7,'vfs.file.regexp[file,regexp]','Find string in a file. Matched string');
insert into help_items values(7,'vfs.file.regmatch[file,regexp]','Find string in a file. 0 - expression not found, 1 - found');
insert into help_items values(7,'vfs.file.size[file]','Size of a given file. Size in bytes. File must have read permissions for user zabbix. Example: vfs.file.size[/var/log/syslog]');
insert into help_items values(7,'vfs.file.time[file&lt;, mode&gt;]','File time information. Number of seconds.	The mode is optional. If mode is missing modify is used.');
insert into help_items values(7,'vfs.fs.inode[fs &lt;,mode&gt;]','Number of inodes for a given volume. If mode is missing total is used.');
insert into help_items values(7,'vfs.fs.size[fs &lt;,mode&gt;]','Calculate disk space for a given volume. Disk space in KB. If mode is missing total is used.  In case of mounted volume, unused disk space for local file system is returned. Example: vfs.fs.size[/tmp,free].');
insert into help_items values(7,'vm.memory.size[&lt;mode&gt;]','Amount of memory size in bytes. If mode is missing total is used.');
insert into help_items values(7,'web.page.get[host,&lt;path&gt;,&lt;port&gt;]','Get content of web page. Default path is /');
insert into help_items values(7,'web.page.perf[host,&lt;path&gt;,&lt;port&gt;]','Get timing of loading full web page. Default path is /');
insert into help_items values(7,'web.page.regexp[host,&lt;path&gt;,&lt;port&gt;,&lt;regexp&gt;,&lt;length&gt;]','Get first occurrence of regexp in web page. Default path is /');
insert into help_items values(7,'perf_counter[counter]','Value of any performance counter, where parameter is the counter path.');
insert into help_items values(7,'service_state[service]','State of service. 0 - running, 1 - paused, 2 - start pending, 3 - pause pending, 4 - continue pending, 5 - stop pending, 6 - stopped, 7 - unknown, 255 - no such service');
insert into help_items values(7,'proc_info[&lt;process&gt;,&lt;attribute&gt;,&lt;type&gt;]','Different information about specific process(es)');
insert into help_items values(7,'log[file,&lt;pattern&gt;,&lt;encoding&gt;,&lt;maxlines&gt;]','Monitoring of log file. pattern - regular expression');
insert into help_items values(7,'logrt[file_format,&lt;pattern&gt;,&lt;encoding&gt;,&lt;maxlines&gt;]', 'Monitoring of log file with rotation. fileformat - [path][regexp], pattern - regular expression');
insert into help_items values(7,'eventlog[logtype,&lt;pattern&gt;,&lt;severity&gt;,&lt;source&gt;,&lt;eventid&gt;,&lt;maxlines&gt;]','Monitoring of Windows event logs. pattern, severity, eventid - regular expressions');
insert into help_items values(7,'system.stat[resource &lt;,type&gt;]','Virtual memory statistics.');
alter table history_log add logeventid              integer         DEFAULT '0'     NOT NULL;

CREATE UNIQUE INDEX history_log_2 on history_log (itemid,id);
CREATE UNIQUE INDEX history_text_2 on history_text (itemid,id);
CREATE TABLE hostmacro (
      hostmacroid             bigint unsigned         DEFAULT '0'     NOT NULL,
      hostid          bigint unsigned         DEFAULT '0'     NOT NULL,
      macro           varchar(64)             DEFAULT ''      NOT NULL,
      value           varchar(255)            DEFAULT ''      NOT NULL,
      PRIMARY KEY (hostmacroid)
) ENGINE=InnoDB;
CREATE INDEX hostmacro_1 on hostmacro (hostid,macro);
alter table hosts_groups drop index hosts_groups_groups_1;
CREATE INDEX hosts_groups_1 on hosts_groups (hostid,groupid);
CREATE INDEX hosts_groups_2 on hosts_groups (groupid);
alter table hosts add maintenanceid bigint unsigned DEFAULT '0' NOT NULL;
alter table hosts add maintenance_status integer DEFAULT '0' NOT NULL;
alter table hosts add maintenance_type integer DEFAULT '0' NOT NULL;
alter table hosts add maintenance_from integer DEFAULT '0' NOT NULL;
alter table hosts add ipmi_ip varchar(64) DEFAULT '127.0.0.1' NOT NULL;
alter table hosts add ipmi_errors_from integer DEFAULT '0' NOT NULL;
alter table hosts add snmp_errors_from integer DEFAULT '0' NOT NULL;
alter table hosts add ipmi_error varchar(128) DEFAULT '' NOT NULL;
alter table hosts add snmp_error varchar(128) DEFAULT '' NOT NULL;
CREATE INDEX hosts_templates_2 on hosts_templates (templateid);
alter table httptest add authentication          integer         DEFAULT '0'     NOT NULL;
alter table httptest add http_user               varchar(64)             DEFAULT ''      NOT NULL;
alter table httptest add http_password           varchar(64)             DEFAULT ''      NOT NULL;

CREATE INDEX httptest_2 on httptest (name);
CREATE INDEX httptest_3 on httptest (status);
alter table items drop nextcheck;
alter table items add data_type  integer     DEFAULT '0' NOT NULL;
alter table items add authtype   integer     DEFAULT '0' NOT NULL;
alter table items add username   varchar(64) DEFAULT ''  NOT NULL;
alter table items add password   varchar(64) DEFAULT ''  NOT NULL;
alter table items add publickey  varchar(64) DEFAULT ''  NOT NULL;
alter table items add privatekey varchar(64) DEFAULT ''  NOT NULL;
alter table items add mtime      integer     DEFAULT '0' NOT NULL;

UPDATE items SET units='Bps' WHERE type=9 AND units='bps';
CREATE TABLE maintenances_groups (
      maintenance_groupid             bigint unsigned         DEFAULT '0'     NOT NULL,
      maintenanceid           bigint unsigned         DEFAULT '0'     NOT NULL,
      groupid         bigint unsigned         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (maintenance_groupid)
) ENGINE=InnoDB;
CREATE INDEX maintenances_groups_1 on maintenances_groups (maintenanceid,groupid);
CREATE TABLE maintenances_hosts (
      maintenance_hostid              bigint unsigned         DEFAULT '0'     NOT NULL,
      maintenanceid           bigint unsigned         DEFAULT '0'     NOT NULL,
      hostid          bigint unsigned         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (maintenance_hostid)
) ENGINE=InnoDB;
CREATE INDEX maintenances_hosts_1 on maintenances_hosts (maintenanceid,hostid);
CREATE TABLE maintenances (
      maintenanceid           bigint unsigned         DEFAULT '0'     NOT NULL,
      name            varchar(128)            DEFAULT ''      NOT NULL,
      maintenance_type                integer         DEFAULT '0'     NOT NULL,
      description             blob                    NOT NULL,
      active_since            integer         DEFAULT '0'     NOT NULL,
      active_till             integer         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (maintenanceid)
) ENGINE=InnoDB;
CREATE INDEX maintenances_1 on maintenances (active_since,active_till);
CREATE TABLE maintenances_windows (
      maintenance_timeperiodid                bigint unsigned         DEFAULT '0'     NOT NULL,
      maintenanceid           bigint unsigned         DEFAULT '0'     NOT NULL,
      timeperiodid            bigint unsigned         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (maintenance_timeperiodid)
) ENGINE=InnoDB;
CREATE INDEX maintenances_windows_1 on maintenances_windows (maintenanceid,timeperiodid);
DROP TABLE node_cksum;
CREATE TABLE node_cksum (
	nodeid		integer		DEFAULT '0'	NOT NULL,
	tablename		varchar(64)		DEFAULT ''	NOT NULL,
	recordid		bigint unsigned		DEFAULT '0'	NOT NULL,
	cksumtype		integer		DEFAULT '0'	NOT NULL,
	cksum		text			NOT NULL,
	sync		char(128)		DEFAULT ''	NOT NULL
) ENGINE=InnoDB;
CREATE INDEX node_cksum_1 on node_cksum (nodeid,cksumtype,tablename,recordid);
CREATE TABLE opmediatypes (
      opmediatypeid           bigint unsigned         DEFAULT '0'     NOT NULL,
      operationid             bigint unsigned         DEFAULT '0'     NOT NULL,
      mediatypeid             bigint unsigned         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (opmediatypeid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX opmediatypes_1 on opmediatypes (operationid);
CREATE INDEX profiles_2 on profiles (userid,profileid);
CREATE TABLE proxy_autoreg_host (
      id              bigint unsigned                 NOT NULL        auto_increment unique,
      clock           integer         DEFAULT '0'     NOT NULL,
      host            varchar(64)             DEFAULT ''      NOT NULL,
      PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE INDEX proxy_autoreg_host_1 on proxy_autoreg_host (clock);
alter table proxy_dhistory change key_ key_ varchar(255) DEFAULT '' NOT NULL;
alter table proxy_dhistory change value value varchar(255) DEFAULT '' NOT NULL;

alter table proxy_dhistory add dcheckid bigint unsigned DEFAULT '0' NOT NULL;
alter table proxy_history add logeventid              integer         DEFAULT '0'     NOT NULL;
CREATE TABLE regexps (
      regexpid                bigint unsigned         DEFAULT '0'     NOT NULL,
      name            varchar(128)            DEFAULT ''      NOT NULL,
      test_string             blob                    NOT NULL,
      PRIMARY KEY (regexpid)
) ENGINE=InnoDB;
CREATE INDEX regexps_1 on regexps (name);
CREATE INDEX rights_2 on rights (id);
CREATE INDEX services_1 on services (triggerid);
CREATE INDEX sessions_1 on sessions (userid, status);
alter table sysmaps_elements change label label varchar(255) DEFAULT '' NOT NULL;

ALTER TABLE sysmaps_elements ADD iconid_maintenance BIGINT unsigned DEFAULT '0' NOT NULL;
alter table sysmaps_links  add label           varchar(255)            DEFAULT ''      NOT NULL;
ALTER TABLE sysmaps ADD highlight INTEGER DEFAULT '1' NOT NULL;
CREATE TABLE timeperiods (
      timeperiodid            bigint unsigned         DEFAULT '0'     NOT NULL,
      timeperiod_type         integer         DEFAULT '0'     NOT NULL,
      every           integer         DEFAULT '0'     NOT NULL,
      month           integer         DEFAULT '0'     NOT NULL,
      dayofweek               integer         DEFAULT '0'     NOT NULL,
      day             integer         DEFAULT '0'     NOT NULL,
      start_time              integer         DEFAULT '0'     NOT NULL,
      period          integer         DEFAULT '0'     NOT NULL,
      start_date              integer         DEFAULT '0'     NOT NULL,
      PRIMARY KEY (timeperiodid)
) ENGINE=InnoDB;
CREATE TABLE user_history (
      userhistoryid           bigint unsigned         DEFAULT '0'     NOT NULL,
      userid          bigint unsigned         DEFAULT '0'     NOT NULL,
      title1          varchar(255)            DEFAULT ''      NOT NULL,
      url1            varchar(255)            DEFAULT ''      NOT NULL,
      title2          varchar(255)            DEFAULT ''      NOT NULL,
      url2            varchar(255)            DEFAULT ''      NOT NULL,
      title3          varchar(255)            DEFAULT ''      NOT NULL,
      url3            varchar(255)            DEFAULT ''      NOT NULL,
      title4          varchar(255)            DEFAULT ''      NOT NULL,
      url4            varchar(255)            DEFAULT ''      NOT NULL,
      title5          varchar(255)            DEFAULT ''      NOT NULL,
      url5            varchar(255)            DEFAULT ''      NOT NULL,
      PRIMARY KEY (userhistoryid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX user_history_1 on user_history (userid);
alter table users add rows_per_page           integer         DEFAULT 50      NOT NULL;
alter table usrgrp add api_access              integer         DEFAULT '0'     NOT NULL;
alter table usrgrp add debug_mode              integer         DEFAULT '0'     NOT NULL;
