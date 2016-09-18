alter table acknowledges modify message         nvarchar2(255)          DEFAULT '';
CREATE INDEX actions_1 on actions (eventsource,status);

alter table actions modify name            nvarchar2(255)          DEFAULT '';
alter table actions modify def_shortdata           nvarchar2(255)          DEFAULT '';
alter table actions modify def_longdata            nvarchar2(2048)         DEFAULT '';
alter table actions modify r_shortdata             nvarchar2(255)          DEFAULT '';
alter table actions modify r_longdata              nvarchar2(2048)         DEFAULT '';

alter table alerts modify sendto          nvarchar2(100)          DEFAULT '';
alter table alerts modify subject         nvarchar2(255)          DEFAULT '';
alter table alerts modify message         nvarchar2(2048)         DEFAULT '';
alter table alerts modify error           nvarchar2(128)          DEFAULT '';
alter table applications modify name            nvarchar2(255)          DEFAULT '';
CREATE TABLE auditlog_details (
        auditdetailid           number(20)              DEFAULT '0'     NOT NULL,
        auditid         number(20)              DEFAULT '0'     NOT NULL,
        table_name              nvarchar2(64)           DEFAULT ''      ,
        field_name              nvarchar2(64)           DEFAULT ''      ,
        oldvalue                nvarchar2(2048)         DEFAULT ''      ,
        newvalue                nvarchar2(2048)         DEFAULT ''      ,
        PRIMARY KEY (auditdetailid)
);
CREATE INDEX auditlog_details_1 on auditlog_details (auditid);
alter table auditlog add ip nvarchar2(39)           DEFAULT '';
alter table auditlog add resourceid              number(20)              DEFAULT '0'     NOT NULL;
alter table auditlog add resourcename            nvarchar2(255)          DEFAULT '';

alter table auditlog modify details         nvarchar2(128)          DEFAULT '0';
CREATE TABLE autoreg_host (
        autoreg_hostid          number(20)              DEFAULT '0'     NOT NULL,
        proxy_hostid            number(20)              DEFAULT '0'     NOT NULL,
        host            nvarchar2(64)           DEFAULT ''      ,
        PRIMARY KEY (autoreg_hostid)
);
CREATE UNIQUE INDEX autoreg_host_1 on autoreg_host (proxy_hostid,host);
alter table conditions modify value           nvarchar2(255)          DEFAULT '';
alter table config add dropdown_first_entry number(10) DEFAULT '1' NOT NULL;
alter table config add dropdown_first_remember number(10) DEFAULT '1' NOT NULL;
alter table config add discovery_groupid number(20) DEFAULT '0' NOT NULL;
alter table config add max_in_table number(10) DEFAULT '50' NOT NULL;
alter table config add search_limit number(10) DEFAULT '1000' NOT NULL;


alter table config modify work_period             nvarchar2(100)          DEFAULT '1-5,00:00-24:00';
alter table config modify default_theme           nvarchar2(128)          DEFAULT 'default.css';
alter table config modify ldap_host               nvarchar2(255)          DEFAULT '';
alter table config modify ldap_base_dn            nvarchar2(255)          DEFAULT '';
alter table config modify ldap_bind_dn            nvarchar2(255)          DEFAULT '';
alter table config modify ldap_bind_password              nvarchar2(128)          DEFAULT '';
alter table config modify ldap_search_attribute           nvarchar2(128)          DEFAULT '';
alter table dchecks add snmpv3_securityname             nvarchar2(64)           DEFAULT '';
alter table dchecks add snmpv3_securitylevel            number(10)              DEFAULT '0'     NOT NULL;
alter table dchecks add snmpv3_authpassphrase           nvarchar2(64)           DEFAULT '';
alter table dchecks add snmpv3_privpassphrase           nvarchar2(64)           DEFAULT '';

CREATE INDEX dchecks_1 on dchecks (druleid);

