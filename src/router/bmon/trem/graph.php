<?
/*
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

include "config.php";
include "charts.php";

$table = "";
$attr = $_GET["attr"];
$unit = $_GET["unit"];
$limit = $_GET["limit"];
$type = $_GET["type"];
$update = $_GET["update"];
$from_method = $_GET["from_method"];
$from_ts = $_GET["from_ts"];
$to_method = $_GET["to_method"];
$to_ts = $_GET["to_ts"];
$gwidth = $_GET["gwidth"];
$gheight = $_GET["gheight"];

if (!isset($attr) || !is_numeric($attr))
	return;

if (isset($limit)) {
	if (!is_numeric($limit))
		return;
} else
	$limit = 60;

if (!isset($gwidth) || $gwidth < 200)
	$gwidth = 550;

if (!isset($gheight) || $gheight < 50)
	$gheight = 150;

if (!isset($unit))
	return;

if (!isset($type) ||
    ($type != "l" && $type != "c" && $type != "sc" && $type != "a" &&
     $type != "sa"))
	$type = "l";

if (isset($update) && !is_numeric($update))
	return;

if (!isset($update) || $update < 0 || $update > 31536000)
	$update = 0;

if (isset($from_method) &&
    ($from_method != "auto" && $from_method != "sel"))
	return;

if (!isset($from_method))
	$from_method = "auto";

if (isset($to_method) &&
    ($to_method != "auto" && $to_method != "sel"))
	return;

if (!isset($to_method))
	$to_method = "auto";

if ($unit == "s")
	$table = "hist_s";
else if ($unit == "m")
	$table = "hist_m";
else if ($unit == "h")
	$table = "hist_h";
else if ($unit == "d")
	$table = "hist_d";
else
	return;

if ($type == "l")
	$rtype = "line";
else if ($type == "c")
	$rtype = "column";
else if ($type == "sc")
	$rtype = "stacked column";
else if ($type == "a")
	$rtype = "area";
else if ($type == "sa")
	$rtype = "stacked area";
else
	return;

mysql_connect($db_host, $db_user, $db_password);
mysql_select_db($db_name);

$query = "SELECT (concat(ts, '.', offset) - 0.0) ts, rx_rate, tx_rate
	FROM $table WHERE attr = " . mysql_escape_string($attr);

if ($from_method == "sel" && isset($from_ts))
	$query .= " AND ts > ". mysql_escape_string($from_ts);

if ($to_method == "sel" && isset($to_ts))
	$query .= " AND ts < ". mysql_escape_string($to_ts);

$query .= " ORDER BY ts DESC LIMIT " . mysql_escape_string($limit);

$r = mysql_query($query);
if (!$r) {
	print "<b>Database Error: " . mysql_error() . "</b><br>\n";
	return 1;
}

$a_ts = array();
$a_rx = array();
$a_tx = array();

$i = 0;
$max = 0;
$nr_cols = 0;

while (list($ts, $rx, $tx) = mysql_fetch_row($r)) {
	$nr_cols++;
	if ($unit == "s")
		array_push($a_ts, strftime("%H:%M:%S", $ts));
	else if ($unit == "m")
		array_push($a_ts, strftime("%H:%M:%S", $ts));
	else if ($unit == "h")
		array_push($a_ts, strftime("%H:%M", $ts));
	else if ($unit == "d")
		array_push($a_ts, strftime("%m/%d", $ts));

	if ($rx == 4294967295)
		array_push($a_rx, null);
	else
		array_push($a_rx, $rx);

	if ($tx == 4294967295)
		array_push($a_tx, null);
	else
		array_push($a_tx, $tx);

	if ($rx && $rx > $max)
		$max = $rx;
	if ($tx && $tx > $max)
		$max = $tx;
}

$divisor = find_divisor($max * 0.8);
$u = find_unit($divisor);

for ($i = 0; $i < count($a_rx); $i++)
	if ($a_rx[$i])
		$a_rx[$i] = $a_rx[$i] / $divisor;

for ($i = 0; $i < count($a_tx); $i++)
	if ($a_tx[$i])
		$a_tx[$i] = $a_tx[$i] / $divisor;

array_push($a_ts, "");
array_push($a_rx, "RX");
array_push($a_tx, "TX");

$chart["axis_category"] = array('skip'	=> (($nr_cols / 6) - 1),
				'bold'	=> false,
				'size'	=> 10);

$chart["axis_ticks"] = array('value_ticks'	=> true,
			     'category_ticks'	=> true,
			     'minor_count'	=> 4);

$chart["axis_value"] = array('decimals'		=> 0,
			     'bold'		=> false,
			     'size'		=> 10,
			     'suffix'		=> " $u",
			     'show_min'		=> false);

$chart["chart_bg"] = array('positive_color'	=> "EEEEEE");

$chart["chart_pref"] = array('line_thickness'	=> 1,
			     'point_shape'	=> "none");

if ($update > 0) {
	$chart["live_update"] = array('url'   => "$PHP_SELF?attr=$attr&unit=$unit&limit=$limit&type=$type&update=$update&time=".time(),
				      'delay' => $update);
}

$chart["chart_border"] = array('bottom_thickness' => 0);

$chart["chart_data"] = array(array_reverse($a_ts),
	array_reverse($a_rx), array_reverse($a_tx));

$chart["chart_rect"] = array('width' => $gwidth - 80,
			     'x' => $gheight - 90);

$chart["chart_value"] = array('position' => "cursor",
			      'bold' => false,
			      'size' => 12);

$chart["legend_rect"] = array('width' => 120,
			      'x' => ($gwidth / 2) - 40,
			      'y' => 5,
			      'height' => 10);

$chart["legend_label"] = array('bullet' => "line",
			       'size' => 11);

$chart["legend_bg"] = array('bg_alpha' => 0);

$chart["series_gap"] = array('bar_gap'	=> 0,
			     'set_gap'  => 0);

$chart["chart_type"] = $rtype;

mysql_close();

SendChartData($chart);
?>
