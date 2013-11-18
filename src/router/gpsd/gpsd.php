<?php

#$CSK: gpsd.php,v 1.39 2006/11/21 22:31:10 ckuethe Exp $

# Copyright (c) 2006 Chris Kuethe <chris.kuethe@gmail.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

global $head, $blurb, $title, $googlemap, $autorefresh, $footer, $gmap_key;
global $GPS, $server, $advertise, $port, $magic, $swap_ew;
$magic = 1; # leave this set to 1

if (!file_exists("gpsd_config.inc"))
	write_config();

require_once("gpsd_config.inc");

# sample data
$resp = 'GPSD,S=2,P=53.527167 -113.530168,A=704.542,M=3,Q=10 1.77 0.80 0.66 0.61 1.87,Y=MID9 1158081774.000000 12:25 24 70 42 1:4 13 282 36 1:23 87 196 48 1:6 9 28 29 1:16 54 102 47 1:20 34 190 45 1:2 12 319 36 1:13 52 292 46 1:24 12 265 0 0:1 8 112 41 1:27 16 247 40 1:122 23 213 31 0:';

# if we're passing in a query, let's unpack and use it
if (isset($_GET['imgdata']) && isset($_GET['op']) && ($_GET['op'] == 'view')){
	$resp = base64_decode($_GET['imgdata']);
	if ($resp){
		gen_image($resp);
		exit(0);
	}
} else {
	if (isset($_GET['host']))
		if (!preg_match('/[^a-zA-Z0-9\.-]/', $_GET['host']))
			$server = $_GET['host'];

	if (isset($_GET['port']))
		if (!preg_match('/\D/', $_GET['port']) && ($port>0) && ($port<65536))
			$port = $_GET['port'];

	if ($magic){
		$sock = @fsockopen($server, $port, $errno, $errstr, 2);
		fwrite($sock, "J=1,W=1\n");	# watcher mode and buffering
		$resp = fread($sock, 384);
		$resp = fread($sock, 384);	# Wait for O
		$resp = fread($sock, 384);	# Wait for O
		fwrite($sock, "SPAMQY\n");	# Query what we actually want
		# the O report doesn't give us satellite usage or DOP
		$resp = fread($sock, 384);
		@fclose($sock);
	}
}

if (isset($_GET['op']) && ($_GET['op'] == 'view')){
	gen_image($resp);
} else {
	parse_pvt($resp);
	write_html($resp);
}

exit(0);

###########################################################################
function colorsetup($im){
	$C['white']	= imageColorAllocate($im, 255, 255, 255);
	$C['ltgray']	= imageColorAllocate($im, 191, 191, 191);
	$C['mdgray']	= imageColorAllocate($im, 127, 127, 127);
	$C['dkgray']	= imageColorAllocate($im, 63, 63, 63);
	$C['black']	= imageColorAllocate($im, 0, 0, 0);
	$C['red']	= imageColorAllocate($im, 255, 0, 0);
	$C['brightgreen'] = imageColorAllocate($im, 0, 255, 0);
	$C['darkgreen']	= imageColorAllocate($im, 0, 192, 0);
	$C['blue']	= imageColorAllocate($im, 0, 0, 255);
	$C['cyan']	= imageColorAllocate($im, 0, 255, 255);
	$C['magenta']	= imageColorAllocate($im, 255, 0, 255);
	$C['yellow']	= imageColorAllocate($im, 255, 255, 0);
	$C['orange']	= imageColorAllocate($im, 255, 128, 0);

	return $C;
}

function legend($im, $sz, $C){
	$r = 30;
	$fn = 5;
	$x = $sz - (4*$r+7) - 2;
	$y = $sz - $r - 3;

	imageFilledRectangle($im, $x, $y, $x + 4*$r + 7, $y + $r +1, $C['dkgray']);
	imageRectangle($im, $x+0*$r+1, $y+1, $x + 1*$r + 0, $y + $r, $C['red']);
	imageRectangle($im, $x+1*$r+2, $y+1, $x + 2*$r + 2, $y + $r, $C['yellow']);
	imageRectangle($im, $x+2*$r+4, $y+1, $x + 3*$r + 4, $y + $r, $C['darkgreen']);
	imageRectangle($im, $x+4*$r+6, $y+1, $x + 3*$r + 6, $y + $r, $C['brightgreen']);
	imageString($im, $fn, $x+3+0*$r, $y+$r/3, "<30", $C['red']);
	imageString($im, $fn, $x+5+1*$r, $y+$r/3, "30+", $C['yellow']);
	imageString($im, $fn, $x+7+2*$r, $y+$r/3, "35+", $C['darkgreen']);
	imageString($im, $fn, $x+9+3*$r, $y+$r/3, "40+", $C['brightgreen']);
}