alter table dchecks modify key_            nvarchar2(255)          DEFAULT '0';
alter table dchecks modify snmp_community          nvarchar2(255)          DEFAULT '0';
alter table dchecks modify ports           nvarchar2(255)          DEFAULT '0';
alter table dservices add dcheckid                number(20)              DEFAULT '0'     NOT NULL;
alter table dservices add ip              nvarchar2(39)           DEFAULT '';

update dservices set ip=(select dhosts.ip from dhosts where dservices.dhostid=dhosts.dhostid);

alter table dhosts drop column ip;

CREATE INDEX dhosts_1 on dhosts (druleid);

alter table drules add unique_dcheckid number(20) DEFAULT '0' NOT NULL;

alter table drules modify name            nvarchar2(255)          DEFAULT '';
alter table drules modify iprange         nvarchar2(255)          DEFAULT '';
-- See also dhosts.sql

CREATE UNIQUE INDEX dservices_1 on dservices (dcheckid,type,key_,ip,port);
CREATE INDEX dservices_2 on dservices (dhostid);

alter table dservices modify key_            nvarchar2(255)          DEFAULT '0';
alter table dservices modify value           nvarchar2(255)          DEFAULT '0';
alter table dservices modify ip              nvarchar2(39)           DEFAULT '';

CREATE INDEX escalations_2 on escalations (status,nextcheck);
DROP INDEX events_2;
CREATE INDEX events_2 on events (clock, objectid);
CREATE TABLE expressions (
        expressionid            number(20)              DEFAULT '0'     NOT NULL,
        regexpid                number(20)              DEFAULT '0'     NOT NULL,
        expression              nvarchar2(255)          DEFAULT ''      ,
        expression_type         number(10)              DEFAULT '0'     NOT NULL,
        exp_delimiter           nvarchar2(1)            DEFAULT ''      ,
        case_sensitive          number(10)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (expressionid)
);
CREATE INDEX expressions_1 on expressions (regexpid);

alter table functions modify lastvalue               nvarchar2(255);
alter table functions modify function                nvarchar2(12);
alter table functions modify parameter               nvarchar2(255);

CREATE TABLE globalmacro (
        globalmacroid           number(20)              DEFAULT '0'     NOT NULL,
        macro           nvarchar2(64)           DEFAULT ''      ,
        value           nvarchar2(255)          DEFAULT ''      ,
        PRIMARY KEY (globalmacroid)
);
CREATE INDEX globalmacro_1 on globalmacro (macro);
alter table graphs_items modify color           nvarchar2(6)            DEFAULT '009600';

CREATE INDEX graphs_items_1 on graphs_items (itemid);
CREATE INDEX graphs_items_2 on graphs_items (graphid);

alter table graphs_items modify color           nvarchar2(6)            DEFAULT '009600';
alter table graphs add ymin_type               number(10)              DEFAULT '0'     NOT NULL;
alter table graphs add ymax_type               number(10)              DEFAULT '0'     NOT NULL;
alter table graphs add ymin_itemid             number(20)              DEFAULT '0'     NOT NULL;
alter table graphs add ymax_itemid             number(20)              DEFAULT '0'     NOT NULL;

update graphs set ymin_type=yaxistype;
update graphs set ymax_type=yaxistype;

alter table graphs drop column yaxistype;

alter table graphs modify name            nvarchar2(128)          DEFAULT '';
CREATE TABLE graph_theme (
        graphthemeid            number(20)              DEFAULT '0'     NOT NULL,
        description             nvarchar2(64)           DEFAULT ''      ,
        theme           nvarchar2(64)           DEFAULT ''      ,
        backgroundcolor         nvarchar2(6)            DEFAULT 'F0F0F0'        ,
        graphcolor              nvarchar2(6)            DEFAULT 'FFFFFF'        ,
        graphbordercolor                nvarchar2(6)            DEFAULT '222222'        ,
        gridcolor               nvarchar2(6)            DEFAULT 'CCCCCC'        ,
        maingridcolor           nvarchar2(6)            DEFAULT 'AAAAAA'        ,
        gridbordercolor         nvarchar2(6)            DEFAULT '000000'        ,
        textcolor               nvarchar2(6)            DEFAULT '202020'        ,
        highlightcolor          nvarchar2(6)            DEFAULT 'AA4444'        ,
        leftpercentilecolor             nvarchar2(6)            DEFAULT '11CC11'        ,
        rightpercentilecolor            nvarchar2(6)            DEFAULT 'CC1111'        ,
        noneworktimecolor               nvarchar2(6)            DEFAULT 'E0E0E0'        ,
        gridview                number(10)              DEFAULT 1       NOT NULL,
        legendview              number(10)              DEFAULT 1       NOT NULL,
        PRIMARY KEY (graphthemeid)
);
CREATE INDEX graph_theme_1 on graph_theme (description);
CREATE INDEX graph_theme_2 on graph_theme (theme);

