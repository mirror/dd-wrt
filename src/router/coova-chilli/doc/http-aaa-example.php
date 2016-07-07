<?php
/* 
 * Copyright (C) 2010-2012 David Bird (Coova Technologies)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


// == start main ==

// globals for convenience
global $dblink;
global $hotspot_ap;
global $hotspot_network;
global $hotspot_user;
global $hotspot_device;
global $hotspot_code;
global $hotspot_session;
global $aaa_config;
global $db_config;

// initialize globals
$dblink = null;
$hotspot_ap = null;
$hotspot_network = null;
$hotspot_user = null;
$hotspot_device = null;
$hotspot_code = null;
$hotspot_session = null;
$aaa_config = null;
$db_config = null;

require "aaa-config.php";

#  Your http-aaa-config.php could contain:
#
# <?php
# global $aaa_config;
# global $db_config;
#
# $aaa_config = 
#   array(
#	 'using_swapoctets' => 0,
#	 );
#
# $db_config = 
#   array(
# 	'host' => 'localhost',
#	'user' => 'root',
#	'pass' => '',
#	'db' => 'aaa',
#	'networks_table' => 'networks',
#	'networks_uamsecret_field' => 'uamsecret',
#	);
#

// initialize globals
$hotspot_ap = false;
$hotspot_network = false;
$hotspot_user = false;
$hotspot_device = false;
$hotspot_code = false;
$hotspot_session = false;

$dblink = db_open();

// Look up access point based on MAC address
$hotspot_ap = get_ap();
if (!is_array($hotspot_ap) || !isset($hotspot_ap['network_id'])) {
  echo 'Reply-Message: Access Point not found';
  exit;
}

// Load the network that owns the access point to get uamsecret
$hotspot_network = get_network();
if (!is_array($hotspot_network) || !isset($hotspot_network['id'])) {
  echo 'Reply-Message: Network is not configured correctly';
  exit;
}

// Verify the query string parameters with uamsecret
check_url();

switch ($_GET['stage']) {
 case 'login':
   $sent_reply = false;
   $attrs = array();
   switch ($_GET['service']) {
     case 'login':  $sent_reply = do_login_service($attrs);   break; // Standard login
     case 'framed': $sent_reply = do_macauth_service($attrs); break; // MAC authentication
     case 'admin':  $sent_reply = do_admin_service($attrs);   break; // Admin-User session
   }
   if (!$sent_reply) {
     do_auth_reject(array( // Attribute allowed in access-reject
			  'Reply-Message' => $attrs['Reply-Message'],
			  'WISPr-Redirection-URL' => $attrs['WISPr-Redirection-URL'],
			  'CoovaChilli-Config' => $attrs['CoovaChilli-Config'],
			  ));
   }
   break;

 case 'counters':
  do_accounting();
  break;

 case 'register':
  do_register();
  break;

}

db_close();

// == end main ==


// == functions ==

function set_attribute         ($n, &$a, $v, $o = true) { if ($o || !isset($a[$n])) $a[$n]=$v; }
function set_idle_timeout      (&$a, $v, $o = true) { set_attribute('Idle-Timeout',$a,$v,$o); }
function set_reply_message     (&$a, $v, $o = true) { set_attribute('Reply-Message',$a,$v,$o); } 
function set_session_time      (&$a, $v, $o = true) { set_attribute('Acct-Session-Time',$a,$v,$o); } 
function set_session_timeout   (&$a, $v, $o = true) { set_attribute('Session-Timeout',$a,$v,$o); } 
function set_interim_interval  (&$a, $v, $o = true) { set_attribute('Acct-Interim-Interval',$a,$v,$o); } 
function set_max_total_octets  (&$a, $v, $o = true) { set_attribute('CoovaChilli-Max-Total-Octets',$a,$v,$o); } 
function set_max_input_octets  (&$a, $v, $o = true) { set_attribute('CoovaChilli-Max-Input-Octets',$a,$v,$o); } 
function set_max_output_octets (&$a, $v, $o = true) { set_attribute('CoovaChilli-Max-Output-Octets',$a,$v,$o); } 
function set_max_total_kbytes  (&$a, $v, $o = true) { set_attribute('CoovaChilli-Max-Total-Octets',$a,($v*1000),$o); } 
function set_max_input_kbytes  (&$a, $v, $o = true) { set_attribute('CoovaChilli-Max-Input-Octets',$a,($v*1000),$o); } 
function set_max_output_kbytes (&$a, $v, $o = true) { set_attribute('CoovaChilli-Max-Output-Octets',$a,($v*1000),$o); } 
function set_redirection_url   (&$a, $v, $o = true) { set_attribute('WISPr-Redirection-URL',$a,$v,$o); } 

#function set_max_bandwidth_up_bit_sec     (&$a, $v, $o = true) { set_attribute('WISPr-Bandwidth-Max-Up',$a,$v,$o); } 
#function set_max_bandwidth_down_bit_sec   (&$a, $v, $o = true) { set_attribute('WISPr-Bandwidth-Max-Down',$a,$v,$o); } 
function set_max_bandwidth_up_kbit_sec    (&$a, $v, $o = true) { set_attribute('CoovaChilli-Bandwidth-Max-Up',$a,$v,$o); } 
function set_max_bandwidth_down_kbit_sec  (&$a, $v, $o = true) { set_attribute('CoovaChilli-Bandwidth-Max-Down',$a,$v,$o); } 
function set_max_bandwidth_up_kbyte_sec   (&$a, $v, $o = true) { set_attribute('CoovaChilli-Bandwidth-Max-Up',$a,($v*8),$o); } 
function set_max_bandwidth_down_kbyte_sec (&$a, $v, $o = true) { set_attribute('CoovaChilli-Bandwidth-Max-Down',$a,($v*8),$o); } 
function set_max_bandwidth_up_mbyte_sec   (&$a, $v, $o = true) { set_attribute('CoovaChilli-Bandwidth-Max-Up',$a,($v*8000),$o); } 
function set_max_bandwidth_down_mbyte_sec (&$a, $v, $o = true) { set_attribute('CoovaChilli-Bandwidth-Max-Down',$a,($v*8000),$o); } 

function set_limit_interval(&$a, $v, $o = true) { set_attribute('Meta-Interval',$a,$v,$o); }
function set_limit_value(&$a, $v, $o = true) { set_attribute('Meta-Interval-Value',$a,$v,$o); }

function check_validity($t, &$obj, &$a) {

  if ($obj['valid_from'] == '') {
    db_query('UPDATE '.$t.' SET valid_from = now()', false);
    $obj['valid_from'] =  date('Y-m-d H:i:s', time());
  }

  if ($obj['valid_until'] != '') {
    $d = strtotime($obj['valid_until']);
    $n = time();

    if ($d < $n) {
      if ($obj['repeat_interval']) {
	db_query('UPDATE '.$t.' SET valid_from = now(), valid_until = null', false);
	$obj['valid_from'] =  date('Y-m-d H:i:s', time());
	$obj['valid_until'] = '';
      } else {
	set_reply_message($a, 'Expired', false);
	return false;
      }
    }
  }

  return true;
}

function proc_attributes(&$a) {
  global $aaa_config;
  global $hotspot_user;
  global $hotspot_device;
  global $hotspot_code;

  $login_type = $a['Meta-Login'];
  $interval = $a['Meta-Interval'];
  $interval_value = $a['Meta-Interval-Value'];
  if ($interval_value == '') $interval_value = 1;

  $t = null;

  switch($login_type) {
  case 'device': 
    $obj = $hotspot_device;
    $t = db_table('devices');
    if (!check_validity($t, $obj, $a)) return false;
    $sum = get_device_summary($obj);
    break;
  case 'user': 
    $obj = $hotspot_user;
    $t = db_table('users');
    if (!check_validity($t, $obj, $a)) return false;
    $sum = get_user_summary($obj);
    if ($obj['id'] != $hotspot_device['owner_id']) {
      $sql = 'UPDATE '.db_table('devices').' SET owner_id = '.$obj['id'].' WHERE id = '.$hotspot_device['id'];
      db_query($sql, false);
    }
    break;
  case 'code': 
    $t = db_table('codes');
    $obj = $hotspot_code;
    if (!check_validity($t, $obj, $a)) return false;
    $sum = get_code_summary($obj);
    if ($obj['device_id'] != $hotspot_device['id']) {
      $sql = 'UPDATE '.$t.' SET device_id = '.$hotspot_device['id'].' WHERE id = '.$obj['id'];
      db_query($sql, false);
    }
    break;
  default:
    return false;
  }

  switch($interval) {
  case 'minute':
  case 'hour':
  case 'day':
  case 'month':
  case 'year':
    if ($obj['valid_until'] == '') {
      db_query('UPDATE '.$t.' SET valid_until = DATE_ADD(valid_from, INTERVAL '.$interval_value.' '.$interval.')', false);
    }
    break;

  default:
    break;
  }

  $swap = $aaa_config['using_swapoctets'];

  # Down
  $n = 'CoovaChilli-Max-'.($swap ? 'Out' : 'In').'put-Octets'; 
  $v = $a[$n]; 
  if (isset($v) && $v > 0 && $sum['bytes_down'] > 0) {
    $s = $v - $sum['bytes_down'];

    if ($s <= 0) {
      set_reply_message($a, 'Download data limit reached', false);
      return false;
    }

    $a[$n] = $s;
  }

  # Up
  $n = 'CoovaChilli-Max-'.($swap ? 'In' : 'Out').'put-Octets'; 
  $v = $a[$n]; 
  if (isset($v) && $v > 0 && $sum['bytes_up'] > 0) {
    $s = $v - $sum['bytes_up'];

    if ($s <= 0) {
      set_reply_message($a, 'Upload data limit reached', false);
      return false;
    }

    $a[$n] = $s;
  }

  # Total
  $n = 'CoovaChilli-Max-Total-Octets'; 
  $v = $a[$n]; 
  if (isset($v) && $v > 0 && ($sum['bytes_up'] > 0 || $sum['bytes_down'] > 0)) {
    $s = $v;
    if ($sum['bytes_up']   > 0) $s -= $sum['bytes_up'];
    if ($sum['bytes_down'] > 0) $s -= $sum['bytes_down'];

    if ($s <= 0) {
      set_reply_message($a, 'Total data limit reached', false);
      return false;
    }

    $a[$n] = $s;
  }

  $ses_time = $a['Acct-Session-Time'];
  if (isset($ses_time) && $ses_time > 0) {
    $ses_time -= $sum['seconds'];

    if ($ses_time <= 0) {
      set_reply_message($a, 'Time expired', false);
      return false;
    }

    set_session_timeout($a, $ses_time, true);
    unset($a['Acct-Session-Time']);
  }

  return true;
}

function format_attributes(&$attrs) {
  # format for HTTP API
  foreach ($attrs as $n => $v) {
    if ($n == '' || $v == '') continue;
    if (strpos($n, 'Meta-') === 0) continue;
    echo "$n:$v\n";
  }
}

function do_auth_accept(&$attrs) {

  if (!proc_attributes($attrs)) {
    return false;
  }

  do_acct_status('auth');

  echo "Auth: 1\n";
  if ($attrs) format_attributes($attrs);
  return true;
}

function do_auth_reject($attrs = false) {
  echo "Auth: 0\n";
  if ($attrs) format_attributes($attrs);
  return true;
}

function login_user(&$attrs) {
  $attrs['Meta-Login'] = 'user';
  user_attributes($attrs);
  ap_attributes($attrs);
  network_attributes($attrs);
  return do_auth_accept($attrs);
}

function login_code(&$attrs) {
  $attrs['Meta-Login'] = 'code';
  code_attributes($attrs);
  ap_attributes($attrs);
  network_attributes($attrs);
  return do_auth_accept($attrs);
}

function login_device(&$attrs) {
  $attrs['Meta-Login'] = 'device';
  device_attributes($attrs);
  ap_attributes($attrs);
  network_attributes($attrs);
  return do_auth_accept($attrs);
}

function session_key() {
  return str_replace(array('-','.'),
		     array(),$_GET['sessionid'].$_GET['ap'].
		     $_GET['mac'].$_GET['ip'].$_GET['user']);
}

function do_macauth_service(&$attrs) {
  $device = get_device();

#  echo "Auth: 1\n";
#  echo "Acct-Interim-Interval:3600\n";
#  echo "CoovaChilli-Config:splash\n";
#exit;

  if ($device['always_reject']) {
    return do_auth_reject($attrs);
  } 

  if ($device['always_allow']) {
    if (login_device($attrs))
      return true;
  } 

  if (false) { #XXX
    if ($device['owner_id'] > 0) {
      $user = get_user_by_id($device['owner_id']);

      if ($user) {
	if (login_user($attrs))
	  return true;
      }
    }
  } #XXXX

  $code = get_code_by_device_id($device['id']);

  if ($code) {
    if (login_code($attrs))
      return true;
  } 

  return do_auth_reject($attrs);
}

function do_login_service(&$attrs) {
  $device = get_device();

#echo "Auth: 1
#WISPr-Redirection-URL: http://www.coova.com/
#Acct-Interim-Interval: 1440
#Session-Timeout: 1800
#Idle-Timeout: 1800
#";

  if ($_GET['user'] == $_GET['mac']) {
    $password = 'admpwd';
    if (isset($_GET['chap_id'])) {
      // CHAP Challenge/Response Validation
      $chal = pack('H32', $_GET['chap_chal']);
      $check = md5("\0" . $password . $chal);
      
      if ($check == $_GET['chap_pass']) {
	return login_device($attrs);
      }
    }
    else if ($user_or_code['password'] == $_GET['pass']) {
      return login_device($attrs);
    }
  }


  $login_func = 'login_user';
  $user_or_code = get_user();

  if (!$user_or_code) {
    $login_func = 'login_code';
    $user_or_code = get_code();
  }

  if (is_array($user_or_code)) {
    if (isset($_GET['chap_id'])) {

      // CHAP Challenge/Response Validation
      $chal = pack('H32', $_GET['chap_chal']);
      $check = md5("\0" . $user_or_code['password'] . $chal);

      if ($check == $_GET['chap_pass']) {
	return $login_func($attrs);
      }
    }
    else if ($user_or_code['password'] == $_GET['pass']) {
      return $login_func($attrs);
    }
  }

  set_reply_message($attrs, "Either your username or password did not match our records. [".$_GET['user']."]");
  return do_auth_reject($attrs);
}

function do_admin_service(&$attrs) {
  set_interim_interval($attrs, 300);

  echo "Auth: 1\n";
  echo "Acct-Interim-Interval:120\n";
  echo "CoovaChilli-Config:uamanyip\n";
  echo "CoovaChilli-Config:uamnatanyip\n";
  //echo "CoovaChilli-Config:seskeepalive\n";
  //echo "CoovaChilli-Config:usestatusfile=chilli.status\n";
  echo "CoovaChilli-Config:statip 5.0.0.0/24\n";
  //echo "CoovaChilli-Config:txqlen 1000\n";
  //echo "CoovaChilli-Config:tcpmss 1460\n";
  echo "CoovaChilli-Config:acctupdate\n";
  //echo "CoovaChilli-Config:macreauth\n";
  exit;

  return do_auth_accept($attrs);
}

function do_acct_status($status) {

  $do_admin_acct = false; // Change to 'true', if desired

  if (get_device() || $do_admin_acct) {

    switch($status) {

    case 'update':
      update_session();
      break;
      
    case 'start':
      start_session();
      break;
      
    case 'stop':
      stop_session();
      break;

    case 'auth':
      auth_session();
      break;

    }
  }
}

function do_accounting() {
  $attrs = array();

  do_acct_status($_GET['status']);

  echo "Ack: 1\n";
  if ($attrs) format_attributes($attrs);
}

function check_url() {
  global $hotspot_network;

  $uamsecret = $hotspot_network[db_field('uamsecret')];

  $md = $_GET['md'];

  $check = (empty($_SERVER['HTTPS']) ? 'http' : 'https').'://'.
    $_SERVER['SERVER_NAME'].preg_replace('/&md=[^&=]+$/', '', $_SERVER['REQUEST_URI']);

  $match = strtoupper(md5($check.$uamsecret));

  if ($md == $match) return;

  echo "Reply-Message: bad url or uamsecret [$check $uamsecret]\n";
  exit;
}



// == database ==

/*

drop table networks;
create table networks (
  id serial,
  name varchar(200),

  -- defaults for 'classes' of service

  defcode_redirection_url varchar(200),
  defcode_idle_timeout integer unsigned,
  defcode_kbps_down integer unsigned,
  defcode_kbps_up integer unsigned,
  defcode_limit_interval varchar(10), -- values: minute, hour, day, week, month
  defcode_limit_value integer unsigned, -- times limit_interval
  defcode_repeat_interval boolean default false,
  defcode_session_time integer unsigned,
  defcode_kbytes_total integer unsigned,
  defcode_kbytes_down integer unsigned,
  defcode_kbytes_up integer unsigned,

  defuser_redirection_url varchar(200),
  defuser_idle_timeout integer unsigned,
  defuser_kbps_down integer unsigned,
  defuser_kbps_up integer unsigned,
  defuser_limit_interval varchar(10), -- values: minute, hour, day, week, month
  defuser_limit_value integer unsigned, -- times limit_interval
  defuser_repeat_interval boolean default false,
  defuser_session_time integer unsigned,
  defuser_kbytes_total integer unsigned,
  defuser_kbytes_down integer unsigned,
  defuser_kbytes_up integer unsigned,

  defdev_always_allow boolean default false,
  defdev_redirection_url varchar(200),
  defdev_idle_timeout integer unsigned,
  defdev_kbps_down integer unsigned,
  defdev_kbps_up integer unsigned,
  defdev_limit_interval varchar(10), -- values: minute, hour, day, week, month
  defdev_limit_value integer unsigned, -- times limit_interval
  defdev_repeat_interval boolean default false,
  defdev_session_time integer unsigned,
  defdev_kbytes_total integer unsigned,
  defdev_kbytes_down integer unsigned,
  defdev_kbytes_up integer unsigned,

  uamsecret varchar(200),

  KEY(name),
  PRIMARY KEY(id)
);

drop table users;
create table users (
  id serial,
  network_id bigint unsigned,
  username varchar(200),
  password varchar(200),
  email varchar(200),

  -- attributes for acces control
  redirection_url varchar(200),
  idle_timeout integer unsigned,
  kbps_down integer unsigned,
  kbps_up integer unsigned,
  limit_interval varchar(10), -- values: minute, hour, day, week, month
  limit_value integer unsigned, -- times limit_interval
  repeat_interval boolean default false,
  session_time integer unsigned,
  kbytes_total integer unsigned,
  kbytes_down integer unsigned,
  kbytes_up integer unsigned,
  valid_from datetime,
  valid_until datetime,

  created datetime,
  FOREIGN KEY (network_id) REFERENCES networks(id),
  UNIQUE KEY (network_id, username),
  KEY(created),
  KEY(username),
  KEY(email),
  PRIMARY KEY(id)
);

drop table devices;
create table devices (
  id serial,
  network_id bigint unsigned,
  owner_id bigint unsigned,
  mac_address varchar(200),

  -- attributes for acces control
  always_reject boolean default false,
  reply_message varchar(200),
  -- the following default from 'networks'
  redirection_url varchar(200),
  always_allow boolean default false,
  idle_timeout integer unsigned,
  kbps_down integer unsigned,
  kbps_up integer unsigned,
  limit_interval varchar(10), -- values: minute, hour, day, week, month
  limit_value integer unsigned, -- times limit_interval
  repeat_interval boolean default false,
  session_time integer unsigned,
  kbytes_total integer unsigned,
  kbytes_down integer unsigned,
  kbytes_up integer unsigned,
  valid_from datetime,
  valid_until datetime,

  created datetime,

  FOREIGN KEY (network_id) REFERENCES networks(id),
  FOREIGN KEY (owner_id) REFERENCES users(id),
  KEY(mac_address),
  PRIMARY KEY(id)
);

drop table codes;
create table codes (
  id serial,
  network_id bigint unsigned,
  device_id bigint unsigned,
  username varchar(200),
  password varchar(200),

  -- attributes for acces control
  redirection_url varchar(200),
  idle_timeout integer unsigned,
  kbps_down integer unsigned,
  kbps_up integer unsigned,
  limit_interval varchar(10), -- values: minute, hour, day, week, month
  limit_value integer unsigned, -- times limit_interval
  repeat_interval boolean default false,
  session_time integer unsigned,
  kbytes_total integer unsigned,
  kbytes_down integer unsigned,
  kbytes_up integer unsigned,
  valid_from datetime,
  valid_until datetime,

  created datetime,
  FOREIGN KEY (network_id) REFERENCES networks(id),
  FOREIGN KEY (device_id) REFERENCES devices(id),
  UNIQUE KEY (network_id, username),
  KEY(created),
  KEY(username),
  PRIMARY KEY(id)
);

drop table aps;
create table aps (
  id serial,
  network_id bigint unsigned,
  mac_address varchar(200),
  FOREIGN KEY (network_id) REFERENCES networks(id),
  KEY(mac_address),
  PRIMARY KEY(id)
);

drop table attributes;
create table attributes (
  key_id bigint unsigned not null,
  resource varchar(16) not null,
  name varchar(200) not null,
  value varchar(200),
  overwrite boolean default true,
  orderby integer default 0,
  KEY(orderby),
  KEY(key_id),
  KEY(resource),
  KEY(name)
);

drop table sessions;
create table sessions (
  id serial,
  ap_id bigint unsigned,
  network_id bigint unsigned,
  device_id bigint unsigned,
  user_id bigint unsigned,
  code_id bigint unsigned,
  bytes_up bigint unsigned,      -- bytes uploaded by user
  bytes_down bigint unsigned,    -- bytes downloaded by user
  duration bigint unsigned,      -- duration in seconds
  auth_time datetime,            -- set to now() at authentication
  start_time datetime,           -- set to now() on accounting start
  update_time datetime,          -- set to now() on accounting start,update,stop
  stop_time datetime,            -- set to now() on accounting stop
  session_key varchar(200),      -- a unique key generated from session data
  FOREIGN KEY (ap_id) REFERENCES aps(id),
  FOREIGN KEY (network_id) REFERENCES networks(id),
  FOREIGN KEY (device_id) REFERENCES devices(id),
  FOREIGN KEY (user_id) REFERENCES users(id),
  FOREIGN KEY (code_id) REFERENCES codes(id),
  KEY(session_key),
  KEY(bytes_up),
  KEY(bytes_down),
  KEY(duration),
  KEY(auth_time),
  KEY(start_time),
  KEY(update_time),
  KEY(stop_time),
  PRIMARY KEY(id)
);


*/

