<html>
	<head>
		<title><% nvram_get("router_name"); %> - Wi-viz 2.0: Wireless Network Visualization</title>
		<script type="text/javascript" src="js/wiviz2.js"></script>
		<link type="text/css" rel="stylesheet" href="style/wiviz2.css" />
		<!-- The proper way to deal with memory leaks -->
		<meta http-equiv="refresh" content="1800">
	</head>

<body>

<img class='logo' id='logo' src='images/wiviz/wiviz2logo-smaller.png' height=75 width=207>

<div class='mainarea' id='mainarea'>
</div>

<div class='controls'>
	<a href='javascript:cameraElevation(0.2)'><img src='images/wiviz/up.gif' alt='Up'></a>
	<a href='javascript:cameraElevation(-0.2)'><img src='images/wiviz/down.gif' alt='Down'></a>
	<a href='javascript:cameraZoom(1.2)'><img src='images/wiviz/zoomin.gif' alt='Zoom in'></a>
	<a href='javascript:cameraZoom(1/1.2)'><img src='images/wiviz/zoomout.gif' alt='Zoom out'></a>
	<a href='javascript:resetCameraWithFlair();'><img src='images/wiviz/reset.gif' alt='Reset view'></a>
</div>

<div class='rightmenu'>
	<div class='expando'>
		<div class='slidingheader' id='scanoptions' onClick='toggleExpando(this)'>
			Scanning options
		</div>
		<div class='slidingbodyshow'>
			<center><% tran("share.statu"); %></center>
			Monitoring<br>
			<center>Channel setting</center> 
			<form name="channelform" action="apply.cgi" method="post" />
				   	<input type="hidden" name="action" value="Apply" />
					<input type="hidden" name="change_action" value="gozila_cgi" />
					<input type="hidden" name="submit_button" value="Wiviz_Survey" />
						<input type="hidden" name="submit_type" value="Set" />
						<input type="hidden" name="commit" value="1" />
			<select name='hopseq' onChange='this.form.submit()'>
				<option value='0' <% nvram_selected("hopseq","0"); %> ><% tran("share.all"); %></option>
				<option value='1' <% nvram_selected("hopseq","1"); %> >1</option>
				<option value='2' <% nvram_selected("hopseq","2"); %> >2</option>
				<option value='3' <% nvram_selected("hopseq","3"); %> >3</option>
				<option value='4' <% nvram_selected("hopseq","4"); %> >4</option>
				<option value='5' <% nvram_selected("hopseq","5"); %> >5</option>
				<option value='6' <% nvram_selected("hopseq","6"); %> >6</option>
				<option value='7' <% nvram_selected("hopseq","7"); %> >7</option>
				<option value='8' <% nvram_selected("hopseq","8"); %> >8</option>
				<option value='9' <% nvram_selected("hopseq","9"); %> >9</option>
				<option value='10' <% nvram_selected("hopseq","10"); %> >10</option>
				<option value='11' <% nvram_selected("hopseq","11"); %> >11</option>
				<option value='12' <% nvram_selected("hopseq","12"); %> >12</option>
				<option value='13' <% nvram_selected("hopseq","13"); %> >13</option>
				<option value='14' <% nvram_selected("hopseq","14"); %> >14</option>
				<option value='1,6,11' <% nvram_selected("hopseq","1,6,11"); %> >1,6,11</option>
				<option value='1,3,6,8,11' <% nvram_selected("hopseq","1,3,6,8,11"); %> >1,3,6,8,11</option>
				<option value='1,2,3,4,5,6,7,8,9,10,11' <% nvram_selected("hopseq","1,2,3,4,5,6,7,8,9,10,11"); %> >1 to 11</option>
				<option value='1,2,3,4,5,6,7,8,9,10,11,12,13,14' <% nvram_selected("hopseq","1,2,3,4,5,6,7,8,9,10,11,12,13,14"); %> >1 to 14</option>
			</select>
			
			<center>Hopdwell (ms)</center> 
			<select name='hopdwell' onChange='this.form.submit()'>
				<option value='250' <% nvram_selected("hopdwell","250"); %> >250</option>
				<option value='500' <% nvram_selected("hopdwell","500"); %> >500</option>
				<option value='1000' <% nvram_selected("hopdwell","1000"); %> >1000</option>
				<option value='1500' <% nvram_selected("hopdwell","1500"); %> >1500</option>
				<option value='2000' <% nvram_selected("hopdwell","2000"); %> >2000</option>
				<option value='5000' <% nvram_selected("hopdwell","5000"); %> >5000</option>
				<option value='10000' <% nvram_selected("hopdwell","10000"); %> >10000</option>
			</select>


			</form>
		</div>
	</div>

	<div class='expando'>
		<div class='slidingheader' id='displayoptions' onClick='toggleExpando(this)'>
			Display options
		</div>
		<div class='slidingbody'>
			<center>Show/Hide</center>
			<input type='checkbox' id='apunenc' onClick='updatePrefs(this)'> AP - Unsecured<br>
			<input type='checkbox' id='apenc' onClick='updatePrefs(this)'> AP - Encrypted<br>
			<input type='checkbox' id='clientass' onClick='updatePrefs(this)'> Clients - Conn'd<br>
			<input type='checkbox' id='clientdiss' onClick='updatePrefs(this)'> Clients - Searching<br>
			<center>Effects</center>
			<input type='checkbox' id='rotate' onClick='updatePrefs(this)'> Rotate slowly<br>
			<input type='checkbox' id='grid' onClick='updatePrefs(this)'> Show grid<br>
			<input type='checkbox' id='animation' onClick='updatePrefs(this)'> Smooth animation<br>
			<input type='checkbox' id='blend' onClick='updatePrefs(this)'> Blending effects<br>
			<input type='checkbox' id='scale' onClick='updatePrefs(this)'> Scaling<br>
			<input type='checkbox' id='flyin' onClick='updatePrefs(this)'> Fly in on click<br>

		</div>
	</div>
	<div class='expando'>
		<div class='slidingheader' id='details' onClick='toggleExpando(this)'>
			 Details
		</div>
		<div class='slidingbody'>
			<center><span id='detail_type'>Nothing here yet</span></center>
			<span id='detail_info'>
			<i>Please select a host to view details.</i>
			</span>
			<span id='detail_network_actions' class='actionlist'>