INSERT INTO graph_theme VALUES (1,'Original Blue','css_ob.css','F0F0F0','FFFFFF','333333','CCCCCC','AAAAAA','000000','222222','AA4444','11CC11','CC1111','E0E0E0',1,1);
INSERT INTO graph_theme VALUES (2,'Black & Blue','css_bb.css','333333','0A0A0A','888888','222222','4F4F4F','EFEFEF','0088FF','CC4444','1111FF','FF1111','1F1F1F',1,1);
alter table groups add internal                number(10)         DEFAULT '0'     NOT NULL;

alter table groups modify name            nvarchar2(64)           DEFAULT '';
drop table help_items;

CREATE TABLE help_items (
        itemtype                number(10)              DEFAULT '0'     NOT NULL,
        key_            nvarchar2(255)          DEFAULT ''      ,
        description             nvarchar2(255)          DEFAULT ''      ,
        PRIMARY KEY (itemtype,key_)
);

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
alter table history_log add logeventid              number(10)         DEFAULT '0'     NOT NULL;

CREATE TABLE history_log_tmp (
        id              number(20)              DEFAULT '0'     NOT NULL,
        itemid          number(20)              DEFAULT '0'     NOT NULL,
        clock           number(10)              DEFAULT '0'     NOT NULL,
        timestamp               number(10)              DEFAULT '0'     NOT NULL,
        source          nvarchar2(64)           DEFAULT ''      ,
        severity                number(10)              DEFAULT '0'     NOT NULL,
        value           nclob           DEFAULT ''      ,
        logeventid              number(10)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (id)
);


insert into history_log_tmp select * from history_log;
drop table history_log;

alter table history_log_tmp rename to history_log;

CREATE INDEX history_log_1 on history_log (itemid,clock);
CREATE UNIQUE INDEX history_log_2 on history_log (itemid,id);

alter table history_str modify value           nvarchar2(255)          DEFAULT '';
alter table history_str_sync modify value           nvarchar2(255)          DEFAULT '';

CREATE SEQUENCE history_str_sync_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/

CREATE TRIGGER history_str_sync_tr
BEFORE INSERT ON history_str_sync
FOR EACH ROW
BEGIN
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;
END;
/

CREATE SEQUENCE history_sync_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/

CREATE TRIGGER history_sync_tr
BEFORE INSERT ON history_sync
FOR EACH ROW
BEGIN
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;
END;
/

CREATE TABLE history_text_tmp (
        id              number(20)              DEFAULT '0'     NOT NULL,
        itemid          number(20)              DEFAULT '0'     NOT NULL,
        clock           number(10)              DEFAULT '0'     NOT NULL,
        value           nclob           DEFAULT ''      ,
        PRIMARY KEY (id)
);

insert into history_text_tmp select * from history_text;
drop table history_text;

alter table history_text_tmp rename to history_text;

CREATE INDEX history_text_1 on history_text (itemid,clock);
CREATE UNIQUE INDEX history_text_2 on history_text (itemid,id);
CREATE SEQUENCE history_uint_sync_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/

CREATE TRIGGER history_uint_sync_tr
BEFORE INSERT ON history_uint_sync
FOR EACH ROW
BEGIN
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;
END;
/

