<% do_hpagehead("service.pptp_legend"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("service.pptp_legend"); %></h2>
			<dl>
				<dd>PPTP Server設定</dd>

				
				<dt><% tran("share.srvip"); %><dt>
				<dd>PPTPサーバのLocal IPアドレスを指定します。大抵の場合はLAN側IPアドレスを指定します。<dd>
				
				<dt><% tran("service.pptp_client"); %><dt>
				<dd>クライアントに通知するIPアドレスを指定します。コンマで区切ったアドレスのリストか、範囲指定IPアドレスのいずれかのフォーマットで入力を行います。<dd>
				<div class="note">
					<h4>メモ</h4><br />
					<div>IPアドレスの設定は、次のように行います。(リストの例： 192.168.0.2,192.168.0.3),(範囲の例： 192.168.0.1-254 or 192.168.0-255.2) (双方を用いた例： 192.168.0.2,192.168.0.5-8)</div>
				</div>
				
				<dt><% tran("service.pptp_chap"); %><dt>
				<dd>CHAP Secretを入力します。ここで入力したユーザ名・パスワードはPPTP接続時のアカウントとして利用されます<dd>
				<div class="note">
					<h4>メモ</h4><br />
					<div>フォーマットは次のようにしてください。ユーザ名・パスワード・アスタリスクの間には必ずスペースが挿入されている必要があります。<br>myuser * mypassword *</div>
				</div>			
				
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
			
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
	</body>
</html>
