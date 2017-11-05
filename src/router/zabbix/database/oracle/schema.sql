CREATE TABLE users (
	userid                   number(20)                                NOT NULL,
	alias                    nvarchar2(100)  DEFAULT ''                ,
	name                     nvarchar2(100)  DEFAULT ''                ,
	surname                  nvarchar2(100)  DEFAULT ''                ,
	passwd                   nvarchar2(32)   DEFAULT ''                ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	autologin                number(10)      DEFAULT '0'               NOT NULL,
	autologout               nvarchar2(32)   DEFAULT '15m'             ,
	lang                     nvarchar2(5)    DEFAULT 'en_GB'           ,
	refresh                  nvarchar2(32)   DEFAULT '30s'             ,
	type                     number(10)      DEFAULT '1'               NOT NULL,
	theme                    nvarchar2(128)  DEFAULT 'default'         ,
	attempt_failed           number(10)      DEFAULT 0                 NOT NULL,
	attempt_ip               nvarchar2(39)   DEFAULT ''                ,
	attempt_clock            number(10)      DEFAULT 0                 NOT NULL,
	rows_per_page            number(10)      DEFAULT 50                NOT NULL,
	PRIMARY KEY (userid)
);
CREATE UNIQUE INDEX users_1 ON users (alias);
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
CREATE UNIQUE INDEX maintenances_2 ON maintenances (name);
CREATE TABLE hosts (
	hostid                   number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	host                     nvarchar2(128)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	disable_until            number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(2048) DEFAULT ''                ,
	available                number(10)      DEFAULT '0'               NOT NULL,
	errors_from              number(10)      DEFAULT '0'               NOT NULL,
	lastaccess               number(10)      DEFAULT '0'               NOT NULL,
	ipmi_authtype            number(10)      DEFAULT '-1'              NOT NULL,
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
	ipmi_error               nvarchar2(2048) DEFAULT ''                ,
	snmp_error               nvarchar2(2048) DEFAULT ''                ,
	jmx_disable_until        number(10)      DEFAULT '0'               NOT NULL,
	jmx_available            number(10)      DEFAULT '0'               NOT NULL,
	jmx_errors_from          number(10)      DEFAULT '0'               NOT NULL,
	jmx_error                nvarchar2(2048) DEFAULT ''                ,
	name                     nvarchar2(128)  DEFAULT ''                ,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	templateid               number(20)                                NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	tls_connect              number(10)      DEFAULT '1'               NOT NULL,
	tls_accept               number(10)      DEFAULT '1'               NOT NULL,
	tls_issuer               nvarchar2(1024) DEFAULT ''                ,
	tls_subject              nvarchar2(1024) DEFAULT ''                ,
	tls_psk_identity         nvarchar2(128)  DEFAULT ''                ,
	tls_psk                  nvarchar2(512)  DEFAULT ''                ,
	PRIMARY KEY (hostid)
);
CREATE INDEX hosts_1 ON hosts (host);
CREATE INDEX hosts_2 ON hosts (status);
CREATE INDEX hosts_3 ON hosts (proxy_hostid);
CREATE INDEX hosts_4 ON hosts (name);
CREATE INDEX hosts_5 ON hosts (maintenanceid);
CREATE TABLE groups (
	groupid                  number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	internal                 number(10)      DEFAULT '0'               NOT NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE INDEX groups_1 ON groups (name);
CREATE TABLE group_prototype (
	group_prototypeid        number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	groupid                  number(20)                                NULL,
	templateid               number(20)                                NULL,
	PRIMARY KEY (group_prototypeid)
);
CREATE INDEX group_prototype_1 ON group_prototype (hostid);
CREATE TABLE group_discovery (
	groupid                  number(20)                                NOT NULL,
	parent_group_prototypeid number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE TABLE screens (
	screenid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	hsize                    number(10)      DEFAULT '1'               NOT NULL,
	vsize                    number(10)      DEFAULT '1'               NOT NULL,
	templateid               number(20)                                NULL,
	userid                   number(20)                                NULL,
	private                  number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (screenid)
);
CREATE INDEX screens_1 ON screens (templateid);
CREATE TABLE screens_items (
	screenitemid             number(20)                                NOT NULL,
	screenid                 number(20)                                NOT NULL,
	resourcetype             number(10)      DEFAULT '0'               NOT NULL,
	resourceid               number(20)      DEFAULT '0'               NOT NULL,
	width                    number(10)      DEFAULT '320'             NOT NULL,
	height                   number(10)      DEFAULT '200'             NOT NULL,
	x                        number(10)      DEFAULT '0'               NOT NULL,
	y                        number(10)      DEFAULT '0'               NOT NULL,
	colspan                  number(10)      DEFAULT '1'               NOT NULL,
	rowspan                  number(10)      DEFAULT '1'               NOT NULL,
	elements                 number(10)      DEFAULT '25'              NOT NULL,
	valign                   number(10)      DEFAULT '0'               NOT NULL,
	halign                   number(10)      DEFAULT '0'               NOT NULL,
	style                    number(10)      DEFAULT '0'               NOT NULL,
	url                      nvarchar2(255)  DEFAULT ''                ,
	dynamic                  number(10)      DEFAULT '0'               NOT NULL,
	sort_triggers            number(10)      DEFAULT '0'               NOT NULL,
	application              nvarchar2(255)  DEFAULT ''                ,
	max_columns              number(10)      DEFAULT '3'               NOT NULL,
	PRIMARY KEY (screenitemid)
);
CREATE INDEX screens_items_1 ON screens_items (screenid);
CREATE TABLE screen_user (
	screenuserid             number(20)                                NOT NULL,
	screenid                 number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (screenuserid)
);
CREATE UNIQUE INDEX screen_user_1 ON screen_user (screenid,userid);
CREATE TABLE screen_usrgrp (
	screenusrgrpid           number(20)                                NOT NULL,
	screenid                 number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (screenusrgrpid)
);
CREATE UNIQUE INDEX screen_usrgrp_1 ON screen_usrgrp (screenid,usrgrpid);
CREATE TABLE slideshows (
	slideshowid              number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	delay                    nvarchar2(32)   DEFAULT '30s'             ,
	userid                   number(20)                                NOT NULL,
	private                  number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (slideshowid)
);
CREATE UNIQUE INDEX slideshows_1 ON slideshows (name);
CREATE TABLE slideshow_user (
	slideshowuserid          number(20)                                NOT NULL,
	slideshowid              number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (slideshowuserid)
);
CREATE UNIQUE INDEX slideshow_user_1 ON slideshow_user (slideshowid,userid);
CREATE TABLE slideshow_usrgrp (
	slideshowusrgrpid        number(20)                                NOT NULL,
	slideshowid              number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (slideshowusrgrpid)
);
CREATE UNIQUE INDEX slideshow_usrgrp_1 ON slideshow_usrgrp (slideshowid,usrgrpid);
CREATE TABLE slides (
	slideid                  number(20)                                NOT NULL,
	slideshowid              number(20)                                NOT NULL,
	screenid                 number(20)                                NOT NULL,
	step                     number(10)      DEFAULT '0'               NOT NULL,
	delay                    nvarchar2(32)   DEFAULT '0'               ,
	PRIMARY KEY (slideid)
);
CREATE INDEX slides_1 ON slides (slideshowid);
CREATE INDEX slides_2 ON slides (screenid);
CREATE TABLE drules (
	druleid                  number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	iprange                  nvarchar2(2048) DEFAULT ''                ,
	delay                    nvarchar2(255)  DEFAULT '1h'              ,
	nextcheck                number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (druleid)
);
CREATE INDEX drules_1 ON drules (proxy_hostid);
CREATE UNIQUE INDEX drules_2 ON drules (name);
CREATE TABLE dchecks (
	dcheckid                 number(20)                                NOT NULL,
	druleid                  number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	key_                     nvarchar2(512)  DEFAULT ''                ,
	snmp_community           nvarchar2(255)  DEFAULT ''                ,
	ports                    nvarchar2(255)  DEFAULT '0'               ,
	snmpv3_securityname      nvarchar2(64)   DEFAULT ''                ,
	snmpv3_securitylevel     number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_authpassphrase    nvarchar2(64)   DEFAULT ''                ,
	snmpv3_privpassphrase    nvarchar2(64)   DEFAULT ''                ,
	uniq                     number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_authprotocol      number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_privprotocol      number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_contextname       nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (dcheckid)
);
CREATE INDEX dchecks_1 ON dchecks (druleid);
CREATE TABLE applications (
	applicationid            number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (applicationid)
);
CREATE UNIQUE INDEX applications_2 ON applications (hostid,name);
CREATE TABLE httptest (
	httptestid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	applicationid            number(20)                                NULL,
	nextcheck                number(10)      DEFAULT '0'               NOT NULL,
	delay                    nvarchar2(255)  DEFAULT '1m'              ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	agent                    nvarchar2(255)  DEFAULT 'Zabbix'          ,
	authentication           number(10)      DEFAULT '0'               NOT NULL,
	http_user                nvarchar2(64)   DEFAULT ''                ,
	http_password            nvarchar2(64)   DEFAULT ''                ,
	hostid                   number(20)                                NOT NULL,
	templateid               number(20)                                NULL,
	http_proxy               nvarchar2(255)  DEFAULT ''                ,
	retries                  number(10)      DEFAULT '1'               NOT NULL,
	ssl_cert_file            nvarchar2(255)  DEFAULT ''                ,
	ssl_key_file             nvarchar2(255)  DEFAULT ''                ,
	ssl_key_password         nvarchar2(64)   DEFAULT ''                ,
	verify_peer              number(10)      DEFAULT '0'               NOT NULL,
	verify_host              number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httptestid)
);
CREATE INDEX httptest_1 ON httptest (applicationid);
CREATE UNIQUE INDEX httptest_2 ON httptest (hostid,name);
CREATE INDEX httptest_3 ON httptest (status);
CREATE INDEX httptest_4 ON httptest (templateid);
CREATE TABLE httpstep (
	httpstepid               number(20)                                NOT NULL,
	httptestid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	no                       number(10)      DEFAULT '0'               NOT NULL,
	url                      nvarchar2(2048) DEFAULT ''                ,
	timeout                  nvarchar2(255)  DEFAULT '15s'             ,
	posts                    nvarchar2(2048) DEFAULT ''                ,
	required                 nvarchar2(255)  DEFAULT ''                ,
	status_codes             nvarchar2(255)  DEFAULT ''                ,
	follow_redirects         number(10)      DEFAULT '1'               NOT NULL,
	retrieve_mode            number(10)      DEFAULT '0'               NOT NULL,
	post_type                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httpstepid)
);
CREATE INDEX httpstep_1 ON httpstep (httptestid);
CREATE TABLE interface (
	interfaceid              number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	main                     number(10)      DEFAULT '0'               NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	useip                    number(10)      DEFAULT '1'               NOT NULL,
	ip                       nvarchar2(64)   DEFAULT '127.0.0.1'       ,
	dns                      nvarchar2(64)   DEFAULT ''                ,
	port                     nvarchar2(64)   DEFAULT '10050'           ,
	bulk                     number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_1 ON interface (hostid,type);
CREATE INDEX interface_2 ON interface (ip,dns);
CREATE TABLE valuemaps (
	valuemapid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (valuemapid)
);
CREATE UNIQUE INDEX valuemaps_1 ON valuemaps (name);
CREATE TABLE items (
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	snmp_community           nvarchar2(64)   DEFAULT ''                ,
	snmp_oid                 nvarchar2(512)  DEFAULT ''                ,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	key_                     nvarchar2(255)  DEFAULT ''                ,
	delay                    nvarchar2(1024) DEFAULT '0'               ,
	history                  nvarchar2(255)  DEFAULT '90d'             ,
	trends                   nvarchar2(255)  DEFAULT '365d'            ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	value_type               number(10)      DEFAULT '0'               NOT NULL,
	trapper_hosts            nvarchar2(255)  DEFAULT ''                ,
	units                    nvarchar2(255)  DEFAULT ''                ,
	snmpv3_securityname      nvarchar2(64)   DEFAULT ''                ,
	snmpv3_securitylevel     number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_authpassphrase    nvarchar2(64)   DEFAULT ''                ,
	snmpv3_privpassphrase    nvarchar2(64)   DEFAULT ''                ,
	formula                  nvarchar2(255)  DEFAULT ''                ,
	error                    nvarchar2(2048) DEFAULT ''                ,
	lastlogsize              number(20)      DEFAULT '0'               NOT NULL,
	logtimefmt               nvarchar2(64)   DEFAULT ''                ,
	templateid               number(20)                                NULL,
	valuemapid               number(20)                                NULL,
	params                   nvarchar2(2048) DEFAULT ''                ,
	ipmi_sensor              nvarchar2(128)  DEFAULT ''                ,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	publickey                nvarchar2(64)   DEFAULT ''                ,
	privatekey               nvarchar2(64)   DEFAULT ''                ,
	mtime                    number(10)      DEFAULT '0'               NOT NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	interfaceid              number(20)                                NULL,
	port                     nvarchar2(64)   DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	inventory_link           number(10)      DEFAULT '0'               NOT NULL,
	lifetime                 nvarchar2(255)  DEFAULT '30d'             ,
	snmpv3_authprotocol      number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_privprotocol      number(10)      DEFAULT '0'               NOT NULL,
	state                    number(10)      DEFAULT '0'               NOT NULL,
	snmpv3_contextname       nvarchar2(255)  DEFAULT ''                ,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	jmx_endpoint             nvarchar2(255)  DEFAULT ''                ,
	master_itemid            number(20)                                NULL,
	PRIMARY KEY (itemid)
);
CREATE UNIQUE INDEX items_1 ON items (hostid,key_);
CREATE INDEX items_3 ON items (status);
CREATE INDEX items_4 ON items (templateid);
CREATE INDEX items_5 ON items (valuemapid);
CREATE INDEX items_6 ON items (interfaceid);
CREATE INDEX items_7 ON items (master_itemid);
CREATE TABLE httpstepitem (
	httpstepitemid           number(20)                                NOT NULL,
	httpstepid               number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httpstepitemid)
);
CREATE UNIQUE INDEX httpstepitem_1 ON httpstepitem (httpstepid,itemid);
CREATE INDEX httpstepitem_2 ON httpstepitem (itemid);
CREATE TABLE httptestitem (
	httptestitemid           number(20)                                NOT NULL,
	httptestid               number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (httptestitemid)
);
CREATE UNIQUE INDEX httptestitem_1 ON httptestitem (httptestid,itemid);
CREATE INDEX httptestitem_2 ON httptestitem (itemid);
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
	smtp_port                number(10)      DEFAULT '25'              NOT NULL,
	smtp_security            number(10)      DEFAULT '0'               NOT NULL,
	smtp_verify_peer         number(10)      DEFAULT '0'               NOT NULL,
	smtp_verify_host         number(10)      DEFAULT '0'               NOT NULL,
	smtp_authentication      number(10)      DEFAULT '0'               NOT NULL,
	exec_params              nvarchar2(255)  DEFAULT ''                ,
	maxsessions              number(10)      DEFAULT '1'               NOT NULL,
	maxattempts              number(10)      DEFAULT '3'               NOT NULL,
	attempt_interval         nvarchar2(32)   DEFAULT '10s'             ,
	PRIMARY KEY (mediatypeid)
);
CREATE UNIQUE INDEX media_type_1 ON media_type (description);
CREATE TABLE usrgrp (
	usrgrpid                 number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	gui_access               number(10)      DEFAULT '0'               NOT NULL,
	users_status             number(10)      DEFAULT '0'               NOT NULL,
	debug_mode               number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (usrgrpid)
);
CREATE UNIQUE INDEX usrgrp_1 ON usrgrp (name);
CREATE TABLE users_groups (
	id                       number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	PRIMARY KEY (id)
);
CREATE UNIQUE INDEX users_groups_1 ON users_groups (usrgrpid,userid);
CREATE INDEX users_groups_2 ON users_groups (userid);
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
	execute_on               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (scriptid)
);
CREATE INDEX scripts_1 ON scripts (usrgrpid);
CREATE INDEX scripts_2 ON scripts (groupid);
CREATE UNIQUE INDEX scripts_3 ON scripts (name);
CREATE TABLE actions (
	actionid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	eventsource              number(10)      DEFAULT '0'               NOT NULL,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	esc_period               nvarchar2(255)  DEFAULT '1h'              ,
	def_shortdata            nvarchar2(255)  DEFAULT ''                ,
	def_longdata             nvarchar2(2048) DEFAULT ''                ,
	r_shortdata              nvarchar2(255)  DEFAULT ''                ,
	r_longdata               nvarchar2(2048) DEFAULT ''                ,
	formula                  nvarchar2(255)  DEFAULT ''                ,
	maintenance_mode         number(10)      DEFAULT '1'               NOT NULL,
	ack_shortdata            nvarchar2(255)  DEFAULT ''                ,
	ack_longdata             nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (actionid)
);
CREATE INDEX actions_1 ON actions (eventsource,status);
CREATE UNIQUE INDEX actions_2 ON actions (name);
CREATE TABLE operations (
	operationid              number(20)                                NOT NULL,
	actionid                 number(20)                                NOT NULL,
	operationtype            number(10)      DEFAULT '0'               NOT NULL,
	esc_period               nvarchar2(255)  DEFAULT '0'               ,
	esc_step_from            number(10)      DEFAULT '1'               NOT NULL,
	esc_step_to              number(10)      DEFAULT '1'               NOT NULL,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	recovery                 number(10)      DEFAULT '0'               NOT NULL,
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
CREATE INDEX opmessage_1 ON opmessage (mediatypeid);
CREATE TABLE opmessage_grp (
	opmessage_grpid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	PRIMARY KEY (opmessage_grpid)
);
CREATE UNIQUE INDEX opmessage_grp_1 ON opmessage_grp (operationid,usrgrpid);
CREATE INDEX opmessage_grp_2 ON opmessage_grp (usrgrpid);
CREATE TABLE opmessage_usr (
	opmessage_usrid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	PRIMARY KEY (opmessage_usrid)
);
CREATE UNIQUE INDEX opmessage_usr_1 ON opmessage_usr (operationid,userid);
CREATE INDEX opmessage_usr_2 ON opmessage_usr (userid);
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
CREATE INDEX opcommand_1 ON opcommand (scriptid);
CREATE TABLE opcommand_hst (
	opcommand_hstid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	hostid                   number(20)                                NULL,
	PRIMARY KEY (opcommand_hstid)
);
CREATE INDEX opcommand_hst_1 ON opcommand_hst (operationid);
CREATE INDEX opcommand_hst_2 ON opcommand_hst (hostid);
CREATE TABLE opcommand_grp (
	opcommand_grpid          number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (opcommand_grpid)
);
CREATE INDEX opcommand_grp_1 ON opcommand_grp (operationid);
CREATE INDEX opcommand_grp_2 ON opcommand_grp (groupid);
CREATE TABLE opgroup (
	opgroupid                number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (opgroupid)
);
CREATE UNIQUE INDEX opgroup_1 ON opgroup (operationid,groupid);
CREATE INDEX opgroup_2 ON opgroup (groupid);
CREATE TABLE optemplate (
	optemplateid             number(20)                                NOT NULL,
	operationid              number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (optemplateid)
);
CREATE UNIQUE INDEX optemplate_1 ON optemplate (operationid,templateid);
CREATE INDEX optemplate_2 ON optemplate (templateid);
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
	value2                   nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (conditionid)
);
CREATE INDEX conditions_1 ON conditions (actionid);
CREATE TABLE config (
	configid                 number(20)                                NOT NULL,
	refresh_unsupported      nvarchar2(32)   DEFAULT '10m'             ,
	work_period              nvarchar2(255)  DEFAULT '1-5,09:00-18:00' ,
	alert_usrgrpid           number(20)                                NULL,
	event_ack_enable         number(10)      DEFAULT '1'               NOT NULL,
	event_expire             nvarchar2(32)   DEFAULT '1w'              ,
	event_show_max           number(10)      DEFAULT '100'             NOT NULL,
	default_theme            nvarchar2(128)  DEFAULT 'blue-theme'      ,
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
	severity_color_0         nvarchar2(6)    DEFAULT '97AAB3'          ,
	severity_color_1         nvarchar2(6)    DEFAULT '7499FF'          ,
	severity_color_2         nvarchar2(6)    DEFAULT 'FFC859'          ,
	severity_color_3         nvarchar2(6)    DEFAULT 'FFA059'          ,
	severity_color_4         nvarchar2(6)    DEFAULT 'E97659'          ,
	severity_color_5         nvarchar2(6)    DEFAULT 'E45959'          ,
	severity_name_0          nvarchar2(32)   DEFAULT 'Not classified'  ,
	severity_name_1          nvarchar2(32)   DEFAULT 'Information'     ,
	severity_name_2          nvarchar2(32)   DEFAULT 'Warning'         ,
	severity_name_3          nvarchar2(32)   DEFAULT 'Average'         ,
	severity_name_4          nvarchar2(32)   DEFAULT 'High'            ,
	severity_name_5          nvarchar2(32)   DEFAULT 'Disaster'        ,
	ok_period                nvarchar2(32)   DEFAULT '30m'             ,
	blink_period             nvarchar2(32)   DEFAULT '30m'             ,
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
	hk_events_mode           number(10)      DEFAULT '1'               NOT NULL,
	hk_events_trigger        nvarchar2(32)   DEFAULT '365d'            ,
	hk_events_internal       nvarchar2(32)   DEFAULT '1d'              ,
	hk_events_discovery      nvarchar2(32)   DEFAULT '1d'              ,
	hk_events_autoreg        nvarchar2(32)   DEFAULT '1d'              ,
	hk_services_mode         number(10)      DEFAULT '1'               NOT NULL,
	hk_services              nvarchar2(32)   DEFAULT '365d'            ,
	hk_audit_mode            number(10)      DEFAULT '1'               NOT NULL,
	hk_audit                 nvarchar2(32)   DEFAULT '365d'            ,
	hk_sessions_mode         number(10)      DEFAULT '1'               NOT NULL,
	hk_sessions              nvarchar2(32)   DEFAULT '365d'            ,
	hk_history_mode          number(10)      DEFAULT '1'               NOT NULL,
	hk_history_global        number(10)      DEFAULT '0'               NOT NULL,
	hk_history               nvarchar2(32)   DEFAULT '90d'             ,
	hk_trends_mode           number(10)      DEFAULT '1'               NOT NULL,
	hk_trends_global         number(10)      DEFAULT '0'               NOT NULL,
	hk_trends                nvarchar2(32)   DEFAULT '365d'            ,
	default_inventory_mode   number(10)      DEFAULT '-1'              NOT NULL,
	PRIMARY KEY (configid)
);
CREATE INDEX config_1 ON config (alert_usrgrpid);
CREATE INDEX config_2 ON config (discovery_groupid);
CREATE TABLE triggers (
	triggerid                number(20)                                NOT NULL,
	expression               nvarchar2(2048) DEFAULT ''                ,
	description              nvarchar2(255)  DEFAULT ''                ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	value                    number(10)      DEFAULT '0'               NOT NULL,
	priority                 number(10)      DEFAULT '0'               NOT NULL,
	lastchange               number(10)      DEFAULT '0'               NOT NULL,
	comments                 nvarchar2(2048) DEFAULT ''                ,
	error                    nvarchar2(2048) DEFAULT ''                ,
	templateid               number(20)                                NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	state                    number(10)      DEFAULT '0'               NOT NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	recovery_mode            number(10)      DEFAULT '0'               NOT NULL,
	recovery_expression      nvarchar2(2048) DEFAULT ''                ,
	correlation_mode         number(10)      DEFAULT '0'               NOT NULL,
	correlation_tag          nvarchar2(255)  DEFAULT ''                ,
	manual_close             number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX triggers_1 ON triggers (status);
CREATE INDEX triggers_2 ON triggers (value,lastchange);
CREATE INDEX triggers_3 ON triggers (templateid);
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
	width                    number(10)      DEFAULT '900'             NOT NULL,
	height                   number(10)      DEFAULT '200'             NOT NULL,
	yaxismin                 number(20,4)    DEFAULT '0'               NOT NULL,
	yaxismax                 number(20,4)    DEFAULT '100'             NOT NULL,
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
CREATE INDEX graphs_1 ON graphs (name);
CREATE INDEX graphs_2 ON graphs (templateid);
CREATE INDEX graphs_3 ON graphs (ymin_itemid);
CREATE INDEX graphs_4 ON graphs (ymax_itemid);
CREATE TABLE graphs_items (
	gitemid                  number(20)                                NOT NULL,
	graphid                  number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	drawtype                 number(10)      DEFAULT '0'               NOT NULL,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	color                    nvarchar2(6)    DEFAULT '009600'          ,
	yaxisside                number(10)      DEFAULT '0'               NOT NULL,
	calc_fnc                 number(10)      DEFAULT '2'               NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (gitemid)
);
CREATE INDEX graphs_items_1 ON graphs_items (itemid);
CREATE INDEX graphs_items_2 ON graphs_items (graphid);
CREATE TABLE graph_theme (
	graphthemeid             number(20)                                NOT NULL,
	theme                    nvarchar2(64)   DEFAULT ''                ,
	backgroundcolor          nvarchar2(6)    DEFAULT ''                ,
	graphcolor               nvarchar2(6)    DEFAULT ''                ,
	gridcolor                nvarchar2(6)    DEFAULT ''                ,
	maingridcolor            nvarchar2(6)    DEFAULT ''                ,
	gridbordercolor          nvarchar2(6)    DEFAULT ''                ,
	textcolor                nvarchar2(6)    DEFAULT ''                ,
	highlightcolor           nvarchar2(6)    DEFAULT ''                ,
	leftpercentilecolor      nvarchar2(6)    DEFAULT ''                ,
	rightpercentilecolor     nvarchar2(6)    DEFAULT ''                ,
	nonworktimecolor         nvarchar2(6)    DEFAULT ''                ,
	PRIMARY KEY (graphthemeid)
);
CREATE UNIQUE INDEX graph_theme_1 ON graph_theme (theme);
CREATE TABLE globalmacro (
	globalmacroid            number(20)                                NOT NULL,
	macro                    nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (globalmacroid)
);
CREATE UNIQUE INDEX globalmacro_1 ON globalmacro (macro);
CREATE TABLE hostmacro (
	hostmacroid              number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	macro                    nvarchar2(255)  DEFAULT ''                ,
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
	period                   nvarchar2(1024) DEFAULT '1-7,00:00-24:00' ,
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
CREATE INDEX services_links_1 ON services_links (servicedownid);
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);
CREATE TABLE services_times (
	timeid                   number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	ts_from                  number(10)      DEFAULT '0'               NOT NULL,
	ts_to                    number(10)      DEFAULT '0'               NOT NULL,
	note                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (timeid)
);
CREATE INDEX services_times_1 ON services_times (serviceid,type,ts_from,ts_to);
CREATE TABLE icon_map (
	iconmapid                number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	default_iconid           number(20)                                NOT NULL,
	PRIMARY KEY (iconmapid)
);
CREATE UNIQUE INDEX icon_map_1 ON icon_map (name);
CREATE INDEX icon_map_2 ON icon_map (default_iconid);
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
CREATE INDEX icon_mapping_2 ON icon_mapping (iconid);
CREATE TABLE sysmaps (
	sysmapid                 number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	width                    number(10)      DEFAULT '600'             NOT NULL,
	height                   number(10)      DEFAULT '400'             NOT NULL,
	backgroundid             number(20)                                NULL,
	label_type               number(10)      DEFAULT '2'               NOT NULL,
	label_location           number(10)      DEFAULT '0'               NOT NULL,
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
	severity_min             number(10)      DEFAULT '0'               NOT NULL,
	userid                   number(20)                                NOT NULL,
	private                  number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (sysmapid)
);
CREATE UNIQUE INDEX sysmaps_1 ON sysmaps (name);
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);
CREATE TABLE sysmaps_elements (
	selementid               number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	elementid                number(20)      DEFAULT '0'               NOT NULL,
	elementtype              number(10)      DEFAULT '0'               NOT NULL,
	iconid_off               number(20)                                NULL,
	iconid_on                number(20)                                NULL,
	label                    nvarchar2(2048) DEFAULT ''                ,
	label_location           number(10)      DEFAULT '-1'              NOT NULL,
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
	application              nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (selementid)
);
CREATE INDEX sysmaps_elements_1 ON sysmaps_elements (sysmapid);
CREATE INDEX sysmaps_elements_2 ON sysmaps_elements (iconid_off);
CREATE INDEX sysmaps_elements_3 ON sysmaps_elements (iconid_on);
CREATE INDEX sysmaps_elements_4 ON sysmaps_elements (iconid_disabled);
CREATE INDEX sysmaps_elements_5 ON sysmaps_elements (iconid_maintenance);
CREATE TABLE sysmaps_links (
	linkid                   number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	selementid1              number(20)                                NOT NULL,
	selementid2              number(20)                                NOT NULL,
	drawtype                 number(10)      DEFAULT '0'               NOT NULL,
	color                    nvarchar2(6)    DEFAULT '000000'          ,
	label                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (linkid)
);
CREATE INDEX sysmaps_links_1 ON sysmaps_links (sysmapid);
CREATE INDEX sysmaps_links_2 ON sysmaps_links (selementid1);
CREATE INDEX sysmaps_links_3 ON sysmaps_links (selementid2);
CREATE TABLE sysmaps_link_triggers (
	linktriggerid            number(20)                                NOT NULL,
	linkid                   number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	drawtype                 number(10)      DEFAULT '0'               NOT NULL,
	color                    nvarchar2(6)    DEFAULT '000000'          ,
	PRIMARY KEY (linktriggerid)
);
CREATE UNIQUE INDEX sysmaps_link_triggers_1 ON sysmaps_link_triggers (linkid,triggerid);
CREATE INDEX sysmaps_link_triggers_2 ON sysmaps_link_triggers (triggerid);
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
CREATE TABLE sysmap_user (
	sysmapuserid             number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (sysmapuserid)
);
CREATE UNIQUE INDEX sysmap_user_1 ON sysmap_user (sysmapid,userid);
CREATE TABLE sysmap_usrgrp (
	sysmapusrgrpid           number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (sysmapusrgrpid)
);
CREATE UNIQUE INDEX sysmap_usrgrp_1 ON sysmap_usrgrp (sysmapid,usrgrpid);
CREATE TABLE maintenances_hosts (
	maintenance_hostid       number(20)                                NOT NULL,
	maintenanceid            number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	PRIMARY KEY (maintenance_hostid)
);
CREATE UNIQUE INDEX maintenances_hosts_1 ON maintenances_hosts (maintenanceid,hostid);
CREATE INDEX maintenances_hosts_2 ON maintenances_hosts (hostid);
CREATE TABLE maintenances_groups (
	maintenance_groupid      number(20)                                NOT NULL,
	maintenanceid            number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (maintenance_groupid)
);
CREATE UNIQUE INDEX maintenances_groups_1 ON maintenances_groups (maintenanceid,groupid);
CREATE INDEX maintenances_groups_2 ON maintenances_groups (groupid);
CREATE TABLE timeperiods (
	timeperiodid             number(20)                                NOT NULL,
	timeperiod_type          number(10)      DEFAULT '0'               NOT NULL,
	every                    number(10)      DEFAULT '1'               NOT NULL,
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
CREATE INDEX maintenances_windows_2 ON maintenances_windows (timeperiodid);
CREATE TABLE regexps (
	regexpid                 number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	test_string              nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (regexpid)
);
CREATE UNIQUE INDEX regexps_1 ON regexps (name);
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
CREATE TABLE ids (
	table_name               nvarchar2(64)   DEFAULT ''                ,
	field_name               nvarchar2(64)   DEFAULT ''                ,
	nextid                   number(20)                                NOT NULL,
	PRIMARY KEY (table_name,field_name)
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
	message                  nclob           DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	retries                  number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(2048) DEFAULT ''                ,
	esc_step                 number(10)      DEFAULT '0'               NOT NULL,
	alerttype                number(10)      DEFAULT '0'               NOT NULL,
	p_eventid                number(20)                                NULL,
	acknowledgeid            number(20)                                NULL,
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
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20,4)    DEFAULT '0.0000'          NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_1 ON history (itemid,clock);
CREATE TABLE history_uint (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_uint_1 ON history_uint (itemid,clock);
CREATE TABLE history_str (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_str_1 ON history_str (itemid,clock);
CREATE TABLE history_log (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	timestamp                number(10)      DEFAULT '0'               NOT NULL,
	source                   nvarchar2(64)   DEFAULT ''                ,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	logeventid               number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_log_1 ON history_log (itemid,clock);
CREATE TABLE history_text (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL
);
CREATE INDEX history_text_1 ON history_text (itemid,clock);
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
	state                    number(10)      DEFAULT '0'               NOT NULL,
	lastlogsize              number(20)      DEFAULT '0'               NOT NULL,
	mtime                    number(10)      DEFAULT '0'               NOT NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_history_1 ON proxy_history (clock);
CREATE TABLE proxy_dhistory (
	id                       number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	druleid                  number(20)                                NOT NULL,
	ip                       nvarchar2(39)   DEFAULT ''                ,
	port                     number(10)      DEFAULT '0'               NOT NULL,
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
	PRIMARY KEY (eventid)
);
CREATE INDEX events_1 ON events (source,object,objectid,clock);
CREATE INDEX events_2 ON events (source,object,clock);
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
	action                   number(10)      DEFAULT '0'               NOT NULL,
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
	host_metadata            nvarchar2(255)  DEFAULT ''                ,
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
	host_metadata            nvarchar2(255)  DEFAULT ''                ,
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
CREATE UNIQUE INDEX dservices_1 ON dservices (dcheckid,ip,port);
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
	itemid                   number(20)                                NULL,
	acknowledgeid            number(20)                                NULL,
	PRIMARY KEY (escalationid)
);
CREATE UNIQUE INDEX escalations_1 ON escalations (actionid,triggerid,itemid,escalationid);
CREATE TABLE globalvars (
	globalvarid              number(20)                                NOT NULL,
	snmp_lastsize            number(20)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
);
CREATE TABLE graph_discovery (
	graphid                  number(20)                                NOT NULL,
	parent_graphid           number(20)                                NOT NULL,
	PRIMARY KEY (graphid)
);
CREATE INDEX graph_discovery_1 ON graph_discovery (parent_graphid);
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
CREATE UNIQUE INDEX images_1 ON images (name);
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
CREATE INDEX item_discovery_2 ON item_discovery (parent_itemid);
CREATE TABLE host_discovery (
	hostid                   number(20)                                NOT NULL,
	parent_hostid            number(20)                                NULL,
	parent_itemid            number(20)                                NULL,
	host                     nvarchar2(64)   DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE TABLE interface_discovery (
	interfaceid              number(20)                                NOT NULL,
	parent_interfaceid       number(20)                                NOT NULL,
	PRIMARY KEY (interfaceid)
);
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
CREATE INDEX sessions_1 ON sessions (userid,status,lastaccess);
CREATE TABLE trigger_discovery (
	triggerid                number(20)                                NOT NULL,
	parent_triggerid         number(20)                                NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX trigger_discovery_1 ON trigger_discovery (parent_triggerid);
CREATE TABLE application_template (
	application_templateid   number(20)                                NOT NULL,
	applicationid            number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (application_templateid)
);
CREATE UNIQUE INDEX application_template_1 ON application_template (applicationid,templateid);
CREATE INDEX application_template_2 ON application_template (templateid);
CREATE TABLE item_condition (
	item_conditionid         number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	operator                 number(10)      DEFAULT '8'               NOT NULL,
	macro                    nvarchar2(64)   DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (item_conditionid)
);
CREATE INDEX item_condition_1 ON item_condition (itemid);
CREATE TABLE application_prototype (
	application_prototypeid  number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	templateid               number(20)                                NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (application_prototypeid)
);
CREATE INDEX application_prototype_1 ON application_prototype (itemid);
CREATE INDEX application_prototype_2 ON application_prototype (templateid);
CREATE TABLE item_application_prototype (
	item_application_prototypeid number(20)                                NOT NULL,
	application_prototypeid  number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	PRIMARY KEY (item_application_prototypeid)
);
CREATE UNIQUE INDEX item_application_prototype_1 ON item_application_prototype (application_prototypeid,itemid);
CREATE INDEX item_application_prototype_2 ON item_application_prototype (itemid);
CREATE TABLE application_discovery (
	application_discoveryid  number(20)                                NOT NULL,
	applicationid            number(20)                                NOT NULL,
	application_prototypeid  number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (application_discoveryid)
);
CREATE INDEX application_discovery_1 ON application_discovery (applicationid);
CREATE INDEX application_discovery_2 ON application_discovery (application_prototypeid);
CREATE TABLE opinventory (
	operationid              number(20)                                NOT NULL,
	inventory_mode           number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (operationid)
);
CREATE TABLE trigger_tag (
	triggertagid             number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (triggertagid)
);
CREATE INDEX trigger_tag_1 ON trigger_tag (triggerid);
CREATE TABLE event_tag (
	eventtagid               number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (eventtagid)
);
CREATE INDEX event_tag_1 ON event_tag (eventid);
CREATE TABLE problem (
	eventid                  number(20)                                NOT NULL,
	source                   number(10)      DEFAULT '0'               NOT NULL,
	object                   number(10)      DEFAULT '0'               NOT NULL,
	objectid                 number(20)      DEFAULT '0'               NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	r_eventid                number(20)                                NULL,
	r_clock                  number(10)      DEFAULT '0'               NOT NULL,
	r_ns                     number(10)      DEFAULT '0'               NOT NULL,
	correlationid            number(20)                                NULL,
	userid                   number(20)                                NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX problem_1 ON problem (source,object,objectid);
CREATE INDEX problem_2 ON problem (r_clock);
CREATE TABLE problem_tag (
	problemtagid             number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (problemtagid)
);
CREATE INDEX problem_tag_1 ON problem_tag (eventid);
CREATE INDEX problem_tag_2 ON problem_tag (tag,value);
CREATE TABLE event_recovery (
	eventid                  number(20)                                NOT NULL,
	r_eventid                number(20)                                NOT NULL,
	c_eventid                number(20)                                NULL,
	correlationid            number(20)                                NULL,
	userid                   number(20)                                NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX event_recovery_1 ON event_recovery (r_eventid);
CREATE INDEX event_recovery_2 ON event_recovery (c_eventid);
CREATE TABLE correlation (
	correlationid            number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	formula                  nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (correlationid)
);
CREATE INDEX correlation_1 ON correlation (status);
CREATE UNIQUE INDEX correlation_2 ON correlation (name);
CREATE TABLE corr_condition (
	corr_conditionid         number(20)                                NOT NULL,
	correlationid            number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE INDEX corr_condition_1 ON corr_condition (correlationid);
CREATE TABLE corr_condition_tag (
	corr_conditionid         number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (corr_conditionid)
);
CREATE TABLE corr_condition_group (
	corr_conditionid         number(20)                                NOT NULL,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	groupid                  number(20)                                NOT NULL,
	PRIMARY KEY (corr_conditionid)
);
CREATE INDEX corr_condition_group_1 ON corr_condition_group (groupid);
CREATE TABLE corr_condition_tagpair (
	corr_conditionid         number(20)                                NOT NULL,
	oldtag                   nvarchar2(255)  DEFAULT ''                ,
	newtag                   nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (corr_conditionid)
);
CREATE TABLE corr_condition_tagvalue (
	corr_conditionid         number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (corr_conditionid)
);
CREATE TABLE corr_operation (
	corr_operationid         number(20)                                NOT NULL,
	correlationid            number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (corr_operationid)
);
CREATE INDEX corr_operation_1 ON corr_operation (correlationid);
CREATE TABLE task (
	taskid                   number(20)                                NOT NULL,
	type                     number(10)                                NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	ttl                      number(10)      DEFAULT '0'               NOT NULL,
	proxy_hostid             number(20)                                NULL,
	PRIMARY KEY (taskid)
);
CREATE INDEX task_1 ON task (status,proxy_hostid);
CREATE TABLE task_close_problem (
	taskid                   number(20)                                NOT NULL,
	acknowledgeid            number(20)                                NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE item_preproc (
	item_preprocid           number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	step                     number(10)      DEFAULT '0'               NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	params                   nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (item_preprocid)
);
CREATE INDEX item_preproc_1 ON item_preproc (itemid,step);
CREATE TABLE task_remote_command (
	taskid                   number(20)                                NOT NULL,
	command_type             number(10)      DEFAULT '0'               NOT NULL,
	execute_on               number(10)      DEFAULT '0'               NOT NULL,
	port                     number(10)      DEFAULT '0'               NOT NULL,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	publickey                nvarchar2(64)   DEFAULT ''                ,
	privatekey               nvarchar2(64)   DEFAULT ''                ,
	command                  nvarchar2(2048) DEFAULT ''                ,
	alertid                  number(20)                                NULL,
	parent_taskid            number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE task_remote_command_result (
	taskid                   number(20)                                NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	parent_taskid            number(20)                                NOT NULL,
	info                     nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (taskid)
);
CREATE TABLE task_acknowledge (
	taskid                   number(20)                                NOT NULL,
	acknowledgeid            number(20)                                NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE sysmap_shape (
	sysmap_shapeid           number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	x                        number(10)      DEFAULT '0'               NOT NULL,
	y                        number(10)      DEFAULT '0'               NOT NULL,
	width                    number(10)      DEFAULT '200'             NOT NULL,
	height                   number(10)      DEFAULT '200'             NOT NULL,
	text                     nvarchar2(2048) DEFAULT ''                ,
	font                     number(10)      DEFAULT '9'               NOT NULL,
	font_size                number(10)      DEFAULT '11'              NOT NULL,
	font_color               nvarchar2(6)    DEFAULT '000000'          ,
	text_halign              number(10)      DEFAULT '0'               NOT NULL,
	text_valign              number(10)      DEFAULT '0'               NOT NULL,
	border_type              number(10)      DEFAULT '0'               NOT NULL,
	border_width             number(10)      DEFAULT '1'               NOT NULL,
	border_color             nvarchar2(6)    DEFAULT '000000'          ,
	background_color         nvarchar2(6)    DEFAULT ''                ,
	zindex                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmap_shapeid)
);
CREATE INDEX sysmap_shape_1 ON sysmap_shape (sysmapid);
CREATE TABLE sysmap_element_trigger (
	selement_triggerid       number(20)                                NOT NULL,
	selementid               number(20)                                NOT NULL,
	triggerid                number(20)                                NOT NULL,
	PRIMARY KEY (selement_triggerid)
);
CREATE UNIQUE INDEX sysmap_element_trigger_1 ON sysmap_element_trigger (selementid,triggerid);
CREATE TABLE httptest_field (
	httptest_fieldid         number(20)                                NOT NULL,
	httptestid               number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (httptest_fieldid)
);
CREATE INDEX httptest_field_1 ON httptest_field (httptestid);
CREATE TABLE httpstep_field (
	httpstep_fieldid         number(20)                                NOT NULL,
	httpstepid               number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (httpstep_fieldid)
);
CREATE INDEX httpstep_field_1 ON httpstep_field (httpstepid);
CREATE TABLE dashboard (
	dashboardid              number(20)                                NOT NULL,
	name                     nvarchar2(255)                            ,
	userid                   number(20)                                NOT NULL,
	private                  number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (dashboardid)
);
CREATE TABLE dashboard_user (
	dashboard_userid         number(20)                                NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (dashboard_userid)
);
CREATE UNIQUE INDEX dashboard_user_1 ON dashboard_user (dashboardid,userid);
CREATE TABLE dashboard_usrgrp (
	dashboard_usrgrpid       number(20)                                NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (dashboard_usrgrpid)
);
CREATE UNIQUE INDEX dashboard_usrgrp_1 ON dashboard_usrgrp (dashboardid,usrgrpid);
CREATE TABLE widget (
	widgetid                 number(20)                                NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	type                     nvarchar2(255)  DEFAULT ''                ,
	name                     nvarchar2(255)  DEFAULT ''                ,
	x                        number(10)      DEFAULT '0'               NOT NULL,
	y                        number(10)      DEFAULT '0'               NOT NULL,
	width                    number(10)      DEFAULT '1'               NOT NULL,
	height                   number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (widgetid)
);
CREATE INDEX widget_1 ON widget (dashboardid);
CREATE TABLE widget_field (
	widget_fieldid           number(20)                                NOT NULL,
	widgetid                 number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value_int                number(10)      DEFAULT '0'               NOT NULL,
	value_str                nvarchar2(255)  DEFAULT ''                ,
	value_groupid            number(20)                                NULL,
	value_hostid             number(20)                                NULL,
	value_itemid             number(20)                                NULL,
	value_graphid            number(20)                                NULL,
	value_sysmapid           number(20)                                NULL,
	PRIMARY KEY (widget_fieldid)
);
CREATE INDEX widget_field_1 ON widget_field (widgetid);
CREATE INDEX widget_field_2 ON widget_field (value_groupid);
CREATE INDEX widget_field_3 ON widget_field (value_hostid);
CREATE INDEX widget_field_4 ON widget_field (value_itemid);
CREATE INDEX widget_field_5 ON widget_field (value_graphid);
CREATE INDEX widget_field_6 ON widget_field (value_sysmapid);
CREATE TABLE dbversion (
	mandatory                number(10)      DEFAULT '0'               NOT NULL,
	optional                 number(10)      DEFAULT '0'               NOT NULL
);
INSERT INTO dbversion VALUES ('3040000','3040005');
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