CREATE TABLE hostmacro (
        hostmacroid             number(20)              DEFAULT '0'     NOT NULL,
        hostid          number(20)              DEFAULT '0'     NOT NULL,
        macro           nvarchar2(64)           DEFAULT ''      ,
        value           nvarchar2(255)          DEFAULT ''      ,
        PRIMARY KEY (hostmacroid)
);
CREATE INDEX hostmacro_1 on hostmacro (hostid,macro);

drop index hosts_groups_groups_1;
CREATE INDEX hosts_groups_1 on hosts_groups (hostid,groupid);
CREATE INDEX hosts_groups_2 on hosts_groups (groupid);
alter table hosts_profiles_ext modify device_alias            nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_type             nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_chassis          nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_os               nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_os_short         nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_hw_arch          nvarchar2(32)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_serial           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_model            nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_tag              nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_vendor           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_contract         nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_who              nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_status           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_app_01           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_app_02           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_app_03           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_app_04           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_app_05           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify device_url_1            nvarchar2(255)          DEFAULT ''  ;
alter table hosts_profiles_ext modify device_url_2            nvarchar2(255)          DEFAULT ''  ;
alter table hosts_profiles_ext modify device_url_3            nvarchar2(255)          DEFAULT ''  ;
alter table hosts_profiles_ext modify device_networks         nvarchar2(2048)         DEFAULT ''  ;
alter table hosts_profiles_ext modify device_notes            nvarchar2(2048)         DEFAULT ''  ;
alter table hosts_profiles_ext modify device_hardware         nvarchar2(2048)         DEFAULT ''  ;
alter table hosts_profiles_ext modify device_software         nvarchar2(2048)         DEFAULT ''  ;
alter table hosts_profiles_ext modify ip_subnet_mask          nvarchar2(39)           DEFAULT ''  ;
alter table hosts_profiles_ext modify ip_router               nvarchar2(39)           DEFAULT ''  ;
alter table hosts_profiles_ext modify ip_macaddress           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify oob_ip          nvarchar2(39)           DEFAULT ''  ;
alter table hosts_profiles_ext modify oob_subnet_mask         nvarchar2(39)           DEFAULT ''  ;
alter table hosts_profiles_ext modify oob_router              nvarchar2(39)           DEFAULT ''  ;
alter table hosts_profiles_ext modify date_hw_buy             nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify date_hw_install         nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify date_hw_expiry          nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify date_hw_decomm          nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify site_street_1           nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify site_street_2           nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify site_street_3           nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify site_city               nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify site_state              nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify site_country            nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify site_zip                nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify site_rack               nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify site_notes              nvarchar2(2048)         DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_name              nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_email             nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_phone_1           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_phone_2           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_cell              nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_screen            nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_1_notes             nvarchar2(2048)         DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_name              nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_email             nvarchar2(128)          DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_phone_1           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_phone_2           nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_cell              nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_screen            nvarchar2(64)           DEFAULT ''  ;
alter table hosts_profiles_ext modify poc_2_notes             nvarchar2(2048)         DEFAULT ''  ;

alter table hosts_profiles modify devicetype              nvarchar2(64)           DEFAULT '';
alter table hosts_profiles modify name            nvarchar2(64)           DEFAULT '';
alter table hosts_profiles modify os              nvarchar2(64)           DEFAULT '';
alter table hosts_profiles modify serialno                nvarchar2(64)           DEFAULT '';
alter table hosts_profiles modify tag             nvarchar2(64)           DEFAULT '';
alter table hosts_profiles modify macaddress              nvarchar2(64)           DEFAULT '';
alter table hosts_profiles modify hardware                nvarchar2(2048)         DEFAULT '';
alter table hosts_profiles modify software                nvarchar2(2048)         DEFAULT '';
alter table hosts_profiles modify contact         nvarchar2(2048)         DEFAULT '';
alter table hosts_profiles modify location                nvarchar2(2048)         DEFAULT '';
alter table hosts_profiles modify notes           nvarchar2(2048)         DEFAULT '';
alter table hosts add maintenanceid number(20) DEFAULT '0' NOT NULL;
alter table hosts add maintenance_status number(10) DEFAULT '0' NOT NULL;
alter table hosts add maintenance_type number(10) DEFAULT '0' NOT NULL;
alter table hosts add maintenance_from number(10) DEFAULT '0' NOT NULL;
alter table hosts add ipmi_ip nvarchar2(64) DEFAULT '127.0.0.1';
alter table hosts add ipmi_errors_from number(10) DEFAULT '0' NOT NULL;
alter table hosts add snmp_errors_from number(10) DEFAULT '0' NOT NULL;
alter table hosts add ipmi_error nvarchar2(128) DEFAULT '';
alter table hosts add snmp_error nvarchar2(128) DEFAULT '';

