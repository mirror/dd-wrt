<% do_hpagehead("management.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("management.h2"); %></h2>
			<dl>
				<dd>この画面では機器に関する基本的な機能の設定を行うことができます。</dd>
				
				<dt><% tran("management.psswd_pass"); %></dt>
				<dd>パスワードを入力します。パスワードは32文字を越えない範囲で入力する必要があります。スペースをパスワードの一部賭することはできません。入力値を確認するため、パスワードは2回入力を行う必要があります。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>ユーザ名の初期値は「root」です。<br/>
						パスワードは初期値のままで使用せず、必ず変更を行うことを強くお勧めします。パスワードの変更はこのWeb画面やSetup Wizardで行うことができます。</div>
					</div>
				</dd>
				
				<dt><% tran("management.remote_legend"); %></dt>
				<dd>この設定を「有効」にすることにより、Internet上など、離れた場所からInternet側ポートを経由して設定インターフェースにアクセスすることができるようになります。初期値は「無効」に設定されています。「有効」にした場合は、Webインターフェースにアクセスするためのポートを指定する必要があります。この機能を利用する前に、必ずパスワードを初期値から変更することを強くお勧めします。<br /><br />
					遠隔地から設定インターフェースにアクセスする場合は、ブラウザに<tt>http://xxx.xxx.xxx.xxx:8080</tt> (「xxx...」は機器のInternet側アドレスを示し、「8080」は設定したポートを示します)と入力する必要があります。LAN側からのアクセスと同様、パスワードを入力することによって設定画面にアクセスすることができるようになります。<br /><br />

					遠隔地からの設定インターフェースとして、SSHを使用することもできます。SSHサービスは<a href="HServices.asp"><% tran("bmenu.servicesServices"); %></a>ページで設定を行う必要があります。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>この機能を有効にすると、機器の管理インターフェースが公開され、その存在が外部に知られる可能性があります。</div>
					</div>
				</dd>
				
				<!--<dt>AP Watchdog</dt>
				<dd>AP Watchdogは、クライアントPCが接続可能な時間を管理することができます。接続クライアントが存在しない場合、Watchdogはresetすべきと判断します。これによってクライアントPCは影響を受けることはありません。WatchdogはインターフェースやChip自体の異常のために、アクセスポイント機能を停止します。</dd>-->
				
				<dt><% tran("management.web_legend"); %></dt>
				<dd>WebインターフェースにアクセスするためのプロトコルをHTTPとするか、HTTPSとするか指定することができます。どちらも使用しない設定にした場合は、設定後に一旦機器を再起動する必要があります。<br/>
				また、システム情報ページを表示するかどうかを指定することもできます。「システム情報表示画面でも認証を行う」チェックすることによって、システム情報ページでも認証を要求するようになります(管理ユーザのユーザ名・パスワードを使用するようになります)。<br/>
				「システム情報でMACアドレスを非表示にする」は、システム情報ページにおけるMACアドレス表示をマスクすることができるようになります。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>「システム情報でMACアドレスを非表示にする」を有効にした場合、MACアドレスの表示は次のようになります：「xx:xx:xx:xx:AA:BB」</div>
					</div>
				</dd>
				
				<dt><% tran("management.boot_legend"); %></dt>
				<dd>「遅延Boot」は、通常であれば変更する必要はありません。この機能を「有効」にすることによって、起動時に5秒程度の遅延が挿入されます。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>この機能は常に有効にしておいてください。「無効」に設定した場合、Flashの復旧ができなくなる可能性があります。</div>
					</div>
				</dd>
				
				<dt><% tran("management.cron_legend"); %></dt>
				<dd>Cronサービス設定を行います。Cronサービスを使用すると、定められた時間に特定の処理を自動的に行わせることができます。実際の動作については、「起動時スクリプト」か、コマンドラインにおける設定が必要になります</dd>
				
				<dt><% tran("management.loop_legend"); %></dt>
				<dd>Loopbackインターフェースの有無を変更することができます。Loopbackインターフェースは外部ネットワークとの通信と同様の手法で機器内のサービス間において通信を行うことができるようになります。この機能はDynDNSサービスの動作試験を行う場合などに利用することができます。Lookbackインターフェースを無効にすると、PPTPや無線デバイスに接続中のWindowsクライアントに影響を及ぼすことがあります。</dd>
				
				<dt><% tran("management.wifi_legend"); %></dt>
				<dd>通常は「有効」のまま利用ください。WindowsXPにおいてWPA認証を行う場合、WPA Handshakeを成功させるために必要になる場合があります。</dd>
				
				<dt><% tran("management.rst_legend"); %></dt>
				<dd>INITスイッチの動作を変更することができます。「無効」にすることによって、リセット動作が行われなくなります。INITスイッチは押している時間によって次のように動作が変わります。
					<ul>
						<li>短時間 &ndash; 再起動を行います</li>
						<li>長時間(&gt;5秒) &ndash; 設定を工場出荷時に戻し、再起動を行います</li>
					</ul>
				</dd>
				
				<dt><% tran("management.routing_legend"); %></dt>
				<dd>OSPF,RIPなどを利用している場合は、この機能を有効にしておく必要があります。</dd>
				
				<dt><% tran("management.net_legend"); %></dt>
				<dd>P2Pアプリケーションなどを利用している場合、最大割り当てポート数の増加や、TCP/UDPタイムアウトを減らすことによってアプリケーションの動作が安定することがあります。一般にP2Pアプリケーションは多くのPortを利用し、またプロトコルが不完全な場合があるため、これらの値はできるだけ余裕を持った数値を設定する必要があります。
					<ul>
						<li>Maximum Ports: 4096エントリ</li>
						<li>TCP Timeout: 120 秒</li>
						<li>UDP Timeout: 120 秒</li>
					</ul>
				</dd>
				<dd>全ての設定値を保存・反映するためには、「設定」をクリックします。設定した内容を破棄するためには、「キャンセル」をクリックします。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HServices.asp"><% tran("bmenu.servicesServices"); %></a></li>
			</ul>
		</div>
	</body>
</html>
