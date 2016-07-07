#!/usr/bin/perl

# CoovaChilli - http://coova.github.io/. A Wireless LAN Access Point Controller
# Copyright (C) 2003, 2004 Mondru AB.
# Copyright (C) 2006-2012 David Bird <david@coova.com>
#
# The contents of this file may be used under the terms of the GNU
# General Public License Version 2, provided that the above copyright
# notice and this permission notice is included in all copies or
# substantial portions of the software.

# This code is horrible -- it came that way, and remains that way. A
# real captive portal is demonstrated here: http://drupal.org/project/hotspot

# Redirects from CoovaChilli daemon:
#
# Redirection when not yet or already authenticated
#   notyet:  CoovaChilli daemon redirects to login page.
#   already: CoovaChilli daemon redirects to success status page.
#
# Response to login:
#   already: Attempt to login when already logged in.
#   failed:  Login failed
#   success: Login succeded
#
# logoff:  Response to a logout

# Shared secret used to encrypt challenge with. Prevents dictionary attacks.
# You should change this to your own shared secret.
$uamsecret = "uamsecret";

# Uncomment the following line if you want to use ordinary user-password (PAP)
# for radius authentication. 
# $userpassword=1;

$loginpath = "/cgi-bin/hotspotlogin.cgi";
$debug = 1;

# To use MSCHAPv2 Authentication with, 
# then uncomment the next two lines. 
#$ntresponse = 1;
#$chilli_response = '/usr/local/sbin/chilli_response';

# start program

use Digest::MD5  qw(md5 md5_hex md5_base64);

%data = { };

$bgcolor = "#c0d8f4";

sub getdata() 
{
    local ($input) = @_;
    foreach $var (split('&',$input))
    {
	local ($name, $value) = split('=',$var);
	$data{lc($name)} = &urldecode($value);
    }
}

&getdata(<STDIN>);
&getdata($ENV{QUERY_STRING});

if (!$debug && !($ENV{HTTPS} =~ /^on$/)) 
{
    &error("CoovaChilli Login Failed", 
	   "Login must use encrypted connection.");
}

if ($data{'button'} =~ /^Login$/ || 
    ($data{'res'} eq "wispr" && $data{'username'} ne "")) 
{
    $hexchal  = pack "H*", $data{'challenge'};

    if (defined $uamsecret)
    {
	$newchal = md5($hexchal, $uamsecret);
    }
    else
    {
	$newchal = $hexchal;
    }

    $logonUrl = "http://$data{'uamip'}:$data{'uamport'}/logon?username=".&urlencode($data{'username'})."&";

    if ($data{'wisprversion'} ne "" && $data{'wispreapmsg'} ne "") 
    {
	$logonUrl .= "WISPrEAPMsg=".&urlencode($data{'wispreapmsg'});
	$logonUrl .= "&WISPrVersion=".&urlencode($data{'wisprversion'});
    }
    elsif ($ntresponse == 1) 
    {
	# Encode plain text into NT-Password 
	$response = `$chilli_response -nt "$data{'challenge'}" "$uamsecret" "$data{'username'}" "$data{'password'}"`;
	$logonUrl .= "ntresponse=".&urlencode($response);
    }
    elsif ($userpassword == 1) 
    {
	# Encode plain text password with challenge 
	# (which may or may not be uamsecret encoded)
	# If challange isn't long enough, repeat it until it is
	while (length($newchal) < length($data{'password'})){
	    $newchal .= $newchal;
        }

	$pappassword = unpack "H*", substr($data{'password'} ^ $newchal, 0, length($data{'password'}));
	$logonUrl .= "password=".&urlencode($pappassword);
    } 
    else 
    {
	# Generate a CHAP response with the password and the
	# challenge (which may have been uamsecret encoded)
	$response = md5_hex("\0", $data{'password'}, $newchal);
	$logonUrl .= "response=".&urlencode($response);
    }

    $loginUrl .= "&userurl=".&urlencode($data{'userurl'});

    &redirect($logonUrl);
}

$result = 0;

if ($data{'res'} eq "success") 
{ 
    $result = 1;
}
elsif ($data{'res'} eq "failed") 
{ 
    $result = 2;
}
elsif ($data{'res'} eq "logoff") 
{ 
    $result = 3;
}
elsif ($data{'res'} eq "already") 
{ 
    $result = 4;
}
elsif ($data{'res'} eq "notyet") 
{ 
    $result = 5;
}
elsif ($data{'res'} eq "wispr") 
{ 
    $result = 6;
}
elsif ($data{'res'} eq "popup1") 
{ 
    $result = 11;
}
elsif ($data{'res'} eq "popup2") 
{ 
    $result = 12;
}
elsif ($data{'res'} eq "popup3") 
{ 
    $result = 13;
}

