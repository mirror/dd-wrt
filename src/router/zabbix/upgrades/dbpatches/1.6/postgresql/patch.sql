alter table actions add        esc_period              integer         DEFAULT '0'     NOT NULL;
alter table actions add        def_shortdata           varchar(255)            DEFAULT ''      NOT NULL;
alter table actions add        def_longdata            text            DEFAULT ''      NOT NULL;
alter table actions add        recovery_msg            integer         DEFAULT '0'     NOT NULL;
alter table actions add        r_shortdata             varchar(255)            DEFAULT ''      NOT NULL;
alter table actions add        r_longdata              text            DEFAULT ''      NOT NULL;

CREATE INDEX actions_1 on actions (eventsource,status);
CREATE TABLE alerts_tmp (
        alertid         bigint          DEFAULT '0'     NOT NULL,
        actionid                bigint          DEFAULT '0'     NOT NULL,
        eventid         bigint          DEFAULT '0'     NOT NULL,
        userid          bigint          DEFAULT '0'     NOT NULL,
        clock           integer         DEFAULT '0'     NOT NULL,
        mediatypeid             bigint          DEFAULT '0'     NOT NULL,
        sendto          varchar(100)            DEFAULT ''      NOT NULL,
        subject         varchar(255)            DEFAULT ''      NOT NULL,
        message         text            DEFAULT ''      NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        retries         integer         DEFAULT '0'     NOT NULL,
        error           varchar(128)            DEFAULT ''      NOT NULL,
        nextcheck               integer         DEFAULT '0'     NOT NULL,
        esc_step                integer         DEFAULT '0'     NOT NULL,
        alerttype               integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (alertid)
) with OIDS;

alter table alerts add eventid bigint DEFAULT '0' NOT NULL;

update alerts set eventid = e.eventid from events e where e.objectid = alerts.triggerid and e.object = 0 and alerts.eventid = 0 and e.clock = alerts.clock;
update alerts set eventid = e.eventid from events e where e.objectid = alerts.triggerid and e.object = 0 and alerts.eventid = 0 and e.clock = alerts.clock + 1;

insert into alerts_tmp (alertid,actionid,eventid,userid,clock,mediatypeid,sendto,subject,message,status,retries,error,nextcheck) select alertid,actionid,eventid,userid,clock,mediatypeid,sendto,subject,message,status,retries,error,nextcheck from alerts;
update alerts_tmp set status=3 where retries>=2;

drop table alerts;
alter table alerts_tmp rename to alerts;

CREATE INDEX alerts_1 on alerts (actionid);
CREATE INDEX alerts_2 on alerts (clock);
CREATE INDEX alerts_3 on alerts (eventid);
CREATE INDEX alerts_4 on alerts (status,retries);
CREATE INDEX alerts_5 on alerts (mediatypeid);
CREATE INDEX alerts_6 on alerts (userid);
alter table config add  event_ack_enable integer NOT NULL default '1';
alter table config add  event_expire integer NOT NULL default '7';
alter table config add  event_show_max integer NOT NULL default '100';
alter table config add  default_theme varchar(128) NOT NULL default 'default.css';


alter table config add authentication_type             integer         DEFAULT 0       NOT NULL;
alter table config add ldap_host               varchar(255)            DEFAULT ''      NOT NULL;
alter table config add ldap_port               integer         DEFAULT 389     NOT NULL;
alter table config add ldap_base_dn            varchar(255)            DEFAULT ''      NOT NULL;
alter table config add ldap_bind_dn            varchar(255)            DEFAULT ''      NOT NULL;
alter table config add ldap_bind_password              varchar(128)            DEFAULT ''      NOT NULL;
alter table config add ldap_search_attribute           varchar(128)            DEFAULT ''      NOT NULL;
CREATE TABLE dhosts_tmp (
        dhostid         bigint          DEFAULT '0'     NOT NULL,
        druleid         bigint          DEFAULT '0'     NOT NULL,
        ip              varchar(39)             DEFAULT ''      NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        lastup          integer         DEFAULT '0'     NOT NULL,
        lastdown                integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (dhostid)
) with OIDS;

insert into dhosts_tmp select * from dhosts;
drop table dhosts;
alter table dhosts_tmp rename to dhosts;

