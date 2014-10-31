alter table actions add esc_period    number(10)     DEFAULT '0' NOT NULL;
alter table actions add def_shortdata varchar2(255)  DEFAULT '';
alter table actions add def_longdata  varchar2(2048) DEFAULT '';
alter table actions add recovery_msg  number(10)     DEFAULT '0' NOT NULL;
alter table actions add r_shortdata   varchar2(255)  DEFAULT '';
alter table actions add r_longdata    varchar2(2048) DEFAULT '';

CREATE INDEX actions_1 on actions (eventsource,status);
CREATE TABLE alerts_tmp (
        alertid         number(20)              DEFAULT '0'     NOT NULL,
        actionid                number(20)              DEFAULT '0'     NOT NULL,
        eventid         number(20)              DEFAULT '0'     NOT NULL,
        userid          number(20)              DEFAULT '0'     NOT NULL,
        clock           number(10)              DEFAULT '0'     NOT NULL,
        mediatypeid             number(20)              DEFAULT '0'     NOT NULL,
        sendto          varchar2(100)           DEFAULT ''      ,
        subject         varchar2(255)           DEFAULT ''      ,
        message         varchar2(2048)          DEFAULT ''      ,
        status          number(10)              DEFAULT '0'     NOT NULL,
        retries         number(10)              DEFAULT '0'     NOT NULL,
        error           varchar2(128)           DEFAULT ''      ,
        nextcheck               number(10)              DEFAULT '0'     NOT NULL,
        esc_step                number(10)              DEFAULT '0'     NOT NULL,
        alerttype               number(10)              DEFAULT '0'     NOT NULL,
        PRIMARY KEY (alertid)
);

alter table alerts add eventid bigint DEFAULT '0' NOT NULL;

update alerts a set eventid = (select min(e.eventid) from events e where e.objectid = a.triggerid and e.object = 0 and e.clock = a.clock) where a.eventid = 0;
update alerts a set eventid = (select min(e.eventid) from events e where e.objectid = a.triggerid and e.object = 0 and e.clock = a.clock + 1) where a.eventid = 0;

insert into alerts_tmp (alertid,actionid,eventid,userid,clock,mediatypeid,sendto,subject,message,status,retries,error,nextcheck) select alertid,actionid,eventid,userid,clock,mediatypeid,sendto,subject,message,status,retries,error,nextcheck from alerts;

drop table alerts;
alter table alerts_tmp rename to alerts;
update alerts set status=3 where retries>=2;

