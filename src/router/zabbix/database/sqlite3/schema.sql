CREATE TABLE maintenances (
	maintenanceid            bigint                                    NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	maintenance_type         integer         DEFAULT '0'               NOT NULL,
	description              text            DEFAULT ''                NOT NULL,
	active_since             integer         DEFAULT '0'               NOT NULL,
	active_till              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (maintenanceid)
);
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);
CREATE TABLE hosts (
	hostid                   bigint                                    NOT NULL,
	proxy_hostid             bigint                                    NULL REFERENCES hosts (hostid),
	host                     varchar(64)     DEFAULT ''                NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	disable_until            integer         DEFAULT '0'               NOT NULL,
	error                    varchar(128)    DEFAULT ''                NOT NULL,
	available                integer         DEFAULT '0'               NOT NULL,
	errors_from              integer         DEFAULT '0'               NOT NULL,
	lastaccess               integer         DEFAULT '0'               NOT NULL,
	ipmi_authtype            integer         DEFAULT '0'               NOT NULL,
	ipmi_privilege           integer         DEFAULT '2'               NOT NULL,
	ipmi_username            varchar(16)     DEFAULT ''                NOT NULL,
	ipmi_password            varchar(20)     DEFAULT ''                NOT NULL,
	ipmi_disable_until       integer         DEFAULT '0'               NOT NULL,
	ipmi_available           integer         DEFAULT '0'               NOT NULL,
	snmp_disable_until       integer         DEFAULT '0'               NOT NULL,
	snmp_available           integer         DEFAULT '0'               NOT NULL,
	maintenanceid            bigint                                    NULL REFERENCES maintenances (maintenanceid),
	maintenance_status       integer         DEFAULT '0'               NOT NULL,
	maintenance_type         integer         DEFAULT '0'               NOT NULL,
	maintenance_from         integer         DEFAULT '0'               NOT NULL,
	ipmi_errors_from         integer         DEFAULT '0'               NOT NULL,
	snmp_errors_from         integer         DEFAULT '0'               NOT NULL,
	ipmi_error               varchar(128)    DEFAULT ''                NOT NULL,
	snmp_error               varchar(128)    DEFAULT ''                NOT NULL,
	jmx_disable_until        integer         DEFAULT '0'               NOT NULL,
	jmx_available            integer         DEFAULT '0'               NOT NULL,
	jmx_errors_from          integer         DEFAULT '0'               NOT NULL,
	jmx_error                varchar(128)    DEFAULT ''                NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE INDEX hosts_1 ON hosts (host);
CREATE INDEX hosts_2 ON hosts (status);
CREATE INDEX hosts_3 ON hosts (proxy_hostid);
CREATE INDEX hosts_4 ON hosts (name);
CREATE TABLE groups (
	groupid                  bigint                                    NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	internal                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE INDEX groups_1 ON groups (name);
CREATE TABLE screens (
	screenid                 bigint                                    NOT NULL,
	name                     varchar(255)                              NOT NULL,
	hsize                    integer         DEFAULT '1'               NOT NULL,
	vsize                    integer         DEFAULT '1'               NOT NULL,
	templateid               bigint                                    NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	PRIMARY KEY (screenid)
);
CREATE TABLE screens_items (
	screenitemid             bigint                                    NOT NULL,
	screenid                 bigint                                    NOT NULL REFERENCES screens (screenid) ON DELETE CASCADE,
	resourcetype             integer         DEFAULT '0'               NOT NULL,
	resourceid               bigint          DEFAULT '0'               NOT NULL,
	width                    integer         DEFAULT '320'             NOT NULL,
	height                   integer         DEFAULT '200'             NOT NULL,
	x                        integer         DEFAULT '0'               NOT NULL,
	y                        integer         DEFAULT '0'               NOT NULL,
	colspan                  integer         DEFAULT '0'               NOT NULL,
	rowspan                  integer         DEFAULT '0'               NOT NULL,
	elements                 integer         DEFAULT '25'              NOT NULL,
	valign                   integer         DEFAULT '0'               NOT NULL,
	halign                   integer         DEFAULT '0'               NOT NULL,
	style                    integer         DEFAULT '0'               NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	dynamic                  integer         DEFAULT '0'               NOT NULL,
	sort_triggers            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (screenitemid)
);
CREATE TABLE slideshows (
	slideshowid              bigint                                    NOT NULL,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	delay                    integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (slideshowid)
);
CREATE TABLE slides (
	slideid                  bigint                                    NOT NULL,
	slideshowid              bigint                                    NOT NULL REFERENCES slideshows (slideshowid) ON DELETE CASCADE,
	screenid                 bigint                                    NOT NULL REFERENCES screens (screenid) ON DELETE CASCADE,
	step                     integer         DEFAULT '0'               NOT NULL,
	delay                    integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (slideid)
);
CREATE INDEX slides_slides_1 ON slides (slideshowid);
CREATE TABLE drules (
	druleid                  bigint                                    NOT NULL,
	proxy_hostid             bigint                                    NULL REFERENCES hosts (hostid),
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	iprange                  varchar(255)    DEFAULT ''                NOT NULL,
	delay                    integer         DEFAULT '3600'            NOT NULL,
	nextcheck                integer         DEFAULT '0'               NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (druleid)
);
CREATE TABLE dchecks (
	dcheckid                 bigint                                    NOT NULL,
	druleid                  bigint                                    NOT NULL REFERENCES drules (druleid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	snmp_community           varchar(255)    DEFAULT ''                NOT NULL,
	ports                    varchar(255)    DEFAULT '0'               NOT NULL,
	snmpv3_securityname      varchar(64)     DEFAULT ''                NOT NULL,
	snmpv3_securitylevel     integer         DEFAULT '0'               NOT NULL,
	snmpv3_authpassphrase    varchar(64)     DEFAULT ''                NOT NULL,
	snmpv3_privpassphrase    varchar(64)     DEFAULT ''                NOT NULL,
	uniq                     integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dcheckid)
);
CREATE INDEX dchecks_1 ON dchecks (druleid);
CREATE TABLE applications (
	applicationid            bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	templateid               bigint                                    NULL REFERENCES applications (applicationid) ON DELETE CASCADE,
	PRIMARY KEY (applicationid)
);
CREATE INDEX applications_1 ON applications (templateid);
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);
CREATE TABLE httptest (
	httptestid               bigint                                    NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	applicationid            bigint                                    NOT NULL REFERENCES applications (applicationid) ON DELETE CASCADE,
	nextcheck                integer         DEFAULT '0'               NOT NULL,
	delay                    integer         DEFAULT '60'              NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	macros                   text            DEFAULT ''                NOT NULL,
	agent                    varchar(255)    DEFAULT ''                NOT NULL,
	authentication           integer         DEFAULT '0'               NOT NULL,
	http_user                varchar(64)     DEFAULT ''                NOT NULL,
	http_password            varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (httptestid)
);
CREATE INDEX httptest_httptest_1 ON httptest (applicationid);
CREATE INDEX httptest_2 ON httptest (name);
CREATE INDEX httptest_3 ON httptest (status);
CREATE TABLE httpstep (
	httpstepid               bigint                                    NOT NULL,
	httptestid               bigint                                    NOT NULL REFERENCES httptest (httptestid) ON DELETE CASCADE,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	no                       integer         DEFAULT '0'               NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	timeout                  integer         DEFAULT '30'              NOT NULL,
	posts                    text            DEFAULT ''                NOT NULL,
	required                 varchar(255)    DEFAULT ''                NOT NULL,
	status_codes             varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (httpstepid)
);
CREATE INDEX httpstep_httpstep_1 ON httpstep (httptestid);
CREATE TABLE interface (
	interfaceid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	main                     integer         DEFAULT '0'               NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	useip                    integer         DEFAULT '1'               NOT NULL,
	ip                       varchar(39)     DEFAULT '127.0.0.1'       NOT NULL,
	dns                      varchar(64)     DEFAULT ''                NOT NULL,
	port                     varchar(64)     DEFAULT '10050'           NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_1 ON interface (hostid,type);
CREATE INDEX interface_2 ON interface (ip,dns);
CREATE TABLE valuemaps (
	valuemapid               bigint                                    NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (valuemapid)
);
CREATE INDEX valuemaps_1 ON valuemaps (name);
CREATE TABLE items (
	itemid                   bigint                                    NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	snmp_community           varchar(64)     DEFAULT ''                NOT NULL,
	snmp_oid                 varchar(255)    DEFAULT ''                NOT NULL,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	delay                    integer         DEFAULT '0'               NOT NULL,
	history                  integer         DEFAULT '90'              NOT NULL,
	trends                   integer         DEFAULT '365'             NOT NULL,
	lastvalue                varchar(255)                              NULL,
	lastclock                integer                                   NULL,
	prevvalue                varchar(255)                              NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	value_type               integer         DEFAULT '0'               NOT NULL,
	trapper_hosts            varchar(255)    DEFAULT ''                NOT NULL,
	units                    varchar(255)    DEFAULT ''                NOT NULL,
	multiplier               integer         DEFAULT '0'               NOT NULL,
	delta                    integer         DEFAULT '0'               NOT NULL,
	prevorgvalue             varchar(255)                              NULL,
	snmpv3_securityname      varchar(64)     DEFAULT ''                NOT NULL,
	snmpv3_securitylevel     integer         DEFAULT '0'               NOT NULL,
	snmpv3_authpassphrase    varchar(64)     DEFAULT ''                NOT NULL,
	snmpv3_privpassphrase    varchar(64)     DEFAULT ''                NOT NULL,
	formula                  varchar(255)    DEFAULT '1'               NOT NULL,
	error                    varchar(128)    DEFAULT ''                NOT NULL,
	lastlogsize              bigint          DEFAULT '0'               NOT NULL,
	logtimefmt               varchar(64)     DEFAULT ''                NOT NULL,
	templateid               bigint                                    NULL REFERENCES items (itemid) ON DELETE CASCADE,
	valuemapid               bigint                                    NULL REFERENCES valuemaps (valuemapid),
	delay_flex               varchar(255)    DEFAULT ''                NOT NULL,
	params                   text            DEFAULT ''                NOT NULL,
	ipmi_sensor              varchar(128)    DEFAULT ''                NOT NULL,
	data_type                integer         DEFAULT '0'               NOT NULL,
	authtype                 integer         DEFAULT '0'               NOT NULL,
	username                 varchar(64)     DEFAULT ''                NOT NULL,
	password                 varchar(64)     DEFAULT ''                NOT NULL,
	publickey                varchar(64)     DEFAULT ''                NOT NULL,
	privatekey               varchar(64)     DEFAULT ''                NOT NULL,
	mtime                    integer         DEFAULT '0'               NOT NULL,
	lastns                   integer                                   NULL,
	flags                    integer         DEFAULT '0'               NOT NULL,
	filter                   varchar(255)    DEFAULT ''                NOT NULL,
	interfaceid              bigint                                    NULL REFERENCES interface (interfaceid),
	port                     varchar(64)     DEFAULT ''                NOT NULL,
	description              text            DEFAULT ''                NOT NULL,
	inventory_link           integer         DEFAULT '0'               NOT NULL,
	lifetime                 varchar(64)     DEFAULT '30'              NOT NULL,
	PRIMARY KEY (itemid)
);
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);
CREATE INDEX items_3 ON items (status);
CREATE INDEX items_4 ON items (templateid);
CREATE INDEX items_5 ON items (valuemapid);
CREATE TABLE httpstepitem (
	httpstepitemid           bigint                                    NOT NULL,
	httpstepid               bigint                                    NOT NULL REFERENCES httpstep (httpstepid) ON DELETE CASCADE,
	itemid                   bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httpstepitemid)
);
CREATE UNIQUE INDEX httpstepitem_httpstepitem_1 ON httpstepitem (httpstepid,itemid);
CREATE TABLE httptestitem (
	httptestitemid           bigint                                    NOT NULL,
	httptestid               bigint                                    NOT NULL REFERENCES httptest (httptestid) ON DELETE CASCADE,
	itemid                   bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httptestitemid)
);
CREATE UNIQUE INDEX httptestitem_httptestitem_1 ON httptestitem (httptestid,itemid);
CREATE TABLE media_type (
	mediatypeid              bigint                                    NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	description              varchar(100)    DEFAULT ''                NOT NULL,
	smtp_server              varchar(255)    DEFAULT ''                NOT NULL,
	smtp_helo                varchar(255)    DEFAULT ''                NOT NULL,
	smtp_email               varchar(255)    DEFAULT ''                NOT NULL,
	exec_path                varchar(255)    DEFAULT ''                NOT NULL,
	gsm_modem                varchar(255)    DEFAULT ''                NOT NULL,
	username                 varchar(255)    DEFAULT ''                NOT NULL,
	passwd                   varchar(255)    DEFAULT ''                NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (mediatypeid)
);
CREATE TABLE users (
	userid                   bigint                                    NOT NULL,
	alias                    varchar(100)    DEFAULT ''                NOT NULL,
	name                     varchar(100)    DEFAULT ''                NOT NULL,
	surname                  varchar(100)    DEFAULT ''                NOT NULL,
	passwd                   char(32)        DEFAULT ''                NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	autologin                integer         DEFAULT '0'               NOT NULL,
	autologout               integer         DEFAULT '900'             NOT NULL,
	lang                     varchar(5)      DEFAULT 'en_GB'           NOT NULL,
	refresh                  integer         DEFAULT '30'              NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	theme                    varchar(128)    DEFAULT 'default'         NOT NULL,
	attempt_failed           integer         DEFAULT 0                 NOT NULL,
	attempt_ip               varchar(39)     DEFAULT ''                NOT NULL,
	attempt_clock            integer         DEFAULT 0                 NOT NULL,
	rows_per_page            integer         DEFAULT 50                NOT NULL,
	PRIMARY KEY (userid)
);
CREATE INDEX users_1 ON users (alias);
CREATE TABLE usrgrp (
	usrgrpid                 bigint                                    NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	gui_access               integer         DEFAULT '0'               NOT NULL,
	users_status             integer         DEFAULT '0'               NOT NULL,
	debug_mode               integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (usrgrpid)
);
CREATE INDEX usrgrp_1 ON usrgrp (name);
CREATE TABLE users_groups (
	id                       bigint                                    NOT NULL,
	usrgrpid                 bigint                                    NOT NULL REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	PRIMARY KEY (id)
);
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);
CREATE TABLE scripts (
	scriptid                 bigint                                    NOT NULL,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	command                  varchar(255)    DEFAULT ''                NOT NULL,
	host_access              integer         DEFAULT '2'               NOT NULL,
	usrgrpid                 bigint                                    NULL REFERENCES usrgrp (usrgrpid),
	groupid                  bigint                                    NULL REFERENCES groups (groupid),
	description              text            DEFAULT ''                NOT NULL,
	confirmation             varchar(255)    DEFAULT ''                NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	execute_on               integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (scriptid)
);
CREATE TABLE actions (
	actionid                 bigint                                    NOT NULL,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	eventsource              integer         DEFAULT '0'               NOT NULL,
	evaltype                 integer         DEFAULT '0'               NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	esc_period               integer         DEFAULT '0'               NOT NULL,
	def_shortdata            varchar(255)    DEFAULT ''                NOT NULL,
	def_longdata             text            DEFAULT ''                NOT NULL,
	recovery_msg             integer         DEFAULT '0'               NOT NULL,
	r_shortdata              varchar(255)    DEFAULT ''                NOT NULL,
	r_longdata               text            DEFAULT ''                NOT NULL,
	PRIMARY KEY (actionid)
);
CREATE INDEX actions_1 ON actions (eventsource,status);
CREATE TABLE operations (
	operationid              bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL REFERENCES actions (actionid) ON DELETE CASCADE,
	operationtype            integer         DEFAULT '0'               NOT NULL,
	esc_period               integer         DEFAULT '0'               NOT NULL,
	esc_step_from            integer         DEFAULT '1'               NOT NULL,
	esc_step_to              integer         DEFAULT '1'               NOT NULL,
	evaltype                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE INDEX operations_1 ON operations (actionid);
CREATE TABLE opmessage (
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	default_msg              integer         DEFAULT '0'               NOT NULL,
	subject                  varchar(255)    DEFAULT ''                NOT NULL,
	message                  text            DEFAULT ''                NOT NULL,
	mediatypeid              bigint                                    NULL REFERENCES media_type (mediatypeid),
	PRIMARY KEY (operationid)
);
CREATE TABLE opmessage_grp (
	opmessage_grpid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	usrgrpid                 bigint                                    NOT NULL REFERENCES usrgrp (usrgrpid),
	PRIMARY KEY (opmessage_grpid)
);
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
CREATE TABLE opmessage_usr (
	opmessage_usrid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	userid                   bigint                                    NOT NULL REFERENCES users (userid),
	PRIMARY KEY (opmessage_usrid)
);
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
CREATE TABLE opcommand (
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	scriptid                 bigint                                    NULL REFERENCES scripts (scriptid),
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
CREATE TABLE opcommand_hst (
	opcommand_hstid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	hostid                   bigint                                    NULL REFERENCES hosts (hostid),
	PRIMARY KEY (opcommand_hstid)
);
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
CREATE TABLE opcommand_grp (
	opcommand_grpid          bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	groupid                  bigint                                    NOT NULL REFERENCES groups (groupid),
	PRIMARY KEY (opcommand_grpid)
);
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
CREATE TABLE opgroup (
	opgroupid                bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	groupid                  bigint                                    NOT NULL REFERENCES groups (groupid),
	PRIMARY KEY (opgroupid)
);
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
CREATE TABLE optemplate (
	optemplateid             bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	templateid               bigint                                    NOT NULL REFERENCES hosts (hostid),
	PRIMARY KEY (optemplateid)
);
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
CREATE TABLE opconditions (
	opconditionid            bigint                                    NOT NULL,
	operationid              bigint                                    NOT NULL REFERENCES operations (operationid) ON DELETE CASCADE,
	conditiontype            integer         DEFAULT '0'               NOT NULL,
	operator                 integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (opconditionid)
);
CREATE INDEX opconditions_1 ON opconditions (operationid);
CREATE TABLE conditions (
	conditionid              bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL REFERENCES actions (actionid) ON DELETE CASCADE,
	conditiontype            integer         DEFAULT '0'               NOT NULL,
	operator                 integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (conditionid)
);
CREATE INDEX conditions_1 ON conditions (actionid);
CREATE TABLE config (
	configid                 bigint                                    NOT NULL,
	alert_history            integer         DEFAULT '0'               NOT NULL,
	event_history            integer         DEFAULT '0'               NOT NULL,
	refresh_unsupported      integer         DEFAULT '0'               NOT NULL,
	work_period              varchar(100)    DEFAULT '1-5,00:00-24:00' NOT NULL,
	alert_usrgrpid           bigint                                    NULL REFERENCES usrgrp (usrgrpid),
	event_ack_enable         integer         DEFAULT '1'               NOT NULL,
	event_expire             integer         DEFAULT '7'               NOT NULL,
	event_show_max           integer         DEFAULT '100'             NOT NULL,
	default_theme            varchar(128)    DEFAULT 'originalblue'    NOT NULL,
	authentication_type      integer         DEFAULT '0'               NOT NULL,
	ldap_host                varchar(255)    DEFAULT ''                NOT NULL,
	ldap_port                integer         DEFAULT 389               NOT NULL,
	ldap_base_dn             varchar(255)    DEFAULT ''                NOT NULL,
	ldap_bind_dn             varchar(255)    DEFAULT ''                NOT NULL,
	ldap_bind_password       varchar(128)    DEFAULT ''                NOT NULL,
	ldap_search_attribute    varchar(128)    DEFAULT ''                NOT NULL,
	dropdown_first_entry     integer         DEFAULT '1'               NOT NULL,
	dropdown_first_remember  integer         DEFAULT '1'               NOT NULL,
	discovery_groupid        bigint                                    NOT NULL REFERENCES groups (groupid),
	max_in_table             integer         DEFAULT '50'              NOT NULL,
	search_limit             integer         DEFAULT '1000'            NOT NULL,
	severity_color_0         varchar(6)      DEFAULT 'DBDBDB'          NOT NULL,
	severity_color_1         varchar(6)      DEFAULT 'D6F6FF'          NOT NULL,
	severity_color_2         varchar(6)      DEFAULT 'FFF6A5'          NOT NULL,
	severity_color_3         varchar(6)      DEFAULT 'FFB689'          NOT NULL,
	severity_color_4         varchar(6)      DEFAULT 'FF9999'          NOT NULL,
	severity_color_5         varchar(6)      DEFAULT 'FF3838'          NOT NULL,
	severity_name_0          varchar(32)     DEFAULT 'Not classified'  NOT NULL,
	severity_name_1          varchar(32)     DEFAULT 'Information'     NOT NULL,
	severity_name_2          varchar(32)     DEFAULT 'Warning'         NOT NULL,
	severity_name_3          varchar(32)     DEFAULT 'Average'         NOT NULL,
	severity_name_4          varchar(32)     DEFAULT 'High'            NOT NULL,
	severity_name_5          varchar(32)     DEFAULT 'Disaster'        NOT NULL,
	ok_period                integer         DEFAULT '1800'            NOT NULL,
	blink_period             integer         DEFAULT '1800'            NOT NULL,
	problem_unack_color      varchar(6)      DEFAULT 'DC0000'          NOT NULL,
	problem_ack_color        varchar(6)      DEFAULT 'DC0000'          NOT NULL,
	ok_unack_color           varchar(6)      DEFAULT '00AA00'          NOT NULL,
	ok_ack_color             varchar(6)      DEFAULT '00AA00'          NOT NULL,
	problem_unack_style      integer         DEFAULT '1'               NOT NULL,
	problem_ack_style        integer         DEFAULT '1'               NOT NULL,
	ok_unack_style           integer         DEFAULT '1'               NOT NULL,
	ok_ack_style             integer         DEFAULT '1'               NOT NULL,
	snmptrap_logging         integer         DEFAULT '1'               NOT NULL,
	server_check_interval    integer         DEFAULT '10'              NOT NULL,
	PRIMARY KEY (configid)
);
CREATE TABLE triggers (
	triggerid                bigint                                    NOT NULL,
	expression               varchar(255)    DEFAULT ''                NOT NULL,
	description              varchar(255)    DEFAULT ''                NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	value                    integer         DEFAULT '0'               NOT NULL,
	priority                 integer         DEFAULT '0'               NOT NULL,
	lastchange               integer         DEFAULT '0'               NOT NULL,
	comments                 text            DEFAULT ''                NOT NULL,
	error                    varchar(128)    DEFAULT ''                NOT NULL,
	templateid               bigint                                    NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	value_flags              integer         DEFAULT '0'               NOT NULL,
	flags                    integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX triggers_1 ON triggers (status);
CREATE INDEX triggers_2 ON triggers (value);
CREATE TABLE trigger_depends (
	triggerdepid             bigint                                    NOT NULL,
	triggerid_down           bigint                                    NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	triggerid_up             bigint                                    NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	PRIMARY KEY (triggerdepid)
);
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);
CREATE TABLE functions (
	functionid               bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	triggerid                bigint                                    NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	function                 varchar(12)     DEFAULT ''                NOT NULL,
	parameter                varchar(255)    DEFAULT '0'               NOT NULL,
	PRIMARY KEY (functionid)
);
CREATE INDEX functions_1 ON functions (triggerid);
CREATE INDEX functions_2 ON functions (itemid,function,parameter);
CREATE TABLE graphs (
	graphid                  bigint                                    NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	width                    integer         DEFAULT '0'               NOT NULL,
	height                   integer         DEFAULT '0'               NOT NULL,
	yaxismin                 double(16,4)    DEFAULT '0'               NOT NULL,
	yaxismax                 double(16,4)    DEFAULT '0'               NOT NULL,
	templateid               bigint                                    NULL REFERENCES graphs (graphid) ON DELETE CASCADE,
	show_work_period         integer         DEFAULT '1'               NOT NULL,
	show_triggers            integer         DEFAULT '1'               NOT NULL,
	graphtype                integer         DEFAULT '0'               NOT NULL,
	show_legend              integer         DEFAULT '1'               NOT NULL,
	show_3d                  integer         DEFAULT '0'               NOT NULL,
	percent_left             double(16,4)    DEFAULT '0'               NOT NULL,
	percent_right            double(16,4)    DEFAULT '0'               NOT NULL,
	ymin_type                integer         DEFAULT '0'               NOT NULL,
	ymax_type                integer         DEFAULT '0'               NOT NULL,
	ymin_itemid              bigint                                    NULL REFERENCES items (itemid),
	ymax_itemid              bigint                                    NULL REFERENCES items (itemid),
	flags                    integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (graphid)
);
CREATE INDEX graphs_graphs_1 ON graphs (name);
CREATE TABLE graphs_items (
	gitemid                  bigint                                    NOT NULL,
	graphid                  bigint                                    NOT NULL REFERENCES graphs (graphid) ON DELETE CASCADE,
	itemid                   bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	drawtype                 integer         DEFAULT '0'               NOT NULL,
	sortorder                integer         DEFAULT '0'               NOT NULL,
	color                    varchar(6)      DEFAULT '009600'          NOT NULL,
	yaxisside                integer         DEFAULT '1'               NOT NULL,
	calc_fnc                 integer         DEFAULT '2'               NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (gitemid)
);
CREATE INDEX graphs_items_1 ON graphs_items (itemid);
CREATE INDEX graphs_items_2 ON graphs_items (graphid);
CREATE TABLE graph_theme (
	graphthemeid             bigint                                    NOT NULL,
	description              varchar(64)     DEFAULT ''                NOT NULL,
	theme                    varchar(64)     DEFAULT ''                NOT NULL,
	backgroundcolor          varchar(6)      DEFAULT 'F0F0F0'          NOT NULL,
	graphcolor               varchar(6)      DEFAULT 'FFFFFF'          NOT NULL,
	graphbordercolor         varchar(6)      DEFAULT '222222'          NOT NULL,
	gridcolor                varchar(6)      DEFAULT 'CCCCCC'          NOT NULL,
	maingridcolor            varchar(6)      DEFAULT 'AAAAAA'          NOT NULL,
	gridbordercolor          varchar(6)      DEFAULT '000000'          NOT NULL,
	textcolor                varchar(6)      DEFAULT '202020'          NOT NULL,
	highlightcolor           varchar(6)      DEFAULT 'AA4444'          NOT NULL,
	leftpercentilecolor      varchar(6)      DEFAULT '11CC11'          NOT NULL,
	rightpercentilecolor     varchar(6)      DEFAULT 'CC1111'          NOT NULL,
	nonworktimecolor         varchar(6)      DEFAULT 'CCCCCC'          NOT NULL,
	gridview                 integer         DEFAULT 1                 NOT NULL,
	legendview               integer         DEFAULT 1                 NOT NULL,
	PRIMARY KEY (graphthemeid)
);
CREATE INDEX graph_theme_1 ON graph_theme (description);
CREATE INDEX graph_theme_2 ON graph_theme (theme);
CREATE TABLE help_items (
	itemtype                 integer         DEFAULT '0'               NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	description              varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (itemtype,key_)
);
CREATE TABLE globalmacro (
	globalmacroid            bigint                                    NOT NULL,
	macro                    varchar(64)     DEFAULT ''                NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (globalmacroid)
);
CREATE INDEX globalmacro_1 ON globalmacro (macro);
CREATE TABLE hostmacro (
	hostmacroid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	macro                    varchar(64)     DEFAULT ''                NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (hostmacroid)
);
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);
CREATE TABLE hosts_groups (
	hostgroupid              bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	groupid                  bigint                                    NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,
	PRIMARY KEY (hostgroupid)
);
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);
CREATE TABLE hosts_templates (
	hosttemplateid           bigint                                    NOT NULL,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	templateid               bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	PRIMARY KEY (hosttemplateid)
);
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
CREATE TABLE items_applications (
	itemappid                bigint                                    NOT NULL,
	applicationid            bigint                                    NOT NULL REFERENCES applications (applicationid) ON DELETE CASCADE,
	itemid                   bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	PRIMARY KEY (itemappid)
);
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);
CREATE INDEX items_applications_2 ON items_applications (itemid);
CREATE TABLE mappings (
	mappingid                bigint                                    NOT NULL,
	valuemapid               bigint                                    NOT NULL REFERENCES valuemaps (valuemapid) ON DELETE CASCADE,
	value                    varchar(64)     DEFAULT ''                NOT NULL,
	newvalue                 varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (mappingid)
);
CREATE INDEX mappings_1 ON mappings (valuemapid);
CREATE TABLE media (
	mediaid                  bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	mediatypeid              bigint                                    NOT NULL REFERENCES media_type (mediatypeid) ON DELETE CASCADE,
	sendto                   varchar(100)    DEFAULT ''                NOT NULL,
	active                   integer         DEFAULT '0'               NOT NULL,
	severity                 integer         DEFAULT '63'              NOT NULL,
	period                   varchar(100)    DEFAULT '1-7,00:00-24:00' NOT NULL,
	PRIMARY KEY (mediaid)
);
CREATE INDEX media_1 ON media (userid);
CREATE INDEX media_2 ON media (mediatypeid);
CREATE TABLE rights (
	rightid                  bigint                                    NOT NULL,
	groupid                  bigint                                    NOT NULL REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE,
	permission               integer         DEFAULT '0'               NOT NULL,
	id                       bigint                                    NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,
	PRIMARY KEY (rightid)
);
CREATE INDEX rights_1 ON rights (groupid);
CREATE INDEX rights_2 ON rights (id);
CREATE TABLE services (
	serviceid                bigint                                    NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	algorithm                integer         DEFAULT '0'               NOT NULL,
	triggerid                bigint                                    NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	showsla                  integer         DEFAULT '0'               NOT NULL,
	goodsla                  double(16,4)    DEFAULT '99.9'            NOT NULL,
	sortorder                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (serviceid)
);
CREATE INDEX services_1 ON services (triggerid);
CREATE TABLE services_links (
	linkid                   bigint                                    NOT NULL,
	serviceupid              bigint                                    NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,
	servicedownid            bigint                                    NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,
	soft                     integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (linkid)
);
CREATE INDEX services_links_links_1 ON services_links (servicedownid);
CREATE UNIQUE INDEX services_links_links_2 ON services_links (serviceupid,servicedownid);
CREATE TABLE services_times (
	timeid                   bigint                                    NOT NULL,
	serviceid                bigint                                    NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	ts_from                  integer         DEFAULT '0'               NOT NULL,
	ts_to                    integer         DEFAULT '0'               NOT NULL,
	note                     varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (timeid)
);
CREATE INDEX services_times_times_1 ON services_times (serviceid,type,ts_from,ts_to);
CREATE TABLE icon_map (
	iconmapid                bigint                                    NOT NULL,
	name                     varchar(64)     DEFAULT ''                NOT NULL,
	default_iconid           bigint                                    NOT NULL REFERENCES images (imageid),
	PRIMARY KEY (iconmapid)
);
CREATE INDEX icon_map_1 ON icon_map (name);
CREATE TABLE icon_mapping (
	iconmappingid            bigint                                    NOT NULL,
	iconmapid                bigint                                    NOT NULL REFERENCES icon_map (iconmapid) ON DELETE CASCADE,
	iconid                   bigint                                    NOT NULL REFERENCES images (imageid),
	inventory_link           integer         DEFAULT '0'               NOT NULL,
	expression               varchar(64)     DEFAULT ''                NOT NULL,
	sortorder                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (iconmappingid)
);
CREATE INDEX icon_mapping_1 ON icon_mapping (iconmapid);
CREATE TABLE sysmaps (
	sysmapid                 bigint                                    NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	width                    integer         DEFAULT '600'             NOT NULL,
	height                   integer         DEFAULT '400'             NOT NULL,
	backgroundid             bigint                                    NULL REFERENCES images (imageid),
	label_type               integer         DEFAULT '2'               NOT NULL,
	label_location           integer         DEFAULT '3'               NOT NULL,
	highlight                integer         DEFAULT '1'               NOT NULL,
	expandproblem            integer         DEFAULT '1'               NOT NULL,
	markelements             integer         DEFAULT '0'               NOT NULL,
	show_unack               integer         DEFAULT '0'               NOT NULL,
	grid_size                integer         DEFAULT '50'              NOT NULL,
	grid_show                integer         DEFAULT '1'               NOT NULL,
	grid_align               integer         DEFAULT '1'               NOT NULL,
	label_format             integer         DEFAULT '0'               NOT NULL,
	label_type_host          integer         DEFAULT '2'               NOT NULL,
	label_type_hostgroup     integer         DEFAULT '2'               NOT NULL,
	label_type_trigger       integer         DEFAULT '2'               NOT NULL,
	label_type_map           integer         DEFAULT '2'               NOT NULL,
	label_type_image         integer         DEFAULT '2'               NOT NULL,
	label_string_host        varchar(255)    DEFAULT ''                NOT NULL,
	label_string_hostgroup   varchar(255)    DEFAULT ''                NOT NULL,
	label_string_trigger     varchar(255)    DEFAULT ''                NOT NULL,
	label_string_map         varchar(255)    DEFAULT ''                NOT NULL,
	label_string_image       varchar(255)    DEFAULT ''                NOT NULL,
	iconmapid                bigint                                    NULL REFERENCES icon_map (iconmapid),
	expand_macros            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapid)
);
CREATE INDEX sysmaps_1 ON sysmaps (name);
CREATE TABLE sysmaps_elements (
	selementid               bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL REFERENCES sysmaps (sysmapid) ON DELETE CASCADE,
	elementid                bigint          DEFAULT '0'               NOT NULL,
	elementtype              integer         DEFAULT '0'               NOT NULL,
	iconid_off               bigint                                    NULL REFERENCES images (imageid),
	iconid_on                bigint                                    NULL REFERENCES images (imageid),
	label                    varchar(255)    DEFAULT ''                NOT NULL,
	label_location           integer                                   NULL,
	x                        integer         DEFAULT '0'               NOT NULL,
	y                        integer         DEFAULT '0'               NOT NULL,
	iconid_disabled          bigint                                    NULL REFERENCES images (imageid),
	iconid_maintenance       bigint                                    NULL REFERENCES images (imageid),
	elementsubtype           integer         DEFAULT '0'               NOT NULL,
	areatype                 integer         DEFAULT '0'               NOT NULL,
	width                    integer         DEFAULT '200'             NOT NULL,
	height                   integer         DEFAULT '200'             NOT NULL,
	viewtype                 integer         DEFAULT '0'               NOT NULL,
	use_iconmap              integer         DEFAULT '1'               NOT NULL,
	PRIMARY KEY (selementid)
);
CREATE TABLE sysmaps_links (
	linkid                   bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL REFERENCES sysmaps (sysmapid) ON DELETE CASCADE,
	selementid1              bigint                                    NOT NULL REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE,
	selementid2              bigint                                    NOT NULL REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE,
	drawtype                 integer         DEFAULT '0'               NOT NULL,
	color                    varchar(6)      DEFAULT '000000'          NOT NULL,
	label                    varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (linkid)
);
CREATE TABLE sysmaps_link_triggers (
	linktriggerid            bigint                                    NOT NULL,
	linkid                   bigint                                    NOT NULL REFERENCES sysmaps_links (linkid) ON DELETE CASCADE,
	triggerid                bigint                                    NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	drawtype                 integer         DEFAULT '0'               NOT NULL,
	color                    varchar(6)      DEFAULT '000000'          NOT NULL,
	PRIMARY KEY (linktriggerid)
);
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);
CREATE TABLE sysmap_element_url (
	sysmapelementurlid       bigint                                    NOT NULL,
	selementid               bigint                                    NOT NULL REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (sysmapelementurlid)
);
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);
CREATE TABLE sysmap_url (
	sysmapurlid              bigint                                    NOT NULL,
	sysmapid                 bigint                                    NOT NULL REFERENCES sysmaps (sysmapid) ON DELETE CASCADE,
	name                     varchar(255)                              NOT NULL,
	url                      varchar(255)    DEFAULT ''                NOT NULL,
	elementtype              integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapurlid)
);
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);
CREATE TABLE maintenances_hosts (
	maintenance_hostid       bigint                                    NOT NULL,
	maintenanceid            bigint                                    NOT NULL REFERENCES maintenances (maintenanceid) ON DELETE CASCADE,
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	PRIMARY KEY (maintenance_hostid)
);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
CREATE TABLE maintenances_groups (
	maintenance_groupid      bigint                                    NOT NULL,
	maintenanceid            bigint                                    NOT NULL REFERENCES maintenances (maintenanceid) ON DELETE CASCADE,
	groupid                  bigint                                    NOT NULL REFERENCES groups (groupid) ON DELETE CASCADE,
	PRIMARY KEY (maintenance_groupid)
);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
CREATE TABLE timeperiods (
	timeperiodid             bigint                                    NOT NULL,
	timeperiod_type          integer         DEFAULT '0'               NOT NULL,
	every                    integer         DEFAULT '0'               NOT NULL,
	month                    integer         DEFAULT '0'               NOT NULL,
	dayofweek                integer         DEFAULT '0'               NOT NULL,
	day                      integer         DEFAULT '0'               NOT NULL,
	start_time               integer         DEFAULT '0'               NOT NULL,
	period                   integer         DEFAULT '0'               NOT NULL,
	start_date               integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (timeperiodid)
);
CREATE TABLE maintenances_windows (
	maintenance_timeperiodid bigint                                    NOT NULL,
	maintenanceid            bigint                                    NOT NULL REFERENCES maintenances (maintenanceid) ON DELETE CASCADE,
	timeperiodid             bigint                                    NOT NULL REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE,
	PRIMARY KEY (maintenance_timeperiodid)
);
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);
CREATE TABLE regexps (
	regexpid                 bigint                                    NOT NULL,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	test_string              text            DEFAULT ''                NOT NULL,
	PRIMARY KEY (regexpid)
);
CREATE INDEX regexps_1 ON regexps (name);
CREATE TABLE expressions (
	expressionid             bigint                                    NOT NULL,
	regexpid                 bigint                                    NOT NULL REFERENCES regexps (regexpid) ON DELETE CASCADE,
	expression               varchar(255)    DEFAULT ''                NOT NULL,
	expression_type          integer         DEFAULT '0'               NOT NULL,
	exp_delimiter            varchar(1)      DEFAULT ''                NOT NULL,
	case_sensitive           integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (expressionid)
);
CREATE INDEX expressions_1 ON expressions (regexpid);
CREATE TABLE nodes (
	nodeid                   integer                                   NOT NULL,
	name                     varchar(64)     DEFAULT '0'               NOT NULL,
	ip                       varchar(39)     DEFAULT ''                NOT NULL,
	port                     integer         DEFAULT '10051'           NOT NULL,
	nodetype                 integer         DEFAULT '0'               NOT NULL,
	masterid                 integer                                   NULL REFERENCES nodes (nodeid),
	PRIMARY KEY (nodeid)
);
CREATE TABLE node_cksum (
	nodeid                   integer                                   NOT NULL REFERENCES nodes (nodeid) ON DELETE CASCADE,
	tablename                varchar(64)     DEFAULT ''                NOT NULL,
	recordid                 bigint                                    NOT NULL,
	cksumtype                integer         DEFAULT '0'               NOT NULL,
	cksum                    text            DEFAULT ''                NOT NULL,
	sync                     char(128)       DEFAULT ''                NOT NULL
);
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);
CREATE TABLE ids (
	nodeid                   integer                                   NOT NULL,
	table_name               varchar(64)     DEFAULT ''                NOT NULL,
	field_name               varchar(64)     DEFAULT ''                NOT NULL,
	nextid                   bigint                                    NOT NULL,
	PRIMARY KEY (nodeid,table_name,field_name)
);
CREATE TABLE alerts (
	alertid                  bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL REFERENCES actions (actionid) ON DELETE CASCADE,
	eventid                  bigint                                    NOT NULL REFERENCES events (eventid) ON DELETE CASCADE,
	userid                   bigint                                    NULL REFERENCES users (userid) ON DELETE CASCADE,
	clock                    integer         DEFAULT '0'               NOT NULL,
	mediatypeid              bigint                                    NULL REFERENCES media_type (mediatypeid) ON DELETE CASCADE,
	sendto                   varchar(100)    DEFAULT ''                NOT NULL,
	subject                  varchar(255)    DEFAULT ''                NOT NULL,
	message                  text            DEFAULT ''                NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	retries                  integer         DEFAULT '0'               NOT NULL,
	error                    varchar(128)    DEFAULT ''                NOT NULL,
	nextcheck                integer         DEFAULT '0'               NOT NULL,
	esc_step                 integer         DEFAULT '0'               NOT NULL,
	alerttype                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (alertid)
);
CREATE INDEX alerts_1 ON alerts (actionid);
CREATE INDEX alerts_2 ON alerts (clock);
CREATE INDEX alerts_3 ON alerts (eventid);
CREATE INDEX alerts_4 ON alerts (status,retries);
CREATE INDEX alerts_5 ON alerts (mediatypeid);
CREATE INDEX alerts_6 ON alerts (userid);
CREATE TABLE history (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    double(16,4)    DEFAULT '0.0000'          NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX history_1 ON history (itemid,clock);
CREATE TABLE history_sync (
	id                       integer                                   NOT NULL PRIMARY KEY AUTOINCREMENT,
	nodeid                   integer                                   NOT NULL,
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    double(16,4)    DEFAULT '0.0000'          NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX history_sync_1 ON history_sync (nodeid,id);
CREATE TABLE history_uint (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    bigint          DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);
CREATE TABLE history_uint_sync (
	id                       integer                                   NOT NULL PRIMARY KEY AUTOINCREMENT,
	nodeid                   integer                                   NOT NULL,
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    bigint          DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX history_uint_sync_1 ON history_uint_sync (nodeid,id);
CREATE TABLE history_str (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX history_str_1 ON history_str (itemid,clock);
CREATE TABLE history_str_sync (
	id                       integer                                   NOT NULL PRIMARY KEY AUTOINCREMENT,
	nodeid                   integer                                   NOT NULL,
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX history_str_sync_1 ON history_str_sync (nodeid,id);
CREATE TABLE history_log (
	id                       bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	timestamp                integer         DEFAULT '0'               NOT NULL,
	source                   varchar(64)     DEFAULT ''                NOT NULL,
	severity                 integer         DEFAULT '0'               NOT NULL,
	value                    text            DEFAULT ''                NOT NULL,
	logeventid               integer         DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_log_1 ON history_log (itemid,clock);
CREATE UNIQUE INDEX history_log_2 ON history_log (itemid,id);
CREATE TABLE history_text (
	id                       bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    text            DEFAULT ''                NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_text_1 ON history_text (itemid,clock);
CREATE UNIQUE INDEX history_text_2 ON history_text (itemid,id);
CREATE TABLE proxy_history (
	id                       integer                                   NOT NULL PRIMARY KEY AUTOINCREMENT,
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	timestamp                integer         DEFAULT '0'               NOT NULL,
	source                   varchar(64)     DEFAULT ''                NOT NULL,
	severity                 integer         DEFAULT '0'               NOT NULL,
	value                    text            DEFAULT ''                NOT NULL,
	logeventid               integer         DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL
);
CREATE INDEX proxy_history_1 ON proxy_history (clock);
CREATE TABLE proxy_dhistory (
	id                       integer                                   NOT NULL PRIMARY KEY AUTOINCREMENT,
	clock                    integer         DEFAULT '0'               NOT NULL,
	druleid                  bigint                                    NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	ip                       varchar(39)     DEFAULT ''                NOT NULL,
	port                     integer         DEFAULT '0'               NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	dcheckid                 bigint                                    NULL,
	dns                      varchar(64)     DEFAULT ''                NOT NULL
);
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);
CREATE TABLE events (
	eventid                  bigint                                    NOT NULL,
	source                   integer         DEFAULT '0'               NOT NULL,
	object                   integer         DEFAULT '0'               NOT NULL,
	objectid                 bigint          DEFAULT '0'               NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    integer         DEFAULT '0'               NOT NULL,
	acknowledged             integer         DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL,
	value_changed            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX events_1 ON events (object,objectid,eventid);
CREATE INDEX events_2 ON events (clock);
CREATE TABLE trends (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	num                      integer         DEFAULT '0'               NOT NULL,
	value_min                double(16,4)    DEFAULT '0.0000'          NOT NULL,
	value_avg                double(16,4)    DEFAULT '0.0000'          NOT NULL,
	value_max                double(16,4)    DEFAULT '0.0000'          NOT NULL,
	PRIMARY KEY (itemid,clock)
);
CREATE TABLE trends_uint (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	num                      integer         DEFAULT '0'               NOT NULL,
	value_min                bigint          DEFAULT '0'               NOT NULL,
	value_avg                bigint          DEFAULT '0'               NOT NULL,
	value_max                bigint          DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock)
);
CREATE TABLE acknowledges (
	acknowledgeid            bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	eventid                  bigint                                    NOT NULL REFERENCES events (eventid) ON DELETE CASCADE,
	clock                    integer         DEFAULT '0'               NOT NULL,
	message                  varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (acknowledgeid)
);
CREATE INDEX acknowledges_1 ON acknowledges (userid);
CREATE INDEX acknowledges_2 ON acknowledges (eventid);
CREATE INDEX acknowledges_3 ON acknowledges (clock);
CREATE TABLE auditlog (
	auditid                  bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	clock                    integer         DEFAULT '0'               NOT NULL,
	action                   integer         DEFAULT '0'               NOT NULL,
	resourcetype             integer         DEFAULT '0'               NOT NULL,
	details                  varchar(128)    DEFAULT '0'               NOT NULL,
	ip                       varchar(39)     DEFAULT ''                NOT NULL,
	resourceid               bigint          DEFAULT '0'               NOT NULL,
	resourcename             varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (auditid)
);
CREATE INDEX auditlog_1 ON auditlog (userid,clock);
CREATE INDEX auditlog_2 ON auditlog (clock);
CREATE TABLE auditlog_details (
	auditdetailid            bigint                                    NOT NULL,
	auditid                  bigint                                    NOT NULL REFERENCES auditlog (auditid) ON DELETE CASCADE,
	table_name               varchar(64)     DEFAULT ''                NOT NULL,
	field_name               varchar(64)     DEFAULT ''                NOT NULL,
	oldvalue                 text            DEFAULT ''                NOT NULL,
	newvalue                 text            DEFAULT ''                NOT NULL,
	PRIMARY KEY (auditdetailid)
);
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);
CREATE TABLE service_alarms (
	servicealarmid           bigint                                    NOT NULL,
	serviceid                bigint                                    NOT NULL REFERENCES services (serviceid) ON DELETE CASCADE,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (servicealarmid)
);
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);
CREATE INDEX service_alarms_2 ON service_alarms (clock);
CREATE TABLE autoreg_host (
	autoreg_hostid           bigint                                    NOT NULL,
	proxy_hostid             bigint                                    NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
	host                     varchar(64)     DEFAULT ''                NOT NULL,
	listen_ip                varchar(39)     DEFAULT ''                NOT NULL,
	listen_port              integer         DEFAULT '0'               NOT NULL,
	listen_dns               varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (autoreg_hostid)
);
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);
CREATE TABLE proxy_autoreg_host (
	id                       integer                                   NOT NULL PRIMARY KEY AUTOINCREMENT,
	clock                    integer         DEFAULT '0'               NOT NULL,
	host                     varchar(64)     DEFAULT ''                NOT NULL,
	listen_ip                varchar(39)     DEFAULT ''                NOT NULL,
	listen_port              integer         DEFAULT '0'               NOT NULL,
	listen_dns               varchar(64)     DEFAULT ''                NOT NULL
);
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);
CREATE TABLE dhosts (
	dhostid                  bigint                                    NOT NULL,
	druleid                  bigint                                    NOT NULL REFERENCES drules (druleid) ON DELETE CASCADE,
	status                   integer         DEFAULT '0'               NOT NULL,
	lastup                   integer         DEFAULT '0'               NOT NULL,
	lastdown                 integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dhostid)
);
CREATE INDEX dhosts_1 ON dhosts (druleid);
CREATE TABLE dservices (
	dserviceid               bigint                                    NOT NULL,
	dhostid                  bigint                                    NOT NULL REFERENCES dhosts (dhostid) ON DELETE CASCADE,
	type                     integer         DEFAULT '0'               NOT NULL,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	port                     integer         DEFAULT '0'               NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	lastup                   integer         DEFAULT '0'               NOT NULL,
	lastdown                 integer         DEFAULT '0'               NOT NULL,
	dcheckid                 bigint                                    NOT NULL REFERENCES dchecks (dcheckid) ON DELETE CASCADE,
	ip                       varchar(39)     DEFAULT ''                NOT NULL,
	dns                      varchar(64)     DEFAULT ''                NOT NULL,
	PRIMARY KEY (dserviceid)
);
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,type,key_,ip,port);
CREATE INDEX dservices_2 ON dservices (dhostid);
CREATE TABLE escalations (
	escalationid             bigint                                    NOT NULL,
	actionid                 bigint                                    NOT NULL,
	triggerid                bigint                                    NULL,
	eventid                  bigint                                    NULL,
	r_eventid                bigint                                    NULL,
	nextcheck                integer         DEFAULT '0'               NOT NULL,
	esc_step                 integer         DEFAULT '0'               NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (escalationid)
);
CREATE INDEX escalations_1 ON escalations (actionid,triggerid);
CREATE TABLE globalvars (
	globalvarid              bigint                                    NOT NULL,
	snmp_lastsize            integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
);
CREATE TABLE graph_discovery (
	graphdiscoveryid         bigint                                    NOT NULL,
	graphid                  bigint                                    NOT NULL REFERENCES graphs (graphid) ON DELETE CASCADE,
	parent_graphid           bigint                                    NOT NULL REFERENCES graphs (graphid) ON DELETE CASCADE,
	name                     varchar(128)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (graphdiscoveryid)
);
CREATE UNIQUE INDEX graph_discovery_1 ON graph_discovery (graphid,parent_graphid);
CREATE TABLE host_inventory (
	hostid                   bigint                                    NOT NULL REFERENCES hosts (hostid) ON DELETE CASCADE,
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
CREATE TABLE housekeeper (
	housekeeperid            bigint                                    NOT NULL,
	tablename                varchar(64)     DEFAULT ''                NOT NULL,
	field                    varchar(64)     DEFAULT ''                NOT NULL,
	value                    bigint                                    NOT NULL,
	PRIMARY KEY (housekeeperid)
);
CREATE TABLE images (
	imageid                  bigint                                    NOT NULL,
	imagetype                integer         DEFAULT '0'               NOT NULL,
	name                     varchar(64)     DEFAULT '0'               NOT NULL,
	image                    longblob        DEFAULT ''                NOT NULL,
	PRIMARY KEY (imageid)
);
CREATE INDEX images_1 ON images (imagetype,name);
CREATE TABLE item_discovery (
	itemdiscoveryid          bigint                                    NOT NULL,
	itemid                   bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	parent_itemid            bigint                                    NOT NULL REFERENCES items (itemid) ON DELETE CASCADE,
	key_                     varchar(255)    DEFAULT ''                NOT NULL,
	lastcheck                integer         DEFAULT '0'               NOT NULL,
	ts_delete                integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
);
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);
CREATE TABLE profiles (
	profileid                bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	idx                      varchar(96)     DEFAULT ''                NOT NULL,
	idx2                     bigint          DEFAULT '0'               NOT NULL,
	value_id                 bigint          DEFAULT '0'               NOT NULL,
	value_int                integer         DEFAULT '0'               NOT NULL,
	value_str                varchar(255)    DEFAULT ''                NOT NULL,
	source                   varchar(96)     DEFAULT ''                NOT NULL,
	type                     integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (profileid)
);
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);
CREATE INDEX profiles_2 ON profiles (userid,profileid);
CREATE TABLE sessions (
	sessionid                varchar(32)     DEFAULT ''                NOT NULL,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	lastaccess               integer         DEFAULT '0'               NOT NULL,
	status                   integer         DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sessionid)
);
CREATE INDEX sessions_1 ON sessions (userid,status);
CREATE TABLE trigger_discovery (
	triggerdiscoveryid       bigint                                    NOT NULL,
	triggerid                bigint                                    NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	parent_triggerid         bigint                                    NOT NULL REFERENCES triggers (triggerid) ON DELETE CASCADE,
	name                     varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (triggerdiscoveryid)
);
CREATE UNIQUE INDEX trigger_discovery_1 ON trigger_discovery (triggerid,parent_triggerid);
CREATE TABLE user_history (
	userhistoryid            bigint                                    NOT NULL,
	userid                   bigint                                    NOT NULL REFERENCES users (userid) ON DELETE CASCADE,
	title1                   varchar(255)    DEFAULT ''                NOT NULL,
	url1                     varchar(255)    DEFAULT ''                NOT NULL,
	title2                   varchar(255)    DEFAULT ''                NOT NULL,
	url2                     varchar(255)    DEFAULT ''                NOT NULL,
	title3                   varchar(255)    DEFAULT ''                NOT NULL,
	url3                     varchar(255)    DEFAULT ''                NOT NULL,
	title4                   varchar(255)    DEFAULT ''                NOT NULL,
	url4                     varchar(255)    DEFAULT ''                NOT NULL,
	title5                   varchar(255)    DEFAULT ''                NOT NULL,
	url5                     varchar(255)    DEFAULT ''                NOT NULL,
	PRIMARY KEY (userhistoryid)
);
CREATE UNIQUE INDEX user_history_1 ON user_history (userid);