alter table hosts modify host            nvarchar2(64)           DEFAULT '';
alter table hosts modify dns             nvarchar2(64)           DEFAULT '';
alter table hosts modify ip              nvarchar2(39)           DEFAULT '127.0.0.1';
alter table hosts modify error           nvarchar2(128)          DEFAULT '';
alter table hosts modify ipmi_username           nvarchar2(16)           DEFAULT '';
alter table hosts modify ipmi_password           nvarchar2(20)           DEFAULT '';
alter table hosts modify ipmi_ip         nvarchar2(64)           DEFAULT '127.0.0.1';

CREATE INDEX hosts_templates_2 on hosts_templates (templateid);
alter table housekeeper modify tablename               nvarchar2(64)           DEFAULT '';
alter table housekeeper modify field           nvarchar2(64)           DEFAULT '';
alter table httpstep modify name            nvarchar2(64)           DEFAULT '';
alter table httpstep modify url             nvarchar2(255)          DEFAULT '';
alter table httpstep modify posts           nvarchar2(2048)         DEFAULT '';
alter table httpstep modify required                nvarchar2(255)          DEFAULT '';
alter table httpstep modify status_codes            nvarchar2(255)          DEFAULT '';
alter table httptest add authentication          number(10)         DEFAULT '0'     NOT NULL;
alter table httptest add http_user               nvarchar2(64)             DEFAULT '';
alter table httptest add http_password           nvarchar2(64)             DEFAULT '';

CREATE INDEX httptest_2 on httptest (name);
CREATE INDEX httptest_3 on httptest (status);

alter table httptest modify name            nvarchar2(64)           DEFAULT '';
alter table httptest modify macros          nvarchar2(2048)         DEFAULT '';
alter table httptest modify agent           nvarchar2(255)          DEFAULT '';
alter table httptest modify error           nvarchar2(255)          DEFAULT '';
alter table ids modify table_name              nvarchar2(64)           DEFAULT '';
alter table ids modify field_name              nvarchar2(64)           DEFAULT '';

alter table images modify name            nvarchar2(64)           DEFAULT '0';

alter table items drop column nextcheck;
alter table items add data_type  number(10)     DEFAULT '0' NOT NULL;
alter table items add authtype   number(10)     DEFAULT '0' NOT NULL;
alter table items add username   nvarchar2(64) DEFAULT '';
alter table items add password   nvarchar2(64) DEFAULT '';
alter table items add publickey  nvarchar2(64) DEFAULT '';
alter table items add privatekey nvarchar2(64) DEFAULT '';
alter table items add mtime      number(10)     DEFAULT '0' NOT NULL;

alter table items modify snmp_community          nvarchar2(64)           DEFAULT '';
alter table items modify snmp_oid                nvarchar2(255)          DEFAULT '';
alter table items modify description             nvarchar2(255)          DEFAULT '';
alter table items modify key_            nvarchar2(255)          DEFAULT '';
alter table items modify lastvalue               nvarchar2(255);
alter table items modify prevvalue               nvarchar2(255);
alter table items modify trapper_hosts           nvarchar2(255);
alter table items modify units           nvarchar2(10);
alter table items modify prevorgvalue            nvarchar2(255);
alter table items modify snmpv3_securityname             nvarchar2(64);
alter table items modify snmpv3_authpassphrase           nvarchar2(64);
alter table items modify snmpv3_privpassphrase           nvarchar2(64);
alter table items modify formula         nvarchar2(255)          DEFAULT '1';
alter table items modify error           nvarchar2(128)          DEFAULT '';
alter table items modify logtimefmt              nvarchar2(64)           DEFAULT '';
alter table items modify delay_flex              nvarchar2(255)          DEFAULT '';
alter table items modify params          nvarchar2(2048)         DEFAULT '';
alter table items modify ipmi_sensor             nvarchar2(128)          DEFAULT '';