CREATE INDEX dhosts_1 on dhosts (druleid,ip);
CREATE TABLE drules_tmp (
        druleid         bigint          DEFAULT '0'     NOT NULL,
        proxy_hostid            bigint          DEFAULT '0'     NOT NULL,
        name            varchar(255)            DEFAULT ''      NOT NULL,
        iprange         varchar(255)            DEFAULT ''      NOT NULL,
        delay           integer         DEFAULT '0'     NOT NULL,
        nextcheck               integer         DEFAULT '0'     NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (druleid)
) with OIDS;

insert into drules_tmp select druleid,0,name,iprange,delay,nextcheck,status from drules;
drop table drules;
alter table drules_tmp rename to drules;
CREATE INDEX dservices_1 on dservices (dhostid,type,key_,port);
DROP TABLE IF EXISTS escalations;
CREATE TABLE escalations (
        escalationid            bigint          DEFAULT '0'     NOT NULL,
        actionid                bigint          DEFAULT '0'     NOT NULL,
        triggerid               bigint          DEFAULT '0'     NOT NULL,
        eventid         bigint          DEFAULT '0'     NOT NULL,
        r_eventid               bigint          DEFAULT '0'     NOT NULL,
        nextcheck               integer         DEFAULT '0'     NOT NULL,
        esc_step                integer         DEFAULT '0'     NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (escalationid)
) with OIDS;
CREATE INDEX escalations_1 on escalations (actionid,triggerid);
CREATE INDEX escalations_2 on escalations (status,nextcheck);
drop index events_1;
drop index events_2;

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
alter table graphs_items alter color set default '009600';
alter table graphs_items alter color type varchar(6);
CREATE INDEX graphs_items_1 on graphs_items (itemid);
CREATE INDEX graphs_items_2 on graphs_items (graphid);
alter table graphs add show_legend integer NOT NULL default '0';
alter table graphs add show_3d integer NOT NULL default '0';
alter table graphs add percent_left            numeric(16,4)            DEFAULT '0'     NOT NULL;
alter table graphs add percent_right           numeric(16,4)            DEFAULT '0'     NOT NULL;
CREATE TABLE history_log_tmp (
	id		bigint		DEFAULT '0'	NOT NULL,
	itemid		bigint		DEFAULT '0'	NOT NULL,
	clock		integer		DEFAULT '0'	NOT NULL,
	timestamp		integer		DEFAULT '0'	NOT NULL,
	source		varchar(64)		DEFAULT ''	NOT NULL,
	severity		integer		DEFAULT '0'	NOT NULL,
	value		text		DEFAULT ''	NOT NULL,
	PRIMARY KEY (id)
) with OIDS;
insert into history_log_tmp select id,itemid,clock,timestamp,source,severity,value from history_log;
drop table history_log;
alter table history_log_tmp rename to history_log;
CREATE INDEX history_log_1 on history_log (itemid,clock);
CREATE UNIQUE INDEX history_log_2 on history_log (itemid,id);
CREATE UNIQUE INDEX history_text_2 on history_text (itemid,id);
CREATE TABLE hosts_profiles_ext (
        hostid          bigint          DEFAULT '0'     NOT NULL,
        device_alias            varchar(64)             DEFAULT ''      NOT NULL,
        device_type             varchar(64)             DEFAULT ''      NOT NULL,
        device_chassis          varchar(64)             DEFAULT ''      NOT NULL,
        device_os               varchar(64)             DEFAULT ''      NOT NULL,
        device_os_short         varchar(64)             DEFAULT ''      NOT NULL,
        device_hw_arch          varchar(32)             DEFAULT ''      NOT NULL,
        device_serial           varchar(64)             DEFAULT ''      NOT NULL,
        device_model            varchar(64)             DEFAULT ''      NOT NULL,
        device_tag              varchar(64)             DEFAULT ''      NOT NULL,
        device_vendor           varchar(64)             DEFAULT ''      NOT NULL,
        device_contract         varchar(64)             DEFAULT ''      NOT NULL,
        device_who              varchar(64)             DEFAULT ''      NOT NULL,
        device_status           varchar(64)             DEFAULT ''      NOT NULL,
        device_app_01           varchar(64)             DEFAULT ''      NOT NULL,
        device_app_02           varchar(64)             DEFAULT ''      NOT NULL,
        device_app_03           varchar(64)             DEFAULT ''      NOT NULL,
        device_app_04           varchar(64)             DEFAULT ''      NOT NULL,
        device_app_05           varchar(64)             DEFAULT ''      NOT NULL,
        device_url_1            varchar(255)            DEFAULT ''      NOT NULL,
        device_url_2            varchar(255)            DEFAULT ''      NOT NULL,
        device_url_3            varchar(255)            DEFAULT ''      NOT NULL,
        device_networks         text            DEFAULT ''      NOT NULL,
        device_notes            text            DEFAULT ''      NOT NULL,
        device_hardware         text            DEFAULT ''      NOT NULL,
        device_software         text            DEFAULT ''      NOT NULL,
        ip_subnet_mask          varchar(39)             DEFAULT ''      NOT NULL,
        ip_router               varchar(39)             DEFAULT ''      NOT NULL,
        ip_macaddress           varchar(64)             DEFAULT ''      NOT NULL,
        oob_ip          varchar(39)             DEFAULT ''      NOT NULL,
        oob_subnet_mask         varchar(39)             DEFAULT ''      NOT NULL,
        oob_router              varchar(39)             DEFAULT ''      NOT NULL,
        date_hw_buy             varchar(64)             DEFAULT ''      NOT NULL,
        date_hw_install         varchar(64)             DEFAULT ''      NOT NULL,
        date_hw_expiry          varchar(64)             DEFAULT ''      NOT NULL,
        date_hw_decomm          varchar(64)             DEFAULT ''      NOT NULL,
        site_street_1           varchar(128)            DEFAULT ''      NOT NULL,
        site_street_2           varchar(128)            DEFAULT ''      NOT NULL,
        site_street_3           varchar(128)            DEFAULT ''      NOT NULL,
        site_city               varchar(128)            DEFAULT ''      NOT NULL,
        site_state              varchar(64)             DEFAULT ''      NOT NULL,
        site_country            varchar(64)             DEFAULT ''      NOT NULL,
        site_zip                varchar(64)             DEFAULT ''      NOT NULL,
        site_rack               varchar(128)            DEFAULT ''      NOT NULL,
        site_notes              text            DEFAULT ''      NOT NULL,
        poc_1_name              varchar(128)            DEFAULT ''      NOT NULL,
        poc_1_email             varchar(128)            DEFAULT ''      NOT NULL,
        poc_1_phone_1           varchar(64)             DEFAULT ''      NOT NULL,
        poc_1_phone_2           varchar(64)             DEFAULT ''      NOT NULL,
        poc_1_cell              varchar(64)             DEFAULT ''      NOT NULL,
        poc_1_screen            varchar(64)             DEFAULT ''      NOT NULL,
        poc_1_notes             text            DEFAULT ''      NOT NULL,
        poc_2_name              varchar(128)            DEFAULT ''      NOT NULL,
        poc_2_email             varchar(128)            DEFAULT ''      NOT NULL,
        poc_2_phone_1           varchar(64)             DEFAULT ''      NOT NULL,
        poc_2_phone_2           varchar(64)             DEFAULT ''      NOT NULL,
        poc_2_cell              varchar(64)             DEFAULT ''      NOT NULL,
        poc_2_screen            varchar(64)             DEFAULT ''      NOT NULL,
        poc_2_notes             text            DEFAULT ''      NOT NULL,
        PRIMARY KEY (hostid)
) with OIDS;
CREATE TABLE hosts_tmp (
        hostid          bigint          DEFAULT '0'     NOT NULL,
        proxy_hostid            bigint          DEFAULT '0'     NOT NULL,
        host            varchar(64)             DEFAULT ''      NOT NULL,
        dns             varchar(64)             DEFAULT ''      NOT NULL,
        useip           integer         DEFAULT '1'     NOT NULL,
        ip              varchar(39)             DEFAULT '127.0.0.1'     NOT NULL,
        port            integer         DEFAULT '10050' NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        disable_until           integer         DEFAULT '0'     NOT NULL,
        error           varchar(128)            DEFAULT ''      NOT NULL,
        available               integer         DEFAULT '0'     NOT NULL,
        errors_from             integer         DEFAULT '0'     NOT NULL,
        lastaccess              integer         DEFAULT '0'     NOT NULL,
        inbytes         bigint          DEFAULT '0'     NOT NULL,
        outbytes                bigint          DEFAULT '0'     NOT NULL,
        useipmi         integer         DEFAULT '0'     NOT NULL,
        ipmi_port               integer         DEFAULT '623'   NOT NULL,
        ipmi_authtype           integer         DEFAULT '0'     NOT NULL,
        ipmi_privilege          integer         DEFAULT '2'     NOT NULL,
        ipmi_username           varchar(16)             DEFAULT ''      NOT NULL,
        ipmi_password           varchar(20)             DEFAULT ''      NOT NULL,
        ipmi_disable_until              integer         DEFAULT '0'     NOT NULL,
        ipmi_available          integer         DEFAULT '0'     NOT NULL,
        snmp_disable_until              integer         DEFAULT '0'     NOT NULL,
        snmp_available          integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (hostid)
) with OIDS;

