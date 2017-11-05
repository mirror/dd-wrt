CREATE TABLE users (
	userid                   bigint                                    NOT NULL,
	alias                    varchar(100)    WITH DEFAULT ''           NOT NULL,
	name                     varchar(100)    WITH DEFAULT ''           NOT NULL,
	surname                  varchar(100)    WITH DEFAULT ''           NOT NULL,
	passwd                   varchar(32)     WITH DEFAULT ''           NOT NULL,
	url                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	autologin                integer         WITH DEFAULT '0'          NOT NULL,
	autologout               varchar(32)     WITH DEFAULT '15m'        NOT NULL,
	lang                     varchar(5)      WITH DEFAULT 'en_GB'      NOT NULL,
	refresh                  varchar(32)     WITH DEFAULT '30s'        NOT NULL,
	type                     integer         WITH DEFAULT '1'          NOT NULL,
	theme                    varchar(128)    WITH DEFAULT 'default'    NOT NULL,
	attempt_failed           integer         WITH DEFAULT 0            NOT NULL,
	attempt_ip               varchar(39)     WITH DEFAULT ''           NOT NULL,
	attempt_clock            integer         WITH DEFAULT 0            NOT NULL,
	rows_per_page            integer         WITH DEFAULT 50           NOT NULL,
	PRIMARY KEY (userid)
);
CREATE UNIQUE INDEX users_1 ON users (alias);
CREATE TABLE maintenances (
	maintenanceid            bigint                                    NOT NULL,
	name                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	maintenance_type         integer         WITH DEFAULT '0'          NOT NULL,
	description              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	active_since             integer         WITH DEFAULT '0'          NOT NULL,
	active_till              integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (maintenanceid)
);
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);
CREATE UNIQUE INDEX maintenances_2 ON maintenances (name);
CREATE TABLE hosts (
	hostid                   bigint                                    NOT NULL,
	proxy_hostid             bigint                                    NULL,
	host                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	disable_until            integer         WITH DEFAULT '0'          NOT NULL,
	error                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	available                integer         WITH DEFAULT '0'          NOT NULL,
	errors_from              integer         WITH DEFAULT '0'          NOT NULL,
	lastaccess               integer         WITH DEFAULT '0'          NOT NULL,
	ipmi_authtype            integer         WITH DEFAULT '-1'         NOT NULL,
	ipmi_privilege           integer         WITH DEFAULT '2'          NOT NULL,
	ipmi_username            varchar(16)     WITH DEFAULT ''           NOT NULL,
	ipmi_password            varchar(20)     WITH DEFAULT ''           NOT NULL,
	ipmi_disable_until       integer         WITH DEFAULT '0'          NOT NULL,
	ipmi_available           integer         WITH DEFAULT '0'          NOT NULL,
	snmp_disable_until       integer         WITH DEFAULT '0'          NOT NULL,
	snmp_available           integer         WITH DEFAULT '0'          NOT NULL,
	maintenanceid            bigint                                    NULL,
	maintenance_status       integer         WITH DEFAULT '0'          NOT NULL,
	maintenance_type         integer         WITH DEFAULT '0'          NOT NULL,
	maintenance_from         integer         WITH DEFAULT '0'          NOT NULL,
	ipmi_errors_from         integer         WITH DEFAULT '0'          NOT NULL,
	snmp_errors_from         integer         WITH DEFAULT '0'          NOT NULL,
	ipmi_error               varchar(2048)   WITH DEFAULT ''           NOT NULL,
	snmp_error               varchar(2048)   WITH DEFAULT ''           NOT NULL,
	jmx_disable_until        integer         WITH DEFAULT '0'          NOT NULL,
	jmx_available            integer         WITH DEFAULT '0'          NOT NULL,
	jmx_errors_from          integer         WITH DEFAULT '0'          NOT NULL,
	jmx_error                varchar(2048)   WITH DEFAULT ''           NOT NULL,
	name                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	templateid               bigint                                    NULL,
	description              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	tls_connect              integer         WITH DEFAULT '1'          NOT NULL,
	tls_accept               integer         WITH DEFAULT '1'          NOT NULL,
	tls_issuer               varchar(1024)   WITH DEFAULT ''           NOT NULL,
	tls_subject              varchar(1024)   WITH DEFAULT ''           NOT NULL,
	tls_psk_identity         varchar(128)    WITH DEFAULT ''           NOT NULL,
	tls_psk                  varchar(512)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE INDEX hosts_1 ON hosts (host);
CREATE INDEX hosts_2 ON hosts (status);
CREATE INDEX hosts_3 ON hosts (proxy_hostid);
CREATE INDEX hosts_4 ON hosts (name);
CREATE INDEX hosts_5 ON hosts (maintenanceid);
CREATE TABLE groups (
	groupid                  bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	internal                 integer         WITH DEFAULT '0'          NOT NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE INDEX groups_1 ON groups (name);
CREATE TABLE group_prototype (
	group_prototypeid        bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	groupid                  bigint                                    NULL,
	templateid               bigint                                    NULL,
	PRIMARY KEY (group_prototypeid)
);
CREATE INDEX group_prototype_1 ON group_prototype (hostid);
CREATE TABLE group_discovery (
	groupid                  bigint                                    NOT NULL,
	parent_group_prototypeid bigint                                    NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	lastcheck                integer         WITH DEFAULT '0'          NOT NULL,
	ts_delete                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE TABLE screens (
	screenid                 bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	hsize                    integer         WITH DEFAULT '1'          NOT NULL,
	vsize                    integer         WITH DEFAULT '1'          NOT NULL,
	templateid               bigint                                    NULL,
	userid                   bigint                                    NULL,
	private                  integer         WITH DEFAULT '1'          NOT NULL,
	PRIMARY KEY (screenid)
);
CREATE INDEX screens_1 ON screens (templateid);
CREATE TABLE screens_items (
	screenitemid             bigint                                    NOT NULL,
	screenid                 bigint                                    NOT NULL,
	resourcetype             integer         WITH DEFAULT '0'          NOT NULL,
	resourceid               bigint          WITH DEFAULT '0'          NOT NULL,
	width                    integer         WITH DEFAULT '320'        NOT NULL,
	height                   integer         WITH DEFAULT '200'        NOT NULL,
	x                        integer         WITH DEFAULT '0'          NOT NULL,
	y                        integer         WITH DEFAULT '0'          NOT NULL,
	colspan                  integer         WITH DEFAULT '1'          NOT NULL,
	rowspan                  integer         WITH DEFAULT '1'          NOT NULL,
	elements                 integer         WITH DEFAULT '25'         NOT NULL,
	valign                   integer         WITH DEFAULT '0'          NOT NULL,
	halign                   integer         WITH DEFAULT '0'          NOT NULL,
	style                    integer         WITH DEFAULT '0'          NOT NULL,
	url                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	dynamic                  integer         WITH DEFAULT '0'          NOT NULL,
	sort_triggers            integer         WITH DEFAULT '0'          NOT NULL,
	application              varchar(255)    WITH DEFAULT ''           NOT NULL,
	max_columns              integer         WITH DEFAULT '3'          NOT NULL,
	PRIMARY KEY (screenitemid)
);
CREATE INDEX screens_items_1 ON screens_items (screenid);
CREATE TABLE screen_user (
	screenuserid             bigint                                    NOT NULL,
	screenid                 bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (screenuserid)
);
CREATE UNIQUE INDEX screen_user_1 ON screen_user (screenid,userid);
CREATE TABLE screen_usrgrp (
	screenusrgrpid           bigint                                    NOT NULL,
	screenid                 bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (screenusrgrpid)
);
CREATE UNIQUE INDEX screen_usrgrp_1 ON screen_usrgrp (screenid,usrgrpid);
CREATE TABLE slideshows (
	slideshowid              bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	delay                    varchar(32)     WITH DEFAULT '30s'        NOT NULL,
	userid                   bigint                                    NOT NULL,
	private                  integer         WITH DEFAULT '1'          NOT NULL,
	PRIMARY KEY (slideshowid)
);
CREATE UNIQUE INDEX slideshows_1 ON slideshows (name);
CREATE TABLE slideshow_user (
	slideshowuserid          bigint                                    NOT NULL,
	slideshowid              bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (slideshowuserid)
);
CREATE UNIQUE INDEX slideshow_user_1 ON slideshow_user (slideshowid,userid);
CREATE TABLE slideshow_usrgrp (
	slideshowusrgrpid        bigint                                    NOT NULL,
	slideshowid              bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (slideshowusrgrpid)
);
CREATE UNIQUE INDEX slideshow_usrgrp_1 ON slideshow_usrgrp (slideshowid,usrgrpid);
CREATE TABLE slides (
	slideid                  bigint                                    NOT NULL,
	slideshowid              bigint                                    NOT NULL,
	screenid                 bigint                                    NOT NULL,
	step                     integer         WITH DEFAULT '0'          NOT NULL,
	delay                    varchar(32)     WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (slideid)
);
CREATE INDEX slides_1 ON slides (slideshowid);
CREATE INDEX slides_2 ON slides (screenid);
CREATE TABLE drules (
	druleid                  bigint                                    NOT NULL,
	proxy_hostid             bigint                                    NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	iprange                  varchar(2048)   WITH DEFAULT ''           NOT NULL,
	delay                    varchar(255)    WITH DEFAULT '1h'         NOT NULL,
	nextcheck                integer         WITH DEFAULT '0'          NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (druleid)
);
CREATE INDEX drules_1 ON drules (proxy_hostid);
CREATE UNIQUE INDEX drules_2 ON drules (name);
CREATE TABLE dchecks (
	dcheckid                 bigint                                    NOT NULL,
	druleid                  bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	key_                     varchar(512)    WITH DEFAULT ''           NOT NULL,
	snmp_community           varchar(255)    WITH DEFAULT ''           NOT NULL,
	ports                    varchar(255)    WITH DEFAULT '0'          NOT NULL,
	snmpv3_securityname      varchar(64)     WITH DEFAULT ''           NOT NULL,
	snmpv3_securitylevel     integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_authpassphrase    varchar(64)     WITH DEFAULT ''           NOT NULL,
	snmpv3_privpassphrase    varchar(64)     WITH DEFAULT ''           NOT NULL,
	uniq                     integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_authprotocol      integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_privprotocol      integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_contextname       varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (dcheckid)
);
CREATE INDEX dchecks_1 ON dchecks (druleid);
CREATE TABLE applications (
	applicationid            bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (applicationid)
);
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);
CREATE TABLE httptest (
	httptestid               bigint                                    NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	applicationid            bigint                                    NULL,
	nextcheck                integer         WITH DEFAULT '0'          NOT NULL,
	delay                    varchar(255)    WITH DEFAULT '1m'         NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	agent                    varchar(255)    WITH DEFAULT 'Zabbix'     NOT NULL,
	authentication           integer         WITH DEFAULT '0'          NOT NULL,
	http_user                varchar(64)     WITH DEFAULT ''           NOT NULL,
	http_password            varchar(64)     WITH DEFAULT ''           NOT NULL,
	hostid                   bigint                                    NOT NULL,
	templateid               bigint                                    NULL,
	http_proxy               varchar(255)    WITH DEFAULT ''           NOT NULL,
	retries                  integer         WITH DEFAULT '1'          NOT NULL,
	ssl_cert_file            varchar(255)    WITH DEFAULT ''           NOT NULL,
	ssl_key_file             varchar(255)    WITH DEFAULT ''           NOT NULL,
	ssl_key_password         varchar(64)     WITH DEFAULT ''           NOT NULL,
	verify_peer              integer         WITH DEFAULT '0'          NOT NULL,
	verify_host              integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (httptestid)
);
CREATE INDEX httptest_1 ON httptest (applicationid);
CREATE UNIQUE INDEX httptest_2 ON httptest (hostid,name);
CREATE INDEX httptest_3 ON httptest (status);
CREATE INDEX httptest_4 ON httptest (templateid);
CREATE TABLE httpstep (
	httpstepid               bigint                                    NOT NULL,
	httptestid               bigint                                    NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	no                       integer         WITH DEFAULT '0'          NOT NULL,
	url                      varchar(2048)   WITH DEFAULT ''           NOT NULL,
	timeout                  varchar(255)    WITH DEFAULT '15s'        NOT NULL,
	posts                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	required                 varchar(255)    WITH DEFAULT ''           NOT NULL,
	status_codes             varchar(255)    WITH DEFAULT ''           NOT NULL,
	follow_redirects         integer         WITH DEFAULT '1'          NOT NULL,
	retrieve_mode            integer         WITH DEFAULT '0'          NOT NULL,
	post_type                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (httpstepid)
);
CREATE INDEX httpstep_1 ON httpstep (httptestid);
CREATE TABLE interface (
	interfaceid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	main                     integer         WITH DEFAULT '0'          NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	useip                    integer         WITH DEFAULT '1'          NOT NULL,
	ip                       varchar(64)     WITH DEFAULT '127.0.0.1'  NOT NULL,
	dns                      varchar(64)     WITH DEFAULT ''           NOT NULL,
	port                     varchar(64)     WITH DEFAULT '10050'      NOT NULL,
	bulk                     integer         WITH DEFAULT '1'          NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_1 ON interface (hostid,type);
CREATE INDEX interface_2 ON interface (ip,dns);
CREATE TABLE valuemaps (
	valuemapid               bigint                                    NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (valuemapid)
);
CREATE UNIQUE INDEX valuemaps_1 ON valuemaps (name);
CREATE TABLE items (
	itemid                   bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	snmp_community           varchar(64)     WITH DEFAULT ''           NOT NULL,
	snmp_oid                 varchar(512)    WITH DEFAULT ''           NOT NULL,
	hostid                   bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	key_                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	delay                    varchar(1024)   WITH DEFAULT '0'          NOT NULL,
	history                  varchar(255)    WITH DEFAULT '90d'        NOT NULL,
	trends                   varchar(255)    WITH DEFAULT '365d'       NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	value_type               integer         WITH DEFAULT '0'          NOT NULL,
	trapper_hosts            varchar(255)    WITH DEFAULT ''           NOT NULL,
	units                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	snmpv3_securityname      varchar(64)     WITH DEFAULT ''           NOT NULL,
	snmpv3_securitylevel     integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_authpassphrase    varchar(64)     WITH DEFAULT ''           NOT NULL,
	snmpv3_privpassphrase    varchar(64)     WITH DEFAULT ''           NOT NULL,
	formula                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	error                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	lastlogsize              bigint          WITH DEFAULT '0'          NOT NULL,
	logtimefmt               varchar(64)     WITH DEFAULT ''           NOT NULL,
	templateid               bigint                                    NULL,
	valuemapid               bigint                                    NULL,
	params                   varchar(2048)   WITH DEFAULT ''           NOT NULL,
	ipmi_sensor              varchar(128)    WITH DEFAULT ''           NOT NULL,
	authtype                 integer         WITH DEFAULT '0'          NOT NULL,
	username                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	password                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	publickey                varchar(64)     WITH DEFAULT ''           NOT NULL,
	privatekey               varchar(64)     WITH DEFAULT ''           NOT NULL,
	mtime                    integer         WITH DEFAULT '0'          NOT NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	interfaceid              bigint                                    NULL,
	port                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	description              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	inventory_link           integer         WITH DEFAULT '0'          NOT NULL,
	lifetime                 varchar(255)    WITH DEFAULT '30d'        NOT NULL,
	snmpv3_authprotocol      integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_privprotocol      integer         WITH DEFAULT '0'          NOT NULL,
	state                    integer         WITH DEFAULT '0'          NOT NULL,
	snmpv3_contextname       varchar(255)    WITH DEFAULT ''           NOT NULL,
	evaltype                 integer         WITH DEFAULT '0'          NOT NULL,
	jmx_endpoint             varchar(255)    WITH DEFAULT ''           NOT NULL,
	master_itemid            bigint                                    NULL,
	PRIMARY KEY (itemid)
);
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);
CREATE INDEX items_3 ON items (status);
CREATE INDEX items_4 ON items (templateid);
CREATE INDEX items_5 ON items (valuemapid);
CREATE INDEX items_6 ON items (interfaceid);
CREATE INDEX items_7 ON items (master_itemid);
CREATE TABLE httpstepitem (
	httpstepitemid           bigint                                    NOT NULL,
	httpstepid               bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (httpstepitemid)
);
CREATE UNIQUE INDEX httpstepitem_1 ON httpstepitem (httpstepid,itemid);
CREATE INDEX httpstepitem_2 ON httpstepitem (itemid);
CREATE TABLE httptestitem (
	httptestitemid           bigint                                    NOT NULL,
	httptestid               bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (httptestitemid)
);
CREATE UNIQUE INDEX httptestitem_1 ON httptestitem (httptestid,itemid);
CREATE INDEX httptestitem_2 ON httptestitem (itemid);
CREATE TABLE media_type (
	mediatypeid              bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	description              varchar(100)    WITH DEFAULT ''           NOT NULL,
	smtp_server              varchar(255)    WITH DEFAULT ''           NOT NULL,
	smtp_helo                varchar(255)    WITH DEFAULT ''           NOT NULL,
	smtp_email               varchar(255)    WITH DEFAULT ''           NOT NULL,
	exec_path                varchar(255)    WITH DEFAULT ''           NOT NULL,
	gsm_modem                varchar(255)    WITH DEFAULT ''           NOT NULL,
	username                 varchar(255)    WITH DEFAULT ''           NOT NULL,
	passwd                   varchar(255)    WITH DEFAULT ''           NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	smtp_port                integer         WITH DEFAULT '25'         NOT NULL,
	smtp_security            integer         WITH DEFAULT '0'          NOT NULL,
	smtp_verify_peer         integer         WITH DEFAULT '0'          NOT NULL,
	smtp_verify_host         integer         WITH DEFAULT '0'          NOT NULL,
	smtp_authentication      integer         WITH DEFAULT '0'          NOT NULL,
	exec_params              varchar(255)    WITH DEFAULT ''           NOT NULL,
	maxsessions              integer         WITH DEFAULT '1'          NOT NULL,
	maxattempts              integer         WITH DEFAULT '3'          NOT NULL,
	attempt_interval         varchar(32)     WITH DEFAULT '10s'        NOT NULL,
	PRIMARY KEY (mediatypeid)
);
CREATE UNIQUE INDEX media_type_1 ON media_type (description);
CREATE TABLE usrgrp (
	usrgrpid                 bigint                                    NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	gui_access               integer         WITH DEFAULT '0'          NOT NULL,
	users_status             integer         WITH DEFAULT '0'          NOT NULL,
	debug_mode               integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (usrgrpid)
);
CREATE UNIQUE INDEX usrgrp_1 ON usrgrp (name);
CREATE TABLE users_groups (
	id                       bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	PRIMARY KEY (id)
);
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);
CREATE INDEX users_groups_2 ON users_groups (userid);
CREATE TABLE scripts (
	scriptid                 bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	command                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	host_access              integer         WITH DEFAULT '2'          NOT NULL,
	usrgrpid                 bigint                                    NULL,
	groupid                  bigint                                    NULL,
	description              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	confirmation             varchar(255)    WITH DEFAULT ''           NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	execute_on               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (scriptid)
);
CREATE INDEX scripts_1 ON scripts (usrgrpid);
CREATE INDEX scripts_2 ON scripts (groupid);
CREATE UNIQUE INDEX scripts_3 ON scripts (name);
CREATE TABLE actions (
	actionid                 bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	eventsource              integer         WITH DEFAULT '0'          NOT NULL,
	evaltype                 integer         WITH DEFAULT '0'          NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	esc_period               varchar(255)    WITH DEFAULT '1h'         NOT NULL,
	def_shortdata            varchar(255)    WITH DEFAULT ''           NOT NULL,
	def_longdata             varchar(2048)   WITH DEFAULT ''           NOT NULL,
	r_shortdata              varchar(255)    WITH DEFAULT ''           NOT NULL,
	r_longdata               varchar(2048)   WITH DEFAULT ''           NOT NULL,
	formula                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	maintenance_mode         integer         WITH DEFAULT '1'          NOT NULL,
	ack_shortdata            varchar(255)    WITH DEFAULT ''           NOT NULL,
	ack_longdata             varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (actionid)
);
CREATE INDEX actions_1 ON actions (eventsource,status);
CREATE UNIQUE INDEX actions_2 ON actions (name);
CREATE TABLE operations (
	operationid              bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL,
	operationtype            integer         WITH DEFAULT '0'          NOT NULL,
	esc_period               varchar(255)    WITH DEFAULT '0'          NOT NULL,
	esc_step_from            integer         WITH DEFAULT '1'          NOT NULL,
	esc_step_to              integer         WITH DEFAULT '1'          NOT NULL,
	evaltype                 integer         WITH DEFAULT '0'          NOT NULL,
	recovery                 integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE INDEX operations_1 ON operations (actionid);
CREATE TABLE opmessage (
	operationid              bigint                                    NOT NULL,
	default_msg              integer         WITH DEFAULT '0'          NOT NULL,
	subject                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	message                  varchar(2048)   WITH DEFAULT ''           NOT NULL,
	mediatypeid              bigint                                    NULL,
	PRIMARY KEY (operationid)
);
CREATE INDEX opmessage_1 ON opmessage (mediatypeid);
CREATE TABLE opmessage_grp (
	opmessage_grpid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	PRIMARY KEY (opmessage_grpid)
);
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
CREATE INDEX opmessage_grp_2 ON opmessage_grp (usrgrpid);
CREATE TABLE opmessage_usr (
	opmessage_usrid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	PRIMARY KEY (opmessage_usrid)
);
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
CREATE INDEX opmessage_usr_2 ON opmessage_usr (userid);
CREATE TABLE opcommand (
	operationid              bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	scriptid                 bigint                                    NULL,
	execute_on               integer         WITH DEFAULT '0'          NOT NULL,
	port                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	authtype                 integer         WITH DEFAULT '0'          NOT NULL,
	username                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	password                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	publickey                varchar(64)     WITH DEFAULT ''           NOT NULL,
	privatekey               varchar(64)     WITH DEFAULT ''           NOT NULL,
	command                  varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE INDEX opcommand_1 ON opcommand (scriptid);
CREATE TABLE opcommand_hst (
	opcommand_hstid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	hostid                   bigint                                    NULL,
	PRIMARY KEY (opcommand_hstid)
);
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
CREATE INDEX opcommand_hst_2 ON opcommand_hst (hostid);
CREATE TABLE opcommand_grp (
	opcommand_grpid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (opcommand_grpid)
);
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
CREATE INDEX opcommand_grp_2 ON opcommand_grp (groupid);
CREATE TABLE opgroup (
	opgroupid                bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (opgroupid)
);
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
CREATE INDEX opgroup_2 ON opgroup (groupid);
CREATE TABLE optemplate (
	optemplateid             bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	templateid               bigint                                    NOT NULL,
	PRIMARY KEY (optemplateid)
);
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
CREATE INDEX optemplate_2 ON optemplate (templateid);
CREATE TABLE opconditions (
	opconditionid            bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL,
	conditiontype            integer         WITH DEFAULT '0'          NOT NULL,
	operator                 integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (opconditionid)
);
CREATE INDEX opconditions_1 ON opconditions (operationid);
CREATE TABLE conditions (
	conditionid              bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL,
	conditiontype            integer         WITH DEFAULT '0'          NOT NULL,
	operator                 integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	value2                   varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (conditionid)
);
CREATE INDEX conditions_1 ON conditions (actionid);
CREATE TABLE config (
	configid                 bigint                                    NOT NULL,
	refresh_unsupported      varchar(32)     WITH DEFAULT '10m'        NOT NULL,
	work_period              varchar(255)    WITH DEFAULT '1-5,09:00-18:00' NOT NULL,
	alert_usrgrpid           bigint                                    NULL,
	event_ack_enable         integer         WITH DEFAULT '1'          NOT NULL,
	event_expire             varchar(32)     WITH DEFAULT '1w'         NOT NULL,
	event_show_max           integer         WITH DEFAULT '100'        NOT NULL,
	default_theme            varchar(128)    WITH DEFAULT 'blue-theme' NOT NULL,
	authentication_type      integer         WITH DEFAULT '0'          NOT NULL,
	ldap_host                varchar(255)    WITH DEFAULT ''           NOT NULL,
	ldap_port                integer         WITH DEFAULT 389          NOT NULL,
	ldap_base_dn             varchar(255)    WITH DEFAULT ''           NOT NULL,
	ldap_bind_dn             varchar(255)    WITH DEFAULT ''           NOT NULL,
	ldap_bind_password       varchar(128)    WITH DEFAULT ''           NOT NULL,
	ldap_search_attribute    varchar(128)    WITH DEFAULT ''           NOT NULL,
	dropdown_first_entry     integer         WITH DEFAULT '1'          NOT NULL,
	dropdown_first_remember  integer         WITH DEFAULT '1'          NOT NULL,
	discovery_groupid        bigint                                    NOT NULL,
	max_in_table             integer         WITH DEFAULT '50'         NOT NULL,
	search_limit             integer         WITH DEFAULT '1000'       NOT NULL,
	severity_color_0         varchar(6)      WITH DEFAULT '97AAB3'     NOT NULL,
	severity_color_1         varchar(6)      WITH DEFAULT '7499FF'     NOT NULL,
	severity_color_2         varchar(6)      WITH DEFAULT 'FFC859'     NOT NULL,
	severity_color_3         varchar(6)      WITH DEFAULT 'FFA059'     NOT NULL,
	severity_color_4         varchar(6)      WITH DEFAULT 'E97659'     NOT NULL,
	severity_color_5         varchar(6)      WITH DEFAULT 'E45959'     NOT NULL,
	severity_name_0          varchar(32)     WITH DEFAULT 'Not classified' NOT NULL,
	severity_name_1          varchar(32)     WITH DEFAULT 'Information' NOT NULL,
	severity_name_2          varchar(32)     WITH DEFAULT 'Warning'    NOT NULL,
	severity_name_3          varchar(32)     WITH DEFAULT 'Average'    NOT NULL,
	severity_name_4          varchar(32)     WITH DEFAULT 'High'       NOT NULL,
	severity_name_5          varchar(32)     WITH DEFAULT 'Disaster'   NOT NULL,
	ok_period                varchar(32)     WITH DEFAULT '30m'        NOT NULL,
	blink_period             varchar(32)     WITH DEFAULT '30m'        NOT NULL,
	problem_unack_color      varchar(6)      WITH DEFAULT 'DC0000'     NOT NULL,
	problem_ack_color        varchar(6)      WITH DEFAULT 'DC0000'     NOT NULL,
	ok_unack_color           varchar(6)      WITH DEFAULT '00AA00'     NOT NULL,
	ok_ack_color             varchar(6)      WITH DEFAULT '00AA00'     NOT NULL,
	problem_unack_style      integer         WITH DEFAULT '1'          NOT NULL,
	problem_ack_style        integer         WITH DEFAULT '1'          NOT NULL,
	ok_unack_style           integer         WITH DEFAULT '1'          NOT NULL,
	ok_ack_style             integer         WITH DEFAULT '1'          NOT NULL,
	snmptrap_logging         integer         WITH DEFAULT '1'          NOT NULL,
	server_check_interval    integer         WITH DEFAULT '10'         NOT NULL,
	hk_events_mode           integer         WITH DEFAULT '1'          NOT NULL,
	hk_events_trigger        varchar(32)     WITH DEFAULT '365d'       NOT NULL,
	hk_events_internal       varchar(32)     WITH DEFAULT '1d'         NOT NULL,
	hk_events_discovery      varchar(32)     WITH DEFAULT '1d'         NOT NULL,
	hk_events_autoreg        varchar(32)     WITH DEFAULT '1d'         NOT NULL,
	hk_services_mode         integer         WITH DEFAULT '1'          NOT NULL,
	hk_services              varchar(32)     WITH DEFAULT '365d'       NOT NULL,
	hk_audit_mode            integer         WITH DEFAULT '1'          NOT NULL,
	hk_audit                 varchar(32)     WITH DEFAULT '365d'       NOT NULL,
	hk_sessions_mode         integer         WITH DEFAULT '1'          NOT NULL,
	hk_sessions              varchar(32)     WITH DEFAULT '365d'       NOT NULL,
	hk_history_mode          integer         WITH DEFAULT '1'          NOT NULL,
	hk_history_global        integer         WITH DEFAULT '0'          NOT NULL,
	hk_history               varchar(32)     WITH DEFAULT '90d'        NOT NULL,
	hk_trends_mode           integer         WITH DEFAULT '1'          NOT NULL,
	hk_trends_global         integer         WITH DEFAULT '0'          NOT NULL,
	hk_trends                varchar(32)     WITH DEFAULT '365d'       NOT NULL,
	default_inventory_mode   integer         WITH DEFAULT '-1'         NOT NULL,
	PRIMARY KEY (configid)
);
CREATE INDEX config_1 ON config (alert_usrgrpid);
CREATE INDEX config_2 ON config (discovery_groupid);
CREATE TABLE triggers (
	triggerid                bigint                                    NOT NULL,
	expression               varchar(2048)   WITH DEFAULT ''           NOT NULL,
	description              varchar(255)    WITH DEFAULT ''           NOT NULL,
	url                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	value                    integer         WITH DEFAULT '0'          NOT NULL,
	priority                 integer         WITH DEFAULT '0'          NOT NULL,
	lastchange               integer         WITH DEFAULT '0'          NOT NULL,
	comments                 varchar(2048)   WITH DEFAULT ''           NOT NULL,
	error                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	templateid               bigint                                    NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	state                    integer         WITH DEFAULT '0'          NOT NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	recovery_mode            integer         WITH DEFAULT '0'          NOT NULL,
	recovery_expression      varchar(2048)   WITH DEFAULT ''           NOT NULL,
	correlation_mode         integer         WITH DEFAULT '0'          NOT NULL,
	correlation_tag          varchar(255)    WITH DEFAULT ''           NOT NULL,
	manual_close             integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX triggers_1 ON triggers (status);
CREATE INDEX triggers_2 ON triggers (value,lastchange);
CREATE INDEX triggers_3 ON triggers (templateid);
CREATE TABLE trigger_depends (
	triggerdepid             bigint                                    NOT NULL,
	triggerid_down           bigint                                    NOT NULL,
	triggerid_up             bigint                                    NOT NULL,
	PRIMARY KEY (triggerdepid)
);
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);
CREATE TABLE functions (
	functionid               bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	triggerid                bigint                                    NOT NULL,
	function                 varchar(12)     WITH DEFAULT ''           NOT NULL,
	parameter                varchar(255)    WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (functionid)
);
CREATE INDEX functions_1 ON functions (triggerid);
CREATE INDEX functions_2 ON functions (itemid,function,parameter);
CREATE TABLE graphs (
	graphid                  bigint                                    NOT NULL,
	name                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	width                    integer         WITH DEFAULT '900'        NOT NULL,
	height                   integer         WITH DEFAULT '200'        NOT NULL,
	yaxismin                 decfloat(16)    WITH DEFAULT '0'          NOT NULL,
	yaxismax                 decfloat(16)    WITH DEFAULT '100'        NOT NULL,
	templateid               bigint                                    NULL,
	show_work_period         integer         WITH DEFAULT '1'          NOT NULL,
	show_triggers            integer         WITH DEFAULT '1'          NOT NULL,
	graphtype                integer         WITH DEFAULT '0'          NOT NULL,
	show_legend              integer         WITH DEFAULT '1'          NOT NULL,
	show_3d                  integer         WITH DEFAULT '0'          NOT NULL,
	percent_left             decfloat(16)    WITH DEFAULT '0'          NOT NULL,
	percent_right            decfloat(16)    WITH DEFAULT '0'          NOT NULL,
	ymin_type                integer         WITH DEFAULT '0'          NOT NULL,
	ymax_type                integer         WITH DEFAULT '0'          NOT NULL,
	ymin_itemid              bigint                                    NULL,
	ymax_itemid              bigint                                    NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (graphid)
);
CREATE INDEX graphs_1 ON graphs (name);
CREATE INDEX graphs_2 ON graphs (templateid);
CREATE INDEX graphs_3 ON graphs (ymin_itemid);
CREATE INDEX graphs_4 ON graphs (ymax_itemid);
CREATE TABLE graphs_items (
	gitemid                  bigint                                    NOT NULL,
	graphid                  bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	drawtype                 integer         WITH DEFAULT '0'          NOT NULL,
	sortorder                integer         WITH DEFAULT '0'          NOT NULL,
	color                    varchar(6)      WITH DEFAULT '009600'     NOT NULL,
	yaxisside                integer         WITH DEFAULT '0'          NOT NULL,
	calc_fnc                 integer         WITH DEFAULT '2'          NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (gitemid)
);
CREATE INDEX graphs_items_1 ON graphs_items (itemid);
CREATE INDEX graphs_items_2 ON graphs_items (graphid);
CREATE TABLE graph_theme (
	graphthemeid             bigint                                    NOT NULL,
	theme                    varchar(64)     WITH DEFAULT ''           NOT NULL,
	backgroundcolor          varchar(6)      WITH DEFAULT ''           NOT NULL,
	graphcolor               varchar(6)      WITH DEFAULT ''           NOT NULL,
	gridcolor                varchar(6)      WITH DEFAULT ''           NOT NULL,
	maingridcolor            varchar(6)      WITH DEFAULT ''           NOT NULL,
	gridbordercolor          varchar(6)      WITH DEFAULT ''           NOT NULL,
	textcolor                varchar(6)      WITH DEFAULT ''           NOT NULL,
	highlightcolor           varchar(6)      WITH DEFAULT ''           NOT NULL,
	leftpercentilecolor      varchar(6)      WITH DEFAULT ''           NOT NULL,
	rightpercentilecolor     varchar(6)      WITH DEFAULT ''           NOT NULL,
	nonworktimecolor         varchar(6)      WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (graphthemeid)
);
CREATE UNIQUE INDEX graph_theme_1 ON graph_theme (theme);
CREATE TABLE globalmacro (
	globalmacroid            bigint                                    NOT NULL,
	macro                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (globalmacroid)
);
CREATE UNIQUE INDEX globalmacro_1 ON globalmacro (macro);
CREATE TABLE hostmacro (
	hostmacroid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	macro                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (hostmacroid)
);
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);
CREATE TABLE hosts_groups (
	hostgroupid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (hostgroupid)
);
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);
CREATE TABLE hosts_templates (
	hosttemplateid           bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	templateid               bigint                                    NOT NULL,
	PRIMARY KEY (hosttemplateid)
);
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
CREATE TABLE items_applications (
	itemappid                bigint                                    NOT NULL,
	applicationid            bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	PRIMARY KEY (itemappid)
);
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);
CREATE INDEX items_applications_2 ON items_applications (itemid);
CREATE TABLE mappings (
	mappingid                bigint                                    NOT NULL,
	valuemapid               bigint                                    NOT NULL,
	value                    varchar(64)     WITH DEFAULT ''           NOT NULL,
	newvalue                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (mappingid)
);
CREATE INDEX mappings_1 ON mappings (valuemapid);
CREATE TABLE media (
	mediaid                  bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	mediatypeid              bigint                                    NOT NULL,
	sendto                   varchar(100)    WITH DEFAULT ''           NOT NULL,
	active                   integer         WITH DEFAULT '0'          NOT NULL,
	severity                 integer         WITH DEFAULT '63'         NOT NULL,
	period                   varchar(1024)   WITH DEFAULT '1-7,00:00-24:00' NOT NULL,
	PRIMARY KEY (mediaid)
);
CREATE INDEX media_1 ON media (userid);
CREATE INDEX media_2 ON media (mediatypeid);
CREATE TABLE rights (
	rightid                  bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '0'          NOT NULL,
	id                       bigint                                    NOT NULL,
	PRIMARY KEY (rightid)
);
CREATE INDEX rights_1 ON rights (groupid);
CREATE INDEX rights_2 ON rights (id);
CREATE TABLE services (
	serviceid                bigint                                    NOT NULL,
	name                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	algorithm                integer         WITH DEFAULT '0'          NOT NULL,
	triggerid                bigint                                    NULL,
	showsla                  integer         WITH DEFAULT '0'          NOT NULL,
	goodsla                  decfloat(16)    WITH DEFAULT '99.9'       NOT NULL,
	sortorder                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (serviceid)
);
CREATE INDEX services_1 ON services (triggerid);
CREATE TABLE services_links (
	linkid                   bigint                                    NOT NULL,
	serviceupid              bigint                                    NOT NULL,
	servicedownid            bigint                                    NOT NULL,
	soft                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (linkid)
);
CREATE INDEX services_links_1 ON services_links (servicedownid);
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);
CREATE TABLE services_times (
	timeid                   bigint                                    NOT NULL,
	serviceid                bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	ts_from                  integer         WITH DEFAULT '0'          NOT NULL,
	ts_to                    integer         WITH DEFAULT '0'          NOT NULL,
	note                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (timeid)
);
CREATE INDEX services_times_1 ON services_times (serviceid,type,ts_from,ts_to);
CREATE TABLE icon_map (
	iconmapid                bigint                                    NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	default_iconid           bigint                                    NOT NULL,
	PRIMARY KEY (iconmapid)
);
CREATE UNIQUE INDEX icon_map_1 ON icon_map (name);
CREATE INDEX icon_map_2 ON icon_map (default_iconid);
CREATE TABLE icon_mapping (
	iconmappingid            bigint                                    NOT NULL,
	iconmapid                bigint                                    NOT NULL,
	iconid                   bigint                                    NOT NULL,
	inventory_link           integer         WITH DEFAULT '0'          NOT NULL,
	expression               varchar(64)     WITH DEFAULT ''           NOT NULL,
	sortorder                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (iconmappingid)
);
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);
CREATE INDEX icon_mapping_2 ON icon_mapping (iconid);
CREATE TABLE sysmaps (
	sysmapid                 bigint                                    NOT NULL,
	name                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	width                    integer         WITH DEFAULT '600'        NOT NULL,
	height                   integer         WITH DEFAULT '400'        NOT NULL,
	backgroundid             bigint                                    NULL,
	label_type               integer         WITH DEFAULT '2'          NOT NULL,
	label_location           integer         WITH DEFAULT '0'          NOT NULL,
	highlight                integer         WITH DEFAULT '1'          NOT NULL,
	expandproblem            integer         WITH DEFAULT '1'          NOT NULL,
	markelements             integer         WITH DEFAULT '0'          NOT NULL,
	show_unack               integer         WITH DEFAULT '0'          NOT NULL,
	grid_size                integer         WITH DEFAULT '50'         NOT NULL,
	grid_show                integer         WITH DEFAULT '1'          NOT NULL,
	grid_align               integer         WITH DEFAULT '1'          NOT NULL,
	label_format             integer         WITH DEFAULT '0'          NOT NULL,
	label_type_host          integer         WITH DEFAULT '2'          NOT NULL,
	label_type_hostgroup     integer         WITH DEFAULT '2'          NOT NULL,
	label_type_trigger       integer         WITH DEFAULT '2'          NOT NULL,
	label_type_map           integer         WITH DEFAULT '2'          NOT NULL,
	label_type_image         integer         WITH DEFAULT '2'          NOT NULL,
	label_string_host        varchar(255)    WITH DEFAULT ''           NOT NULL,
	label_string_hostgroup   varchar(255)    WITH DEFAULT ''           NOT NULL,
	label_string_trigger     varchar(255)    WITH DEFAULT ''           NOT NULL,
	label_string_map         varchar(255)    WITH DEFAULT ''           NOT NULL,
	label_string_image       varchar(255)    WITH DEFAULT ''           NOT NULL,
	iconmapid                bigint                                    NULL,
	expand_macros            integer         WITH DEFAULT '0'          NOT NULL,
	severity_min             integer         WITH DEFAULT '0'          NOT NULL,
	userid                   bigint                                    NOT NULL,
	private                  integer         WITH DEFAULT '1'          NOT NULL,
	PRIMARY KEY (sysmapid)
);
CREATE UNIQUE INDEX sysmaps_1 ON sysmaps (name);
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);
CREATE TABLE sysmaps_elements (
	selementid               bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	elementid                bigint          WITH DEFAULT '0'          NOT NULL,
	elementtype              integer         WITH DEFAULT '0'          NOT NULL,
	iconid_off               bigint                                    NULL,
	iconid_on                bigint                                    NULL,
	label                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	label_location           integer         WITH DEFAULT '-1'         NOT NULL,
	x                        integer         WITH DEFAULT '0'          NOT NULL,
	y                        integer         WITH DEFAULT '0'          NOT NULL,
	iconid_disabled          bigint                                    NULL,
	iconid_maintenance       bigint                                    NULL,
	elementsubtype           integer         WITH DEFAULT '0'          NOT NULL,
	areatype                 integer         WITH DEFAULT '0'          NOT NULL,
	width                    integer         WITH DEFAULT '200'        NOT NULL,
	height                   integer         WITH DEFAULT '200'        NOT NULL,
	viewtype                 integer         WITH DEFAULT '0'          NOT NULL,
	use_iconmap              integer         WITH DEFAULT '1'          NOT NULL,
	application              varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (selementid)
);
CREATE INDEX sysmaps_elements_1 ON sysmaps_elements (sysmapid);
CREATE INDEX sysmaps_elements_2 ON sysmaps_elements (iconid_off);
CREATE INDEX sysmaps_elements_3 ON sysmaps_elements (iconid_on);
CREATE INDEX sysmaps_elements_4 ON sysmaps_elements (iconid_disabled);
CREATE INDEX sysmaps_elements_5 ON sysmaps_elements (iconid_maintenance);
CREATE TABLE sysmaps_links (
	linkid                   bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	selementid1              bigint                                    NOT NULL,
	selementid2              bigint                                    NOT NULL,
	drawtype                 integer         WITH DEFAULT '0'          NOT NULL,
	color                    varchar(6)      WITH DEFAULT '000000'     NOT NULL,
	label                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (linkid)
);
CREATE INDEX sysmaps_links_1 ON sysmaps_links (sysmapid);
CREATE INDEX sysmaps_links_2 ON sysmaps_links (selementid1);
CREATE INDEX sysmaps_links_3 ON sysmaps_links (selementid2);
CREATE TABLE sysmaps_link_triggers (
	linktriggerid            bigint                                    NOT NULL,
	linkid                   bigint                                    NOT NULL,
	triggerid                bigint                                    NOT NULL,
	drawtype                 integer         WITH DEFAULT '0'          NOT NULL,
	color                    varchar(6)      WITH DEFAULT '000000'     NOT NULL,
	PRIMARY KEY (linktriggerid)
);
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);
CREATE INDEX sysmaps_link_triggers_2 ON sysmaps_link_triggers (triggerid);
CREATE TABLE sysmap_element_url (
	sysmapelementurlid       bigint                                    NOT NULL,
	selementid               bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (sysmapelementurlid)
);
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);
CREATE TABLE sysmap_url (
	sysmapurlid              bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	elementtype              integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (sysmapurlid)
);
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);
CREATE TABLE sysmap_user (
	sysmapuserid             bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (sysmapuserid)
);
CREATE UNIQUE INDEX sysmap_user_1 ON sysmap_user (sysmapid,userid);
CREATE TABLE sysmap_usrgrp (
	sysmapusrgrpid           bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (sysmapusrgrpid)
);
CREATE UNIQUE INDEX sysmap_usrgrp_1 ON sysmap_usrgrp (sysmapid,usrgrpid);
CREATE TABLE maintenances_hosts (
	maintenance_hostid       bigint                                    NOT NULL,
	maintenanceid            bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	PRIMARY KEY (maintenance_hostid)
);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
CREATE INDEX maintenances_hosts_2 ON maintenances_hosts (hostid);
CREATE TABLE maintenances_groups (
	maintenance_groupid      bigint                                    NOT NULL,
	maintenanceid            bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (maintenance_groupid)
);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
CREATE INDEX maintenances_groups_2 ON maintenances_groups (groupid);
CREATE TABLE timeperiods (
	timeperiodid             bigint                                    NOT NULL,
	timeperiod_type          integer         WITH DEFAULT '0'          NOT NULL,
	every                    integer         WITH DEFAULT '1'          NOT NULL,
	month                    integer         WITH DEFAULT '0'          NOT NULL,
	dayofweek                integer         WITH DEFAULT '0'          NOT NULL,
	day                      integer         WITH DEFAULT '0'          NOT NULL,
	start_time               integer         WITH DEFAULT '0'          NOT NULL,
	period                   integer         WITH DEFAULT '0'          NOT NULL,
	start_date               integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (timeperiodid)
);
CREATE TABLE maintenances_windows (
	maintenance_timeperiodid bigint                                    NOT NULL,
	maintenanceid            bigint                                    NOT NULL,
	timeperiodid             bigint                                    NOT NULL,
	PRIMARY KEY (maintenance_timeperiodid)
);
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);
CREATE INDEX maintenances_windows_2 ON maintenances_windows (timeperiodid);
CREATE TABLE regexps (
	regexpid                 bigint                                    NOT NULL,
	name                     varchar(128)    WITH DEFAULT ''           NOT NULL,
	test_string              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (regexpid)
);
CREATE UNIQUE INDEX regexps_1 ON regexps (name);
CREATE TABLE expressions (
	expressionid             bigint                                    NOT NULL,
	regexpid                 bigint                                    NOT NULL,
	expression               varchar(255)    WITH DEFAULT ''           NOT NULL,
	expression_type          integer         WITH DEFAULT '0'          NOT NULL,
	exp_delimiter            varchar(1)      WITH DEFAULT ''           NOT NULL,
	case_sensitive           integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (expressionid)
);
CREATE INDEX expressions_1 ON expressions (regexpid);
CREATE TABLE ids (
	table_name               varchar(64)     WITH DEFAULT ''           NOT NULL,
	field_name               varchar(64)     WITH DEFAULT ''           NOT NULL,
	nextid                   bigint                                    NOT NULL,
	PRIMARY KEY (table_name,field_name)
);
CREATE TABLE alerts (
	alertid                  bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL,
	eventid                  bigint                                    NOT NULL,
	userid                   bigint                                    NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	mediatypeid              bigint                                    NULL,
	sendto                   varchar(100)    WITH DEFAULT ''           NOT NULL,
	subject                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	message                  varchar(2048)   WITH DEFAULT ''           NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	retries                  integer         WITH DEFAULT '0'          NOT NULL,
	error                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	esc_step                 integer         WITH DEFAULT '0'          NOT NULL,
	alerttype                integer         WITH DEFAULT '0'          NOT NULL,
	p_eventid                bigint                                    NULL,
	acknowledgeid            bigint                                    NULL,
	PRIMARY KEY (alertid)
);
CREATE INDEX alerts_1 ON alerts (actionid);
CREATE INDEX alerts_2 ON alerts (clock);
CREATE INDEX alerts_3 ON alerts (eventid);
CREATE INDEX alerts_4 ON alerts (status);
CREATE INDEX alerts_5 ON alerts (mediatypeid);
CREATE INDEX alerts_6 ON alerts (userid);
CREATE INDEX alerts_7 ON alerts (p_eventid);
CREATE TABLE history (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	value                    decfloat(16)    WITH DEFAULT '0.0000'     NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL
);
CREATE INDEX history_1 ON history (itemid,clock);
CREATE TABLE history_uint (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	value                    bigint          WITH DEFAULT '0'          NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL
);
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);
CREATE TABLE history_str (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL
);
CREATE INDEX history_str_1 ON history_str (itemid,clock);
CREATE TABLE history_log (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	timestamp                integer         WITH DEFAULT '0'          NOT NULL,
	source                   varchar(64)     WITH DEFAULT ''           NOT NULL,
	severity                 integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	logeventid               integer         WITH DEFAULT '0'          NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL
);
CREATE INDEX history_log_1 ON history_log (itemid,clock);
CREATE TABLE history_text (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL
);
CREATE INDEX history_text_1 ON history_text (itemid,clock);
CREATE TABLE proxy_history (
	id                       bigint                                    NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	timestamp                integer         WITH DEFAULT '0'          NOT NULL,
	source                   varchar(64)     WITH DEFAULT ''           NOT NULL,
	severity                 integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	logeventid               integer         WITH DEFAULT '0'          NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL,
	state                    integer         WITH DEFAULT '0'          NOT NULL,
	lastlogsize              bigint          WITH DEFAULT '0'          NOT NULL,
	mtime                    integer         WITH DEFAULT '0'          NOT NULL,
	flags                    integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_history_1 ON proxy_history (clock);
CREATE TABLE proxy_dhistory (
	id                       bigint                                    NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	druleid                  bigint                                    NOT NULL,
	ip                       varchar(39)     WITH DEFAULT ''           NOT NULL,
	port                     integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	dcheckid                 bigint                                    NULL,
	dns                      varchar(64)     WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);
CREATE TABLE events (
	eventid                  bigint                                    NOT NULL,
	source                   integer         WITH DEFAULT '0'          NOT NULL,
	object                   integer         WITH DEFAULT '0'          NOT NULL,
	objectid                 bigint          WITH DEFAULT '0'          NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	value                    integer         WITH DEFAULT '0'          NOT NULL,
	acknowledged             integer         WITH DEFAULT '0'          NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX events_1 ON events (source,object,objectid,clock);
CREATE INDEX events_2 ON events (source,object,clock);
CREATE TABLE trends (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	num                      integer         WITH DEFAULT '0'          NOT NULL,
	value_min                decfloat(16)    WITH DEFAULT '0.0000'     NOT NULL,
	value_avg                decfloat(16)    WITH DEFAULT '0.0000'     NOT NULL,
	value_max                decfloat(16)    WITH DEFAULT '0.0000'     NOT NULL,
	PRIMARY KEY (itemid,clock)
);
CREATE TABLE trends_uint (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	num                      integer         WITH DEFAULT '0'          NOT NULL,
	value_min                bigint          WITH DEFAULT '0'          NOT NULL,
	value_avg                bigint          WITH DEFAULT '0'          NOT NULL,
	value_max                bigint          WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (itemid,clock)
);
CREATE TABLE acknowledges (
	acknowledgeid            bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	eventid                  bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	message                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	action                   integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (acknowledgeid)
);
CREATE INDEX acknowledges_1 ON acknowledges (userid);
CREATE INDEX acknowledges_2 ON acknowledges (eventid);
CREATE INDEX acknowledges_3 ON acknowledges (clock);
CREATE TABLE auditlog (
	auditid                  bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	action                   integer         WITH DEFAULT '0'          NOT NULL,
	resourcetype             integer         WITH DEFAULT '0'          NOT NULL,
	details                  varchar(128)    WITH DEFAULT '0'          NOT NULL,
	ip                       varchar(39)     WITH DEFAULT ''           NOT NULL,
	resourceid               bigint          WITH DEFAULT '0'          NOT NULL,
	resourcename             varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (auditid)
);
CREATE INDEX auditlog_1 ON auditlog (userid,clock);
CREATE INDEX auditlog_2 ON auditlog (clock);
CREATE TABLE auditlog_details (
	auditdetailid            bigint                                    NOT NULL,
	auditid                  bigint                                    NOT NULL,
	table_name               varchar(64)     WITH DEFAULT ''           NOT NULL,
	field_name               varchar(64)     WITH DEFAULT ''           NOT NULL,
	oldvalue                 varchar(2048)   WITH DEFAULT ''           NOT NULL,
	newvalue                 varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (auditdetailid)
);
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);
CREATE TABLE service_alarms (
	servicealarmid           bigint                                    NOT NULL,
	serviceid                bigint                                    NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	value                    integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (servicealarmid)
);
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);
CREATE INDEX service_alarms_2 ON service_alarms (clock);
CREATE TABLE autoreg_host (
	autoreg_hostid           bigint                                    NOT NULL,
	proxy_hostid             bigint                                    NULL,
	host                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	listen_ip                varchar(39)     WITH DEFAULT ''           NOT NULL,
	listen_port              integer         WITH DEFAULT '0'          NOT NULL,
	listen_dns               varchar(64)     WITH DEFAULT ''           NOT NULL,
	host_metadata            varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (autoreg_hostid)
);
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);
CREATE TABLE proxy_autoreg_host (
	id                       bigint                                    NOT NULL	GENERATED ALWAYS AS IDENTITY (START WITH 1 INCREMENT BY 1),
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	host                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	listen_ip                varchar(39)     WITH DEFAULT ''           NOT NULL,
	listen_port              integer         WITH DEFAULT '0'          NOT NULL,
	listen_dns               varchar(64)     WITH DEFAULT ''           NOT NULL,
	host_metadata            varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);