UPDATE items SET units='Bps' WHERE type=9 AND units='bps';
CREATE TABLE maintenances_groups (
        maintenance_groupid             number(20)              DEFAULT '0'     NOT NULL,
        maintenanceid           number(20)              DEFAULT '0'     NOT NULL,
        groupid         number(20)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (maintenance_groupid)
);
CREATE INDEX maintenances_groups_1 on maintenances_groups (maintenanceid,groupid);

CREATE TABLE maintenances_hosts (
        maintenance_hostid              number(20)              DEFAULT '0'     NOT NULL,
        maintenanceid           number(20)              DEFAULT '0'     NOT NULL,
        hostid          number(20)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (maintenance_hostid)
);
CREATE INDEX maintenances_hosts_1 on maintenances_hosts (maintenanceid,hostid);

CREATE TABLE maintenances (
        maintenanceid           number(20)              DEFAULT '0'     NOT NULL,
        name            nvarchar2(128)          DEFAULT ''      ,
        maintenance_type                number(10)              DEFAULT '0'     NOT NULL,
        description             nvarchar2(2048)         DEFAULT ''      ,
        active_since            number(10)              DEFAULT '0'     NOT NULL,
        active_till             number(10)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (maintenanceid)
);
CREATE INDEX maintenances_1 on maintenances (active_since,active_till);

CREATE TABLE maintenances_windows (
        maintenance_timeperiodid                number(20)              DEFAULT '0'     NOT NULL,
        maintenanceid           number(20)              DEFAULT '0'     NOT NULL,
        timeperiodid            number(20)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (maintenance_timeperiodid)
);
CREATE INDEX maintenances_windows_1 on maintenances_windows (maintenanceid,timeperiodid);

alter table mappings modify value           nvarchar2(64)           DEFAULT '';
alter table mappings modify newvalue                nvarchar2(64)           DEFAULT '';

alter table media modify sendto          nvarchar2(100)          DEFAULT '';
alter table media modify period          nvarchar2(100)          DEFAULT '1-7,00:00-23:59';

alter table media_type modify description             nvarchar2(100)          DEFAULT '';
alter table media_type modify smtp_server             nvarchar2(255)          DEFAULT '';
alter table media_type modify smtp_helo               nvarchar2(255)          DEFAULT '';
alter table media_type modify smtp_email              nvarchar2(255)          DEFAULT '';
alter table media_type modify exec_path               nvarchar2(255)          DEFAULT '';
alter table media_type modify gsm_modem               nvarchar2(255)          DEFAULT '';
alter table media_type modify username                nvarchar2(255)          DEFAULT '';
alter table media_type modify passwd          nvarchar2(255)          DEFAULT '';

DROP TABLE node_cksum;
CREATE TABLE node_cksum (
	nodeid		number(10)		DEFAULT '0'	NOT NULL,
	tablename		nvarchar2(64)		DEFAULT ''	,
	recordid		number(20)		DEFAULT '0'	NOT NULL,
	cksumtype		number(10)		DEFAULT '0'	NOT NULL,
	cksum		nclob		DEFAULT ''	,
	sync		nvarchar2(128)		DEFAULT ''
);
CREATE INDEX node_cksum_1 on node_cksum (nodeid,cksumtype,tablename,recordid);
alter table nodes modify name            nvarchar2(64)           DEFAULT '0';
alter table nodes modify ip              nvarchar2(39)           DEFAULT '';
alter table opconditions modify value           nvarchar2(255)          DEFAULT '';
alter table operations modify shortdata               nvarchar2(255)          DEFAULT '';
alter table operations modify longdata                nvarchar2(2048)         DEFAULT '';