function db_open() {
  global $db_config;

  $dblink = mysql_connect($db_config['host'], 
			 $db_config['user'], 
			 $db_config['pass'])
    or die('Could not connect: ' . mysql_error());

  mysql_select_db($db_config['db']) 
    or die('Could not select database');

  return $dblink;
}

function db_query($query, $is_select = true) {
  global $dblink;
  $return = array();
  $result = mysql_query($query, $dblink) or die('Query failed: ' . mysql_error());
  if ($is_select && isset($result)) {
    while ($line = mysql_fetch_array($result, MYSQL_ASSOC)) $return[] = $line;
    mysql_free_result($result);
  }
  return $return;
}

function db_lastid() {
  global $dblink;
  return mysql_insert_id($dblink);
}

function db_close() {
  global $dblink;
  mysql_close($dblink);
}

function db_table($t) {
  global $db_config;
  if ($db_config["table_$t"]) 
    return $db_config["table_$t"];
  return $t;
}

function db_field($f) {
  global $db_config;
  if ($db_config["field_$f"]) 
    return $db_config["field_$f"];
  return $f;
}

function get_user() {
  global $hotspot_user;
  if ($hotspot_user) return $hotspot_user;
  $username = $_GET['user'];
  $network = get_network();
  $result = db_query('SELECT * FROM '.db_table('users').' WHERE username = \''.$username.'\' '.
		     'AND network_id = '.$network['id']);
  if (is_array($result)) return $hotspot_user = $result[0];
  return null;
}

