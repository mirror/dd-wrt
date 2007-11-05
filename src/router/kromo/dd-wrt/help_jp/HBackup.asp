<% do_hpagehead("config.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("config.h2"); %> / <% tran("config.h22"); %></h2>
			<dl>
				<dd>設定保存は、機器に設定した内容をブラウザ参照元のPCのファイルとして保存することができる機能です。設定を初期化する必要がある場合など、元の情報を復元する必要がある場合などに使用します。</dd>

				<dt><% tran("config.legend"); %></dt>
				<dd>「設定の保存」をクリックすることにより、PC内のファイルに設定内容を保存することができます。</dd>
        <dt><% tran("config.legend2"); %></dt>
        <dd>復元を行うには、「Browse」ボタンをクリックし、PC内の設定保存時に作成されたファイルを指定します。そのあと、「設定の復元」をクリックすることにより、保存した設定の復元を行うことができます。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>復元可能な設定ファイルは、同一製品・型番のものに限られます。</dd>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HDefault.asp"><% tran("bmenu.adminFactory"); %></a></li>
			</ul>
		</div>
	</body>
</html>