function radial($angle, $sz){
	#turn into radians
	$angle = deg2rad($angle);

	# determine length of radius
	$r = $sz * 0.5 * 0.95;

	# and convert length/azimuth to cartesian
	$x0 = sprintf("%d", (($sz * 0.5) - ($r * cos($angle))));
	$y0 = sprintf("%d", (($sz * 0.5) - ($r * sin($angle))));
	$x1 = sprintf("%d", (($sz * 0.5) + ($r * cos($angle))));
	$y1 = sprintf("%d", (($sz * 0.5) + ($r * sin($angle))));

	return array($x0, $y0, $x1, $y1);
}

function azel2xy($az, $el, $sz){
	global $swap_ew;
	#rotate coords... 90deg W = 180deg trig
	$az += 270;

	#turn into radians
	$az = deg2rad($az);

	# determine length of radius
	$r = $sz * 0.5 * 0.95;
	$r -= ($r * ($el/90));

	# and convert length/azimuth to cartesian
	$x = sprintf("%d", (($sz * 0.5) + ($r * cos($az))));
	$y = sprintf("%d", (($sz * 0.5) + ($r * sin($az))));
	if ($swap_ew == 0)
		$x = $sz - $x;

	return array($x, $y);
}

function splot($im, $sz, $C, $e){
	list($sv, $az, $el, $snr, $u) = $e;

	if ((0 == $sv) || (0 == $az + $el + $snr))
		return;

	$color = $C['brightgreen'];
	if ($snr < 40)
		$color = $C['darkgreen'];
	if ($snr < 35)
		$color = $C['yellow'];
	if ($snr < 30)
		$color = $C['red'];
	if ($el<10)
		$color = $C['blue'];

	list($x, $y) = azel2xy($el, $az, $sz);

	$r = 12;
	if (isset($_GET['sz']) && ($_GET['sz'] == 'small'))
		$r = 8;

	imageString($im, 3, $x+4, $y+4, $sv, $C['black']);
	if ($u)
		imageFilledArc($im, $x, $y, $r, $r, 0, 360, $color, 0);
	else
		if ($sv > 32) {
			imageDiamond($im, $x, $y, $r, $color);
		} else {
			imageArc($im, $x, $y, $r, $r, 0, 360, $color);
		}
}

function imageDiamond($im, $x, $y, $r, $color){
	$t = $r/2;
	# this lunacy is because imagesetthickness doesn't seem to work
	$vx = array ( $x+$t, $y, $x, $y+$t, $x-$t, $y, $x, $y-$t );
	imagepolygon($im, $vx, 4, $color);
	$t--;
	$vx = array ( $x+$t, $y, $x, $y+$t, $x-$t, $y, $x, $y-$t );
	imagepolygon($im, $vx, 4, $color);
	$t--;
	$vx = array ( $x+$t, $y, $x, $y+$t, $x-$t, $y, $x, $y-$t );
	imagepolygon($im, $vx, 4, $color);
}

function elevation($im, $sz, $C, $a){
	$b = 90 - $a;
	$a = $sz * 0.95 * ($a/180);
	imageArc($im, $sz/2, $sz/2, $a*2, $a*2, 0, 360, $C['ltgray']);
	$x = $sz/2 - 16;
	$y = $sz/2 - $a;
	imageString($im, 2, $x, $y, $b, $C['ltgray']);
}

