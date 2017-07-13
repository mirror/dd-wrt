<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=600, initial-scale=0.8">
    <meta name="robots" content="noindex, nofollow">
    <meta charset="utf-8">
<script type="text/javascript">
//<![CDATA[
var val = <% nvg("speedchecker_enable"); %>;
var RID = '';
var SECRET = '';
var server = '';
var port = '';
if (val==1) {
	RID = '<% nvg("speedchecker_uuid"); %>';
	secret = '<% nvg("speedchecker_uuid2"); %>';
	server = '<% nvg("lan_ipaddr"); %>';
	port = '<% nvg("http_lanport"); %>';
}
//]]>
</script>
    <script src="https://speedchecker.dd-wrt.com/public/demo/page-speedtest.js"></script>
</head>
<body>
</body>
</html>