<!--			<center>Network Actions</center>
			<a class='action' href='javascript:ap_join(this)'>Join this network</a> 
-->			</span>
			<span id='detail_ap_actions' class='actionlist'>
<!--			<center>AP Actions</center>
			<a class='action' href='javascript:ap_wds(this)'>Join via WDS</a>
			<a class='action' href='javascript:ap_copy(this)'>Copy this AP's settings</a>
-->			</span>
			<span id='detail_sta_actions' class='actionlist'>
<!--			<center>Foreign-client actions</center>
			<a class='action' href='javascript:sta_spy(this)'>Spy on this client</a> 
			<a class='action' href='javascript:sta_unblock(this)'>Unblock this MAC</a>
-->			</span>
			<span id='detail_sta_assoc_actions' class='actionlist'>
<!--			<center>Local-client actions</center>
			<a class='action' href='javascript:sta_disassoc(this)'>Disconnect this client</a>
			<a class='action' href='javascript:sta_block(this)'>Block this MAC</a>
			<a class='action' href='javascript:sta_static(this)'>Set DHCP static IP</a>
-->			</span>
			<span id='detail_local_actions' class='actionlist'>
<!--			<center>DD-WRT actions</center>
			<a class='action' href='javascript:ap_join_box(this)'>Join a network</a>
			<a class='action' href='javascript:ap_setup(this)'>AP settings</a>
-->			</span>
		</div> 

	</div>
<!--	<div class='expando' style='display:none'>
		<div class='slidingheader' id='clientspy' onClick='toggleExpando(this)'>
			Client spy
		</div>
		<div class='slidingbody'>
			Content<br>
		</div>
	</div> -->
		<div class='expando'>
		<div class='slidingheader' id='configuration' onClick="self.close()">
			<% tran("sbutton.clos"); %>
		</div>
		</div>
	
</div> 
<div class='poweredby'>By Nate True<br>Powered by DD-WRT</div>
<div class='debugger' id='debugger'></div>
<iframe class='wiviz' id='wivizGetFrame' src='about:blank'></iframe>
<iframe class='wiviz' id='wivizSetFrame' name='wivizSetFrame' src='about:blank'></iframe>
</body>
</html>
