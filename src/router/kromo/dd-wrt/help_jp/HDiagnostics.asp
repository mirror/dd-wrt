<% do_hpagehead("diag.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("diag.h2"); %></h2>
			<dl>
				<dd>このページからコマンドライン命令を実行することができます。</dd>
				<dt><% tran("diag.legend"); %></dt>
				<dd>Textareaに実行するコマンド文を入力し、「実行する」ボタンをクリックすることによって、入力コマンドの実行を行うことができます</dd>
				<dt><% tran("diag.startup"); %></dt>
				<dd>入力したコマンドを起動時に実行するスクリプトとして保存することもできます。コマンドを入力し(1行につき1つの実行文のみ入力することができます)、「起動時スクリプトとして保存」ボタンをクリックすることによって、起動時実行スクリプトとして保存することができます。</dd>
				<dt><% tran("diag.firewall"); %></dt>
				<dd>Firewallが実行されたときの追加テーブルを定義することができます。コマンドを入力し(1行につき1つの実行文のみ入力することができます)、「Firewallスクリプトとして保存」ボタンをクリックすることによって、Firewallの追加スクリプトとして保存する個とができます。<br/><br/>
				<dt><% tran("diag.custom"); %></dt>
				<dd>手動スクリプトとして、/tmp/custom.shに保存されるコマンドを登録することができます。このスクリプトは手動もしくはCronに指定することにより実行することができます。登録するためにはコマンドを入力し(1行につき1つの実行文のみ入力することができます)、、「手動スクリプトとして保存」ボタンをクリックすることによって保存を行うことができます。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>
							<ul class="wide">
								<li>起動時スクリプトはNVRAM変数「rc_startup」に保存されます。</li>
								<li>FirewallスクリプトはんNVRAM変数「rc_firewall」に保存されます。</li>
								<li>手動実行スクリプトはNVRAM変数「rc_custom」に保存されます。</li>
							</ul>
						</div>
					</div>
				</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
