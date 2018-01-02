<?php
/*
** Zabbix
** Copyright (C) 2001-2017 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/


require_once dirname(__FILE__).'/include/config.inc.php';
require_once dirname(__FILE__).'/include/hostgroups.inc.php';
require_once dirname(__FILE__).'/include/hosts.inc.php';
require_once dirname(__FILE__).'/include/triggers.inc.php';
require_once dirname(__FILE__).'/include/items.inc.php';
require_once dirname(__FILE__).'/include/users.inc.php';
require_once dirname(__FILE__).'/include/js.inc.php';
require_once dirname(__FILE__).'/include/discovery.inc.php';

$srctbl = getRequest('srctbl', ''); // source table name

// set page title
switch ($srctbl) {
	case 'hosts':
		$page['title'] = _('Hosts');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'templates':
		$page['title'] = _('Templates');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	case 'host_templates':
		$page['title'] = _('Hosts');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	case 'host_groups':
		$page['title'] = _('Host groups');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'proxies':
		$page['title'] = _('Proxies');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	case 'applications':
		$page['title'] = _('Applications');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'triggers':
		$page['title'] = _('Triggers');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'trigger_prototypes':
		$page['title'] = _('Trigger prototypes');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	case 'usrgrp':
		$page['title'] = _('User groups');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'users':
		$page['title'] = _('Users');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'items':
		$page['title'] = _('Items');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'help_items':
		$page['title'] = _('Standard items');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'screens':
		$page['title'] = _('Screens');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'graphs':
		$page['title'] = _('Graphs');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'graph_prototypes':
		$page['title'] = _('Graph prototypes');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'item_prototypes':
		$page['title'] = _('Item prototypes');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'sysmaps':
		$page['title'] = _('Maps');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'screens2':
		$page['title'] = _('Screens');
		$min_user_type = USER_TYPE_ZABBIX_USER;
		break;
	case 'drules':
		$page['title'] = _('Discovery rules');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	case 'dchecks':
		$page['title'] = _('Discovery checks');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	case 'scripts':
		$page['title'] = _('Global scripts');
		$min_user_type = USER_TYPE_ZABBIX_ADMIN;
		break;
	default:
		$page['title'] = _('Error');
		$error = true;
		break;
}
$page['file'] = 'popup.php';

define('ZBX_PAGE_NO_MENU', 1);

require_once dirname(__FILE__).'/include/page_header.php';

if (isset($error)) {
	invalid_url();
}
if ($min_user_type > CWebUser::$data['type']) {
	access_deny();
}

/*
 * Fields
 */
// allowed 'srcfld*' parameter values for each 'srctbl' value
$allowedSrcFields = [
	'users'					=> '"usergrpid", "alias", "fullname", "userid"',
	'triggers'				=> '"description", "triggerid", "expression"',
	'trigger_prototypes'	=> '"description", "triggerid", "expression"',
	'items'					=> '"itemid", "name", "master_itemname"',
	'graphs'				=> '"graphid", "name"',
	'graph_prototypes'		=> '"graphid", "name"',
	'item_prototypes'		=> '"itemid", "name", "flags", "master_itemname"',
	'sysmaps'				=> '"sysmapid", "name"',
	'help_items'			=> '"key"',
	'screens'				=> '"screenid"',
	'screens2'				=> '"screenid", "name"',
	'drules'				=> '"druleid", "name"',
	'dchecks'				=> '"dcheckid", "name"',
	'proxies'				=> '"hostid", "host"',
	'usrgrp'				=> '"usrgrpid", "name"',
	'applications'			=> '"applicationid", "name"',
	'scripts'				=> '"scriptid", "name"',
	'hosts'					=> '"hostid", "host"',
	'templates'				=> '"hostid", "host"',
	'host_templates'		=> '"hostid", "host"',
	'host_groups'			=> '"groupid", "name"'
];