insert into hosts_tmp select hostid,0,host,dns,useip,ip,port,status,disable_until,error,available,errors_from from hosts;
drop table hosts;
alter table hosts_tmp rename to hosts;

CREATE INDEX hosts_1 on hosts (host);
CREATE INDEX hosts_2 on hosts (status);
CREATE INDEX hosts_3 on hosts (proxy_hostid);
CREATE TABLE httpstep_tmp (
        httpstepid              bigint          DEFAULT '0'     NOT NULL,
        httptestid              bigint          DEFAULT '0'     NOT NULL,
        name            varchar(64)             DEFAULT ''      NOT NULL,
        no              integer         DEFAULT '0'     NOT NULL,
        url             varchar(255)            DEFAULT ''      NOT NULL,
        timeout         integer         DEFAULT '30'    NOT NULL,
        posts           text            DEFAULT ''      NOT NULL,
        required                varchar(255)            DEFAULT ''      NOT NULL,
        status_codes            varchar(255)            DEFAULT ''      NOT NULL,
        PRIMARY KEY (httpstepid)
) with OIDS;

insert into httpstep_tmp select * from httpstep;
drop table httpstep;
alter table httpstep_tmp rename to httpstep;

CREATE INDEX httpstep_httpstep_1 on httpstep (httptestid);
CREATE TABLE httptest_tmp (
        httptestid              bigint          DEFAULT '0'     NOT NULL,
        name            varchar(64)             DEFAULT ''      NOT NULL,
        applicationid           bigint          DEFAULT '0'     NOT NULL,
        lastcheck               integer         DEFAULT '0'     NOT NULL,
        nextcheck               integer         DEFAULT '0'     NOT NULL,
        curstate                integer         DEFAULT '0'     NOT NULL,
        curstep         integer         DEFAULT '0'     NOT NULL,
        lastfailedstep          integer         DEFAULT '0'     NOT NULL,
        delay           integer         DEFAULT '60'    NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        macros          text            DEFAULT ''      NOT NULL,
        agent           varchar(255)            DEFAULT ''      NOT NULL,
        time            numeric(16,4)           DEFAULT '0'     NOT NULL,
        error           varchar(255)            DEFAULT ''      NOT NULL,
        PRIMARY KEY (httptestid)
) with OIDS;

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
        nodeid          integer         DEFAULT '0'     NOT NULL,
        tablename               varchar(64)             DEFAULT ''      NOT NULL,
        recordid                bigint          DEFAULT '0'     NOT NULL,
        cksumtype               integer         DEFAULT '0'     NOT NULL,
        cksum           text            DEFAULT ''      NOT NULL,
        sync            char(128)               DEFAULT ''      NOT NULL
) with OIDS;
CREATE INDEX node_cksum_cksum_1 on node_cksum (nodeid,tablename,recordid,cksumtype);
drop table node_configlog;
CREATE TABLE nodes_tmp (
        nodeid          integer         DEFAULT '0'     NOT NULL,
        name            varchar(64)             DEFAULT '0'     NOT NULL,
        timezone                integer         DEFAULT '0'     NOT NULL,
        ip              varchar(39)             DEFAULT ''      NOT NULL,
        port            integer         DEFAULT '10051' NOT NULL,
        slave_history           integer         DEFAULT '30'    NOT NULL,
        slave_trends            integer         DEFAULT '365'   NOT NULL,
        nodetype                integer         DEFAULT '0'     NOT NULL,
        masterid                integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (nodeid)
) with OIDS;

