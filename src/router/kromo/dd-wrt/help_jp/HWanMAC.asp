<% do_hpagehead("wanmac.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("wanmac.h2"); %></h2>
			<dl>
				<dd>MACアドレスは12桁の16進数で、ネットワーク機器を唯一に示すものです。ISPの中にはユーザが接続する機器のMACアドレスを登録することを要求するところもあります。いままで他の機器をDSLやモデムに接続していた場合は、ISPに登録しなおすか、このページで本機が登録済MACアドレスを使用できるように設定を行う必要があります。</dd>
				
				<dt><% tran("wanmac.legend"); %></dt>
					<dd>MACアドレスを手動設定するためには、「MACアドレス設定」において「MACアドレス」を有効にし、インターネット側のMACアドレスを変更する必要があります。設定したあと、「設定」ボタンをクリックします。
					<br/><br/>
					MACアドレスを本来のものに戻すためには、「無効」を選択します。
					</dd>
					<dd>使用しているクライアントPCのMACアドレスを取得するためには、「現在接続中の機器からMACアドレスを取得する」をクリックします。
					</dd>
					<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HStatus.asp"><% tran("bmenu.statuRouter"); %></a></li>
			</ul>
		</div>
	</body>
</html>
