<% do_hpagehead("wds.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><%% tran("wds.h2"); %%></h2>
			<dl>
				<dd>WDS (Wireless Distribution System)はアクセスポイント同士のみ、またはInfrastructureモードの混在を含むアクセスポイント同士の通信を行うことができる機能です。(Infrastructureモードを混在する場合は、パフォーマンスが低下します)この製品では「WDS」,「Point to Point」の2つのモードを使用することができます。</dd>
				
				<dt>LANモードWDS</dt>
				<dd>通常利用されるWDSの形態で、ルーティングに関する知識なく設定を行うことができます。最も一般的な利用方法では、もう一つのAPをこのモードで接続し、Bridgeを構成することで通信範囲を広げることができます。
					<ol class="wide">
						<li>それぞれのAPにおいて、<a href="HWireless.asp">[無線LAN]-[基本]</a>と、<a href="HWPA.asp">[無線セキュリティ]</a>の設定を全て同じパラメータで構成します。</li>
						<li>それぞれのAPのWDS接続先設定において、一番左のBoxから「LAN」を選択します</li>
						<li>1番目のルータの無線LANデバイスのMACアドレスを、2番目のルータに設定します。</li>
						<li>2番目のルータの無線LANデバイスのMACアドレスを1番目のルータに設定します。</li>
						<li>上記設定に間違いが無いのを確認し、「設定」をクリックします。</li>
						<li>[機器診断] - [無線LAN]をクリックし、「WDS接続先」の接続内容を確認します。信号が「0dBm」である場合は、何らかの設定か、もしくはアンテナ設定が正しく行われていないことを示します。</li>
						<li>信号がある程度(少なくとも-70dBm以上)あれば問題ありません。Ping等を用いて、アクセスポイント間が接続されていることを確認してください。</li>
						<li>暗号化を行うことを強く推奨します。<a href="HWPA.asp">[無線LANセキュリティ]</a>ページにおいて、「WPA Pre-shared Key」と「AES」を設定するようにしてください</li>
					</ol><br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>WDSは、無線LAN MAC動作モードが「AP」に設定されている場合のみ使用可能です。また、無線LANセキュリティが「WPA2」の場合、無線LAN PHY動作モードが「B-Only」の場合は使用することができません。</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">基本(無線LAN)</a></li>
				<li><a href="HWPA.asp">無線LANセキュリティ</a></li>
				<li><a href="HStatusWireless.asp">無線LAN(機器情報)</a></li>
			</ul>
		</div>
	</body>
</html>