function get_code() {
  global $hotspot_code;
  if ($hotspot_code) return $hotspot_code;
  $username = $_GET['user'];
  $network = get_network();
  $result = db_query('SELECT * FROM '.db_table('codes').' WHERE username = \''.$username.'\' '.
		     'AND network_id = '.$network['id']);
  if (is_array($result)) return $hotspot_code = $result[0];
  return null;
}

function get_user_by_id($id) {
  global $hotspot_user;
  if ($hotspot_user) return $hotspot_user;
  $result = db_query('SELECT * FROM '.db_table('users').' WHERE id = '.$id);
  if (is_array($result)) return $hotspot_user = $result[0];
  return null;
}

function get_code_by_device_id($id) {
  global $hotspot_code;
  if ($hotspot_code) return $hotspot_code;
  $result = db_query('SELECT * FROM '.db_table('codes').' WHERE device_id = '.$id);
  if (is_array($result)) return $hotspot_code = $result[0];
  return null;
}

function value_fmt(&$a, $n) {
  $v = $a[$n];
  if (!isset($v) || !is_numeric($v)) return '0';
  return $v;
}

function access_control_fields() {
  return 'redirection_url, idle_timeout, kbps_down, kbps_up, limit_interval, limit_value, repeat_interval, session_time, kbytes_total, kbytes_down, kbytes_up';
}

