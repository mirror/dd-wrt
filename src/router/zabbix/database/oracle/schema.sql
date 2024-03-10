CREATE TABLE role (
	roleid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	readonly                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (roleid)
);
CREATE UNIQUE INDEX role_1 ON role (name);
CREATE TABLE users (
	userid                   number(20)                                NOT NULL,
	username                 nvarchar2(100)  DEFAULT ''                ,
	name                     nvarchar2(100)  DEFAULT ''                ,
	surname                  nvarchar2(100)  DEFAULT ''                ,
	passwd                   nvarchar2(60)   DEFAULT ''                ,
	url                      nvarchar2(2048) DEFAULT ''                ,
	autologin                number(10)      DEFAULT '0'               NOT NULL,
	autologout               nvarchar2(32)   DEFAULT '15m'             ,
	lang                     nvarchar2(7)    DEFAULT 'default'         ,
	refresh                  nvarchar2(32)   DEFAULT '30s'             ,
	theme                    nvarchar2(128)  DEFAULT 'default'         ,
	attempt_failed           number(10)      DEFAULT 0                 NOT NULL,
	attempt_ip               nvarchar2(39)   DEFAULT ''                ,
	attempt_clock            number(10)      DEFAULT 0                 NOT NULL,
	rows_per_page            number(10)      DEFAULT 50                NOT NULL,
	timezone                 nvarchar2(50)   DEFAULT 'default'         ,
	roleid                   number(20)      DEFAULT NULL              NULL,
	userdirectoryid          number(20)      DEFAULT NULL              NULL,
	ts_provisioned           number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (userid)
);
CREATE UNIQUE INDEX users_1 ON users (username);
CREATE INDEX users_2 ON users (userdirectoryid);
CREATE INDEX users_3 ON users (roleid);
CREATE TABLE maintenances (
	maintenanceid            number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	maintenance_type         number(10)      DEFAULT '0'               NOT NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	active_since             number(10)      DEFAULT '0'               NOT NULL,
	active_till              number(10)      DEFAULT '0'               NOT NULL,
	tags_evaltype            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (maintenanceid)
);
CREATE INDEX maintenances_1 ON maintenances (active_since,active_till);
CREATE UNIQUE INDEX maintenances_2 ON maintenances (name);
CREATE TABLE hosts (
	hostid                   number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	host                     nvarchar2(128)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	ipmi_authtype            number(10)      DEFAULT '-1'              NOT NULL,
	ipmi_privilege           number(10)      DEFAULT '2'               NOT NULL,
	ipmi_username            nvarchar2(16)   DEFAULT ''                ,
	ipmi_password            nvarchar2(20)   DEFAULT ''                ,
	maintenanceid            number(20)                                NULL,
	maintenance_status       number(10)      DEFAULT '0'               NOT NULL,
	maintenance_type         number(10)      DEFAULT '0'               NOT NULL,
	maintenance_from         number(10)      DEFAULT '0'               NOT NULL,
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
	proxy_address            nvarchar2(255)  DEFAULT ''                ,
	auto_compress            number(10)      DEFAULT '1'               NOT NULL,
	discover                 number(10)      DEFAULT '0'               NOT NULL,
	custom_interfaces        number(10)      DEFAULT '0'               NOT NULL,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	name_upper               nvarchar2(128)  DEFAULT ''                ,
	vendor_name              nvarchar2(64)   DEFAULT ''                ,
	vendor_version           nvarchar2(32)   DEFAULT ''                ,
	PRIMARY KEY (hostid)
);
CREATE INDEX hosts_1 ON hosts (host);
CREATE INDEX hosts_2 ON hosts (status);
CREATE INDEX hosts_3 ON hosts (proxy_hostid);
CREATE INDEX hosts_4 ON hosts (name);
CREATE INDEX hosts_5 ON hosts (maintenanceid);
CREATE INDEX hosts_6 ON hosts (name_upper);
CREATE INDEX hosts_7 ON hosts (templateid);
CREATE TABLE hstgrp (
	groupid                  number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE UNIQUE INDEX hstgrp_1 ON hstgrp (type,name);
CREATE TABLE group_prototype (
	group_prototypeid        number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	groupid                  number(20)                                NULL,
	templateid               number(20)                                NULL,
	PRIMARY KEY (group_prototypeid)
);
CREATE INDEX group_prototype_1 ON group_prototype (hostid);
CREATE INDEX group_prototype_2 ON group_prototype (groupid);
CREATE INDEX group_prototype_3 ON group_prototype (templateid);
CREATE TABLE group_discovery (
	groupid                  number(20)                                NOT NULL,
	parent_group_prototypeid number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (groupid)
);
CREATE INDEX group_discovery_1 ON group_discovery (parent_group_prototypeid);
CREATE TABLE drules (
	druleid                  number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	iprange                  nvarchar2(2048) DEFAULT ''                ,
	delay                    nvarchar2(255)  DEFAULT '1h'              ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (druleid)
);
CREATE INDEX drules_1 ON drules (proxy_hostid);
CREATE UNIQUE INDEX drules_2 ON drules (name);
CREATE TABLE dchecks (
	dcheckid                 number(20)                                NOT NULL,
	druleid                  number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	key_                     nvarchar2(2048) DEFAULT ''                ,
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
	host_source              number(10)      DEFAULT '1'               NOT NULL,
	name_source              number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dcheckid)
);
CREATE INDEX dchecks_1 ON dchecks (druleid,host_source,name_source);
CREATE TABLE httptest (
	httptestid               number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
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
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	PRIMARY KEY (httptestid)
);
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
	type                     number(10)      DEFAULT '1'               NOT NULL,
	useip                    number(10)      DEFAULT '1'               NOT NULL,
	ip                       nvarchar2(64)   DEFAULT '127.0.0.1'       ,
	dns                      nvarchar2(255)  DEFAULT ''                ,
	port                     nvarchar2(64)   DEFAULT '10050'           ,
	available                number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(2048) DEFAULT ''                ,
	errors_from              number(10)      DEFAULT '0'               NOT NULL,
	disable_until            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_1 ON interface (hostid,type);
