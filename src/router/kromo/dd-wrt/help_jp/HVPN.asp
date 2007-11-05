<% do_hpagehead("vpn.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("vpn.legend"); %></h2>
			<dl>
				<dd>このページでは、クライアントPCがVPNを利用した場合のパススルー設定を行うことができます。</dd>
				
				<dt><% tran("vpn.ipsec"); %></dt>
				<dd>異なる複数のユーザがIPSecを利用できるようになります。</dd>
				
				<dt><% tran("vpn.pptp"); %></dt>
				<dd>異なる複数のユーザがPPTPを利用できるようになります。</dd>
				
				<dt><% tran("vpn.l2tp"); %></dt>
				<dd>異なる複数のユーザがL2TPを利用できるようになります。</dd>
				
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div id="also">
			<b>See also:</b><br />
			<ul>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
