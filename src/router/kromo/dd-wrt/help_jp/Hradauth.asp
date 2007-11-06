<% do_hpagehead("radius.legend"); %>
	<body>
	<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("radius.h2"); %></h2>
			<dl>
				<dd>RADIUS (Remote Authentication Dial-In User Service)は、ネットワーク上の機器に対して認証・承認を行うためのプロトコルです。一般的な企業ネットワークには大抵ダイヤルアップ等の用途向けにRAIDUSサーバを持ち、ユーザ認証を行います。この製品においても、無線LANに接続を行うユーザに対し、認証を行うことができます。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>RADIUS機能はAPモード設定時のみ利用できます。</div>

					</div>
				</dd>
				
				<dt><% tran("radius.label2"); %></dt>
				<dd>RADIUSサーバに認証要求を行ったとき、無線LANクライアントのMACアドレスを用いて認証要求を行います。RADIUSサーバのユーザ名は次のフォーマットのいずれかである必要があります。
					<ul class="wide">
						<li>aabbcc-ddeeff</li>
						<li>aabbccddeeff</li>
						<li>aa-bb-cc-dd-ee-ff</li>
					</ul>
				</dd>				
				
				<dt><% tran("radius.label3"); %> - <% tran("radius.label4"); %></dt>
				<dd>RAIDUSサーバのIPアドレスとUDPポートを設定します。</dd>
				
				<dt><% tran("radius.label5"); %></dt>
				<dd>認証無しでアクセス可能なユーザの数を設定します</dd>
				
				<dt><% tran("radius.label6"); %></dt>
				<dd>RADIUSサーバに認証を行う場合のパスワードを指定します。Shared Secretか、ユーザ名と同じMACアドレスフォーマットを用いることもできます。</dd>
				
				<dt><% tran("radius.label7"); %></dt>
				<dd>RADIUSサーバとの通信に使用される共有キーを設定します。RADIUSサーバと同様の共有キーを設定する必要があります。</dd>
				
				<dt><% tran("radius.label8"); %></dt>
				<dd>RADIUSサーバと通信できなくなった時に、通信できるようになるまで認証を停止します。この設定はそういった場合に、通信が可能とすることができます</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWireless.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
				<li><a href="HWPA.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
				<li><a href="HWirelessAdvanced.asp"><% tran("bmenu.wirelessAdvanced"); %></a></li>
			</ul>
		</div>
	</body>
</html>