function skyview($im, $sz, $C){
	global $swap_ew;
	$a = 90; $a = $sz * 0.95 * ($a/180);
	imageFilledArc($im, $sz/2, $sz/2, $a*2, $a*2, 0, 360, $C['mdgray'], 0);
	imageArc($im, $sz/2, $sz/2, $a*2, $a*2, 0, 360, $C['black']);
	$x = $sz/2 - 16; $y = $sz/2 - $a;
	imageString($im, 2, $x, $y, "0", $C['ltgray']);

	$a = 85; $a = $sz * 0.95 * ($a/180);
	imageFilledArc($im, $sz/2, $sz/2, $a*2, $a*2, 0, 360, $C['white'], 0);
	imageArc($im, $sz/2, $sz/2, $a*2, $a*2, 0, 360, $C['ltgray']);
	imageString($im, 1, $sz/2 - 6, $sz+$a, '5', $C['black']);
	$x = $sz/2 - 16; $y = $sz/2 - $a;
	imageString($im, 2, $x, $y, "5", $C['ltgray']);

	for($i = 0; $i < 180; $i += 15){
		list($x0, $y0, $x1, $y1) = radial($i, $sz);
		imageLine($im, $x0, $y0, $x1, $y1, $C['ltgray']);
	}

	for($i = 15; $i < 90; $i += 15)
		elevation($im, $sz, $C, $i);

	$x = $sz/2 - 16; $y = $sz/2 - 8;
	/* imageString($im, 2, $x, $y, "90", $C['ltgray']); */

	imageString($im, 4, $sz/2 + 4, 2        , 'N', $C['black']);
	imageString($im, 4, $sz/2 + 4, $sz - 16 , 'S', $C['black']);
	if ($swap_ew == 0){
		imageString($im, 4, 4        , $sz/2 + 4, 'E', $C['black']);
		imageString($im, 4, $sz - 10 , $sz/2 + 4, 'W', $C['black']);
	} else {
		imageString($im, 4, 4        , $sz/2 + 4, 'W', $C['black']);
		imageString($im, 4, $sz - 10 , $sz/2 + 4, 'E', $C['black']);
	}
}

function gen_image($resp){
	global $magic;

	$sz = 640;
	if (isset($_GET['sz']) && ($_GET['sz'] == 'small'))
		$sz = 240;

	if (!preg_match('/,Y=\S+ [0-9\.]+ (\d+):/', $resp, $m))
		die("can't parse gpsd's response");
	$n = $m[1];	

	$im = imageCreate($sz, $sz);
	$C = colorsetup($im);
	skyview($im, $sz, $C);
	if ($sz > 240)
		legend($im, $sz, $C);

	$s = explode(':', $resp);
	for($i = 1; $i <= $n; $i++){
		$e = explode(' ', $s[$i]);
		splot($im, $sz, $C, $e);
	}

	header("Content-type: image/png");
	imagePNG($im);
	imageDestroy($im);
}

function clearstate(){
	global $GPS;

	$GPS['loc'] = '';
	$GPS['alt'] = 'Unavailable';
	$GPS['lat'] = 'Unavailable';
	$GPS['lon'] = 'Unavailable';
	$GPS['sat'] = 'Unavailable';
	$GPS['hdop'] = 'Unavailable';
	$GPS['dgps'] = 'Unavailable';
	$GPS['fix'] = 'Unavailable';
	$GPS['gt'] = '';
	$GPS['lt'] = '';
}

function dfix($x, $y, $z){
	if ($x < 0){
		$x = sprintf("%f %s", -1 * $x, $z);
	} else {
		$x = sprintf("%f %s", $x, $y);
	}
	return $x;
}