CREATE TABLE opmediatypes (
        opmediatypeid           number(20)              DEFAULT '0'     NOT NULL,
        operationid             number(20)              DEFAULT '0'     NOT NULL,
        mediatypeid             number(20)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (opmediatypeid)
);
CREATE UNIQUE INDEX opmediatypes_1 on opmediatypes (operationid);

CREATE INDEX profiles_2 on profiles (userid,profileid);

alter table profiles modify idx             nvarchar2(96)           DEFAULT '';
alter table profiles modify value_str               nvarchar2(255)          DEFAULT '';
alter table profiles modify source          nvarchar2(96)           DEFAULT '';

CREATE TABLE proxy_autoreg_host (
	id	number(20)			NOT NULL,
	clock	number(10)	DEFAULT '0'	NOT NULL,
	host	nvarchar2(64)	DEFAULT '',
	PRIMARY KEY (id)
)
/

CREATE INDEX proxy_autoreg_host_1 on proxy_autoreg_host (clock)
/

CREATE SEQUENCE proxy_autoreg_host_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/

CREATE TRIGGER proxy_autoreg_host_tr
BEFORE INSERT ON proxy_autoreg_host
FOR EACH ROW
BEGIN
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;
END;
/
DROP TABLE proxy_dhistory
/

CREATE TABLE proxy_dhistory (
	id		number(20)			NOT NULL,
	clock		number(10)	DEFAULT '0'	NOT NULL,
	druleid		number(20)	DEFAULT '0'	NOT NULL,
	type		number(10)	DEFAULT '0'	NOT NULL,
	ip		nvarchar2(39)	DEFAULT '',
	port		number(10)	DEFAULT '0'	NOT NULL,
	key_		nvarchar2(255)	DEFAULT '',
	value		nvarchar2(255)	DEFAULT '',
	status		number(10)	DEFAULT '0'	NOT NULL,
	dcheckid	number(20)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (id)
)
/

CREATE INDEX proxy_dhistory_1 on proxy_dhistory (clock)
/

CREATE SEQUENCE proxy_dhistory_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/

CREATE TRIGGER proxy_dhistory_tr
BEFORE INSERT ON proxy_dhistory
FOR EACH ROW
BEGIN
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;
END;
/

DROP TABLE proxy_history
/

CREATE TABLE proxy_history (
	id		number(20)			NOT NULL,
	itemid		number(20)	DEFAULT '0'	NOT NULL,
	clock		number(10)	DEFAULT '0'	NOT NULL,
	timestamp	number(10)	DEFAULT '0'	NOT NULL,
	source		nvarchar2(64)	DEFAULT '',
	severity	number(10)	DEFAULT '0'	NOT NULL,
	value		nclob		DEFAULT '',
	logeventid	number(10)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (id)
)
/

CREATE INDEX proxy_history_1 on proxy_history (clock)
/

CREATE SEQUENCE proxy_history_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/

CREATE TRIGGER proxy_history_tr
BEFORE INSERT ON proxy_history
FOR EACH ROW
BEGIN
SELECT proxy_history_seq.nextval INTO :new.id FROM dual;
END;
/

CREATE TABLE regexps (
        regexpid                number(20)              DEFAULT '0'     NOT NULL,
        name            nvarchar2(128)          DEFAULT ''      ,
        test_string             nvarchar2(2048)         DEFAULT ''      ,
        PRIMARY KEY (regexpid)
);
CREATE INDEX regexps_1 on regexps (name);

CREATE INDEX rights_2 on rights (id);
alter table screens_items modify url             nvarchar2(255)          DEFAULT '';

alter table screens modify name            nvarchar2(255)          DEFAULT 'Screen';

alter table scripts modify name            nvarchar2(255)          DEFAULT '';
alter table scripts modify command         nvarchar2(255)          DEFAULT '';

CREATE INDEX services_1 on services (triggerid);

alter table services modify name            nvarchar2(128)          DEFAULT '';