// VAR	TYPE	OPTIONAL	FLAGS	VALIDATION	EXCEPTION
$fields = [
	'dstfrm' =>						[T_ZBX_STR, O_OPT, P_SYS,	NOT_EMPTY,	'!isset({multiselect})'],
	'dstfld1' =>					[T_ZBX_STR, O_OPT, P_SYS,	NOT_EMPTY,	'!isset({multiselect})'],
	'srctbl' =>						[T_ZBX_STR, O_MAND, P_SYS,	NOT_EMPTY,	null],
	'srcfld1' =>					[T_ZBX_STR, O_MAND, P_SYS,	IN($allowedSrcFields[$_REQUEST['srctbl']]), null],
	'groupid' =>					[T_ZBX_INT, O_OPT, P_SYS,	DB_ID,		null],
	'group' =>						[T_ZBX_STR, O_OPT, null,	null,		null],
	'hostid' =>						[T_ZBX_INT, O_OPT, P_SYS,	DB_ID,		null],
	'host' =>						[T_ZBX_STR, O_OPT, null,	null,		null],
	'parent_discoveryid' =>			[T_ZBX_INT, O_OPT, P_SYS,	DB_ID,		null],
	'screenid' =>					[T_ZBX_INT, O_OPT, P_SYS,	DB_ID,		null],
	'templates' =>					[T_ZBX_STR, O_OPT, null,	NOT_EMPTY,	null],
	'host_templates' =>				[T_ZBX_STR, O_OPT, null,	NOT_EMPTY,	null],
	'multiselect' =>				[T_ZBX_INT, O_OPT, null,	null,		null],
	'submit' =>						[T_ZBX_STR, O_OPT, null,	null,		null],
	'excludeids' =>					[T_ZBX_STR, O_OPT, null,	null,		null],
	'only_hostid' =>				[T_ZBX_INT, O_OPT, P_SYS,	DB_ID,		null],
	'monitored_hosts' =>			[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'templated_hosts' =>			[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'real_hosts' =>					[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'normal_only' =>				[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'with_applications' =>			[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'with_graphs' =>				[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'with_items' =>					[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'with_simple_graph_items' =>	[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'with_triggers' =>				[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'with_monitored_triggers' =>	[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'itemtype' =>					[T_ZBX_INT, O_OPT, null,	null,		null],
	'value_types' =>				[T_ZBX_INT, O_OPT, null,	BETWEEN(0, 15), null],
	'numeric' =>					[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'reference' =>					[T_ZBX_STR, O_OPT, null,	null,		null],
	'writeonly' =>					[T_ZBX_STR, O_OPT, null,	null,		null],
	'noempty' =>					[T_ZBX_STR, O_OPT, null,	null,		null],
	'select' =>						[T_ZBX_STR, O_OPT, P_SYS|P_ACT, null,	null],
	'submitParent' =>				[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null],
	'templateid' =>					[T_ZBX_INT, O_OPT, P_SYS,	DB_ID,		null],
	'with_webitems' =>				[T_ZBX_INT, O_OPT, null,	IN('0,1'),	null]
];

// unset disabled item types
$allowedItemTypes = [ITEM_TYPE_ZABBIX, ITEM_TYPE_ZABBIX_ACTIVE, ITEM_TYPE_SIMPLE, ITEM_TYPE_INTERNAL,
	ITEM_TYPE_AGGREGATE, ITEM_TYPE_SNMPTRAP, ITEM_TYPE_DB_MONITOR, ITEM_TYPE_JMX
];
if (hasRequest('itemtype') && !str_in_array(getRequest('itemtype'), $allowedItemTypes)) {
	unset($_REQUEST['itemtype']);
}

// set destination/source fields
$dstfldCount = countRequest('dstfld');
for ($i = 2; $i <= $dstfldCount; $i++) {
	$fields['dstfld'.$i] = [T_ZBX_STR, O_OPT, P_SYS, null, null];
}
$srcfldCount = countRequest('srcfld');
for ($i = 2; $i <= $srcfldCount; $i++) {
	$fields['srcfld'.$i] = [T_ZBX_STR, O_OPT, P_SYS, IN($allowedSrcFields[$_REQUEST['srctbl']]), null];
}
check_fields($fields);

// validate permissions
if (getRequest('only_hostid')) {
	if (!isReadableHostTemplates([getRequest('only_hostid')])) {
		access_deny();
	}
}
else {
	if (getRequest('hostid') && !isReadableHostTemplates([getRequest('hostid')])) {
		access_deny();
	}
	if (getRequest('groupid') && !isReadableHostGroups([getRequest('groupid')])) {
		access_deny();
	}
}
if (getRequest('parent_discoveryid')) {
	$lld_rules = API::DiscoveryRule()->get([
		'output' => [],
		'itemids' => getRequest('parent_discoveryid')
	]);

	if (!$lld_rules) {
		access_deny();
	}
}

$dstfrm = getRequest('dstfrm', ''); // destination form
$dstfld1 = getRequest('dstfld1', ''); // output field on destination form
$dstfld2 = getRequest('dstfld2', ''); // second output field on destination form
$dstfld3 = getRequest('dstfld3', ''); // third output field on destination form
$srcfld1 = getRequest('srcfld1', ''); // source table field [can be different from fields of source table]
$srcfld2 = getRequest('srcfld2'); // second source table field [can be different from fields of source table]
$srcfld3 = getRequest('srcfld3'); //  source table field [can be different from fields of source table]
$multiselect = getRequest('multiselect', 0); // if create popup with checkboxes
$dstact = getRequest('dstact', '');
$writeonly = getRequest('writeonly');
$withApplications = getRequest('with_applications', 0);
$withGraphs = getRequest('with_graphs', 0);
$withItems = getRequest('with_items', 0);
$noempty = getRequest('noempty'); // display/hide "Empty" button
$excludeids = zbx_toHash(getRequest('excludeids', []));
$reference = getRequest('reference', getRequest('srcfld1', 'unknown'));
$realHosts = getRequest('real_hosts', 0);
$monitoredHosts = getRequest('monitored_hosts', 0);
$templatedHosts = getRequest('templated_hosts', 0);
$withSimpleGraphItems = getRequest('with_simple_graph_items', 0);
$withTriggers = getRequest('with_triggers', 0);
$withMonitoredTriggers = getRequest('with_monitored_triggers', 0);
$submitParent = getRequest('submitParent', 0);
$normalOnly = getRequest('normal_only');
$group = getRequest('group', '');
$host = getRequest('host', '');
$onlyHostid = getRequest('only_hostid');
$parentDiscoveryId = getRequest('parent_discoveryid');
$templateid = getRequest('templateid');
$with_webitems = (bool) getRequest('with_webitems', true);

if (isset($onlyHostid)) {
	$_REQUEST['hostid'] = $onlyHostid;

	unset($_REQUEST['groupid']);
}

// value types
$value_types = null;
if (getRequest('value_types')) {
	$value_types = getRequest('value_types');
}
elseif (getRequest('numeric')) {
	$value_types = [ITEM_VALUE_TYPE_FLOAT, ITEM_VALUE_TYPE_UINT64];
}

uncheckTableRows();

/*
 * Page filter
 */
if (!empty($group)) {
	$dbGroup = DBfetch(DBselect('SELECT g.groupid FROM groups g WHERE g.name='.zbx_dbstr($group)));
	if (!empty($dbGroup) && !empty($dbGroup['groupid'])) {
		$_REQUEST['groupid'] = $dbGroup['groupid'];
	}
	unset($dbGroup);
}
if (!empty($host)) {
	$dbHost = DBfetch(DBselect('SELECT h.hostid FROM hosts h WHERE h.name='.zbx_dbstr($host)));
	if (!empty($dbHost) && !empty($dbHost['hostid'])) {
		$_REQUEST['hostid'] = $dbHost['hostid'];
	}
	unset($dbHost);
}

$options = [
	'config' => ['select_latest' => true, 'deny_all' => true, 'popupDD' => true],
	'groups' => [],
	'hosts' => [],
	'groupid' => getRequest('groupid'),
	'hostid' => getRequest('hostid')
];

if (!is_null($writeonly)) {
	$options['groups']['editable'] = true;
	$options['hosts']['editable'] = true;
}

$host_status = null;
$templated = null;

if ($monitoredHosts) {
	$options['groups']['monitored_hosts'] = true;
	$options['hosts']['monitored_hosts'] = true;
	$host_status = 'monitored_hosts';
}
elseif ($realHosts) {
	$options['groups']['real_hosts'] = true;
	$templated = 0;
}
elseif ($templatedHosts) {
	$options['hosts']['templated_hosts'] = true;
	$options['groups']['templated_hosts'] = true;
	$templated = 1;
	$host_status = 'templated_hosts';
}
else {
	$options['groups']['with_hosts_and_templates'] = true;
	$options['hosts']['templated_hosts'] = true; // for hosts templated_hosts comes with monitored and not monitored hosts
}

if ($withApplications) {
	$options['groups']['with_applications'] = true;
	$options['hosts']['with_applications'] = true;
}
elseif ($withGraphs) {
	$options['groups']['with_graphs'] = true;
	$options['hosts']['with_graphs'] = true;
}
elseif ($withSimpleGraphItems) {
	$options['groups']['with_simple_graph_items'] = true;
	$options['hosts']['with_simple_graph_items'] = true;
}
elseif ($withTriggers) {
	$options['groups']['with_triggers'] = true;
	$options['hosts']['with_triggers'] = true;
}
elseif ($withMonitoredTriggers) {
	$options['groups']['with_monitored_triggers'] = true;
	$options['hosts']['with_monitored_triggers'] = true;
}

$pageFilter = new CPageFilter($options);

$groupids = $pageFilter->groupids;

// get hostid
$hostid = null;
if ($pageFilter->hostsSelected) {
	if ($pageFilter->hostid > 0) {
		$hostid = $pageFilter->hostid;
	}
}
else {
	$hostid = 0;
}
if (isset($onlyHostid)) {
	$hostid = $onlyHostid;
}

/*
 * Display table header
 */
$widget = (new CWidget())->setTitle($page['title']);

$frmTitle = new CForm();
if ($monitoredHosts) {
	$frmTitle->addVar('monitored_hosts', 1);
}
if ($realHosts) {
	$frmTitle->addVar('real_hosts', 1);
}
if ($templatedHosts) {
	$frmTitle->addVar('templated_hosts', 1);
}
if ($withApplications) {
	$frmTitle->addVar('with_applications', 1);
}
if ($withGraphs) {
	$frmTitle->addVar('with_graphs', 1);
}
if ($withItems) {
	$frmTitle->addVar('with_items', 1);
}
if ($withSimpleGraphItems) {
	$frmTitle->addVar('with_simple_graph_items', 1);
}
if ($withTriggers) {
	$frmTitle->addVar('with_triggers', 1);
}
if ($withMonitoredTriggers) {
	$frmTitle->addVar('with_monitored_triggers', 1);
}
if ($value_types) {
	$frmTitle->addVar('value_types', $value_types);
}
if ($normalOnly) {
	$frmTitle->addVar('normal_only', $normalOnly);
}
if (hasRequest('excludeids')) {
	$frmTitle->addVar('excludeids', getRequest('excludeids'));
}
if (isset($onlyHostid)) {
	$frmTitle->addVar('only_hostid', $onlyHostid);
}
if (getRequest('screenid')) {
	$frmTitle->addVar('screenid', getRequest('screenid'));
}
if (hasRequest('templateid')) {
	$frmTitle->addVar('templateid', $templateid);
}

// adding param to a form, so that it would remain when page is refreshed
$frmTitle->addVar('dstfrm', $dstfrm);
$frmTitle->addVar('dstact', $dstact);
$frmTitle->addVar('srctbl', $srctbl);
$frmTitle->addVar('multiselect', $multiselect);
$frmTitle->addVar('writeonly', $writeonly);
$frmTitle->addVar('reference', $reference);
$frmTitle->addVar('submitParent', $submitParent);
$frmTitle->addVar('noempty', $noempty);

for ($i = 1; $i <= $dstfldCount; $i++) {
	$frmTitle->addVar('dstfld'.$i, getRequest('dstfld'.$i));
}
for ($i = 1; $i <= $srcfldCount; $i++) {
	$frmTitle->addVar('srcfld'.$i, getRequest('srcfld'.$i));
}

/*
 * Only host id
 */

$controls = [];

if (isset($onlyHostid)) {
	$only_hosts = API::Host()->get([
		'hostids' => $hostid,
		'templated_hosts' => true,
		'output' => ['hostid', 'host', 'name'],
		'limit' => 1
	]);
	$host = reset($only_hosts);

	$controls[] = [
		new CLabel(_('Host'), 'hostid'),
		(new CDiv())->addClass(ZBX_STYLE_FORM_INPUT_MARGIN),
		(new CComboBox('hostid', $hostid))
			->addItem($hostid, $host['name'])
			->setEnabled(false)
			->setAttribute('title', _('You can not switch hosts for current selection.'))
	];
}
else {
	// show Group dropdown in header for these specified sources
	$showGroupCmbBox = ['triggers', 'items', 'applications', 'graphs', 'graph_prototypes', 'item_prototypes',
		'templates', 'hosts', 'host_templates'
	];
	if (str_in_array($srctbl, $showGroupCmbBox) && ($srctbl !== 'item_prototypes' || !$parentDiscoveryId)) {
		$controls[] = [
			new CLabel(_('Group'), 'groupid'),
			(new CDiv())->addClass(ZBX_STYLE_FORM_INPUT_MARGIN),
			$pageFilter->getGroupsCB()
		];
	}

	// show Type dropdown in header for help items
	if ($srctbl === 'help_items') {
		$itemType = getRequest('itemtype', 0);
		$cmbTypes = new CComboBox('itemtype', $itemType, 'javascript: submit();');

		foreach ($allowedItemTypes as $type) {
			$cmbTypes->addItem($type, item_type2str($type));
		}

		$controls[] = [new CLabel(_('Type'), 'itemtype'), (new CDiv())->addClass(ZBX_STYLE_FORM_INPUT_MARGIN), $cmbTypes];
	}

	// show Host dropdown in header for these specified sources
	$showHostCmbBox = ['triggers', 'items', 'applications', 'graphs', 'graph_prototypes', 'item_prototypes'];
	if (str_in_array($srctbl, $showHostCmbBox) && ($srctbl !== 'item_prototypes' || !$parentDiscoveryId)) {
		$controls[] = [
			new CLabel(_('Host'), 'hostid'),
			(new CDiv())->addClass(ZBX_STYLE_FORM_INPUT_MARGIN),
			$pageFilter->getHostsCB()
		];
	}
}

if (str_in_array($srctbl, ['applications', 'triggers'])) {
	if (zbx_empty($noempty)) {
		$elements = [];

		foreach (['dstfld1', 'dstfld2', 'dstfld3'] as $field_name) {
			if (hasRequest($field_name)) {
				$elements[] = ['id' => getRequest($field_name),
					'value' => (strpos(getRequest($field_name), 'id') !== false) ? 0 : ''
				];
			}
		}

		$controls[] = [(new CButton('empty', _('Empty')))->setAttribute('data-object', ['elements' => $elements])];
	}
}

if ($controls) {
	$frmTitle->addItem(new CList($controls));
}
$widget->setControls($frmTitle);

insert_js_function('popupSelectHandlers');

/*
 * User group
 */
if ($srctbl == 'usrgrp') {
	$form = (new CForm())
		->setName('usrgrpform')
		->setId('usrgrps');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_usrgrps'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_usrgrps', 'usrgrps');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => API_OUTPUT_EXTEND,
		'preservekeys' => true
	];
	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}
	$userGroups = API::UserGroup()->get($options);
	order_result($userGroups, 'name');
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($userGroups as &$userGroup) {
		$userGroup['id'] = $userGroup['usrgrpid'];
		$link = (new CLink($userGroup['name'], 'javascript:void(0);'));
		$js_object = [];

		if ($multiselect) {
			$js_object = [
				'object' => $reference,
				'values' => [$userGroup],
				'parentId' => $parentid
			];
		}
		else {
			$elements = [];

			if (array_key_exists($srcfld1, $userGroup)) {
				$elements[] = ['id' => $dstfld1, 'value' => $userGroup[$srcfld1]];
			}
			if (array_key_exists($srcfld2, $userGroup)) {
				$elements[] = ['id' => $dstfld2, 'value' => $userGroup[$srcfld2]];
			}
			if ($elements) {
				$js_object['elements'] = $elements;
			}
		}

		if ($js_object) {
			$link->setAttribute('data-object', $js_object);
		}

		$table->addRow([
			$multiselect
				? new CCheckBox('usrgrps['.$userGroup['usrgrpid'].']', $userGroup['usrgrpid'])
				: null,
			$link
		]);
	}
	unset($userGroup);

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}
/*
 * Users
 */
elseif ($srctbl === 'users') {
	$form = (new CForm())
		->setName('userform')
		->setId('users');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_users'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_users', 'users');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Alias'),
			_x('Name', 'user first name'),
			_('Surname')
		]);

	$options = [
		'output' => ['alias', 'name', 'surname', 'type', 'theme', 'lang'],
		'preservekeys' => true
	];

	if ($writeonly !== null) {
		$options['editable'] = true;
	}

	$users = API::User()->get($options);
	order_result($users, 'alias');
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($users as &$user) {
		if ($multiselect) {
			$checkBox = new CCheckBox('users['.$user['userid'].']', $user['userid']);
		}

		$alias = new CLink($user['alias'], 'javascript:void(0);');
		$user_data = [];

		if ($srcfld1) {
			$user_data = [
				'id' => $user['userid'],
				'name' => $user['alias']
			];
		}

		if ($srcfld2) {
			if ($srcfld2 === 'fullname') {
				$user_data['name'] = getUserFullname($user);
			}
			elseif (array_key_exists($srcfld2, $user)) {
				$user_data[$srcfld2] = $user[$srcfld2];
			}
		}

		if ($user_data) {
			$alias->setAttribute('data-object', [
				'object' => $reference,
				'values' => [$user_data],
				'parentId' => $parentid
			]);
		}

		$table->addRow([$multiselect ? $checkBox : null, $alias, $user['name'], $user['surname']]);
	}
	unset($user);

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Templates
 */
elseif ($srctbl == 'templates') {
	$form = (new CForm())
		->setName('templateform')
		->setId('templates');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_templates'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_templates', 'templates');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => ['templateid', 'name'],
		'groupids' => $groupids,
		'preservekeys' => true
	];

	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}

	$templates = API::Template()->get($options);
	order_result($templates, 'name');
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($templates as &$template) {
		// dont show itself
		if (bccomp($template['templateid'], $templateid) == 0) {
			continue;
		}

		if ($multiselect) {
			$checkBox = new CCheckBox('templates['.$template['templateid'].']', $template['templateid']);
		}

		// check for existing
		if (isset($excludeids[$template['templateid']])) {
			if ($multiselect) {
				$checkBox->setChecked(1);
				$checkBox->setEnabled(false);
			}
			$name = $template['name'];
		}
		else {
			$name = (new CLink($template['name'], 'javascript:void(0);'))->setAttribute('data-object', [
				'object' => $reference,
				'values' => [['id' => $template['templateid'], 'name' => $template['name']]],
				'parentId' => $parentid
			]);
		}

		$table->addRow([$multiselect ? $checkBox : null, $name]);
	}
	unset($template);

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Hosts
 */
elseif ($srctbl == 'hosts') {
	$form = (new CForm())
		->setName('hostform')
		->setId('hosts');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_hosts'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_hosts', 'hosts');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => ['hostid', 'name'],
		'groupids' => $groupids,
		'preservekeys' => true
	];

	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}

	$hosts = API::Host()->get($options);
	order_result($hosts, 'name');
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($hosts as &$host) {
		if ($multiselect) {
			$checkBox = new CCheckBox('hosts['.$host['hostid'].']', $host['hostid']);
		}

		// check for existing
		if (isset($excludeids[$host['hostid']])) {
			if ($multiselect) {
				$checkBox->setChecked(1);
				$checkBox->setEnabled(false);
			}
			$name = $host['name'];
		}
		else {
			$name = (new CLink($host['name'], 'javascript:void(0);'))->setAttribute('data-object', [
				'object' => $reference,
				'values' => [['id' => $host['hostid'], 'name' => $host['name']]],
				'parentId' => $parentid
			]);
		}

		$table->addRow([$multiselect ? $checkBox : null, $name]);
	}
	unset($host);

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Hosts and templates
 */
elseif ($srctbl == 'host_templates') {
	$form = (new CForm())
		->setName('hosttemplateform')
		->setId('hosts');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_hosts'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_hosts', 'hosts');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => ['hostid', 'name'],
		'groupids' => $groupids,
		'templated_hosts' => true,
		'preservekeys' => true
	];

	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}

	$hosts = API::Host()->get($options);
	order_result($hosts, 'name');
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($hosts as &$host) {
		if ($multiselect) {
			$checkBox = new CCheckBox('hosts['.$host['hostid'].']', $host['hostid']);
		}

		// check for existing
		if (isset($excludeids[$host['hostid']])) {
			if ($multiselect) {
				$checkBox->setChecked(1);
				$checkBox->setEnabled(false);
			}
			$name = $host['name'];
		}
		else {
			$name = (new CLink($host['name'], 'javascript:void(0);'))->setAttribute('data-object', [
				'object' => $reference,
				'values' => [['id' => $host['hostid'], 'name' => $host['name']]],
				'parentId' => $parentid
			]);
		}

		$table->addRow([$multiselect ? $checkBox : null, $name]);
	}
	unset($host);

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Host group
 */
