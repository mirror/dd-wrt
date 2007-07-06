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
?>
<!DOCTYPE html PUBLIC "-//W3c/DTD XHTML 1.0 Strict//EN" 
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<?
	include "config.php";
	include "charts.php";

	function db_err()
	{
		print "<b>Database Error: ". mysql_error() ."</b>\n";
		return 1;
	}

	$item		= $_GET["item"];
	$node		= $_GET["node"];
	$attr		= $_GET["attr"];
	$unit		= $_GET["unit"];
	$type		= $_GET["type"];
	$limit		= $_GET["limit"];
	$gwidth		= $_GET["gwidth"];
	$gheight	= $_GET["gheight"];
	$update		= $_GET["update"];
	$from_method	= $_GET["from_method"];
	$to_method	= $_GET["to_method"];
	$from_month	= $_GET["from_month"];
	$from_day	= $_GET["from_day"];
	$from_year	= $_GET["from_year"];
	$from_hour	= $_GET["from_hour"];
	$from_min	= $_GET["from_min"];
	$from_sec	= $_GET["from_sec"];
	$from_usec	= $_GET["from_usec"];
	$to_month	= $_GET["to_month"];
	$to_day		= $_GET["to_day"];
	$to_year	= $_GET["to_year"];
	$to_hour	= $_GET["to_hour"];
	$to_min		= $_GET["to_min"];
	$to_sec		= $_GET["to_sec"];
	$to_usec	= $_GET["to_usec"];

	if (isset($item) && !is_numeric($item))
		return;

	if (isset($node) && !is_numeric($node))
		return;

	if (isset($attr) && !is_numeric($attr))
		return;

	if (isset($limit) && !is_numeric($limit))
		return;

	if (isset($update) && !is_numeric($update))
		return;

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

	if (!isset($update) || $update < 0 || $update > 31536000)
		$update = 0;

	if (!isset($limit) || ($limit < 0 || $limit > 16384))
		$limit = 100;

	if (!isset($unit) ||
	    ($unit != "s" && $unit != "m" && $unit != "h" && $unit != "d"))
		$unit = "m";

	if (!isset($type) ||
	    ($type != "l" && $type != "c" && $type != "sc" && $type != "a" &&
	     $type != "sa"))
		$type = "l";

	if (!isset($gwidth) || $gwidth < 200)
		$gwidth = 550;
	
	if (!isset($gheight) || $gheight < 50)
		$gheight = 150;

	$now = getdate();

	if (!isset($from_month) || $from_month < 1 || $from_month > 12 ||
	    isset($from_now))
		$from_month = $now['mon'];

	if (!isset($from_year) || $from_year < 1900 || isset($from_now))
		$from_year = $now['year'];

	if (!isset($from_day) || $from_day < 1 || $from_day > 31 ||
	    isset($from_now))
		$from_day = $now['mday'];

	if (!isset($from_hour) || $from_hour < 0 || $from_hour > 24 ||
	    isset($from_now))
		$from_hour = $now['hours'];

	if (!isset($from_min) || $from_min < 0 || $from_min > 59 ||
	    isset($from_now))
		$from_min = $now['minutes'];
	
	if (!isset($from_sec) || $from_sec < 0 || $from_sec > 59 ||
	    isset($from_now))
		$from_sec = $now['seconds'];

	if (!isset($from_usec) || $from_usec < 0 || $from_usec > 999999)
		$from_usec = 0;

	if (!isset($to_month) || $to_month < 1 || $to_month > 12 ||
	    isset($to_now))
		$to_month = $now['mon'];

	if (!isset($to_year) || $to_year < 1900 || isset($to_now))
		$to_year = $now['year'];

	if (!isset($to_day) || $to_day < 1 || $to_day > 31 || isset($to_now))
		$to_day = $now['mday'];

	if (!isset($to_hour) || $to_hour < 0 || $to_hour > 24 || isset($to_now))
		$to_hour = $now['hours'];

	if (!isset($to_min) || $to_min < 0 || $to_min > 59 || isset($to_now))
		$to_min = $now['minutes'];
	
	if (!isset($to_sec) || $to_sec < 0 || $to_sec > 59 || isset($to_now))
		$to_sec = $now['seconds'];

	if (!isset($to_usec) || $to_usec < 0 || $to_usec > 999999)
		$to_usec = 0;

	$from_ts = mktime($from_hour, $from_min, $from_sec, $from_month,
			  $from_day, $from_year);
	$to_ts = mktime($to_hour, $to_min, $to_sec, $to_month, $to_day,
			$to_year);
