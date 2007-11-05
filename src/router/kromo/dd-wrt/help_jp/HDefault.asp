<% do_hpagehead("factdef.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("factdef.h2"); %></h2>
			<dl>
				<dd>設定初期化は機器の動作が不安定になった場合、設定内容を忘れた場合、予め保存しておいた設定内容を復元する場合などに使用します。</dd>
				<dt><% tran("factdef.legend"); %></dt>
				<dd>「工場出荷時に設定パラメータを戻す」を「はい」に設定し、「設定(保存のみ)」をクリックすることによって、全ての設定パラメータを工場出荷時の状態に戻すことができます。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>設定初期化を実行すると、これまで保存していたすべての情報を工場出荷状態に戻します。初期化を行った場合IPアドレスやパスワードも初期値に戻りますので、初期値IPアドレス(192.168.11.1)および、初期パスワード(root/admnin)を用いて再設定を行う必要があります。</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HBackup.asp"><% tran("bmenu.adminBackup"); %></a></li>
			</ul>
		</div>
	</body>
</html>
