alter table actions add	esc_period	integer		DEFAULT '0'	NOT NULL;
alter table actions add	def_shortdata	varchar(255)	DEFAULT ''	NOT NULL;
alter table actions add	def_longdata	blob				NOT NULL;
alter table actions add	recovery_msg	integer		DEFAULT '0'	NOT NULL;
alter table actions add	r_shortdata	varchar(255)	DEFAULT ''	NOT NULL;
alter table actions add	r_longdata	blob				NOT NULL;

CREATE INDEX actions_1 on actions (eventsource,status);
alter table alerts drop index alerts_1;
alter table alerts drop index alerts_2;
alter table alerts drop index alerts_3;
alter table alerts drop index alerts_4;
alter table alerts drop index alerts_5;
alter table alerts drop index alerts_6;

alter table alerts add eventid bigint(20) unsigned NOT NULL default '0' after actionid;
alter table alerts add esc_step integer DEFAULT '0'     NOT NULL;
alter table alerts add alerttype integer DEFAULT '0'     NOT NULL;

update alerts, events set alerts.eventid = events.eventid where events.objectid = alerts.triggerid and events.object = 0 and alerts.eventid = 0 and events.clock = alerts.clock;
update alerts, events set alerts.eventid = events.eventid where events.objectid = alerts.triggerid and events.object = 0 and alerts.eventid = 0 and events.clock = alerts.clock + 1;
alter table alerts drop triggerid;

CREATE INDEX alerts_1 on alerts (actionid);
CREATE INDEX alerts_2 on alerts (clock);
CREATE INDEX alerts_3 on alerts (eventid);
CREATE INDEX alerts_4 on alerts (status,retries);
CREATE INDEX alerts_5 on alerts (mediatypeid);
CREATE INDEX alerts_6 on alerts (userid);

update alerts set status=3 where retries>=2;
alter table config add  event_ack_enable int(11) NOT NULL default '1';
alter table config add  event_expire int(11) NOT NULL default '7';
alter table config add  event_show_max int(11) NOT NULL default '100';
alter table config add  default_theme varchar(128) NOT NULL default 'default.css';