insert into nodes_tmp select nodeid,name,timezone,ip,port,slave_history,slave_trends,nodetype,masterid from nodes;
drop table nodes;
alter table nodes_tmp rename to nodes;
CREATE TABLE opconditions (
        opconditionid           bigint          DEFAULT '0'     NOT NULL,
        operationid             bigint          DEFAULT '0'     NOT NULL,
        conditiontype           integer         DEFAULT '0'     NOT NULL,
        operator                integer         DEFAULT '0'     NOT NULL,
        value           varchar(255)            DEFAULT ''      NOT NULL,
        PRIMARY KEY (opconditionid)
) with OIDS;
CREATE INDEX opconditions_1 on opconditions (operationid);
alter table operations add        esc_period              integer         DEFAULT '0'     NOT NULL;
alter table operations add        esc_step_from           integer         DEFAULT '0'     NOT NULL;
alter table operations add        esc_step_to             integer         DEFAULT '0'     NOT NULL;
alter table operations add        default_msg             integer         DEFAULT '0'     NOT NULL;
alter table operations add        evaltype                integer         DEFAULT '0'     NOT NULL;
drop table profiles;
CREATE TABLE profiles (
        profileid               bigint          DEFAULT '0'     NOT NULL,
        userid          bigint          DEFAULT '0'     NOT NULL,
        idx             varchar(96)             DEFAULT ''      NOT NULL,
        idx2            bigint          DEFAULT '0'     NOT NULL,
        value_id                bigint          DEFAULT '0'     NOT NULL,
        value_int               integer         DEFAULT '0'     NOT NULL,
        value_str               varchar(255)            DEFAULT ''      NOT NULL,
        source          varchar(96)             DEFAULT ''      NOT NULL,
        type            integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (profileid)
) with OIDS;
CREATE INDEX profiles_1 on profiles (userid,idx,idx2);
CREATE TABLE proxy_dhistory (
        id              serial                  NOT NULL,
        clock           integer         DEFAULT '0'     NOT NULL,
        druleid         bigint          DEFAULT '0'     NOT NULL,
        type            integer         DEFAULT '0'     NOT NULL,
        ip              varchar(39)             DEFAULT ''      NOT NULL,
        port            integer         DEFAULT '0'     NOT NULL,
        key_            varchar(255)            DEFAULT '0'     NOT NULL,
        value           varchar(255)            DEFAULT '0'     NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (id)
) with OIDS;
CREATE INDEX proxy_dhistory_1 on proxy_dhistory (clock);
CREATE TABLE proxy_history (
        id              serial                  NOT NULL,
        itemid          bigint          DEFAULT '0'     NOT NULL,
        clock           integer         DEFAULT '0'     NOT NULL,
        timestamp               integer         DEFAULT '0'     NOT NULL,
        source          varchar(64)             DEFAULT ''      NOT NULL,
        severity                integer         DEFAULT '0'     NOT NULL,
        value           text            DEFAULT ''      NOT NULL,
        PRIMARY KEY (id)
) with OIDS;
CREATE INDEX proxy_history_1 on proxy_history (clock);
alter table rights drop type;
alter table screens_items add dynamic integer DEFAULT '0' NOT NULL;
CREATE TABLE scripts (
        scriptid                bigint          DEFAULT '0'     NOT NULL,
        name            varchar(255)            DEFAULT ''      NOT NULL,
        command         varchar(255)            DEFAULT ''      NOT NULL,
        host_access             integer         DEFAULT '2'     NOT NULL,
        usrgrpid                bigint          DEFAULT '0'     NOT NULL,
        groupid         bigint          DEFAULT '0'     NOT NULL,
        PRIMARY KEY (scriptid)
) with OIDS;
CREATE TABLE services_tmp (
        serviceid               bigint          DEFAULT '0'     NOT NULL,
        name            varchar(128)            DEFAULT ''      NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        algorithm               integer         DEFAULT '0'     NOT NULL,
        triggerid               bigint                  ,
        showsla         integer         DEFAULT '0'     NOT NULL,
        goodsla         numeric(16,4)           DEFAULT '99.9'  NOT NULL,
        sortorder               integer         DEFAULT '0'     NOT NULL,
        PRIMARY KEY (serviceid)
) with OIDS;