?>
<head>
<title>Traffic Rate Estimation and Monitoring</title>
<link rel="stylesheet" type="text/css" href="layout.css" />
</head>
<body>
<?
	mysql_connect($db_host, $db_user, $db_password);
	mysql_select_db($db_name);
?>
<table class="overall" align="center">
  <tr>
    <td colspan="2" class="header">
      <p class="banner">Traffic Rate Estimation and Monitoring</p>
    </td>
  </tr>
  <tr>
    <td class="left_col">
      <ul class="node_list">
      <!-- node list -->
<?
	function list_sub_items($parent, $node)
	{
		$r = mysql_query("SELECT id, name FROM items WHERE parent = ".
				 mysql_escape_string($parent) ." AND node = ".
				 mysql_escape_string($node));

		if (!$r)
			return db_err();

		if (mysql_num_rows($r) > 0) {
			print "<ul class=\"sub_intf\">\n";
			while (list($id, $name) = mysql_fetch_row($r)) {
				print "<li><a class=\"a_intf_link\" ".
				      "href=\"$PHP_SELF?node=$node".
				      "&amp;item=$id\">$name</a>\n";
				list_sub_items($id, $node);
				print "</li>\n";
			}
			print "</ul>\n";
		}
	}

	function list_nodes()
	{
		$r = mysql_query("SELECT id, name from nodes");
		if (!$r)
			return db_err();

		while (list($id, $name) = mysql_fetch_row($r)) {
			print "<li><a class=\"a_node_link\" ".
			      "href=\"$PHP_SELF?node=$id\">$name</a>";

			$n = mysql_query("SELECT id, name FROM items WHERE ".
			                 "parent IS NULL AND node = ".
					 mysql_escape_string($id));

			if (!$n)
				return db_err();

			print "<ul class=\"node_intf_list\">\n";
			while (list($i_id, $i_name) = mysql_fetch_row($n)) {
				print "<li><a class=\"a_intf_link\" ".
				      "href=\"$PHP_SELF?node=$id".
				      "&amp;item=$i_id\">$i_name</a>\n";
				list_sub_items($i_id, $id);
				print "</li>\n";
			}
			print "</ul>\n</li>\n";
		}
	}

	function print_intf_list()
	{
		global $node;

		$r = mysql_query("SELECT name FROM nodes WHERE id = "
			. mysql_escape_string($node));
		if (!$r)
			return db_err();

		while (list($name) = mysql_fetch_row($r))
			print "<p class=\"node_title\">$name</p>\n";

		print	"<table class=\"intf_list\">\n".
			"  <tr class=\"intf_list_hdr\">\n".
			"    <th class=\"intf_list_hdr_classx\">#</th>\n".
			"    <th class=\"intf_list_hdr_name\">Name</th>\n".
			"    <th class=\"intf_list_hdr_rx\">RX</th>\n".
			"    <th class=\"intf_list_hdr_rxp\">#</th>\n".
			"    <th class=\"intf_list_hdr_tx\">TX</th>\n".
			"    <th class=\"intf_list_hdr_txp\">#</th>\n".
			"  </tr>\n";
	
		$r = mysql_query("SELECT i.id, i.name, i.indent, ab.rx_rate, ".
				 "ab.tx_rate, ap.rx_rate, ap.tx_rate FROM ".
				 "items AS i, attrs AS ab, attrs AS ap ".
				 "WHERE ab.item = i.id AND ap.item = i.id ".
				 "AND ab.name = 'bytes' AND ap.name = ".
				 "'packets' AND i.node = ".
				 mysql_escape_string($node));
		if (!$r)
			return db_err();

		$inr = 0;

		while (list($id, $name, $indent, $brx, $btx, $prx, $ptx)
					= mysql_fetch_row($r)) {
			$rd = find_divisor($brx);
			$td = find_divisor($btx);
			print	"  <tr class=\"intf_list_row\">\n".
				"    <td class=\"intf_list_nr\">$inr</td>\n".
				"    <td class=\"intf_list_name\">";

			for ($i = 0; $i < $indent; $i++)
				print "&nbsp;&nbsp";

			print "<a class=\"a_intf\" href=\"$PHP_SELF?".
			      "node=$node&amp;item=$id\">$name</a></td>\n".
			      "    <td class=\"intf_list_rx\">";
			printf("%.2f %s", $brx / $rd, find_unit($rd));
			print "</td>\n".
			      "    <td class=\"intf_list_rxp\">$prx</td>\n".
			      "    <td class=\"intf_list_tx\">";
			printf("%.2f %s", $btx / $td, find_unit($td));
			print "</td>\n".
			      "    <td class=\"intf_list_txp\">$ptx</td>\n".
			      "  </tr>\n";
			$inr++;
		}

		print "</table>\n";
	}

	function print_intf_section()
	{
		global $node, $item, $attr, $unit, $limit, $type;
		global $update, $from_method, $from_ts, $from_usec;
		global $to_method, $to_ts, $to_usec, $gwidth, $gheight;
		global $from_month, $from_day, $from_year, $from_hour;
		global $from_min, $from_sec, $to_month, $to_day, $to_year;
		global $to_hour, $to_min, $to_sec;

		$r = mysql_query("SELECT name FROM items WHERE id = "
			. mysql_escape_string($item));
		if (!$r)
			return db_err();

		while (list($name) = mysql_fetch_row($r))
			print "<p class=\"title\">$name</p>\n";

		$r = mysql_query("SELECT a.id, a.name, a.rx_counter, ".
				 "a.rx_rate, a.tx_counter, a.tx_rate, ".
				 "d.is_num, d.txt FROM attrs AS a, ".
				 "attr_desc AS d WHERE d.id = a.name ".
				 "AND a.item = ".
				 mysql_escape_string($item));

		$attrs = array();
		
		if (!$r)
			return db_err();

		while (list($id, $name, $rxc, $rxr, $txc, $txr, $is_num, $txt)
					= mysql_fetch_row($r)) {
			array_push($attrs, array($id, $name, $rxc,
						 $rxr, $txc, $txr,
						 $is_num, $txt));
			if ($name == "bytes" && !isset($attr))
				$attr = $id;
		}

		print "<table class=\"graph\">\n".
		      "  <tr>\n".
		      "    <td>\n";
		print InsertChart("charts.swf",
				  "graph.php?attr=$attr".
				  "&unit=$unit".
				  "&limit=$limit".
				  "&type=$type".
				  "&update=$update".
				  "&from_method=$from_method".
				  "&from_ts=$from_ts.$from_usec".
				  "&to_method=$to_method".
				  "&to_ts=$to_ts.$to_usec".
				  "&gwidth=$gwidth".
				  "&gheight=$gheight",
				  $gwidth, $gheight, "F8F8F8", true);

		print "\n".
		      "    </td>\n".
		      "  </tr>\n".
		      "  <tr>\n".
		      "    <td class=\"gconf_td\">\n".
		      "<form method=\"get\" action=\"$PHP_SELF\">\n".
		      "<table class=\"gconf\">\n".
		      "  <tr>\n".
		      "    <td>\n".
		      "    <input type=\"hidden\" name=\"item\" ".
		                 "value=\"$item\" />\n".
		      "    <input type=\"hidden\" name=\"node\" ".
		                 "value=\"$node\" />\n".
		      "    <p class=\"glabel\">Attribute:<br /></p>\n".
		      "    <select class=\"attribute\" name=\"attr\" ".
		                  "size=\"1\">\n";

		foreach($attrs as $a) {
			if ($a[3] != null) {
				print "      <option value=\"$a[0]\"";
				if ($attr == $a[0])
					print " selected";
				print ">$a[7]</option>\n";
			}
		}

		print "    </select>\n".
		      "    </td>\n".
		      "    <td>\n".
		      "    <p class=\"glabel\">Unit Table:<br /></p>\n".
		      "    <select name=\"unit\" size=\"1\">\n";

		printf("     <option value=\"s\" %s>Seconds</option>\n",
		       $unit == "s" ? "selected" : "");

		printf("     <option value=\"m\" %s>Minutes</option>\n",
		       $unit == "m" ? "selected" : "");

		printf("     <option value=\"h\" %s>Hours</option>\n",
		       $unit == "h" ? "selected" : "");

		printf("     <option value=\"d\" %s>Days</option>\n",
		       $unit == "d" ? "selected" : "");

		print "    </select>\n".
		      "    </td>\n".
		      "    <td>\n".
		      "    <p class=\"glabel\">X-Grid:<br /></p>\n".
		      "    <input type=\"text\" name=\"limit\" ".
		                 "value=\"$limit\" size=\"10\" />\n".
		      "    </td>\n".
		      "    <td>\n".
		      "    <p class=\"glabel\">Graph Type:<br /></p>\n".
		      "    <select name=\"type\">\n";


		printf("      <option value=\"l\" %s>Lines</option>\n",
		       $type == "l" ? "selected" : "");

		printf("      <option value=\"c\" %s>Column</option>\n",
		       $type == "c" ? "selected" : "");

		printf("      <option value=\"sc\" %s>Stacked Column</option>\n",
		       $type == "sc" ? "selected" : "");

		printf("      <option value=\"a\" %s>Area</option>\n",
		       $type == "a" ? "selected" : "");

		printf("      <option value=\"sa\" %s>Stacked Area</option>\n",
		       $type == "sa" ? "selected" : "");

		print "    </select>\n".
		      "    </td>\n".
		      "  </tr>\n".
		      "  <tr>\n".
		      "    <td colspan=\"2\">\n".
		      "    <p class=\"glabel\">From:<br /></p>\n";

		printf("    <input type=\"radio\" name=\"from_method\" ".
		                   "value=\"auto\" %s />\n",
		       $from_method == "auto" ? "checked" : "");

		print "    <span class=\"gtext\">Latest to now</span>\n".
		      "  </td>\n".
		      "  <td colspan=\"2\">\n".
		      "    <p class=\"glabel\">To:<br /></p>\n";

		printf("    <input type=\"radio\" name=\"to_method\" ".
		                  "value=\"auto\" %s />\n",
		       $to_method == "auto" ? "checked" : "");

		print "    <span class=\"gtext\">Automatic</span><br />\n".
		      "    </td>\n".
		      "  </tr>\n".
		      "  <tr>\n".
		      "    <td colspan=\"2\" style=\"padding: 0px:\">".
		      "    <table style=\"padding: 0px; margin: 0px; ".
			     	         "border-spacing: 0px; ".
				         "border-collapse:collapse;\">".
		      "      <tr>\n".
		      "        <td>\n";

		printf("       <input type=\"radio\" name=\"from_method\" ".
		                     "value=\"sel\" %s />\n",
		       $from_method == "sel" ? "checked" : "");

		print "        </td>\n".
		      "        <td>\n".
		      "        <input type=\"text\" name=\"from_month\"".
		                           "size=\"2\" value=\"$from_month\" />".
		      "        <span class=\"gtext\">/</span>".
		      "        <input type=\"text\" name=\"from_day\" ".
		                            "size=\"2\" value=\"$from_day\" />".
		      "        <span class=\"gtext\">/</span>".
		      "        <input type=\"text\" name=\"from_year\" ".
		                            "size=\"4\" value=\"$from_year\" />".
		      "        <input type=\"submit\" name=\"from_now\" ".
		                     "value=\"now\" />\n".
		      "        <p style=\"margin: 0px; margin-top: 3px;\">\n".
		      "        <input type=\"text\" name=\"from_hour\" ".
		                     "size=\"2\" value=\"$from_hour\" />".
		      "        <span class=\"gtext\">:</span>".
		      "        <input type=\"text\" name=\"from_min\" ".
		                     "size=\"2\" value=\"$from_min\" />".
		      "        <span class=\"gtext\">:</span>".
		      "        <input type=\"text\" name=\"from_sec\" ".
		                     "size=\"2\" value=\"$from_sec\" />".
		      "        <span class=\"gtext\">:</span>".
		      "        <input type=\"text\" name=\"from_usec\" ".
		                     "size=\"6\" value=\"$from_usec\" />\n".
		      "        </p>\n".
		      "       </td>\n".
		      "      </tr>\n".
		      "    </table>\n".
		      "    </td>\n".
		      "    <td colspan=\"2\" style=\"padding: 0px:\">".
		      "    <table style=\"padding: 0px; margin: 0px; ".
			     	         "border-spacing: 0px; ".
				         "border-collapse:collapse;\">".
		      "      <tr>\n".
		      "        <td>\n";

		printf("       <input type=\"radio\" name=\"to_method\" ".
		                     "value=\"sel\" %s />\n",
		       $to_method == "sel" ? "checked" : "");

		print "        </td>\n".
		      "        <td>\n".
		      "        <input type=\"text\" name=\"to_month\"".
		                           "size=\"2\" value=\"$to_month\" />".
		      "        <span class=\"gtext\">/</span>".
		      "        <input type=\"text\" name=\"to_day\" ".
		                            "size=\"2\" value=\"$to_day\" />".
		      "        <span class=\"gtext\">/</span>".
		      "        <input type=\"text\" name=\"to_year\" ".
		                            "size=\"4\" value=\"$to_year\" />".
		      "        <input type=\"submit\" name=\"to_now\" ".
		                     "value=\"now\" />\n".
		      "        <p style=\"margin: 0px; margin-top: 3px;\">\n".
		      "        <input type=\"text\" name=\"to_hour\" ".
		                     "size=\"2\" value=\"$to_hour\" />".
		      "        <span class=\"gtext\">:</span>".
		      "        <input type=\"text\" name=\"to_min\" ".
		                     "size=\"2\" value=\"$to_min\" />".
		      "        <span class=\"gtext\">:</span>".
		      "        <input type=\"text\" name=\"to_sec\" ".
		                     "size=\"2\" value=\"$to_sec\" />".
		      "        <span class=\"gtext\">:</span>".
		      "        <input type=\"text\" name=\"to_usec\" ".
		                     "size=\"6\" value=\"$to_usec\" />\n".
		      "        </p>\n".
		      "       </td>\n".
		      "      </tr>\n".
		      "    </table>\n".
		      "    </td>\n".
		      "  </tr>\n".
		      "  <tr>\n".
		      "    <td>\n".
		      "    <p class=\"glabel\">Graph Width:<br /></p>\n".
		      "    <input type=\"text\" name=\"gwidth\" ".
		                 "value=\"$gwidth\" size=\"6\" />\n".
		      "    <span class=\"gtext\">px</span>\n".
		      "    </td>\n".
		      "    <td>\n".
		      "    <p class=\"glabel\">Graph Height:<br /></p>\n".
		      "    <input type=\"text\" name=\"gheight\" ".
		                 "value=\"$gheight\" size=\"6\" />\n".
		      "    <span class=\"gtext\">px</span>\n".
		      "    </td>\n".
		      "    <td>\n".
		      "    <p class=\"glabel\">Live Update:<br /></p>\n".
		      "    <input type=\"text\" name=\"update\" ".
		                 "value=\"$update\" size=\"6\" />\n".
		      "    <span class=\"gtext\">sec</span>\n".
		      "    </td>\n".
		      "    <td style=\"text-align: right; vertical-align: ".
		                      "bottom;\">\n".
		      "    <input type=\"submit\" name=\"do_graph\" ".
		                 "value=\"Configure\" />\n".
		      "    </td>\n".
		      "  </tr>\n".
		      "</table>\n".
		      "</form>\n".
		      "    </td>\n".
		      "  </tr>\n".
		      "</table>\n";

		print "<table class=\"details\">\n".
		      "  <tr class=\"details_hdr\">\n".
		      "    <th class=\"details_hdr_name\">Details</th>\n".
		      "    <th class=\"details_hdr_rx\">RX</th>\n".
		      "    <th class=\"details_hdr_tx\">TX</th>\n".
		      "  </tr>\n";

		foreach($attrs as $a) {
			print "  <tr class=\"details\">\n".
			      "    <td class=\"details_name\">$a[7]</td>\n".
			      "    <td class=\"details_rx\">";

			if ($a[6] == 0) {
				$d = find_divisor($a[2]);
				printf("%.2f %s", $a[2] / $d, find_unit($d));
			} else
				print $a[2];
			
			print "</td>\n".
			      "    <td class=\"details_tx\">";

			if ($a[6] == 0) {
				$d = find_divisor($a[4]);
				printf("%.2f %s", $a[4] / $d, find_unit($d));
			} else
				print $a[4];

			print "</td>\n".
			      "  </tr>\n";
		}

		print "</table>\n";
	}

	list_nodes();
	
?>
      <!-- end node list -->
      </ul>
    </td>
    <td class="right_col">
      <!-- right column -->

<?
	if (isset($node)) {
		print_intf_list();
		if (isset($item))
			print_intf_section();
	}
?>

      <!-- end right column -->
    </td>
  </tr>
  <tr>
    <td colspan="2" class="footer">
      <p class="footer">Last Updated: <? print strftime("%D %T"); ?><br />
      Automatically generated by <a href="http://people.suug.ch/~tgr/bmon/">bmon</a>.</p>
    </td>
  </tr>
</table>

<?
	mysql_close();
?>

</body>
</html>
