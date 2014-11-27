#!/usr/bin/perl -w

# Set
#  action on detect = "/path/to/send_alert.pl"
# in /etc/arpalert/arpalert.conf to enable this script.

#
# This script is using Mail::Sendmail
# Web site: http://alma.ch/perl/mail.html#Mail::Sendmail
#
# Arguments sent by ArpAlert are :
# 1 : MAC Address
# 2 : IP Address
# 3 : supp (used with unathrq alert)
# 4 : Type of alert (cf arpalert.conf)
#

use Mail::Sendmail;
use Socket; # for inet_aton()

# Intruder MAC address
$intruder_MAC = $ARGV[0];

# Intruder IP address
$intruder_IP = $ARGV[1];

# Alert Type
$intruder_AlertType = $ARGV[3] or die "4 arguments needed";

open(MAILNAME, "</etc/mailname") or die "can't open /etc/mailname";
$mailname = <MAILNAME>;
chomp $mailname;

$mail{From} = 'ARP Alert <arpalert@' . "$mailname>";

# Separate multi receiver by coma (,)
# $mail{To}   = 'Mail 1 <mail.one@domain.com>, Mail 2 <mail.two@domain.com>';
$mail{To}   = "root <root@" . "$mailname>";

# SMTP server / IP or DNS name
# $server = 'smtp.domain.com';
$server = 'localhost';

if ($server) {
	$mail{Smtp} = $server;
	print "Server set to: $server\n";
}

$iaddr = inet_aton($intruder_IP) || "";
$intruder_Name = gethostbyaddr($iaddr, AF_INET) || "";

# Subject
$mail{Subject} = "[Warning] Intrusion Detection [Warning]";

# Body
$mail{Message} = "/!\\ Intruder Detected /!\\\n\n";
$mail{Message} .= "Intrusion time stamp : " . Mail::Sendmail::time_to_date() . "\n\n";
$mail{Message} .= "Intruder FQDN : $intruder_Name\n";
$mail{Message} .= "Intruder IP Address : $intruder_IP\n";
$mail{Message} .= "Intruder MAC Address : $intruder_MAC\n";
$mail{Message} .= "Type of alert : $intruder_AlertType\n";

# Send Alert
if (sendmail %mail) {
	print "content of \$Mail::Sendmail::log:\n$Mail::Sendmail::log\n";
	if ($Mail::Sendmail::error) {
		print "content of \$Mail::Sendmail::error:\n$Mail::Sendmail::error\n";
	}
	print "ok 2\n";
}
else {
	print "\n!Error sending mail:\n$Mail::Sendmail::error\n";
	print "not ok 2\n";
}
