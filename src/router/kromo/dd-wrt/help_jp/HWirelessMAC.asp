<% do_hpagehead("wl_mac.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>MACアドレスフィルタ</h2>
			<dl>
				<dd>MACアドレスフィルタは、予め許可を与えたMACアドレスを持つクライアントのみが接続・通信を行うことができるようにする機能です。Windowsにおける無線LANクライアントのMACアドレスについては、<a href="HWanMAC.asp">こちら</a>を参照ください。</dd>
				<dd>MACアドレスフィルタ機能を使用するためには、「MACアドレスフィルタ」を「有効」に設定し、次の設定を行います：
					<ol class="wide">
						<li>特定の無線LANクライアントを排除したい場合は、「リストに登録されているクライアントの通信を拒否する」を選択します。特定の無線LANクライアントのみ通信をさせたい場合は、「リストに登録されているクライアントの通信を許可する」を選択します。</li>
						<li>「MACアドレスの編集」をクリックし、拒否(許可)したいクライアントのMACアドレスを登録します。<br /><br />
							<div class="note">
								<b>メモ</b><br />
								MACアドレスは、次のようなフォーマットで入力する必要があります："001601xxxxxx" (MACアドレスが00:16:01:xx:xx:xxの場合)
							</div>
						</li>
						<li>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</li>
					</ol><br />
					MACアドレスフィルタを使用しない場合は、MACアドレスフィルタを「無効」に設定してください。
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp">基本(無線LAN)</a></li>
				<li><a href="HWPA.asp">無線LANセキュリティ</a></li>
			</ul>
		</div>
	</body>
</html>