elseif ($srctbl == 'host_groups') {
	$form = (new CForm())
		->setName('hostGroupsform')
		->setId('hostGroups');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_hostgroups'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_hostgroups', 'hostGroups');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => ['groupid', 'name'],
		'preservekeys' => true
	];
	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}
	$hostgroups = API::HostGroup()->get($options);
	order_result($hostgroups, 'name');
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($hostgroups as &$hostgroup) {
		if ($multiselect) {
			$checkBox = new CCheckBox('hostGroups['.$hostgroup['groupid'].']', $hostgroup['groupid']);
		}

		// check for existing
		if (isset($excludeids[$hostgroup['groupid']])) {
			if ($multiselect) {
				$checkBox->setChecked(1);
				$checkBox->setEnabled(false);
			}
			$name = $hostgroup['name'];
		}
		else {
			$name = (new CLink($hostgroup['name'], 'javascript:void(0);'))->setAttribute('data-object', [
				'object' => $reference,
				'values' => [['id' => $hostgroup['groupid'], 'name' => $hostgroup['name']]],
				'parentId' => $parentid
			]);
		}

		$table->addRow([$multiselect ? $checkBox : null, $name]);
	}
	unset($hostgroup);

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Help items
 */
elseif ($srctbl === 'help_items') {
	$table = (new CTableInfo())->setHeader([_('Key'), _('Name')]);
	$help_items = new CHelpItems();

	foreach ($help_items->getByType($itemType) as $help_item) {
		if (array_key_exists($srcfld1, $help_item)) {
			$table->addRow([
				(new CLink($help_item['key'], 'javascript:void(0);'))->setAttribute('data-object', [
					'elements' => [
						['id' => $dstfld1, 'value' => $help_item[$srcfld1]]
					]
				]),
				$help_item['description']
			]);
		}
	}

	$widget->addItem($table)->show();
}
/*
 * Triggers and trigger prototypes
 */
