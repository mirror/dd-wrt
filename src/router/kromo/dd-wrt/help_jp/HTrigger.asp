<% do_hpagehead("trforward.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>アドレス変換(Trigger指定)</h2>
			<dl>
				<dd>Triggerパケットを指定することにより動作するアドレス変換ルールを設定できます。Triggerパケットを指定することにより、LAN側IPアドレスを指定することなく送信元アドレスへ返答パケットが届くようにアドレス変換を行うことができます。<br /><br />
					その他のアドレス変換設定については、<a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a>もしくは<a href="Forward_range.asp">アドレス変換(Port範囲)</a>を参照ください</dd>
				<dd>新たにルールを登録するときは、「追加」をクリックして必要な項目を設定してください。ルールを削除する場合は「削除」ボタンをクリックしてください。</dd>
				<dt>アプリケーション名</dt>
				<dd>ルールに任意の名前をつけます</dd>
				<dt>Triggerポート</dt>
				<dd>Triggerに設定するポートの範囲を指定します。クライアントPCがこのポートへパケットを送信した時に、対応する逆方向の「転送ポート」からのパケットを受け付けるようになります</dd>
				<dt>転送ポート</dt>
				<dd>Triggerポートに対応させるポート範囲を指定します。</dd>
				<dt>有効</td>
				<dd>「有効」をチェックすることにより、ルールが有効になります</dd>
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>ルールの追加・削除を行った場合は、必ず設定を反映するために「設定」ボタンをクリックしてください</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HForward.asp"><% tran("bmenu.applicationsprforwarding"); %></a></li>
				<li><a href="HDMZ.asp"><% tran("bmenu.applicationsDMZ"); %></a></li>
			</ul>
		</div>
	</body>
</html>
