<% do_hpagehead("firewall.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("bmenu.firwall"); %></h2>
			<dl>
				
				<dt><% tran("firewall.proxy"); %></dt>
				<dd>文字列「Host:」が含まれるHTTP要求をBlockします。</dd>
				
				<dt><% tran("firewall.cookies"); %></dt>
				<dd>文字列「Cookie:」を持つHTTP要求を判別し、Cookieの仕様を制限します。</dd>
				
				<dt><% tran("firewall.applet"); %></dt>
				<dd>サフィクスが「.js」もしくは「.class」であるHTTP要求をBlockします。</dd>
				
				<dt><% tran("firewall.activex"); %></dt>
				<dd>URLのサフィクスに「.ocx」もしくは「.cab」が含まれるHTTP要求をBlockします。</dd>
				
				<dt><% tran("firewall.ping"); %></dt>
				<dd>有効である場合、Internet側からのPingに応答しません</dd>
				
				<dt><% tran("firewall.muticast"); %></dt>
				<dd>LAN側から転送されたMulticastパケットをフィルタします。</dd>
				
				<dt><% tran("filter.nat"); %></dt>
				<dd>LAN側ネットワークに転送されれる、送信元がInernet側アドレスで受信側がLAN側アドレスのフレーム(Port Redirectionされるパケット)をフィルタします。</dd>
				
				<dt><% tran("filter.port113"); %></dt>
				<dd>Internet側の113番ポートをフィルタします。</dd>
			
			</dl>
			
			<h2><% tran("log.h2"); %></h2>
			<dl>
				<dd>この機器を経由する全てのパケットをLogに記録します。</dd>
				
				<dt><% tran("log.legend"); %></dt>
				<dd>Logを記録する場合は「有効」をクリックしてください。停止する場合「無効」を選択してください。</dd>
				
				<dt><% tran("log.lvl"); %></dt>
				<dd>Logの出力レベルを設定できます。Log Levelを高くするほど詳細な情報を記録することができます。</dd>
				
				<dt><% tran("sbutton.log_in"); %></dt>
				<dd>入力パケットに関する情報を表示します。</td>
				
				<dt><% tran("sbutton.log_out"); %></dt>
				<dd>出力パケットに関する情報を表示します。</dd>
				
				<dd>変更した内容を保存するためには、「設定」ボタンをクリックしてください。変更を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>

			</dl>
			
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
			</ul>
		</div>
	</body>
</html>
