<?php
/* (c) Blue Wave Projects and Services 2015-2023. This software is released under the GNU GPL license.

 This is a FAS script providing an example of remote Forward Authentication for openNDS (NDS) on an http web server supporting PHP.

 The following NDS configurations must be set:
 1. fasport: Set to the port number the remote webserver is using (typically port 443)

 2. faspath: This is the path from the FAS Web Root to the location of this FAS script (not from the file system root).
	eg. /nds/fas-aes-https.php

 3. fasremoteip: The remote IPv4 address of the remote server eg. 46.32.240.41

 4. fasremotefqdn: The fully qualified domain name of the remote web server.
	This is required in the case of a shared web server (ie. a server that hosts multiple domains on a single IP),
	but is optional for a dedicated web server (ie. a server that hosts only a single domain on a single IP).
	eg. onboard-wifi.net

 5. faskey: Matching $key as set in this script (see below this introduction).
	This is a key phrase for NDS to encrypt the query string sent to FAS.
	It can be any combination of A-Z, a-z and 0-9, up to 16 characters with no white space.
	eg 1234567890

 6. fas_secure_enabled:  set to level 3
	The NDS parameters: clientip, clientmac, gatewayname, client token, gatewayaddress, authdir and originurl
	are encrypted using fas_key and passed to FAS in the query string.

	The query string will also contain a randomly generated initialization vector to be used by the FAS for decryption.

	The "php-cli" package and the "php-openssl" module must both be installed for fas_secure level 2 and above.

 openNDS does not have "php-cli" and "php-openssl" as dependencies, but will exit gracefully at runtime if this package and module
 are not installed when fas_secure_enabled is set to level 2 or 3.

 The FAS must use the initialisation vector passed with the query string and the pre shared faskey to decrypt the required information.

 The remote web server (that runs this script) must have the "php-openssl" module installed (standard for most hosting services).

 This script requires the client user to enter their Fullname and email address. This information is stored in a log file kept
 in the same folder as this script.

 This script requests the client CPD to display the NDS avatar image directly from Github.

 This script displays an example Terms of Service. **You should modify this for your local legal juristiction**.

 The script is provided as a fully functional https splash page sequence.
 In its present form it does not do any verification, but serves as an example for customisation projects.

 The script retreives the clientif string sent from NDS and displays it on the login form.
 "clientif" is of the form [client_local_interface] [remote_meshnode_mac] [local_mesh_if]
 The returned values can be used to dynamically modify the login form presented to the client,
 depending on the interface the client is connected to.
 eg. The login form can be different for an ethernet connection, a private wifi, a public wifi or a remote mesh network zone.

*/

// Set the pre-shared key. This **MUST** be the same as faskey in the openNDS config:
$key="1234567890";

// Allow immediate flush to browser
if (ob_get_level()){ob_end_clean();}

//force redirect to secure page
if(empty($_SERVER['HTTPS']) || $_SERVER['HTTPS'] == "off"){
    $redirect = 'https://' . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
    header('HTTP/1.1 301 Moved Permanently');
    header('Location: ' . $redirect);
    exit(0);
}

// setup basic defaults
date_default_timezone_set("UTC");
$client_zone=$fullname=$email=$invalid="";
$me=$_SERVER['SCRIPT_NAME'];

// Set logpath
if (file_exists("/etc/config/opennds")) {
	$logpath="/tmp/";
} elseif (file_exists("/etc/opennds/opennds.conf")) {
	$logpath="/run/";
} else {
	$logpath="";
}


/*Configure Quotas - Time, Data and Data Rate - Override global settings (openNDS config values or defaults)

	Description of values that can be set:
	1. Set the session length(minutes), upload/download quotas(kBytes), upload/download rates(kbits/s)
		and custom string to be sent to the BinAuth script.
	2. Upload and download quotas are in kilobytes.
		If a client exceeds its upload or download quota it will be deauthenticated on the next cycle of the client checkinterval.
		(see openNDS config for checkinterval)

	3. Client Upload and Download Rates are the average rates a client achieves since authentication
		If a client exceeds its set upload or download rate it will be deauthenticated on the next cycle of the client checkinterval.

	**Note** - The following variables are set on a client by client basis. If a more sophisticated client credential verification was implemented,
		these variables could be set dynamically.

	In addition, choice of the values of these variables can be determined, based on the interface used by the client
		(as identified by the clientif parsed variable). For example, a system with two wireless interfaces such as "members" and "guests".

	A value of 0 means no limit
*/

