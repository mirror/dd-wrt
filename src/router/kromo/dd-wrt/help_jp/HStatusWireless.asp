<% do_hpagehead("status_wireless.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("status_wireless.h2"); %></h2>
			<dl>
				<dd>無線LANに関する情報を表示します。</dd>
				
				<dt><% tran("share.mac"); %></dt>
				<dd>無線LANデバイスのMACアドレスを表示します</dd>
				
				<dt><% tran("share.mode"); %></dt>
				<dd>無線LANが動作するモードを表示します</dd>
				
				<dt><% tran("share.ssid"); %></dt>
				<dd>無線LANデバイスが使用しているSSIDを表示します</dd>
				
				<dt><% tran("share.channel"); %></dt>
				<dd>無線LANデバイスが使用しているチャンネルを表示します</dd>
				
				<dt><% tran("wl_basic.TXpower"); %></dt>
				<dd>この機器の送信電波出力を表示します</dd>
				
				<dt><% tran("share.rates"); %></dt>
				<dd>現在使用している通信速度の最高速度を表示します</dd>
				
				<dt><% tran("share.encrypt"); %></dt>
				<dd>現在の暗号化モードを表示します</dd>
				
				<dd>「サイトサーベイ」ボタンをクリックすると、近隣の無線LANの利用状況が表示されます。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatus.asp">機器情報</a></li>
				<li><a href="HStatusLan.asp">LAN</a></li>
			</ul>
		</div>
	</body>
</html>