function access_control_values(&$network, $prefix) {
  return '\''.$network[$prefix.'_redirection_url'].'\''.
    ', '.value_fmt($network,$prefix.'_idle_timeout').
    ', '.value_fmt($network,$prefix.'_kbps_down').
    ', '.value_fmt($network,$prefix.'_kbps_up').
    ', \''.$network[$prefix.'_limit_interval'].'\''.
    ', '.value_fmt($network,$prefix.'_limit_value').
    ', '.value_fmt($network,$prefix.'_repeat_interval').
    ', '.value_fmt($network,$prefix.'_session_time').
    ', '.value_fmt($network,$prefix.'_kbytes_total').
    ', '.value_fmt($network,$prefix.'_kbytes_down').
    ', '.value_fmt($network,$prefix.'_kbytes_up');
}

function get_device() {
  global $hotspot_device;
  if ($hotspot_device) return $hotspot_device;

  $mac = $_GET['mac'];
  if (!isset($mac) || $mac == '') return false;

  $network = get_network();

  $sql = 'SELECT * FROM '.db_table('devices').' WHERE mac_address = \''.$mac.'\' AND network_id = '.$network['id'];

  $result = db_query($sql);

  if (is_array($result) && is_array($result[0])) return $hotspot_device = $result[0];

  $sql2 = 'INSERT INTO '.db_table('devices').' (network_id, mac_address, always_allow, valid_from, created, '.access_control_fields().') '.
    'VALUES ('.$network['id'].',\''.$mac.
    '\', \''.$network['defdev_always_allow'].
    '\', now(), now(), '.access_control_values($network, 'defdev').')';

  db_query($sql2, false);

  $result = db_query($sql);
  $result = db_query($sql);

  if (is_array($result)) return $hotspot_device = $result[0];
  return null;
}

