CREATE TABLE maintenances (
	maintenanceid            number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	maintenance_type         number(10)      DEFAULT '0'               NOT NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	active_since             number(10)      DEFAULT '0'               NOT NULL,
	active_till              number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (maintenanceid)
);
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);
CREATE TABLE hosts (
	hostid                   number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	host                     nvarchar2(64)   DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	disable_until            number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(128)  DEFAULT ''                ,
	available                number(10)      DEFAULT '0'               NOT NULL,
	errors_from              number(10)      DEFAULT '0'               NOT NULL,
	lastaccess               number(10)      DEFAULT '0'               NOT NULL,
	ipmi_authtype            number(10)      DEFAULT '0'               NOT NULL,
	ipmi_privilege           number(10)      DEFAULT '2'               NOT NULL,
	ipmi_username            nvarchar2(16)   DEFAULT ''                ,
	ipmi_password            nvarchar2(20)   DEFAULT ''                ,
	ipmi_disable_until       number(10)      DEFAULT '0'               NOT NULL,
	ipmi_available           number(10)      DEFAULT '0'               NOT NULL,
	snmp_disable_until       number(10)      DEFAULT '0'               NOT NULL,
	snmp_available           number(10)      DEFAULT '0'               NOT NULL,
	maintenanceid            number(20)                                NULL,
	maintenance_status       number(10)      DEFAULT '0'               NOT NULL,
	maintenance_type         number(10)      DEFAULT '0'               NOT NULL,
	maintenance_from         number(10)      DEFAULT '0'               NOT NULL,
	ipmi_errors_from         number(10)      DEFAULT '0'               NOT NULL,
	snmp_errors_from         number(10)      DEFAULT '0'               NOT NULL,
	ipmi_error               nvarchar2(128)  DEFAULT ''                ,
	snmp_error               nvarchar2(128)  DEFAULT ''                ,
	jmx_disable_until        number(10)      DEFAULT '0'               NOT NULL,
	jmx_available            number(10)      DEFAULT '0'               NOT NULL,
	jmx_errors_from          number(10)      DEFAULT '0'               NOT NULL,
	jmx_error                nvarchar2(128)  DEFAULT ''                ,
	name                     nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (hostid)
);
CREATE INDEX hosts_1 ON hosts (host);
CREATE INDEX hosts_2 ON hosts (status);
CREATE INDEX hosts_3 ON hosts (proxy_hostid);
CREATE INDEX hosts_4 ON hosts (name);
CREATE TABLE groups (
	groupid                  number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	internal                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE INDEX groups_1 ON groups (name);
CREATE TABLE screens (
	screenid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	hsize                    number(10)      DEFAULT '1'               NOT NULL,
	vsize                    number(10)      DEFAULT '1'               NOT NULL,
	templateid               number(20)                                NULL,
	PRIMARY KEY (screenid)
);
CREATE TABLE screens_items (
	screenitemid             number(20)                                NOT NULL,
	screenid                 number(20)                                NOT NULL,
	resourcetype             number(10)      DEFAULT '0'               NOT NULL,
	resourceid               number(20)      DEFAULT '0'               NOT NULL,
	width                    number(10)      DEFAULT '320'             NOT NULL,
	height                   number(10)      DEFAULT '200'             NOT NULL,
	x                        number(10)      DEFAULT '0'               NOT NULL,
	y                        number(10)      DEFAULT '0'               NOT NULL,
	colspan                  number(10)      DEFAULT '0'               NOT NULL,
	rowspan                  number(10)      DEFAULT '0'               NOT NULL,
	elements                 number(10)      DEFAULT '25'              NOT NULL,
	valign                   number(10)      DEFAULT '0'               NOT NULL,
	halign                   number(10)      DEFAULT '0'               NOT NULL,
	style                    number(10)      DEFAULT '0'               NOT NULL,
	url                      nvarchar2(255)  DEFAULT ''                ,
	dynamic                  number(10)      DEFAULT '0'               NOT NULL,
	sort_triggers            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (screenitemid)
);
CREATE TABLE slideshows (
	slideshowid              number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	delay                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (slideshowid)
);
CREATE TABLE slides (
	slideid                  number(20)                                NOT NULL,
	slideshowid              number(20)                                NOT NULL,
	screenid                 number(20)                                NOT NULL,
	step                     number(10)      DEFAULT '0'               NOT NULL,
	delay                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (slideid)
);
CREATE INDEX slides_slides_1 ON slides (slideshowid);
CREATE TABLE drules (
	druleid                  number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	iprange                  nvarchar2(255)  DEFAULT ''                ,
	delay                    number(10)      DEFAULT '3600'            NOT NULL,
	nextcheck                number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (druleid)
);
CREATE TABLE dchecks (
	dcheckid                 number(20)                                NOT NULL,
	druleid                  number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	snmp_community           nvarchar2(255)  DEFAULT ''                ,
	ports                    nvarchar2(255)  DEFAULT '0'               ,
	snmpv3_securityname      nvarchar2(64)   DEFAULT ''                ,
	snmpv3_securitylevel     number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_authpassphrase    nvarchar2(64)   DEFAULT ''                ,
	snmpv3_privpassphrase    nvarchar2(64)   DEFAULT ''                ,
	uniq                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dcheckid)
);
CREATE INDEX dchecks_1 ON dchecks (druleid);
CREATE TABLE applications (
	applicationid            number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	templateid               number(20)                                NULL,
	PRIMARY KEY (applicationid)
);
CREATE INDEX applications_1 ON applications (templateid);
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);
CREATE TABLE httptest (
	httptestid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	applicationid            number(20)                                NOT NULL,
	nextcheck                number(10)      DEFAULT '0'               NOT NULL,
	delay                    number(10)      DEFAULT '60'              NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	macros                   nvarchar2(2048) DEFAULT ''                ,
	agent                    nvarchar2(255)  DEFAULT ''                ,
	authentication           number(10)      DEFAULT '0'               NOT NULL,
	http_user                nvarchar2(64)   DEFAULT ''                ,
	http_password            nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (httptestid)
);
CREATE INDEX httptest_httptest_1 ON httptest (applicationid);
CREATE INDEX httptest_2 ON httptest (name);
CREATE INDEX httptest_3 ON httptest (status);
CREATE TABLE httpstep (
	httpstepid               number(20)                                NOT NULL,
	httptestid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	no                       number(10)      DEFAULT '0'               NOT NULL,
	url                      nvarchar2(255)  DEFAULT ''                ,
	timeout                  number(10)      DEFAULT '30'              NOT NULL,
	posts                    nvarchar2(2048) DEFAULT ''                ,
	required                 nvarchar2(255)  DEFAULT ''                ,
	status_codes             nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (httpstepid)
);
CREATE INDEX httpstep_httpstep_1 ON httpstep (httptestid);
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
CREATE INDEX interface_1 ON interface (hostid,type);
CREATE INDEX interface_2 ON interface (ip,dns);
CREATE TABLE valuemaps (
	valuemapid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (valuemapid)
);
CREATE INDEX valuemaps_1 ON valuemaps (name);
CREATE TABLE items (
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	snmp_community           nvarchar2(64)   DEFAULT ''                ,
	snmp_oid                 nvarchar2(255)  DEFAULT ''                ,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	delay                    number(10)      DEFAULT '0'               NOT NULL,
	history                  number(10)      DEFAULT '90'              NOT NULL,
	trends                   number(10)      DEFAULT '365'             NOT NULL,
	lastvalue                nvarchar2(255)                            ,
	lastclock                number(10)                                NULL,
	prevvalue                nvarchar2(255)                            ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	value_type               number(10)      DEFAULT '0'               NOT NULL,
	trapper_hosts            nvarchar2(255)  DEFAULT ''                ,
	units                    nvarchar2(255)  DEFAULT ''                ,
	multiplier               number(10)      DEFAULT '0'               NOT NULL,
	delta                    number(10)      DEFAULT '0'               NOT NULL,
	prevorgvalue             nvarchar2(255)                            ,
	snmpv3_securityname      nvarchar2(64)   DEFAULT ''                ,
	snmpv3_securitylevel     number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_authpassphrase    nvarchar2(64)   DEFAULT ''                ,
	snmpv3_privpassphrase    nvarchar2(64)   DEFAULT ''                ,
	formula                  nvarchar2(255)  DEFAULT '1'               ,
	error                    nvarchar2(128)  DEFAULT ''                ,
	lastlogsize              number(20)      DEFAULT '0'               NOT NULL,
	logtimefmt               nvarchar2(64)   DEFAULT ''                ,
	templateid               number(20)                                NULL,
	valuemapid               number(20)                                NULL,
	delay_flex               nvarchar2(255)  DEFAULT ''                ,
	params                   nvarchar2(2048) DEFAULT ''                ,
	ipmi_sensor              nvarchar2(128)  DEFAULT ''                ,
	data_type                number(10)      DEFAULT '0'               NOT NULL,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	publickey                nvarchar2(64)   DEFAULT ''                ,
	privatekey               nvarchar2(64)   DEFAULT ''                ,
	mtime                    number(10)      DEFAULT '0'               NOT NULL,
	lastns                   number(10)                                NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	filter                   nvarchar2(255)  DEFAULT ''                ,
	interfaceid              number(20)                                NULL,
	port                     nvarchar2(64)   DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	inventory_link           number(10)      DEFAULT '0'               NOT NULL,
	lifetime                 nvarchar2(64)   DEFAULT '30'              ,
	PRIMARY KEY (itemid)
);
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);
CREATE INDEX items_3 ON items (status);
CREATE INDEX items_4 ON items (templateid);
CREATE INDEX items_5 ON items (valuemapid);
CREATE TABLE httpstepitem (
	httpstepitemid           number(20)                                NOT NULL,
	httpstepid               number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httpstepitemid)
);
CREATE UNIQUE INDEX httpstepitem_httpstepitem_1 ON httpstepitem (httpstepid,itemid);
CREATE TABLE httptestitem (
	httptestitemid           number(20)                                NOT NULL,
	httptestid               number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httptestitemid)
);
CREATE UNIQUE INDEX httptestitem_httptestitem_1 ON httptestitem (httptestid,itemid);
CREATE TABLE media_type (
	mediatypeid              number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	description              nvarchar2(100)  DEFAULT ''                ,
	smtp_server              nvarchar2(255)  DEFAULT ''                ,
	smtp_helo                nvarchar2(255)  DEFAULT ''                ,
	smtp_email               nvarchar2(255)  DEFAULT ''                ,
	exec_path                nvarchar2(255)  DEFAULT ''                ,
	gsm_modem                nvarchar2(255)  DEFAULT ''                ,
	username                 nvarchar2(255)  DEFAULT ''                ,
	passwd                   nvarchar2(255)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (mediatypeid)
);
CREATE TABLE users (
	userid                   number(20)                                NOT NULL,
	alias                    nvarchar2(100)  DEFAULT ''                ,
	name                     nvarchar2(100)  DEFAULT ''                ,
	surname                  nvarchar2(100)  DEFAULT ''                ,
	passwd                   nvarchar2(32)   DEFAULT ''                ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	autologin                number(10)      DEFAULT '0'               NOT NULL,
	autologout               number(10)      DEFAULT '900'             NOT NULL,
	lang                     nvarchar2(5)    DEFAULT 'en_GB'           ,
	refresh                  number(10)      DEFAULT '30'              NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	theme                    nvarchar2(128)  DEFAULT 'default'         ,
	attempt_failed           number(10)      DEFAULT 0                 NOT NULL,
	attempt_ip               nvarchar2(39)   DEFAULT ''                ,
	attempt_clock            number(10)      DEFAULT 0                 NOT NULL,
	rows_per_page            number(10)      DEFAULT 50                NOT NULL,
	PRIMARY KEY (userid)
);
CREATE INDEX users_1 ON users (alias);
CREATE TABLE usrgrp (
	usrgrpid                 number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	gui_access               number(10)      DEFAULT '0'               NOT NULL,
	users_status             number(10)      DEFAULT '0'               NOT NULL,
	debug_mode               number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (usrgrpid)
);
CREATE INDEX usrgrp_1 ON usrgrp (name);
CREATE TABLE users_groups (
	id                       number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	PRIMARY KEY (id)
);
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);
CREATE TABLE scripts (
	scriptid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	command                  nvarchar2(255)  DEFAULT ''                ,
	host_access              number(10)      DEFAULT '2'               NOT NULL,
	usrgrpid                 number(20)                                NULL,
	groupid                  number(20)                                NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	confirmation             nvarchar2(255)  DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	execute_on               number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (scriptid)
);
CREATE TABLE actions (
	actionid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	eventsource              number(10)      DEFAULT '0'               NOT NULL,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	esc_period               number(10)      DEFAULT '0'               NOT NULL,
	def_shortdata            nvarchar2(255)  DEFAULT ''                ,
	def_longdata             nvarchar2(2048) DEFAULT ''                ,
	recovery_msg             number(10)      DEFAULT '0'               NOT NULL,
	r_shortdata              nvarchar2(255)  DEFAULT ''                ,
	r_longdata               nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (actionid)
);
CREATE INDEX actions_1 ON actions (eventsource,status);
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
CREATE TABLE opmessage (
	operationid              number(20)                                NOT NULL,
	default_msg              number(10)      DEFAULT '0'               NOT NULL,
	subject                  nvarchar2(255)  DEFAULT ''                ,
	message                  nvarchar2(2048) DEFAULT ''                ,
	mediatypeid              number(20)                                NULL,
	PRIMARY KEY (operationid)
);
CREATE TABLE opmessage_grp (
	opmessage_grpid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	PRIMARY KEY (opmessage_grpid)
);
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
CREATE TABLE opmessage_usr (
	opmessage_usrid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	PRIMARY KEY (opmessage_usrid)
);
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
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
CREATE TABLE opcommand_hst (
	opcommand_hstid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	hostid                   number(20)                                NULL,
	PRIMARY KEY (opcommand_hstid)
);
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
CREATE TABLE opcommand_grp (
	opcommand_grpid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (opcommand_grpid)
);
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
CREATE TABLE opgroup (
	opgroupid                number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (opgroupid)
);
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
CREATE TABLE optemplate (
	optemplateid             number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (optemplateid)
);
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
CREATE TABLE opconditions (
	opconditionid            number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	conditiontype            number(10)      DEFAULT '0'               NOT NULL,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (opconditionid)
);
CREATE INDEX opconditions_1 ON opconditions (operationid);
CREATE TABLE conditions (
	conditionid              number(20)                                NOT NULL,
	actionid                 number(20)                                NOT NULL,
	conditiontype            number(10)      DEFAULT '0'               NOT NULL,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (conditionid)
);
CREATE INDEX conditions_1 ON conditions (actionid);
CREATE TABLE config (
	configid                 number(20)                                NOT NULL,
	alert_history            number(10)      DEFAULT '0'               NOT NULL,
	event_history            number(10)      DEFAULT '0'               NOT NULL,
	refresh_unsupported      number(10)      DEFAULT '0'               NOT NULL,
	work_period              nvarchar2(100)  DEFAULT '1-5,00:00-24:00' ,
	alert_usrgrpid           number(20)                                NULL,
	event_ack_enable         number(10)      DEFAULT '1'               NOT NULL,
	event_expire             number(10)      DEFAULT '7'               NOT NULL,
	event_show_max           number(10)      DEFAULT '100'             NOT NULL,
	default_theme            nvarchar2(128)  DEFAULT 'originalblue'    ,
	authentication_type      number(10)      DEFAULT '0'               NOT NULL,
	ldap_host                nvarchar2(255)  DEFAULT ''                ,
	ldap_port                number(10)      DEFAULT 389               NOT NULL,
	ldap_base_dn             nvarchar2(255)  DEFAULT ''                ,
	ldap_bind_dn             nvarchar2(255)  DEFAULT ''                ,
	ldap_bind_password       nvarchar2(128)  DEFAULT ''                ,
	ldap_search_attribute    nvarchar2(128)  DEFAULT ''                ,
	dropdown_first_entry     number(10)      DEFAULT '1'               NOT NULL,
	dropdown_first_remember  number(10)      DEFAULT '1'               NOT NULL,
	discovery_groupid        number(20)                                NOT NULL,
	max_in_table             number(10)      DEFAULT '50'              NOT NULL,
	search_limit             number(10)      DEFAULT '1000'            NOT NULL,
	severity_color_0         nvarchar2(6)    DEFAULT 'DBDBDB'          ,
	severity_color_1         nvarchar2(6)    DEFAULT 'D6F6FF'          ,
	severity_color_2         nvarchar2(6)    DEFAULT 'FFF6A5'          ,
	severity_color_3         nvarchar2(6)    DEFAULT 'FFB689'          ,
	severity_color_4         nvarchar2(6)    DEFAULT 'FF9999'          ,
	severity_color_5         nvarchar2(6)    DEFAULT 'FF3838'          ,
	severity_name_0          nvarchar2(32)   DEFAULT 'Not classified'  ,
	severity_name_1          nvarchar2(32)   DEFAULT 'Information'     ,
	severity_name_2          nvarchar2(32)   DEFAULT 'Warning'         ,
	severity_name_3          nvarchar2(32)   DEFAULT 'Average'         ,
	severity_name_4          nvarchar2(32)   DEFAULT 'High'            ,
	severity_name_5          nvarchar2(32)   DEFAULT 'Disaster'        ,
	ok_period                number(10)      DEFAULT '1800'            NOT NULL,
	blink_period             number(10)      DEFAULT '1800'            NOT NULL,
	problem_unack_color      nvarchar2(6)    DEFAULT 'DC0000'          ,
	problem_ack_color        nvarchar2(6)    DEFAULT 'DC0000'          ,
	ok_unack_color           nvarchar2(6)    DEFAULT '00AA00'          ,
	ok_ack_color             nvarchar2(6)    DEFAULT '00AA00'          ,
	problem_unack_style      number(10)      DEFAULT '1'               NOT NULL,
	problem_ack_style        number(10)      DEFAULT '1'               NOT NULL,
	ok_unack_style           number(10)      DEFAULT '1'               NOT NULL,
	ok_ack_style             number(10)      DEFAULT '1'               NOT NULL,
	snmptrap_logging         number(10)      DEFAULT '1'               NOT NULL,
	server_check_interval    number(10)      DEFAULT '10'              NOT NULL,
	PRIMARY KEY (configid)
);
CREATE TABLE triggers (
	triggerid                number(20)                                NOT NULL,
	expression               nvarchar2(255)  DEFAULT ''                ,
	description              nvarchar2(255)  DEFAULT ''                ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	value                    number(10)      DEFAULT '0'               NOT NULL,
	priority                 number(10)      DEFAULT '0'               NOT NULL,
	lastchange               number(10)      DEFAULT '0'               NOT NULL,
	comments                 nvarchar2(2048) DEFAULT ''                ,
	error                    nvarchar2(128)  DEFAULT ''                ,
	templateid               number(20)                                NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	value_flags              number(10)      DEFAULT '0'               NOT NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX triggers_1 ON triggers (status);
CREATE INDEX triggers_2 ON triggers (value);
CREATE TABLE trigger_depends (
	triggerdepid             number(20)                                NOT NULL,
	triggerid_down           number(20)                                NOT NULL,
	triggerid_up             number(20)                                NOT NULL,
	PRIMARY KEY (triggerdepid)
);
CREATE UNIQUE INDEX trigger_depends_1 ON trigger_depends (triggerid_down,triggerid_up);
CREATE INDEX trigger_depends_2 ON trigger_depends (triggerid_up);
CREATE TABLE functions (
	functionid               number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	function                 nvarchar2(12)   DEFAULT ''                ,
	parameter                nvarchar2(255)  DEFAULT '0'               ,
	PRIMARY KEY (functionid)
);
CREATE INDEX functions_1 ON functions (triggerid);
CREATE INDEX functions_2 ON functions (itemid,function,parameter);
CREATE TABLE graphs (
	graphid                  number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	width                    number(10)      DEFAULT '0'               NOT NULL,
	height                   number(10)      DEFAULT '0'               NOT NULL,
	yaxismin                 number(20,4)    DEFAULT '0'               NOT NULL,
	yaxismax                 number(20,4)    DEFAULT '0'               NOT NULL,
	templateid               number(20)                                NULL,
	show_work_period         number(10)      DEFAULT '1'               NOT NULL,
	show_triggers            number(10)      DEFAULT '1'               NOT NULL,
	graphtype                number(10)      DEFAULT '0'               NOT NULL,
	show_legend              number(10)      DEFAULT '1'               NOT NULL,
	show_3d                  number(10)      DEFAULT '0'               NOT NULL,
	percent_left             number(20,4)    DEFAULT '0'               NOT NULL,
	percent_right            number(20,4)    DEFAULT '0'               NOT NULL,
	ymin_type                number(10)      DEFAULT '0'               NOT NULL,
	ymax_type                number(10)      DEFAULT '0'               NOT NULL,
	ymin_itemid              number(20)                                NULL,
	ymax_itemid              number(20)                                NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (graphid)
);
CREATE INDEX graphs_graphs_1 ON graphs (name);
CREATE TABLE graphs_items (
	gitemid                  number(20)                                NOT NULL,
	graphid                  number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	drawtype                 number(10)      DEFAULT '0'               NOT NULL,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	color                    nvarchar2(6)    DEFAULT '009600'          ,
	yaxisside                number(10)      DEFAULT '1'               NOT NULL,
	calc_fnc                 number(10)      DEFAULT '2'               NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (gitemid)
);
CREATE INDEX graphs_items_1 ON graphs_items (itemid);
CREATE INDEX graphs_items_2 ON graphs_items (graphid);
CREATE TABLE graph_theme (
	graphthemeid             number(20)                                NOT NULL,
	description              nvarchar2(64)   DEFAULT ''                ,
	theme                    nvarchar2(64)   DEFAULT ''                ,
	backgroundcolor          nvarchar2(6)    DEFAULT 'F0F0F0'          ,
	graphcolor               nvarchar2(6)    DEFAULT 'FFFFFF'          ,
	graphbordercolor         nvarchar2(6)    DEFAULT '222222'          ,
	gridcolor                nvarchar2(6)    DEFAULT 'CCCCCC'          ,
	maingridcolor            nvarchar2(6)    DEFAULT 'AAAAAA'          ,
	gridbordercolor          nvarchar2(6)    DEFAULT '000000'          ,
	textcolor                nvarchar2(6)    DEFAULT '202020'          ,
	highlightcolor           nvarchar2(6)    DEFAULT 'AA4444'          ,
	leftpercentilecolor      nvarchar2(6)    DEFAULT '11CC11'          ,
	rightpercentilecolor     nvarchar2(6)    DEFAULT 'CC1111'          ,
	nonworktimecolor         nvarchar2(6)    DEFAULT 'CCCCCC'          ,
	gridview                 number(10)      DEFAULT 1                 NOT NULL,
	legendview               number(10)      DEFAULT 1                 NOT NULL,
	PRIMARY KEY (graphthemeid)
);
CREATE INDEX graph_theme_1 ON graph_theme (description);
CREATE INDEX graph_theme_2 ON graph_theme (theme);
CREATE TABLE help_items (
	itemtype                 number(10)      DEFAULT '0'               NOT NULL,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	description              nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (itemtype,key_)
);
CREATE TABLE globalmacro (
	globalmacroid            number(20)                                NOT NULL,
	macro                    nvarchar2(64)   DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (globalmacroid)
);
CREATE INDEX globalmacro_1 ON globalmacro (macro);
CREATE TABLE hostmacro (
	hostmacroid              number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	macro                    nvarchar2(64)   DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (hostmacroid)
);
CREATE UNIQUE INDEX hostmacro_1 ON hostmacro (hostid,macro);
CREATE TABLE hosts_groups (
	hostgroupid              number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (hostgroupid)
);
CREATE UNIQUE INDEX hosts_groups_1 ON hosts_groups (hostid,groupid);
CREATE INDEX hosts_groups_2 ON hosts_groups (groupid);
CREATE TABLE hosts_templates (
	hosttemplateid           number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (hosttemplateid)
);
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
CREATE TABLE items_applications (
	itemappid                number(20)                                NOT NULL,
	applicationid            number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	PRIMARY KEY (itemappid)
);
CREATE UNIQUE INDEX items_applications_1 ON items_applications (applicationid,itemid);
CREATE INDEX items_applications_2 ON items_applications (itemid);
CREATE TABLE mappings (
	mappingid                number(20)                                NOT NULL,
	valuemapid               number(20)                                NOT NULL,
	value                    nvarchar2(64)   DEFAULT ''                ,
	newvalue                 nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (mappingid)
);
CREATE INDEX mappings_1 ON mappings (valuemapid);
CREATE TABLE media (
	mediaid                  number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	mediatypeid              number(20)                                NOT NULL,
	sendto                   nvarchar2(100)  DEFAULT ''                ,
	active                   number(10)      DEFAULT '0'               NOT NULL,
	severity                 number(10)      DEFAULT '63'              NOT NULL,
	period                   nvarchar2(100)  DEFAULT '1-7,00:00-24:00' ,
	PRIMARY KEY (mediaid)
);
CREATE INDEX media_1 ON media (userid);
CREATE INDEX media_2 ON media (mediatypeid);
CREATE TABLE rights (
	rightid                  number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '0'               NOT NULL,
	id                       number(20)                                NOT NULL,
	PRIMARY KEY (rightid)
);
CREATE INDEX rights_1 ON rights (groupid);
CREATE INDEX rights_2 ON rights (id);
CREATE TABLE services (
	serviceid                number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	algorithm                number(10)      DEFAULT '0'               NOT NULL,
	triggerid                number(20)                                NULL,
	showsla                  number(10)      DEFAULT '0'               NOT NULL,
	goodsla                  number(20,4)    DEFAULT '99.9'            NOT NULL,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (serviceid)
);
CREATE INDEX services_1 ON services (triggerid);
CREATE TABLE services_links (
	linkid                   number(20)                                NOT NULL,
	serviceupid              number(20)                                NOT NULL,
	servicedownid            number(20)                                NOT NULL,
	soft                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (linkid)
);
CREATE INDEX services_links_links_1 ON services_links (servicedownid);
CREATE UNIQUE INDEX services_links_links_2 ON services_links (serviceupid,servicedownid);
CREATE TABLE services_times (
	timeid                   number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	ts_from                  number(10)      DEFAULT '0'               NOT NULL,
	ts_to                    number(10)      DEFAULT '0'               NOT NULL,
	note                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (timeid)
);
CREATE INDEX services_times_times_1 ON services_times (serviceid,type,ts_from,ts_to);
CREATE TABLE icon_map (
	iconmapid                number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	default_iconid           number(20)                                NOT NULL,
	PRIMARY KEY (iconmapid)
);
CREATE INDEX icon_map_1 ON icon_map (name);
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
CREATE TABLE sysmaps (
	sysmapid                 number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	width                    number(10)      DEFAULT '600'             NOT NULL,
	height                   number(10)      DEFAULT '400'             NOT NULL,
	backgroundid             number(20)                                NULL,
	label_type               number(10)      DEFAULT '2'               NOT NULL,
	label_location           number(10)      DEFAULT '3'               NOT NULL,
	highlight                number(10)      DEFAULT '1'               NOT NULL,
	expandproblem            number(10)      DEFAULT '1'               NOT NULL,
	markelements             number(10)      DEFAULT '0'               NOT NULL,
	show_unack               number(10)      DEFAULT '0'               NOT NULL,
	grid_size                number(10)      DEFAULT '50'              NOT NULL,
	grid_show                number(10)      DEFAULT '1'               NOT NULL,
	grid_align               number(10)      DEFAULT '1'               NOT NULL,
	label_format             number(10)      DEFAULT '0'               NOT NULL,
	label_type_host          number(10)      DEFAULT '2'               NOT NULL,
	label_type_hostgroup     number(10)      DEFAULT '2'               NOT NULL,
	label_type_trigger       number(10)      DEFAULT '2'               NOT NULL,
	label_type_map           number(10)      DEFAULT '2'               NOT NULL,
	label_type_image         number(10)      DEFAULT '2'               NOT NULL,
	label_string_host        nvarchar2(255)  DEFAULT ''                ,
	label_string_hostgroup   nvarchar2(255)  DEFAULT ''                ,
	label_string_trigger     nvarchar2(255)  DEFAULT ''                ,
	label_string_map         nvarchar2(255)  DEFAULT ''                ,
	label_string_image       nvarchar2(255)  DEFAULT ''                ,
	iconmapid                number(20)                                NULL,
	expand_macros            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapid)
);
CREATE INDEX sysmaps_1 ON sysmaps (name);
CREATE TABLE sysmaps_elements (
	selementid               number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	elementid                number(20)      DEFAULT '0'               NOT NULL,
	elementtype              number(10)      DEFAULT '0'               NOT NULL,
	iconid_off               number(20)                                NULL,
	iconid_on                number(20)                                NULL,
	label                    nvarchar2(255)  DEFAULT ''                ,
	label_location           number(10)                                NULL,
	x                        number(10)      DEFAULT '0'               NOT NULL,
	y                        number(10)      DEFAULT '0'               NOT NULL,
	iconid_disabled          number(20)                                NULL,
	iconid_maintenance       number(20)                                NULL,
	elementsubtype           number(10)      DEFAULT '0'               NOT NULL,
	areatype                 number(10)      DEFAULT '0'               NOT NULL,
	width                    number(10)      DEFAULT '200'             NOT NULL,
	height                   number(10)      DEFAULT '200'             NOT NULL,
	viewtype                 number(10)      DEFAULT '0'               NOT NULL,
	use_iconmap              number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (selementid)
);
CREATE TABLE sysmaps_links (
	linkid                   number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	selementid1              number(20)                                NOT NULL,
	selementid2              number(20)                                NOT NULL,
	drawtype                 number(10)      DEFAULT '0'               NOT NULL,
	color                    nvarchar2(6)    DEFAULT '000000'          ,
	label                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (linkid)
);
CREATE TABLE sysmaps_link_triggers (
	linktriggerid            number(20)                                NOT NULL,
	linkid                   number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	drawtype                 number(10)      DEFAULT '0'               NOT NULL,
	color                    nvarchar2(6)    DEFAULT '000000'          ,
	PRIMARY KEY (linktriggerid)
);
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);
CREATE TABLE sysmap_element_url (
	sysmapelementurlid       number(20)                                NOT NULL,
	selementid               number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (sysmapelementurlid)
);
CREATE UNIQUE INDEX sysmap_element_url_1 ON sysmap_element_url (selementid,name);
CREATE TABLE sysmap_url (
	sysmapurlid              number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	elementtype              number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapurlid)
);
CREATE UNIQUE INDEX sysmap_url_1 ON sysmap_url (sysmapid,name);
CREATE TABLE maintenances_hosts (
	maintenance_hostid       number(20)                                NOT NULL,
	maintenanceid            number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	PRIMARY KEY (maintenance_hostid)
);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
CREATE TABLE maintenances_groups (
	maintenance_groupid      number(20)                                NOT NULL,
	maintenanceid            number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (maintenance_groupid)
);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
CREATE TABLE timeperiods (
	timeperiodid             number(20)                                NOT NULL,
	timeperiod_type          number(10)      DEFAULT '0'               NOT NULL,
	every                    number(10)      DEFAULT '0'               NOT NULL,
	month                    number(10)      DEFAULT '0'               NOT NULL,
	dayofweek                number(10)      DEFAULT '0'               NOT NULL,
	day                      number(10)      DEFAULT '0'               NOT NULL,
	start_time               number(10)      DEFAULT '0'               NOT NULL,
	period                   number(10)      DEFAULT '0'               NOT NULL,
	start_date               number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (timeperiodid)
);
CREATE TABLE maintenances_windows (
	maintenance_timeperiodid number(20)                                NOT NULL,
	maintenanceid            number(20)                                NOT NULL,
	timeperiodid             number(20)                                NOT NULL,
	PRIMARY KEY (maintenance_timeperiodid)
);
CREATE UNIQUE INDEX maintenances_windows_1 ON maintenances_windows (maintenanceid,timeperiodid);
CREATE TABLE regexps (
	regexpid                 number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	test_string              nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (regexpid)
);
CREATE INDEX regexps_1 ON regexps (name);
CREATE TABLE expressions (
	expressionid             number(20)                                NOT NULL,
	regexpid                 number(20)                                NOT NULL,
	expression               nvarchar2(255)  DEFAULT ''                ,
	expression_type          number(10)      DEFAULT '0'               NOT NULL,
	exp_delimiter            nvarchar2(1)    DEFAULT ''                ,
	case_sensitive           number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (expressionid)
);
CREATE INDEX expressions_1 ON expressions (regexpid);
CREATE TABLE nodes (
	nodeid                   number(10)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT '0'               ,
	ip                       nvarchar2(39)   DEFAULT ''                ,
	port                     number(10)      DEFAULT '10051'           NOT NULL,
	nodetype                 number(10)      DEFAULT '0'               NOT NULL,
	masterid                 number(10)                                NULL,
	PRIMARY KEY (nodeid)
);
CREATE TABLE node_cksum (
	nodeid                   number(10)                                NOT NULL,
	tablename                nvarchar2(64)   DEFAULT ''                ,
	recordid                 number(20)                                NOT NULL,
	cksumtype                number(10)      DEFAULT '0'               NOT NULL,
	cksum                    nclob           DEFAULT ''                ,
	sync                     nvarchar2(128)  DEFAULT ''                
);
CREATE INDEX node_cksum_1 ON node_cksum (nodeid,cksumtype,tablename,recordid);
CREATE TABLE ids (
	nodeid                   number(10)                                NOT NULL,
	table_name               nvarchar2(64)   DEFAULT ''                ,
	field_name               nvarchar2(64)   DEFAULT ''                ,
	nextid                   number(20)                                NOT NULL,
	PRIMARY KEY (nodeid,table_name,field_name)
);
CREATE TABLE alerts (
	alertid                  number(20)                                NOT NULL,
	actionid                 number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	userid                   number(20)                                NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	mediatypeid              number(20)                                NULL,
	sendto                   nvarchar2(100)  DEFAULT ''                ,
	subject                  nvarchar2(255)  DEFAULT ''                ,
	message                  nvarchar2(2048) DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	retries                  number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(128)  DEFAULT ''                ,
	nextcheck                number(10)      DEFAULT '0'               NOT NULL,
	esc_step                 number(10)      DEFAULT '0'               NOT NULL,
	alerttype                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (alertid)
);
CREATE INDEX alerts_1 ON alerts (actionid);
CREATE INDEX alerts_2 ON alerts (clock);
CREATE INDEX alerts_3 ON alerts (eventid);
CREATE INDEX alerts_4 ON alerts (status,retries);
CREATE INDEX alerts_5 ON alerts (mediatypeid);
CREATE INDEX alerts_6 ON alerts (userid);
CREATE TABLE history (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20,4)    DEFAULT '0.0000'          NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_1 ON history (itemid,clock);
CREATE TABLE history_sync (
	id                       number(20)                                NOT NULL,
	nodeid                   number(10)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20,4)    DEFAULT '0.0000'          NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_sync_1 ON history_sync (nodeid,id);