insert into services_tmp select * from services;
drop table services;
alter table services_tmp rename to services;

CREATE INDEX services_1 on services (triggerid);
alter table sessions add status          integer         DEFAULT '0'     NOT NULL;
alter table sysmaps_elements add iconid_disabled         bigint         DEFAULT '0'     NOT NULL;
update sysmaps_elements set iconid_disabled=iconid_off;
CREATE TABLE sysmaps_link_triggers (
        linktriggerid bigint     DEFAULT '0'      NOT NULL,
        linkid        bigint     DEFAULT '0'      NOT NULL,
        triggerid     bigint     DEFAULT '0'      NOT NULL,
        drawtype      integer    DEFAULT '0'      NOT NULL,
        color         varchar(6) DEFAULT '000000' NOT NULL,
        PRIMARY KEY (linktriggerid)
) with OIDS;
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
alter table sysmaps_links add drawtype integer DEFAULT '0' NOT NULL;
alter table sysmaps_links add color varchar(6) DEFAULT '000000' NOT NULL;
update sysmaps_links set drawtype=drawtype_off,color=color_off;
alter table sysmaps_links drop triggerid;
alter table sysmaps_links drop drawtype_off;
alter table sysmaps_links drop color_off;
alter table sysmaps_links drop drawtype_on;
alter table sysmaps_links drop color_on;
-- see sysmaps_links
CREATE TABLE trends_uint (
        itemid          bigint          DEFAULT '0'     NOT NULL,
        clock           integer         DEFAULT '0'     NOT NULL,
        num             integer         DEFAULT '0'     NOT NULL,
        value_min               bigint          DEFAULT '0'     NOT NULL,
        value_avg               bigint          DEFAULT '0'     NOT NULL,
        value_max               bigint          DEFAULT '0'     NOT NULL,
        PRIMARY KEY (itemid,clock)
) with OIDS;
update triggers set comments='' where comments is null;

