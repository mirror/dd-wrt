<% do_hpagehead("wpa.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>無線LANセキュリティ</h2>
			<dl>
				<dd>この機器はWPA Personal, WPA RADIUS, RADIUS, WEPなど、多くの無線LAN認証方式の中からを選択することができます。認証・暗号化を行わない場合は、「無効」を選択します。</dd>


				<dt>WPA Personal</dt>
				<dd>WPA(Wi-Fi Protected Access)に準拠した無線LAN機器の認証をおこないます。エアステーションに設定した共有キー(Pre Shared Key)と同じ共有キーに設定された無線LANパソコンのみが通信できます。また、暗号化方式を「TKIP」「AES」の中から選択することができます。TKIPは、WEP方式をベースに、耐セキュリティ性能を強化した暗号通信方式です。AESは、暗号化に強力なアルゴリズムを利用した暗号通信方式です。<br /><br />
					WPA Personalを選択する場合は、<i>WPA共有キー</i>を入力する必要があります。共有キーは8〜63文字の任意の記号(&#035;, &#058;などを除く)を設定する必要があります。Key更新間隔は、0〜99,999(秒)の範囲内で設定してください。</dd>
				<dt>WPA Enterprise</dt>
				<dd>WPA Enterpriseは、ユーザ認証にRADIUSサーバを用いた無線LAN認証です。「RADIUS認証サーバ」にはサーバのIPアドレスを、「RADIUS Shared secret」には、サーバに設定するものと同じ共有鍵を設定する必要があります。「RADIUS認証サーバポート」については、ほとんどの場合初期値(1812)で問題ありません。</dd>
				<dt>WPA2 Only</dt>
				<dd>WPA2は、WPA準拠の認証方式に加え、IEEE802.11iに基づく追加オプションを加えた者です。この方式を使用するためには、WPA2認証方式に対応したクライアントが必要です。但し、WPA2とTKIPの組み合わせはサポートされていません。また、WDSと同時に使用することはできません。</dd>
				<dt>WPA2 Mixed</dt>
				<dd>WPA2対応のクライアント、WPA対応のクライアントを同時に運用することができます。「WPAアルゴリズム」に「TKIP+AES」を選択することによって、より多くのWPA/WPA2クライアントを同時にサポートすることができます。</dd>
				<dt>RADIUS</dt>
				<dd>この認証方式はユーザ認証にIEEE802.1X/EAPを、暗号化にはWEPを利用する認証方式です。この認証方式を使用するためには、RADIUS認証サーバのIPアドレスとUDPポート、Shared Secretの入力が必要となります。</dd>
				<dt>WEP</dt>


				<dd>WEPを使用する場合は、Key長を「64bits」「128bits」のなかから設定する必要があります。「暗号化」の中からKey長を選択し、4つのスロット(キー1〜キー4)に対し、ASCII文字列もしくは16進数で入力する必要があります。16進数は0〜9の数字とA〜F(もしくはa〜f)いずれかの文字を使用することができます。<br /><br />
					WEPキーはセキュリティ上の理由からあまり推奨されません。可能であればWPAを使用することを推奨します。</dd>

				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>

			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">基本(無線LAN)</a></li>
				<li><a href="Hradauth.asp">RADIUS</a></li>
				<li><a href="HWirelessMAC.asp">MACアドレスフィルタ</a></li>
				<li><a href="HWirelessAdvanced.asp">詳細(無線LAN)</a></li>
			</ul>
		</div>
	</body>
</html>