$sessionlength=0; // minutes (1440 minutes = 24 hours)
$uploadrate=0; // kbits/sec (500 kilobits/sec = 0.5 Megabits/sec)
$downloadrate=0; // kbits/sec (1000 kilobits/sec = 1.0 Megabits/sec)
$uploadquota=0; // kBytes (500000 kiloBytes = 500 MegaBytes)
$downloadquota=0; // kBytes (1000000 kiloBytes = 1 GigaByte)


/* define a remote image to display
	eg. https://avatars1.githubusercontent.com/u/62547912 is the openNDS Portal Lens Flare
	$imagepath is used function footer()
*/

$imageurl="https://avatars1.githubusercontent.com/u/62547912";
$imagetype="png";
$scriptname=basename($_SERVER['SCRIPT_NAME']);
$imagepath=htmlentities("$scriptname?get_image=$imageurl&imagetype=$imagetype");


###################################
#Begin processing inbound requests:
###################################

// Send The Auth List when requested by openNDS (authmon daemon)
auth_get();

// Service requests for remote image
if (isset($_GET["get_image"])) {
	$url=$_GET["get_image"];
	$imagetype=$_GET["imagetype"];
	get_image($url, $imagetype);
	exit(0);
}

// Get the query string components
if (isset($_GET['status'])) {
	@$redir=$_GET['redir'];
	@$redir_r=explode("fas=", $redir);
	@$fas=$redir_r[1];

	if (isset($_GET['iv'])) {
		$iv=$_GET['iv'];
	} else {
		$iv="error";
	}

} else if (isset($_GET['fas']))  {
	$fas=$_GET['fas'];

	if (isset($_GET['iv'])) {
		$iv=$_GET['iv'];
	} else {
		$iv="error";
	}
} else {
	exit(0);
}

//Decrypt and Parse the querystring
decrypt_parse();

if ( ! isset($clientmac) ) {
	//Encryption error
	err403();
	exit(0);
}

// Extract the client zone:
$client_zone_r=explode(" ",trim($clientif));

if ( ! isset($client_zone_r[1])) {
	$client_zone="LocalZone:".$client_zone_r[0];
} else {
	$client_zone="MeshZone:".str_replace(":","",$client_zone_r[1]);
}

/* Create auth list directory for this gateway
	This list will be sent to NDS when it requests it.
*/
$gwname=hash('sha256', trim($gatewayname));

if (!file_exists("$logpath"."$gwname")) {
	mkdir("$logpath"."$gwname", 0700);
}

#######################################################
//Start Outputing the requested responsive page:
#######################################################

splash_header();

if (isset($_GET["terms"])) {
	// ToS requested
	display_terms();
	footer();
} elseif (isset($_GET["status"])) {
	// The status page is triggered by a client if already authenticated by openNDS (eg by clicking "back" on their browser)
	status_page();
	footer();
} elseif (isset($_GET["auth"])) {
	# Verification is complete so now wait for openNDS to authenticate the client.
	authenticate_page();
	footer();
} elseif (isset($_GET["landing"])) {
	// The landing page is served to the client after openNDS authentication, but many CPDs will immediately close so this page might not be seen
	landing_page();
	footer();
} else {
	login_page();
	footer();
}

// Functions:

function decrypt_parse() {
	/*
	Decrypt and Parse the querystring
		Note: $ndsparamlist is an array of parameter names to parse for.
			Add your own custom parameters to **this array** as well as to the **config file**.
			"admin_email" and "location" are examples of custom parameters.
	*/

	$cipher="AES-256-CBC";
	$ndsparamlist=explode(" ", "clientip clientmac client_type gatewayname gatewayurl version hid gatewayaddress gatewaymac originurl clientif admin_email location");

	if (isset($_GET['fas']) and isset($_GET['iv']))  {
		$string=$_GET['fas'];
		$iv=$_GET['iv'];
		$decrypted=openssl_decrypt( base64_decode( $string ), $cipher, $GLOBALS["key"], 0, $iv );
		$dec_r=explode(", ",$decrypted);

		foreach ($ndsparamlist as $ndsparm) {
			foreach ($dec_r as $dec) {
				@list($name,$value)=explode("=",$dec);
				if ($name == $ndsparm) {
					$GLOBALS["$name"] = $value;
					break;
				}
			}
		}
	}
}

function get_image($url, $imagetype) {
	// download the requested remote image
	header("Content-type: image/$imagetype");
	readfile($url);
}