function parse_pvt($resp){
	global $GPS, $magic;

	clearstate();

	if (strlen($resp)){
		$GPS['fix']  = 'No';
		if (preg_match('/M=(\d),/', $resp, $m)){
			switch ($m[1]){
			case 2:
				$GPS['fix']  = '2D';
				break;
			case 3:
				$GPS['fix']  = '3D';
				break;
			case 4:
				$GPS['fix']  = '3D (PPS)';
				break;
			default:
				$GPS['fix']  = "No";
			}
		}

		if (preg_match('/S=(\d),/', $resp, $m)){
			$GPS['fix'] .= ' (';
			if ($m[1] != 2){
				$GPS['fix'] .= 'not ';
			}
			$GPS['fix'] .= 'DGPS corrected)';
		}

		if (preg_match('/A=([0-9\.-]+),/', $resp, $m)){
			$GPS['alt'] = ($m[1] . ' m');
		}

		if (preg_match('/P=([0-9\.-]+) ([0-9\.-]+),/',
		    $resp, $m)){
			$GPS['lat'] = $m[1]; $GPS['lon'] = $m[2];
		}

		if (preg_match('/Q=(\d+) ([0-9\.]+) ([0-9\.]+) ([0-9\.]+) ([0-9\.]+)/', $resp, $m)){
			$GPS['sat']  = $m[1]; $GPS['gdop'] = $m[2];
			$GPS['hdop'] = $m[3]; $GPS['vdop'] = $m[4];
		}

		if ($GPS['lat'] != 'Unavailable' &&
		    $GPS['lon'] != 'Unavailable'){
			$GPS['lat'] = dfix($GPS['lat'], 'N', 'S');
			$GPS['lon'] = dfix($GPS['lon'], 'E', 'W');

			$GPS['loc'] = sprintf('at %s / %s',
			    $GPS['lat'], $GPS['lon']);
		}

		if (preg_match('/^No/', $GPS['fix'])){
			clearstate();
		}
	} else
		$GPS['loc'] = '';

	$GPS['gt'] = time();
	$GPS['lt'] = date("r", $GPS['gt']);
	$GPS['gt'] = gmdate("r", $GPS['gt']);
}

function write_html($resp){
	global $GPS, $sock, $errstr, $errno, $server, $port, $head, $body;
	global $blurb, $title, $autorefresh, $googlemap, $gmap_key, $footer;

	header("Content-type: text/html; charset=UTF-8");

	global $lat, $lon;
	$lat = (float)$GPS['lat'];
	$lon = -(float)$GPS['lon'];
	$x = $server; $y = $port;
	$imgdata = base64_encode($resp);
	include("gpsd_config.inc"); # breaks things
	$server = $x; $port = $y;

	if ($autorefresh > 0)
		$autorefresh = "<meta http-equiv='Refresh' content='$autorefresh'/>";
	else
		$autorefresh = '';

	$gmap_head = $gmap_body = $gmap_code = '';
	if ($googlemap){
		$gmap_head = gen_gmap_head();
		$gmap_body = 'onload="Load()" onunload="GUnload()"';
		$gmap_code = gen_gmap_code();
	}
	$svn ='$Rev: 4124 $';
	$part1 = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
{$head}
{$gmap_head}
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<meta http-equiv="Content-Language" content="en,en-us"/>
<title>{$title} - GPSD Test Station {$GPS['loc']}</title>
{$autorefresh}
<style>
.warning {
    color: #FF0000;
 }

.fixed {
    font-family: courier, fixed;
}

.caption {
    text-align: left; 
    margin: 1ex 3em 1ex 3em; /* top right bottom left */
}

.administrivia {
    font-size: small; 
    font-family: verdana, sans-serif;
}
</style>
</head>

<body {$body} {$gmap_body}>
<center>
<table border="0">
<tr><td align="justify">
{$blurb}
</td>
EOF;

	if (!strlen($advertise))
		$advertise = $server;

	if (!$sock)
		$part2 = "";
	else
		$part2 = <<<EOF
<!-- ------------------------------------------------------------ -->

<td rowspan="4" align="center" valign="top">
<img src="?op=view&amp;imgdata={$imgdata}"
width="640" height="640"/>
<br clear="all"/>
<p class="caption">A filled circle means the satellite was used in
the last fix. Green-yellow-red colors indicate signal strength in dB, 
 green=most and red=least.  Diamonds indicate SBAS satellites.</p>
</td>
</tr>
EOF;

	$part3 = <<<EOF
<!-- ------------------------------------------------------------ -->

<tr><td align="justify">To get real-time information, connect to
<span class="fixed">telnet://{$advertise}:{$port}/</span> and type "R".<br/>
Use a different server:<br/>
<form method=GET action="${_SERVER['SCRIPT_NAME']}">
<input name="host" value="{$server}">:
<input name="port" value="{$port}" size="5" maxlength="5">
<input type=submit value="Get Position"><input type=reset></form>
<br/>
</td>
</tr>
EOF;

	if (!$sock)
		$part4 = "<tr><td><font color='red'>The gpsd instance that this page monitors is not running.</font></td></tr>";
	else
		$part4 = <<<EOF
<!-- ------------------------------------------------------------ -->
        <tr><td align=center valign=top>
	<table border=1>
	<tr><td colspan=2 align=center><b>Current Information</b></td></tr>
	<tr><td>Time (Local)</td><td>{$GPS['lt']}</td></tr>
	<tr><td>Time (UTC)</td><td>{$GPS['gt']}</td></tr>
	<tr><td>Latitude</td><td>{$GPS['lat']}</td></tr>
	<tr><td>Longitude</td><td>{$GPS['lon']}</td></tr>
	<tr><td>Altitude</td><td>{$GPS['alt']}</td></tr>
	<tr><td>Fix Type</td><td>{$GPS['fix']}</td></tr>
	<tr><td>Satellites</td><td>{$GPS['sat']}</td></tr>
	<tr><td>HDOP</td><td>{$GPS['hdop']}</td></tr>
	</table>
</tr>
<tr><td><small>{$resp}</small></td></tr>
EOF;

	$part5 = <<<EOF
</table>
</center>

{$footer}

<hr/>
<span class="administrivia">This script is distributed by the <a href="http://gpsd.berlios.de">GPSD project</a>.</span><br/>
<span class="administrivia">{$svn}<br/>
</body>
</body>

EOF;

print $part1 . $part2 . $part3 . $part4 . $part5;

}

