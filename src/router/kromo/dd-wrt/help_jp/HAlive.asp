<% do_hpagehead("upgrad.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>キープアライブ</h2>
			<dl>
				<dt><% tran("alive.legend"); %></dt>
				<dd>定期的に再起動を行うように設定することができます:
					<ul>
						<li>通常は「間隔(秒)」を指定します</li>
						<li>毎日決まった時間に再起動を行う場合は、「指定時刻に再起動を行う」を選択します。</li>
					</ul><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>「指定時刻に再起動を行う」を選択する場合は、Cronを有効にしておく必要があります。詳しくは<a href="HManagement.asp">管理</a>を参照ください</div>
					</div>
				</dd>
				<dt><% tran("alive.legend2"); %></dt>
				<dd></dd>
				<dd><em>設定(保存のみ)</em>をクリックすることにより、設定値が保存されます。設定をキャンセルする場合は<em>キャンセル</em>をクリックしてください。<em>再起動</em>をクリックすると、アクセスポイントは再起動します。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWDS.asp"><% tran("bmenu.wirelessWds"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>