function auth_get_custom() {
	// Add your own function to handle auth_get custom payload
	$payload_decoded=base64_decode($_POST["payload"]);

	$logpath=$GLOBALS["logpath"];
	$log=date('Y-m-d H:i:s', $_SERVER['REQUEST_TIME']).
		", $payload_decoded\n";

	if ($logpath == "") {
		$logfile="ndslog/customlog_log.php";

		if (!file_exists($logfile)) {
			@file_put_contents($logfile, "<?php exit(0); ?>\n");
		}
	} else {
		$logfile="$logpath"."ndslog/customlog_log.log";
	}

	@file_put_contents($logfile, $log,  FILE_APPEND );

	echo "ack";

}

function auth_get_deauthed() {
	// Add your own function to handle auth_get deauthed payload
	// By default it isappended to the FAS deauth log
	$payload_decoded=base64_decode($_POST["payload"]);

	$logpath=$GLOBALS["logpath"];
	$log=date('Y-m-d H:i:s', $_SERVER['REQUEST_TIME']).
		", $payload_decoded\n";

	if ($logpath == "") {
		$logfile="ndslog/deauthlog_log.php";

		if (!file_exists($logfile)) {
			@file_put_contents($logfile, "<?php exit(0); ?>\n");
		}
	} else {
		$logfile="$logpath"."ndslog/deauthlog_log.log";
	}

	@file_put_contents($logfile, $log,  FILE_APPEND );

	echo "ack";


}

function auth_get() {
	/* Send and/or clear the Auth List when requested by openNDS
		When a client was verified, their parameters were added to the "auth list"
		The auth list is sent to openNDS when authmon requests it.

		auth_get:
		auth_get is sent by authmon or libopennds in a POST request and can have the following values:

		1. Value "list".
			FAS sends the auth list and deletes each client entry currently on that list.

		2. Value "view".
			FAS checks the received payload for an ack list of successfully authenticated clients from previous auth lists.
			Clients on the auth list are only deleted if they are in a received ack list.
			Authmon will have sent the ack list as acknowledgement of all clients that were successfully authenticated in the previous auth list.
			Finally FAS replies by sending the next auth list.
			"view" is the default method used by authmon.

		3. Value "clear".
			This is a housekeeping function and is called by authmon on startup of openNDS.
			The auth list is cleared as any entries held by this FAS at the time of openNDS startup will be stale.

		4. Value "deauthed".
			FAS receives a payload containing notification of deauthentication of a client and the reason for that notification.
			FAS replies with an ack., confirming reception of the notification.

		5. Value "custom".
			FAS receives a payload containing a b64 encoded string to be used by FAS to provide custom functionality.
			FAS replies with an ack., confirming reception of the custom string.
	*/

	$logpath=$GLOBALS["logpath"];

	if (isset($_POST["auth_get"])) {

		if (isset($_POST["gatewayhash"])) {
			$gatewayhash=$_POST["gatewayhash"];
		} else {
			# invalid call, so:
			exit(0);
		}

		if ($_POST["auth_get"] == "deauthed") {
			auth_get_deauthed();
			exit(0);
		}

		if ($_POST["auth_get"] == "custom") {
			auth_get_custom();
			exit(0);
		}

		if (! file_exists("$logpath"."$gatewayhash")) {
			# no clients waiting, so:
			exit(0);
		}

		if ($_POST["auth_get"] == "clear") {
			$auth_list=scandir("$logpath"."$gatewayhash");
			array_shift($auth_list);
			array_shift($auth_list);

			foreach ($auth_list as $client) {
				unlink("$logpath"."$gatewayhash/$client");
			}
			# Stale entries cleared, so:
			exit(0);
		}

		# Set default empty authlist:
		$authlist="*";

		$acklist=base64_decode($_POST["payload"]);

		if ($_POST["auth_get"] == "list") {
			$auth_list=scandir("$logpath"."$gatewayhash");
			array_shift($auth_list);
			array_shift($auth_list);

			foreach ($auth_list as $client) {
				$clientauth=file("$logpath"."$gatewayhash/$client");
				$authlist=$authlist." ".rawurlencode(trim($clientauth[0]));
				unlink("$logpath"."$gatewayhash/$client");
			}
			echo trim("$authlist");

		} else if ($_POST["auth_get"] == "view") {

			if ($acklist != "none") {
				$acklist_r=explode("\n",$acklist);

				foreach ($acklist_r as $client) {
					$client=ltrim($client, "* ");

					if ($client != "") {
						if (file_exists("$logpath"."$gatewayhash/$client")) {
							unlink("$logpath"."$gatewayhash/$client");
						}
					}
				}
				echo "ack";
			} else {
				$auth_list=scandir("$logpath"."$gatewayhash");
				array_shift($auth_list);
				array_shift($auth_list);

				foreach ($auth_list as $client) {
					$clientauth=file("$logpath"."$gatewayhash/$client");
					$authlist=$authlist." ".rawurlencode(trim($clientauth[0]));
				}
			echo trim("$authlist");
			}
		}
		exit(0);
	}
}