CREATE TABLE history_uint (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);
CREATE TABLE history_uint_sync (
	id                       number(20)                                NOT NULL,
	nodeid                   number(10)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_uint_sync_1 ON history_uint_sync (nodeid,id);
CREATE TABLE history_str (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_str_1 ON history_str (itemid,clock);
CREATE TABLE history_str_sync (
	id                       number(20)                                NOT NULL,
	nodeid                   number(10)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_str_sync_1 ON history_str_sync (nodeid,id);
CREATE TABLE history_log (
	id                       number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	timestamp                number(10)      DEFAULT '0'               NOT NULL,
	source                   nvarchar2(64)   DEFAULT ''                ,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	logeventid               number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_log_1 ON history_log (itemid,clock);
CREATE UNIQUE INDEX history_log_2 ON history_log (itemid,id);
CREATE TABLE history_text (
	id                       number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX history_text_1 ON history_text (itemid,clock);
CREATE UNIQUE INDEX history_text_2 ON history_text (itemid,id);
CREATE TABLE proxy_history (
	id                       number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	timestamp                number(10)      DEFAULT '0'               NOT NULL,
	source                   nvarchar2(64)   DEFAULT ''                ,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	logeventid               number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_history_1 ON proxy_history (clock);
CREATE TABLE proxy_dhistory (
	id                       number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	druleid                  number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	ip                       nvarchar2(39)   DEFAULT ''                ,
	port                     number(10)      DEFAULT '0'               NOT NULL,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	dcheckid                 number(20)                                NULL,
	dns                      nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);
CREATE TABLE events (
	eventid                  number(20)                                NOT NULL,
	source                   number(10)      DEFAULT '0'               NOT NULL,
	object                   number(10)      DEFAULT '0'               NOT NULL,
	objectid                 number(20)      DEFAULT '0'               NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(10)      DEFAULT '0'               NOT NULL,
	acknowledged             number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	value_changed            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX events_1 ON events (object,objectid,eventid);
CREATE INDEX events_2 ON events (clock);
CREATE TABLE trends (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	num                      number(10)      DEFAULT '0'               NOT NULL,
	value_min                number(20,4)    DEFAULT '0.0000'          NOT NULL,
	value_avg                number(20,4)    DEFAULT '0.0000'          NOT NULL,
	value_max                number(20,4)    DEFAULT '0.0000'          NOT NULL,
	PRIMARY KEY (itemid,clock)
);
CREATE TABLE trends_uint (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	num                      number(10)      DEFAULT '0'               NOT NULL,
	value_min                number(20)      DEFAULT '0'               NOT NULL,
	value_avg                number(20)      DEFAULT '0'               NOT NULL,
	value_max                number(20)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock)
);
CREATE TABLE acknowledges (
	acknowledgeid            number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	message                  nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (acknowledgeid)
);
CREATE INDEX acknowledges_1 ON acknowledges (userid);
CREATE INDEX acknowledges_2 ON acknowledges (eventid);
CREATE INDEX acknowledges_3 ON acknowledges (clock);
CREATE TABLE auditlog (
	auditid                  number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	action                   number(10)      DEFAULT '0'               NOT NULL,
	resourcetype             number(10)      DEFAULT '0'               NOT NULL,
	details                  nvarchar2(128)  DEFAULT '0'               ,
	ip                       nvarchar2(39)   DEFAULT ''                ,
	resourceid               number(20)      DEFAULT '0'               NOT NULL,
	resourcename             nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (auditid)
);
CREATE INDEX auditlog_1 ON auditlog (userid,clock);
CREATE INDEX auditlog_2 ON auditlog (clock);
CREATE TABLE auditlog_details (
	auditdetailid            number(20)                                NOT NULL,
	auditid                  number(20)                                NOT NULL,
	table_name               nvarchar2(64)   DEFAULT ''                ,
	field_name               nvarchar2(64)   DEFAULT ''                ,
	oldvalue                 nvarchar2(2048) DEFAULT ''                ,
	newvalue                 nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (auditdetailid)
);
CREATE INDEX auditlog_details_1 ON auditlog_details (auditid);
CREATE TABLE service_alarms (
	servicealarmid           number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (servicealarmid)
);
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);
CREATE INDEX service_alarms_2 ON service_alarms (clock);
CREATE TABLE autoreg_host (
	autoreg_hostid           number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	host                     nvarchar2(64)   DEFAULT ''                ,
	listen_ip                nvarchar2(39)   DEFAULT ''                ,
	listen_port              number(10)      DEFAULT '0'               NOT NULL,
	listen_dns               nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (autoreg_hostid)
);
CREATE INDEX autoreg_host_1 ON autoreg_host (proxy_hostid,host);
CREATE TABLE proxy_autoreg_host (
	id                       number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	host                     nvarchar2(64)   DEFAULT ''                ,
	listen_ip                nvarchar2(39)   DEFAULT ''                ,
	listen_port              number(10)      DEFAULT '0'               NOT NULL,
	listen_dns               nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_autoreg_host_1 ON proxy_autoreg_host (clock);
CREATE TABLE dhosts (
	dhostid                  number(20)                                NOT NULL,
	druleid                  number(20)                                NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	lastup                   number(10)      DEFAULT '0'               NOT NULL,
	lastdown                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dhostid)
);
CREATE INDEX dhosts_1 ON dhosts (druleid);
CREATE TABLE dservices (
	dserviceid               number(20)                                NOT NULL,
	dhostid                  number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	port                     number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	lastup                   number(10)      DEFAULT '0'               NOT NULL,
	lastdown                 number(10)      DEFAULT '0'               NOT NULL,
	dcheckid                 number(20)                                NOT NULL,
	ip                       nvarchar2(39)   DEFAULT ''                ,
	dns                      nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (dserviceid)
);
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,type,key_,ip,port);
CREATE INDEX dservices_2 ON dservices (dhostid);
CREATE TABLE escalations (
	escalationid             number(20)                                NOT NULL,
	actionid                 number(20)                                NOT NULL,
	triggerid                number(20)                                NULL,
	eventid                  number(20)                                NULL,
	r_eventid                number(20)                                NULL,
	nextcheck                number(10)      DEFAULT '0'               NOT NULL,
	esc_step                 number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (escalationid)
);
CREATE INDEX escalations_1 ON escalations (actionid,triggerid);
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
CREATE UNIQUE INDEX graph_discovery_1 ON graph_discovery (graphid,parent_graphid);
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
CREATE TABLE housekeeper (
	housekeeperid            number(20)                                NOT NULL,
	tablename                nvarchar2(64)   DEFAULT ''                ,
	field                    nvarchar2(64)   DEFAULT ''                ,
	value                    number(20)                                NOT NULL,
	PRIMARY KEY (housekeeperid)
);
CREATE TABLE images (
	imageid                  number(20)                                NOT NULL,
	imagetype                number(10)      DEFAULT '0'               NOT NULL,
	name                     nvarchar2(64)   DEFAULT '0'               ,
	image                    blob            DEFAULT ''                NOT NULL,
	PRIMARY KEY (imageid)
);
CREATE INDEX images_1 ON images (imagetype,name);
CREATE TABLE item_discovery (
	itemdiscoveryid          number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	parent_itemid            number(20)                                NOT NULL,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemdiscoveryid)
);
CREATE UNIQUE INDEX item_discovery_1 ON item_discovery (itemid,parent_itemid);
CREATE TABLE profiles (
	profileid                number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	idx                      nvarchar2(96)   DEFAULT ''                ,
	idx2                     number(20)      DEFAULT '0'               NOT NULL,
	value_id                 number(20)      DEFAULT '0'               NOT NULL,
	value_int                number(10)      DEFAULT '0'               NOT NULL,
	value_str                nvarchar2(255)  DEFAULT ''                ,
	source                   nvarchar2(96)   DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (profileid)
);
CREATE INDEX profiles_1 ON profiles (userid,idx,idx2);
CREATE INDEX profiles_2 ON profiles (userid,profileid);
CREATE TABLE sessions (
	sessionid                nvarchar2(32)   DEFAULT ''                ,
	userid                   number(20)                                NOT NULL,
	lastaccess               number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sessionid)
);
CREATE INDEX sessions_1 ON sessions (userid,status);
CREATE TABLE trigger_discovery (
	triggerdiscoveryid       number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	parent_triggerid         number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (triggerdiscoveryid)
);
CREATE UNIQUE INDEX trigger_discovery_1 ON trigger_discovery (triggerid,parent_triggerid);
CREATE TABLE user_history (
	userhistoryid            number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	title1                   nvarchar2(255)  DEFAULT ''                ,
	url1                     nvarchar2(255)  DEFAULT ''                ,
	title2                   nvarchar2(255)  DEFAULT ''                ,
	url2                     nvarchar2(255)  DEFAULT ''                ,
	title3                   nvarchar2(255)  DEFAULT ''                ,
	url3                     nvarchar2(255)  DEFAULT ''                ,
	title4                   nvarchar2(255)  DEFAULT ''                ,
	url4                     nvarchar2(255)  DEFAULT ''                ,
	title5                   nvarchar2(255)  DEFAULT ''                ,
	url5                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (userhistoryid)
);
CREATE UNIQUE INDEX user_history_1 ON user_history (userid);
CREATE SEQUENCE history_sync_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/
CREATE TRIGGER history_sync_tr
BEFORE INSERT ON history_sync
FOR EACH ROW
BEGIN
SELECT history_sync_seq.nextval INTO :new.id FROM dual;
END;
/
CREATE SEQUENCE history_uint_sync_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/
CREATE TRIGGER history_uint_sync_tr
BEFORE INSERT ON history_uint_sync
FOR EACH ROW
BEGIN
SELECT history_uint_sync_seq.nextval INTO :new.id FROM dual;
END;
/
CREATE SEQUENCE history_str_sync_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/
CREATE TRIGGER history_str_sync_tr
BEFORE INSERT ON history_str_sync
FOR EACH ROW
BEGIN
SELECT history_str_sync_seq.nextval INTO :new.id FROM dual;
END;
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
CREATE SEQUENCE proxy_dhistory_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/
CREATE TRIGGER proxy_dhistory_tr
BEFORE INSERT ON proxy_dhistory
FOR EACH ROW
BEGIN
SELECT proxy_dhistory_seq.nextval INTO :new.id FROM dual;
END;
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
SELECT proxy_autoreg_host_seq.nextval INTO :new.id FROM dual;
END;
/
ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid);
ALTER TABLE screens ADD CONSTRAINT c_screens_1 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE screens_items ADD CONSTRAINT c_screens_items_1 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE slides ADD CONSTRAINT c_slides_1 FOREIGN KEY (slideshowid) REFERENCES slideshows (slideshowid) ON DELETE CASCADE;
ALTER TABLE slides ADD CONSTRAINT c_slides_2 FOREIGN KEY (screenid) REFERENCES screens (screenid) ON DELETE CASCADE;
ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid) ON DELETE CASCADE;
ALTER TABLE applications ADD CONSTRAINT c_applications_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE applications ADD CONSTRAINT c_applications_2 FOREIGN KEY (templateid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE httptest ADD CONSTRAINT c_httptest_1 FOREIGN KEY (applicationid) REFERENCES applications (applicationid) ON DELETE CASCADE;
ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemaps (valuemapid);
ALTER TABLE items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid);
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
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_hosts ADD CONSTRAINT c_maintenances_hosts_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES groups (groupid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE;
ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE;
ALTER TABLE nodes ADD CONSTRAINT c_nodes_1 FOREIGN KEY (masterid) REFERENCES nodes (nodeid);
ALTER TABLE node_cksum ADD CONSTRAINT c_node_cksum_1 FOREIGN KEY (nodeid) REFERENCES nodes (nodeid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
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
ALTER TABLE graph_discovery ADD CONSTRAINT c_graph_discovery_2 FOREIGN KEY (parent_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE host_inventory ADD CONSTRAINT c_host_inventory_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE item_discovery ADD CONSTRAINT c_item_discovery_2 FOREIGN KEY (parent_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE profiles ADD CONSTRAINT c_profiles_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE sessions ADD CONSTRAINT c_sessions_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_discovery ADD CONSTRAINT c_trigger_discovery_2 FOREIGN KEY (parent_triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE user_history ADD CONSTRAINT c_user_history_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