elseif ($srctbl === 'triggers' || $srctbl === 'trigger_prototypes') {
	$config = select_config();

	$form = (new CForm())
		->setName('triggerform')
		->setId('triggers');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_triggers'))
						->onClick("checkAll('".$form->getName()."', 'all_triggers', 'triggers');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name'),
			_('Severity'),
			_('Status')
		]);

	$options = [
		'output' => ['triggerid', 'expression', 'description', 'status', 'priority', 'state'],
		'selectHosts' => ['name'],
		'selectDependencies' => ['triggerid', 'expression', 'description'],
		'expandDescription' => true
	];

	if ($srctbl === 'trigger_prototypes') {
		if ($parentDiscoveryId) {
			$options['discoveryids'] = [$parentDiscoveryId];
		}
		else {
			$options['hostids'] = [$hostid];
		}
		if ($writeonly !== null) {
			$options['editable'] = true;
		}

		if ($templated !== null) {
			$options['templated'] = $templated;
		}

		$triggers = API::TriggerPrototype()->get($options);
	}
	else {
		if ($hostid === null) {
			$options['groupids'] = $groupids;
		}
		else {
			$options['hostids'] = [$hostid];
		}

		if ($writeonly !== null) {
			$options['editable'] = true;
		}

		if ($templated !== null) {
			$options['templated'] = $templated;
		}

		if ($withMonitoredTriggers) {
			$options['monitored'] = true;
		}

		if ($normalOnly) {
			$options['filter']['flags'] = ZBX_FLAG_DISCOVERY_NORMAL;
		}

		$triggers = API::Trigger()->get($options);
	}

	order_result($triggers, 'description');

	if ($multiselect) {
		$jsTriggers = [];
	}

	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($triggers as $trigger) {
		$host = reset($trigger['hosts']);
		$trigger['hostname'] = $host['name'];

		$description = new CLink($trigger['description'], 'javascript:void(0);');
		$trigger['description'] = $trigger['hostname'].NAME_DELIMITER.$trigger['description'];
		$js_object = [];

		if ($multiselect) {
			$js_object = [
				'object' => $reference,
				'values' => [[
					'id' => $trigger['triggerid'],
					'name' => $trigger['description'],
					'triggerid' => $trigger['triggerid'],
					'description' => $trigger['description'],
					'expression' => $trigger['expression'],
					'priority' => $trigger['priority'],
					'status' => $trigger['status'],
					'host' => $trigger['hostname']
				]],
				'parentId' => $parentid
			];
		}
		else {
			$elements = [];

			if (array_key_exists($srcfld1, $trigger)) {
				$elements[] = ['id' => $dstfld1, 'value' => $trigger[$srcfld1]];
			}
			if (array_key_exists($srcfld2, $trigger)) {
				$elements[] = ['id' => $dstfld2, 'value' => $trigger[$srcfld2]];
			}
			if (array_key_exists($srcfld3, $trigger)) {
				$elements[] = ['id' => $dstfld3, 'value' => $trigger[$srcfld3]];
			}
			if ($elements) {
				$js_object['elements'] = $elements;
			}
		}

		if ($js_object) {
			$description->setAttribute('data-object', $js_object);
		}

		if ($trigger['dependencies']) {
			$description = [$description, BR(), bold(_('Depends on')), BR()];

			$dependencies = CMacrosResolverHelper::resolveTriggerNames(
				zbx_toHash($trigger['dependencies'], 'triggerid')
			);

			foreach ($dependencies as $dependency) {
				$description[] = $dependency['description'];
				$description[] = BR();
			}
			array_pop($description);
		}

		$table->addRow([
			$multiselect
				? new CCheckBox('triggers['.zbx_jsValue($trigger[$srcfld1]).']', $trigger['triggerid'])
				: null,
			$description,
			getSeverityCell($trigger['priority'], $config),
			(new CSpan(triggerIndicator($trigger['status'], $trigger['state'])))
				->addClass(triggerIndicatorStyle($trigger['status'], $trigger['state']))
		]);

		// made to save memory usage
		if ($multiselect) {
			$jsTriggers[$trigger['triggerid']] = [
				'id' => $trigger['triggerid'],
				'name' => $trigger['description'],
				'prefix' => $trigger['hostname'].NAME_DELIMITER,
				'triggerid' => $trigger['triggerid'],
				'description' => $trigger['description'],
				'expression' => $trigger['expression'],
				'priority' => $trigger['priority'],
				'status' => $trigger['status'],
				'host' => $trigger['hostname']
			];
		}
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Items or Item prototypes
 */
elseif ($srctbl === 'items' || $srctbl === 'item_prototypes') {
	$form = (new CForm())
		->setName('itemform')
		->setId('items');

	$itemPrototypesPopup = ($srctbl === 'item_prototypes');

	$table = (new CTableInfo())
		->setHeader([
			$pageFilter->hostsAll ? _('Host') : null,
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_items'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_items', 'items');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name'),
			_('Key'),
			_('Type'),
			_('Type of information'),
			_('Status')
		]);

	$options = [
		'output' => ['itemid', 'hostid', 'name', 'key_', 'flags', 'type', 'value_type', 'status', 'state'],
		'selectHosts' => ['name']
	];

	if ($parentDiscoveryId) {
		$options['discoveryids'] = [$parentDiscoveryId];
	}
	else {
		$options['hostids'] = $hostid;
	}

	if ($templated == 1) {
		$options['templated'] = true;
	}

	if ($writeonly !== null) {
		$options['editable'] = true;
	}

	if ($value_types !== null) {
		$options['filter']['value_type'] = $value_types;
	}

	if ($itemPrototypesPopup) {
		$items = API::ItemPrototype()->get($options);
	}
	else {
		if ($with_webitems) {
			$options['webitems'] = true;
		}

		if ($normalOnly !== null) {
			$options['filter']['flags'] = ZBX_FLAG_DISCOVERY_NORMAL;
		}

		$items = API::Item()->get($options);
	}

	$items = CMacrosResolverHelper::resolveItemNames($items);
	order_result($items, 'name_expanded');

	foreach ($items as $item) {
		if ($excludeids && array_key_exists($item['itemid'], $excludeids)) {
			// Exclude item from list.
			continue;
		}

		$host = reset($item['hosts']);
		$item['hostname'] = $host['name'];

		$description = new CLink($item['name_expanded'], 'javascript:void(0);');
		$item['name'] = $item['hostname'].NAME_DELIMITER.$item['name_expanded'];
		$item['master_itemname'] = $item['name_expanded'].NAME_DELIMITER.$item['key_'];
		$js_object = [];

		if ($multiselect) {
			$js_object = [
				'object' => $reference,
				'values' => [[
					'itemid' => $item['itemid'],
					'name' => $item['name'],
					'key_' => $item['key_'],
					'flags' => $item['flags'],
					'type' => $item['type'],
					'value_type' => $item['value_type'],
					'host' => $item['hostname']
				]],
				'parentId' => null
			];
		}
		else {
			$elements = [];

			if (array_key_exists($srcfld1, $item)) {
				$elements[] = ['id' => $dstfld1, 'value' => $item[$srcfld1]];
			}
			if ($srcfld2 && array_key_exists($srcfld2, $item)) {
				$elements[] = ['id' => $dstfld2, 'value' => $item[$srcfld2]];
			}
			if ($srcfld3 && array_key_exists($srcfld3, $item)) {
				$elements[] = ['id' => $dstfld3, 'value' => $item[$srcfld3]];
			}
			if ($elements) {
				$js_object['elements'] = $elements;
			}
		}

		if ($js_object) {
			$description->setAttribute('data-object', $js_object);
		}

		$table->addRow([
			($hostid > 0) ? null : $item['hostname'],
			$multiselect
				? new CCheckBox('items['.zbx_jsValue($item[$srcfld1]).']', $item['itemid'])
				: null,
			$description,
			$item['key_'],
			item_type2str($item['type']),
			itemValueTypeString($item['value_type']),
			(new CSpan(itemIndicator($item['status'], $item['state'])))
				->addClass(itemIndicatorStyle($item['status'], $item['state']))
		]);
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Applications
 */
elseif ($srctbl == 'applications') {
	$form = (new CForm())
		->setName('applicationform')
		->setId('applications');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_applications'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_applications', 'applications');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => ['applicationid', 'name'],
		'hostids' => $hostid
	];
	if (is_null($hostid)) {
		$options['groupids'] = $groupids;
	}
	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}
	if (!is_null($templated)) {
		$options['templated'] = $templated;
	}
	$apps = API::Application()->get($options);
	CArrayHelper::sort($apps, ['name']);
	$parentid = $dstfld1 ? $dstfld1 : '';

	foreach ($apps as $app) {
		$table->addRow([
			$multiselect
				? (new CCheckBox('applications['.$app[$srcfld1].']', $app['applicationid']))
				: null,
			(new CLink($app['name'], 'javascript:void(0);'))->setAttribute('data-object', [
				'object' => $reference,
				'values' => [['id' => $app['applicationid'], 'name' => $app['name']]],
				'parentId' => $parentid
			])
		]);
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

/*
 * Graphs or Graph prototypes
 */
elseif ($srctbl === 'graphs' || $srctbl === 'graph_prototypes') {
	$form = (new CForm())
		->setName('graphform')
		->setId('graphs');

	$graphPrototypesPopup = ($srctbl === 'graph_prototypes');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_graphs'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_graphs', 'graphs');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name'),
			_('Graph type')
		]);

	if ($pageFilter->hostsSelected) {
		$options = [
			'output' => API_OUTPUT_EXTEND,
			'hostids' => $hostid,
			'selectHosts' => ['name'],
			'preservekeys' => true
		];

		if (!is_null($writeonly)) {
			$options['editable'] = true;
		}
		if (!is_null($templated)) {
			$options['templated'] = $templated;
		}

		if ($graphPrototypesPopup) {
			$graphs = API::GraphPrototype()->get($options);
		}
		else {
			$graphs = API::Graph()->get($options);
		}
		order_result($graphs, 'name');
	}
	else {
		$graphs = [];
	}

	foreach ($graphs as $graph) {
		$host = reset($graph['hosts']);
		$graph['hostname'] = $host['name'];
		$description = new CLink($graph['name'], 'javascript:void(0);');
		$graph['name'] = $graph['hostname'].NAME_DELIMITER.$graph['name'];
		$js_object = [];

		if ($multiselect) {
			$js_object = [
				'object' => $reference,
				'values' => [$graph],
				'parentId' => null
			];
		}
		else {
			$elements = [];

			if (array_key_exists($srcfld1, $graph)) {
				$elements[] = ['id' => $dstfld1, 'value' => $graph[$srcfld1]];
			}
			if ($srcfld2 && array_key_exists($srcfld2, $graph)) {
				$elements[] = ['id' => $dstfld2, 'value' => $graph[$srcfld2]];
			}
			if ($elements) {
				$js_object['elements'] = $elements;
			}
		}

		if ($js_object) {
			$description->setAttribute('data-object', $js_object);
		}

		switch ($graph['graphtype']) {
			case GRAPH_TYPE_STACKED:
				$graphtype = _('Stacked');
				break;
			case GRAPH_TYPE_PIE:
				$graphtype = _('Pie');
				break;
			case GRAPH_TYPE_EXPLODED:
				$graphtype = _('Exploded');
				break;
			default:
				$graphtype = _('Normal');
				break;
		}
		$table->addRow([
			$multiselect
				? new CCheckBox('graphs['.zbx_jsValue($graph[$srcfld1]).']', $graph['graphid'])
				: null,
			$description,
			$graphtype
		]);
		unset($description);
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}
/*
 * Sysmaps
 */