CREATE INDEX interface_2 ON interface (ip,dns);
CREATE INDEX interface_3 ON interface (available);
CREATE TABLE valuemap (
	valuemapid               number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	PRIMARY KEY (valuemapid)
);
CREATE UNIQUE INDEX valuemap_1 ON valuemap (hostid,name);
CREATE TABLE items (
	itemid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	snmp_oid                 nvarchar2(512)  DEFAULT ''                ,
	hostid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	key_                     nvarchar2(2048) DEFAULT ''                ,
	delay                    nvarchar2(1024) DEFAULT '0'               ,
	history                  nvarchar2(255)  DEFAULT '90d'             ,
	trends                   nvarchar2(255)  DEFAULT '365d'            ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	value_type               number(10)      DEFAULT '0'               NOT NULL,
	trapper_hosts            nvarchar2(255)  DEFAULT ''                ,
	units                    nvarchar2(255)  DEFAULT ''                ,
	formula                  nvarchar2(255)  DEFAULT ''                ,
	logtimefmt               nvarchar2(64)   DEFAULT ''                ,
	templateid               number(20)                                NULL,
	valuemapid               number(20)                                NULL,
	params                   nclob           DEFAULT ''                ,
	ipmi_sensor              nvarchar2(128)  DEFAULT ''                ,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	publickey                nvarchar2(64)   DEFAULT ''                ,
	privatekey               nvarchar2(64)   DEFAULT ''                ,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	interfaceid              number(20)                                NULL,
	description              nclob           DEFAULT ''                ,
	inventory_link           number(10)      DEFAULT '0'               NOT NULL,
	lifetime                 nvarchar2(255)  DEFAULT '30d'             ,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	jmx_endpoint             nvarchar2(255)  DEFAULT ''                ,
	master_itemid            number(20)                                NULL,
	timeout                  nvarchar2(255)  DEFAULT '3s'              ,
	url                      nvarchar2(2048) DEFAULT ''                ,
	query_fields             nvarchar2(2048) DEFAULT ''                ,
	posts                    nclob           DEFAULT ''                ,
	status_codes             nvarchar2(255)  DEFAULT '200'             ,
	follow_redirects         number(10)      DEFAULT '1'               NOT NULL,
	post_type                number(10)      DEFAULT '0'               NOT NULL,
	http_proxy               nvarchar2(255)  DEFAULT ''                ,
	headers                  nclob           DEFAULT ''                ,
	retrieve_mode            number(10)      DEFAULT '0'               NOT NULL,
	request_method           number(10)      DEFAULT '0'               NOT NULL,
	output_format            number(10)      DEFAULT '0'               NOT NULL,
	ssl_cert_file            nvarchar2(255)  DEFAULT ''                ,
	ssl_key_file             nvarchar2(255)  DEFAULT ''                ,
	ssl_key_password         nvarchar2(64)   DEFAULT ''                ,
	verify_peer              number(10)      DEFAULT '0'               NOT NULL,
	verify_host              number(10)      DEFAULT '0'               NOT NULL,
	allow_traps              number(10)      DEFAULT '0'               NOT NULL,
	discover                 number(10)      DEFAULT '0'               NOT NULL,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	name_upper               nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (itemid)
);
CREATE INDEX items_1 ON items (hostid,key_);
CREATE INDEX items_3 ON items (status);
CREATE INDEX items_4 ON items (templateid);
CREATE INDEX items_5 ON items (valuemapid);
CREATE INDEX items_6 ON items (interfaceid);
CREATE INDEX items_7 ON items (master_itemid);
CREATE INDEX items_8 ON items (key_);
CREATE INDEX items_9 ON items (hostid,name_upper);
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
	name                     nvarchar2(100)  DEFAULT ''                ,
	smtp_server              nvarchar2(255)  DEFAULT ''                ,
	smtp_helo                nvarchar2(255)  DEFAULT ''                ,
	smtp_email               nvarchar2(255)  DEFAULT ''                ,
	exec_path                nvarchar2(255)  DEFAULT ''                ,
	gsm_modem                nvarchar2(255)  DEFAULT ''                ,
	username                 nvarchar2(255)  DEFAULT ''                ,
	passwd                   nvarchar2(255)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '1'               NOT NULL,
	smtp_port                number(10)      DEFAULT '25'              NOT NULL,
	smtp_security            number(10)      DEFAULT '0'               NOT NULL,
	smtp_verify_peer         number(10)      DEFAULT '0'               NOT NULL,
	smtp_verify_host         number(10)      DEFAULT '0'               NOT NULL,
	smtp_authentication      number(10)      DEFAULT '0'               NOT NULL,
	maxsessions              number(10)      DEFAULT '1'               NOT NULL,
	maxattempts              number(10)      DEFAULT '3'               NOT NULL,
	attempt_interval         nvarchar2(32)   DEFAULT '10s'             ,
	content_type             number(10)      DEFAULT '1'               NOT NULL,
	script                   nclob           DEFAULT ''                ,
	timeout                  nvarchar2(32)   DEFAULT '30s'             ,
	process_tags             number(10)      DEFAULT '0'               NOT NULL,
	show_event_menu          number(10)      DEFAULT '0'               NOT NULL,
	event_menu_url           nvarchar2(2048) DEFAULT ''                ,
	event_menu_name          nvarchar2(255)  DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	provider                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (mediatypeid)
);
CREATE UNIQUE INDEX media_type_1 ON media_type (name);
CREATE TABLE media_type_param (
	mediatype_paramid        number(20)                                NOT NULL,
	mediatypeid              number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (mediatype_paramid)
);
CREATE INDEX media_type_param_1 ON media_type_param (mediatypeid);
CREATE TABLE media_type_message (
	mediatype_messageid      number(20)                                NOT NULL,
	mediatypeid              number(20)                                NOT NULL,
	eventsource              number(10)                                NOT NULL,
	recovery                 number(10)                                NOT NULL,
	subject                  nvarchar2(255)  DEFAULT ''                ,
	message                  nclob           DEFAULT ''                ,
	PRIMARY KEY (mediatype_messageid)
);
CREATE UNIQUE INDEX media_type_message_1 ON media_type_message (mediatypeid,eventsource,recovery);
CREATE TABLE usrgrp (
	usrgrpid                 number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	gui_access               number(10)      DEFAULT '0'               NOT NULL,
	users_status             number(10)      DEFAULT '0'               NOT NULL,
	debug_mode               number(10)      DEFAULT '0'               NOT NULL,
	userdirectoryid          number(20)      DEFAULT NULL              NULL,
	PRIMARY KEY (usrgrpid)
);
CREATE UNIQUE INDEX usrgrp_1 ON usrgrp (name);
CREATE INDEX usrgrp_2 ON usrgrp (userdirectoryid);
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
	command                  nclob           DEFAULT ''                ,
	host_access              number(10)      DEFAULT '2'               NOT NULL,
	usrgrpid                 number(20)                                NULL,
	groupid                  number(20)                                NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	confirmation             nvarchar2(255)  DEFAULT ''                ,
	type                     number(10)      DEFAULT '5'               NOT NULL,
	execute_on               number(10)      DEFAULT '2'               NOT NULL,
	timeout                  nvarchar2(32)   DEFAULT '30s'             ,
	scope                    number(10)      DEFAULT '1'               NOT NULL,
	port                     nvarchar2(64)   DEFAULT ''                ,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	publickey                nvarchar2(64)   DEFAULT ''                ,
	privatekey               nvarchar2(64)   DEFAULT ''                ,
	menu_path                nvarchar2(255)  DEFAULT ''                ,
	url                      nvarchar2(2048) DEFAULT ''                ,
	new_window               number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (scriptid)
);
CREATE INDEX scripts_1 ON scripts (usrgrpid);
CREATE INDEX scripts_2 ON scripts (groupid);
CREATE UNIQUE INDEX scripts_3 ON scripts (name,menu_path);
CREATE TABLE script_param (
	script_paramid           number(20)                                NOT NULL,
	scriptid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (script_paramid)
);
CREATE UNIQUE INDEX script_param_1 ON script_param (scriptid,name);
CREATE TABLE actions (
	actionid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	eventsource              number(10)      DEFAULT '0'               NOT NULL,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	esc_period               nvarchar2(255)  DEFAULT '1h'              ,
	formula                  nvarchar2(1024) DEFAULT ''                ,
	pause_suppressed         number(10)      DEFAULT '1'               NOT NULL,
	notify_if_canceled       number(10)      DEFAULT '1'               NOT NULL,
	pause_symptoms           number(10)      DEFAULT '1'               NOT NULL,
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
	default_msg              number(10)      DEFAULT '1'               NOT NULL,
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
	scriptid                 number(20)                                NOT NULL,
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
	work_period              nvarchar2(255)  DEFAULT '1-5,09:00-18:00' ,
	alert_usrgrpid           number(20)                                NULL,
	default_theme            nvarchar2(128)  DEFAULT 'blue-theme'      ,
	authentication_type      number(10)      DEFAULT '0'               NOT NULL,
	discovery_groupid        number(20)                                NULL,
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
	ok_period                nvarchar2(32)   DEFAULT '5m'              ,
	blink_period             nvarchar2(32)   DEFAULT '2m'              ,
	problem_unack_color      nvarchar2(6)    DEFAULT 'CC0000'          ,
	problem_ack_color        nvarchar2(6)    DEFAULT 'CC0000'          ,
	ok_unack_color           nvarchar2(6)    DEFAULT '009900'          ,
	ok_ack_color             nvarchar2(6)    DEFAULT '009900'          ,
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
	custom_color             number(10)      DEFAULT '0'               NOT NULL,
	http_auth_enabled        number(10)      DEFAULT '0'               NOT NULL,
	http_login_form          number(10)      DEFAULT '0'               NOT NULL,
	http_strip_domains       nvarchar2(2048) DEFAULT ''                ,
	http_case_sensitive      number(10)      DEFAULT '1'               NOT NULL,
	ldap_auth_enabled        number(10)      DEFAULT '0'               NOT NULL,
	ldap_case_sensitive      number(10)      DEFAULT '1'               NOT NULL,
	db_extension             nvarchar2(32)   DEFAULT ''                ,
	autoreg_tls_accept       number(10)      DEFAULT '1'               NOT NULL,
	compression_status       number(10)      DEFAULT '0'               NOT NULL,
	compress_older           nvarchar2(32)   DEFAULT '7d'              ,
	instanceid               nvarchar2(32)   DEFAULT ''                ,
	saml_auth_enabled        number(10)      DEFAULT '0'               NOT NULL,
	saml_case_sensitive      number(10)      DEFAULT '0'               NOT NULL,
	default_lang             nvarchar2(5)    DEFAULT 'en_US'           ,
	default_timezone         nvarchar2(50)   DEFAULT 'system'          ,
	login_attempts           number(10)      DEFAULT '5'               NOT NULL,
	login_block              nvarchar2(32)   DEFAULT '30s'             ,
	show_technical_errors    number(10)      DEFAULT '0'               NOT NULL,
	validate_uri_schemes     number(10)      DEFAULT '1'               NOT NULL,
	uri_valid_schemes        nvarchar2(255)  DEFAULT 'http,https,ftp,file,mailto,tel,ssh' ,
	x_frame_options          nvarchar2(255)  DEFAULT 'SAMEORIGIN'      ,
	iframe_sandboxing_enabled number(10)      DEFAULT '1'               NOT NULL,
	iframe_sandboxing_exceptions nvarchar2(255)  DEFAULT ''                ,
	max_overview_table_size  number(10)      DEFAULT '50'              NOT NULL,
	history_period           nvarchar2(32)   DEFAULT '24h'             ,
	period_default           nvarchar2(32)   DEFAULT '1h'              ,
	max_period               nvarchar2(32)   DEFAULT '2y'              ,
	socket_timeout           nvarchar2(32)   DEFAULT '3s'              ,
	connect_timeout          nvarchar2(32)   DEFAULT '3s'              ,
	media_type_test_timeout  nvarchar2(32)   DEFAULT '65s'             ,
	script_timeout           nvarchar2(32)   DEFAULT '60s'             ,
	item_test_timeout        nvarchar2(32)   DEFAULT '60s'             ,
	session_key              nvarchar2(32)   DEFAULT ''                ,
	url                      nvarchar2(255)  DEFAULT ''                ,
	report_test_timeout      nvarchar2(32)   DEFAULT '60s'             ,
	dbversion_status         nvarchar2(2048) DEFAULT ''                ,
	hk_events_service        nvarchar2(32)   DEFAULT '1d'              ,
	passwd_min_length        number(10)      DEFAULT '8'               NOT NULL,
	passwd_check_rules       number(10)      DEFAULT '8'               NOT NULL,
	auditlog_enabled         number(10)      DEFAULT '1'               NOT NULL,
	ha_failover_delay        nvarchar2(32)   DEFAULT '1m'              ,
	geomaps_tile_provider    nvarchar2(255)  DEFAULT ''                ,
	geomaps_tile_url         nvarchar2(1024) DEFAULT ''                ,
	geomaps_max_zoom         number(10)      DEFAULT '0'               NOT NULL,
	geomaps_attribution      nvarchar2(1024) DEFAULT ''                ,
	vault_provider           number(10)      DEFAULT '0'               NOT NULL,
	ldap_userdirectoryid     number(20)      DEFAULT NULL              NULL,
	server_status            nvarchar2(2048) DEFAULT ''                ,
	jit_provision_interval   nvarchar2(32)   DEFAULT '1h'              ,
	saml_jit_status          number(10)      DEFAULT '0'               NOT NULL,
	ldap_jit_status          number(10)      DEFAULT '0'               NOT NULL,
	disabled_usrgrpid        number(20)      DEFAULT NULL              NULL,
	PRIMARY KEY (configid)
);
CREATE INDEX config_1 ON config (alert_usrgrpid);
CREATE INDEX config_2 ON config (discovery_groupid);
CREATE INDEX config_3 ON config (ldap_userdirectoryid);
CREATE INDEX config_4 ON config (disabled_usrgrpid);
CREATE TABLE triggers (
	triggerid                number(20)                                NOT NULL,
	expression               nvarchar2(2048) DEFAULT ''                ,
	description              nvarchar2(255)  DEFAULT ''                ,
	url                      nvarchar2(2048) DEFAULT ''                ,
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
	opdata                   nvarchar2(255)  DEFAULT ''                ,
	discover                 number(10)      DEFAULT '0'               NOT NULL,
	event_name               nvarchar2(2048) DEFAULT ''                ,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	url_name                 nvarchar2(64)   DEFAULT ''                ,
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
	name                     nvarchar2(12)   DEFAULT ''                ,
	parameter                nvarchar2(255)  DEFAULT '0'               ,
	PRIMARY KEY (functionid)
);
CREATE INDEX functions_1 ON functions (triggerid);
CREATE INDEX functions_2 ON functions (itemid,name,parameter);
CREATE TABLE graphs (
	graphid                  number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	width                    number(10)      DEFAULT '900'             NOT NULL,
	height                   number(10)      DEFAULT '200'             NOT NULL,
	yaxismin                 BINARY_DOUBLE   DEFAULT '0'               NOT NULL,
	yaxismax                 BINARY_DOUBLE   DEFAULT '100'             NOT NULL,
	templateid               number(20)                                NULL,
	show_work_period         number(10)      DEFAULT '1'               NOT NULL,
	show_triggers            number(10)      DEFAULT '1'               NOT NULL,
	graphtype                number(10)      DEFAULT '0'               NOT NULL,
	show_legend              number(10)      DEFAULT '1'               NOT NULL,
	show_3d                  number(10)      DEFAULT '0'               NOT NULL,
	percent_left             BINARY_DOUBLE   DEFAULT '0'               NOT NULL,
	percent_right            BINARY_DOUBLE   DEFAULT '0'               NOT NULL,
	ymin_type                number(10)      DEFAULT '0'               NOT NULL,
	ymax_type                number(10)      DEFAULT '0'               NOT NULL,
	ymin_itemid              number(20)                                NULL,
	ymax_itemid              number(20)                                NULL,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	discover                 number(10)      DEFAULT '0'               NOT NULL,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
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
	colorpalette             nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (graphthemeid)
);
CREATE UNIQUE INDEX graph_theme_1 ON graph_theme (theme);
CREATE TABLE globalmacro (
	globalmacroid            number(20)                                NOT NULL,
	macro                    nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalmacroid)
);
CREATE UNIQUE INDEX globalmacro_1 ON globalmacro (macro);
CREATE TABLE hostmacro (
	hostmacroid              number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	macro                    nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	automatic                number(10)      DEFAULT '0'               NOT NULL,
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
	link_type                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (hosttemplateid)
);
CREATE UNIQUE INDEX hosts_templates_1 ON hosts_templates (hostid,templateid);
CREATE INDEX hosts_templates_2 ON hosts_templates (templateid);
CREATE TABLE valuemap_mapping (
	valuemap_mappingid       number(20)                                NOT NULL,
	valuemapid               number(20)                                NOT NULL,
	value                    nvarchar2(64)   DEFAULT ''                ,
	newvalue                 nvarchar2(64)   DEFAULT ''                ,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (valuemap_mappingid)
);
CREATE UNIQUE INDEX valuemap_mapping_1 ON valuemap_mapping (valuemapid,value,type);
CREATE TABLE media (
	mediaid                  number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	mediatypeid              number(20)                                NOT NULL,
	sendto                   nvarchar2(1024) DEFAULT ''                ,
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
	status                   number(10)      DEFAULT '-1'              NOT NULL,
	algorithm                number(10)      DEFAULT '0'               NOT NULL,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	weight                   number(10)      DEFAULT '0'               NOT NULL,
	propagation_rule         number(10)      DEFAULT '0'               NOT NULL,
	propagation_value        number(10)      DEFAULT '0'               NOT NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	created_at               number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (serviceid)
);
CREATE TABLE services_links (
	linkid                   number(20)                                NOT NULL,
	serviceupid              number(20)                                NOT NULL,
	servicedownid            number(20)                                NOT NULL,
	PRIMARY KEY (linkid)
);
CREATE INDEX services_links_1 ON services_links (servicedownid);
CREATE UNIQUE INDEX services_links_2 ON services_links (serviceupid,servicedownid);
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
	show_suppressed          number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sysmapid)
);
CREATE UNIQUE INDEX sysmaps_1 ON sysmaps (name);
CREATE INDEX sysmaps_2 ON sysmaps (backgroundid);
CREATE INDEX sysmaps_3 ON sysmaps (iconmapid);
CREATE INDEX sysmaps_4 ON sysmaps (userid);
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
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
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
CREATE INDEX sysmap_user_2 ON sysmap_user (userid);
CREATE TABLE sysmap_usrgrp (
	sysmapusrgrpid           number(20)                                NOT NULL,
	sysmapid                 number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (sysmapusrgrpid)
);
CREATE UNIQUE INDEX sysmap_usrgrp_1 ON sysmap_usrgrp (sysmapid,usrgrpid);
CREATE INDEX sysmap_usrgrp_2 ON sysmap_usrgrp (usrgrpid);
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
	sendto                   nvarchar2(1024) DEFAULT ''                ,
	subject                  nvarchar2(255)  DEFAULT ''                ,
	message                  nclob           DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	retries                  number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(2048) DEFAULT ''                ,
	esc_step                 number(10)      DEFAULT '0'               NOT NULL,
	alerttype                number(10)      DEFAULT '0'               NOT NULL,
	p_eventid                number(20)                                NULL,
	acknowledgeid            number(20)                                NULL,
	parameters               nclob           DEFAULT '{}'              ,
	PRIMARY KEY (alertid)
);
CREATE INDEX alerts_1 ON alerts (actionid);
CREATE INDEX alerts_2 ON alerts (clock);
CREATE INDEX alerts_3 ON alerts (eventid);
CREATE INDEX alerts_4 ON alerts (status);
CREATE INDEX alerts_5 ON alerts (mediatypeid);
CREATE INDEX alerts_6 ON alerts (userid);
CREATE INDEX alerts_7 ON alerts (p_eventid);
CREATE INDEX alerts_8 ON alerts (acknowledgeid);
CREATE TABLE history (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    BINARY_DOUBLE   DEFAULT '0.0000'          NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock,ns)
);
CREATE TABLE history_uint (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(20)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock,ns)
);
CREATE TABLE history_str (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock,ns)
);
CREATE TABLE history_log (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	timestamp                number(10)      DEFAULT '0'               NOT NULL,
	source                   nvarchar2(64)   DEFAULT ''                ,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	logeventid               number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock,ns)
);
CREATE TABLE history_text (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    nclob           DEFAULT ''                ,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (itemid,clock,ns)
);
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
	write_clock              number(10)      DEFAULT '0'               NOT NULL,
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
	dns                      nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (id)
);
CREATE INDEX proxy_dhistory_1 ON proxy_dhistory (clock);
CREATE INDEX proxy_dhistory_2 ON proxy_dhistory (druleid);
CREATE TABLE events (
	eventid                  number(20)                                NOT NULL,
	source                   number(10)      DEFAULT '0'               NOT NULL,
	object                   number(10)      DEFAULT '0'               NOT NULL,
	objectid                 number(20)      DEFAULT '0'               NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(10)      DEFAULT '0'               NOT NULL,
	acknowledged             number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	name                     nvarchar2(2048) DEFAULT ''                ,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX events_1 ON events (source,object,objectid,clock);
CREATE INDEX events_2 ON events (source,object,clock);
CREATE TABLE event_symptom (
	eventid                  number(20)                                NOT NULL,
	cause_eventid            number(20)                                NOT NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX event_symptom_1 ON event_symptom (cause_eventid);
CREATE TABLE trends (
	itemid                   number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	num                      number(10)      DEFAULT '0'               NOT NULL,
	value_min                BINARY_DOUBLE   DEFAULT '0.0000'          NOT NULL,
	value_avg                BINARY_DOUBLE   DEFAULT '0.0000'          NOT NULL,
	value_max                BINARY_DOUBLE   DEFAULT '0.0000'          NOT NULL,
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
	message                  nvarchar2(2048) DEFAULT ''                ,
	action                   number(10)      DEFAULT '0'               NOT NULL,
	old_severity             number(10)      DEFAULT '0'               NOT NULL,
	new_severity             number(10)      DEFAULT '0'               NOT NULL,
	suppress_until           number(10)      DEFAULT '0'               NOT NULL,
	taskid                   number(20)                                NULL,
	PRIMARY KEY (acknowledgeid)
);
CREATE INDEX acknowledges_1 ON acknowledges (userid);
CREATE INDEX acknowledges_2 ON acknowledges (eventid);
CREATE INDEX acknowledges_3 ON acknowledges (clock);
CREATE TABLE auditlog (
	auditid                  nvarchar2(25)                             ,
	userid                   number(20)                                NULL,
	username                 nvarchar2(100)  DEFAULT ''                ,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	ip                       nvarchar2(39)   DEFAULT ''                ,
	action                   number(10)      DEFAULT '0'               NOT NULL,
	resourcetype             number(10)      DEFAULT '0'               NOT NULL,
	resourceid               number(20)                                NULL,
	resource_cuid            nvarchar2(25)                             ,
	resourcename             nvarchar2(255)  DEFAULT ''                ,
	recordsetid              nvarchar2(25)                             ,
	details                  nclob           DEFAULT ''                ,
	PRIMARY KEY (auditid)
);
CREATE INDEX auditlog_1 ON auditlog (userid,clock);
CREATE INDEX auditlog_2 ON auditlog (clock);
CREATE INDEX auditlog_3 ON auditlog (resourcetype,resourceid);
CREATE TABLE service_alarms (
	servicealarmid           number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	value                    number(10)      DEFAULT '-1'              NOT NULL,
	PRIMARY KEY (servicealarmid)
);
CREATE INDEX service_alarms_1 ON service_alarms (serviceid,clock);
CREATE INDEX service_alarms_2 ON service_alarms (clock);
CREATE TABLE autoreg_host (
	autoreg_hostid           number(20)                                NOT NULL,
	proxy_hostid             number(20)                                NULL,
	host                     nvarchar2(128)  DEFAULT ''                ,
	listen_ip                nvarchar2(39)   DEFAULT ''                ,
	listen_port              number(10)      DEFAULT '0'               NOT NULL,
	listen_dns               nvarchar2(255)  DEFAULT ''                ,
	host_metadata            nclob           DEFAULT ''                ,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	tls_accepted             number(10)      DEFAULT '1'               NOT NULL,
	PRIMARY KEY (autoreg_hostid)
);
CREATE INDEX autoreg_host_1 ON autoreg_host (host);
CREATE INDEX autoreg_host_2 ON autoreg_host (proxy_hostid);
CREATE TABLE proxy_autoreg_host (
	id                       number(20)                                NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	host                     nvarchar2(128)  DEFAULT ''                ,
	listen_ip                nvarchar2(39)   DEFAULT ''                ,
	listen_port              number(10)      DEFAULT '0'               NOT NULL,
	listen_dns               nvarchar2(255)  DEFAULT ''                ,
	host_metadata            nclob           DEFAULT ''                ,
	flags                    number(10)      DEFAULT '0'               NOT NULL,
	tls_accepted             number(10)      DEFAULT '1'               NOT NULL,
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
	dns                      nvarchar2(255)  DEFAULT ''                ,
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
	servicealarmid           number(20)                                NULL,
	serviceid                number(20)                                NULL,
	PRIMARY KEY (escalationid)
);
CREATE UNIQUE INDEX escalations_1 ON escalations (triggerid,itemid,serviceid,escalationid);
CREATE INDEX escalations_2 ON escalations (eventid);
CREATE INDEX escalations_3 ON escalations (nextcheck);
CREATE TABLE globalvars (
	globalvarid              number(20)                                NOT NULL,
	snmp_lastsize            number(20)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (globalvarid)
);
CREATE TABLE graph_discovery (
	graphid                  number(20)                                NOT NULL,
	parent_graphid           number(20)                                NOT NULL,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (graphid)
);
CREATE INDEX graph_discovery_1 ON graph_discovery (parent_graphid);
CREATE TABLE host_inventory (
	hostid                   number(20)                                NOT NULL,
	inventory_mode           number(10)      DEFAULT '0'               NOT NULL,
	type                     nvarchar2(64)   DEFAULT ''                ,
	type_full                nvarchar2(64)   DEFAULT ''                ,
	name                     nvarchar2(128)  DEFAULT ''                ,
	alias                    nvarchar2(128)  DEFAULT ''                ,
	os                       nvarchar2(128)  DEFAULT ''                ,
	os_full                  nvarchar2(255)  DEFAULT ''                ,
	os_short                 nvarchar2(128)  DEFAULT ''                ,
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
	key_                     nvarchar2(2048) DEFAULT ''                ,
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
	host                     nvarchar2(128)  DEFAULT ''                ,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE INDEX host_discovery_1 ON host_discovery (parent_hostid);
CREATE INDEX host_discovery_2 ON host_discovery (parent_itemid);
CREATE TABLE interface_discovery (
	interfaceid              number(20)                                NOT NULL,
	parent_interfaceid       number(20)                                NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE INDEX interface_discovery_1 ON interface_discovery (parent_interfaceid);
CREATE TABLE profiles (
	profileid                number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	idx                      nvarchar2(96)   DEFAULT ''                ,
	idx2                     number(20)      DEFAULT '0'               NOT NULL,
	value_id                 number(20)      DEFAULT '0'               NOT NULL,
	value_int                number(10)      DEFAULT '0'               NOT NULL,
	value_str                nclob           DEFAULT ''                ,
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
	secret                   nvarchar2(32)   DEFAULT ''                ,
	PRIMARY KEY (sessionid)
);
CREATE INDEX sessions_1 ON sessions (userid,status,lastaccess);
CREATE TABLE trigger_discovery (
	triggerid                number(20)                                NOT NULL,
	parent_triggerid         number(20)                                NOT NULL,
	lastcheck                number(10)      DEFAULT '0'               NOT NULL,
	ts_delete                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (triggerid)
);
CREATE INDEX trigger_discovery_1 ON trigger_discovery (parent_triggerid);
CREATE TABLE item_condition (
	item_conditionid         number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	operator                 number(10)      DEFAULT '8'               NOT NULL,
	macro                    nvarchar2(64)   DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (item_conditionid)
);
CREATE INDEX item_condition_1 ON item_condition (itemid);
CREATE TABLE item_rtdata (
	itemid                   number(20)                                NOT NULL,
	lastlogsize              number(20)      DEFAULT '0'               NOT NULL,
	state                    number(10)      DEFAULT '0'               NOT NULL,
	mtime                    number(10)      DEFAULT '0'               NOT NULL,
	error                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (itemid)
);
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
	name                     nvarchar2(2048) DEFAULT ''                ,
	acknowledged             number(10)      DEFAULT '0'               NOT NULL,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	cause_eventid            number(20)                                NULL,
	PRIMARY KEY (eventid)
);
CREATE INDEX problem_1 ON problem (source,object,objectid);
CREATE INDEX problem_2 ON problem (r_clock);
CREATE INDEX problem_3 ON problem (r_eventid);
CREATE INDEX problem_4 ON problem (cause_eventid);
CREATE TABLE problem_tag (
	problemtagid             number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (problemtagid)
);
CREATE INDEX problem_tag_1 ON problem_tag (eventid,tag,value);
CREATE TABLE tag_filter (
	tag_filterid             number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	groupid                  number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (tag_filterid)
);
CREATE INDEX tag_filter_1 ON tag_filter (usrgrpid);
CREATE INDEX tag_filter_2 ON tag_filter (groupid);
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
CREATE INDEX task_2 ON task (proxy_hostid);
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
	params                   nclob           DEFAULT ''                ,
	error_handler            number(10)      DEFAULT '0'               NOT NULL,
	error_handler_params     nvarchar2(255)  DEFAULT ''                ,
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
	command                  nclob           DEFAULT ''                ,
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
CREATE TABLE task_data (
	taskid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	data                     nclob           DEFAULT ''                ,
	parent_taskid            number(20)                                NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE task_result (
	taskid                   number(20)                                NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	parent_taskid            number(20)                                NOT NULL,
	info                     nclob           DEFAULT ''                ,
	PRIMARY KEY (taskid)
);
CREATE INDEX task_result_1 ON task_result (parent_taskid);
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
CREATE INDEX sysmap_element_trigger_2 ON sysmap_element_trigger (triggerid);
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
	userid                   number(20)                                NULL,
	private                  number(10)      DEFAULT '1'               NOT NULL,
	templateid               number(20)                                NULL,
	display_period           number(10)      DEFAULT '30'              NOT NULL,
	auto_start               number(10)      DEFAULT '1'               NOT NULL,
	uuid                     nvarchar2(32)   DEFAULT ''                ,
	PRIMARY KEY (dashboardid)
);
CREATE INDEX dashboard_1 ON dashboard (userid);
CREATE INDEX dashboard_2 ON dashboard (templateid);
CREATE TABLE dashboard_user (
	dashboard_userid         number(20)                                NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (dashboard_userid)
);
CREATE UNIQUE INDEX dashboard_user_1 ON dashboard_user (dashboardid,userid);
CREATE INDEX dashboard_user_2 ON dashboard_user (userid);
CREATE TABLE dashboard_usrgrp (
	dashboard_usrgrpid       number(20)                                NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	permission               number(10)      DEFAULT '2'               NOT NULL,
	PRIMARY KEY (dashboard_usrgrpid)
);
CREATE UNIQUE INDEX dashboard_usrgrp_1 ON dashboard_usrgrp (dashboardid,usrgrpid);
CREATE INDEX dashboard_usrgrp_2 ON dashboard_usrgrp (usrgrpid);
CREATE TABLE dashboard_page (
	dashboard_pageid         number(20)                                NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	display_period           number(10)      DEFAULT '0'               NOT NULL,
	sortorder                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dashboard_pageid)
);
CREATE INDEX dashboard_page_1 ON dashboard_page (dashboardid);
CREATE TABLE widget (
	widgetid                 number(20)                                NOT NULL,
	type                     nvarchar2(255)  DEFAULT ''                ,
	name                     nvarchar2(255)  DEFAULT ''                ,
	x                        number(10)      DEFAULT '0'               NOT NULL,
	y                        number(10)      DEFAULT '0'               NOT NULL,
	width                    number(10)      DEFAULT '1'               NOT NULL,
	height                   number(10)      DEFAULT '2'               NOT NULL,
	view_mode                number(10)      DEFAULT '0'               NOT NULL,
	dashboard_pageid         number(20)                                NOT NULL,
	PRIMARY KEY (widgetid)
);
CREATE INDEX widget_1 ON widget (dashboard_pageid);
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
	value_serviceid          number(20)                                NULL,
	value_slaid              number(20)                                NULL,
	value_userid             number(20)                                NULL,
	value_actionid           number(20)                                NULL,
	value_mediatypeid        number(20)                                NULL,
	PRIMARY KEY (widget_fieldid)
);
CREATE INDEX widget_field_1 ON widget_field (widgetid);
CREATE INDEX widget_field_2 ON widget_field (value_groupid);
CREATE INDEX widget_field_3 ON widget_field (value_hostid);
CREATE INDEX widget_field_4 ON widget_field (value_itemid);
CREATE INDEX widget_field_5 ON widget_field (value_graphid);
CREATE INDEX widget_field_6 ON widget_field (value_sysmapid);
CREATE INDEX widget_field_7 ON widget_field (value_serviceid);
CREATE INDEX widget_field_8 ON widget_field (value_slaid);
CREATE INDEX widget_field_9 ON widget_field (value_userid);
CREATE INDEX widget_field_10 ON widget_field (value_actionid);
CREATE INDEX widget_field_11 ON widget_field (value_mediatypeid);
CREATE TABLE task_check_now (
	taskid                   number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	PRIMARY KEY (taskid)
);
CREATE TABLE event_suppress (
	event_suppressid         number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	maintenanceid            number(20)                                NULL,
	suppress_until           number(10)      DEFAULT '0'               NOT NULL,
	userid                   number(20)                                NULL,
	PRIMARY KEY (event_suppressid)
);
CREATE UNIQUE INDEX event_suppress_1 ON event_suppress (eventid,maintenanceid);
CREATE INDEX event_suppress_2 ON event_suppress (suppress_until);
CREATE INDEX event_suppress_3 ON event_suppress (maintenanceid);
CREATE INDEX event_suppress_4 ON event_suppress (userid);
CREATE TABLE maintenance_tag (
	maintenancetagid         number(20)                                NOT NULL,
	maintenanceid            number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	operator                 number(10)      DEFAULT '2'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (maintenancetagid)
);
CREATE INDEX maintenance_tag_1 ON maintenance_tag (maintenanceid);
CREATE TABLE lld_macro_path (
	lld_macro_pathid         number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	lld_macro                nvarchar2(255)  DEFAULT ''                ,
	path                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (lld_macro_pathid)
);
CREATE UNIQUE INDEX lld_macro_path_1 ON lld_macro_path (itemid,lld_macro);
CREATE TABLE host_tag (
	hosttagid                number(20)                                NOT NULL,
	hostid                   number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	automatic                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (hosttagid)
);
CREATE INDEX host_tag_1 ON host_tag (hostid);
CREATE TABLE config_autoreg_tls (
	autoreg_tlsid            number(20)                                NOT NULL,
	tls_psk_identity         nvarchar2(128)  DEFAULT ''                ,
	tls_psk                  nvarchar2(512)  DEFAULT ''                ,
	PRIMARY KEY (autoreg_tlsid)
);
CREATE UNIQUE INDEX config_autoreg_tls_1 ON config_autoreg_tls (tls_psk_identity);
CREATE TABLE module (
	moduleid                 number(20)                                NOT NULL,
	id                       nvarchar2(255)  DEFAULT ''                ,
	relative_path            nvarchar2(255)  DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	config                   nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (moduleid)
);
CREATE TABLE interface_snmp (
	interfaceid              number(20)                                NOT NULL,
	version                  number(10)      DEFAULT '2'               NOT NULL,
	bulk                     number(10)      DEFAULT '1'               NOT NULL,
	community                nvarchar2(64)   DEFAULT ''                ,
	securityname             nvarchar2(64)   DEFAULT ''                ,
	securitylevel            number(10)      DEFAULT '0'               NOT NULL,
	authpassphrase           nvarchar2(64)   DEFAULT ''                ,
	privpassphrase           nvarchar2(64)   DEFAULT ''                ,
	authprotocol             number(10)      DEFAULT '0'               NOT NULL,
	privprotocol             number(10)      DEFAULT '0'               NOT NULL,
	contextname              nvarchar2(255)  DEFAULT ''                ,
	max_repetitions          number(10)      DEFAULT '10'              NOT NULL,
	PRIMARY KEY (interfaceid)
);
CREATE TABLE lld_override (
	lld_overrideid           number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	step                     number(10)      DEFAULT '0'               NOT NULL,
	evaltype                 number(10)      DEFAULT '0'               NOT NULL,
	formula                  nvarchar2(255)  DEFAULT ''                ,
	stop                     number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (lld_overrideid)
);
CREATE UNIQUE INDEX lld_override_1 ON lld_override (itemid,name);
CREATE TABLE lld_override_condition (
	lld_override_conditionid number(20)                                NOT NULL,
	lld_overrideid           number(20)                                NOT NULL,
	operator                 number(10)      DEFAULT '8'               NOT NULL,
	macro                    nvarchar2(64)   DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (lld_override_conditionid)
);
CREATE INDEX lld_override_condition_1 ON lld_override_condition (lld_overrideid);
CREATE TABLE lld_override_operation (
	lld_override_operationid number(20)                                NOT NULL,
	lld_overrideid           number(20)                                NOT NULL,
	operationobject          number(10)      DEFAULT '0'               NOT NULL,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (lld_override_operationid)
);
CREATE INDEX lld_override_operation_1 ON lld_override_operation (lld_overrideid);
CREATE TABLE lld_override_opstatus (
	lld_override_operationid number(20)                                NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE lld_override_opdiscover (
	lld_override_operationid number(20)                                NOT NULL,
	discover                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE lld_override_opperiod (
	lld_override_operationid number(20)                                NOT NULL,
	delay                    nvarchar2(1024) DEFAULT '0'               ,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE lld_override_ophistory (
	lld_override_operationid number(20)                                NOT NULL,
	history                  nvarchar2(255)  DEFAULT '90d'             ,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE lld_override_optrends (
	lld_override_operationid number(20)                                NOT NULL,
	trends                   nvarchar2(255)  DEFAULT '365d'            ,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE lld_override_opseverity (
	lld_override_operationid number(20)                                NOT NULL,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE lld_override_optag (
	lld_override_optagid     number(20)                                NOT NULL,
	lld_override_operationid number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (lld_override_optagid)
);
CREATE INDEX lld_override_optag_1 ON lld_override_optag (lld_override_operationid);
CREATE TABLE lld_override_optemplate (
	lld_override_optemplateid number(20)                                NOT NULL,
	lld_override_operationid number(20)                                NOT NULL,
	templateid               number(20)                                NOT NULL,
	PRIMARY KEY (lld_override_optemplateid)
);
CREATE UNIQUE INDEX lld_override_optemplate_1 ON lld_override_optemplate (lld_override_operationid,templateid);
CREATE INDEX lld_override_optemplate_2 ON lld_override_optemplate (templateid);
CREATE TABLE lld_override_opinventory (
	lld_override_operationid number(20)                                NOT NULL,
	inventory_mode           number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (lld_override_operationid)
);
CREATE TABLE trigger_queue (
	trigger_queueid          number(20)                                NOT NULL,
	objectid                 number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	ns                       number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (trigger_queueid)
);
CREATE TABLE item_parameter (
	item_parameterid         number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (item_parameterid)
);
CREATE INDEX item_parameter_1 ON item_parameter (itemid);
CREATE TABLE role_rule (
	role_ruleid              number(20)                                NOT NULL,
	roleid                   number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value_int                number(10)      DEFAULT '0'               NOT NULL,
	value_str                nvarchar2(255)  DEFAULT ''                ,
	value_moduleid           number(20)                                NULL,
	value_serviceid          number(20)                                NULL,
	PRIMARY KEY (role_ruleid)
);
CREATE INDEX role_rule_1 ON role_rule (roleid);
CREATE INDEX role_rule_2 ON role_rule (value_moduleid);
CREATE INDEX role_rule_3 ON role_rule (value_serviceid);
CREATE TABLE token (
	tokenid                  number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	userid                   number(20)                                NOT NULL,
	token                    nvarchar2(128)                            ,
	lastaccess               number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	expires_at               number(10)      DEFAULT '0'               NOT NULL,
	created_at               number(10)      DEFAULT '0'               NOT NULL,
	creator_userid           number(20)                                NULL,
	PRIMARY KEY (tokenid)
);
CREATE INDEX token_1 ON token (name);
CREATE UNIQUE INDEX token_2 ON token (userid,name);
CREATE UNIQUE INDEX token_3 ON token (token);
CREATE INDEX token_4 ON token (creator_userid);
CREATE TABLE item_tag (
	itemtagid                number(20)                                NOT NULL,
	itemid                   number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (itemtagid)
);
CREATE INDEX item_tag_1 ON item_tag (itemid);
CREATE TABLE httptest_tag (
	httptesttagid            number(20)                                NOT NULL,
	httptestid               number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (httptesttagid)
);
CREATE INDEX httptest_tag_1 ON httptest_tag (httptestid);
CREATE TABLE sysmaps_element_tag (
	selementtagid            number(20)                                NOT NULL,
	selementid               number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (selementtagid)
);
CREATE INDEX sysmaps_element_tag_1 ON sysmaps_element_tag (selementid);
CREATE TABLE report (
	reportid                 number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	dashboardid              number(20)                                NOT NULL,
	period                   number(10)      DEFAULT '0'               NOT NULL,
	cycle                    number(10)      DEFAULT '0'               NOT NULL,
	weekdays                 number(10)      DEFAULT '0'               NOT NULL,
	start_time               number(10)      DEFAULT '0'               NOT NULL,
	active_since             number(10)      DEFAULT '0'               NOT NULL,
	active_till              number(10)      DEFAULT '0'               NOT NULL,
	state                    number(10)      DEFAULT '0'               NOT NULL,
	lastsent                 number(10)      DEFAULT '0'               NOT NULL,
	info                     nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (reportid)
);
CREATE UNIQUE INDEX report_1 ON report (name);
CREATE INDEX report_2 ON report (userid);
CREATE INDEX report_3 ON report (dashboardid);
CREATE TABLE report_param (
	reportparamid            number(20)                                NOT NULL,
	reportid                 number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (reportparamid)
);
CREATE INDEX report_param_1 ON report_param (reportid);
CREATE TABLE report_user (
	reportuserid             number(20)                                NOT NULL,
	reportid                 number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	exclude                  number(10)      DEFAULT '0'               NOT NULL,
	access_userid            number(20)                                NULL,
	PRIMARY KEY (reportuserid)
);
CREATE INDEX report_user_1 ON report_user (reportid);
CREATE INDEX report_user_2 ON report_user (userid);
CREATE INDEX report_user_3 ON report_user (access_userid);
CREATE TABLE report_usrgrp (
	reportusrgrpid           number(20)                                NOT NULL,
	reportid                 number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	access_userid            number(20)                                NULL,
	PRIMARY KEY (reportusrgrpid)
);
CREATE INDEX report_usrgrp_1 ON report_usrgrp (reportid);
CREATE INDEX report_usrgrp_2 ON report_usrgrp (usrgrpid);
CREATE INDEX report_usrgrp_3 ON report_usrgrp (access_userid);
CREATE TABLE service_problem_tag (
	service_problem_tagid    number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (service_problem_tagid)
);
CREATE INDEX service_problem_tag_1 ON service_problem_tag (serviceid);
CREATE TABLE service_problem (
	service_problemid        number(20)                                NOT NULL,
	eventid                  number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	severity                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (service_problemid)
);
CREATE INDEX service_problem_1 ON service_problem (eventid);
CREATE INDEX service_problem_2 ON service_problem (serviceid);
CREATE TABLE service_tag (
	servicetagid             number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (servicetagid)
);
CREATE INDEX service_tag_1 ON service_tag (serviceid);
CREATE TABLE service_status_rule (
	service_status_ruleid    number(20)                                NOT NULL,
	serviceid                number(20)                                NOT NULL,
	type                     number(10)      DEFAULT '0'               NOT NULL,
	limit_value              number(10)      DEFAULT '0'               NOT NULL,
	limit_status             number(10)      DEFAULT '0'               NOT NULL,
	new_status               number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (service_status_ruleid)
);
CREATE INDEX service_status_rule_1 ON service_status_rule (serviceid);
CREATE TABLE ha_node (
	ha_nodeid                nvarchar2(25)                             ,
	name                     nvarchar2(255)  DEFAULT ''                ,
	address                  nvarchar2(255)  DEFAULT ''                ,
	port                     number(10)      DEFAULT '10051'           NOT NULL,
	lastaccess               number(10)      DEFAULT '0'               NOT NULL,
	status                   number(10)      DEFAULT '0'               NOT NULL,
	ha_sessionid             nvarchar2(25)   DEFAULT ''                ,
	PRIMARY KEY (ha_nodeid)
);
CREATE UNIQUE INDEX ha_node_1 ON ha_node (name);
CREATE INDEX ha_node_2 ON ha_node (status,lastaccess);
CREATE TABLE sla (
	slaid                    number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	period                   number(10)      DEFAULT '0'               NOT NULL,
	slo                      BINARY_DOUBLE   DEFAULT '99.9'            NOT NULL,
	effective_date           number(10)      DEFAULT '0'               NOT NULL,
	timezone                 nvarchar2(50)   DEFAULT 'UTC'             ,
	status                   number(10)      DEFAULT '1'               NOT NULL,
	description              nvarchar2(2048) DEFAULT ''                ,
	PRIMARY KEY (slaid)
);
CREATE UNIQUE INDEX sla_1 ON sla (name);
CREATE TABLE sla_schedule (
	sla_scheduleid           number(20)                                NOT NULL,
	slaid                    number(20)                                NOT NULL,
	period_from              number(10)      DEFAULT '0'               NOT NULL,
	period_to                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sla_scheduleid)
);
CREATE INDEX sla_schedule_1 ON sla_schedule (slaid);
CREATE TABLE sla_excluded_downtime (
	sla_excluded_downtimeid  number(20)                                NOT NULL,
	slaid                    number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	period_from              number(10)      DEFAULT '0'               NOT NULL,
	period_to                number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (sla_excluded_downtimeid)
);
CREATE INDEX sla_excluded_downtime_1 ON sla_excluded_downtime (slaid);
CREATE TABLE sla_service_tag (
	sla_service_tagid        number(20)                                NOT NULL,
	slaid                    number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (sla_service_tagid)
);
CREATE INDEX sla_service_tag_1 ON sla_service_tag (slaid);
CREATE TABLE host_rtdata (
	hostid                   number(20)                                NOT NULL,
	active_available         number(10)      DEFAULT '0'               NOT NULL,
	lastaccess               number(10)      DEFAULT '0'               NOT NULL,
	version                  number(10)      DEFAULT '0'               NOT NULL,
	compatibility            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (hostid)
);
CREATE TABLE userdirectory (
	userdirectoryid          number(20)                                NOT NULL,
	name                     nvarchar2(128)  DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	idp_type                 number(10)      DEFAULT '1'               NOT NULL,
	provision_status         number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (userdirectoryid)
);
CREATE INDEX userdirectory_1 ON userdirectory (idp_type);
CREATE TABLE userdirectory_ldap (
	userdirectoryid          number(20)                                NOT NULL,
	host                     nvarchar2(255)  DEFAULT ''                ,
	port                     number(10)      DEFAULT '389'             NOT NULL,
	base_dn                  nvarchar2(255)  DEFAULT ''                ,
	search_attribute         nvarchar2(128)  DEFAULT ''                ,
	bind_dn                  nvarchar2(255)  DEFAULT ''                ,
	bind_password            nvarchar2(128)  DEFAULT ''                ,
	start_tls                number(10)      DEFAULT '0'               NOT NULL,
	search_filter            nvarchar2(255)  DEFAULT ''                ,
	group_basedn             nvarchar2(255)  DEFAULT ''                ,
	group_name               nvarchar2(255)  DEFAULT ''                ,
	group_member             nvarchar2(255)  DEFAULT ''                ,
	user_ref_attr            nvarchar2(255)  DEFAULT ''                ,
	group_filter             nvarchar2(255)  DEFAULT ''                ,
	group_membership         nvarchar2(255)  DEFAULT ''                ,
	user_username            nvarchar2(255)  DEFAULT ''                ,
	user_lastname            nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (userdirectoryid)
);
CREATE TABLE userdirectory_saml (
	userdirectoryid          number(20)                                NOT NULL,
	idp_entityid             nvarchar2(1024) DEFAULT ''                ,
	sso_url                  nvarchar2(2048) DEFAULT ''                ,
	slo_url                  nvarchar2(2048) DEFAULT ''                ,
	username_attribute       nvarchar2(128)  DEFAULT ''                ,
	sp_entityid              nvarchar2(1024) DEFAULT ''                ,
	nameid_format            nvarchar2(2048) DEFAULT ''                ,
	sign_messages            number(10)      DEFAULT '0'               NOT NULL,
	sign_assertions          number(10)      DEFAULT '0'               NOT NULL,
	sign_authn_requests      number(10)      DEFAULT '0'               NOT NULL,
	sign_logout_requests     number(10)      DEFAULT '0'               NOT NULL,
	sign_logout_responses    number(10)      DEFAULT '0'               NOT NULL,
	encrypt_nameid           number(10)      DEFAULT '0'               NOT NULL,
	encrypt_assertions       number(10)      DEFAULT '0'               NOT NULL,
	group_name               nvarchar2(255)  DEFAULT ''                ,
	user_username            nvarchar2(255)  DEFAULT ''                ,
	user_lastname            nvarchar2(255)  DEFAULT ''                ,
	scim_status              number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (userdirectoryid)
);
CREATE TABLE userdirectory_media (
	userdirectory_mediaid    number(20)                                NOT NULL,
	userdirectoryid          number(20)                                NOT NULL,
	mediatypeid              number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	attribute                nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (userdirectory_mediaid)
);
CREATE INDEX userdirectory_media_1 ON userdirectory_media (userdirectoryid);
CREATE INDEX userdirectory_media_2 ON userdirectory_media (mediatypeid);
CREATE TABLE userdirectory_usrgrp (
	userdirectory_usrgrpid   number(20)                                NOT NULL,
	userdirectory_idpgroupid number(20)                                NOT NULL,
	usrgrpid                 number(20)                                NOT NULL,
	PRIMARY KEY (userdirectory_usrgrpid)
);
CREATE UNIQUE INDEX userdirectory_usrgrp_1 ON userdirectory_usrgrp (userdirectory_idpgroupid,usrgrpid);
CREATE INDEX userdirectory_usrgrp_2 ON userdirectory_usrgrp (usrgrpid);
CREATE INDEX userdirectory_usrgrp_3 ON userdirectory_usrgrp (userdirectory_idpgroupid);
CREATE TABLE userdirectory_idpgroup (
	userdirectory_idpgroupid number(20)                                NOT NULL,
	userdirectoryid          number(20)                                NOT NULL,
	roleid                   number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (userdirectory_idpgroupid)
);
CREATE INDEX userdirectory_idpgroup_1 ON userdirectory_idpgroup (userdirectoryid);
CREATE INDEX userdirectory_idpgroup_2 ON userdirectory_idpgroup (roleid);
CREATE TABLE changelog (
	changelogid              number(20)                                NOT NULL,
	object                   number(10)      DEFAULT '0'               NOT NULL,
	objectid                 number(20)                                NOT NULL,
	operation                number(10)      DEFAULT '0'               NOT NULL,
	clock                    number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (changelogid)
);
CREATE INDEX changelog_1 ON changelog (clock);
CREATE TABLE scim_group (
	scim_groupid             number(20)                                NOT NULL,
	name                     nvarchar2(64)   DEFAULT ''                ,
	PRIMARY KEY (scim_groupid)
);
CREATE UNIQUE INDEX scim_group_1 ON scim_group (name);
CREATE TABLE user_scim_group (
	user_scim_groupid        number(20)                                NOT NULL,
	userid                   number(20)                                NOT NULL,
	scim_groupid             number(20)                                NOT NULL,
	PRIMARY KEY (user_scim_groupid)
);
CREATE INDEX user_scim_group_1 ON user_scim_group (userid);
CREATE INDEX user_scim_group_2 ON user_scim_group (scim_groupid);
CREATE TABLE connector (
	connectorid              number(20)                                NOT NULL,
	name                     nvarchar2(255)  DEFAULT ''                ,
	protocol                 number(10)      DEFAULT '0'               NOT NULL,
	data_type                number(10)      DEFAULT '0'               NOT NULL,
	url                      nvarchar2(2048) DEFAULT ''                ,
	max_records              number(10)      DEFAULT '0'               NOT NULL,
	max_senders              number(10)      DEFAULT '1'               NOT NULL,
	max_attempts             number(10)      DEFAULT '1'               NOT NULL,
	timeout                  nvarchar2(255)  DEFAULT '5s'              ,
	http_proxy               nvarchar2(255)  DEFAULT ''                ,
	authtype                 number(10)      DEFAULT '0'               NOT NULL,
	username                 nvarchar2(64)   DEFAULT ''                ,
	password                 nvarchar2(64)   DEFAULT ''                ,
	token                    nvarchar2(128)  DEFAULT ''                ,
	verify_peer              number(10)      DEFAULT '1'               NOT NULL,
	verify_host              number(10)      DEFAULT '1'               NOT NULL,
	ssl_cert_file            nvarchar2(255)  DEFAULT ''                ,
	ssl_key_file             nvarchar2(255)  DEFAULT ''                ,
	ssl_key_password         nvarchar2(64)   DEFAULT ''                ,
	description              nvarchar2(2048) DEFAULT ''                ,
	status                   number(10)      DEFAULT '1'               NOT NULL,
	tags_evaltype            number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (connectorid)
);
CREATE UNIQUE INDEX connector_1 ON connector (name);
CREATE TABLE connector_tag (
	connector_tagid          number(20)                                NOT NULL,
	connectorid              number(20)                                NOT NULL,
	tag                      nvarchar2(255)  DEFAULT ''                ,
	operator                 number(10)      DEFAULT '0'               NOT NULL,
	value                    nvarchar2(255)  DEFAULT ''                ,
	PRIMARY KEY (connector_tagid)
);
CREATE INDEX connector_tag_1 ON connector_tag (connectorid);
CREATE TABLE dbversion (
	dbversionid              number(20)                                NOT NULL,
	mandatory                number(10)      DEFAULT '0'               NOT NULL,
	optional                 number(10)      DEFAULT '0'               NOT NULL,
	PRIMARY KEY (dbversionid)
);
INSERT INTO dbversion VALUES ('1','6040000','6040026');
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
CREATE SEQUENCE changelog_seq
START WITH 1
INCREMENT BY 1
NOMAXVALUE
/
CREATE TRIGGER changelog_tr
BEFORE INSERT ON changelog
FOR EACH ROW
BEGIN
SELECT changelog_seq.nextval INTO :new.changelogid FROM dual;
END;
/
create trigger hosts_insert after insert on hosts
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (1,:new.hostid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger hosts_update after update on hosts
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (1,:old.hostid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger hosts_delete before delete on hosts
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (1,:old.hostid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger hosts_name_upper_insert
before insert on hosts for each row
begin
:new.name_upper:=upper(:new.name);
end;
/
create trigger hosts_name_upper_update
before update on hosts for each row
begin
if :new.name<>:old.name
then
:new.name_upper:=upper(:new.name);
end if;
end;
/
create trigger drules_insert after insert on drules
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (9,:new.druleid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger drules_update after update on drules
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (9,:old.druleid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger drules_delete before delete on drules
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (9,:old.druleid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger dchecks_insert after insert on dchecks
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (10,:new.dcheckid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger dchecks_update after update on dchecks
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (10,:old.dcheckid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger dchecks_delete before delete on dchecks
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (10,:old.dcheckid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptest_insert after insert on httptest
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (11,:new.httptestid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptest_update after update on httptest
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (11,:old.httptestid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptest_delete before delete on httptest
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (11,:old.httptestid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstep_insert after insert on httpstep
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (14,:new.httpstepid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstep_update after update on httpstep
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (14,:old.httpstepid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstep_delete before delete on httpstep
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (14,:old.httpstepid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger items_insert after insert on items
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (3,:new.itemid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger items_update after update on items
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (3,:old.itemid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger items_delete before delete on items
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (3,:old.itemid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger items_name_upper_insert
before insert on items for each row
begin
:new.name_upper:=upper(:new.name);
end;
/
create trigger items_name_upper_update
before update on items for each row
begin
if :new.name<>:old.name
then
:new.name_upper:=upper(:new.name);
end if;
end;
/
create trigger httpstepitem_insert after insert on httpstepitem
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (16,:new.httpstepitemid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstepitem_update after update on httpstepitem
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (16,:old.httpstepitemid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstepitem_delete before delete on httpstepitem
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (16,:old.httpstepitemid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptestitem_insert after insert on httptestitem
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (13,:new.httptestitemid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptestitem_update after update on httptestitem
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (13,:old.httptestitemid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptestitem_delete before delete on httptestitem
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (13,:old.httptestitemid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger triggers_insert after insert on triggers
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (5,:new.triggerid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger triggers_update after update on triggers
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (5,:old.triggerid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger triggers_delete before delete on triggers
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (5,:old.triggerid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger functions_insert after insert on functions
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (7,:new.functionid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger functions_update after update on functions
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (7,:old.functionid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger functions_delete before delete on functions
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (7,:old.functionid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger trigger_tag_insert after insert on trigger_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (6,:new.triggertagid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger trigger_tag_update after update on trigger_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (6,:old.triggertagid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger trigger_tag_delete before delete on trigger_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (6,:old.triggertagid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger item_preproc_insert after insert on item_preproc
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (8,:new.item_preprocid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger item_preproc_update after update on item_preproc
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (8,:old.item_preprocid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger item_preproc_delete before delete on item_preproc
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (8,:old.item_preprocid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptest_field_insert after insert on httptest_field
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (12,:new.httptest_fieldid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptest_field_update after update on httptest_field
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (12,:old.httptest_fieldid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httptest_field_delete before delete on httptest_field
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (12,:old.httptest_fieldid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstep_field_insert after insert on httpstep_field
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (15,:new.httpstep_fieldid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstep_field_update after update on httpstep_field
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (15,:old.httpstep_fieldid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger httpstep_field_delete before delete on httpstep_field
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (15,:old.httpstep_fieldid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger host_tag_insert after insert on host_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (2,:new.hosttagid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger host_tag_update after update on host_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (2,:old.hosttagid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger host_tag_delete before delete on host_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (2,:old.hosttagid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger item_tag_insert after insert on item_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (4,:new.itemtagid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger item_tag_update after update on item_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (4,:old.itemtagid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger item_tag_delete before delete on item_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (4,:old.itemtagid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger connector_insert after insert on connector
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (17,:new.connectorid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger connector_update after update on connector
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (17,:old.connectorid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger connector_delete before delete on connector
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (17,:old.connectorid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger connector_tag_insert after insert on connector_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (18,:new.connector_tagid,1,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger connector_tag_update after update on connector_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (18,:old.connector_tagid,2,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
create trigger connector_tag_delete before delete on connector_tag
for each row
begin
insert into changelog (object,objectid,operation,clock)
values (18,:old.connector_tagid,3,(cast(sys_extract_utc(systimestamp) as date)-date'1970-01-01')*86400);
end;
/
ALTER TABLE users ADD CONSTRAINT c_users_1 FOREIGN KEY (roleid) REFERENCES role (roleid) ON DELETE CASCADE;
ALTER TABLE users ADD CONSTRAINT c_users_2 FOREIGN KEY (userdirectoryid) REFERENCES userdirectory (userdirectoryid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid);
ALTER TABLE hosts ADD CONSTRAINT c_hosts_3 FOREIGN KEY (templateid) REFERENCES hosts (hostid);
ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid);
ALTER TABLE group_prototype ADD CONSTRAINT c_group_prototype_3 FOREIGN KEY (templateid) REFERENCES group_prototype (group_prototypeid) ON DELETE CASCADE;
ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_1 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid) ON DELETE CASCADE;
ALTER TABLE group_discovery ADD CONSTRAINT c_group_discovery_2 FOREIGN KEY (parent_group_prototypeid) REFERENCES group_prototype (group_prototypeid);
ALTER TABLE drules ADD CONSTRAINT c_drules_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid);
ALTER TABLE dchecks ADD CONSTRAINT c_dchecks_1 FOREIGN KEY (druleid) REFERENCES drules (druleid);
ALTER TABLE httptest ADD CONSTRAINT c_httptest_2 FOREIGN KEY (hostid) REFERENCES hosts (hostid);
ALTER TABLE httptest ADD CONSTRAINT c_httptest_3 FOREIGN KEY (templateid) REFERENCES httptest (httptestid);
ALTER TABLE httpstep ADD CONSTRAINT c_httpstep_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid);
ALTER TABLE interface ADD CONSTRAINT c_interface_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE valuemap ADD CONSTRAINT c_valuemap_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE items ADD CONSTRAINT c_items_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid);
ALTER TABLE items ADD CONSTRAINT c_items_2 FOREIGN KEY (templateid) REFERENCES items (itemid);
ALTER TABLE items ADD CONSTRAINT c_items_3 FOREIGN KEY (valuemapid) REFERENCES valuemap (valuemapid);
ALTER TABLE items ADD CONSTRAINT c_items_4 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid);
ALTER TABLE items ADD CONSTRAINT c_items_5 FOREIGN KEY (master_itemid) REFERENCES items (itemid);
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid);
ALTER TABLE httpstepitem ADD CONSTRAINT c_httpstepitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid);
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid);
ALTER TABLE httptestitem ADD CONSTRAINT c_httptestitem_2 FOREIGN KEY (itemid) REFERENCES items (itemid);
ALTER TABLE media_type_param ADD CONSTRAINT c_media_type_param_1 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE media_type_message ADD CONSTRAINT c_media_type_message_1 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE usrgrp ADD CONSTRAINT c_usrgrp_2 FOREIGN KEY (userdirectoryid) REFERENCES userdirectory (userdirectoryid);
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE users_groups ADD CONSTRAINT c_users_groups_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE scripts ADD CONSTRAINT c_scripts_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE scripts ADD CONSTRAINT c_scripts_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid);
ALTER TABLE script_param ADD CONSTRAINT c_script_param_1 FOREIGN KEY (scriptid) REFERENCES scripts (scriptid) ON DELETE CASCADE;
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
ALTER TABLE opcommand_grp ADD CONSTRAINT c_opcommand_grp_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid);
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE opgroup ADD CONSTRAINT c_opgroup_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid);
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE optemplate ADD CONSTRAINT c_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid);
ALTER TABLE opconditions ADD CONSTRAINT c_opconditions_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE conditions ADD CONSTRAINT c_conditions_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE config ADD CONSTRAINT c_config_1 FOREIGN KEY (alert_usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE config ADD CONSTRAINT c_config_2 FOREIGN KEY (discovery_groupid) REFERENCES hstgrp (groupid);
ALTER TABLE config ADD CONSTRAINT c_config_3 FOREIGN KEY (ldap_userdirectoryid) REFERENCES userdirectory (userdirectoryid);
ALTER TABLE config ADD CONSTRAINT c_config_4 FOREIGN KEY (disabled_usrgrpid) REFERENCES usrgrp (usrgrpid);
ALTER TABLE triggers ADD CONSTRAINT c_triggers_1 FOREIGN KEY (templateid) REFERENCES triggers (triggerid);
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_1 FOREIGN KEY (triggerid_down) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE trigger_depends ADD CONSTRAINT c_trigger_depends_2 FOREIGN KEY (triggerid_up) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE functions ADD CONSTRAINT c_functions_1 FOREIGN KEY (itemid) REFERENCES items (itemid);
ALTER TABLE functions ADD CONSTRAINT c_functions_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid);
ALTER TABLE graphs ADD CONSTRAINT c_graphs_1 FOREIGN KEY (templateid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs ADD CONSTRAINT c_graphs_2 FOREIGN KEY (ymin_itemid) REFERENCES items (itemid);
ALTER TABLE graphs ADD CONSTRAINT c_graphs_3 FOREIGN KEY (ymax_itemid) REFERENCES items (itemid);
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_1 FOREIGN KEY (graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE graphs_items ADD CONSTRAINT c_graphs_items_2 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE hostmacro ADD CONSTRAINT c_hostmacro_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_groups ADD CONSTRAINT c_hosts_groups_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid) ON DELETE CASCADE;
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE hosts_templates ADD CONSTRAINT c_hosts_templates_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE valuemap_mapping ADD CONSTRAINT c_valuemap_mapping_1 FOREIGN KEY (valuemapid) REFERENCES valuemap (valuemapid) ON DELETE CASCADE;
ALTER TABLE media ADD CONSTRAINT c_media_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE media ADD CONSTRAINT c_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE rights ADD CONSTRAINT c_rights_1 FOREIGN KEY (groupid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE rights ADD CONSTRAINT c_rights_2 FOREIGN KEY (id) REFERENCES hstgrp (groupid) ON DELETE CASCADE;
ALTER TABLE services_links ADD CONSTRAINT c_services_links_1 FOREIGN KEY (serviceupid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE services_links ADD CONSTRAINT c_services_links_2 FOREIGN KEY (servicedownid) REFERENCES services (serviceid) ON DELETE CASCADE;
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
ALTER TABLE maintenances_groups ADD CONSTRAINT c_maintenances_groups_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE maintenances_windows ADD CONSTRAINT c_maintenances_windows_2 FOREIGN KEY (timeperiodid) REFERENCES timeperiods (timeperiodid) ON DELETE CASCADE;
ALTER TABLE expressions ADD CONSTRAINT c_expressions_1 FOREIGN KEY (regexpid) REFERENCES regexps (regexpid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_1 FOREIGN KEY (actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_4 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_5 FOREIGN KEY (p_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE alerts ADD CONSTRAINT c_alerts_6 FOREIGN KEY (acknowledgeid) REFERENCES acknowledges (acknowledgeid) ON DELETE CASCADE;
ALTER TABLE event_symptom ADD CONSTRAINT c_event_symptom_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE event_symptom ADD CONSTRAINT c_event_symptom_2 FOREIGN KEY (cause_eventid) REFERENCES events (eventid);
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE acknowledges ADD CONSTRAINT c_acknowledges_2 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
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
ALTER TABLE item_condition ADD CONSTRAINT c_item_condition_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE item_rtdata ADD CONSTRAINT c_item_rtdata_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE opinventory ADD CONSTRAINT c_opinventory_1 FOREIGN KEY (operationid) REFERENCES operations (operationid) ON DELETE CASCADE;
ALTER TABLE trigger_tag ADD CONSTRAINT c_trigger_tag_1 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid);
ALTER TABLE event_tag ADD CONSTRAINT c_event_tag_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE problem ADD CONSTRAINT c_problem_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE problem ADD CONSTRAINT c_problem_2 FOREIGN KEY (r_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE problem ADD CONSTRAINT c_problem_3 FOREIGN KEY (cause_eventid) REFERENCES events (eventid);
ALTER TABLE problem_tag ADD CONSTRAINT c_problem_tag_1 FOREIGN KEY (eventid) REFERENCES problem (eventid) ON DELETE CASCADE;
ALTER TABLE tag_filter ADD CONSTRAINT c_tag_filter_1 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE tag_filter ADD CONSTRAINT c_tag_filter_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid) ON DELETE CASCADE;
ALTER TABLE event_recovery ADD CONSTRAINT c_event_recovery_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE event_recovery ADD CONSTRAINT c_event_recovery_2 FOREIGN KEY (r_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE event_recovery ADD CONSTRAINT c_event_recovery_3 FOREIGN KEY (c_eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE corr_condition ADD CONSTRAINT c_corr_condition_1 FOREIGN KEY (correlationid) REFERENCES correlation (correlationid) ON DELETE CASCADE;
ALTER TABLE corr_condition_tag ADD CONSTRAINT c_corr_condition_tag_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_condition_group ADD CONSTRAINT c_corr_condition_group_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_condition_group ADD CONSTRAINT c_corr_condition_group_2 FOREIGN KEY (groupid) REFERENCES hstgrp (groupid);
ALTER TABLE corr_condition_tagpair ADD CONSTRAINT c_corr_condition_tagpair_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_condition_tagvalue ADD CONSTRAINT c_corr_condition_tagvalue_1 FOREIGN KEY (corr_conditionid) REFERENCES corr_condition (corr_conditionid) ON DELETE CASCADE;
ALTER TABLE corr_operation ADD CONSTRAINT c_corr_operation_1 FOREIGN KEY (correlationid) REFERENCES correlation (correlationid) ON DELETE CASCADE;
ALTER TABLE task ADD CONSTRAINT c_task_1 FOREIGN KEY (proxy_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE task_close_problem ADD CONSTRAINT c_task_close_problem_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE item_preproc ADD CONSTRAINT c_item_preproc_1 FOREIGN KEY (itemid) REFERENCES items (itemid);
ALTER TABLE task_remote_command ADD CONSTRAINT c_task_remote_command_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE task_remote_command_result ADD CONSTRAINT c_task_remote_command_result_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE task_data ADD CONSTRAINT c_task_data_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE task_result ADD CONSTRAINT c_task_result_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE task_acknowledge ADD CONSTRAINT c_task_acknowledge_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE sysmap_shape ADD CONSTRAINT c_sysmap_shape_1 FOREIGN KEY (sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE sysmap_element_trigger ADD CONSTRAINT c_sysmap_element_trigger_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE sysmap_element_trigger ADD CONSTRAINT c_sysmap_element_trigger_2 FOREIGN KEY (triggerid) REFERENCES triggers (triggerid) ON DELETE CASCADE;
ALTER TABLE httptest_field ADD CONSTRAINT c_httptest_field_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid);
ALTER TABLE httpstep_field ADD CONSTRAINT c_httpstep_field_1 FOREIGN KEY (httpstepid) REFERENCES httpstep (httpstepid);
ALTER TABLE dashboard ADD CONSTRAINT c_dashboard_1 FOREIGN KEY (userid) REFERENCES users (userid);
ALTER TABLE dashboard ADD CONSTRAINT c_dashboard_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE dashboard_user ADD CONSTRAINT c_dashboard_user_1 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE dashboard_user ADD CONSTRAINT c_dashboard_user_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE dashboard_usrgrp ADD CONSTRAINT c_dashboard_usrgrp_1 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE dashboard_usrgrp ADD CONSTRAINT c_dashboard_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE dashboard_page ADD CONSTRAINT c_dashboard_page_1 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE widget ADD CONSTRAINT c_widget_1 FOREIGN KEY (dashboard_pageid) REFERENCES dashboard_page (dashboard_pageid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_1 FOREIGN KEY (widgetid) REFERENCES widget (widgetid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_2 FOREIGN KEY (value_groupid) REFERENCES hstgrp (groupid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_3 FOREIGN KEY (value_hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_4 FOREIGN KEY (value_itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_5 FOREIGN KEY (value_graphid) REFERENCES graphs (graphid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_6 FOREIGN KEY (value_sysmapid) REFERENCES sysmaps (sysmapid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_7 FOREIGN KEY (value_serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_8 FOREIGN KEY (value_slaid) REFERENCES sla (slaid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_9 FOREIGN KEY (value_userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_10 FOREIGN KEY (value_actionid) REFERENCES actions (actionid) ON DELETE CASCADE;
ALTER TABLE widget_field ADD CONSTRAINT c_widget_field_11 FOREIGN KEY (value_mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE task_check_now ADD CONSTRAINT c_task_check_now_1 FOREIGN KEY (taskid) REFERENCES task (taskid) ON DELETE CASCADE;
ALTER TABLE event_suppress ADD CONSTRAINT c_event_suppress_1 FOREIGN KEY (eventid) REFERENCES events (eventid) ON DELETE CASCADE;
ALTER TABLE event_suppress ADD CONSTRAINT c_event_suppress_2 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE event_suppress ADD CONSTRAINT c_event_suppress_3 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE maintenance_tag ADD CONSTRAINT c_maintenance_tag_1 FOREIGN KEY (maintenanceid) REFERENCES maintenances (maintenanceid) ON DELETE CASCADE;
ALTER TABLE lld_macro_path ADD CONSTRAINT c_lld_macro_path_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE host_tag ADD CONSTRAINT c_host_tag_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid);
ALTER TABLE interface_snmp ADD CONSTRAINT c_interface_snmp_1 FOREIGN KEY (interfaceid) REFERENCES interface (interfaceid) ON DELETE CASCADE;
ALTER TABLE lld_override ADD CONSTRAINT c_lld_override_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE lld_override_condition ADD CONSTRAINT c_lld_override_condition_1 FOREIGN KEY (lld_overrideid) REFERENCES lld_override (lld_overrideid) ON DELETE CASCADE;
ALTER TABLE lld_override_operation ADD CONSTRAINT c_lld_override_operation_1 FOREIGN KEY (lld_overrideid) REFERENCES lld_override (lld_overrideid) ON DELETE CASCADE;
ALTER TABLE lld_override_opstatus ADD CONSTRAINT c_lld_override_opstatus_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_opdiscover ADD CONSTRAINT c_lld_override_opdiscover_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_opperiod ADD CONSTRAINT c_lld_override_opperiod_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_ophistory ADD CONSTRAINT c_lld_override_ophistory_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_optrends ADD CONSTRAINT c_lld_override_optrends_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_opseverity ADD CONSTRAINT c_lld_override_opseverity_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_optag ADD CONSTRAINT c_lld_override_optag_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_optemplate ADD CONSTRAINT c_lld_override_optemplate_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE lld_override_optemplate ADD CONSTRAINT c_lld_override_optemplate_2 FOREIGN KEY (templateid) REFERENCES hosts (hostid);
ALTER TABLE lld_override_opinventory ADD CONSTRAINT c_lld_override_opinventory_1 FOREIGN KEY (lld_override_operationid) REFERENCES lld_override_operation (lld_override_operationid) ON DELETE CASCADE;
ALTER TABLE item_parameter ADD CONSTRAINT c_item_parameter_1 FOREIGN KEY (itemid) REFERENCES items (itemid) ON DELETE CASCADE;
ALTER TABLE role_rule ADD CONSTRAINT c_role_rule_1 FOREIGN KEY (roleid) REFERENCES role (roleid) ON DELETE CASCADE;
ALTER TABLE role_rule ADD CONSTRAINT c_role_rule_2 FOREIGN KEY (value_moduleid) REFERENCES module (moduleid) ON DELETE CASCADE;
ALTER TABLE role_rule ADD CONSTRAINT c_role_rule_3 FOREIGN KEY (value_serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE token ADD CONSTRAINT c_token_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE token ADD CONSTRAINT c_token_2 FOREIGN KEY (creator_userid) REFERENCES users (userid);
ALTER TABLE item_tag ADD CONSTRAINT c_item_tag_1 FOREIGN KEY (itemid) REFERENCES items (itemid);
ALTER TABLE httptest_tag ADD CONSTRAINT c_httptest_tag_1 FOREIGN KEY (httptestid) REFERENCES httptest (httptestid) ON DELETE CASCADE;
ALTER TABLE sysmaps_element_tag ADD CONSTRAINT c_sysmaps_element_tag_1 FOREIGN KEY (selementid) REFERENCES sysmaps_elements (selementid) ON DELETE CASCADE;
ALTER TABLE report ADD CONSTRAINT c_report_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE report ADD CONSTRAINT c_report_2 FOREIGN KEY (dashboardid) REFERENCES dashboard (dashboardid) ON DELETE CASCADE;
ALTER TABLE report_param ADD CONSTRAINT c_report_param_1 FOREIGN KEY (reportid) REFERENCES report (reportid) ON DELETE CASCADE;
ALTER TABLE report_user ADD CONSTRAINT c_report_user_1 FOREIGN KEY (reportid) REFERENCES report (reportid) ON DELETE CASCADE;
ALTER TABLE report_user ADD CONSTRAINT c_report_user_2 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE report_user ADD CONSTRAINT c_report_user_3 FOREIGN KEY (access_userid) REFERENCES users (userid);
ALTER TABLE report_usrgrp ADD CONSTRAINT c_report_usrgrp_1 FOREIGN KEY (reportid) REFERENCES report (reportid) ON DELETE CASCADE;
ALTER TABLE report_usrgrp ADD CONSTRAINT c_report_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE report_usrgrp ADD CONSTRAINT c_report_usrgrp_3 FOREIGN KEY (access_userid) REFERENCES users (userid);
ALTER TABLE service_problem_tag ADD CONSTRAINT c_service_problem_tag_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE service_problem ADD CONSTRAINT c_service_problem_1 FOREIGN KEY (eventid) REFERENCES problem (eventid) ON DELETE CASCADE;
ALTER TABLE service_problem ADD CONSTRAINT c_service_problem_2 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE service_tag ADD CONSTRAINT c_service_tag_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE service_status_rule ADD CONSTRAINT c_service_status_rule_1 FOREIGN KEY (serviceid) REFERENCES services (serviceid) ON DELETE CASCADE;
ALTER TABLE sla_schedule ADD CONSTRAINT c_sla_schedule_1 FOREIGN KEY (slaid) REFERENCES sla (slaid) ON DELETE CASCADE;
ALTER TABLE sla_excluded_downtime ADD CONSTRAINT c_sla_excluded_downtime_1 FOREIGN KEY (slaid) REFERENCES sla (slaid) ON DELETE CASCADE;
ALTER TABLE sla_service_tag ADD CONSTRAINT c_sla_service_tag_1 FOREIGN KEY (slaid) REFERENCES sla (slaid) ON DELETE CASCADE;
ALTER TABLE host_rtdata ADD CONSTRAINT c_host_rtdata_1 FOREIGN KEY (hostid) REFERENCES hosts (hostid) ON DELETE CASCADE;
ALTER TABLE userdirectory_ldap ADD CONSTRAINT c_userdirectory_ldap_1 FOREIGN KEY (userdirectoryid) REFERENCES userdirectory (userdirectoryid) ON DELETE CASCADE;
ALTER TABLE userdirectory_saml ADD CONSTRAINT c_userdirectory_saml_1 FOREIGN KEY (userdirectoryid) REFERENCES userdirectory (userdirectoryid) ON DELETE CASCADE;
ALTER TABLE userdirectory_media ADD CONSTRAINT c_userdirectory_media_1 FOREIGN KEY (userdirectoryid) REFERENCES userdirectory (userdirectoryid) ON DELETE CASCADE;
ALTER TABLE userdirectory_media ADD CONSTRAINT c_userdirectory_media_2 FOREIGN KEY (mediatypeid) REFERENCES media_type (mediatypeid) ON DELETE CASCADE;
ALTER TABLE userdirectory_usrgrp ADD CONSTRAINT c_userdirectory_usrgrp_1 FOREIGN KEY (userdirectory_idpgroupid) REFERENCES userdirectory_idpgroup (userdirectory_idpgroupid) ON DELETE CASCADE;
ALTER TABLE userdirectory_usrgrp ADD CONSTRAINT c_userdirectory_usrgrp_2 FOREIGN KEY (usrgrpid) REFERENCES usrgrp (usrgrpid) ON DELETE CASCADE;
ALTER TABLE userdirectory_idpgroup ADD CONSTRAINT c_userdirectory_idpgroup_1 FOREIGN KEY (userdirectoryid) REFERENCES userdirectory (userdirectoryid) ON DELETE CASCADE;
ALTER TABLE userdirectory_idpgroup ADD CONSTRAINT c_userdirectory_idpgroup_2 FOREIGN KEY (roleid) REFERENCES role (roleid) ON DELETE CASCADE;
ALTER TABLE user_scim_group ADD CONSTRAINT c_user_scim_group_1 FOREIGN KEY (userid) REFERENCES users (userid) ON DELETE CASCADE;
ALTER TABLE user_scim_group ADD CONSTRAINT c_user_scim_group_2 FOREIGN KEY (scim_groupid) REFERENCES scim_group (scim_groupid) ON DELETE CASCADE;
ALTER TABLE connector_tag ADD CONSTRAINT c_connector_tag_1 FOREIGN KEY (connectorid) REFERENCES connector (connectorid);