function get_ap() {
  global $hotspot_ap;
  if ($hotspot_ap) return $hotspot_ap;

  $sql = 'SELECT * FROM '.db_table('aps').' WHERE mac_address = \''.$_GET['ap'].'\'';

  $result = db_query($sql);

  if (is_array($result)) $hotspot_ap = $result[0];
  return $hotspot_ap;
}

function get_network() {
  global $hotspot_ap;
  global $hotspot_network;

  if ($hotspot_network) return $hotspot_network;
  if (!$hotspot_ap) return false;

  $sql = 'SELECT * FROM '.db_table('networks').' WHERE id = \''.$hotspot_ap['network_id'].'\'';

  $result = db_query($sql);

  if (is_array($result)) return $hotspot_network = $result[0];
  return null;
}

function get_session_summary($col, $obj, $since = '') {

  if ($obj['valid_from']) {
    $since = $obj['valid_from'];
  }

  if ($since != '') {
    $since = ' AND start_time >= \''.$since.'\'';
  }

  $sql = 'SELECT sum(duration) as seconds, sum(bytes_up) as bytes_up, '.
    'sum(bytes_down) as bytes_down FROM '.db_table('sessions').' WHERE '.$col.' = '.$obj['id'].$since;

  foreach (db_query($sql) as $n) return $n;
}