CREATE TABLE dhosts (
	dhostid                  bigint                                    NOT NULL,
	druleid                  bigint                                    NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	lastup                   integer         WITH DEFAULT '0'          NOT NULL,
	lastdown                 integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (dhostid)
);
CREATE INDEX dhosts_1 ON dhosts (druleid);
CREATE TABLE dservices (
	dserviceid               bigint                                    NOT NULL,
	dhostid                  bigint                                    NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	port                     integer         WITH DEFAULT '0'          NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	lastup                   integer         WITH DEFAULT '0'          NOT NULL,
	lastdown                 integer         WITH DEFAULT '0'          NOT NULL,
	dcheckid                 bigint                                    NOT NULL,
	ip                       varchar(39)     WITH DEFAULT ''           NOT NULL,
	dns                      varchar(64)     WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (dserviceid)
);
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,ip,port);
CREATE INDEX dservices_2 ON dservices (dhostid);
CREATE TABLE escalations (
	escalationid             bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL,
	triggerid                bigint                                    NULL,
	eventid                  bigint                                    NULL,
	r_eventid                bigint                                    NULL,
	nextcheck                integer         WITH DEFAULT '0'          NOT NULL,
	esc_step                 integer         WITH DEFAULT '0'          NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	itemid                   bigint                                    NULL,
	acknowledgeid            bigint                                    NULL,
	PRIMARY KEY (escalationid)
);
CREATE UNIQUE INDEX escalations_1 ON escalations (actionid,triggerid,itemid,escalationid);
CREATE TABLE globalvars (
	globalvarid              bigint                                    NOT NULL,
	snmp_lastsize            bigint          WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (globalvarid)
);
CREATE TABLE graph_discovery (
	graphid                  bigint                                    NOT NULL,
	parent_graphid           bigint                                    NOT NULL,
	PRIMARY KEY (graphid)
);
CREATE INDEX graph_discovery_1 ON graph_discovery (parent_graphid);
CREATE TABLE host_inventory (
	hostid                   bigint                                    NOT NULL,
	inventory_mode           integer         WITH DEFAULT '0'          NOT NULL,
	type                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	type_full                varchar(64)     WITH DEFAULT ''           NOT NULL,
	name                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	alias                    varchar(64)     WITH DEFAULT ''           NOT NULL,
	os                       varchar(64)     WITH DEFAULT ''           NOT NULL,
	os_full                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	os_short                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	serialno_a               varchar(64)     WITH DEFAULT ''           NOT NULL,
	serialno_b               varchar(64)     WITH DEFAULT ''           NOT NULL,
	tag                      varchar(64)     WITH DEFAULT ''           NOT NULL,
	asset_tag                varchar(64)     WITH DEFAULT ''           NOT NULL,
	macaddress_a             varchar(64)     WITH DEFAULT ''           NOT NULL,
	macaddress_b             varchar(64)     WITH DEFAULT ''           NOT NULL,
	hardware                 varchar(255)    WITH DEFAULT ''           NOT NULL,
	hardware_full            varchar(2048)   WITH DEFAULT ''           NOT NULL,
	software                 varchar(255)    WITH DEFAULT ''           NOT NULL,
	software_full            varchar(2048)   WITH DEFAULT ''           NOT NULL,
	software_app_a           varchar(64)     WITH DEFAULT ''           NOT NULL,
	software_app_b           varchar(64)     WITH DEFAULT ''           NOT NULL,
	software_app_c           varchar(64)     WITH DEFAULT ''           NOT NULL,
	software_app_d           varchar(64)     WITH DEFAULT ''           NOT NULL,
	software_app_e           varchar(64)     WITH DEFAULT ''           NOT NULL,
	contact                  varchar(2048)   WITH DEFAULT ''           NOT NULL,
	location                 varchar(2048)   WITH DEFAULT ''           NOT NULL,
	location_lat             varchar(16)     WITH DEFAULT ''           NOT NULL,
	location_lon             varchar(16)     WITH DEFAULT ''           NOT NULL,
	notes                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	chassis                  varchar(64)     WITH DEFAULT ''           NOT NULL,
	model                    varchar(64)     WITH DEFAULT ''           NOT NULL,
	hw_arch                  varchar(32)     WITH DEFAULT ''           NOT NULL,
	vendor                   varchar(64)     WITH DEFAULT ''           NOT NULL,
	contract_number          varchar(64)     WITH DEFAULT ''           NOT NULL,
	installer_name           varchar(64)     WITH DEFAULT ''           NOT NULL,
	deployment_status        varchar(64)     WITH DEFAULT ''           NOT NULL,
	url_a                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	url_b                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	url_c                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	host_networks            varchar(2048)   WITH DEFAULT ''           NOT NULL,
	host_netmask             varchar(39)     WITH DEFAULT ''           NOT NULL,
	host_router              varchar(39)     WITH DEFAULT ''           NOT NULL,
	oob_ip                   varchar(39)     WITH DEFAULT ''           NOT NULL,
	oob_netmask              varchar(39)     WITH DEFAULT ''           NOT NULL,
	oob_router               varchar(39)     WITH DEFAULT ''           NOT NULL,
	date_hw_purchase         varchar(64)     WITH DEFAULT ''           NOT NULL,
	date_hw_install          varchar(64)     WITH DEFAULT ''           NOT NULL,
	date_hw_expiry           varchar(64)     WITH DEFAULT ''           NOT NULL,
	date_hw_decomm           varchar(64)     WITH DEFAULT ''           NOT NULL,
	site_address_a           varchar(128)    WITH DEFAULT ''           NOT NULL,
	site_address_b           varchar(128)    WITH DEFAULT ''           NOT NULL,
	site_address_c           varchar(128)    WITH DEFAULT ''           NOT NULL,
	site_city                varchar(128)    WITH DEFAULT ''           NOT NULL,
	site_state               varchar(64)     WITH DEFAULT ''           NOT NULL,
	site_country             varchar(64)     WITH DEFAULT ''           NOT NULL,
	site_zip                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	site_rack                varchar(128)    WITH DEFAULT ''           NOT NULL,
	site_notes               varchar(2048)   WITH DEFAULT ''           NOT NULL,
	poc_1_name               varchar(128)    WITH DEFAULT ''           NOT NULL,
	poc_1_email              varchar(128)    WITH DEFAULT ''           NOT NULL,
	poc_1_phone_a            varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_1_phone_b            varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_1_cell               varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_1_screen             varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_1_notes              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	poc_2_name               varchar(128)    WITH DEFAULT ''           NOT NULL,
	poc_2_email              varchar(128)    WITH DEFAULT ''           NOT NULL,
	poc_2_phone_a            varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_2_phone_b            varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_2_cell               varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_2_screen             varchar(64)     WITH DEFAULT ''           NOT NULL,
	poc_2_notes              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE TABLE housekeeper (
	housekeeperid            bigint                                    NOT NULL,
	tablename                varchar(64)     WITH DEFAULT ''           NOT NULL,
	field                    varchar(64)     WITH DEFAULT ''           NOT NULL,
	value                    bigint                                    NOT NULL,
	PRIMARY KEY (housekeeperid)
);
CREATE TABLE images (
	imageid                  bigint                                    NOT NULL,
	imagetype                integer         WITH DEFAULT '0'          NOT NULL,
	name                     varchar(64)     WITH DEFAULT '0'          NOT NULL,
	image                    blob                                      NOT NULL,
	PRIMARY KEY (imageid)
);
CREATE UNIQUE INDEX images_1 ON images (name);
CREATE TABLE item_discovery (
	itemdiscoveryid          bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	parent_itemid            bigint                                    NOT NULL,
	key_                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	lastcheck                integer         WITH DEFAULT '0'          NOT NULL,
	ts_delete                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
);
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);
CREATE INDEX item_discovery_2 ON item_discovery (parent_itemid);
CREATE TABLE host_discovery (
	hostid                   bigint                                    NOT NULL,
	parent_hostid            bigint                                    NULL,
	parent_itemid            bigint                                    NULL,
	host                     varchar(64)     WITH DEFAULT ''           NOT NULL,
	lastcheck                integer         WITH DEFAULT '0'          NOT NULL,
	ts_delete                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE TABLE interface_discovery (
	interfaceid              bigint                                    NOT NULL,
	parent_interfaceid       bigint                                    NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE TABLE profiles (
	profileid                bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	idx                      varchar(96)     WITH DEFAULT ''           NOT NULL,
	idx2                     bigint          WITH DEFAULT '0'          NOT NULL,
	value_id                 bigint          WITH DEFAULT '0'          NOT NULL,
	value_int                integer         WITH DEFAULT '0'          NOT NULL,
	value_str                varchar(255)    WITH DEFAULT ''           NOT NULL,
	source                   varchar(96)     WITH DEFAULT ''           NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (profileid)
);
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);
CREATE INDEX profiles_2 ON profiles (userid,profileid);
CREATE TABLE sessions (
	sessionid                varchar(32)     WITH DEFAULT ''           NOT NULL,
	userid                   bigint                                    NOT NULL,
	lastaccess               integer         WITH DEFAULT '0'          NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (sessionid)
);
CREATE INDEX sessions_1 ON sessions (userid,status,lastaccess);
CREATE TABLE trigger_discovery (
	triggerid                bigint                                    NOT NULL,
	parent_triggerid         bigint                                    NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX trigger_discovery_1 ON trigger_discovery (parent_triggerid);
CREATE TABLE application_template (
	application_templateid   bigint                                    NOT NULL,
	applicationid            bigint                                    NOT NULL,
	templateid               bigint                                    NOT NULL,
	PRIMARY KEY (application_templateid)
);
CREATE UNIQUE INDEX application_template_1 ON application_template (applicationid,templateid);
CREATE INDEX application_template_2 ON application_template (templateid);
CREATE TABLE item_condition (
	item_conditionid         bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	operator                 integer         WITH DEFAULT '8'          NOT NULL,
	macro                    varchar(64)     WITH DEFAULT ''           NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (item_conditionid)
);
CREATE INDEX item_condition_1 ON item_condition (itemid);
CREATE TABLE application_prototype (
	application_prototypeid  bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	templateid               bigint                                    NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (application_prototypeid)
);
CREATE INDEX application_prototype_1 ON application_prototype (itemid);
CREATE INDEX application_prototype_2 ON application_prototype (templateid);
CREATE TABLE item_application_prototype (
	item_application_prototypeid bigint                                    NOT NULL,
	application_prototypeid  bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	PRIMARY KEY (item_application_prototypeid)
);
CREATE UNIQUE INDEX item_application_prototype_1 ON item_application_prototype (application_prototypeid,itemid);
CREATE INDEX item_application_prototype_2 ON item_application_prototype (itemid);
CREATE TABLE application_discovery (
	application_discoveryid  bigint                                    NOT NULL,
	applicationid            bigint                                    NOT NULL,
	application_prototypeid  bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	lastcheck                integer         WITH DEFAULT '0'          NOT NULL,
	ts_delete                integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (application_discoveryid)
);
CREATE INDEX application_discovery_1 ON application_discovery (applicationid);
CREATE INDEX application_discovery_2 ON application_discovery (application_prototypeid);
CREATE TABLE opinventory (
	operationid              bigint                                    NOT NULL,
	inventory_mode           integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE TABLE trigger_tag (
	triggertagid             bigint                                    NOT NULL,
	triggerid                bigint                                    NOT NULL,
	tag                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (triggertagid)
);
CREATE INDEX trigger_tag_1 ON trigger_tag (triggerid);
CREATE TABLE event_tag (
	eventtagid               bigint                                    NOT NULL,
	eventid                  bigint                                    NOT NULL,
	tag                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (eventtagid)
);
CREATE INDEX event_tag_1 ON event_tag (eventid);
CREATE TABLE problem (
	eventid                  bigint                                    NOT NULL,
	source                   integer         WITH DEFAULT '0'          NOT NULL,
	object                   integer         WITH DEFAULT '0'          NOT NULL,
	objectid                 bigint          WITH DEFAULT '0'          NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	ns                       integer         WITH DEFAULT '0'          NOT NULL,
	r_eventid                bigint                                    NULL,
	r_clock                  integer         WITH DEFAULT '0'          NOT NULL,
	r_ns                     integer         WITH DEFAULT '0'          NOT NULL,
	correlationid            bigint                                    NULL,
	userid                   bigint                                    NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX problem_1 ON problem (source,object,objectid);
CREATE INDEX problem_2 ON problem (r_clock);
CREATE TABLE problem_tag (
	problemtagid             bigint                                    NOT NULL,
	eventid                  bigint                                    NOT NULL,
	tag                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (problemtagid)
);
CREATE INDEX problem_tag_1 ON problem_tag (eventid);
CREATE INDEX problem_tag_2 ON problem_tag (tag,value);
CREATE TABLE event_recovery (
	eventid                  bigint                                    NOT NULL,
	r_eventid                bigint                                    NOT NULL,
	c_eventid                bigint                                    NULL,
	correlationid            bigint                                    NULL,
	userid                   bigint                                    NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX event_recovery_1 ON event_recovery (r_eventid);
CREATE INDEX event_recovery_2 ON event_recovery (c_eventid);
CREATE TABLE correlation (
	correlationid            bigint                                    NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	description              varchar(2048)   WITH DEFAULT ''           NOT NULL,
	evaltype                 integer         WITH DEFAULT '0'          NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	formula                  varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (correlationid)
);
CREATE INDEX correlation_1 ON correlation (status);
CREATE UNIQUE INDEX correlation_2 ON correlation (name);
CREATE TABLE corr_condition (
	corr_conditionid         bigint                                    NOT NULL,
	correlationid            bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE INDEX corr_condition_1 ON corr_condition (correlationid);
CREATE TABLE corr_condition_tag (
	corr_conditionid         bigint                                    NOT NULL,
	tag                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE TABLE corr_condition_group (
	corr_conditionid         bigint                                    NOT NULL,
	operator                 integer         WITH DEFAULT '0'          NOT NULL,
	groupid                  bigint                                    NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE INDEX corr_condition_group_1 ON corr_condition_group (groupid);
CREATE TABLE corr_condition_tagpair (
	corr_conditionid         bigint                                    NOT NULL,
	oldtag                   varchar(255)    WITH DEFAULT ''           NOT NULL,
	newtag                   varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE TABLE corr_condition_tagvalue (
	corr_conditionid         bigint                                    NOT NULL,
	tag                      varchar(255)    WITH DEFAULT ''           NOT NULL,
	operator                 integer         WITH DEFAULT '0'          NOT NULL,
	value                    varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE TABLE corr_operation (
	corr_operationid         bigint                                    NOT NULL,
	correlationid            bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (corr_operationid)
);
CREATE INDEX corr_operation_1 ON corr_operation (correlationid);
CREATE TABLE task (
	taskid                   bigint                                    NOT NULL,
	type                     integer                                   NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	clock                    integer         WITH DEFAULT '0'          NOT NULL,
	ttl                      integer         WITH DEFAULT '0'          NOT NULL,
	proxy_hostid             bigint                                    NULL,
	PRIMARY KEY (taskid)
);
CREATE INDEX task_1 ON task (status,proxy_hostid);
CREATE TABLE task_close_problem (
	taskid                   bigint                                    NOT NULL,
	acknowledgeid            bigint                                    NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE item_preproc (
	item_preprocid           bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	step                     integer         WITH DEFAULT '0'          NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	params                   varchar(255)    WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (item_preprocid)
);
CREATE INDEX item_preproc_1 ON item_preproc (itemid,step);
CREATE TABLE task_remote_command (
	taskid                   bigint                                    NOT NULL,
	command_type             integer         WITH DEFAULT '0'          NOT NULL,
	execute_on               integer         WITH DEFAULT '0'          NOT NULL,
	port                     integer         WITH DEFAULT '0'          NOT NULL,
	authtype                 integer         WITH DEFAULT '0'          NOT NULL,
	username                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	password                 varchar(64)     WITH DEFAULT ''           NOT NULL,
	publickey                varchar(64)     WITH DEFAULT ''           NOT NULL,
	privatekey               varchar(64)     WITH DEFAULT ''           NOT NULL,
	command                  varchar(2048)   WITH DEFAULT ''           NOT NULL,
	alertid                  bigint                                    NULL,
	parent_taskid            bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE task_remote_command_result (
	taskid                   bigint                                    NOT NULL,
	status                   integer         WITH DEFAULT '0'          NOT NULL,
	parent_taskid            bigint                                    NOT NULL,
	info                     varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE task_acknowledge (
	taskid                   bigint                                    NOT NULL,
	acknowledgeid            bigint                                    NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE sysmap_shape (
	sysmap_shapeid           bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	x                        integer         WITH DEFAULT '0'          NOT NULL,
	y                        integer         WITH DEFAULT '0'          NOT NULL,
	width                    integer         WITH DEFAULT '200'        NOT NULL,
	height                   integer         WITH DEFAULT '200'        NOT NULL,
	text                     varchar(2048)   WITH DEFAULT ''           NOT NULL,
	font                     integer         WITH DEFAULT '9'          NOT NULL,
	font_size                integer         WITH DEFAULT '11'         NOT NULL,
	font_color               varchar(6)      WITH DEFAULT '000000'     NOT NULL,
	text_halign              integer         WITH DEFAULT '0'          NOT NULL,
	text_valign              integer         WITH DEFAULT '0'          NOT NULL,
	border_type              integer         WITH DEFAULT '0'          NOT NULL,
	border_width             integer         WITH DEFAULT '1'          NOT NULL,
	border_color             varchar(6)      WITH DEFAULT '000000'     NOT NULL,
	background_color         varchar(6)      WITH DEFAULT ''           NOT NULL,
	zindex                   integer         WITH DEFAULT '0'          NOT NULL,
	PRIMARY KEY (sysmap_shapeid)
);
CREATE INDEX sysmap_shape_1 ON sysmap_shape (sysmapid);
CREATE TABLE sysmap_element_trigger (
	selement_triggerid       bigint                                    NOT NULL,
	selementid               bigint                                    NOT NULL,
	triggerid                bigint                                    NOT NULL,
	PRIMARY KEY (selement_triggerid)
);
CREATE UNIQUE INDEX sysmap_element_trigger_1 ON sysmap_element_trigger (selementid,triggerid);
CREATE TABLE httptest_field (
	httptest_fieldid         bigint                                    NOT NULL,
	httptestid               bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (httptest_fieldid)
);
CREATE INDEX httptest_field_1 ON httptest_field (httptestid);
CREATE TABLE httpstep_field (
	httpstep_fieldid         bigint                                    NOT NULL,
	httpstepid               bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	value                    varchar(2048)   WITH DEFAULT ''           NOT NULL,
	PRIMARY KEY (httpstep_fieldid)
);
CREATE INDEX httpstep_field_1 ON httpstep_field (httpstepid);
CREATE TABLE dashboard (
	dashboardid              bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	userid                   bigint                                    NOT NULL,
	private                  integer         WITH DEFAULT '1'          NOT NULL,
	PRIMARY KEY (dashboardid)
);
CREATE TABLE dashboard_user (
	dashboard_userid         bigint                                    NOT NULL,
	dashboardid              bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (dashboard_userid)
);
CREATE UNIQUE INDEX dashboard_user_1 ON dashboard_user (dashboardid,userid);
CREATE TABLE dashboard_usrgrp (
	dashboard_usrgrpid       bigint                                    NOT NULL,
	dashboardid              bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL,
	permission               integer         WITH DEFAULT '2'          NOT NULL,
	PRIMARY KEY (dashboard_usrgrpid)
);
CREATE UNIQUE INDEX dashboard_usrgrp_1 ON dashboard_usrgrp (dashboardid,usrgrpid);
CREATE TABLE widget (
	widgetid                 bigint                                    NOT NULL,
	dashboardid              bigint                                    NOT NULL,
	type                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	x                        integer         WITH DEFAULT '0'          NOT NULL,
	y                        integer         WITH DEFAULT '0'          NOT NULL,
	width                    integer         WITH DEFAULT '1'          NOT NULL,
	height                   integer         WITH DEFAULT '1'          NOT NULL,
	PRIMARY KEY (widgetid)
);
CREATE INDEX widget_1 ON widget (dashboardid);
CREATE TABLE widget_field (
	widget_fieldid           bigint                                    NOT NULL,
	widgetid                 bigint                                    NOT NULL,
	type                     integer         WITH DEFAULT '0'          NOT NULL,
	name                     varchar(255)    WITH DEFAULT ''           NOT NULL,
	value_int                integer         WITH DEFAULT '0'          NOT NULL,
	value_str                varchar(255)    WITH DEFAULT ''           NOT NULL,
	value_groupid            bigint                                    NULL,
	value_hostid             bigint                                    NULL,
	value_itemid             bigint                                    NULL,
	value_graphid            bigint                                    NULL,
	value_sysmapid           bigint                                    NULL,
	PRIMARY KEY (widget_fieldid)
);
CREATE INDEX widget_field_1 ON widget_field (widgetid);
CREATE INDEX widget_field_2 ON widget_field (value_groupid);
CREATE INDEX widget_field_3 ON widget_field (value_hostid);
CREATE INDEX widget_field_4 ON widget_field (value_itemid);
CREATE INDEX widget_field_5 ON widget_field (value_graphid);
CREATE INDEX widget_field_6 ON widget_field (value_sysmapid);
CREATE TABLE dbversion (
	mandatory                integer         WITH DEFAULT '0'          NOT NULL,
	optional                 integer         WITH DEFAULT '0'          NOT NULL
);
INSERT INTO dbversion VALUES ('3040000','3040005');
ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_3 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_3 FOREIGN KEY (templateid) REFERENCES group_prototype (group_prototypeid) ON DELETE CASCADE;
ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_1 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_2 FOREIGN KEY (parent_group_prototypeid) REFERENCES group_prototype (group_prototypeid);
ALTER TABLE screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE screens ADD CONSTRAINT c_screens_3 FOREIGN KEY (userid) REFERENCES users (userid);
ALTER TABLE screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE screen_user ADD CONSTRAINT c_screen_user_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE screen_user ADD CONSTRAINT c_screen_user_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE screen_usrgrp ADD CONSTRAINT c_screen_usrgrp_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE screen_usrgrp ADD CONSTRAINT c_screen_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE slideshows ADD CONSTRAINT c_slideshows_3 FOREIGN KEY (userid) REFERENCES users (userid);
ALTER TABLE slideshow_user ADD CONSTRAINT c_slideshow_user_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE slideshow_user ADD CONSTRAINT c_slideshow_user_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE slideshow_usrgrp ADD CONSTRAINT c_slideshow_usrgrp_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE slideshow_usrgrp ADD CONSTRAINT c_slideshow_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
ALTER TABLE applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid);
ALTER TABLE httptest ADD CONSTRAINT c_httptest_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE httptest ADD CONSTRAINT c_httptest_3 FOREIGN KEY (templateid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid);
ALTER TABLE items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid);
ALTER TABLE items ADD CONSTRAINT c_items_5 FOREIGN KEY (master_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE;
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE operations ADD CONSTRAINT c_operations_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage ADD CONSTRAINT c_opmessage_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid);
ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage_grp ADD CONSTRAINT c_opmessage_grp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opmessage_usr ADD CONSTRAINT c_opmessage_usr_2 FOREIGN KEY (userid) REFERENCES users (userid);
ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand ADD CONSTRAINT c_opcommand_2 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid);
ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand_hst ADD CONSTRAINT c_opcommand_hst_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid);
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid);
ALTER TABLE opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES groups (groupid);
ALTER TABLE triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid);
ALTER TABLE graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid);
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE items_applications ADD CONSTRAINT c_items_applications_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE mappings ADD CONSTRAINT c_mappings_1 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid) ON DELETE CASCADE;
ALTER TABLE media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE services ADD CONSTRAINT c_services_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_times ADD CONSTRAINT c_services_times_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE icon_map ADD CONSTRAINT c_icon_map_1 FOREIGN KEY (default_iconid) REFERENCES images (imageid);
ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_1 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid) ON DELETE CASCADE;
ALTER TABLE icon_mapping ADD CONSTRAINT c_icon_mapping_2 FOREIGN KEY (iconid) REFERENCES images (imageid);
ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_1 FOREIGN KEY (backgroundid) REFERENCES images (imageid);
ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_2 FOREIGN KEY (iconmapid) REFERENCES icon_map (iconmapid);
ALTER TABLE sysmaps ADD CONSTRAINT c_sysmaps_3 FOREIGN KEY (userid) REFERENCES users (userid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_2 FOREIGN KEY (iconid_off) REFERENCES images (imageid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_3 FOREIGN KEY (iconid_on) REFERENCES images (imageid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_4 FOREIGN KEY (iconid_disabled) REFERENCES images (imageid);
ALTER TABLE sysmaps_elements ADD CONSTRAINT c_sysmaps_elements_5 FOREIGN KEY (iconid_maintenance) REFERENCES images (imageid);
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_2 FOREIGN KEY (selementid1) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmaps_links ADD CONSTRAINT c_sysmaps_links_3 FOREIGN KEY (selementid2) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_1 FOREIGN KEY (linkid) REFERENCES sysmaps_links (linkid) ON DELETE CASCADE;
ALTER TABLE sysmaps_link_triggers ADD CONSTRAINT c_sysmaps_link_triggers_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE sysmap_element_url ADD CONSTRAINT c_sysmap_element_url_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmap_url ADD CONSTRAINT c_sysmap_url_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmap_user ADD CONSTRAINT c_sysmap_user_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmap_user ADD CONSTRAINT c_sysmap_user_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE sysmap_usrgrp ADD CONSTRAINT c_sysmap_usrgrp_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmap_usrgrp ADD CONSTRAINT c_sysmap_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE;
ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_5 FOREIGN KEY (p_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_6 FOREIGN KEY (acknowledgeid) REFERENCES acknowledges (acknowledgeid) ON DELETE CASCADE;
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE auditlog ADD CONSTRAINT c_auditlog_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE auditlog_details ADD CONSTRAINT c_auditlog_details_1 FOREIGN KEY (auditid) REFERENCES auditlog (auditid) ON DELETE CASCADE;
ALTER TABLE service_alarms ADD CONSTRAINT c_service_alarms_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE autoreg_host ADD CONSTRAINT c_autoreg_host_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE dhosts ADD CONSTRAINT c_dhosts_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
ALTER TABLE dservices ADD CONSTRAINT c_dservices_1 FOREIGN KEY (dhostid) REFERENCES dhosts (dhostid) ON DELETE CASCADE;
ALTER TABLE dservices ADD CONSTRAINT c_dservices_2 FOREIGN KEY (dcheckid) REFERENCES dchecks (dcheckid) ON DELETE CASCADE;
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid);
ALTER TABLE host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_2 FOREIGN KEY (parent_hostid) REFERENCES hosts (hostid);
ALTER TABLE host_discovery ADD CONSTRAINT c_host_discovery_3 FOREIGN KEY (parent_itemid) REFERENCES items (itemid);
ALTER TABLE interface_discovery ADD CONSTRAINT c_interface_discovery_1 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE;
ALTER TABLE interface_discovery ADD CONSTRAINT c_interface_discovery_2 FOREIGN KEY (parent_interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE;
ALTER TABLE profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid);
ALTER TABLE application_template ADD CONSTRAINT c_application_template_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE application_template ADD CONSTRAINT c_application_template_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE item_condition ADD CONSTRAINT c_item_condition_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE application_prototype ADD CONSTRAINT c_application_prototype_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE application_prototype ADD CONSTRAINT c_application_prototype_2 FOREIGN KEY (templateid) REFERENCES application_prototype (application_prototypeid) ON DELETE CASCADE;
ALTER TABLE item_application_prototype ADD CONSTRAINT c_item_application_prototype_1 FOREIGN KEY (application_prototypeid) REFERENCES application_prototype (application_prototypeid) ON DELETE CASCADE;
ALTER TABLE item_application_prototype ADD CONSTRAINT c_item_application_prototype_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE application_discovery ADD CONSTRAINT c_application_discovery_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE application_discovery ADD CONSTRAINT c_application_discovery_2 FOREIGN KEY (application_prototypeid) REFERENCES application_prototype (application_prototypeid) ON DELETE CASCADE;
ALTER TABLE opinventory ADD CONSTRAINT c_opinventory_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE trigger_tag ADD CONSTRAINT c_trigger_tag_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE event_tag ADD CONSTRAINT c_event_tag_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE problem ADD CONSTRAINT c_problem_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE problem ADD CONSTRAINT c_problem_2 FOREIGN KEY (r_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE problem_tag ADD CONSTRAINT c_problem_tag_1 FOREIGN KEY (eventid) REFERENCES problem (eventid) ON DELETE CASCADE;
ALTER TABLE event_recovery ADD CONSTRAINT c_event_recovery_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE event_recovery ADD CONSTRAINT c_event_recovery_2 FOREIGN KEY (r_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE event_recovery ADD CONSTRAINT c_event_recovery_3 FOREIGN KEY (c_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE corr_condition ADD CONSTRAINT c_corr_condition_1 FOREIGN KEY (correlationid) REFERENCES correlation (correlationid) ON DELETE CASCADE;
ALTER TABLE corr_condition_tag ADD CONSTRAINT c_corr_condition_tag_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_condition_group ADD CONSTRAINT c_corr_condition_group_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_condition_group ADD CONSTRAINT c_corr_condition_group_2 FOREIGN KEY (groupid) REFERENCES groups (groupid);
ALTER TABLE corr_condition_tagpair ADD CONSTRAINT c_corr_condition_tagpair_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_condition_tagvalue ADD CONSTRAINT c_corr_condition_tagvalue_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_operation ADD CONSTRAINT c_corr_operation_1 FOREIGN KEY (correlationid) REFERENCES correlation (correlationid) ON DELETE CASCADE;
ALTER TABLE task ADD CONSTRAINT c_task_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE task_close_problem ADD CONSTRAINT c_task_close_problem_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE item_preproc ADD CONSTRAINT c_item_preproc_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE task_remote_command ADD CONSTRAINT c_task_remote_command_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE task_remote_command_result ADD CONSTRAINT c_task_remote_command_result_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE task_acknowledge ADD CONSTRAINT c_task_acknowledge_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE sysmap_shape ADD CONSTRAINT c_sysmap_shape_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmap_element_trigger ADD CONSTRAINT c_sysmap_element_trigger_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmap_element_trigger ADD CONSTRAINT c_sysmap_element_trigger_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE httptest_field ADD CONSTRAINT c_httptest_field_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE httpstep_field ADD CONSTRAINT c_httpstep_field_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid) ON DELETE CASCADE;
ALTER TABLE dashboard ADD CONSTRAINT c_dashboard_1 FOREIGN KEY (userid) REFERENCES users (userid);
ALTER TABLE dashboard_user ADD CONSTRAINT c_dashboard_user_1 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE dashboard_user ADD CONSTRAINT c_dashboard_user_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE dashboard_usrgrp ADD CONSTRAINT c_dashboard_usrgrp_1 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE dashboard_usrgrp ADD CONSTRAINT c_dashboard_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE widget ADD CONSTRAINT c_widget_1 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_1 FOREIGN KEY (widgetid) REFERENCES widget (widgetid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_2 FOREIGN KEY (value_groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_3 FOREIGN KEY (value_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_4 FOREIGN KEY (value_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_5 FOREIGN KEY (value_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_6 FOREIGN KEY (value_sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