elseif ($srctbl == 'sysmaps') {
	$form = (new CForm())
		->setName('sysmapform')
		->setId('sysmaps');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_sysmaps'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_sysmaps', 'sysmaps');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$options = [
		'output' => API_OUTPUT_EXTEND,
		'preservekeys' => true
	];
	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}
	$sysmaps = API::Map()->get($options);
	order_result($sysmaps, 'name');

	foreach ($sysmaps as $sysmap) {
		if (isset($excludeids[$sysmap['sysmapid']])) {
			$description = $sysmap['name'];
		}
		else {
			$description = new CLink($sysmap['name'], 'javascript:void(0);');
			$js_object = [];

			if ($multiselect) {
				$js_object = [
					'object' => $reference,
					'values' => [$sysmap],
					'parentId' => null
				];
			}
			else {
				$elements = [];

				if (array_key_exists($srcfld1, $sysmap)) {
					$elements[] = ['id' => $dstfld1, 'value' => $sysmap[$srcfld1]];
				}
				if ($srcfld2 && array_key_exists($srcfld2, $sysmap)) {
					$elements[] = ['id' => $dstfld2, 'value' => $sysmap[$srcfld2]];
				}
				if ($elements) {
					$js_object['elements'] = $elements;
				}
			}

			if ($js_object) {
				$description->setAttribute('data-object', $js_object);
			}
		}

		$table->addRow([
			$multiselect ? new CCheckBox('sysmaps['.zbx_jsValue($sysmap[$srcfld1]).']', $sysmap['sysmapid']) : null,
			$description
		]);
		unset($description);
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}
/*
 * Screens
 */