function get_user_summary(&$user) {
  return get_session_summary('user_id', $user);
}

function get_code_summary(&$code) {
  return get_session_summary('code_id', $code);
}

function get_device_summary(&$device) {
  return get_session_summary('device_id', $device);
}

function get_attributes($id, $tbl, &$array) {

  $sql = 'SELECT orderby, name, value, overwrite FROM '.db_table('attributes').
    ' WHERE key_id = \''.$id.'\' AND resource = \''.$tbl.'\' order by orderby';

  $result = db_query($sql);

  if (is_array($result)) {
    foreach ($result as $row) {
      if ($row['overwrite'] == 1 || !isset($array[$row['name']])) {
	$array[$row['name']] = $row['value'];
      }
    }
  }
}

function code_update_device_id(&$code, $id) {
}

function user_attributes(&$array, $user = false) {
  if (!$user) $user = get_user();

  obj_attributes($user, $array);
  get_attributes($user['id'], 'users', $array);
}

function code_attributes(&$array, $code = false) {
  if (!$code) $code = get_code();

  obj_attributes($code, $array);
  get_attributes($code['id'], 'codes', $array);
}

function network_attributes(&$array, $network = false) {
  if (!$network) $network = get_network();

  obj_attributes($network, $array);
  get_attributes($network['id'], 'networks', $array);
}