alter table config add authentication_type             integer         DEFAULT 0       NOT NULL;
alter table config add ldap_host               varchar(255)            DEFAULT ''      NOT NULL;
alter table config add ldap_port               integer         DEFAULT 389     NOT NULL;
alter table config add ldap_base_dn            varchar(255)            DEFAULT ''      NOT NULL;
alter table config add ldap_bind_dn            varchar(255)            DEFAULT ''      NOT NULL;
alter table config add ldap_bind_password              varchar(128)            DEFAULT ''      NOT NULL;
alter table config add ldap_search_attribute           varchar(128)            DEFAULT ''      NOT NULL;
alter table dhosts modify ip varchar(39) NOT NULL default '';
CREATE INDEX dhosts_1 on dhosts (druleid,ip);
alter table drules add proxy_hostid bigint(20) unsigned NOT NULL default '0' after druleid;
CREATE INDEX dservices_1 on dservices (dhostid,type,key_,port);
DROP TABLE IF EXISTS escalations;
CREATE TABLE escalations (
        escalationid            bigint unsigned         DEFAULT '0'     NOT NULL,
        actionid                bigint unsigned         DEFAULT '0'     NOT NULL,
        triggerid               bigint unsigned         DEFAULT '0'     NOT NULL,
        eventid         bigint unsigned         DEFAULT '0'     NOT NULL,
        r_eventid               bigint unsigned         DEFAULT '0'     NOT NULL,
        nextcheck               integer         DEFAULT '0'     NOT NULL,
        esc_step                integer         DEFAULT '0'     NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (escalationid)
) ENGINE=InnoDB;
CREATE INDEX escalations_1 on escalations (actionid,triggerid);
CREATE INDEX escalations_2 on escalations (status,nextcheck);
alter table events drop index events_1;
alter table events drop index events_2;

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
alter table graphs_items change color color varchar(6) DEFAULT '009600' NOT NULL;
CREATE INDEX graphs_items_1 on graphs_items (itemid);
CREATE INDEX graphs_items_2 on graphs_items (graphid);
alter table graphs add show_legend int(11) NOT NULL default '0';
alter table graphs add show_3d int(11) NOT NULL default '0';
alter table graphs add percent_left            double(16,4)            DEFAULT '0'     NOT NULL;
alter table graphs add percent_right           double(16,4)            DEFAULT '0'     NOT NULL;
CREATE UNIQUE INDEX history_log_2 on history_log (itemid,id);
CREATE UNIQUE INDEX history_text_2 on history_text (itemid,id);
CREATE TABLE hosts_profiles_ext (
	hostid		bigint unsigned		DEFAULT '0'	NOT NULL,
	device_alias		varchar(64)		DEFAULT ''	NOT NULL,
	device_type		varchar(64)		DEFAULT ''	NOT NULL,
	device_chassis		varchar(64)		DEFAULT ''	NOT NULL,
	device_os		varchar(64)		DEFAULT ''	NOT NULL,
	device_os_short		varchar(64)		DEFAULT ''	NOT NULL,
	device_hw_arch		varchar(32)		DEFAULT ''	NOT NULL,
	device_serial		varchar(64)		DEFAULT ''	NOT NULL,
	device_model		varchar(64)		DEFAULT ''	NOT NULL,
	device_tag		varchar(64)		DEFAULT ''	NOT NULL,
	device_vendor		varchar(64)		DEFAULT ''	NOT NULL,
	device_contract		varchar(64)		DEFAULT ''	NOT NULL,
	device_who		varchar(64)		DEFAULT ''	NOT NULL,
	device_status		varchar(64)		DEFAULT ''	NOT NULL,
	device_app_01		varchar(64)		DEFAULT ''	NOT NULL,
	device_app_02		varchar(64)		DEFAULT ''	NOT NULL,
	device_app_03		varchar(64)		DEFAULT ''	NOT NULL,
	device_app_04		varchar(64)		DEFAULT ''	NOT NULL,
	device_app_05		varchar(64)		DEFAULT ''	NOT NULL,
	device_url_1		varchar(255)		DEFAULT ''	NOT NULL,
	device_url_2		varchar(255)		DEFAULT ''	NOT NULL,
	device_url_3		varchar(255)		DEFAULT ''	NOT NULL,
	device_networks		blob			NOT NULL,
	device_notes		blob			NOT NULL,
	device_hardware		blob			NOT NULL,
	device_software		blob			NOT NULL,
	ip_subnet_mask		varchar(39)		DEFAULT ''	NOT NULL,
	ip_router		varchar(39)		DEFAULT ''	NOT NULL,
	ip_macaddress		varchar(64)		DEFAULT ''	NOT NULL,
	oob_ip		varchar(39)		DEFAULT ''	NOT NULL,
	oob_subnet_mask		varchar(39)		DEFAULT ''	NOT NULL,
	oob_router		varchar(39)		DEFAULT ''	NOT NULL,
	date_hw_buy		varchar(64)		DEFAULT ''	NOT NULL,
	date_hw_install		varchar(64)		DEFAULT ''	NOT NULL,
	date_hw_expiry		varchar(64)		DEFAULT ''	NOT NULL,
	date_hw_decomm		varchar(64)		DEFAULT ''	NOT NULL,
	site_street_1		varchar(128)		DEFAULT ''	NOT NULL,
	site_street_2		varchar(128)		DEFAULT ''	NOT NULL,
	site_street_3		varchar(128)		DEFAULT ''	NOT NULL,
	site_city		varchar(128)		DEFAULT ''	NOT NULL,
	site_state		varchar(64)		DEFAULT ''	NOT NULL,
	site_country		varchar(64)		DEFAULT ''	NOT NULL,
	site_zip		varchar(64)		DEFAULT ''	NOT NULL,
	site_rack		varchar(128)		DEFAULT ''	NOT NULL,
	site_notes		blob			NOT NULL,
	poc_1_name		varchar(128)		DEFAULT ''	NOT NULL,
	poc_1_email		varchar(128)		DEFAULT ''	NOT NULL,
	poc_1_phone_1		varchar(64)		DEFAULT ''	NOT NULL,
	poc_1_phone_2		varchar(64)		DEFAULT ''	NOT NULL,
	poc_1_cell		varchar(64)		DEFAULT ''	NOT NULL,
	poc_1_screen		varchar(64)		DEFAULT ''	NOT NULL,
	poc_1_notes		blob			NOT NULL,
	poc_2_name		varchar(128)		DEFAULT ''	NOT NULL,
	poc_2_email		varchar(128)		DEFAULT ''	NOT NULL,
	poc_2_phone_1		varchar(64)		DEFAULT ''	NOT NULL,
	poc_2_phone_2		varchar(64)		DEFAULT ''	NOT NULL,
	poc_2_cell		varchar(64)		DEFAULT ''	NOT NULL,
	poc_2_screen		varchar(64)		DEFAULT ''	NOT NULL,
	poc_2_notes		blob			NOT NULL,
	PRIMARY KEY (hostid)
) ENGINE=InnoDB;
alter table hosts add proxy_hostid bigint unsigned DEFAULT '0' NOT NULL after hostid;
alter table hosts add lastaccess integer DEFAULT '0' NOT NULL;
alter table hosts add inbytes bigint unsigned DEFAULT '0' NOT NULL;
alter table hosts add outbytes bigint unsigned DEFAULT '0' NOT NULL;
alter table hosts modify ip varchar(39) DEFAULT '127.0.0.1' NOT NULL;
alter table hosts add useipmi         integer         DEFAULT '0'     NOT NULL;
alter table hosts add ipmi_port               integer         DEFAULT '623'   NOT NULL;
alter table hosts add ipmi_authtype           integer         DEFAULT '0'     NOT NULL;
alter table hosts add ipmi_privilege          integer         DEFAULT '2'     NOT NULL;
alter table hosts add ipmi_username           varchar(16)             DEFAULT ''      NOT NULL;
alter table hosts add ipmi_password           varchar(20)             DEFAULT ''      NOT NULL;
alter table hosts add ipmi_disable_until              integer         DEFAULT '0'     NOT NULL;
alter table hosts add ipmi_available          integer         DEFAULT '0'     NOT NULL;
alter table hosts add snmp_disable_until              integer         DEFAULT '0'     NOT NULL;
alter table hosts add snmp_available          integer         DEFAULT '0'     NOT NULL;