elseif ($srctbl == 'screens') {
	require_once dirname(__FILE__).'/include/screens.inc.php';

	$form = (new CForm())
		->setName('screenform')
		->setId('screens');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_screens'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_screens', 'screens');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name')
		]);

	$screens = API::Screen()->get([
		'output' => ['screenid', 'name'],
		'preservekeys' => true,
		'editable' => ($writeonly !== null)
	]);
	order_result($screens, 'name');

	foreach ($screens as $screen) {
		$link = new CLink($screen['name'], 'javascript:void(0);');
		$js_object = [];

		if ($multiselect) {
			$js_object = [
				'object' => $reference,
				'values' => [$screen],
				'parentId' => null
			];
		}
		else {
			$elements = [];

			if (array_key_exists($srcfld1, $screen)) {
				$elements[] = ['id' => $dstfld1, 'value' => $screen[$srcfld1]];
			}
			if ($srcfld2 && array_key_exists($srcfld2, $screen)) {
				$elements[] = ['id' => $dstfld2, 'value' => $screen[$srcfld2]];
			}
			if ($elements) {
				$js_object['elements'] = $elements;
			}
		}

		if ($js_object) {
			$link->setAttribute('data-object', $js_object);
		}

		$table->addRow([
			$multiselect ? new CCheckBox('screens['.zbx_jsValue($screen[$srcfld1]).']', $screen['screenid']) : null,
			$link
		]);
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}
/*
 * Screens 2
 */
