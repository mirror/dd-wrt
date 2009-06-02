<% do_pagehead("status_conn.titl"); %>

<style type="text/css">
A:link {text-decoration: none; color: black;}
A:hover {text-decoration: underline; color: black;}
</style>

<script type="text/javascript">
//<![CDATA[
function openGeotool(IP) {
	var top = 30;
	var left = Math.floor(screen.availWidth * .66) - 10;
	var width = 920
	var height = 700
	var win = window.open("http://geotool.flagfox.net/?ip=" + IP, 'Geotool', 'top=' + top + ',left=' + left + ',width=' + width + ',height=' + height + ",resizable=yes,scrollbars=yes,statusbar=no");
	addEvent(window, "unload", function() { if(!win.closed) win.close(); });
	win.focus();
}
//]]>
</script>
	</head>
	<body>
		<div class="popup">
			<form>
				<div id="bulle" class="bulle"></div>
				<h2><% tran("status_conn.h2"); %></h2>
				<div class="setting">
					<div class="label"><% tran("status_router.net_conntrack"); %></div>
					<% dumpip_conntrack(); %>
				</div><br />
				<table class="table" cellspacing="4" id="conntrack_table" summary="conntrack table">
					<tr>
						<th><% tran("filterSum.polnum"); %></th>
						<th><% tran("share.proto"); %></th>
						<th><% tran("share.timeout"); %></th>
						<th><% tran("share.src"); %></th>
						<th><% tran("share.dst"); %></th>
						<th><% tran("share.srv"); %></th>
						<th><% tran("share.state"); %></th>
					</tr>
					<% ip_conntrack_table(); %>
				</table>
				<script type="text/javascript">
				//<![CDATA[
				var t = new SortableTable(document.getElementById('conntrack_table'), 4000);
				//]]>
				</script>				
				<br />
				<div class="submitFooter">
					<script type="text/javascript">
					//<![CDATA[
					submitFooterButton(0,0,0,0,1,1);
					//]]>
					</script>
				</div>
			</form>
		</div>
	</body>
</html>