CREATE INDEX alerts_1 on alerts (actionid);
CREATE INDEX alerts_2 on alerts (clock);
CREATE INDEX alerts_3 on alerts (eventid);
CREATE INDEX alerts_4 on alerts (status,retries);
CREATE INDEX alerts_5 on alerts (mediatypeid);
CREATE INDEX alerts_6 on alerts (userid);
alter table config add event_ack_enable      number(10)    DEFAULT '1'     NOT NULL;
alter table config add event_expire          number(10)    DEFAULT '7'     NOT NULL;
alter table config add event_show_max        number(10)    DEFAULT '100'   NOT NULL;
alter table config add default_theme         varchar2(128) DEFAULT 'default.css';
alter table config add authentication_type   number(10)    DEFAULT 0       NOT NULL;
alter table config add ldap_host             varchar2(255) DEFAULT '';
alter table config add ldap_port             number(10)    DEFAULT 389     NOT NULL;
alter table config add ldap_base_dn          varchar2(255) DEFAULT '';
alter table config add ldap_bind_dn          varchar2(255) DEFAULT '';
alter table config add ldap_bind_password    varchar2(128) DEFAULT '';
alter table config add ldap_search_attribute varchar2(128) DEFAULT '';
alter table dhosts modify ip varchar2(39) default '';
CREATE INDEX dhosts_1 on dhosts (druleid,ip);
CREATE TABLE drules_tmp (
	druleid		number(20)		DEFAULT '0'	NOT NULL,
	proxy_hostid		number(20)		DEFAULT '0'	NOT NULL,
	name		varchar2(255)		DEFAULT ''	,
	iprange		varchar2(255)		DEFAULT ''	,
	delay		number(10)		DEFAULT '0'	NOT NULL,
	nextcheck		number(10)		DEFAULT '0'	NOT NULL,
	status		number(10)		DEFAULT '0'	NOT NULL,
	PRIMARY KEY (druleid)
);
insert into drules_tmp select druleid,0,name,iprange,delay,nextcheck,status from drules;
drop table drules;
alter table drules_tmp rename to drules;
CREATE INDEX dservices_1 on dservices (dhostid,type,key_,port);
begin execute immediate 'drop table escalations'; exception when others then null; end;
CREATE TABLE escalations (
	escalationid	number(20)	DEFAULT '0'	NOT NULL,
	actionid	number(20)	DEFAULT '0'	NOT NULL,
	triggerid	number(20)	DEFAULT '0'	NOT NULL,
	eventid		number(20)	DEFAULT '0'	NOT NULL,
	r_eventid	number(20)	DEFAULT '0'	NOT NULL,
	nextcheck	number(10)	DEFAULT '0'	NOT NULL,
	esc_step	number(10)	DEFAULT '0'	NOT NULL,
	status		number(10)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (escalationid)
);
CREATE INDEX escalations_1 on escalations (actionid,triggerid);
CREATE INDEX escalations_2 on escalations (status,nextcheck);
CREATE TABLE events_tmp (
	eventid		number(20)		DEFAULT '0'	NOT NULL,
	source		number(10)		DEFAULT '0'	NOT NULL,
	object		number(10)		DEFAULT '0'	NOT NULL,
	objectid		number(20)		DEFAULT '0'	NOT NULL,
	clock		number(10)		DEFAULT '0'	NOT NULL,
	value		number(10)		DEFAULT '0'	NOT NULL,
	acknowledged		number(10)		DEFAULT '0'	NOT NULL,
	PRIMARY KEY (eventid)
);
insert into events_tmp select eventid,source,object,objectid,clock,value,acknowledged from events;
drop table events;
alter table events_tmp rename to events;
CREATE INDEX events_1 on events (object,objectid,eventid);
CREATE INDEX events_2 on events (clock);
update graphs_items set color='FF0000' where color='Red';
update graphs_items set color='960000' where color='Dark Red';
update graphs_items set color='00FF00' where color='Green';
update graphs_items set color='009600' where color='Dark Green';
update graphs_items set color='0000FF' where color='Blue';
update graphs_items set color='000096' where color='Dark Blue';
update graphs_items set color='FFFF00' where color='Yellow';
update graphs_items set color='969600' where color='Dark Yellow';
update graphs_items set color='00FFFF' where color='Cyan';
update graphs_items set color='000000' where color='Black';
update graphs_items set color='969696' where color='Gray';
update graphs_items set color='FFFFFF' where color='White';
alter table graphs_items modify color varchar2(6) DEFAULT '009600';

