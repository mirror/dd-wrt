<% do_hpagehead("status_lan.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>LAN</h2>
			<dl>
				<dd>LAN側ネットワークに関する情報を表示します。</dd>
				<dt>MACアドレス</dt>
				<dd>LAN側ネットワークで使用されているMACアドレスを表示します。</dd>
				<dt>IPアドレス・サブネットマスク</dt>
				<dd>LAN側ネットワークのIPアドレス・サブネットマスクをそれぞれ表示します。</dd>
				<dt>DHCPサーバ</dt>
				<dd>DHCPサーバの動作状態を表示します</dd>
				<dt>割り当て開始・終了アドレス</dt>
				<dd>DHCPサーバが割り当てるIPアドレスの範囲を表示します</dd>
				<dt>DHCPクライアント</dt>
				<dd>割り当て中のIPアドレスを表示します</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp">機器情報</a></li>
				<li><a href="HStatusWireless.asp">無線LAN</a></li>
			</ul>
		</div>
	</body>
</html>