elseif ($srctbl == 'screens2') {
	require_once dirname(__FILE__).'/include/screens.inc.php';

	$table = (new CTableInfo())->setHeader(_('Name'));

	$screens = API::Screen()->get([
		'output' => ['screenid', 'name'],
		'editable' => ($writeonly !== null)
	]);
	order_result($screens, 'name');

	foreach ($screens as $screen) {
		if (check_screen_recursion($_REQUEST['screenid'], $screen['screenid'])) {
			continue;
		}

		$link = new CLink($screen['name'], 'javascript:void(0);');
		$elements = [];

		if (array_key_exists($srcfld1, $screen)) {
			$elements[] = ['id' => $dstfld1, 'value' => $screen[$srcfld1]];
		}
		if (array_key_exists($srcfld2, $screen)) {
			$elements[] = ['id' => $dstfld2, 'value' => $screen[$srcfld2]];
		}
		if ($elements) {
			$link->setAttribute('data-object', ['elements' => $elements]);
		}

		$table->addRow($link);
	}

	$widget->addItem($table)->show();
}

/*
 * Discovery rules
 */
elseif ($srctbl === 'drules') {
	$table = (new CTableInfo())->setHeader(_('Name'));

	$drules = API::DRule()->get([
		'output' => ['druleid', 'name']
	]);

	order_result($drules, 'name');

	foreach ($drules as $drule) {
		$link = new CLink($drule['name'], 'javascript:void(0);');
		$elements = [];

		if (array_key_exists($srcfld1, $drule)) {
			$elements[] = ['id' => $dstfld1, 'value' => $drule[$srcfld1]];
		}
		if (array_key_exists($srcfld2, $drule)) {
			$elements[] = ['id' => $dstfld2, 'value' => $drule[$srcfld2]];
		}
		if ($elements) {
			$link->setAttribute('data-object', ['elements' => $elements]);
		}

		$table->addRow($link);
	}
	$widget->addItem($table)->show();
}
/*
 * Discovery checks
 */