CREATE INDEX graphs_items_1 on graphs_items (itemid);
CREATE INDEX graphs_items_2 on graphs_items (graphid);
alter table graphs add show_legend	number(10)	DEFAULT '0'	NOT NULL;
alter table graphs add show_3d		number(10)	DEFAULT '0'	NOT NULL;
alter table graphs add percent_left	number(5,2)	DEFAULT '0'	NOT NULL;
alter table graphs add percent_right	number(5,2)	DEFAULT '0'	NOT NULL;
CREATE UNIQUE INDEX history_log_2 on history_log (itemid,id);
CREATE UNIQUE INDEX history_text_2 on history_text (itemid,id);
CREATE TABLE hosts_profiles_ext (
	hostid		number(20)	DEFAULT '0'	NOT NULL,
	device_alias	varchar2(64)	DEFAULT '',
	device_type	varchar2(64)	DEFAULT '',
	device_chassis	varchar2(64)	DEFAULT '',
	device_os	varchar2(64)	DEFAULT '',
	device_os_short	varchar2(64)	DEFAULT '',
	device_hw_arch	varchar2(32)	DEFAULT '',
	device_serial	varchar2(64)	DEFAULT '',
	device_model	varchar2(64)	DEFAULT '',
	device_tag	varchar2(64)	DEFAULT '',
	device_vendor	varchar2(64)	DEFAULT '',
	device_contract	varchar2(64)	DEFAULT '',
	device_who	varchar2(64)	DEFAULT '',
	device_status	varchar2(64)	DEFAULT '',
	device_app_01	varchar2(64)	DEFAULT '',
	device_app_02	varchar2(64)	DEFAULT '',
	device_app_03	varchar2(64)	DEFAULT '',
	device_app_04	varchar2(64)	DEFAULT '',
	device_app_05	varchar2(64)	DEFAULT '',
	device_url_1	varchar2(255)	DEFAULT '',
	device_url_2	varchar2(255)	DEFAULT '',
	device_url_3	varchar2(255)	DEFAULT '',
	device_networks	varchar2(2048)	DEFAULT '',
	device_notes	varchar2(2048)	DEFAULT '',
	device_hardware	varchar2(2048)	DEFAULT '',
	device_software	varchar2(2048)	DEFAULT '',
	ip_subnet_mask	varchar2(39)	DEFAULT '',
	ip_router	varchar2(39)	DEFAULT '',
	ip_macaddress	varchar2(64)	DEFAULT '',
	oob_ip		varchar2(39)	DEFAULT '',
	oob_subnet_mask	varchar2(39)	DEFAULT '',
	oob_router	varchar2(39)	DEFAULT '',
	date_hw_buy	varchar2(64)	DEFAULT '',
	date_hw_install	varchar2(64)	DEFAULT '',
	date_hw_expiry	varchar2(64)	DEFAULT '',
	date_hw_decomm	varchar2(64)	DEFAULT '',
	site_street_1	varchar2(128)	DEFAULT '',
	site_street_2	varchar2(128)	DEFAULT '',
	site_street_3	varchar2(128)	DEFAULT '',
	site_city	varchar2(128)	DEFAULT '',
	site_state	varchar2(64)	DEFAULT '',
	site_country	varchar2(64)	DEFAULT '',
	site_zip	varchar2(64)	DEFAULT '',
	site_rack	varchar2(128)	DEFAULT '',
	site_notes	varchar2(2048)	DEFAULT '',
	poc_1_name	varchar2(128)	DEFAULT '',
	poc_1_email	varchar2(128)	DEFAULT '',
	poc_1_phone_1	varchar2(64)	DEFAULT '',
	poc_1_phone_2	varchar2(64)	DEFAULT '',
	poc_1_cell	varchar2(64)	DEFAULT '',
	poc_1_screen	varchar2(64)	DEFAULT '',
	poc_1_notes	varchar2(2048)	DEFAULT '',
	poc_2_name	varchar2(128)	DEFAULT '',
	poc_2_email	varchar2(128)	DEFAULT '',
	poc_2_phone_1	varchar2(64)	DEFAULT '',
	poc_2_phone_2	varchar2(64)	DEFAULT '',
	poc_2_cell	varchar2(64)	DEFAULT '',
	poc_2_screen	varchar2(64)	DEFAULT '',
	poc_2_notes	varchar2(2048)	DEFAULT '',
	PRIMARY KEY (hostid)
);
CREATE TABLE hosts_tmp (
	hostid		number(20)		DEFAULT '0'	NOT NULL,
	proxy_hostid		number(20)		DEFAULT '0'	NOT NULL,
	host		varchar2(64)		DEFAULT ''	,
	dns		varchar2(64)		DEFAULT ''	,
	useip		number(10)		DEFAULT '1'	NOT NULL,
	ip		varchar2(39)		DEFAULT '127.0.0.1'	,
	port		number(10)		DEFAULT '10050'	NOT NULL,
	status		number(10)		DEFAULT '0'	NOT NULL,
	disable_until		number(10)		DEFAULT '0'	NOT NULL,
	error		varchar2(128)		DEFAULT ''	,
	available		number(10)		DEFAULT '0'	NOT NULL,
	errors_from		number(10)		DEFAULT '0'	NOT NULL,
	lastaccess		number(10)		DEFAULT '0'	NOT NULL,
	inbytes		number(20)		DEFAULT '0'	NOT NULL,
	outbytes		number(20)		DEFAULT '0'	NOT NULL,
	useipmi		number(10)		DEFAULT '0'	NOT NULL,
	ipmi_port		number(10)		DEFAULT '623'	NOT NULL,
	ipmi_authtype		number(10)		DEFAULT '0'	NOT NULL,
	ipmi_privilege		number(10)		DEFAULT '2'	NOT NULL,
	ipmi_username		varchar2(16)		DEFAULT ''	,
	ipmi_password		varchar2(20)		DEFAULT ''	,
	PRIMARY KEY (hostid)
);
insert into hosts_tmp select hostid,0,host,dns,useip,ip,port,status,disable_until,error,available,errors_from,0,0,0,0,623,0,2,'','' from hosts;
drop table hosts;
alter table hosts_tmp rename to hosts;
CREATE INDEX hosts_1 on hosts (host);
CREATE INDEX hosts_2 on hosts (status);
CREATE INDEX hosts_3 on hosts (proxy_hostid);
alter table httpstep modify url varchar2(255) DEFAULT '';
CREATE TABLE httptest_tmp (
        httptestid              number(20)              DEFAULT '0'     NOT NULL,
        name            varchar2(64)            DEFAULT ''      ,
        applicationid           number(20)              DEFAULT '0'     NOT NULL,
        lastcheck               number(10)              DEFAULT '0'     NOT NULL,
        nextcheck               number(10)              DEFAULT '0'     NOT NULL,
        curstate                number(10)              DEFAULT '0'     NOT NULL,
        curstep         number(10)              DEFAULT '0'     NOT NULL,
        lastfailedstep          number(10)              DEFAULT '0'     NOT NULL,
        delay           number(10)              DEFAULT '60'    NOT NULL,
        status          number(10)              DEFAULT '0'     NOT NULL,
        macros          varchar2(2048)          DEFAULT ''      ,
        agent           varchar2(255)           DEFAULT ''      ,
        time            number(20,4)            DEFAULT '0'     NOT NULL,
        error           varchar2(255)           DEFAULT ''      ,
        PRIMARY KEY (httptestid)
);

