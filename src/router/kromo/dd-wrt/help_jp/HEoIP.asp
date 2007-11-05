<% do_hpagehead("eoip.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("eoip.legend"); %></h2>
			<dl>
				
				<dt><% tran("eoip.legend"); %></dt>
				<dd>Ethernet over IP (EoIP) Tunnelingは、2つのルータ間をIPレイヤの上に作成されたEthernetトンネルを作成し、データリンク層の通信を可能にするものです。EoIPインターフェースは通常のEthernetデバイスのように扱われ、Routerのブリッジ機能と組み合わせると、双方のセグメント間の通信を、あたかも同一セグメントであるかのように通信を行うことができます。<br/>
EoIPインターフェースの設定については、次のようなパターンが考えられます：<br/>
					<ul>
						<li>Inetnet上のルータ間のTunneling</li>
						<li>予め暗号かされたトンネル内で使用する</li>
						<li>無線LANのAd-Hocネットワーク間で利用する</li>
					</ul>
				</dd> 
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
