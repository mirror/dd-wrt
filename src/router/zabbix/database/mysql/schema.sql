CREATE TABLE `users` (
	`userid`                 bigint unsigned                           NOT NULL,
	`alias`                  varchar(100)    DEFAULT ''                NOT NULL,
	`name`                   varchar(100)    DEFAULT ''                NOT NULL,
	`surname`                varchar(100)    DEFAULT ''                NOT NULL,
	`passwd`                 varchar(32)     DEFAULT ''                NOT NULL,
	`url`                    varchar(255)    DEFAULT ''                NOT NULL,
	`autologin`              integer         DEFAULT '0'               NOT NULL,
	`autologout`             varchar(32)     DEFAULT '15m'             NOT NULL,
	`lang`                   varchar(5)      DEFAULT 'en_GB'           NOT NULL,
	`refresh`                varchar(32)     DEFAULT '30s'             NOT NULL,
	`type`                   integer         DEFAULT '1'               NOT NULL,
	`theme`                  varchar(128)    DEFAULT 'default'         NOT NULL,
	`attempt_failed`         integer         DEFAULT 0                 NOT NULL,
	`attempt_ip`             varchar(39)     DEFAULT ''                NOT NULL,
	`attempt_clock`          integer         DEFAULT 0                 NOT NULL,
	`rows_per_page`          integer         DEFAULT 50                NOT NULL,
	PRIMARY KEY (userid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `users_1` ON `users` (`alias`);
CREATE TABLE `maintenances` (
	`maintenanceid`          bigint unsigned                           NOT NULL,
	`name`                   varchar(128)    DEFAULT ''                NOT NULL,
	`maintenance_type`       integer         DEFAULT '0'               NOT NULL,
	`description`            text                                      NOT NULL,
	`active_since`           integer         DEFAULT '0'               NOT NULL,
	`active_till`            integer         DEFAULT '0'               NOT NULL,
	`tags_evaltype`          integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (maintenanceid)
) ENGINE=InnoDB;
CREATE INDEX `maintenances_1` ON `maintenances` (`active_since`,`active_till`);
CREATE UNIQUE INDEX `maintenances_2` ON `maintenances` (`name`);
CREATE TABLE `hosts` (
	`hostid`                 bigint unsigned                           NOT NULL,
	`proxy_hostid`           bigint unsigned                           NULL,
	`host`                   varchar(128)    DEFAULT ''                NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`disable_until`          integer         DEFAULT '0'               NOT NULL,
	`error`                  varchar(2048)   DEFAULT ''                NOT NULL,
	`available`              integer         DEFAULT '0'               NOT NULL,
	`errors_from`            integer         DEFAULT '0'               NOT NULL,
	`lastaccess`             integer         DEFAULT '0'               NOT NULL,
	`ipmi_authtype`          integer         DEFAULT '-1'              NOT NULL,
	`ipmi_privilege`         integer         DEFAULT '2'               NOT NULL,
	`ipmi_username`          varchar(16)     DEFAULT ''                NOT NULL,
	`ipmi_password`          varchar(20)     DEFAULT ''                NOT NULL,
	`ipmi_disable_until`     integer         DEFAULT '0'               NOT NULL,
	`ipmi_available`         integer         DEFAULT '0'               NOT NULL,
	`snmp_disable_until`     integer         DEFAULT '0'               NOT NULL,
	`snmp_available`         integer         DEFAULT '0'               NOT NULL,
	`maintenanceid`          bigint unsigned                           NULL,
	`maintenance_status`     integer         DEFAULT '0'               NOT NULL,
	`maintenance_type`       integer         DEFAULT '0'               NOT NULL,
	`maintenance_from`       integer         DEFAULT '0'               NOT NULL,
	`ipmi_errors_from`       integer         DEFAULT '0'               NOT NULL,
	`snmp_errors_from`       integer         DEFAULT '0'               NOT NULL,
	`ipmi_error`             varchar(2048)   DEFAULT ''                NOT NULL,
	`snmp_error`             varchar(2048)   DEFAULT ''                NOT NULL,
	`jmx_disable_until`      integer         DEFAULT '0'               NOT NULL,
	`jmx_available`          integer         DEFAULT '0'               NOT NULL,
	`jmx_errors_from`        integer         DEFAULT '0'               NOT NULL,
	`jmx_error`              varchar(2048)   DEFAULT ''                NOT NULL,
	`name`                   varchar(128)    DEFAULT ''                NOT NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`description`            text                                      NOT NULL,
	`tls_connect`            integer         DEFAULT '1'               NOT NULL,
	`tls_accept`             integer         DEFAULT '1'               NOT NULL,
	`tls_issuer`             varchar(1024)   DEFAULT ''                NOT NULL,
	`tls_subject`            varchar(1024)   DEFAULT ''                NOT NULL,
	`tls_psk_identity`       varchar(128)    DEFAULT ''                NOT NULL,
	`tls_psk`                varchar(512)    DEFAULT ''                NOT NULL,
	`proxy_address`          varchar(255)    DEFAULT ''                NOT NULL,
	`auto_compress`          integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (hostid)
) ENGINE=InnoDB;
CREATE INDEX `hosts_1` ON `hosts` (`host`);
CREATE INDEX `hosts_2` ON `hosts` (`status`);
CREATE INDEX `hosts_3` ON `hosts` (`proxy_hostid`);
CREATE INDEX `hosts_4` ON `hosts` (`name`);
CREATE INDEX `hosts_5` ON `hosts` (`maintenanceid`);
CREATE TABLE `hstgrp` (
	`groupid`                bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`internal`               integer         DEFAULT '0'               NOT NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
) ENGINE=InnoDB;
CREATE INDEX `hstgrp_1` ON `hstgrp` (`name`);
CREATE TABLE `group_prototype` (
	`group_prototypeid`      bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`groupid`                bigint unsigned                           NULL,
	`templateid`             bigint unsigned                           NULL,
	PRIMARY KEY (group_prototypeid)
) ENGINE=InnoDB;
CREATE INDEX `group_prototype_1` ON `group_prototype` (`hostid`);
CREATE TABLE `group_discovery` (
	`groupid`                bigint unsigned                           NOT NULL,
	`parent_group_prototypeid` bigint unsigned                           NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	`lastcheck`              integer         DEFAULT '0'               NOT NULL,
	`ts_delete`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
) ENGINE=InnoDB;
CREATE TABLE `screens` (
	`screenid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(255)                              NOT NULL,
	`hsize`                  integer         DEFAULT '1'               NOT NULL,
	`vsize`                  integer         DEFAULT '1'               NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`userid`                 bigint unsigned                           NULL,
	`private`                integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (screenid)
) ENGINE=InnoDB;
CREATE INDEX `screens_1` ON `screens` (`templateid`);
CREATE TABLE `screens_items` (
	`screenitemid`           bigint unsigned                           NOT NULL,
	`screenid`               bigint unsigned                           NOT NULL,
	`resourcetype`           integer         DEFAULT '0'               NOT NULL,
	`resourceid`             bigint unsigned DEFAULT '0'               NOT NULL,
	`width`                  integer         DEFAULT '320'             NOT NULL,
	`height`                 integer         DEFAULT '200'             NOT NULL,
	`x`                      integer         DEFAULT '0'               NOT NULL,
	`y`                      integer         DEFAULT '0'               NOT NULL,
	`colspan`                integer         DEFAULT '1'               NOT NULL,
	`rowspan`                integer         DEFAULT '1'               NOT NULL,
	`elements`               integer         DEFAULT '25'              NOT NULL,
	`valign`                 integer         DEFAULT '0'               NOT NULL,
	`halign`                 integer         DEFAULT '0'               NOT NULL,
	`style`                  integer         DEFAULT '0'               NOT NULL,
	`url`                    varchar(255)    DEFAULT ''                NOT NULL,
	`dynamic`                integer         DEFAULT '0'               NOT NULL,
	`sort_triggers`          integer         DEFAULT '0'               NOT NULL,
	`application`            varchar(255)    DEFAULT ''                NOT NULL,
	`max_columns`            integer         DEFAULT '3'               NOT NULL,
	PRIMARY KEY (screenitemid)
) ENGINE=InnoDB;
CREATE INDEX `screens_items_1` ON `screens_items` (`screenid`);
CREATE TABLE `screen_user` (
	`screenuserid`           bigint unsigned                           NOT NULL,
	`screenid`               bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (screenuserid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `screen_user_1` ON `screen_user` (`screenid`,`userid`);
CREATE TABLE `screen_usrgrp` (
	`screenusrgrpid`         bigint unsigned                           NOT NULL,
	`screenid`               bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (screenusrgrpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `screen_usrgrp_1` ON `screen_usrgrp` (`screenid`,`usrgrpid`);
CREATE TABLE `slideshows` (
	`slideshowid`            bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`delay`                  varchar(32)     DEFAULT '30s'             NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`private`                integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (slideshowid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `slideshows_1` ON `slideshows` (`name`);
CREATE TABLE `slideshow_user` (
	`slideshowuserid`        bigint unsigned                           NOT NULL,
	`slideshowid`            bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (slideshowuserid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `slideshow_user_1` ON `slideshow_user` (`slideshowid`,`userid`);
CREATE TABLE `slideshow_usrgrp` (
	`slideshowusrgrpid`      bigint unsigned                           NOT NULL,
	`slideshowid`            bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (slideshowusrgrpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `slideshow_usrgrp_1` ON `slideshow_usrgrp` (`slideshowid`,`usrgrpid`);
CREATE TABLE `slides` (
	`slideid`                bigint unsigned                           NOT NULL,
	`slideshowid`            bigint unsigned                           NOT NULL,
	`screenid`               bigint unsigned                           NOT NULL,
	`step`                   integer         DEFAULT '0'               NOT NULL,
	`delay`                  varchar(32)     DEFAULT '0'               NOT NULL,
	PRIMARY KEY (slideid)
) ENGINE=InnoDB;
CREATE INDEX `slides_1` ON `slides` (`slideshowid`);
CREATE INDEX `slides_2` ON `slides` (`screenid`);
CREATE TABLE `drules` (
	`druleid`                bigint unsigned                           NOT NULL,
	`proxy_hostid`           bigint unsigned                           NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`iprange`                varchar(2048)   DEFAULT ''                NOT NULL,
	`delay`                  varchar(255)    DEFAULT '1h'              NOT NULL,
	`nextcheck`              integer         DEFAULT '0'               NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (druleid)
) ENGINE=InnoDB;
CREATE INDEX `drules_1` ON `drules` (`proxy_hostid`);
CREATE UNIQUE INDEX `drules_2` ON `drules` (`name`);
CREATE TABLE `dchecks` (
	`dcheckid`               bigint unsigned                           NOT NULL,
	`druleid`                bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`key_`                   varchar(512)    DEFAULT ''                NOT NULL,
	`snmp_community`         varchar(255)    DEFAULT ''                NOT NULL,
	`ports`                  varchar(255)    DEFAULT '0'               NOT NULL,
	`snmpv3_securityname`    varchar(64)     DEFAULT ''                NOT NULL,
	`snmpv3_securitylevel`   integer         DEFAULT '0'               NOT NULL,
	`snmpv3_authpassphrase`  varchar(64)     DEFAULT ''                NOT NULL,
	`snmpv3_privpassphrase`  varchar(64)     DEFAULT ''                NOT NULL,
	`uniq`                   integer         DEFAULT '0'               NOT NULL,
	`snmpv3_authprotocol`    integer         DEFAULT '0'               NOT NULL,
	`snmpv3_privprotocol`    integer         DEFAULT '0'               NOT NULL,
	`snmpv3_contextname`     varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (dcheckid)
) ENGINE=InnoDB;
CREATE INDEX `dchecks_1` ON `dchecks` (`druleid`);
CREATE TABLE `applications` (
	`applicationid`          bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (applicationid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `applications_2` ON `applications` (`hostid`,`name`);
CREATE TABLE `httptest` (
	`httptestid`             bigint unsigned                           NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	`applicationid`          bigint unsigned                           NULL,
	`nextcheck`              integer         DEFAULT '0'               NOT NULL,
	`delay`                  varchar(255)    DEFAULT '1m'              NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`agent`                  varchar(255)    DEFAULT 'Zabbix'          NOT NULL,
	`authentication`         integer         DEFAULT '0'               NOT NULL,
	`http_user`              varchar(64)     DEFAULT ''                NOT NULL,
	`http_password`          varchar(64)     DEFAULT ''                NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`http_proxy`             varchar(255)    DEFAULT ''                NOT NULL,
	`retries`                integer         DEFAULT '1'               NOT NULL,
	`ssl_cert_file`          varchar(255)    DEFAULT ''                NOT NULL,
	`ssl_key_file`           varchar(255)    DEFAULT ''                NOT NULL,
	`ssl_key_password`       varchar(64)     DEFAULT ''                NOT NULL,
	`verify_peer`            integer         DEFAULT '0'               NOT NULL,
	`verify_host`            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httptestid)
) ENGINE=InnoDB;
CREATE INDEX `httptest_1` ON `httptest` (`applicationid`);
CREATE UNIQUE INDEX `httptest_2` ON `httptest` (`hostid`,`name`);
CREATE INDEX `httptest_3` ON `httptest` (`status`);
CREATE INDEX `httptest_4` ON `httptest` (`templateid`);
CREATE TABLE `httpstep` (
	`httpstepid`             bigint unsigned                           NOT NULL,
	`httptestid`             bigint unsigned                           NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	`no`                     integer         DEFAULT '0'               NOT NULL,
	`url`                    varchar(2048)   DEFAULT ''                NOT NULL,
	`timeout`                varchar(255)    DEFAULT '15s'             NOT NULL,
	`posts`                  text                                      NOT NULL,
	`required`               varchar(255)    DEFAULT ''                NOT NULL,
	`status_codes`           varchar(255)    DEFAULT ''                NOT NULL,
	`follow_redirects`       integer         DEFAULT '1'               NOT NULL,
	`retrieve_mode`          integer         DEFAULT '0'               NOT NULL,
	`post_type`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httpstepid)
) ENGINE=InnoDB;
CREATE INDEX `httpstep_1` ON `httpstep` (`httptestid`);
CREATE TABLE `interface` (
	`interfaceid`            bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`main`                   integer         DEFAULT '0'               NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`useip`                  integer         DEFAULT '1'               NOT NULL,
	`ip`                     varchar(64)     DEFAULT '127.0.0.1'       NOT NULL,
	`dns`                    varchar(255)    DEFAULT ''                NOT NULL,
	`port`                   varchar(64)     DEFAULT '10050'           NOT NULL,
	`bulk`                   integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (interfaceid)
) ENGINE=InnoDB;
CREATE INDEX `interface_1` ON `interface` (`hostid`,`type`);
CREATE INDEX `interface_2` ON `interface` (`ip`,`dns`);
CREATE TABLE `valuemaps` (
	`valuemapid`             bigint unsigned                           NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (valuemapid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `valuemaps_1` ON `valuemaps` (`name`);
CREATE TABLE `items` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`snmp_community`         varchar(64)     DEFAULT ''                NOT NULL,
	`snmp_oid`               varchar(512)    DEFAULT ''                NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`key_`                   varchar(255)    DEFAULT ''                NOT NULL,
	`delay`                  varchar(1024)   DEFAULT '0'               NOT NULL,
	`history`                varchar(255)    DEFAULT '90d'             NOT NULL,
	`trends`                 varchar(255)    DEFAULT '365d'            NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`value_type`             integer         DEFAULT '0'               NOT NULL,
	`trapper_hosts`          varchar(255)    DEFAULT ''                NOT NULL,
	`units`                  varchar(255)    DEFAULT ''                NOT NULL,
	`snmpv3_securityname`    varchar(64)     DEFAULT ''                NOT NULL,
	`snmpv3_securitylevel`   integer         DEFAULT '0'               NOT NULL,
	`snmpv3_authpassphrase`  varchar(64)     DEFAULT ''                NOT NULL,
	`snmpv3_privpassphrase`  varchar(64)     DEFAULT ''                NOT NULL,
	`formula`                varchar(255)    DEFAULT ''                NOT NULL,
	`error`                  varchar(2048)   DEFAULT ''                NOT NULL,
	`lastlogsize`            bigint unsigned DEFAULT '0'               NOT NULL,
	`logtimefmt`             varchar(64)     DEFAULT ''                NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`valuemapid`             bigint unsigned                           NULL,
	`params`                 text                                      NOT NULL,
	`ipmi_sensor`            varchar(128)    DEFAULT ''                NOT NULL,
	`authtype`               integer         DEFAULT '0'               NOT NULL,
	`username`               varchar(64)     DEFAULT ''                NOT NULL,
	`password`               varchar(64)     DEFAULT ''                NOT NULL,
	`publickey`              varchar(64)     DEFAULT ''                NOT NULL,
	`privatekey`             varchar(64)     DEFAULT ''                NOT NULL,
	`mtime`                  integer         DEFAULT '0'               NOT NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	`interfaceid`            bigint unsigned                           NULL,
	`port`                   varchar(64)     DEFAULT ''                NOT NULL,
	`description`            text                                      NOT NULL,
	`inventory_link`         integer         DEFAULT '0'               NOT NULL,
	`lifetime`               varchar(255)    DEFAULT '30d'             NOT NULL,
	`snmpv3_authprotocol`    integer         DEFAULT '0'               NOT NULL,
	`snmpv3_privprotocol`    integer         DEFAULT '0'               NOT NULL,
	`state`                  integer         DEFAULT '0'               NOT NULL,
	`snmpv3_contextname`     varchar(255)    DEFAULT ''                NOT NULL,
	`evaltype`               integer         DEFAULT '0'               NOT NULL,
	`jmx_endpoint`           varchar(255)    DEFAULT ''                NOT NULL,
	`master_itemid`          bigint unsigned                           NULL,
	`timeout`                varchar(255)    DEFAULT '3s'              NOT NULL,
	`url`                    varchar(2048)   DEFAULT ''                NOT NULL,
	`query_fields`           varchar(2048)   DEFAULT ''                NOT NULL,
	`posts`                  text                                      NOT NULL,
	`status_codes`           varchar(255)    DEFAULT '200'             NOT NULL,
	`follow_redirects`       integer         DEFAULT '1'               NOT NULL,
	`post_type`              integer         DEFAULT '0'               NOT NULL,
	`http_proxy`             varchar(255)    DEFAULT ''                NOT NULL,
	`headers`                text                                      NOT NULL,
	`retrieve_mode`          integer         DEFAULT '0'               NOT NULL,
	`request_method`         integer         DEFAULT '0'               NOT NULL,
	`output_format`          integer         DEFAULT '0'               NOT NULL,
	`ssl_cert_file`          varchar(255)    DEFAULT ''                NOT NULL,
	`ssl_key_file`           varchar(255)    DEFAULT ''                NOT NULL,
	`ssl_key_password`       varchar(64)     DEFAULT ''                NOT NULL,
	`verify_peer`            integer         DEFAULT '0'               NOT NULL,
	`verify_host`            integer         DEFAULT '0'               NOT NULL,
	`allow_traps`            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `items_1` ON `items` (`hostid`,`key_`);
CREATE INDEX `items_3` ON `items` (`status`);
CREATE INDEX `items_4` ON `items` (`templateid`);
CREATE INDEX `items_5` ON `items` (`valuemapid`);
CREATE INDEX `items_6` ON `items` (`interfaceid`);
CREATE INDEX `items_7` ON `items` (`master_itemid`);
CREATE TABLE `httpstepitem` (
	`httpstepitemid`         bigint unsigned                           NOT NULL,
	`httpstepid`             bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httpstepitemid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `httpstepitem_1` ON `httpstepitem` (`httpstepid`,`itemid`);
CREATE INDEX `httpstepitem_2` ON `httpstepitem` (`itemid`);
CREATE TABLE `httptestitem` (
	`httptestitemid`         bigint unsigned                           NOT NULL,
	`httptestid`             bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httptestitemid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `httptestitem_1` ON `httptestitem` (`httptestid`,`itemid`);
CREATE INDEX `httptestitem_2` ON `httptestitem` (`itemid`);
CREATE TABLE `media_type` (
	`mediatypeid`            bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`description`            varchar(100)    DEFAULT ''                NOT NULL,
	`smtp_server`            varchar(255)    DEFAULT ''                NOT NULL,
	`smtp_helo`              varchar(255)    DEFAULT ''                NOT NULL,
	`smtp_email`             varchar(255)    DEFAULT ''                NOT NULL,
	`exec_path`              varchar(255)    DEFAULT ''                NOT NULL,
	`gsm_modem`              varchar(255)    DEFAULT ''                NOT NULL,
	`username`               varchar(255)    DEFAULT ''                NOT NULL,
	`passwd`                 varchar(255)    DEFAULT ''                NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`smtp_port`              integer         DEFAULT '25'              NOT NULL,
	`smtp_security`          integer         DEFAULT '0'               NOT NULL,
	`smtp_verify_peer`       integer         DEFAULT '0'               NOT NULL,
	`smtp_verify_host`       integer         DEFAULT '0'               NOT NULL,
	`smtp_authentication`    integer         DEFAULT '0'               NOT NULL,
	`exec_params`            varchar(255)    DEFAULT ''                NOT NULL,
	`maxsessions`            integer         DEFAULT '1'               NOT NULL,
	`maxattempts`            integer         DEFAULT '3'               NOT NULL,
	`attempt_interval`       varchar(32)     DEFAULT '10s'             NOT NULL,
	PRIMARY KEY (mediatypeid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `media_type_1` ON `media_type` (`description`);
CREATE TABLE `usrgrp` (
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	`gui_access`             integer         DEFAULT '0'               NOT NULL,
	`users_status`           integer         DEFAULT '0'               NOT NULL,
	`debug_mode`             integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (usrgrpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `usrgrp_1` ON `usrgrp` (`name`);
CREATE TABLE `users_groups` (
	`id`                     bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `users_groups_1` ON `users_groups` (`usrgrpid`,`userid`);
CREATE INDEX `users_groups_2` ON `users_groups` (`userid`);
CREATE TABLE `scripts` (
	`scriptid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`command`                varchar(255)    DEFAULT ''                NOT NULL,
	`host_access`            integer         DEFAULT '2'               NOT NULL,
	`usrgrpid`               bigint unsigned                           NULL,
	`groupid`                bigint unsigned                           NULL,
	`description`            text                                      NOT NULL,
	`confirmation`           varchar(255)    DEFAULT ''                NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`execute_on`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (scriptid)
) ENGINE=InnoDB;
CREATE INDEX `scripts_1` ON `scripts` (`usrgrpid`);
CREATE INDEX `scripts_2` ON `scripts` (`groupid`);
CREATE UNIQUE INDEX `scripts_3` ON `scripts` (`name`);
CREATE TABLE `actions` (
	`actionid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`eventsource`            integer         DEFAULT '0'               NOT NULL,
	`evaltype`               integer         DEFAULT '0'               NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`esc_period`             varchar(255)    DEFAULT '1h'              NOT NULL,
	`def_shortdata`          varchar(255)    DEFAULT ''                NOT NULL,
	`def_longdata`           text                                      NOT NULL,
	`r_shortdata`            varchar(255)    DEFAULT ''                NOT NULL,
	`r_longdata`             text                                      NOT NULL,
	`formula`                varchar(255)    DEFAULT ''                NOT NULL,
	`pause_suppressed`       integer         DEFAULT '1'               NOT NULL,
	`ack_shortdata`          varchar(255)    DEFAULT ''                NOT NULL,
	`ack_longdata`           text                                      NOT NULL,
	PRIMARY KEY (actionid)
) ENGINE=InnoDB;
CREATE INDEX `actions_1` ON `actions` (`eventsource`,`status`);
CREATE UNIQUE INDEX `actions_2` ON `actions` (`name`);
CREATE TABLE `operations` (
	`operationid`            bigint unsigned                           NOT NULL,
	`actionid`               bigint unsigned                           NOT NULL,
	`operationtype`          integer         DEFAULT '0'               NOT NULL,
	`esc_period`             varchar(255)    DEFAULT '0'               NOT NULL,
	`esc_step_from`          integer         DEFAULT '1'               NOT NULL,
	`esc_step_to`            integer         DEFAULT '1'               NOT NULL,
	`evaltype`               integer         DEFAULT '0'               NOT NULL,
	`recovery`               integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
CREATE INDEX `operations_1` ON `operations` (`actionid`);
CREATE TABLE `opmessage` (
	`operationid`            bigint unsigned                           NOT NULL,
	`default_msg`            integer         DEFAULT '0'               NOT NULL,
	`subject`                varchar(255)    DEFAULT ''                NOT NULL,
	`message`                text                                      NOT NULL,
	`mediatypeid`            bigint unsigned                           NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
CREATE INDEX `opmessage_1` ON `opmessage` (`mediatypeid`);
CREATE TABLE `opmessage_grp` (
	`opmessage_grpid`        bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	PRIMARY KEY (opmessage_grpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `opmessage_grp_1` ON `opmessage_grp` (`operationid`,`usrgrpid`);
CREATE INDEX `opmessage_grp_2` ON `opmessage_grp` (`usrgrpid`);
CREATE TABLE `opmessage_usr` (
	`opmessage_usrid`        bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (opmessage_usrid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `opmessage_usr_1` ON `opmessage_usr` (`operationid`,`userid`);
CREATE INDEX `opmessage_usr_2` ON `opmessage_usr` (`userid`);
CREATE TABLE `opcommand` (
	`operationid`            bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`scriptid`               bigint unsigned                           NULL,
	`execute_on`             integer         DEFAULT '0'               NOT NULL,
	`port`                   varchar(64)     DEFAULT ''                NOT NULL,
	`authtype`               integer         DEFAULT '0'               NOT NULL,
	`username`               varchar(64)     DEFAULT ''                NOT NULL,
	`password`               varchar(64)     DEFAULT ''                NOT NULL,
	`publickey`              varchar(64)     DEFAULT ''                NOT NULL,
	`privatekey`             varchar(64)     DEFAULT ''                NOT NULL,
	`command`                text                                      NOT NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
CREATE INDEX `opcommand_1` ON `opcommand` (`scriptid`);
CREATE TABLE `opcommand_hst` (
	`opcommand_hstid`        bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NULL,
	PRIMARY KEY (opcommand_hstid)
) ENGINE=InnoDB;
CREATE INDEX `opcommand_hst_1` ON `opcommand_hst` (`operationid`);
CREATE INDEX `opcommand_hst_2` ON `opcommand_hst` (`hostid`);
CREATE TABLE `opcommand_grp` (
	`opcommand_grpid`        bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	PRIMARY KEY (opcommand_grpid)
) ENGINE=InnoDB;
CREATE INDEX `opcommand_grp_1` ON `opcommand_grp` (`operationid`);
CREATE INDEX `opcommand_grp_2` ON `opcommand_grp` (`groupid`);
CREATE TABLE `opgroup` (
	`opgroupid`              bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	PRIMARY KEY (opgroupid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `opgroup_1` ON `opgroup` (`operationid`,`groupid`);
CREATE INDEX `opgroup_2` ON `opgroup` (`groupid`);
CREATE TABLE `optemplate` (
	`optemplateid`           bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`templateid`             bigint unsigned                           NOT NULL,
	PRIMARY KEY (optemplateid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `optemplate_1` ON `optemplate` (`operationid`,`templateid`);
CREATE INDEX `optemplate_2` ON `optemplate` (`templateid`);
CREATE TABLE `opconditions` (
	`opconditionid`          bigint unsigned                           NOT NULL,
	`operationid`            bigint unsigned                           NOT NULL,
	`conditiontype`          integer         DEFAULT '0'               NOT NULL,
	`operator`               integer         DEFAULT '0'               NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (opconditionid)
) ENGINE=InnoDB;
CREATE INDEX `opconditions_1` ON `opconditions` (`operationid`);
CREATE TABLE `conditions` (
	`conditionid`            bigint unsigned                           NOT NULL,
	`actionid`               bigint unsigned                           NOT NULL,
	`conditiontype`          integer         DEFAULT '0'               NOT NULL,
	`operator`               integer         DEFAULT '0'               NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	`value2`                 varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (conditionid)
) ENGINE=InnoDB;
CREATE INDEX `conditions_1` ON `conditions` (`actionid`);
CREATE TABLE `config` (
	`configid`               bigint unsigned                           NOT NULL,
	`refresh_unsupported`    varchar(32)     DEFAULT '10m'             NOT NULL,
	`work_period`            varchar(255)    DEFAULT '1-5,09:00-18:00' NOT NULL,
	`alert_usrgrpid`         bigint unsigned                           NULL,
	`default_theme`          varchar(128)    DEFAULT 'blue-theme'      NOT NULL,
	`authentication_type`    integer         DEFAULT '0'               NOT NULL,
	`ldap_host`              varchar(255)    DEFAULT ''                NOT NULL,
	`ldap_port`              integer         DEFAULT 389               NOT NULL,
	`ldap_base_dn`           varchar(255)    DEFAULT ''                NOT NULL,
	`ldap_bind_dn`           varchar(255)    DEFAULT ''                NOT NULL,
	`ldap_bind_password`     varchar(128)    DEFAULT ''                NOT NULL,
	`ldap_search_attribute`  varchar(128)    DEFAULT ''                NOT NULL,
	`dropdown_first_entry`   integer         DEFAULT '1'               NOT NULL,
	`dropdown_first_remember` integer         DEFAULT '1'               NOT NULL,
	`discovery_groupid`      bigint unsigned                           NOT NULL,
	`max_in_table`           integer         DEFAULT '50'              NOT NULL,
	`search_limit`           integer         DEFAULT '1000'            NOT NULL,
	`severity_color_0`       varchar(6)      DEFAULT '97AAB3'          NOT NULL,
	`severity_color_1`       varchar(6)      DEFAULT '7499FF'          NOT NULL,
	`severity_color_2`       varchar(6)      DEFAULT 'FFC859'          NOT NULL,
	`severity_color_3`       varchar(6)      DEFAULT 'FFA059'          NOT NULL,
	`severity_color_4`       varchar(6)      DEFAULT 'E97659'          NOT NULL,
	`severity_color_5`       varchar(6)      DEFAULT 'E45959'          NOT NULL,
	`severity_name_0`        varchar(32)     DEFAULT 'Not classified'  NOT NULL,
	`severity_name_1`        varchar(32)     DEFAULT 'Information'     NOT NULL,
	`severity_name_2`        varchar(32)     DEFAULT 'Warning'         NOT NULL,
	`severity_name_3`        varchar(32)     DEFAULT 'Average'         NOT NULL,
	`severity_name_4`        varchar(32)     DEFAULT 'High'            NOT NULL,
	`severity_name_5`        varchar(32)     DEFAULT 'Disaster'        NOT NULL,
	`ok_period`              varchar(32)     DEFAULT '5m'              NOT NULL,
	`blink_period`           varchar(32)     DEFAULT '2m'              NOT NULL,
	`problem_unack_color`    varchar(6)      DEFAULT 'CC0000'          NOT NULL,
	`problem_ack_color`      varchar(6)      DEFAULT 'CC0000'          NOT NULL,
	`ok_unack_color`         varchar(6)      DEFAULT '009900'          NOT NULL,
	`ok_ack_color`           varchar(6)      DEFAULT '009900'          NOT NULL,
	`problem_unack_style`    integer         DEFAULT '1'               NOT NULL,
	`problem_ack_style`      integer         DEFAULT '1'               NOT NULL,
	`ok_unack_style`         integer         DEFAULT '1'               NOT NULL,
	`ok_ack_style`           integer         DEFAULT '1'               NOT NULL,
	`snmptrap_logging`       integer         DEFAULT '1'               NOT NULL,
	`server_check_interval`  integer         DEFAULT '10'              NOT NULL,
	`hk_events_mode`         integer         DEFAULT '1'               NOT NULL,
	`hk_events_trigger`      varchar(32)     DEFAULT '365d'            NOT NULL,
	`hk_events_internal`     varchar(32)     DEFAULT '1d'              NOT NULL,
	`hk_events_discovery`    varchar(32)     DEFAULT '1d'              NOT NULL,
	`hk_events_autoreg`      varchar(32)     DEFAULT '1d'              NOT NULL,
	`hk_services_mode`       integer         DEFAULT '1'               NOT NULL,
	`hk_services`            varchar(32)     DEFAULT '365d'            NOT NULL,
	`hk_audit_mode`          integer         DEFAULT '1'               NOT NULL,
	`hk_audit`               varchar(32)     DEFAULT '365d'            NOT NULL,
	`hk_sessions_mode`       integer         DEFAULT '1'               NOT NULL,
	`hk_sessions`            varchar(32)     DEFAULT '365d'            NOT NULL,
	`hk_history_mode`        integer         DEFAULT '1'               NOT NULL,
	`hk_history_global`      integer         DEFAULT '0'               NOT NULL,
	`hk_history`             varchar(32)     DEFAULT '90d'             NOT NULL,
	`hk_trends_mode`         integer         DEFAULT '1'               NOT NULL,
	`hk_trends_global`       integer         DEFAULT '0'               NOT NULL,
	`hk_trends`              varchar(32)     DEFAULT '365d'            NOT NULL,
	`default_inventory_mode` integer         DEFAULT '-1'              NOT NULL,
	`custom_color`           integer         DEFAULT '0'               NOT NULL,
	`http_auth_enabled`      integer         DEFAULT '0'               NOT NULL,
	`http_login_form`        integer         DEFAULT '0'               NOT NULL,
	`http_strip_domains`     varchar(2048)   DEFAULT ''                NOT NULL,
	`http_case_sensitive`    integer         DEFAULT '1'               NOT NULL,
	`ldap_configured`        integer         DEFAULT '0'               NOT NULL,
	`ldap_case_sensitive`    integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (configid)
) ENGINE=InnoDB;
CREATE INDEX `config_1` ON `config` (`alert_usrgrpid`);
CREATE INDEX `config_2` ON `config` (`discovery_groupid`);
CREATE TABLE `triggers` (
	`triggerid`              bigint unsigned                           NOT NULL,
	`expression`             varchar(2048)   DEFAULT ''                NOT NULL,
	`description`            varchar(255)    DEFAULT ''                NOT NULL,
	`url`                    varchar(255)    DEFAULT ''                NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`value`                  integer         DEFAULT '0'               NOT NULL,
	`priority`               integer         DEFAULT '0'               NOT NULL,
	`lastchange`             integer         DEFAULT '0'               NOT NULL,
	`comments`               text                                      NOT NULL,
	`error`                  varchar(2048)   DEFAULT ''                NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`state`                  integer         DEFAULT '0'               NOT NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	`recovery_mode`          integer         DEFAULT '0'               NOT NULL,
	`recovery_expression`    varchar(2048)   DEFAULT ''                NOT NULL,
	`correlation_mode`       integer         DEFAULT '0'               NOT NULL,
	`correlation_tag`        varchar(255)    DEFAULT ''                NOT NULL,
	`manual_close`           integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (triggerid)
) ENGINE=InnoDB;
CREATE INDEX `triggers_1` ON `triggers` (`status`);
CREATE INDEX `triggers_2` ON `triggers` (`value`,`lastchange`);
CREATE INDEX `triggers_3` ON `triggers` (`templateid`);
CREATE TABLE `trigger_depends` (
	`triggerdepid`           bigint unsigned                           NOT NULL,
	`triggerid_down`         bigint unsigned                           NOT NULL,
	`triggerid_up`           bigint unsigned                           NOT NULL,
	PRIMARY KEY (triggerdepid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `trigger_depends_1` ON `trigger_depends` (`triggerid_down`,`triggerid_up`);
CREATE INDEX `trigger_depends_2` ON `trigger_depends` (`triggerid_up`);
CREATE TABLE `functions` (
	`functionid`             bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`triggerid`              bigint unsigned                           NOT NULL,
	`name`                   varchar(12)     DEFAULT ''                NOT NULL,
	`parameter`              varchar(255)    DEFAULT '0'               NOT NULL,
	PRIMARY KEY (functionid)
) ENGINE=InnoDB;
CREATE INDEX `functions_1` ON `functions` (`triggerid`);
CREATE INDEX `functions_2` ON `functions` (`itemid`,`name`,`parameter`);
CREATE TABLE `graphs` (
	`graphid`                bigint unsigned                           NOT NULL,
	`name`                   varchar(128)    DEFAULT ''                NOT NULL,
	`width`                  integer         DEFAULT '900'             NOT NULL,
	`height`                 integer         DEFAULT '200'             NOT NULL,
	`yaxismin`               double(16,4)    DEFAULT '0'               NOT NULL,
	`yaxismax`               double(16,4)    DEFAULT '100'             NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`show_work_period`       integer         DEFAULT '1'               NOT NULL,
	`show_triggers`          integer         DEFAULT '1'               NOT NULL,
	`graphtype`              integer         DEFAULT '0'               NOT NULL,
	`show_legend`            integer         DEFAULT '1'               NOT NULL,
	`show_3d`                integer         DEFAULT '0'               NOT NULL,
	`percent_left`           double(16,4)    DEFAULT '0'               NOT NULL,
	`percent_right`          double(16,4)    DEFAULT '0'               NOT NULL,
	`ymin_type`              integer         DEFAULT '0'               NOT NULL,
	`ymax_type`              integer         DEFAULT '0'               NOT NULL,
	`ymin_itemid`            bigint unsigned                           NULL,
	`ymax_itemid`            bigint unsigned                           NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (graphid)
) ENGINE=InnoDB;
CREATE INDEX `graphs_1` ON `graphs` (`name`);
CREATE INDEX `graphs_2` ON `graphs` (`templateid`);
CREATE INDEX `graphs_3` ON `graphs` (`ymin_itemid`);
CREATE INDEX `graphs_4` ON `graphs` (`ymax_itemid`);
CREATE TABLE `graphs_items` (
	`gitemid`                bigint unsigned                           NOT NULL,
	`graphid`                bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`drawtype`               integer         DEFAULT '0'               NOT NULL,
	`sortorder`              integer         DEFAULT '0'               NOT NULL,
	`color`                  varchar(6)      DEFAULT '009600'          NOT NULL,
	`yaxisside`              integer         DEFAULT '0'               NOT NULL,
	`calc_fnc`               integer         DEFAULT '2'               NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (gitemid)
) ENGINE=InnoDB;
CREATE INDEX `graphs_items_1` ON `graphs_items` (`itemid`);
CREATE INDEX `graphs_items_2` ON `graphs_items` (`graphid`);
CREATE TABLE `graph_theme` (
	`graphthemeid`           bigint unsigned                           NOT NULL,
	`theme`                  varchar(64)     DEFAULT ''                NOT NULL,
	`backgroundcolor`        varchar(6)      DEFAULT ''                NOT NULL,
	`graphcolor`             varchar(6)      DEFAULT ''                NOT NULL,
	`gridcolor`              varchar(6)      DEFAULT ''                NOT NULL,
	`maingridcolor`          varchar(6)      DEFAULT ''                NOT NULL,
	`gridbordercolor`        varchar(6)      DEFAULT ''                NOT NULL,
	`textcolor`              varchar(6)      DEFAULT ''                NOT NULL,
	`highlightcolor`         varchar(6)      DEFAULT ''                NOT NULL,
	`leftpercentilecolor`    varchar(6)      DEFAULT ''                NOT NULL,
	`rightpercentilecolor`   varchar(6)      DEFAULT ''                NOT NULL,
	`nonworktimecolor`       varchar(6)      DEFAULT ''                NOT NULL,
	`colorpalette`           varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (graphthemeid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `graph_theme_1` ON `graph_theme` (`theme`);
CREATE TABLE `globalmacro` (
	`globalmacroid`          bigint unsigned                           NOT NULL,
	`macro`                  varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (globalmacroid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `globalmacro_1` ON `globalmacro` (`macro`);
CREATE TABLE `hostmacro` (
	`hostmacroid`            bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`macro`                  varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (hostmacroid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `hostmacro_1` ON `hostmacro` (`hostid`,`macro`);
CREATE TABLE `hosts_groups` (
	`hostgroupid`            bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	PRIMARY KEY (hostgroupid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `hosts_groups_1` ON `hosts_groups` (`hostid`,`groupid`);
CREATE INDEX `hosts_groups_2` ON `hosts_groups` (`groupid`);
CREATE TABLE `hosts_templates` (
	`hosttemplateid`         bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	`templateid`             bigint unsigned                           NOT NULL,
	PRIMARY KEY (hosttemplateid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `hosts_templates_1` ON `hosts_templates` (`hostid`,`templateid`);
CREATE INDEX `hosts_templates_2` ON `hosts_templates` (`templateid`);
CREATE TABLE `items_applications` (
	`itemappid`              bigint unsigned                           NOT NULL,
	`applicationid`          bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (itemappid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `items_applications_1` ON `items_applications` (`applicationid`,`itemid`);
CREATE INDEX `items_applications_2` ON `items_applications` (`itemid`);
CREATE TABLE `mappings` (
	`mappingid`              bigint unsigned                           NOT NULL,
	`valuemapid`             bigint unsigned                           NOT NULL,
	`value`                  varchar(64)     DEFAULT ''                NOT NULL,
	`newvalue`               varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (mappingid)
) ENGINE=InnoDB;
CREATE INDEX `mappings_1` ON `mappings` (`valuemapid`);
CREATE TABLE `media` (
	`mediaid`                bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`mediatypeid`            bigint unsigned                           NOT NULL,
	`sendto`                 varchar(1024)   DEFAULT ''                NOT NULL,
	`active`                 integer         DEFAULT '0'               NOT NULL,
	`severity`               integer         DEFAULT '63'              NOT NULL,
	`period`                 varchar(1024)   DEFAULT '1-7,00:00-24:00' NOT NULL,
	PRIMARY KEY (mediaid)
) ENGINE=InnoDB;
CREATE INDEX `media_1` ON `media` (`userid`);
CREATE INDEX `media_2` ON `media` (`mediatypeid`);
CREATE TABLE `rights` (
	`rightid`                bigint unsigned                           NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '0'               NOT NULL,
	`id`                     bigint unsigned                           NOT NULL,
	PRIMARY KEY (rightid)
) ENGINE=InnoDB;
CREATE INDEX `rights_1` ON `rights` (`groupid`);
CREATE INDEX `rights_2` ON `rights` (`id`);
CREATE TABLE `services` (
	`serviceid`              bigint unsigned                           NOT NULL,
	`name`                   varchar(128)    DEFAULT ''                NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`algorithm`              integer         DEFAULT '0'               NOT NULL,
	`triggerid`              bigint unsigned                           NULL,
	`showsla`                integer         DEFAULT '0'               NOT NULL,
	`goodsla`                double(16,4)    DEFAULT '99.9'            NOT NULL,
	`sortorder`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (serviceid)
) ENGINE=InnoDB;
CREATE INDEX `services_1` ON `services` (`triggerid`);
CREATE TABLE `services_links` (
	`linkid`                 bigint unsigned                           NOT NULL,
	`serviceupid`            bigint unsigned                           NOT NULL,
	`servicedownid`          bigint unsigned                           NOT NULL,
	`soft`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (linkid)
) ENGINE=InnoDB;
CREATE INDEX `services_links_1` ON `services_links` (`servicedownid`);
CREATE UNIQUE INDEX `services_links_2` ON `services_links` (`serviceupid`,`servicedownid`);
CREATE TABLE `services_times` (
	`timeid`                 bigint unsigned                           NOT NULL,
	`serviceid`              bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`ts_from`                integer         DEFAULT '0'               NOT NULL,
	`ts_to`                  integer         DEFAULT '0'               NOT NULL,
	`note`                   varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (timeid)
) ENGINE=InnoDB;
CREATE INDEX `services_times_1` ON `services_times` (`serviceid`,`type`,`ts_from`,`ts_to`);
CREATE TABLE `icon_map` (
	`iconmapid`              bigint unsigned                           NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	`default_iconid`         bigint unsigned                           NOT NULL,
	PRIMARY KEY (iconmapid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `icon_map_1` ON `icon_map` (`name`);
CREATE INDEX `icon_map_2` ON `icon_map` (`default_iconid`);
CREATE TABLE `icon_mapping` (
	`iconmappingid`          bigint unsigned                           NOT NULL,
	`iconmapid`              bigint unsigned                           NOT NULL,
	`iconid`                 bigint unsigned                           NOT NULL,
	`inventory_link`         integer         DEFAULT '0'               NOT NULL,
	`expression`             varchar(64)     DEFAULT ''                NOT NULL,
	`sortorder`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (iconmappingid)
) ENGINE=InnoDB;
CREATE INDEX `icon_mapping_1` ON `icon_mapping` (`iconmapid`);
CREATE INDEX `icon_mapping_2` ON `icon_mapping` (`iconid`);
CREATE TABLE `sysmaps` (
	`sysmapid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(128)    DEFAULT ''                NOT NULL,
	`width`                  integer         DEFAULT '600'             NOT NULL,
	`height`                 integer         DEFAULT '400'             NOT NULL,
	`backgroundid`           bigint unsigned                           NULL,
	`label_type`             integer         DEFAULT '2'               NOT NULL,
	`label_location`         integer         DEFAULT '0'               NOT NULL,
	`highlight`              integer         DEFAULT '1'               NOT NULL,
	`expandproblem`          integer         DEFAULT '1'               NOT NULL,
	`markelements`           integer         DEFAULT '0'               NOT NULL,
	`show_unack`             integer         DEFAULT '0'               NOT NULL,
	`grid_size`              integer         DEFAULT '50'              NOT NULL,
	`grid_show`              integer         DEFAULT '1'               NOT NULL,
	`grid_align`             integer         DEFAULT '1'               NOT NULL,
	`label_format`           integer         DEFAULT '0'               NOT NULL,
	`label_type_host`        integer         DEFAULT '2'               NOT NULL,
	`label_type_hostgroup`   integer         DEFAULT '2'               NOT NULL,
	`label_type_trigger`     integer         DEFAULT '2'               NOT NULL,
	`label_type_map`         integer         DEFAULT '2'               NOT NULL,
	`label_type_image`       integer         DEFAULT '2'               NOT NULL,
	`label_string_host`      varchar(255)    DEFAULT ''                NOT NULL,
	`label_string_hostgroup` varchar(255)    DEFAULT ''                NOT NULL,
	`label_string_trigger`   varchar(255)    DEFAULT ''                NOT NULL,
	`label_string_map`       varchar(255)    DEFAULT ''                NOT NULL,
	`label_string_image`     varchar(255)    DEFAULT ''                NOT NULL,
	`iconmapid`              bigint unsigned                           NULL,
	`expand_macros`          integer         DEFAULT '0'               NOT NULL,
	`severity_min`           integer         DEFAULT '0'               NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`private`                integer         DEFAULT '1'               NOT NULL,
	`show_suppressed`        integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmaps_1` ON `sysmaps` (`name`);
CREATE INDEX `sysmaps_2` ON `sysmaps` (`backgroundid`);
CREATE INDEX `sysmaps_3` ON `sysmaps` (`iconmapid`);
CREATE TABLE `sysmaps_elements` (
	`selementid`             bigint unsigned                           NOT NULL,
	`sysmapid`               bigint unsigned                           NOT NULL,
	`elementid`              bigint unsigned DEFAULT '0'               NOT NULL,
	`elementtype`            integer         DEFAULT '0'               NOT NULL,
	`iconid_off`             bigint unsigned                           NULL,
	`iconid_on`              bigint unsigned                           NULL,
	`label`                  varchar(2048)   DEFAULT ''                NOT NULL,
	`label_location`         integer         DEFAULT '-1'              NOT NULL,
	`x`                      integer         DEFAULT '0'               NOT NULL,
	`y`                      integer         DEFAULT '0'               NOT NULL,
	`iconid_disabled`        bigint unsigned                           NULL,
	`iconid_maintenance`     bigint unsigned                           NULL,
	`elementsubtype`         integer         DEFAULT '0'               NOT NULL,
	`areatype`               integer         DEFAULT '0'               NOT NULL,
	`width`                  integer         DEFAULT '200'             NOT NULL,
	`height`                 integer         DEFAULT '200'             NOT NULL,
	`viewtype`               integer         DEFAULT '0'               NOT NULL,
	`use_iconmap`            integer         DEFAULT '1'               NOT NULL,
	`application`            varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (selementid)
) ENGINE=InnoDB;
CREATE INDEX `sysmaps_elements_1` ON `sysmaps_elements` (`sysmapid`);
CREATE INDEX `sysmaps_elements_2` ON `sysmaps_elements` (`iconid_off`);
CREATE INDEX `sysmaps_elements_3` ON `sysmaps_elements` (`iconid_on`);
CREATE INDEX `sysmaps_elements_4` ON `sysmaps_elements` (`iconid_disabled`);
CREATE INDEX `sysmaps_elements_5` ON `sysmaps_elements` (`iconid_maintenance`);
CREATE TABLE `sysmaps_links` (
	`linkid`                 bigint unsigned                           NOT NULL,
	`sysmapid`               bigint unsigned                           NOT NULL,
	`selementid1`            bigint unsigned                           NOT NULL,
	`selementid2`            bigint unsigned                           NOT NULL,
	`drawtype`               integer         DEFAULT '0'               NOT NULL,
	`color`                  varchar(6)      DEFAULT '000000'          NOT NULL,
	`label`                  varchar(2048)   DEFAULT ''                NOT NULL,
	PRIMARY KEY (linkid)
) ENGINE=InnoDB;
CREATE INDEX `sysmaps_links_1` ON `sysmaps_links` (`sysmapid`);
CREATE INDEX `sysmaps_links_2` ON `sysmaps_links` (`selementid1`);
CREATE INDEX `sysmaps_links_3` ON `sysmaps_links` (`selementid2`);
CREATE TABLE `sysmaps_link_triggers` (
	`linktriggerid`          bigint unsigned                           NOT NULL,
	`linkid`                 bigint unsigned                           NOT NULL,
	`triggerid`              bigint unsigned                           NOT NULL,
	`drawtype`               integer         DEFAULT '0'               NOT NULL,
	`color`                  varchar(6)      DEFAULT '000000'          NOT NULL,
	PRIMARY KEY (linktriggerid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmaps_link_triggers_1` ON `sysmaps_link_triggers` (`linkid`,`triggerid`);
CREATE INDEX `sysmaps_link_triggers_2` ON `sysmaps_link_triggers` (`triggerid`);
CREATE TABLE `sysmap_element_url` (
	`sysmapelementurlid`     bigint unsigned                           NOT NULL,
	`selementid`             bigint unsigned                           NOT NULL,
	`name`                   varchar(255)                              NOT NULL,
	`url`                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (sysmapelementurlid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmap_element_url_1` ON `sysmap_element_url` (`selementid`,`name`);
CREATE TABLE `sysmap_url` (
	`sysmapurlid`            bigint unsigned                           NOT NULL,
	`sysmapid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(255)                              NOT NULL,
	`url`                    varchar(255)    DEFAULT ''                NOT NULL,
	`elementtype`            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapurlid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmap_url_1` ON `sysmap_url` (`sysmapid`,`name`);
CREATE TABLE `sysmap_user` (
	`sysmapuserid`           bigint unsigned                           NOT NULL,
	`sysmapid`               bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (sysmapuserid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmap_user_1` ON `sysmap_user` (`sysmapid`,`userid`);
CREATE TABLE `sysmap_usrgrp` (
	`sysmapusrgrpid`         bigint unsigned                           NOT NULL,
	`sysmapid`               bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (sysmapusrgrpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmap_usrgrp_1` ON `sysmap_usrgrp` (`sysmapid`,`usrgrpid`);
CREATE TABLE `maintenances_hosts` (
	`maintenance_hostid`     bigint unsigned                           NOT NULL,
	`maintenanceid`          bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (maintenance_hostid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `maintenances_hosts_1` ON `maintenances_hosts` (`maintenanceid`,`hostid`);
CREATE INDEX `maintenances_hosts_2` ON `maintenances_hosts` (`hostid`);
CREATE TABLE `maintenances_groups` (
	`maintenance_groupid`    bigint unsigned                           NOT NULL,
	`maintenanceid`          bigint unsigned                           NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	PRIMARY KEY (maintenance_groupid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `maintenances_groups_1` ON `maintenances_groups` (`maintenanceid`,`groupid`);
CREATE INDEX `maintenances_groups_2` ON `maintenances_groups` (`groupid`);
CREATE TABLE `timeperiods` (
	`timeperiodid`           bigint unsigned                           NOT NULL,
	`timeperiod_type`        integer         DEFAULT '0'               NOT NULL,
	`every`                  integer         DEFAULT '1'               NOT NULL,
	`month`                  integer         DEFAULT '0'               NOT NULL,
	`dayofweek`              integer         DEFAULT '0'               NOT NULL,
	`day`                    integer         DEFAULT '0'               NOT NULL,
	`start_time`             integer         DEFAULT '0'               NOT NULL,
	`period`                 integer         DEFAULT '0'               NOT NULL,
	`start_date`             integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (timeperiodid)
) ENGINE=InnoDB;
CREATE TABLE `maintenances_windows` (
	`maintenance_timeperiodid` bigint unsigned                           NOT NULL,
	`maintenanceid`          bigint unsigned                           NOT NULL,
	`timeperiodid`           bigint unsigned                           NOT NULL,
	PRIMARY KEY (maintenance_timeperiodid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `maintenances_windows_1` ON `maintenances_windows` (`maintenanceid`,`timeperiodid`);
CREATE INDEX `maintenances_windows_2` ON `maintenances_windows` (`timeperiodid`);
CREATE TABLE `regexps` (
	`regexpid`               bigint unsigned                           NOT NULL,
	`name`                   varchar(128)    DEFAULT ''                NOT NULL,
	`test_string`            text                                      NOT NULL,
	PRIMARY KEY (regexpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `regexps_1` ON `regexps` (`name`);
CREATE TABLE `expressions` (
	`expressionid`           bigint unsigned                           NOT NULL,
	`regexpid`               bigint unsigned                           NOT NULL,
	`expression`             varchar(255)    DEFAULT ''                NOT NULL,
	`expression_type`        integer         DEFAULT '0'               NOT NULL,
	`exp_delimiter`          varchar(1)      DEFAULT ''                NOT NULL,
	`case_sensitive`         integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (expressionid)
) ENGINE=InnoDB;
CREATE INDEX `expressions_1` ON `expressions` (`regexpid`);
CREATE TABLE `ids` (
	`table_name`             varchar(64)     DEFAULT ''                NOT NULL,
	`field_name`             varchar(64)     DEFAULT ''                NOT NULL,
	`nextid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (table_name,field_name)
) ENGINE=InnoDB;
CREATE TABLE `alerts` (
	`alertid`                bigint unsigned                           NOT NULL,
	`actionid`               bigint unsigned                           NOT NULL,
	`eventid`                bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`mediatypeid`            bigint unsigned                           NULL,
	`sendto`                 varchar(1024)   DEFAULT ''                NOT NULL,
	`subject`                varchar(255)    DEFAULT ''                NOT NULL,
	`message`                text                                      NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`retries`                integer         DEFAULT '0'               NOT NULL,
	`error`                  varchar(2048)   DEFAULT ''                NOT NULL,
	`esc_step`               integer         DEFAULT '0'               NOT NULL,
	`alerttype`              integer         DEFAULT '0'               NOT NULL,
	`p_eventid`              bigint unsigned                           NULL,
	`acknowledgeid`          bigint unsigned                           NULL,
	PRIMARY KEY (alertid)
) ENGINE=InnoDB;
CREATE INDEX `alerts_1` ON `alerts` (`actionid`);
CREATE INDEX `alerts_2` ON `alerts` (`clock`);
CREATE INDEX `alerts_3` ON `alerts` (`eventid`);
CREATE INDEX `alerts_4` ON `alerts` (`status`);
CREATE INDEX `alerts_5` ON `alerts` (`mediatypeid`);
CREATE INDEX `alerts_6` ON `alerts` (`userid`);
CREATE INDEX `alerts_7` ON `alerts` (`p_eventid`);
CREATE TABLE `history` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`value`                  double(16,4)    DEFAULT '0.0000'          NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL
) ENGINE=InnoDB;
CREATE INDEX `history_1` ON `history` (`itemid`,`clock`);
CREATE TABLE `history_uint` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`value`                  bigint unsigned DEFAULT '0'               NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL
) ENGINE=InnoDB;
CREATE INDEX `history_uint_1` ON `history_uint` (`itemid`,`clock`);
CREATE TABLE `history_str` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL
) ENGINE=InnoDB;
CREATE INDEX `history_str_1` ON `history_str` (`itemid`,`clock`);
CREATE TABLE `history_log` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`timestamp`              integer         DEFAULT '0'               NOT NULL,
	`source`                 varchar(64)     DEFAULT ''                NOT NULL,
	`severity`               integer         DEFAULT '0'               NOT NULL,
	`value`                  text                                      NOT NULL,
	`logeventid`             integer         DEFAULT '0'               NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL
) ENGINE=InnoDB;
CREATE INDEX `history_log_1` ON `history_log` (`itemid`,`clock`);
CREATE TABLE `history_text` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`value`                  text                                      NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL
) ENGINE=InnoDB;
CREATE INDEX `history_text_1` ON `history_text` (`itemid`,`clock`);
CREATE TABLE `proxy_history` (
	`id`                     bigint unsigned                           NOT NULL auto_increment,
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`timestamp`              integer         DEFAULT '0'               NOT NULL,
	`source`                 varchar(64)     DEFAULT ''                NOT NULL,
	`severity`               integer         DEFAULT '0'               NOT NULL,
	`value`                  longtext                                  NOT NULL,
	`logeventid`             integer         DEFAULT '0'               NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL,
	`state`                  integer         DEFAULT '0'               NOT NULL,
	`lastlogsize`            bigint unsigned DEFAULT '0'               NOT NULL,
	`mtime`                  integer         DEFAULT '0'               NOT NULL,
	`flags`                  integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE INDEX `proxy_history_1` ON `proxy_history` (`clock`);
CREATE TABLE `proxy_dhistory` (
	`id`                     bigint unsigned                           NOT NULL auto_increment,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`druleid`                bigint unsigned                           NOT NULL,
	`ip`                     varchar(39)     DEFAULT ''                NOT NULL,
	`port`                   integer         DEFAULT '0'               NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`dcheckid`               bigint unsigned                           NULL,
	`dns`                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE INDEX `proxy_dhistory_1` ON `proxy_dhistory` (`clock`);
CREATE TABLE `events` (
	`eventid`                bigint unsigned                           NOT NULL,
	`source`                 integer         DEFAULT '0'               NOT NULL,
	`object`                 integer         DEFAULT '0'               NOT NULL,
	`objectid`               bigint unsigned DEFAULT '0'               NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`value`                  integer         DEFAULT '0'               NOT NULL,
	`acknowledged`           integer         DEFAULT '0'               NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL,
	`name`                   varchar(2048)   DEFAULT ''                NOT NULL,
	`severity`               integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (eventid)
) ENGINE=InnoDB;
CREATE INDEX `events_1` ON `events` (`source`,`object`,`objectid`,`clock`);
CREATE INDEX `events_2` ON `events` (`source`,`object`,`clock`);
CREATE TABLE `trends` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`num`                    integer         DEFAULT '0'               NOT NULL,
	`value_min`              double(16,4)    DEFAULT '0.0000'          NOT NULL,
	`value_avg`              double(16,4)    DEFAULT '0.0000'          NOT NULL,
	`value_max`              double(16,4)    DEFAULT '0.0000'          NOT NULL,
	PRIMARY KEY (itemid,clock)
) ENGINE=InnoDB;
CREATE TABLE `trends_uint` (
	`itemid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`num`                    integer         DEFAULT '0'               NOT NULL,
	`value_min`              bigint unsigned DEFAULT '0'               NOT NULL,
	`value_avg`              bigint unsigned DEFAULT '0'               NOT NULL,
	`value_max`              bigint unsigned DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock)
) ENGINE=InnoDB;
CREATE TABLE `acknowledges` (
	`acknowledgeid`          bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`eventid`                bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`message`                varchar(255)    DEFAULT ''                NOT NULL,
	`action`                 integer         DEFAULT '0'               NOT NULL,
	`old_severity`           integer         DEFAULT '0'               NOT NULL,
	`new_severity`           integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (acknowledgeid)
) ENGINE=InnoDB;
CREATE INDEX `acknowledges_1` ON `acknowledges` (`userid`);
CREATE INDEX `acknowledges_2` ON `acknowledges` (`eventid`);
CREATE INDEX `acknowledges_3` ON `acknowledges` (`clock`);
CREATE TABLE `auditlog` (
	`auditid`                bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`action`                 integer         DEFAULT '0'               NOT NULL,
	`resourcetype`           integer         DEFAULT '0'               NOT NULL,
	`details`                varchar(128)    DEFAULT '0'               NOT NULL,
	`ip`                     varchar(39)     DEFAULT ''                NOT NULL,
	`resourceid`             bigint unsigned DEFAULT '0'               NOT NULL,
	`resourcename`           varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (auditid)
) ENGINE=InnoDB;
CREATE INDEX `auditlog_1` ON `auditlog` (`userid`,`clock`);
CREATE INDEX `auditlog_2` ON `auditlog` (`clock`);
CREATE TABLE `auditlog_details` (
	`auditdetailid`          bigint unsigned                           NOT NULL,
	`auditid`                bigint unsigned                           NOT NULL,
	`table_name`             varchar(64)     DEFAULT ''                NOT NULL,
	`field_name`             varchar(64)     DEFAULT ''                NOT NULL,
	`oldvalue`               text                                      NOT NULL,
	`newvalue`               text                                      NOT NULL,
	PRIMARY KEY (auditdetailid)
) ENGINE=InnoDB;
CREATE INDEX `auditlog_details_1` ON `auditlog_details` (`auditid`);
CREATE TABLE `service_alarms` (
	`servicealarmid`         bigint unsigned                           NOT NULL,
	`serviceid`              bigint unsigned                           NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`value`                  integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (servicealarmid)
) ENGINE=InnoDB;
CREATE INDEX `service_alarms_1` ON `service_alarms` (`serviceid`,`clock`);
CREATE INDEX `service_alarms_2` ON `service_alarms` (`clock`);
CREATE TABLE `autoreg_host` (
	`autoreg_hostid`         bigint unsigned                           NOT NULL,
	`proxy_hostid`           bigint unsigned                           NULL,
	`host`                   varchar(64)     DEFAULT ''                NOT NULL,
	`listen_ip`              varchar(39)     DEFAULT ''                NOT NULL,
	`listen_port`            integer         DEFAULT '0'               NOT NULL,
	`listen_dns`             varchar(255)    DEFAULT ''                NOT NULL,
	`host_metadata`          varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (autoreg_hostid)
) ENGINE=InnoDB;
CREATE INDEX `autoreg_host_1` ON `autoreg_host` (`host`);
CREATE INDEX `autoreg_host_2` ON `autoreg_host` (`proxy_hostid`);
CREATE TABLE `proxy_autoreg_host` (
	`id`                     bigint unsigned                           NOT NULL auto_increment,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`host`                   varchar(64)     DEFAULT ''                NOT NULL,
	`listen_ip`              varchar(39)     DEFAULT ''                NOT NULL,
	`listen_port`            integer         DEFAULT '0'               NOT NULL,
	`listen_dns`             varchar(255)    DEFAULT ''                NOT NULL,
	`host_metadata`          varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (id)
) ENGINE=InnoDB;
CREATE INDEX `proxy_autoreg_host_1` ON `proxy_autoreg_host` (`clock`);
CREATE TABLE `dhosts` (
	`dhostid`                bigint unsigned                           NOT NULL,
	`druleid`                bigint unsigned                           NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`lastup`                 integer         DEFAULT '0'               NOT NULL,
	`lastdown`               integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dhostid)
) ENGINE=InnoDB;
CREATE INDEX `dhosts_1` ON `dhosts` (`druleid`);
CREATE TABLE `dservices` (
	`dserviceid`             bigint unsigned                           NOT NULL,
	`dhostid`                bigint unsigned                           NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	`port`                   integer         DEFAULT '0'               NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`lastup`                 integer         DEFAULT '0'               NOT NULL,
	`lastdown`               integer         DEFAULT '0'               NOT NULL,
	`dcheckid`               bigint unsigned                           NOT NULL,
	`ip`                     varchar(39)     DEFAULT ''                NOT NULL,
	`dns`                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (dserviceid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `dservices_1` ON `dservices` (`dcheckid`,`ip`,`port`);
CREATE INDEX `dservices_2` ON `dservices` (`dhostid`);
CREATE TABLE `escalations` (
	`escalationid`           bigint unsigned                           NOT NULL,
	`actionid`               bigint unsigned                           NOT NULL,
	`triggerid`              bigint unsigned                           NULL,
	`eventid`                bigint unsigned                           NULL,
	`r_eventid`              bigint unsigned                           NULL,
	`nextcheck`              integer         DEFAULT '0'               NOT NULL,
	`esc_step`               integer         DEFAULT '0'               NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`itemid`                 bigint unsigned                           NULL,
	`acknowledgeid`          bigint unsigned                           NULL,
	PRIMARY KEY (escalationid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `escalations_1` ON `escalations` (`triggerid`,`itemid`,`escalationid`);
CREATE INDEX `escalations_2` ON `escalations` (`eventid`);
CREATE INDEX `escalations_3` ON `escalations` (`nextcheck`);
CREATE TABLE `globalvars` (
	`globalvarid`            bigint unsigned                           NOT NULL,
	`snmp_lastsize`          bigint unsigned DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
) ENGINE=InnoDB;
CREATE TABLE `graph_discovery` (
	`graphid`                bigint unsigned                           NOT NULL,
	`parent_graphid`         bigint unsigned                           NOT NULL,
	PRIMARY KEY (graphid)
) ENGINE=InnoDB;
CREATE INDEX `graph_discovery_1` ON `graph_discovery` (`parent_graphid`);
CREATE TABLE `host_inventory` (
	`hostid`                 bigint unsigned                           NOT NULL,
	`inventory_mode`         integer         DEFAULT '0'               NOT NULL,
	`type`                   varchar(64)     DEFAULT ''                NOT NULL,
	`type_full`              varchar(64)     DEFAULT ''                NOT NULL,
	`name`                   varchar(64)     DEFAULT ''                NOT NULL,
	`alias`                  varchar(64)     DEFAULT ''                NOT NULL,
	`os`                     varchar(64)     DEFAULT ''                NOT NULL,
	`os_full`                varchar(255)    DEFAULT ''                NOT NULL,
	`os_short`               varchar(64)     DEFAULT ''                NOT NULL,
	`serialno_a`             varchar(64)     DEFAULT ''                NOT NULL,
	`serialno_b`             varchar(64)     DEFAULT ''                NOT NULL,
	`tag`                    varchar(64)     DEFAULT ''                NOT NULL,
	`asset_tag`              varchar(64)     DEFAULT ''                NOT NULL,
	`macaddress_a`           varchar(64)     DEFAULT ''                NOT NULL,
	`macaddress_b`           varchar(64)     DEFAULT ''                NOT NULL,
	`hardware`               varchar(255)    DEFAULT ''                NOT NULL,
	`hardware_full`          text                                      NOT NULL,
	`software`               varchar(255)    DEFAULT ''                NOT NULL,
	`software_full`          text                                      NOT NULL,
	`software_app_a`         varchar(64)     DEFAULT ''                NOT NULL,
	`software_app_b`         varchar(64)     DEFAULT ''                NOT NULL,
	`software_app_c`         varchar(64)     DEFAULT ''                NOT NULL,
	`software_app_d`         varchar(64)     DEFAULT ''                NOT NULL,
	`software_app_e`         varchar(64)     DEFAULT ''                NOT NULL,
	`contact`                text                                      NOT NULL,
	`location`               text                                      NOT NULL,
	`location_lat`           varchar(16)     DEFAULT ''                NOT NULL,
	`location_lon`           varchar(16)     DEFAULT ''                NOT NULL,
	`notes`                  text                                      NOT NULL,
	`chassis`                varchar(64)     DEFAULT ''                NOT NULL,
	`model`                  varchar(64)     DEFAULT ''                NOT NULL,
	`hw_arch`                varchar(32)     DEFAULT ''                NOT NULL,
	`vendor`                 varchar(64)     DEFAULT ''                NOT NULL,
	`contract_number`        varchar(64)     DEFAULT ''                NOT NULL,
	`installer_name`         varchar(64)     DEFAULT ''                NOT NULL,
	`deployment_status`      varchar(64)     DEFAULT ''                NOT NULL,
	`url_a`                  varchar(255)    DEFAULT ''                NOT NULL,
	`url_b`                  varchar(255)    DEFAULT ''                NOT NULL,
	`url_c`                  varchar(255)    DEFAULT ''                NOT NULL,
	`host_networks`          text                                      NOT NULL,
	`host_netmask`           varchar(39)     DEFAULT ''                NOT NULL,
	`host_router`            varchar(39)     DEFAULT ''                NOT NULL,
	`oob_ip`                 varchar(39)     DEFAULT ''                NOT NULL,
	`oob_netmask`            varchar(39)     DEFAULT ''                NOT NULL,
	`oob_router`             varchar(39)     DEFAULT ''                NOT NULL,
	`date_hw_purchase`       varchar(64)     DEFAULT ''                NOT NULL,
	`date_hw_install`        varchar(64)     DEFAULT ''                NOT NULL,
	`date_hw_expiry`         varchar(64)     DEFAULT ''                NOT NULL,
	`date_hw_decomm`         varchar(64)     DEFAULT ''                NOT NULL,
	`site_address_a`         varchar(128)    DEFAULT ''                NOT NULL,
	`site_address_b`         varchar(128)    DEFAULT ''                NOT NULL,
	`site_address_c`         varchar(128)    DEFAULT ''                NOT NULL,
	`site_city`              varchar(128)    DEFAULT ''                NOT NULL,
	`site_state`             varchar(64)     DEFAULT ''                NOT NULL,
	`site_country`           varchar(64)     DEFAULT ''                NOT NULL,
	`site_zip`               varchar(64)     DEFAULT ''                NOT NULL,
	`site_rack`              varchar(128)    DEFAULT ''                NOT NULL,
	`site_notes`             text                                      NOT NULL,
	`poc_1_name`             varchar(128)    DEFAULT ''                NOT NULL,
	`poc_1_email`            varchar(128)    DEFAULT ''                NOT NULL,
	`poc_1_phone_a`          varchar(64)     DEFAULT ''                NOT NULL,
	`poc_1_phone_b`          varchar(64)     DEFAULT ''                NOT NULL,
	`poc_1_cell`             varchar(64)     DEFAULT ''                NOT NULL,
	`poc_1_screen`           varchar(64)     DEFAULT ''                NOT NULL,
	`poc_1_notes`            text                                      NOT NULL,
	`poc_2_name`             varchar(128)    DEFAULT ''                NOT NULL,
	`poc_2_email`            varchar(128)    DEFAULT ''                NOT NULL,
	`poc_2_phone_a`          varchar(64)     DEFAULT ''                NOT NULL,
	`poc_2_phone_b`          varchar(64)     DEFAULT ''                NOT NULL,
	`poc_2_cell`             varchar(64)     DEFAULT ''                NOT NULL,
	`poc_2_screen`           varchar(64)     DEFAULT ''                NOT NULL,
	`poc_2_notes`            text                                      NOT NULL,
	PRIMARY KEY (hostid)
) ENGINE=InnoDB;
CREATE TABLE `housekeeper` (
	`housekeeperid`          bigint unsigned                           NOT NULL,
	`tablename`              varchar(64)     DEFAULT ''                NOT NULL,
	`field`                  varchar(64)     DEFAULT ''                NOT NULL,
	`value`                  bigint unsigned                           NOT NULL,
	PRIMARY KEY (housekeeperid)
) ENGINE=InnoDB;
CREATE TABLE `images` (
	`imageid`                bigint unsigned                           NOT NULL,
	`imagetype`              integer         DEFAULT '0'               NOT NULL,
	`name`                   varchar(64)     DEFAULT '0'               NOT NULL,
	`image`                  longblob                                  NOT NULL,
	PRIMARY KEY (imageid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `images_1` ON `images` (`name`);
CREATE TABLE `item_discovery` (
	`itemdiscoveryid`        bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`parent_itemid`          bigint unsigned                           NOT NULL,
	`key_`                   varchar(255)    DEFAULT ''                NOT NULL,
	`lastcheck`              integer         DEFAULT '0'               NOT NULL,
	`ts_delete`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `item_discovery_1` ON `item_discovery` (`itemid`,`parent_itemid`);
CREATE INDEX `item_discovery_2` ON `item_discovery` (`parent_itemid`);
CREATE TABLE `host_discovery` (
	`hostid`                 bigint unsigned                           NOT NULL,
	`parent_hostid`          bigint unsigned                           NULL,
	`parent_itemid`          bigint unsigned                           NULL,
	`host`                   varchar(64)     DEFAULT ''                NOT NULL,
	`lastcheck`              integer         DEFAULT '0'               NOT NULL,
	`ts_delete`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (hostid)
) ENGINE=InnoDB;
CREATE TABLE `interface_discovery` (
	`interfaceid`            bigint unsigned                           NOT NULL,
	`parent_interfaceid`     bigint unsigned                           NOT NULL,
	PRIMARY KEY (interfaceid)
) ENGINE=InnoDB;
CREATE TABLE `profiles` (
	`profileid`              bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`idx`                    varchar(96)     DEFAULT ''                NOT NULL,
	`idx2`                   bigint unsigned DEFAULT '0'               NOT NULL,
	`value_id`               bigint unsigned DEFAULT '0'               NOT NULL,
	`value_int`              integer         DEFAULT '0'               NOT NULL,
	`value_str`              varchar(255)    DEFAULT ''                NOT NULL,
	`source`                 varchar(96)     DEFAULT ''                NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (profileid)
) ENGINE=InnoDB;
CREATE INDEX `profiles_1` ON `profiles` (`userid`,`idx`,`idx2`);
CREATE INDEX `profiles_2` ON `profiles` (`userid`,`profileid`);
CREATE TABLE `sessions` (
	`sessionid`              varchar(32)     DEFAULT ''                NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`lastaccess`             integer         DEFAULT '0'               NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sessionid)
) ENGINE=InnoDB;
CREATE INDEX `sessions_1` ON `sessions` (`userid`,`status`,`lastaccess`);
CREATE TABLE `trigger_discovery` (
	`triggerid`              bigint unsigned                           NOT NULL,
	`parent_triggerid`       bigint unsigned                           NOT NULL,
	PRIMARY KEY (triggerid)
) ENGINE=InnoDB;
CREATE INDEX `trigger_discovery_1` ON `trigger_discovery` (`parent_triggerid`);
CREATE TABLE `application_template` (
	`application_templateid` bigint unsigned                           NOT NULL,
	`applicationid`          bigint unsigned                           NOT NULL,
	`templateid`             bigint unsigned                           NOT NULL,
	PRIMARY KEY (application_templateid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `application_template_1` ON `application_template` (`applicationid`,`templateid`);
CREATE INDEX `application_template_2` ON `application_template` (`templateid`);
CREATE TABLE `item_condition` (
	`item_conditionid`       bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`operator`               integer         DEFAULT '8'               NOT NULL,
	`macro`                  varchar(64)     DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (item_conditionid)
) ENGINE=InnoDB;
CREATE INDEX `item_condition_1` ON `item_condition` (`itemid`);
CREATE TABLE `application_prototype` (
	`application_prototypeid` bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`templateid`             bigint unsigned                           NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (application_prototypeid)
) ENGINE=InnoDB;
CREATE INDEX `application_prototype_1` ON `application_prototype` (`itemid`);
CREATE INDEX `application_prototype_2` ON `application_prototype` (`templateid`);
CREATE TABLE `item_application_prototype` (
	`item_application_prototypeid` bigint unsigned                           NOT NULL,
	`application_prototypeid` bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (item_application_prototypeid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `item_application_prototype_1` ON `item_application_prototype` (`application_prototypeid`,`itemid`);
CREATE INDEX `item_application_prototype_2` ON `item_application_prototype` (`itemid`);
CREATE TABLE `application_discovery` (
	`application_discoveryid` bigint unsigned                           NOT NULL,
	`applicationid`          bigint unsigned                           NOT NULL,
	`application_prototypeid` bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`lastcheck`              integer         DEFAULT '0'               NOT NULL,
	`ts_delete`              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (application_discoveryid)
) ENGINE=InnoDB;
CREATE INDEX `application_discovery_1` ON `application_discovery` (`applicationid`);
CREATE INDEX `application_discovery_2` ON `application_discovery` (`application_prototypeid`);
CREATE TABLE `opinventory` (
	`operationid`            bigint unsigned                           NOT NULL,
	`inventory_mode`         integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
) ENGINE=InnoDB;
CREATE TABLE `trigger_tag` (
	`triggertagid`           bigint unsigned                           NOT NULL,
	`triggerid`              bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (triggertagid)
) ENGINE=InnoDB;
CREATE INDEX `trigger_tag_1` ON `trigger_tag` (`triggerid`);
CREATE TABLE `event_tag` (
	`eventtagid`             bigint unsigned                           NOT NULL,
	`eventid`                bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (eventtagid)
) ENGINE=InnoDB;
CREATE INDEX `event_tag_1` ON `event_tag` (`eventid`);
CREATE TABLE `problem` (
	`eventid`                bigint unsigned                           NOT NULL,
	`source`                 integer         DEFAULT '0'               NOT NULL,
	`object`                 integer         DEFAULT '0'               NOT NULL,
	`objectid`               bigint unsigned DEFAULT '0'               NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`ns`                     integer         DEFAULT '0'               NOT NULL,
	`r_eventid`              bigint unsigned                           NULL,
	`r_clock`                integer         DEFAULT '0'               NOT NULL,
	`r_ns`                   integer         DEFAULT '0'               NOT NULL,
	`correlationid`          bigint unsigned                           NULL,
	`userid`                 bigint unsigned                           NULL,
	`name`                   varchar(2048)   DEFAULT ''                NOT NULL,
	`acknowledged`           integer         DEFAULT '0'               NOT NULL,
	`severity`               integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (eventid)
) ENGINE=InnoDB;
CREATE INDEX `problem_1` ON `problem` (`source`,`object`,`objectid`);
CREATE INDEX `problem_2` ON `problem` (`r_clock`);
CREATE INDEX `problem_3` ON `problem` (`r_eventid`);
CREATE TABLE `problem_tag` (
	`problemtagid`           bigint unsigned                           NOT NULL,
	`eventid`                bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (problemtagid)
) ENGINE=InnoDB;
CREATE INDEX `problem_tag_1` ON `problem_tag` (`eventid`,`tag`,`value`);
CREATE TABLE `tag_filter` (
	`tag_filterid`           bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (tag_filterid)
) ENGINE=InnoDB;
CREATE TABLE `event_recovery` (
	`eventid`                bigint unsigned                           NOT NULL,
	`r_eventid`              bigint unsigned                           NOT NULL,
	`c_eventid`              bigint unsigned                           NULL,
	`correlationid`          bigint unsigned                           NULL,
	`userid`                 bigint unsigned                           NULL,
	PRIMARY KEY (eventid)
) ENGINE=InnoDB;
CREATE INDEX `event_recovery_1` ON `event_recovery` (`r_eventid`);
CREATE INDEX `event_recovery_2` ON `event_recovery` (`c_eventid`);
CREATE TABLE `correlation` (
	`correlationid`          bigint unsigned                           NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`description`            text                                      NOT NULL,
	`evaltype`               integer         DEFAULT '0'               NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`formula`                varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (correlationid)
) ENGINE=InnoDB;
CREATE INDEX `correlation_1` ON `correlation` (`status`);
CREATE UNIQUE INDEX `correlation_2` ON `correlation` (`name`);
CREATE TABLE `corr_condition` (
	`corr_conditionid`       bigint unsigned                           NOT NULL,
	`correlationid`          bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (corr_conditionid)
) ENGINE=InnoDB;
CREATE INDEX `corr_condition_1` ON `corr_condition` (`correlationid`);
CREATE TABLE `corr_condition_tag` (
	`corr_conditionid`       bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (corr_conditionid)
) ENGINE=InnoDB;
CREATE TABLE `corr_condition_group` (
	`corr_conditionid`       bigint unsigned                           NOT NULL,
	`operator`               integer         DEFAULT '0'               NOT NULL,
	`groupid`                bigint unsigned                           NOT NULL,
	PRIMARY KEY (corr_conditionid)
) ENGINE=InnoDB;
CREATE INDEX `corr_condition_group_1` ON `corr_condition_group` (`groupid`);
CREATE TABLE `corr_condition_tagpair` (
	`corr_conditionid`       bigint unsigned                           NOT NULL,
	`oldtag`                 varchar(255)    DEFAULT ''                NOT NULL,
	`newtag`                 varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (corr_conditionid)
) ENGINE=InnoDB;
CREATE TABLE `corr_condition_tagvalue` (
	`corr_conditionid`       bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	`operator`               integer         DEFAULT '0'               NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (corr_conditionid)
) ENGINE=InnoDB;
CREATE TABLE `corr_operation` (
	`corr_operationid`       bigint unsigned                           NOT NULL,
	`correlationid`          bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (corr_operationid)
) ENGINE=InnoDB;
CREATE INDEX `corr_operation_1` ON `corr_operation` (`correlationid`);
CREATE TABLE `task` (
	`taskid`                 bigint unsigned                           NOT NULL,
	`type`                   integer                                   NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`clock`                  integer         DEFAULT '0'               NOT NULL,
	`ttl`                    integer         DEFAULT '0'               NOT NULL,
	`proxy_hostid`           bigint unsigned                           NULL,
	PRIMARY KEY (taskid)
) ENGINE=InnoDB;
CREATE INDEX `task_1` ON `task` (`status`,`proxy_hostid`);
CREATE TABLE `task_close_problem` (
	`taskid`                 bigint unsigned                           NOT NULL,
	`acknowledgeid`          bigint unsigned                           NOT NULL,
	PRIMARY KEY (taskid)
) ENGINE=InnoDB;
CREATE TABLE `item_preproc` (
	`item_preprocid`         bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	`step`                   integer         DEFAULT '0'               NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`params`                 varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (item_preprocid)
) ENGINE=InnoDB;
CREATE INDEX `item_preproc_1` ON `item_preproc` (`itemid`,`step`);
CREATE TABLE `task_remote_command` (
	`taskid`                 bigint unsigned                           NOT NULL,
	`command_type`           integer         DEFAULT '0'               NOT NULL,
	`execute_on`             integer         DEFAULT '0'               NOT NULL,
	`port`                   integer         DEFAULT '0'               NOT NULL,
	`authtype`               integer         DEFAULT '0'               NOT NULL,
	`username`               varchar(64)     DEFAULT ''                NOT NULL,
	`password`               varchar(64)     DEFAULT ''                NOT NULL,
	`publickey`              varchar(64)     DEFAULT ''                NOT NULL,
	`privatekey`             varchar(64)     DEFAULT ''                NOT NULL,
	`command`                text                                      NOT NULL,
	`alertid`                bigint unsigned                           NULL,
	`parent_taskid`          bigint unsigned                           NOT NULL,
	`hostid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (taskid)
) ENGINE=InnoDB;
CREATE TABLE `task_remote_command_result` (
	`taskid`                 bigint unsigned                           NOT NULL,
	`status`                 integer         DEFAULT '0'               NOT NULL,
	`parent_taskid`          bigint unsigned                           NOT NULL,
	`info`                   text                                      NOT NULL,
	PRIMARY KEY (taskid)
) ENGINE=InnoDB;
CREATE TABLE `task_acknowledge` (
	`taskid`                 bigint unsigned                           NOT NULL,
	`acknowledgeid`          bigint unsigned                           NOT NULL,
	PRIMARY KEY (taskid)
) ENGINE=InnoDB;
CREATE TABLE `sysmap_shape` (
	`sysmap_shapeid`         bigint unsigned                           NOT NULL,
	`sysmapid`               bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`x`                      integer         DEFAULT '0'               NOT NULL,
	`y`                      integer         DEFAULT '0'               NOT NULL,
	`width`                  integer         DEFAULT '200'             NOT NULL,
	`height`                 integer         DEFAULT '200'             NOT NULL,
	`text`                   text                                      NOT NULL,
	`font`                   integer         DEFAULT '9'               NOT NULL,
	`font_size`              integer         DEFAULT '11'              NOT NULL,
	`font_color`             varchar(6)      DEFAULT '000000'          NOT NULL,
	`text_halign`            integer         DEFAULT '0'               NOT NULL,
	`text_valign`            integer         DEFAULT '0'               NOT NULL,
	`border_type`            integer         DEFAULT '0'               NOT NULL,
	`border_width`           integer         DEFAULT '1'               NOT NULL,
	`border_color`           varchar(6)      DEFAULT '000000'          NOT NULL,
	`background_color`       varchar(6)      DEFAULT ''                NOT NULL,
	`zindex`                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmap_shapeid)
) ENGINE=InnoDB;
CREATE INDEX `sysmap_shape_1` ON `sysmap_shape` (`sysmapid`);
CREATE TABLE `sysmap_element_trigger` (
	`selement_triggerid`     bigint unsigned                           NOT NULL,
	`selementid`             bigint unsigned                           NOT NULL,
	`triggerid`              bigint unsigned                           NOT NULL,
	PRIMARY KEY (selement_triggerid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `sysmap_element_trigger_1` ON `sysmap_element_trigger` (`selementid`,`triggerid`);
CREATE TABLE `httptest_field` (
	`httptest_fieldid`       bigint unsigned                           NOT NULL,
	`httptestid`             bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  text                                      NOT NULL,
	PRIMARY KEY (httptest_fieldid)
) ENGINE=InnoDB;
CREATE INDEX `httptest_field_1` ON `httptest_field` (`httptestid`);
CREATE TABLE `httpstep_field` (
	`httpstep_fieldid`       bigint unsigned                           NOT NULL,
	`httpstepid`             bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`value`                  text                                      NOT NULL,
	PRIMARY KEY (httpstep_fieldid)
) ENGINE=InnoDB;
CREATE INDEX `httpstep_field_1` ON `httpstep_field` (`httpstepid`);
CREATE TABLE `dashboard` (
	`dashboardid`            bigint unsigned                           NOT NULL,
	`name`                   varchar(255)                              NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`private`                integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (dashboardid)
) ENGINE=InnoDB;
CREATE TABLE `dashboard_user` (
	`dashboard_userid`       bigint unsigned                           NOT NULL,
	`dashboardid`            bigint unsigned                           NOT NULL,
	`userid`                 bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (dashboard_userid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `dashboard_user_1` ON `dashboard_user` (`dashboardid`,`userid`);
CREATE TABLE `dashboard_usrgrp` (
	`dashboard_usrgrpid`     bigint unsigned                           NOT NULL,
	`dashboardid`            bigint unsigned                           NOT NULL,
	`usrgrpid`               bigint unsigned                           NOT NULL,
	`permission`             integer         DEFAULT '2'               NOT NULL,
	PRIMARY KEY (dashboard_usrgrpid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `dashboard_usrgrp_1` ON `dashboard_usrgrp` (`dashboardid`,`usrgrpid`);
CREATE TABLE `widget` (
	`widgetid`               bigint unsigned                           NOT NULL,
	`dashboardid`            bigint unsigned                           NOT NULL,
	`type`                   varchar(255)    DEFAULT ''                NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`x`                      integer         DEFAULT '0'               NOT NULL,
	`y`                      integer         DEFAULT '0'               NOT NULL,
	`width`                  integer         DEFAULT '1'               NOT NULL,
	`height`                 integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (widgetid)
) ENGINE=InnoDB;
CREATE INDEX `widget_1` ON `widget` (`dashboardid`);
CREATE TABLE `widget_field` (
	`widget_fieldid`         bigint unsigned                           NOT NULL,
	`widgetid`               bigint unsigned                           NOT NULL,
	`type`                   integer         DEFAULT '0'               NOT NULL,
	`name`                   varchar(255)    DEFAULT ''                NOT NULL,
	`value_int`              integer         DEFAULT '0'               NOT NULL,
	`value_str`              varchar(255)    DEFAULT ''                NOT NULL,
	`value_groupid`          bigint unsigned                           NULL,
	`value_hostid`           bigint unsigned                           NULL,
	`value_itemid`           bigint unsigned                           NULL,
	`value_graphid`          bigint unsigned                           NULL,
	`value_sysmapid`         bigint unsigned                           NULL,
	PRIMARY KEY (widget_fieldid)
) ENGINE=InnoDB;
CREATE INDEX `widget_field_1` ON `widget_field` (`widgetid`);
CREATE INDEX `widget_field_2` ON `widget_field` (`value_groupid`);
CREATE INDEX `widget_field_3` ON `widget_field` (`value_hostid`);
CREATE INDEX `widget_field_4` ON `widget_field` (`value_itemid`);
CREATE INDEX `widget_field_5` ON `widget_field` (`value_graphid`);
CREATE INDEX `widget_field_6` ON `widget_field` (`value_sysmapid`);
CREATE TABLE `task_check_now` (
	`taskid`                 bigint unsigned                           NOT NULL,
	`itemid`                 bigint unsigned                           NOT NULL,
	PRIMARY KEY (taskid)
) ENGINE=InnoDB;
CREATE TABLE `event_suppress` (
	`event_suppressid`       bigint unsigned                           NOT NULL,
	`eventid`                bigint unsigned                           NOT NULL,
	`maintenanceid`          bigint unsigned                           NULL,
	`suppress_until`         integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (event_suppressid)
) ENGINE=InnoDB;
CREATE UNIQUE INDEX `event_suppress_1` ON `event_suppress` (`eventid`,`maintenanceid`);
CREATE INDEX `event_suppress_2` ON `event_suppress` (`suppress_until`);
CREATE INDEX `event_suppress_3` ON `event_suppress` (`maintenanceid`);
CREATE TABLE `maintenance_tag` (
	`maintenancetagid`       bigint unsigned                           NOT NULL,
	`maintenanceid`          bigint unsigned                           NOT NULL,
	`tag`                    varchar(255)    DEFAULT ''                NOT NULL,
	`operator`               integer         DEFAULT '2'               NOT NULL,
	`value`                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (maintenancetagid)
) ENGINE=InnoDB;
CREATE INDEX `maintenance_tag_1` ON `maintenance_tag` (`maintenanceid`);
CREATE TABLE `dbversion` (
	`mandatory`              integer         DEFAULT '0'               NOT NULL,
	`optional`               integer         DEFAULT '0'               NOT NULL
) ENGINE=InnoDB;
INSERT INTO dbversion VALUES ('4000000','4000000');
ALTER TABLE `hosts` ADD CONSTRAINT `c_hosts_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`);
ALTER TABLE `hosts` ADD CONSTRAINT `c_hosts_2` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`);
ALTER TABLE `hosts` ADD CONSTRAINT `c_hosts_3` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `group_prototype` ADD CONSTRAINT `c_group_prototype_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `group_prototype` ADD CONSTRAINT `c_group_prototype_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`);
ALTER TABLE `group_prototype` ADD CONSTRAINT `c_group_prototype_3` FOREIGN KEY (`templateid`) REFERENCES `group_prototype` (`group_prototypeid`) ON DELETE CASCADE;
ALTER TABLE `group_discovery` ADD CONSTRAINT `c_group_discovery_1` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`) ON DELETE CASCADE;
ALTER TABLE `group_discovery` ADD CONSTRAINT `c_group_discovery_2` FOREIGN KEY (`parent_group_prototypeid`) REFERENCES `group_prototype` (`group_prototypeid`);
ALTER TABLE `screens` ADD CONSTRAINT `c_screens_1` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `screens` ADD CONSTRAINT `c_screens_3` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`);
ALTER TABLE `screens_items` ADD CONSTRAINT `c_screens_items_1` FOREIGN KEY (`screenid`) REFERENCES `screens` (`screenid`) ON DELETE CASCADE;
ALTER TABLE `screen_user` ADD CONSTRAINT `c_screen_user_1` FOREIGN KEY (`screenid`) REFERENCES `screens` (`screenid`) ON DELETE CASCADE;
ALTER TABLE `screen_user` ADD CONSTRAINT `c_screen_user_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `screen_usrgrp` ADD CONSTRAINT `c_screen_usrgrp_1` FOREIGN KEY (`screenid`) REFERENCES `screens` (`screenid`) ON DELETE CASCADE;
ALTER TABLE `screen_usrgrp` ADD CONSTRAINT `c_screen_usrgrp_2` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `slideshows` ADD CONSTRAINT `c_slideshows_3` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`);
ALTER TABLE `slideshow_user` ADD CONSTRAINT `c_slideshow_user_1` FOREIGN KEY (`slideshowid`) REFERENCES `slideshows` (`slideshowid`) ON DELETE CASCADE;
ALTER TABLE `slideshow_user` ADD CONSTRAINT `c_slideshow_user_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `slideshow_usrgrp` ADD CONSTRAINT `c_slideshow_usrgrp_1` FOREIGN KEY (`slideshowid`) REFERENCES `slideshows` (`slideshowid`) ON DELETE CASCADE;
ALTER TABLE `slideshow_usrgrp` ADD CONSTRAINT `c_slideshow_usrgrp_2` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `slides` ADD CONSTRAINT `c_slides_1` FOREIGN KEY (`slideshowid`) REFERENCES `slideshows` (`slideshowid`) ON DELETE CASCADE;
ALTER TABLE `slides` ADD CONSTRAINT `c_slides_2` FOREIGN KEY (`screenid`) REFERENCES `screens` (`screenid`) ON DELETE CASCADE;
ALTER TABLE `drules` ADD CONSTRAINT `c_drules_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`);
ALTER TABLE `dchecks` ADD CONSTRAINT `c_dchecks_1` FOREIGN KEY (`druleid`) REFERENCES `drules` (`druleid`) ON DELETE CASCADE;
ALTER TABLE `applications` ADD CONSTRAINT `c_applications_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `httptest` ADD CONSTRAINT `c_httptest_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`);
ALTER TABLE `httptest` ADD CONSTRAINT `c_httptest_2` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `httptest` ADD CONSTRAINT `c_httptest_3` FOREIGN KEY (`templateid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE;
ALTER TABLE `httpstep` ADD CONSTRAINT `c_httpstep_1` FOREIGN KEY (`httptestid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE;
ALTER TABLE `interface` ADD CONSTRAINT `c_interface_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `items` ADD CONSTRAINT `c_items_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `items` ADD CONSTRAINT `c_items_2` FOREIGN KEY (`templateid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `items` ADD CONSTRAINT `c_items_3` FOREIGN KEY (`valuemapid`) REFERENCES `valuemaps` (`valuemapid`);
ALTER TABLE `items` ADD CONSTRAINT `c_items_4` FOREIGN KEY (`interfaceid`) REFERENCES `interface` (`interfaceid`);
ALTER TABLE `items` ADD CONSTRAINT `c_items_5` FOREIGN KEY (`master_itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `httpstepitem` ADD CONSTRAINT `c_httpstepitem_1` FOREIGN KEY (`httpstepid`) REFERENCES `httpstep` (`httpstepid`) ON DELETE CASCADE;
ALTER TABLE `httpstepitem` ADD CONSTRAINT `c_httpstepitem_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `httptestitem` ADD CONSTRAINT `c_httptestitem_1` FOREIGN KEY (`httptestid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE;
ALTER TABLE `httptestitem` ADD CONSTRAINT `c_httptestitem_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `users_groups` ADD CONSTRAINT `c_users_groups_1` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `users_groups` ADD CONSTRAINT `c_users_groups_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `scripts` ADD CONSTRAINT `c_scripts_1` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`);
ALTER TABLE `scripts` ADD CONSTRAINT `c_scripts_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`);
ALTER TABLE `operations` ADD CONSTRAINT `c_operations_1` FOREIGN KEY (`actionid`) REFERENCES `actions` (`actionid`) ON DELETE CASCADE;
ALTER TABLE `opmessage` ADD CONSTRAINT `c_opmessage_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opmessage` ADD CONSTRAINT `c_opmessage_2` FOREIGN KEY (`mediatypeid`) REFERENCES `media_type` (`mediatypeid`);
ALTER TABLE `opmessage_grp` ADD CONSTRAINT `c_opmessage_grp_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opmessage_grp` ADD CONSTRAINT `c_opmessage_grp_2` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`);
ALTER TABLE `opmessage_usr` ADD CONSTRAINT `c_opmessage_usr_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opmessage_usr` ADD CONSTRAINT `c_opmessage_usr_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`);
ALTER TABLE `opcommand` ADD CONSTRAINT `c_opcommand_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opcommand` ADD CONSTRAINT `c_opcommand_2` FOREIGN KEY (`scriptid`) REFERENCES `scripts` (`scriptid`);
ALTER TABLE `opcommand_hst` ADD CONSTRAINT `c_opcommand_hst_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opcommand_hst` ADD CONSTRAINT `c_opcommand_hst_2` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`);
ALTER TABLE `opcommand_grp` ADD CONSTRAINT `c_opcommand_grp_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opcommand_grp` ADD CONSTRAINT `c_opcommand_grp_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`);
ALTER TABLE `opgroup` ADD CONSTRAINT `c_opgroup_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `opgroup` ADD CONSTRAINT `c_opgroup_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`);
ALTER TABLE `optemplate` ADD CONSTRAINT `c_optemplate_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `optemplate` ADD CONSTRAINT `c_optemplate_2` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`);
ALTER TABLE `opconditions` ADD CONSTRAINT `c_opconditions_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `conditions` ADD CONSTRAINT `c_conditions_1` FOREIGN KEY (`actionid`) REFERENCES `actions` (`actionid`) ON DELETE CASCADE;
ALTER TABLE `config` ADD CONSTRAINT `c_config_1` FOREIGN KEY (`alert_usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`);
ALTER TABLE `config` ADD CONSTRAINT `c_config_2` FOREIGN KEY (`discovery_groupid`) REFERENCES `hstgrp` (`groupid`);
ALTER TABLE `triggers` ADD CONSTRAINT `c_triggers_1` FOREIGN KEY (`templateid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `trigger_depends` ADD CONSTRAINT `c_trigger_depends_1` FOREIGN KEY (`triggerid_down`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `trigger_depends` ADD CONSTRAINT `c_trigger_depends_2` FOREIGN KEY (`triggerid_up`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `functions` ADD CONSTRAINT `c_functions_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `functions` ADD CONSTRAINT `c_functions_2` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `graphs` ADD CONSTRAINT `c_graphs_1` FOREIGN KEY (`templateid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE;
ALTER TABLE `graphs` ADD CONSTRAINT `c_graphs_2` FOREIGN KEY (`ymin_itemid`) REFERENCES `items` (`itemid`);
ALTER TABLE `graphs` ADD CONSTRAINT `c_graphs_3` FOREIGN KEY (`ymax_itemid`) REFERENCES `items` (`itemid`);
ALTER TABLE `graphs_items` ADD CONSTRAINT `c_graphs_items_1` FOREIGN KEY (`graphid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE;
ALTER TABLE `graphs_items` ADD CONSTRAINT `c_graphs_items_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `hostmacro` ADD CONSTRAINT `c_hostmacro_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `hosts_groups` ADD CONSTRAINT `c_hosts_groups_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `hosts_groups` ADD CONSTRAINT `c_hosts_groups_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`) ON DELETE CASCADE;
ALTER TABLE `hosts_templates` ADD CONSTRAINT `c_hosts_templates_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `hosts_templates` ADD CONSTRAINT `c_hosts_templates_2` FOREIGN KEY (`templateid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `items_applications` ADD CONSTRAINT `c_items_applications_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE;
ALTER TABLE `items_applications` ADD CONSTRAINT `c_items_applications_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `mappings` ADD CONSTRAINT `c_mappings_1` FOREIGN KEY (`valuemapid`) REFERENCES `valuemaps` (`valuemapid`) ON DELETE CASCADE;
ALTER TABLE `media` ADD CONSTRAINT `c_media_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `media` ADD CONSTRAINT `c_media_2` FOREIGN KEY (`mediatypeid`) REFERENCES `media_type` (`mediatypeid`) ON DELETE CASCADE;
ALTER TABLE `rights` ADD CONSTRAINT `c_rights_1` FOREIGN KEY (`groupid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `rights` ADD CONSTRAINT `c_rights_2` FOREIGN KEY (`id`) REFERENCES `hstgrp` (`groupid`) ON DELETE CASCADE;
ALTER TABLE `services` ADD CONSTRAINT `c_services_1` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `services_links` ADD CONSTRAINT `c_services_links_1` FOREIGN KEY (`serviceupid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE;
ALTER TABLE `services_links` ADD CONSTRAINT `c_services_links_2` FOREIGN KEY (`servicedownid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE;
ALTER TABLE `services_times` ADD CONSTRAINT `c_services_times_1` FOREIGN KEY (`serviceid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE;
ALTER TABLE `icon_map` ADD CONSTRAINT `c_icon_map_1` FOREIGN KEY (`default_iconid`) REFERENCES `images` (`imageid`);
ALTER TABLE `icon_mapping` ADD CONSTRAINT `c_icon_mapping_1` FOREIGN KEY (`iconmapid`) REFERENCES `icon_map` (`iconmapid`) ON DELETE CASCADE;
ALTER TABLE `icon_mapping` ADD CONSTRAINT `c_icon_mapping_2` FOREIGN KEY (`iconid`) REFERENCES `images` (`imageid`);
ALTER TABLE `sysmaps` ADD CONSTRAINT `c_sysmaps_1` FOREIGN KEY (`backgroundid`) REFERENCES `images` (`imageid`);
ALTER TABLE `sysmaps` ADD CONSTRAINT `c_sysmaps_2` FOREIGN KEY (`iconmapid`) REFERENCES `icon_map` (`iconmapid`);
ALTER TABLE `sysmaps` ADD CONSTRAINT `c_sysmaps_3` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`);
ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_2` FOREIGN KEY (`iconid_off`) REFERENCES `images` (`imageid`);
ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_3` FOREIGN KEY (`iconid_on`) REFERENCES `images` (`imageid`);
ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_4` FOREIGN KEY (`iconid_disabled`) REFERENCES `images` (`imageid`);
ALTER TABLE `sysmaps_elements` ADD CONSTRAINT `c_sysmaps_elements_5` FOREIGN KEY (`iconid_maintenance`) REFERENCES `images` (`imageid`);
ALTER TABLE `sysmaps_links` ADD CONSTRAINT `c_sysmaps_links_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `sysmaps_links` ADD CONSTRAINT `c_sysmaps_links_2` FOREIGN KEY (`selementid1`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE;
ALTER TABLE `sysmaps_links` ADD CONSTRAINT `c_sysmaps_links_3` FOREIGN KEY (`selementid2`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE;
ALTER TABLE `sysmaps_link_triggers` ADD CONSTRAINT `c_sysmaps_link_triggers_1` FOREIGN KEY (`linkid`) REFERENCES `sysmaps_links` (`linkid`) ON DELETE CASCADE;
ALTER TABLE `sysmaps_link_triggers` ADD CONSTRAINT `c_sysmaps_link_triggers_2` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_element_url` ADD CONSTRAINT `c_sysmap_element_url_1` FOREIGN KEY (`selementid`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_url` ADD CONSTRAINT `c_sysmap_url_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_user` ADD CONSTRAINT `c_sysmap_user_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_user` ADD CONSTRAINT `c_sysmap_user_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_usrgrp` ADD CONSTRAINT `c_sysmap_usrgrp_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_usrgrp` ADD CONSTRAINT `c_sysmap_usrgrp_2` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `maintenances_hosts` ADD CONSTRAINT `c_maintenances_hosts_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE;
ALTER TABLE `maintenances_hosts` ADD CONSTRAINT `c_maintenances_hosts_2` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `maintenances_groups` ADD CONSTRAINT `c_maintenances_groups_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE;
ALTER TABLE `maintenances_groups` ADD CONSTRAINT `c_maintenances_groups_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`) ON DELETE CASCADE;
ALTER TABLE `maintenances_windows` ADD CONSTRAINT `c_maintenances_windows_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE;
ALTER TABLE `maintenances_windows` ADD CONSTRAINT `c_maintenances_windows_2` FOREIGN KEY (`timeperiodid`) REFERENCES `timeperiods` (`timeperiodid`) ON DELETE CASCADE;
ALTER TABLE `expressions` ADD CONSTRAINT `c_expressions_1` FOREIGN KEY (`regexpid`) REFERENCES `regexps` (`regexpid`) ON DELETE CASCADE;
ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_1` FOREIGN KEY (`actionid`) REFERENCES `actions` (`actionid`) ON DELETE CASCADE;
ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_2` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_3` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_4` FOREIGN KEY (`mediatypeid`) REFERENCES `media_type` (`mediatypeid`) ON DELETE CASCADE;
ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_5` FOREIGN KEY (`p_eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `alerts` ADD CONSTRAINT `c_alerts_6` FOREIGN KEY (`acknowledgeid`) REFERENCES `acknowledges` (`acknowledgeid`) ON DELETE CASCADE;
ALTER TABLE `acknowledges` ADD CONSTRAINT `c_acknowledges_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `acknowledges` ADD CONSTRAINT `c_acknowledges_2` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `auditlog` ADD CONSTRAINT `c_auditlog_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `auditlog_details` ADD CONSTRAINT `c_auditlog_details_1` FOREIGN KEY (`auditid`) REFERENCES `auditlog` (`auditid`) ON DELETE CASCADE;
ALTER TABLE `service_alarms` ADD CONSTRAINT `c_service_alarms_1` FOREIGN KEY (`serviceid`) REFERENCES `services` (`serviceid`) ON DELETE CASCADE;
ALTER TABLE `autoreg_host` ADD CONSTRAINT `c_autoreg_host_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `dhosts` ADD CONSTRAINT `c_dhosts_1` FOREIGN KEY (`druleid`) REFERENCES `drules` (`druleid`) ON DELETE CASCADE;
ALTER TABLE `dservices` ADD CONSTRAINT `c_dservices_1` FOREIGN KEY (`dhostid`) REFERENCES `dhosts` (`dhostid`) ON DELETE CASCADE;
ALTER TABLE `dservices` ADD CONSTRAINT `c_dservices_2` FOREIGN KEY (`dcheckid`) REFERENCES `dchecks` (`dcheckid`) ON DELETE CASCADE;
ALTER TABLE `graph_discovery` ADD CONSTRAINT `c_graph_discovery_1` FOREIGN KEY (`graphid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE;
ALTER TABLE `graph_discovery` ADD CONSTRAINT `c_graph_discovery_2` FOREIGN KEY (`parent_graphid`) REFERENCES `graphs` (`graphid`);
ALTER TABLE `host_inventory` ADD CONSTRAINT `c_host_inventory_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `item_discovery` ADD CONSTRAINT `c_item_discovery_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `item_discovery` ADD CONSTRAINT `c_item_discovery_2` FOREIGN KEY (`parent_itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `host_discovery` ADD CONSTRAINT `c_host_discovery_1` FOREIGN KEY (`hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `host_discovery` ADD CONSTRAINT `c_host_discovery_2` FOREIGN KEY (`parent_hostid`) REFERENCES `hosts` (`hostid`);
ALTER TABLE `host_discovery` ADD CONSTRAINT `c_host_discovery_3` FOREIGN KEY (`parent_itemid`) REFERENCES `items` (`itemid`);
ALTER TABLE `interface_discovery` ADD CONSTRAINT `c_interface_discovery_1` FOREIGN KEY (`interfaceid`) REFERENCES `interface` (`interfaceid`) ON DELETE CASCADE;
ALTER TABLE `interface_discovery` ADD CONSTRAINT `c_interface_discovery_2` FOREIGN KEY (`parent_interfaceid`) REFERENCES `interface` (`interfaceid`) ON DELETE CASCADE;
ALTER TABLE `profiles` ADD CONSTRAINT `c_profiles_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `sessions` ADD CONSTRAINT `c_sessions_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `trigger_discovery` ADD CONSTRAINT `c_trigger_discovery_1` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `trigger_discovery` ADD CONSTRAINT `c_trigger_discovery_2` FOREIGN KEY (`parent_triggerid`) REFERENCES `triggers` (`triggerid`);
ALTER TABLE `application_template` ADD CONSTRAINT `c_application_template_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE;
ALTER TABLE `application_template` ADD CONSTRAINT `c_application_template_2` FOREIGN KEY (`templateid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE;
ALTER TABLE `item_condition` ADD CONSTRAINT `c_item_condition_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `application_prototype` ADD CONSTRAINT `c_application_prototype_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `application_prototype` ADD CONSTRAINT `c_application_prototype_2` FOREIGN KEY (`templateid`) REFERENCES `application_prototype` (`application_prototypeid`) ON DELETE CASCADE;
ALTER TABLE `item_application_prototype` ADD CONSTRAINT `c_item_application_prototype_1` FOREIGN KEY (`application_prototypeid`) REFERENCES `application_prototype` (`application_prototypeid`) ON DELETE CASCADE;
ALTER TABLE `item_application_prototype` ADD CONSTRAINT `c_item_application_prototype_2` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `application_discovery` ADD CONSTRAINT `c_application_discovery_1` FOREIGN KEY (`applicationid`) REFERENCES `applications` (`applicationid`) ON DELETE CASCADE;
ALTER TABLE `application_discovery` ADD CONSTRAINT `c_application_discovery_2` FOREIGN KEY (`application_prototypeid`) REFERENCES `application_prototype` (`application_prototypeid`) ON DELETE CASCADE;
ALTER TABLE `opinventory` ADD CONSTRAINT `c_opinventory_1` FOREIGN KEY (`operationid`) REFERENCES `operations` (`operationid`) ON DELETE CASCADE;
ALTER TABLE `trigger_tag` ADD CONSTRAINT `c_trigger_tag_1` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `event_tag` ADD CONSTRAINT `c_event_tag_1` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `problem` ADD CONSTRAINT `c_problem_1` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `problem` ADD CONSTRAINT `c_problem_2` FOREIGN KEY (`r_eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `problem_tag` ADD CONSTRAINT `c_problem_tag_1` FOREIGN KEY (`eventid`) REFERENCES `problem` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `tag_filter` ADD CONSTRAINT `c_tag_filter_1` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `tag_filter` ADD CONSTRAINT `c_tag_filter_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`) ON DELETE CASCADE;
ALTER TABLE `event_recovery` ADD CONSTRAINT `c_event_recovery_1` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `event_recovery` ADD CONSTRAINT `c_event_recovery_2` FOREIGN KEY (`r_eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `event_recovery` ADD CONSTRAINT `c_event_recovery_3` FOREIGN KEY (`c_eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `corr_condition` ADD CONSTRAINT `c_corr_condition_1` FOREIGN KEY (`correlationid`) REFERENCES `correlation` (`correlationid`) ON DELETE CASCADE;
ALTER TABLE `corr_condition_tag` ADD CONSTRAINT `c_corr_condition_tag_1` FOREIGN KEY (`corr_conditionid`) REFERENCES `corr_condition` (`corr_conditionid`) ON DELETE CASCADE;
ALTER TABLE `corr_condition_group` ADD CONSTRAINT `c_corr_condition_group_1` FOREIGN KEY (`corr_conditionid`) REFERENCES `corr_condition` (`corr_conditionid`) ON DELETE CASCADE;
ALTER TABLE `corr_condition_group` ADD CONSTRAINT `c_corr_condition_group_2` FOREIGN KEY (`groupid`) REFERENCES `hstgrp` (`groupid`);
ALTER TABLE `corr_condition_tagpair` ADD CONSTRAINT `c_corr_condition_tagpair_1` FOREIGN KEY (`corr_conditionid`) REFERENCES `corr_condition` (`corr_conditionid`) ON DELETE CASCADE;
ALTER TABLE `corr_condition_tagvalue` ADD CONSTRAINT `c_corr_condition_tagvalue_1` FOREIGN KEY (`corr_conditionid`) REFERENCES `corr_condition` (`corr_conditionid`) ON DELETE CASCADE;
ALTER TABLE `corr_operation` ADD CONSTRAINT `c_corr_operation_1` FOREIGN KEY (`correlationid`) REFERENCES `correlation` (`correlationid`) ON DELETE CASCADE;
ALTER TABLE `task` ADD CONSTRAINT `c_task_1` FOREIGN KEY (`proxy_hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `task_close_problem` ADD CONSTRAINT `c_task_close_problem_1` FOREIGN KEY (`taskid`) REFERENCES `task` (`taskid`) ON DELETE CASCADE;
ALTER TABLE `item_preproc` ADD CONSTRAINT `c_item_preproc_1` FOREIGN KEY (`itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `task_remote_command` ADD CONSTRAINT `c_task_remote_command_1` FOREIGN KEY (`taskid`) REFERENCES `task` (`taskid`) ON DELETE CASCADE;
ALTER TABLE `task_remote_command_result` ADD CONSTRAINT `c_task_remote_command_result_1` FOREIGN KEY (`taskid`) REFERENCES `task` (`taskid`) ON DELETE CASCADE;
ALTER TABLE `task_acknowledge` ADD CONSTRAINT `c_task_acknowledge_1` FOREIGN KEY (`taskid`) REFERENCES `task` (`taskid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_shape` ADD CONSTRAINT `c_sysmap_shape_1` FOREIGN KEY (`sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_element_trigger` ADD CONSTRAINT `c_sysmap_element_trigger_1` FOREIGN KEY (`selementid`) REFERENCES `sysmaps_elements` (`selementid`) ON DELETE CASCADE;
ALTER TABLE `sysmap_element_trigger` ADD CONSTRAINT `c_sysmap_element_trigger_2` FOREIGN KEY (`triggerid`) REFERENCES `triggers` (`triggerid`) ON DELETE CASCADE;
ALTER TABLE `httptest_field` ADD CONSTRAINT `c_httptest_field_1` FOREIGN KEY (`httptestid`) REFERENCES `httptest` (`httptestid`) ON DELETE CASCADE;
ALTER TABLE `httpstep_field` ADD CONSTRAINT `c_httpstep_field_1` FOREIGN KEY (`httpstepid`) REFERENCES `httpstep` (`httpstepid`) ON DELETE CASCADE;
ALTER TABLE `dashboard` ADD CONSTRAINT `c_dashboard_1` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`);
ALTER TABLE `dashboard_user` ADD CONSTRAINT `c_dashboard_user_1` FOREIGN KEY (`dashboardid`) REFERENCES `dashboard` (`dashboardid`) ON DELETE CASCADE;
ALTER TABLE `dashboard_user` ADD CONSTRAINT `c_dashboard_user_2` FOREIGN KEY (`userid`) REFERENCES `users` (`userid`) ON DELETE CASCADE;
ALTER TABLE `dashboard_usrgrp` ADD CONSTRAINT `c_dashboard_usrgrp_1` FOREIGN KEY (`dashboardid`) REFERENCES `dashboard` (`dashboardid`) ON DELETE CASCADE;
ALTER TABLE `dashboard_usrgrp` ADD CONSTRAINT `c_dashboard_usrgrp_2` FOREIGN KEY (`usrgrpid`) REFERENCES `usrgrp` (`usrgrpid`) ON DELETE CASCADE;
ALTER TABLE `widget` ADD CONSTRAINT `c_widget_1` FOREIGN KEY (`dashboardid`) REFERENCES `dashboard` (`dashboardid`) ON DELETE CASCADE;
ALTER TABLE `widget_field` ADD CONSTRAINT `c_widget_field_1` FOREIGN KEY (`widgetid`) REFERENCES `widget` (`widgetid`) ON DELETE CASCADE;
ALTER TABLE `widget_field` ADD CONSTRAINT `c_widget_field_2` FOREIGN KEY (`value_groupid`) REFERENCES `hstgrp` (`groupid`) ON DELETE CASCADE;
ALTER TABLE `widget_field` ADD CONSTRAINT `c_widget_field_3` FOREIGN KEY (`value_hostid`) REFERENCES `hosts` (`hostid`) ON DELETE CASCADE;
ALTER TABLE `widget_field` ADD CONSTRAINT `c_widget_field_4` FOREIGN KEY (`value_itemid`) REFERENCES `items` (`itemid`) ON DELETE CASCADE;
ALTER TABLE `widget_field` ADD CONSTRAINT `c_widget_field_5` FOREIGN KEY (`value_graphid`) REFERENCES `graphs` (`graphid`) ON DELETE CASCADE;
ALTER TABLE `widget_field` ADD CONSTRAINT `c_widget_field_6` FOREIGN KEY (`value_sysmapid`) REFERENCES `sysmaps` (`sysmapid`) ON DELETE CASCADE;
ALTER TABLE `task_check_now` ADD CONSTRAINT `c_task_check_now_1` FOREIGN KEY (`taskid`) REFERENCES `task` (`taskid`) ON DELETE CASCADE;
ALTER TABLE `event_suppress` ADD CONSTRAINT `c_event_suppress_1` FOREIGN KEY (`eventid`) REFERENCES `events` (`eventid`) ON DELETE CASCADE;
ALTER TABLE `event_suppress` ADD CONSTRAINT `c_event_suppress_2` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE;
ALTER TABLE `maintenance_tag` ADD CONSTRAINT `c_maintenance_tag_1` FOREIGN KEY (`maintenanceid`) REFERENCES `maintenances` (`maintenanceid`) ON DELETE CASCADE;
