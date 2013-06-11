<% do_hpagehead("bmenu.servicesMilkfish"); %>

<!--##################################################################
# This program is free software; you can redistribute it and/or      #
# modify it under the terms of the GNU General Public License as     #
# published by the Free Software Foundation; either version 3 of the #
# License, or (at your option) any later version.                    #
#                                                                    #
# This program is distributed in the hope that it will be useful,    #
# but WITHOUT ANY WARRANTY; without even the implied warranty of     #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      #
# GNU General Public License for more details.                       #
#                                                                    #
# You should have received a copy of the GNU General Public License  #
# along with this program; if not, write to the                      #
# Free Software Foundation, Inc.,                                    #
# 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA             #
######################################################################
# This file is part of a Milkfish Embedded OpenSER SIP Router Setup  #
# and was derived from similar files in this folder by other authors #
#                                                                    #
# The Milkfish-dd Web Interface - Milkfish Help Page                 #
#                                                                    #
# Built/Version:  20080506                                           #
# Co-Author/Contact: Michael Poehnl <budrus@sipwerk.com>             #
# Copyright (C) 2008 partly by sipwerk - All rights reserved.        #
#                                                                    #
# Please note that this software is under development and comes with #
# absolutely no warranty, to the extend permitted by applicable law. #
###################################################################-->

	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("bmenu.servicesMilkfish"); %></h2>
			<dl>
				<dt>Milkfish-dd - Professional VoIP/IM Router - v1.0</dt>
				<ul class="wide">
				<br><b>Welcome to the Milkfish on DD-WRT...</b><br>
				The Milkfish enables the operation and management of several SIP phones in spite of NAT.<br><br>
				<u>Provider Phone Configuration</u>: The IP address of this router is the <b>Outbound Proxy</b> or <b>Outbound Server</b> and <b>STUN is deactivated</b>.<br>
				All other settings should be set according to the providers recommendation.<br><br>
				<u>Local Phone Configuration</u>: The IP address of this router is the <b>SIP Server</b> or <b>Registrar Server</b> and <b>a local subscriber account was added</b>.<br>
				<br>Dynamic SIP needs a valid User Account set to work. Homesip Users need to be registered at the Milkfish Forum.<br>
<!--				<br>Key features are...<br>
				<li>the first free and <a href="http://wiki.milkfish.org/index.php?n=MilkfishServices.Homesip" target="new">user specific alphanumeric dialing</a> infrastructure - choose <b>yourname.homesip.net</b></li>
				<li>improved NAT traversal by means of a combined SIP-RTP-proxy - equipped with a selective STUNBLOCKER</li>
				<li>support for LAN-connected PBX-Adapter hardwares - find an example <a href="http://wiki.milkfish.org/index.php?n=TheMilkfish.FritzBox" target="new">here</a></li>
				<li>quick dialling through user-defined Aliases (e.g. call local "123" and reach "alphonse.numeric@myrouter.homesip.net")</li>
				<li>reduction of internet traffic by internal routing of calls among local phones</li>
				<li>direct dialling through the <a href="http://www.sipbroker.com" target="new">SIPBroker</a> Network Directory using <b>*1</b> as a prefix before the SIPBroker <i>SIP-Code</i></li>
				<li>SIP/SIMPLE messaging via web interface</li>
				<li>Find more feature descriptions <a href="http://www.milkfish.org/?page_id=7" target="new">here</a> or <a href="http://wiki.milkfish.org/index.php?n=Milkfish-dd.Features" target="new">there</a>...</li>
-->				</ul><br />
					<div class="note">
						<h4>Get support...</h4>
						<div><br />
						Find the documentation of this embedded software at <a href="http://wiki.milkfish.org" target="new">wiki.milkfish.org</a>.<br>
						Any questions can go to the user forum being available at <a href="http://forum.milkfish.org" target="new">forum.milkfish.org</a>.</div>
					</div><br />
					<div class="note">
						<h4>Give support...</h4>
						<div><br />
						You like the Milkfish? - Then support us through <b>donations@milkfish.org</b>:&nbsp;&nbsp;<br />
						<!-- Begin moneybookers button code --> <a target="_blank" href="http://www.moneybookers.com/app/send.pl"><img border="0" style="border-color: #8b8583; border-width: 1px" src="../images/88_en_interpayments.gif" /></a> <!-- End of moneybookers button code -->
						<br>
						<form action="https://www.paypal.com/cgi-bin/webscr" method="post" target="_blank"><input type="hidden" name="cmd" value="_s-xclick"><input type="image" src="../images/paypal.png" border="0" name="submit" alt=""><img alt="" border="0" src="https://www.paypal.com/de_DE/i/scr/pixel.gif" width="1" height="1"><input type="hidden" name="encrypted" value="-----BEGIN PKCS7-----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-----END PKCS7-----"></form>
						<br></div>
					</div><br />
					<div class="note">
						<h4>Legal</h4>
						<div>
						Copyright © 2005-2008 by <a href="http://www.milkfish.org" target="new">The Milkfish Project</a>. All rights reserved.<br />
						Logos and trademarks are the property of their respective owners.<br />
						The Milkfish software is licensed under the <a href="http://www.gnu.org/licenses/gpl.html" target="new">GNU General Public License</a>.<br />
						Please note that this software is under development and comes with absolutely no warranty, to the extend permitted by applicable law.</div>
					</div><br />
					<div class="note">
        					<div class="center">Milkfish is developed by...<br><a target="_blank" href="http://www.sipwerk.com"><img border="1" src="../images/sipwerk.png" alt="sipwerk logo" /></a><br></div><br />
					</div><br />
				</dd>
				<dd>Click <i>Save Settings</i> to save your settings or click <i>Cancel Changes</i> to cancel your unsaved changes.</dd>
			</dl>
		</div>
<!--		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
-->	</body>
</html>