function ap_attributes(&$array, $ap = false) {
  if (!$ap) $ap = get_ap();

  obj_attributes($ap, $array);
  get_attributes($ap['id'], 'aps', $array);
}

function device_attributes(&$array, $device = false) {
  if (!$device) $device = get_device();

  obj_attributes($device, $array);
  get_attributes($device['id'], 'devices', $array);
}

function obj_attributes(&$obj, &$array) {
  global $aaa_config;

  $swap = $aaa_config['using_swapoctets'];

  $a = 
    array('reply_message'   => 'set_reply_message',
	  'redirection_url' => 'set_redirection_url',
	  'idle_timeout'    => 'set_idle_timeout',
	  'session_time'    => 'set_session_time',
	  'kbps_down'       => 'set_max_bandwidth_down_kbit_sec',
	  'kbps_up'         => 'set_max_bandwidth_up_kbit_sec',
	  'kbytes_total'    => 'set_max_total_kbytes',
	  'kbytes_down'     => 'set_max_'.($swap ? 'out' : 'in').'put_kbytes',
	  'kbytes_up'       => 'set_max_'.($swap ? 'in' : 'out').'put_kbytes',
	  'limit_interval'  => 'set_limit_interval',
	  'limit_value'     => 'set_limit_value',
	  );

  foreach ($a as $s => $f) {
    if (isset($obj[$s])) $f($array, $obj[$s], false);
  }
}

