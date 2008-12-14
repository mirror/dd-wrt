<% do_hpagehead("idx.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>Index</h2>
			<dl>
				<dt><% tran("pforward.app"); %></dt>
				<dd>
					<ul>
						<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
						<li><a href="HDDNS.asp"><% tran("bmenu.setupddns"); %></a></li>
						<li><a href="HWanMAC.asp"><% tran("bmenu.setupmacclone"); %></a></li>
						<li><a href="HRouting.asp"><% tran("bmenu.setuprouting"); %></a></li>
						<li><a href="HNetworking.asp"><% tran("bmenu.networking"); %></script></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.wireless"); %></dt>
				<dd>
					<ul>
						<li><a href="HWireless.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
						<li><a href="Hradauth.asp"><% tran("bmenu.wirelessRadius"); %></a></li>
						<li><a href="HWPA.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
						<li><a href="HWirelessMAC.asp"><% tran("bmenu.wirelessMac"); %></a></li>
						<li><a href="HWirelessAdvanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
						<li><a href="HWDS.asp"><% tran("bmenu.wirelessWds"); %></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.services"); %></dt>
				<dd>
					<ul>
						<li><a href="HServices.asp"><% tran("bmenu.servicesServices"); %></a></li>
						<% ifndef("PPPOESERVER", "<!--"); %>
						<li><a href="HPPPoE_Server.asp"><% tran("bmenu.servicesPppoesrv"); %></a></li>
						<% ifndef("PPPOESERVER", "-->"); %>
						<li><a href="HPPTP.asp"><% tran("bmenu.servicesPptp"); %></a></li>
						<li><a href="HUSB.asp"><% tran("bmenu.servicesUSB"); %></a></li>
						<li><a href="HNAS.asp"><% tran("bmenu.servicesNAS"); %></a></li>
						<li><a href="HHotspot.asp"><% tran("bmenu.servicesHotspot"); %></a></li>
						<% ifndef("MILKFISH", "<!--"); %>
						<li><a href="HMilkfish.asp"><% tran("bmenu.servicesMilkfish"); %></a></li>
						<% ifndef("MILKFISH", "-->"); %>
						<li><a href="HAnchorFree.asp"><% tran("bmenu.servicesAnchorFree"); %></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.security"); %></dt>
				<dd>
					<ul>
						<li><a href="HFirewall.asp"><% tran("bmenu.firwall"); %></a></li>
						<li><a href="HVPN.asp"><% tran("bmenu.vpn"); %></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.accrestriction"); %></dt>
				<dd>
					<ul>
						<li><a href="HFilters.asp"><% tran("bmenu.webaccess"); %></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.applications"); %></dt>
				<dd>
					<ul>
						<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
						<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
						<li><a href="HTrigger.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
						<li><a href="HUPnP.asp"><% tran("bmenu.applicationsUpnp"); %></a></li>
						<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
						<li><a href="HQos.asp"><% tran("bmenu.applicationsQoS"); %></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.admin"); %></dt>
				<dd>
					<ul>
						<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
						<li><a href="HAlive.asp"><% tran("bmenu.adminAlive"); %></a></li>
						<li><a href="HDiagnostics.asp"><% tran("bmenu.adminDiag"); %></a></li>
						<li><a href="HWol.asp"><% tran("bmenu.adminWol"); %></a></li>
						<li><a href="HDefault.asp"><% tran("bmenu.adminFactory"); %></a></li>
						<li><a href="HUpgrade.asp"><% tran("bmenu.adminUpgrade"); %></a></li>
						<li><a href="HBackup.asp"><% tran("bmenu.adminBackup"); %></a></li>
					</ul>
				</dd>
				<dt><% tran("bmenu.statu"); %></dt>
				<dd>
					<ul>
						<li><a href="HStatus.asp"><% tran("bmenu.statuRouter"); %></a></li>
						<li><a href="HStatusLan.asp"><% tran("bmenu.statuLAN"); %></a></li>
						<li><a href="HStatusWireless.asp"><% tran("bmenu.statuWLAN"); %></a></li>
					</ul>
				</dd>
				<dt>Online / DD-WRT Community</dt>
				<dd>
					<ul>
						<li><a href="http://www.dd-wrt.com" target="_new">DD-WRT Homepage</a></li>
						<li><a href="http://www.dd-wrt.com/forum" target="_new">Forum</a></li>
						<li><a href="http://www.dd-wrt.com/wiki" target="_new">Wiki</a></li>
						<li><a href="http://www.dd-wrt.com/bugtracker" target="_new">Bugtracker</a></li>
					</ul>
				</dd>
			</dl>
		</div>
	</body>
</html>