CREATE INDEX hosts_3 on hosts (proxy_hostid);
alter table httpstep modify url             varchar(255)            DEFAULT ''      NOT NULL;
CREATE TABLE httptest_tmp (
        httptestid              bigint unsigned         DEFAULT '0'     NOT NULL,
        name            varchar(64)             DEFAULT ''      NOT NULL,
        applicationid           bigint unsigned         DEFAULT '0'     NOT NULL,
        lastcheck               integer         DEFAULT '0'     NOT NULL,
        nextcheck               integer         DEFAULT '0'     NOT NULL,
        curstate                integer         DEFAULT '0'     NOT NULL,
        curstep         integer         DEFAULT '0'     NOT NULL,
        lastfailedstep          integer         DEFAULT '0'     NOT NULL,
        delay           integer         DEFAULT '60'    NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        macros          blob                    NOT NULL,
        agent           varchar(255)            DEFAULT ''      NOT NULL,
        time            double(16,4)            DEFAULT '0'     NOT NULL,
        error           varchar(255)            DEFAULT ''      NOT NULL,
        PRIMARY KEY (httptestid)
) ENGINE=InnoDB;

insert into httptest_tmp select * from httptest;
drop table httptest;
alter table httptest_tmp rename to httptest;

CREATE INDEX httptest_httptest_1 on httptest (applicationid);
CREATE INDEX httptest_2 on httptest (name);
CREATE INDEX httptest_3 on httptest (status);
delete from ids;
alter table items add ipmi_sensor             varchar(128)            DEFAULT ''      NOT NULL;
CREATE INDEX items_4 on items (templateid);
drop table node_cksum;

