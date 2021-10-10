<% do_hpagehead("pforward.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("pforward.h2"); %></h2>
			<dl>
				<dd>ポート変換は、WWW,FTP,SMTPなどの公開サービスをこの機器のInternetポートを通して外部に提供するための機能です。このほか、特別な設定が必要なアプリケーション(ビデオ会議・ゲームなど)についても、ポート変換を行うことによって使用することができるようになります。PCから設定されたポートを用いてInternet側へ通信を行うことによって、そのPCへ返答パケットを転送できるようになります。<br /><br />
					複数のポート(範囲)を指定する場合は、<a href="HForward.asp">アドレス変換(Port範囲)</a>を参照してください<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>ポート変換を行うPCは、DHCPから取得したIPアドレスではなく、固定IPアドレスを使用するようにしてください</div>
					</div>
				</dd>

				<dd>新しいルールを追加する場合は、「追加」ボタンをクリックし、必要な情報を入力してください。ルールを削除する場合は「削除」をクリックしてください。</dd>

				
				<dt><% tran("pforward.app"); %></dt>
				<dd>ルールに任意の名前をつけます</dd>
				
				<dt><% tran("pforward.from"); %></dt>
				<dd>Enter the number of the external port (the port number seen by users on the Internet).</dd>
				<dd>変換対象とするポート番号(Internet上から見たポート番号)を指定します。</dd>
				
				<dt><% tran("share.proto"); %></dt>
				<dd>「TCP」「UDP」「両方」から選択することができます。</dd>
				
				<dt><% tran("share.ip"); %></dt>
				<dd>変換対象とするLAN側PC(アプリケーションを動作させるPC)のIPアドレスを指定します。</dd>

				<dt><% tran("pforward.to"); %></dt>
				<dd>変換対象とするポート番号(LAN側からみたポート番号を指定します。</dd>
				
				<dt><% tran("share.enable"); %></dt>
				<dd>「有効」チェックを入力することにより、実際にアドレス変換が行われるようになります。</dd>

				<dd>全ての設定を保存するためには、「設定」ボタンをクリックしてください。設定した内容を破棄するためには「キャンセル」をクリックしてください。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>ルールを追加・設定・削除しても、「設定」ボタンなどをクリックしないと保存されません。</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HTrigger.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
				<li><a href="HUPnP.asp"><% tran("bmenu.applicationsUpnp"); %></a></li>
				<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
			</ul>
		</div>
	</body>
</html>