function save_attributes($id, $tbl, &$array, $overwrite = 1) {
  $sql = 'DELETE FROM '.db_table('attributes').' WHERE key_id = \''.$id.'\' AND resource = \''.$tbl.'\'';
  db_query($sql, false);
  foreach ($array as $n => $v) {
    if ($n == '' || $v == '') continue;
    $sql = 'INSERT INTO '.db_table('attributes').' (key_id,resource,name,value,overwrite) '.
      'VALUES (\''.$id.'\',\''.$tbl.'\',\''.$n.'\',\''.$v.'\','.$overwrite.')';
    db_query($sql, false);
  }
}

function save_user_attributes(&$array, $user = false) {
  if (!$user) $user = get_user();
  return save_attributes($user['id'], 'users', $array);
}

function save_code_attributes(&$array, $user = false) {
  if (!$code) $code = get_code();
  return save_attributes($code['id'], 'codes', $array);
}

function save_network_attributes(&$array, $network = false) {
  if (!$network) $network = get_network();
  return save_attributes($network['id'], 'networks', $array);
}

function save_device_attributes(&$array, $device = false) {
  if (!$device) $device = get_device();
  return save_attributes($device['id'], 'devices', $array);
}

function save_ap_attributes(&$array, $ap = false) {
  if (!$ap) $ap = get_ap();
  return save_attributes($ap['id'], 'aps', $array);
}

function auth_session() {
  $network = get_network();
  $device = get_device();
  $user = get_user();
  $ap = get_ap();

  if (!$user)
    $code = get_code();

  $s = date("Y-m-d H:i:s", time());

  $sql = 'INSERT INTO '.db_table('sessions').' (ap_id, network_id, device_id, user_id, code_id, auth_time, session_key) '.
    'VALUES ('.($ap ? $ap['id'] : 'null').','.
    ($network ? $network['id'] : 'null').','.
    ($device ? $device['id'] : 'null').','.
    ($user ? $user['id'] : 'null').','.
    ($code ? $code['id'] : 'null').','.
    '\''.$s.'\',\''.session_key().'\')';

  db_query($sql, false);
}

function start_session() {
  $s = date("Y-m-d H:i:s", time());

  $sql = 'UPDATE '.db_table('sessions').' SET start_time=\''.$s.'\',update_time=\''.$s.
    '\' WHERE session_key = \''.session_key().'\'';

  db_query($sql, false);
}

function _value_int($n) {
  $v = $_GET[$n];
  if ($v == '') $v = "0";
  return $v;
}

function _ses_update() {
  $result .= ',bytes_up='._value_int('bytes_up');
  $result .= ',bytes_down='._value_int('bytes_down');
  $result .= ',duration='._value_int('duration');
  return $result;
}

function stop_session() {
  $s = date("Y-m-d H:i:s", time());

  $sql = 'UPDATE '.db_table('sessions').' SET stop_time=\''.$s.'\',update_time=\''.$s.'\''._ses_update().
    ' WHERE session_key = \''.session_key().'\'';

  db_query($sql, false);
}

function update_session() {
  $s = date("Y-m-d H:i:s", time());

  $sql = 'UPDATE '.db_table('sessions').' SET update_time=\''.$s.'\''._ses_update().
    ' WHERE session_key = \''.session_key().'\'';

  db_query($sql, false);
}

function do_register() {
  $network = get_network();

  switch ($_GET['status']) {

  case 'check':

    $user_or_code = get_user();

    if (!$user_or_code) {
      $user_or_code = get_code();
    }

    if (!$user_or_code) {
      echo 'available';
    } else {
      echo 'taken';
    }

    return;

  case 'new_code':
    $sql = 'INSERT INTO '.db_table('codes').' (network_id, username, password, valid_from, created, '.access_control_fields().') '.
      'VALUES ('.$network['id'].',\''.$_GET['user'].'\',\''.$_GET['pass'].'\', now(), now(), '.access_control_values($network, 'defcode').')';

    $resource = 'codes';

    break;

  case 'new_user':
    $sql = 'INSERT INTO '.db_table('users').' (network_id, username, password, valid_from, created, '.access_control_fields().') '.
      'VALUES ('.$network['id'].',\''.$_GET['user'].'\',\''.$_GET['pass'].'\', now(), now(), '.access_control_values($network, 'defuser').')';

    $resource = 'users';
    break;
  }

  db_query($sql, false);
  $id = db_lastid();

  if ($id) {
    $input = $_POST;

    if (!$input) 
      $input = file_get_contents("php://input");

    $lines = preg_split("/\n+/",$input);
    $attrs = array();

    foreach ($lines as $line) {
      $p = preg_split('/[=: ]+/', $line, 2);
      if ($p[0] && $p[1])
	$attrs[$p[0]] = $p[1];
    }

    save_attributes($id, $resource, $attrs);
  }
}

