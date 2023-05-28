#!/bin/sh
#Copyright (C) The openNDS Contributors 2004-2022
#Copyright (C) BlueWave Projects and Services 2015-2023
#This software is released under the GNU GPL license.
#
# Warning - shebang sh is for compatibliity with busybox ash (eg on OpenWrt)
# This is changed to bash automatically by Makefile for generic Linux
#

# Title of this theme:
title="theme_click-to-continue-basic"

# functions:

generate_splash_sequence() {
	click_to_continue
}

header() {
# Define a common header html for every page served
	echo "<!DOCTYPE html>
		<html>
		<head>
		<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\">
		<meta http-equiv=\"Pragma\" content=\"no-cache\">
		<meta http-equiv=\"Expires\" content=\"0\">
		<meta charset=\"utf-8\">
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
		<link rel=\"shortcut icon\" href=\"/images/splash.jpg\" type=\"image/x-icon\">
		<link rel=\"stylesheet\" type=\"text/css\" href=\"/splash.css\">
		<title>$gatewayname</title>
		</head>
		<body>
		<div class=\"offset\">
		<med-blue>
			$gatewayname <br>
		</med-blue>
		<div class=\"insert\" style=\"max-width:100%;\">
	"
}

footer() {
	# Define a common footer html for every page served
	year=$(date +'%Y')
	echo "
		<hr>
		<div style=\"font-size:0.5em;\">
			<br>
			<img style=\"height:60px; width:60px; float:left;\" src=\"$imagepath\" alt=\"Splash Page: For access to the Internet.\">
			&copy; Portal: BlueWave Projects and Services 2015 - $year<br>
			<br>
			Portal Version: $version
			<br><br><br><br>
		</div>
		</div>
		</div>
		</body>
		</html>
	"

	exit 0
}



click_to_continue() {
	# This is the simple click to continue splash page with no client validation.
	# The client is however required to accept the terms of service.

	if [ "$continue" = "clicked" ]; then
		thankyou_page
		footer
	fi

	continue_form
	footer
}

continue_form() {
	# Define a click to Continue form

	echo "
		<big-red>Welcome!</big-red><br>
		<med-blue>You are connected to $client_zone</med-blue><br>
		<italic-black>
			To access the Internet you must Accept the Terms of Service.
		</italic-black>
		<hr>
		<form action=\"/opennds_preauth/\" method=\"get\">
			<input type=\"hidden\" name=\"fas\" value=\"$fas\">
			<input type=\"hidden\" name=\"continue\" value=\"clicked\">
			<input type=\"submit\" value=\"Accept Terms of Service\" >
		</form>
		<br>
	"

	read_terms
	footer
}

thankyou_page () {
	# If we got here, we have both the username and emailaddress fields as completed on the login page on the client,
	# or Continue has been clicked on the "Click to Continue" page
	# No further validation is required so we can grant access to the client. The token is not actually required.

	# We now output the "Thankyou page" with a "Continue" button.

	# This is the place to include information or advertising on this page,
	# as this page will stay open until the client user taps or clicks "Continue"

	# Be aware that many devices will close the login browser as soon as
	# the client user continues, so now is the time to deliver your message.

	echo "
		<big-red>
			Thankyou for using this service
		</big-red>
		<br>
		<b>Welcome</b>
		<br>
		<med-blue>You are connected to $client_zone</med-blue><br>
	"

	# Add your message here:
	# You could retrieve text or images from a remote server using wget or curl
	# as this router has Internet access whilst the client device does not (yet).
	echo "
		<br>
		<italic-black>
			Your News or Advertising could be here, contact the owners of this Hotspot to find out how!
			<br>
		</italic-black>
	"

	if [ -z "$custom" ]; then
		customhtml=""
	else
		customhtml="<input type=\"hidden\" name=\"custom\" value=\"$custom\">"
	fi

	# Continue to the landing page, the client is authenticated there
	echo "
		<form action=\"/opennds_preauth/\" method=\"get\">
			<input type=\"hidden\" name=\"fas\" value=\"$fas\">
			$customhtml
			<input type=\"hidden\" name=\"landing\" value=\"yes\">
			<input type=\"submit\" value=\"Continue\" >
		</form>
		<br>
	"

	# Serve the rest of the page:
	read_terms
	footer
}