function write_config(){
	$f = fopen("gpsd_config.inc", "a");
	if (!$f)
		die("can't generate prototype config file. try running this script as root in DOCUMENT_ROOT");

	$buf = <<<EOB
<?PHP
\$title = 'My GPS Server';
\$server = '127.0.0.1';
#\$advertise = 'localhost';
\$port = 2947;
\$autorefresh = 0; # number of seconds after which to refresh
\$googlemap = 0; # set to 1 if you want to have a google map
\$gmap_key = 'GetYourOwnGoogleKey'; # your google API key goes here
\$swap_ew = 0; # set to 1 if you don't understand projections

## You can read the header, footer and blurb from a file...
# \$head = file_get_contents('/path/to/header.inc');
# \$body = file_get_contents('/path/to/body.inc');
# \$footer = file_get_contents('/path/to/footer.hinc');
# \$blurb = file_get_contents('/path/to/blurb.inc');

## ... or you can just define them here
\$head = '';
\$body = '';
\$footer = '';
\$blurb = <<<EOT
This is a
<a href="http://gpsd.berlios.de">gpsd</a>
server <blink><font color="red">located someplace</font></blink>.

The hardware is a
<blink><font color="red">hardware description and link</font></blink>.

This machine is maintained by
<a href="mailto:you@example.com">Your Name Goes Here</a>.<br/>
EOT;

?>

EOB;
	fwrite($f, $buf);
	fclose($f);
}

function gen_gmap_head() {
global $gmap_key;
return <<<EOT
<script src="http://maps.google.com/maps?file=api&amp;v=2&amp;key={$gmap_key}" type="text/javascript"></script>
<script type="text/javascript">
    <!--
    // Create a base icon for all of our markers that specifies the shadow, icon
    // dimensions, etc.
function Load() {
  if (GBrowserIsCompatible()) {
    var map = new GMap2(document.getElementById("map"));
    var point = new GLatLng( {$GLOBALS['lat']}, {$GLOBALS['lon']} );
    map.setCenter( point, 14);
    map.addControl(new GLargeMapControl());
    map.addControl(new GMapTypeControl());

    var baseIcon = new GIcon();
    baseIcon.shadow = "http://www.google.com/mapfiles/shadow50.png";
    baseIcon.iconSize = new GSize(20, 34);
    baseIcon.shadowSize = new GSize(37, 34);
    baseIcon.iconAnchor = new GPoint(9, 34);
    baseIcon.infoWindowAnchor = new GPoint(9, 2);
    baseIcon.infoShadowAnchor = new GPoint(18, 25);

    var icon = new GIcon(baseIcon);
    icon.image = "http://www.google.com/mapfiles/marker.png";
    var marker = new GMarker(point, icon);
    map.addOverlay(marker);
  }
}

    -->
    </script>
EOT;

}
function gen_gmap_code() {
return <<<EOT
<br/>
    <div id="map" style="width: 550px; height: 400px; border:1px; border-style: solid;">
    Loading...
    <noscript>
<span class='warning'>Sorry: you must enable javascript to view our maps.</span><br/>
    </noscript>
</div>


EOT;
}

?>
