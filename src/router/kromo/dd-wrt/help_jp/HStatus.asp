<% do_hpagehead("status_router.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>機器情報</h2>
			<dl>
				<dd>このページでは、製品の現在の状態と設定を表示します。</dd>
				<dt>ファームウェア Version </dt>
				<dd>現在動作しているファームウェアのバージョンを表示します。</dd>
				<dt>現在時刻</dt>
				<dd>現在の日付と時刻を表示します</dd>
				<dt>MACアドレス</dt>
				<dd>Internet側のMACアドレスを表示します</dd>
				<dt>機器名</dt>
				<dd>設定した機器の名称を表示します</dd>
				<dt>製品型番</dt>
				<dd>この製品のモデル名を表示します</dd>
				<dt>CPU</dt>
				<dd>この製品に搭載されているCPUを表示します</dd>
				<dt>CPU Clock</dt>
				<dd>CPUの動作クロックを表示します</dd>
				<dt>ホスト名</dt>
				<dd>この機器の名称を示します。一部のISPではホスト名を設定しなければならない場合があります。</dd>
				<dt>インターネット接続方法</dt>
				<dd>使用しているインターネット接続方法を表示します</dd>
				<dt>IPアドレス・サブネットマスク・ゲートウェイ</dt>
				<dd>ISPから取得したそれぞれの情報を表示します</dd>
				<dt>DNS</dt>
				<dd>使用するDNSアドレスを表示します</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HStatusLan.asp">LAN Status</a></li>
				<li><a href="HStatusWireless.asp">Wireless Status</a></li>
			</ul>
		</div>
	</body>
</html>