CREATE TABLE triggers_tmp (
        triggerid               bigint          DEFAULT '0'     NOT NULL,
        expression              varchar(255)            DEFAULT ''      NOT NULL,
        description             varchar(255)            DEFAULT ''      NOT NULL,
        url             varchar(255)            DEFAULT ''      NOT NULL,
        status          integer         DEFAULT '0'     NOT NULL,
        value           integer         DEFAULT '0'     NOT NULL,
        priority                integer         DEFAULT '0'     NOT NULL,
        lastchange              integer         DEFAULT '0'     NOT NULL,
        dep_level               integer         DEFAULT '0'     NOT NULL,
        comments                text            DEFAULT ''      NOT NULL,
        error           varchar(128)            DEFAULT ''      NOT NULL,
        templateid              bigint          DEFAULT '0'     NOT NULL,
        PRIMARY KEY (triggerid)
) with OIDS;

insert into triggers_tmp select * from triggers;
drop table triggers;
alter table triggers_tmp rename to triggers;

alter table triggers add type integer DEFAULT '0' NOT NULL;
CREATE INDEX triggers_1 on triggers (status);
CREATE INDEX triggers_2 on triggers (value);
CREATE TABLE users_tmp (
        userid          bigint          DEFAULT '0'     NOT NULL,
        alias           varchar(100)            DEFAULT ''      NOT NULL,
        name            varchar(100)            DEFAULT ''      NOT NULL,
        surname         varchar(100)            DEFAULT ''      NOT NULL,
        passwd          char(32)                DEFAULT ''      NOT NULL,
        url             varchar(255)            DEFAULT ''      NOT NULL,
        autologin               integer         DEFAULT '0'     NOT NULL,
        autologout              integer         DEFAULT '900'   NOT NULL,
        lang            varchar(5)              DEFAULT 'en_gb' NOT NULL,
        refresh         integer         DEFAULT '30'    NOT NULL,
        type            integer         DEFAULT '0'     NOT NULL,
        theme           varchar(128)            DEFAULT 'default.css'   NOT NULL,
        attempt_failed          integer         DEFAULT 0       NOT NULL,
        attempt_ip              varchar(39)             DEFAULT ''      NOT NULL,
        attempt_clock           integer         DEFAULT 0       NOT NULL,
        PRIMARY KEY (userid)
) with OIDS;

insert into users_tmp select userid,alias,name,surname,passwd,url,0,autologout,lang,refresh,type from users;
drop table users;
alter table users_tmp rename to users;

CREATE INDEX users_1 on users (alias);

update users set passwd='5fce1b3e34b520afeffb37ce08c7cd66' where alias<>'guest' and passwd='d41d8cd98f00b204e9800998ecf8427e';
alter table usrgrp add gui_access integer DEFAULT '0' NOT NULL;
alter table usrgrp add users_status integer DEFAULT '0' NOT NULL;