CREATE TABLE node_cksum (
nodeid		integer		DEFAULT '0'	NOT NULL,
tablename	varchar(64)	DEFAULT ''	NOT NULL,
recordid	bigint unsigned	DEFAULT '0'	NOT NULL,
cksumtype	integer		DEFAULT '0'	NOT NULL,
cksum		text				NOT NULL,
sync		char(128)	DEFAULT ''	NOT NULL
) ENGINE=InnoDB;
CREATE INDEX node_cksum_cksum_1 on node_cksum (nodeid,tablename,recordid,cksumtype);
drop table node_configlog;
alter table nodes modify ip varchar(39) DEFAULT '' NOT NULL;

alter table nodes drop event_lastid;
alter table nodes drop history_lastid;
alter table nodes drop history_str_lastid;
alter table nodes drop history_uint_lastid;
CREATE TABLE opconditions (
        opconditionid           bigint unsigned         DEFAULT '0'     NOT NULL,
        operationid             bigint unsigned         DEFAULT '0'     NOT NULL,
        conditiontype           integer         DEFAULT '0'     NOT NULL,
        operator                integer         DEFAULT '0'     NOT NULL,
        value           varchar(255)            DEFAULT ''      NOT NULL,
        PRIMARY KEY (opconditionid)
) ENGINE=InnoDB;
CREATE INDEX opconditions_1 on opconditions (operationid);
alter table operations add        esc_period              integer         DEFAULT '0'     NOT NULL;
alter table operations add        esc_step_from           integer         DEFAULT '0'     NOT NULL;
alter table operations add        esc_step_to             integer         DEFAULT '0'     NOT NULL;
alter table operations add        default_msg             integer         DEFAULT '0'     NOT NULL;
alter table operations add        evaltype                integer         DEFAULT '0'     NOT NULL;
drop table profiles;
CREATE TABLE profiles (
        profileid               bigint unsigned         DEFAULT '0'     NOT NULL,
        userid          bigint unsigned         DEFAULT '0'     NOT NULL,
        idx             varchar(96)             DEFAULT ''      NOT NULL,
        idx2            bigint unsigned         DEFAULT '0'     NOT NULL,
        value_id                bigint unsigned         DEFAULT '0'     NOT NULL,
        value_int               integer         DEFAULT '0'     NOT NULL,
        value_str               varchar(255)            DEFAULT ''      NOT NULL,
        source          varchar(96)             DEFAULT ''      NOT NULL,
        type            integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (profileid)
) ENGINE=InnoDB;
CREATE INDEX profiles_1 on profiles (userid,idx,idx2);
CREATE TABLE proxy_dhistory (
        id              bigint unsigned                 NOT NULL        auto_increment unique,
        clock           integer         DEFAULT '0'     NOT NULL,
        druleid         bigint unsigned         DEFAULT '0'     NOT NULL,
        type            integer         DEFAULT '0'     NOT NULL,
        ip              varchar(39)             DEFAULT ''      NOT NULL,
        port            integer         DEFAULT '0'     NOT NULL,
        key_            varchar(255)            DEFAULT '0'     NOT NULL,
        value           varchar(255)            DEFAULT '0'     NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE INDEX proxy_dhistory_1 on proxy_dhistory (clock);
CREATE TABLE proxy_history (
        id              bigint unsigned                 NOT NULL        auto_increment unique,
        itemid          bigint unsigned         DEFAULT '0'     NOT NULL,
        clock           integer         DEFAULT '0'     NOT NULL,
        timestamp               integer         DEFAULT '0'     NOT NULL,
        source          varchar(64)             DEFAULT ''      NOT NULL,
        severity                integer         DEFAULT '0'     NOT NULL,
        value           text                    NOT NULL,
        PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE INDEX proxy_history_1 on proxy_history (clock);
alter table rights drop type;
alter table screens_items add dynamic integer DEFAULT '0' NOT NULL;
CREATE TABLE scripts (
        scriptid                bigint unsigned         DEFAULT '0'     NOT NULL,
        name            varchar(255)            DEFAULT ''      NOT NULL,
        command         varchar(255)            DEFAULT ''      NOT NULL,
        host_access             integer         DEFAULT '2'     NOT NULL,
	usrgrpid                bigint unsigned         DEFAULT '0'     NOT NULL,
	groupid         bigint unsigned         DEFAULT '0'     NOT NULL,

        PRIMARY KEY (scriptid)
) ENGINE=InnoDB;
alter table services modify         goodsla         double(16,4)            DEFAULT '99.9'  NOT NULL;
CREATE INDEX services_1 on services (triggerid);
alter table sessions add status          integer         DEFAULT '0'     NOT NULL;
alter table sysmaps_elements add iconid_disabled bigint unsigned DEFAULT '0' NOT NULL;
update sysmaps_elements set iconid_disabled=iconid_off;
CREATE TABLE sysmaps_link_triggers (
	linktriggerid bigint unsigned DEFAULT '0'      NOT NULL,
	linkid        bigint unsigned DEFAULT '0'      NOT NULL,
	triggerid     bigint unsigned DEFAULT '0'      NOT NULL,
	drawtype      integer         DEFAULT '0'      NOT NULL,
	color         varchar(6)      DEFAULT '000000' NOT NULL,
	PRIMARY KEY (linktriggerid)
) ENGINE=InnoDB;
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
insert into sysmaps_link_triggers (linktriggerid,linkid,triggerid,drawtype,color) select linkid,linkid,triggerid,drawtype_on,color_on from sysmaps_links;
alter table sysmaps_links drop triggerid;
alter table sysmaps_links change drawtype_off drawtype integer DEFAULT '0' NOT NULL;
alter table sysmaps_links change color_off color varchar(6) DEFAULT '000000' NOT NULL;
alter table sysmaps_links drop drawtype_on;
alter table sysmaps_links drop color_on;
-- See sysmaps_links.sql
CREATE TABLE trends_uint (
        itemid          bigint unsigned         DEFAULT '0'     NOT NULL,
        clock           integer         DEFAULT '0'     NOT NULL,
        num             integer         DEFAULT '0'     NOT NULL,
        value_min               bigint unsigned         DEFAULT '0'     NOT NULL,
        value_avg               bigint unsigned         DEFAULT '0'     NOT NULL,
        value_max               bigint unsigned         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (itemid,clock)
) ENGINE=InnoDB;
update triggers set comments='' where comments is null;
alter table triggers modify comments blob NOT NULL;

alter table triggers add type integer DEFAULT '0' NOT NULL;
alter table users add theme varchar(128) DEFAULT 'default.css' NOT NULL;
alter table users add attempt_failed          integer         DEFAULT 0       NOT NULL;
alter table users add attempt_ip              varchar(39)             DEFAULT ''      NOT NULL;
alter table users add attempt_clock           integer         DEFAULT 0       NOT NULL;
alter table users add autologin integer DEFAULT '0' NOT NULL after url;
update users set passwd='5fce1b3e34b520afeffb37ce08c7cd66' where alias<>'guest' and passwd='d41d8cd98f00b204e9800998ecf8427e';
alter table usrgrp add gui_access integer DEFAULT '0' NOT NULL;
alter table usrgrp add users_status integer DEFAULT '0' NOT NULL;