landing_page() {
	originurl=$(printf "${originurl//%/\\x}")
	gatewayurl=$(printf "${gatewayurl//%/\\x}")

	# authenticate and write to the log - returns with $ndsstatus set
	auth_log

	# output the landing page - note many CPD implementations will close as soon as Internet access is detected
	# The client may not see this page, or only see it briefly
	auth_success="
		<p>
			<big-red>
				You are now logged in and have been granted access to the Internet.
			</big-red>
			<hr>

			<italic-black>
				You can use your Browser, Email and other network Apps as you normally would.
			</italic-black>

			(Your device originally requested $originurl)
			<hr>
			Click or tap Continue to show the status of your account.
		</p>
		<form>
			<input type=\"button\" VALUE=\"Continue\" onClick=\"location.href='$gatewayurl'\" >
		</form>
		<hr>
	"
	auth_fail="
		<p>
			<big-red>
				Something went wrong and you have failed to log in.
			</big-red>
			<hr>
		</p>
		<hr>
		<p>
			<italic-black>
				Your login attempt probably timed out.
			</italic-black>
		</p>
		<p>
			<br>
			Click or tap Continue to try again.
		</p>
		<form>
			<input type=\"button\" VALUE=\"Continue\" onClick=\"location.href='$originurl'\" >
		</form>
		<hr>
	"

	if [ "$ndsstatus" = "authenticated" ]; then
		echo "$auth_success"
	else
		echo "$auth_fail"
	fi

	read_terms
	footer
}

read_terms() {
	#terms of service button
	echo "
		<form action=\"/opennds_preauth/\" method=\"get\">
			<input type=\"hidden\" name=\"fas\" value=\"$fas\">
			<input type=\"hidden\" name=\"terms\" value=\"yes\">
			<input type=\"submit\" value=\"Read Terms of Service   \" >
		</form>
	"
}

display_terms() {
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
	"

	# Terms of Service
	echo "
		<b style=\"color:red;\">Terms of Service for this Hotspot.</b> <br>

		<b>Access is granted on a basis of trust that you will NOT misuse or abuse that access in any way.</b><hr>

		<b>Please scroll down to read the Terms of Service in full or click the Continue button to return to the Acceptance Page</b>

		<form>
			<input type=\"button\" VALUE=\"Continue\" onClick=\"history.go(-1);return true;\">
		</form>
	"

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
	"

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
	"

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
	"

	# Indemnity
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
	"
	footer
}

#### end of functions ####


#################################################
#						#
#  Start - Main entry point for this Theme	#
#						#
#  Parameters set here overide those		#
#  set in libopennds.sh			#
#						#
#################################################

# Quotas and Data Rates
#########################################
# Set length of session in minutes (eg 24 hours is 1440 minutes - if set to 0 then defaults to global sessiontimeout value):
# eg for 100 mins:
# session_length="100"
#
# eg for 20 hours:
# session_length=$((20*60))
#
# eg for 20 hours and 30 minutes:
# session_length=$((20*60+30))
session_length="0"

# Set Rate and Quota values for the client
# The session length, rate and quota values could be determined by this script, on a per client basis.
# rates are in kb/s, quotas are in kB. - if set to 0 then defaults to global value).
upload_rate="0"
download_rate="0"
upload_quota="0"
download_quota="0"

quotas="$session_length $upload_rate $download_rate $upload_quota $download_quota"

# Define the list of Parameters we expect to be sent sent from openNDS ($ndsparamlist):
# Note you can add custom parameters to the config file and to read them you must also add them here.
# Custom parameters are "Portal" information and are the same for all clients eg "admin_email" and "location" 
ndscustomparams=""
ndscustomimages=""
ndscustomfiles=""

ndsparamlist="$ndsparamlist $ndscustomparams $ndscustomimages $ndscustomfiles"

# The list of FAS Variables used in the Login Dialogue generated by this script is $fasvarlist and defined in libopennds.sh
#
# Additional custom FAS variables defined in this theme should be added to $fasvarlist here.
additionalthemevars=""

fasvarlist="$fasvarlist $additionalthemevars"

# You can choose to define a custom string. This will be b64 encoded and sent to openNDS.
# There it will be made available to be displayed in the output of ndsctl json as well as being sent
#	to the BinAuth post authentication processing script if enabled.
# Set the variable $binauth_custom to the desired value.
# Values set here can be overridden by the themespec file

#binauth_custom="This is sample text sent from \"$title\" to \"BinAuth\" for post authentication processing."

# Encode and activate the custom string
#encode_custom

# Set the user info string for logs (this can contain any useful information)
userinfo="$title"

# Customise the Logfile location. Note: the default uses the tmpfs "temporary" directory to prevent flash wear.
# Override the defaults to a custom location eg a mounted USB stick.
#mountpoint="/mylogdrivemountpoint"
#logdir="$mountpoint/ndslog/"
#logname="ndslog.log"