function write_log() {
	# In this example we have decided to log all clients who are granted access
	# Note: the web server daemon must have read and write permissions to the folder defined in $logpath
	# By default $logpath is null so the logfile will be written to the folder this script resides in,
	# or the /tmp directory if on the NDS router

	$logpath=$GLOBALS["logpath"];

	if (!file_exists("$logpath"."ndslog")) {
		mkdir("$logpath"."ndslog", 0700);
	}

	$me=$_SERVER['SCRIPT_NAME'];
	$script=basename($me, '.php');
	$host=$_SERVER['HTTP_HOST'];
	$user_agent=$_SERVER['HTTP_USER_AGENT'];
	$clientip=$GLOBALS["clientip"];
	$clientmac=$GLOBALS["clientmac"];
	$client_type=$GLOBALS["client_type"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayaddress=$GLOBALS["gatewayaddress"];
	$gatewaymac=$GLOBALS["gatewaymac"];
	$clientif=$GLOBALS["clientif"];
	$originurl=$GLOBALS["originurl"];
	$redir=rawurldecode($originurl);
	if (isset($_GET["fullname"])) {
		$fullname=$_GET["fullname"];
	} else {
		$fullname="na";
	}

	if (isset($_GET["email"])) {
		$email=$_GET["email"];
	} else {
		$email="na";
	}

	$log=date('Y-m-d H:i:s', $_SERVER['REQUEST_TIME']).
		", $script, $gatewayname, $fullname, $email, $clientip, $clientmac, $client_type, $clientif, $user_agent, $redir\n";

	if ($logpath == "") {
		$logfile="ndslog/ndslog_log.php";

		if (!file_exists($logfile)) {
			@file_put_contents($logfile, "<?php exit(0); ?>\n");
		}
	} else {
		$logfile="$logpath"."ndslog/ndslog.log";
	}

	@file_put_contents($logfile, $log,  FILE_APPEND );
}

###################################
// Functions used to generate html:
###################################

function authenticate_page() {
	# Display a "logged in" landing page once NDS has authenticated the client.
	# or a timed out error if we do not get authenticated by NDS
	$me=$_SERVER['SCRIPT_NAME'];
	$host=$_SERVER['HTTP_HOST'];
	$clientip=$GLOBALS["clientip"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayaddress=$GLOBALS["gatewayaddress"];
	$gatewaymac=$GLOBALS["gatewaymac"];
	$hid=$GLOBALS["hid"];
	$key=$GLOBALS["key"];
	$clientif=$GLOBALS["clientif"];
	$originurl=$GLOBALS["originurl"];
	$redir=rawurldecode($originurl);
	$sessionlength=$GLOBALS["sessionlength"];
	$uploadrate=$GLOBALS["uploadrate"];
	$downloadrate=$GLOBALS["downloadrate"];
	$uploadquota=$GLOBALS["uploadquota"];
	$downloadquota=$GLOBALS["downloadquota"];
	$gwname=$GLOBALS["gwname"];
	$logpath=$GLOBALS["logpath"];

	if (isset($_GET["fullname"])) {
		$fullname=$_GET["fullname"];
	} else {
		$fullname="na";
	}

	if (isset($_GET["email"])) {
		$email=$_GET["email"];
	} else {
		$email="na";
	}


	/*	You can also send a custom data string to BinAuth. Set the variable $custom to the desired value
		It can contain any information that could be used for post authentication processing
		eg. the values set per client for Time, Data and Data Rate quotas can be sent to BinAuth for a custom script to use
		This string will be b64 encoded before sending to binauth and will appear in the output of ndsctl json
	*/

	$custom="fullname=$fullname, email=$email";
	$custom=base64_encode($custom);


	$rhid=hash('sha256', trim($hid).trim($key));

	# Construct the client authentication string or "log"
	# Note: override values set earlier if required, for example by testing clientif 
	$log="$rhid $sessionlength $uploadrate $downloadrate $uploadquota $downloadquota $custom \n";

	$logfile="$logpath"."$gwname/$rhid";

	# Request authentication by openNDS
	if (!file_exists($logfile)) {
		file_put_contents("$logfile", "$log");
	}

	echo "Waiting for link to establish....<br>";
	flush();

	# Display "waiting" ticker, then log authentication if successful:
	$count=0;
	$maxcount=30;

	for ($i=1; $i<=$maxcount; $i++) {
		$count++;
		sleep(1);
		echo "<b style=\"color:red;\">*</b>";

		if ($count == 10) {echo "<br>"; $count=0;}

		flush();

		if (file_exists("$logfile")) {
			$authed="no";
		} else {
			//no list so must be authed
			$authed="yes";
			write_log();
		}

		if ($authed == "yes") {
			echo "<br><b>Authenticated</b><br>";
			landing_page();
			flush();
			break;
		}
	}

	// Serve warning to client if authentication failed/timed out:
	if ($i > $maxcount) {
		unlink("$logfile");
		echo "
			<br>The Portal has timed out<br>You may have to turn your WiFi off and on to reconnect.<br>
			<p>
			Click or tap Continue to try again.
			</p>
			<form>
				<input type=\"button\" VALUE=\"Continue\" onClick=\"location.href='".$redir."'\" >
			</form>
		";
	}
}

function thankyou_page() {
	/* Output the "Thankyou page" with a continue button
		You could include information or advertising on this page
		Be aware that many devices will close the login browser as soon as
		the client taps continue, so now is the time to deliver your message.
	*/

	$me=$_SERVER['SCRIPT_NAME'];
	$host=$_SERVER['HTTP_HOST'];
	$fas=$GLOBALS["fas"];
	$iv=$GLOBALS["iv"];
	$clientip=$GLOBALS["clientip"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayaddress=$GLOBALS["gatewayaddress"];
	$gatewaymac=$GLOBALS["gatewaymac"];
	$key=$GLOBALS["key"];
	$hid=$GLOBALS["hid"];
	$clientif=$GLOBALS["clientif"];
	$originurl=$GLOBALS["originurl"];
	$fullname=$_GET["fullname"];
	$email=$_GET["email"];
	$fullname_url=rawurlencode($fullname);
	$auth="yes";

	echo "
		<big-red>
			Thankyou!
		</big-red>
		<br>
		<b>Welcome $fullname</b>
		<br>
		<italic-black>
			Your News or Advertising could be here, contact the owners of this Hotspot to find out how!
		</italic-black>
		<form action=\"$me\" method=\"get\">
			<input type=\"hidden\" name=\"fas\" value=\"$fas\">
			<input type=\"hidden\" name=\"iv\" value=\"$iv\">
			<input type=\"hidden\" name=\"auth\" value=\"$auth\">
			<input type=\"hidden\" name=\"fullname\" value=\"$fullname_url\">
			<input type=\"hidden\" name=\"email\" value=\"$email\">
			<input type=\"submit\" value=\"Continue\" >
		</form>
		<hr>
	";

	read_terms();
	flush();
}

function login_page() {
	$fullname=$email="";
	$me=$_SERVER['SCRIPT_NAME'];
	$fas=$_GET["fas"];
	$iv=$GLOBALS["iv"];
	$clientip=$GLOBALS["clientip"];
	$clientmac=$GLOBALS["clientmac"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayaddress=$GLOBALS["gatewayaddress"];
	$gatewaymac=$GLOBALS["gatewaymac"];
	$clientif=$GLOBALS["clientif"];
	$client_zone=$GLOBALS["client_zone"];
	$originurl=$GLOBALS["originurl"];

	if (isset($_GET["fullname"])) {
		$fullname=ucwords($_GET["fullname"]);
	}

	if (isset($_GET["email"])) {
		$email=$_GET["email"];
	}

	if ($fullname == "" or $email == "") {
		echo "
			<big-red>Welcome!</big-red><br>
			<med-blue>You are connected to $client_zone</med-blue><br>
			<b>Please enter your Full Name and Email Address</b>
		";

		if (! isset($_GET['fas']))  {
			echo "<br><b style=\"color:red;\">ERROR! Incomplete data passed from NDS</b>\n";
		} else {
			echo "
				<form action=\"$me\" method=\"get\" >
					<input type=\"hidden\" name=\"fas\" value=\"$fas\">
					<input type=\"hidden\" name=\"iv\" value=\"$iv\">
					<hr>Full Name:<br>
					<input type=\"text\" name=\"fullname\" value=\"$fullname\">
					<br>
					Email Address:<br>
					<input type=\"email\" name=\"email\" value=\"$email\">
					<br><br>
					<input type=\"submit\" value=\"Accept Terms of Service\">
				</form>
				<hr>
			";

			read_terms();
			flush();
		}
	} else {
		thankyou_page();
	}
}

function status_page() {
	$me=$_SERVER['SCRIPT_NAME'];
	$clientip=$GLOBALS["clientip"];
	$clientmac=$GLOBALS["clientmac"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayaddress=$GLOBALS["gatewayaddress"];
	$gatewaymac=$GLOBALS["gatewaymac"];
	$clientif=$GLOBALS["clientif"];
	$originurl=$GLOBALS["originurl"];
	$redir=rawurldecode($originurl);

	// Is the client already logged in?
	if ($_GET["status"] == "authenticated") {
		echo "
			<p><big-red>You are already logged in and have access to the Internet.</big-red></p>
			<hr>
			<p><italic-black>You can use your Browser, Email and other network Apps as you normally would.</italic-black></p>
		";

		read_terms();

		echo "
			<p>
			Your device originally requested <b>$redir</b>
			<br>
			Click or tap Continue to go to there.
			</p>
			<form>
				<input type=\"button\" VALUE=\"Continue\" onClick=\"location.href='".$redir."'\" >
			</form>
		";
	} else {
		echo "
			<p><big-red>ERROR 404 - Page Not Found.</big-red></p>
			<hr>
			<p><italic-black>The requested resource could not be found.</italic-black></p>
		";
	}
	flush();
}

function landing_page() {
	$me=$_SERVER['SCRIPT_NAME'];
	$fas=$_GET["fas"];
	$iv=$GLOBALS["iv"];
	$originurl=$GLOBALS["originurl"];
	$gatewayaddress=$GLOBALS["gatewayaddress"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayurl=rawurldecode($GLOBALS["gatewayurl"]);
	$clientif=$GLOBALS["clientif"];
	$client_zone=$GLOBALS["client_zone"];
	$fullname=$_GET["fullname"];
	$email=$_GET["email"];
	$redir=rawurldecode($originurl);

	echo "
		<p>
			<big-red>
				You are now logged in and have been granted access to the Internet.
			</big-red>
		</p>
		<hr>
		<med-blue>You are connected to $client_zone</med-blue><br>
		<p>
			<italic-black>
				You can use your Browser, Email and other network Apps as you normally would.
			</italic-black>
		</p>
		<p>
		(Your device originally requested $redir)
		<hr>
		Click or tap Continue to show the status of your account.
		</p>
		<form>
			<input type=\"button\" VALUE=\"Continue\" onClick=\"location.href='".$gatewayurl."'\" >
		</form>
		<hr>
	";

	read_terms();
	flush();
}

function splash_header() {
	$imagepath=$GLOBALS["imagepath"];
	$gatewayname=$GLOBALS["gatewayname"];
	$gatewayname=htmlentities(rawurldecode($gatewayname), ENT_HTML5, "UTF-8", FALSE);

	// Add headers to stop browsers from cacheing 
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
	header("Cache-Control: no-cache");
	header("Pragma: no-cache");

	// Output the common header html
	echo "<!DOCTYPE html>\n<html>\n<head>
		<meta charset=\"utf-8\" />
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
		<link rel=\"shortcut icon\" href=$imagepath type=\"image/x-icon\">
		<title>$gatewayname</title>
		<style>
	";
	flush();
	insert_css();
	flush();
	echo "
		</style>
		</head>
		<body>
		<div class=\"offset\">
		<med-blue>
			$gatewayname
		</med-blue><br>
		<div class=\"insert\">
	";
	flush();
}

function err403() {
	$imagepath=$GLOBALS["imagepath"];
	// Add headers to stop browsers from cacheing
	header('HTTP/1.1 403 Forbidden');
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
	header("Cache-Control: no-cache");
	header("Pragma: no-cache");


	echo "<!DOCTYPE html>\n<html>\n<head>
		<meta charset=\"utf-8\" />
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
		<link rel=\"shortcut icon\" href=$imagepath type=\"image/x-icon\">
		<title>Forbidden</title>
		<style>
	";
	flush();
	insert_css();
	flush();
	echo "
		</style>
		</head>
		<body>
		<div class=\"offset\">
		<div class=\"insert\">
		<hr>
		<b style=\"color:red; font-size:1.5em;\">Encryption Error <br> Access Forbidden</b><br>
	";
	flush();
	footer();
}

function footer() {
	$imagepath=$GLOBALS["imagepath"];

	if (isset($GLOBALS["version"])) {
		$version=$GLOBALS["version"];
	} else {
		$version="";
	}

	$year=date("Y");
	echo "
		<hr>
		<div style=\"font-size:0.5em;\">
			<img style=\"height:60px; width:60px; float:left;\" src=\"$imagepath\" alt=\"Splash Page: For access to the Internet.\">
			&copy; The openNDS Project 2015 - $year<br>
			Portal Version: $version
			<br><br><br><br>
		</div>
		</div>
		</div>
		</body>
		</html>
	";
	exit(0);
}

function read_terms() {
	#terms of service button
	$me=$_SERVER['SCRIPT_NAME'];
	$fas=$GLOBALS["fas"];
	$iv=$GLOBALS["iv"];

	echo "
		<form action=\"$me\" method=\"get\">
			<input type=\"hidden\" name=\"fas\" value=\"$fas\">
			<input type=\"hidden\" name=\"iv\" value=\"$iv\">
			<input type=\"hidden\" name=\"terms\" value=\"yes\">
			<input type=\"submit\" value=\"Read Terms of Service\" >
		</form>
	";
}

function display_terms () {
	# This is the all important "Terms of service"
	# Edit this long winded generic version to suit your requirements.
	####
	# WARNING #
	# It is your responsibility to ensure these "Terms of Service" are compliant with the REGULATIONS and LAWS of your Country or State.
	# In most locations, a Privacy Statement is an essential part of the Terms of Service.
	####

	#Privacy
	echo "
		<b style=\"color:red;\">Privacy.</b><br>
		<b>
			By logging in to the system, you grant your permission for this system to store any data you provide for
			the purposes of logging in, along with the networking parameters of your device that the system requires to function.<br>
			All information is stored for your convenience and for the protection of both yourself and us.<br>
			All information collected by this system is stored in a secure manner and is not accessible by third parties.<br>
			In return, we grant you FREE Internet access.
		</b><hr>
	";
	flush();

	# Terms of Service
	echo "
		<b style=\"color:red;\">Terms of Service for this Hotspot.</b> <br>

		<b>Access is granted on a basis of trust that you will NOT misuse or abuse that access in any way.</b><hr>

		<b>Please scroll down to read the Terms of Service in full or click the Continue button to return to the Acceptance Page</b>

		<form>
			<input type=\"button\" VALUE=\"Continue\" onClick=\"history.go(-1);return true;\">
		</form>
	";
	flush();

	# Proper Use
	echo "
		<hr>
		<b>Proper Use</b>

		<p>
			This Hotspot provides a wireless network that allows you to connect to the Internet. <br>
			<b>Use of this Internet connection is provided in return for your FULL acceptance of these Terms Of Service.</b>
		</p>

		<p>
			<b>You agree</b> that you are responsible for providing security measures that are suited for your intended use of the Service.
			For example, you shall take full responsibility for taking adequate measures to safeguard your data from loss.
		</p>

		<p>
			While the Hotspot uses commercially reasonable efforts to provide a secure service,
			the effectiveness of those efforts cannot be guaranteed.
		</p>

		<p>
			<b>You may</b> use the technology provided to you by this Hotspot for the sole purpose
			of using the Service as described here.
			You must immediately notify the Owner of any unauthorized use of the Service or any other security breach.<br><br>
			We will give you an IP address each time you access the Hotspot, and it may change.
			<br>
			<b>You shall not</b> program any other IP or MAC address into your device that accesses the Hotspot.
			You may not use the Service for any other reason, including reselling any aspect of the Service.
			Other examples of improper activities include, without limitation:
		</p>

			<ol>
				<li>
					downloading or uploading such large volumes of data that the performance of the Service becomes
					noticeably degraded for other users for a significant period;
				</li>

				<li>
					attempting to break security, access, tamper with or use any unauthorized areas of the Service;
				</li>

				<li>
					removing any copyright, trademark or other proprietary rights notices contained in or on the Service;
				</li>

				<li>
					attempting to collect or maintain any information about other users of the Service
					(including usernames and/or email addresses) or other third parties for unauthorized purposes;
				</li>

				<li>
					logging onto the Service under false or fraudulent pretenses;
				</li>

				<li>
					creating or transmitting unwanted electronic communications such as SPAM or chain letters to other users
					or otherwise interfering with other user's enjoyment of the service;
				</li>

				<li>
					transmitting any viruses, worms, defects, Trojan Horses or other items of a destructive nature; or
				</li>

				<li>
					using the Service for any unlawful, harassing, abusive, criminal or fraudulent purpose.
				</li>
			</ol>
	";
	flush();

	# Content Disclaimer
	echo "
		<hr>
		<b>Content Disclaimer</b>

		<p>
			The Hotspot Owners do not control and are not responsible for data, content, services, or products
			that are accessed or downloaded through the Service.
			The Owners may, but are not obliged to, block data transmissions to protect the Owner and the Public.
		</p>

		The Owners, their suppliers and their licensors expressly disclaim to the fullest extent permitted by law,
		all express, implied, and statutary warranties, including, without limitation, the warranties of merchantability
		or fitness for a particular purpose.
		<br><br>
		The Owners, their suppliers and their licensors expressly disclaim to the fullest extent permitted by law
		any liability for infringement of proprietory rights and/or infringement of Copyright by any user of the system.
		Login details and device identities may be stored and be used as evidence in a Court of Law against such users.
		<br>
	";
	flush();

	# Limitation of Liability
	echo "

		<hr><b>Limitation of Liability</b>

		<p>
			Under no circumstances shall the Owners, their suppliers or their licensors be liable to any user or
			any third party on account of that party's use or misuse of or reliance on the Service.
		</p>

		<hr><b>Changes to Terms of Service and Termination</b>

		<p>
			We may modify or terminate the Service and these Terms of Service and any accompanying policies,
			for any reason, and without notice, including the right to terminate with or without notice,
			without liability to you, any user or any third party. Please review these Terms of Service
			from time to time so that you will be apprised of any changes.
		</p>

		<p>
			We reserve the right to terminate your use of the Service, for any reason, and without notice.
			Upon any such termination, any and all rights granted to you by this Hotspot Owner shall terminate.
		</p>
	";
	flush();

	# Inemnity
	echo "
		<hr><b>Indemnity</b>

		<p>
			<b>You agree</b> to hold harmless and indemnify the Owners of this Hotspot,
			their suppliers and licensors from and against any third party claim arising from
			or in any way related to your use of the Service, including any liability or expense arising from all claims,
			losses, damages (actual and consequential), suits, judgments, litigation costs and legal fees, of every kind and nature.
		</p>

		<hr>
		<form>
			<input type=\"button\" VALUE=\"Continue\" onClick=\"history.go(-1);return true;\">
		</form>
	";
	flush();
}

function insert_css() {
	echo "
	body {
		background-color: lightgrey;
		color: #140f07;
		margin: 0;
		padding: 10px;
		font-family: sans-serif;
	}

	hr {
		display:block;
		margin-top:0.5em;
		margin-bottom:0.5em;
		margin-left:auto;
		margin-right:auto;
		border-style:inset;
		border-width:5px;
	}

	.offset {
		background: rgba(300, 300, 300, 0.6);
		border-radius: 10px;
		margin-left:auto;
		margin-right:auto;
		max-width:600px;
		min-width:200px;
		padding: 5px;
	}

	.insert {
		background: rgba(350, 350, 350, 0.7);
		border: 2px solid #aaa;
		border-radius: 10px;
		min-width:200px;
		max-width:100%;
		padding: 5px;
	}

	.insert > h1 {
		font-size: medium;
		margin: 0 0 15px;
	}

	img {
		width: 40%;
		max-width: 180px;
		margin-left: 0%;
		margin-right: 10px;
		border-radius: 3px;
	}

	input[type=text], input[type=email], input[type=password], input[type=number], input[type=tel] {
		font-size: 1em;
		line-height: 2em;
		height: 2em;
		color: #0c232a;
		background: lightgrey;
	}

	input[type=submit], input[type=button] {
			font-size: 1em;
		line-height: 2em;
		height: 2em;
		font-weight: bold;
		border: 0;
		border-radius: 10px;
		background-color: #1a7856;
		padding: 0 10px;
		color: #fff;
		cursor: pointer;
		box-shadow: rgba(50, 50, 93, 0.1) 0 0 0 1px inset,
		rgba(50, 50, 93, 0.1) 0 2px 5px 0, rgba(0, 0, 0, 0.07) 0 1px 1px 0;
	}

	med-blue {
		font-size: 1.2em;
		color: #0073ff;
		font-weight: bold;
		font-style: normal;
	}

	big-red {
		font-size: 1.5em;
		color: #c20801;
		font-weight: bold;
	}

	italic-black {
		font-size: 1em;
		color: #0c232a;
		font-weight: bold;
		font-style: italic;
		margin-bottom: 10px;
	}

	copy-right {
		font-size: 0.7em;
		color: darkgrey;
		font-weight: bold;
		font-style: italic;
	}

	";
	flush();
}

?>