if ($result == 0) 
{
    &error("CoovaChilli Login Failed",
	   "Login must be performed through CoovaChilli daemon.");
}

if ($result != 6) 
{
    print "Content-type: text/html\n\n
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
  <title>CoovaChilli Login</title>
  <meta http-equiv=\"Cache-control\" content=\"no-cache\">
  <meta http-equiv=\"Pragma\" content=\"no-cache\">
  <SCRIPT LANGUAGE=\"JavaScript\">
    var blur = 0;
    var starttime = new Date();
    var startclock = starttime.getTime();
    var mytimeleft = 0;

    function doTime() {
      window.setTimeout( \"doTime()\", 1000 );
      t = new Date();
      time = Math.round((t.getTime() - starttime.getTime())/1000);
      if (mytimeleft) {
        time = mytimeleft - time;
        if (time <= 0) {
          window.location = \"$loginpath?res=popup3&uamip=$data{'uamip'}&uamport=$data{'uamport'}\";
        }
      }
      if (time < 0) time = 0;
      hours = (time - (time % 3600)) / 3600;
      time = time - (hours * 3600);
      mins = (time - (time % 60)) / 60;
      secs = time - (mins * 60);
      if (hours < 10) hours = \"0\" + hours;
      if (mins < 10) mins = \"0\" + mins;
      if (secs < 10) secs = \"0\" + secs;
      title = \"Online time: \" + hours + \":\" + mins + \":\" + secs;
      if (mytimeleft) {
        title = \"Remaining time: \" + hours + \":\" + mins + \":\" + secs;
      }
      if(document.all || document.getElementById){
         document.title = title;
      }
      else {   
        self.status = title;
      }
    }

    function popUp(URL) {
      if (self.name != \"coovachilli_popup\") {
        coovachilli_popup = window.open(URL, 'coovachilli_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=500,height=375');
      }
    }

    function doOnLoad(result, URL, userurl, redirurl, timeleft) {
      if (timeleft) {
        mytimeleft = timeleft;
      }
      if ((result == 1) && (self.name == \"coovachilli_popup\")) {
        doTime();
      }
      if ((result == 1) && (self.name != \"coovachilli_popup\")) {
        coovachilli_popup = window.open(URL, 'coovachilli_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=500,height=375');
      }
      if ((result == 2) || result == 5) {
        document.form1.UserName.focus()
      }
      if ((result == 2) && (self.name != \"coovachilli_popup\")) {
        coovachilli_popup = window.open('', 'coovachilli_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=400,height=200');
        coovachilli_popup.close();
      }
      if ((result == 12) && (self.name == \"coovachilli_popup\")) {
        doTime();
        if (redirurl) {
          opener.location = redirurl;
        }
        else if (opener.home) {
          opener.home();
        }
        else {
          opener.location = \"about:home\";
        }
        self.focus();
        blur = 0;
      }
      if ((result == 13) && (self.name == \"coovachilli_popup\")) {
        self.focus();
        blur = 1;
      }
    }

    function doOnBlur(result) {
      if ((result == 12) && (self.name == \"coovachilli_popup\")) {
        if (blur == 0) {
          blur = 1;
          self.focus();
        }
      }
    }
  </script>
</head>
<body onLoad=\"javascript:doOnLoad($result, '$loginpath?res=popup2&uamip=$data{'uamip'}&uamport=$data{'uamport'}&userurl=".&urlencode($data{'userurl'})."&redirurl=".&urlencode($data{'redirurl'})."&timeleft=$timeleft','$data{'userurl'}', '$data{'redirurl'}', '$timeleft')\" onBlur = \"javascript:doOnBlur($result)\" bgColor = '$bgcolor'>";
}

if ($result == 2) 
{
    print "<h1 style=\"text-align: center;\">CoovaChilli Login Failed</h1>";
    if ($data{'reply'}) 
    {
	print "<center> $data{'reply'} </BR></BR></center>";
    }
}

if ($result == 5) 
{
    print "<h1 style=\"text-align: center;\">CoovaChilli Login</h1>";
}

if ($result == 2 || $result == 5) 
{
  print "
  <form name=\"form1\" method=\"post\" action=\"$loginpath\">
  <INPUT TYPE=\"hidden\" NAME=\"challenge\" VALUE=\"$data{'challenge'}\">
  <INPUT TYPE=\"hidden\" NAME=\"uamip\" VALUE=\"$data{'uamip'}\">
  <INPUT TYPE=\"hidden\" NAME=\"uamport\" VALUE=\"$data{'uamport'}\">
  <INPUT TYPE=\"hidden\" NAME=\"userurl\" VALUE=\"$data{'userurl'}\">
  <center>
  <table border=\"0\" cellpadding=\"5\" cellspacing=\"0\" style=\"width: 217px;\">
    <tbody>
      <tr>
        <td align=\"right\">Username:</td>
        <td><input STYLE=\"font-family: Arial\" type=\"text\" name=\"UserName\" size=\"20\" maxlength=\"128\"></td>
      </tr>
      <tr>
        <td align=\"right\">Password:</td>
        <td><input STYLE=\"font-family: Arial\" type=\"password\" name=\"Password\" size=\"20\" maxlength=\"128\"></td>
      </tr>
      <tr>
        <td align=\"center\" colspan=\"2\" height=\"23\"><input type=\"submit\" name=\"button\" value=\"Login\" onClick=\"javascript:popUp('$loginpath?res=popup1&uamip=$data{'uamip'}&uamport=$data{'uamport'}')\"></td> 
      </tr>
    </tbody>
  </table>
  </center>
  </form>
</body>
</html>";
}

if ($result == 1) 
{
  print "
  <h1 style=\"text-align: center;\">Logged in to CoovaChilli</h1>";

  if ($data{'reply'}) 
  { 
      print "<center> $data{'reply'} </BR></BR></center>";
  }

  print "
  <center>
    <a href=\"http://$data{'uamip'}:$data{'uamport'}/logoff\">Logout</a>
  </center>
</body>
</html>";
}

if (($result == 4) || ($result == 12)) 
{
  print "
  <h1 style=\"text-align: center;\">Logged in to CoovaChilli</h1>
  <center>
    <a href=\"http://$data{'uamip'}:$data{'uamport'}/logoff\">Logout</a>
  </center>
</body>
</html>";
}


if ($result == 11) 
{
  print "<h1 style=\"text-align: center;\">Logging in to CoovaChilli</h1>";
  print "
  <center>
    Please wait......
  </center>
</body>
</html>";
}


if (($result == 3) || ($result == 13)) 
{
    print "
  <h1 style=\"text-align: center;\">Logged out from CoovaChilli</h1>
  <center>
    <a href=\"http://$data{'uamip'}:$data{'uamport'}/prelogin\">Login</a>
  </center>
</body>
</html>";
}

sub header() {
    local($title, $head) = @_;
    print "Content-type: text/html\n\n
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<head>
<title>$title</title>
<meta http-equiv=\"Cache-control\" content=\"no-cache\">
<meta http-equiv=\"Pragma\" content=\"no-cache\">
$head
</head>";
}

sub body() {
    local($headline, $mesg, $wispr) = @_;
    print "<body bgColor=\"$bgcolor\">
  <h1 style=\"text-align: center;\">$headline</h1>
  <center>
    $mesg
  </center>
</body>
<!--
$wispr
-->
</html>
";
}

sub page() {
    local($title, $head, $headline, $mesg, $wispr) = @_;
    &header($title, $head);
}

sub redirect() {
    local($url) = @_;
    &header("CoovaChilli Login", 
	    "<meta http-equiv=\"refresh\" content=\"0;url=$url\">");
    &body("Logging in to CoovaChilli",
	"Please wait...",
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<WISPAccessGatewayParam 
  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
  xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\">
<AuthenticationReply>
<MessageType>120</MessageType>
<ResponseCode>201</ResponseCode>
<LoginResultsURL>$url</LoginResultsURL>
</AuthenticationReply> 
</WISPAccessGatewayParam>");
    exit(0);
}

sub error() {
    local($title, $mesg) = @_;
    &header($title);
    &body($title, $mesg, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<WISPAccessGatewayParam 
  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
  xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\">
<AuthenticationReply>
<MessageType>120</MessageType>
<ResponseCode>102</ResponseCode>
<ReplyMessage>$mesg</ReplyMessage>
</AuthenticationReply> 
</WISPAccessGatewayParam>");
    exit(0);
}

sub urlencode {
    my $s = shift;
    $s =~ s/ /+/g;
    $s =~ s/([^A-Za-z0-9\+-])/sprintf("%%%02X", ord($1))/seg;
    return $s;
}

sub urldecode {
    my $s = shift;
    $s =~ s/\%([A-Fa-f0-9]{2})/pack('C', hex($1))/seg;
    $s =~ s/\+/ /g;
    return $s;
}