alter table services_times modify note            nvarchar2(255)          DEFAULT '';
CREATE INDEX sessions_1 on sessions (userid, status);

alter table sessions modify sessionid               nvarchar2(32)           DEFAULT '';
alter table slideshows modify name            nvarchar2(255)          DEFAULT '';
alter table sysmaps_elements  modify label           nvarchar2(255)            DEFAULT '';
ALTER TABLE sysmaps_elements ADD iconid_maintenance number(20) DEFAULT '0' NOT NULL;

alter table sysmaps_elements modify url             nvarchar2(255)          DEFAULT '';
alter table sysmaps_links  add label           nvarchar2(255)            DEFAULT '';
alter table sysmaps_links modify color           nvarchar2(6)            DEFAULT '000000';
alter table sysmaps_link_triggers modify color           nvarchar2(6)            DEFAULT '000000';
ALTER TABLE sysmaps ADD highlight number(10) DEFAULT '1' NOT NULL;

alter table sysmaps modify name            nvarchar2(128)          DEFAULT '';
CREATE TABLE timeperiods (
        timeperiodid            number(20)              DEFAULT '0'     NOT NULL,
        timeperiod_type         number(10)              DEFAULT '0'     NOT NULL,
        every           number(10)              DEFAULT '0'     NOT NULL,
        month           number(10)              DEFAULT '0'     NOT NULL,
        dayofweek               number(10)              DEFAULT '0'     NOT NULL,
        day             number(10)              DEFAULT '0'     NOT NULL,
        start_time              number(10)              DEFAULT '0'     NOT NULL,
        period          number(10)              DEFAULT '0'     NOT NULL,
        start_date              number(10)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (timeperiodid)
);
alter table triggers modify expression              nvarchar2(255)          DEFAULT '';
alter table triggers modify description             nvarchar2(255)          DEFAULT '';
alter table triggers modify url             nvarchar2(255)          DEFAULT '';
alter table triggers modify comments                nvarchar2(2048)         DEFAULT '';
alter table triggers modify error           nvarchar2(128)          DEFAULT '';
CREATE TABLE user_history (
        userhistoryid           number(20)              DEFAULT '0'     NOT NULL,
        userid          number(20)              DEFAULT '0'     NOT NULL,
        title1          nvarchar2(255)          DEFAULT ''      ,
        url1            nvarchar2(255)          DEFAULT ''      ,
        title2          nvarchar2(255)          DEFAULT ''      ,
        url2            nvarchar2(255)          DEFAULT ''      ,
        title3          nvarchar2(255)          DEFAULT ''      ,
        url3            nvarchar2(255)          DEFAULT ''      ,
        title4          nvarchar2(255)          DEFAULT ''      ,
        url4            nvarchar2(255)          DEFAULT ''      ,
        title5          nvarchar2(255)          DEFAULT ''      ,
        url5            nvarchar2(255)          DEFAULT ''      ,
        PRIMARY KEY (userhistoryid)
);
CREATE UNIQUE INDEX user_history_1 on user_history (userid);

alter table users add rows_per_page           number(10)         DEFAULT 50      NOT NULL;

alter table users modify alias           nvarchar2(100)          DEFAULT '';
alter table users modify name            nvarchar2(100)          DEFAULT '';
alter table users modify surname         nvarchar2(100)          DEFAULT '';
alter table users modify passwd          nvarchar2(32)           DEFAULT '';
alter table users modify url             nvarchar2(255)          DEFAULT '';
alter table users modify lang            nvarchar2(5)            DEFAULT 'en_gb';
alter table users modify theme           nvarchar2(128)          DEFAULT 'default.css';
alter table users modify attempt_ip              nvarchar2(39)           DEFAULT '';
alter table usrgrp add api_access              number(10)         DEFAULT '0'     NOT NULL;
alter table usrgrp add debug_mode              number(10)         DEFAULT '0'     NOT NULL;

alter table usrgrp modify name            nvarchar2(64)           DEFAULT '';
alter table valuemaps modify name            nvarchar2(64)           DEFAULT '';

