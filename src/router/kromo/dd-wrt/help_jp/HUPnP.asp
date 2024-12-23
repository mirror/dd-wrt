<% do_hpagehead("upnp.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("upnp.h2"); %></h2>
			<dl>
				<dd>Universal Plug and Play (UPnP)は、Microsoftが開発したルータなどの設定をクライアントPCなどから自動的に設定できるようにしたプロトコルです。</dd>
				
				<dt><% tran("upnp.legend"); %></dt>
				<dd>UPnPプロトコルによってクライアントPCから登録されたルールの一覧を表示します。ゴミ箱か、「全て削除」ボタンをクリックすることにより、登録されたルールの削除ができます。</dd>
				
				<dt><% tran("upnp.serv"); %></dt>
				<dd>UPnP機能を有効にします。</dd>
								
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
			</ul>
		</div>
	</body>
</html>