insert into httptest_tmp select * from httptest;
drop table httptest;
alter table httptest_tmp rename to httptest;

CREATE INDEX httptest_httptest_1 on httptest (applicationid);
CREATE INDEX httptest_2 on httptest (name);
CREATE INDEX httptest_3 on httptest (status);
delete from ids;
alter table items add ipmi_sensor varchar2(128) DEFAULT '';
CREATE INDEX items_4 on items (templateid);
drop table node_cksum;

CREATE TABLE node_cksum (
	nodeid		number(10)	DEFAULT '0'	NOT NULL,
	tablename	varchar2(64)	DEFAULT '',
	recordid	number(20)	DEFAULT '0'	NOT NULL,
	cksumtype	number(10)	DEFAULT '0'	NOT NULL,
	cksum		clob		DEFAULT ''	NOT NULL,
	sync		varchar2(128)	DEFAULT ''
);
CREATE INDEX node_cksum_cksum_1 on node_cksum (nodeid,tablename,recordid,cksumtype);
drop table node_configlog;
alter table nodes modify ip varchar2(39) DEFAULT '';
alter table nodes drop column event_lastid;
alter table nodes drop column history_lastid;
alter table nodes drop column history_str_lastid;
alter table nodes drop column history_uint_lastid;
CREATE TABLE opconditions (
	opconditionid	number(20)	DEFAULT '0'	NOT NULL,
	operationid	number(20)	DEFAULT '0'	NOT NULL,
	conditiontype	number(10)	DEFAULT '0'	NOT NULL,
	operator	number(10)	DEFAULT '0'	NOT NULL,
	value		varchar2(255)	DEFAULT '',
	PRIMARY KEY (opconditionid)
);
CREATE INDEX opconditions_1 on opconditions (operationid);
alter table operations add esc_period		number(10)	DEFAULT '0'	NOT NULL;
alter table operations add esc_step_from	number(10)	DEFAULT '0'	NOT NULL;
alter table operations add esc_step_to		number(10)	DEFAULT '0'	NOT NULL;
alter table operations add default_msg		number(10)	DEFAULT '0'	NOT NULL;
alter table operations add evaltype		number(10)	DEFAULT '0'	NOT NULL;
drop table profiles;
CREATE TABLE profiles (
	profileid	number(20)	DEFAULT '0'	NOT NULL,
	userid		number(20)	DEFAULT '0'	NOT NULL,
	idx		varchar2(96)	DEFAULT '',
	idx2		number(20)	DEFAULT '0'	NOT NULL,
	value_id	number(20)	DEFAULT '0'	NOT NULL,
	value_int	number(10)	DEFAULT '0'	NOT NULL,
	value_str	varchar2(255)	DEFAULT '',
	source		varchar2(96)	DEFAULT '',
	type		number(10)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (profileid)
);
CREATE INDEX profiles_1 on profiles (userid,idx,idx2);
CREATE TABLE proxy_dhistory (
	id		number(20)			NOT NULL,
	clock		number(10)	DEFAULT '0'	NOT NULL,
	druleid		number(20)	DEFAULT '0'	NOT NULL,
	type		number(10)	DEFAULT '0'	NOT NULL,
	ip		varchar2(39)	DEFAULT '',
	port		number(10)	DEFAULT '0'	NOT NULL,
	key_		varchar2(255)	DEFAULT '0',
	value		varchar2(255)	DEFAULT '0',
	status		number(10)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_dhistory_1 on proxy_dhistory (clock);
CREATE TABLE proxy_history (
	id		number(20)			NOT NULL,
	itemid		number(20)	DEFAULT '0'	NOT NULL,
	clock		number(10)	DEFAULT '0'	NOT NULL,
	timestamp	number(10)	DEFAULT '0'	NOT NULL,
	source		varchar2(64)	DEFAULT '',
	severity	number(10)	DEFAULT '0'	NOT NULL,
	value		varchar2(2048)	DEFAULT '',
	PRIMARY KEY (id)
);
CREATE INDEX proxy_history_1 on proxy_history (clock);
alter table rights drop column type;
alter table screens_items add dynamic number(10) DEFAULT '0' NOT NULL;
CREATE TABLE scripts (
	scriptid	number(20)	DEFAULT '0'	NOT NULL,
	name		varchar2(255)	DEFAULT '',
	command		varchar2(255)	DEFAULT '',
	host_access	number(10)	DEFAULT '0'	NOT NULL,
	usrgrpid	number(20)	DEFAULT '0'	NOT NULL,
	groupid		number(20)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (scriptid)
);
CREATE INDEX services_1 on services (triggerid);
alter table sessions add status number(10) DEFAULT '0' NOT NULL;
alter table sysmaps_elements add iconid_disabled number(20) DEFAULT '0' NOT NULL;
update sysmaps_elements set iconid_disabled=iconid_off;
CREATE TABLE sysmaps_link_triggers (
	linktriggerid	number(20)	DEFAULT '0'	NOT NULL,
	linkid		number(20)	DEFAULT '0'	NOT NULL,
	triggerid	number(20)	DEFAULT '0'	NOT NULL,
	drawtype	number(10)	DEFAULT '0'	NOT NULL,
	color		varchar2(6)	DEFAULT '000000',
	PRIMARY KEY (linktriggerid)
);
CREATE UNIQUE INDEX sysmaps_link_triggers_1 on sysmaps_link_triggers (linkid,triggerid);
update sysmaps_links set color_on='FF0000' where color_on='Red';
update sysmaps_links set color_on='960000' where color_on='Dark Red';
update sysmaps_links set color_on='00FF00' where color_on='Green';
update sysmaps_links set color_on='009600' where color_on='Dark Green';
update sysmaps_links set color_on='0000FF' where color_on='Blue';
update sysmaps_links set color_on='000096' where color_on='Dark Blue';
update sysmaps_links set color_on='FFFF00' where color_on='Yellow';
update sysmaps_links set color_on='969600' where color_on='Dark Yellow';
update sysmaps_links set color_on='00FFFF' where color_on='Cyan';
update sysmaps_links set color_on='000000' where color_on='Black';
update sysmaps_links set color_on='969696' where color_on='Gray';
update sysmaps_links set color_on='FFFFFF' where color_on='White';
update sysmaps_links set color_off='FF0000' where color_off='Red';
update sysmaps_links set color_off='960000' where color_off='Dark Red';
update sysmaps_links set color_off='00FF00' where color_off='Green';
update sysmaps_links set color_off='009600' where color_off='Dark Green';
update sysmaps_links set color_off='0000FF' where color_off='Blue';
update sysmaps_links set color_off='000096' where color_off='Dark Blue';
update sysmaps_links set color_off='FFFF00' where color_off='Yellow';
update sysmaps_links set color_off='969600' where color_off='Dark Yellow';
update sysmaps_links set color_off='00FFFF' where color_off='Cyan';
update sysmaps_links set color_off='000000' where color_off='Black';
update sysmaps_links set color_off='969696' where color_off='Gray';
update sysmaps_links set color_off='FFFFFF' where color_off='White';
insert into sysmaps_link_triggers select linkid,linkid,triggerid,drawtype_on,color_on from sysmaps_links;
alter table sysmaps_links drop column triggerid;
alter table sysmaps_links rename column drawtype_off to drawtype;
alter table sysmaps_links rename column color_off to color;
alter table sysmaps_links modify color varchar2(6) DEFAULT '000000';
alter table sysmaps_links drop column drawtype_on;
alter table sysmaps_links drop column color_on;
-- See sysmaps_links.sql
CREATE TABLE trends_uint (
	itemid		number(20)	DEFAULT '0'	NOT NULL,
	clock		number(10)	DEFAULT '0'	NOT NULL,
	num		number(10)	DEFAULT '0'	NOT NULL,
	value_min	number(20)	DEFAULT '0'	NOT NULL,
	value_avg	number(20)	DEFAULT '0'	NOT NULL,
	value_max	number(20)	DEFAULT '0'	NOT NULL,
	PRIMARY KEY (itemid,clock)
);
alter table triggers modify comments varchar2(2048) DEFAULT '';
alter table triggers add type number(10) DEFAULT '0' NOT NULL;
CREATE TABLE users_tmp (
	userid		number(20)	DEFAULT '0'	NOT NULL,
	alias		varchar2(100)	DEFAULT '',
	name		varchar2(100)	DEFAULT '',
	surname		varchar2(100)	DEFAULT '',
	passwd		varchar2(32)	DEFAULT '',
	url		varchar2(255)	DEFAULT '',
	autologin	number(10)	DEFAULT '0'	NOT NULL,
	autologout	number(10)	DEFAULT '900'	NOT NULL,
	lang		varchar2(5)	DEFAULT 'en_gb',
	refresh		number(10)	DEFAULT '30'	NOT NULL,
	type		number(10)	DEFAULT '0'	NOT NULL,
	theme		varchar2(128)	DEFAULT 'default.css',
	attempt_failed	number(10)	DEFAULT 0	NOT NULL,
	attempt_ip	varchar2(39)	DEFAULT '',
	attempt_clock	number(10)	DEFAULT 0	NOT NULL,
	PRIMARY KEY (userid)
);
insert into users_tmp select userid,alias,name,surname,passwd,url,0,autologout,lang,refresh,type,'default.css',0,'',0 from users;
drop table users;
alter table users_tmp rename to users;
update users set passwd='5fce1b3e34b520afeffb37ce08c7cd66' where alias<>'guest' and passwd='d41d8cd98f00b204e9800998ecf8427e';
CREATE INDEX users_1 on users (alias);
alter table usrgrp add gui_access	number(10)	DEFAULT '0'	NOT NULL;
alter table usrgrp add users_status	number(10)	DEFAULT '0'	NOT NULL;