elseif ($srctbl === 'dchecks') {
	$table = (new CTableInfo())->setHeader(_('Name'));

	$drules = API::DRule()->get([
		'selectDChecks' => ['dcheckid', 'type', 'key_', 'ports'],
		'output' => ['druleid', 'name']
	]);

	order_result($drules, 'name');

	foreach ($drules as $drule) {
		foreach ($drule['dchecks'] as $dcheck) {
			$name = $drule['name'].NAME_DELIMITER.discovery_check2str($dcheck['type'], $dcheck['key_'], $dcheck['ports']);
			$link = (new CLink($name, 'javascript:void(0);'));
			$elements = [];

			if (array_key_exists($srcfld1, $dcheck)) {
				$elements[] = ['id' => $dstfld1, 'value' => $dcheck[$srcfld1]];
			}
			if ($srcfld2) {
				$elements[] = ['id' => $dstfld2, 'value' => $name];
			}
			if ($elements) {
				$link->setAttribute('data-object', ['elements' => $elements]);
			}

			$table->addRow($link);
		}
	}

	$widget->addItem($table)->show();
}
/*
 * Proxies
 */
elseif ($srctbl == 'proxies') {
	$table = (new CTableInfo())->setHeader(_('Name'));

	$result = DBselect(
		'SELECT h.hostid,h.host'.
		' FROM hosts h'.
		' WHERE h.status IN ('.HOST_STATUS_PROXY_ACTIVE.','.HOST_STATUS_PROXY_PASSIVE.')'.
		' ORDER BY h.host,h.hostid'
	);

	while ($row = DBfetch($result)) {
		$link = (new CLink($row['host'], 'javascript:void(0);'));
		$elements = [];

		if (array_key_exists($srcfld1, $row)) {
			$elements[] = ['id' => $dstfld1, 'value' => $row[$srcfld1]];
		}
		if (array_key_exists($srcfld2, $row)) {
			$elements[] = ['id' => $dstfld2, 'value' => $row[$srcfld2]];
		}
		if ($elements) {
			$link->setAttribute('data-object', ['elements' => $elements]);
		}

		$table->addRow($link);
	}

	$widget->addItem($table)->show();
}
/*
 * Scripts
 */
elseif ($srctbl == 'scripts') {
	$form = (new CForm())
		->setName('scriptform')
		->setId('scripts');

	$table = (new CTableInfo())
		->setHeader([
			$multiselect
				? (new CColHeader(
					(new CCheckBox('all_scripts'))
						->onClick("javascript: checkAll('".$form->getName()."', 'all_scripts', 'scripts');")
				))->addClass(ZBX_STYLE_CELL_WIDTH)
				: null,
			_('Name'),
			_('Execute on'),
			_('Commands')
		]);

	$options = [
		'output' => API_OUTPUT_EXTEND,
		'preservekeys' => true
	];
	if (is_null($hostid)) {
		$options['groupids'] = $groupids;
	}
	if (!is_null($writeonly)) {
		$options['editable'] = true;
	}
	$scripts = API::Script()->get($options);
	order_result($scripts, 'name');

	foreach ($scripts as $script) {
		$description = new CLink($script['name'], 'javascript:void(0);');
		$js_object = [];

		if ($multiselect) {
			$js_object = [
				'object' => $reference,
				'values' => [$script],
				'parentId' => null
			];
		}
		else {
			$elements = [];

			if (array_key_exists($srcfld1, $script)) {
				$elements[] = ['id' => $dstfld1, 'value' => $script[$srcfld1]];
			}
			if ($srcfld2 && array_key_exists($srcfld2, $script)) {
				$elements[] = ['id' => $dstfld2, 'value' => $script[$srcfld2]];
			}
			if ($elements) {
				$js_object['elements'] = $elements;
			}
		}

		if ($js_object) {
			$description->setAttribute('data-object', $js_object);
		}

		if ($script['type'] == ZBX_SCRIPT_TYPE_CUSTOM_SCRIPT) {
			switch ($script['execute_on']) {
				case ZBX_SCRIPT_EXECUTE_ON_AGENT:
					$scriptExecuteOn = _('Agent');
					break;
				case ZBX_SCRIPT_EXECUTE_ON_SERVER:
					$scriptExecuteOn = _('Server');
					break;
				case ZBX_SCRIPT_EXECUTE_ON_PROXY:
					$scriptExecuteOn = _('Server (proxy)');
					break;
			}
		}
		else {
			$scriptExecuteOn = '';
		}
		$table->addRow([
			$multiselect ? new CCheckBox('scripts['.zbx_jsValue($script[$srcfld1]).']', $script['scriptid']) : null,
			$description,
			$scriptExecuteOn,
			zbx_nl2br(htmlspecialchars($script['command'], ENT_COMPAT, 'UTF-8')),
		]);
	}

	if ($multiselect) {
		$table->setFooter(
			new CCol(
				(new CButton('select', _('Select')))
					->onClick('addSelectedFormValuesHandler("'.$form->getId().'")')
			)
		);
	}

	$form->addItem($table);
	$widget->addItem($form)->show();
}

require_once dirname(__FILE__).'/include/page_footer.php';
