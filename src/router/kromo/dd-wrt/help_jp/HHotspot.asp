<% do_hpagehead("hotspot.titl"); %>
	<body class="help-bg">
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("hotspot.h2"); %></h2>
			<dl>
				<dd>この製品はHotSpotゲートウェイ(Chilispotサービスを利用)として、ユーザの認証・アカウンティングを行うことができます。ChilispotはOpensourceの無線LANアクセスポイントの制御サービスとして、また制限されたPortalとして動作します。このサービスを利用することにより、無線LANを利用するユーザに対し認証を行うことができます。この認証は一般的なHotSpotが持つようなWebベースのLogin機構を、また暗号化にはWPAを使用することができます。認証・承認・アカウンティング(AAA)は、ユーザが指定した任意のサーバで行うことができます。</dd>
				
				<dt><% tran("hotspot.pserver"); %></dt>
				<dd>各RADIUSサーバのIPアドレスを設定します</dd>
				
				<dt><% tran("hotspot.dns"); %></dt>
				<dd>DNSサーバのアドレスを指定します。このサーバはクライアントに対して名前解決を行わせる為に使用します。ここでDNSサーバが指定されなかった場合はプライマリDNSが使用されます。</dd>
				
				<dt><% tran("hotspot.url"); %></dt>
				<dd>認証サーバがアクセスを行う認証ページのURLを指定します</dd>
				
				<dt><% tran("share.share_key"); %></dt>
				<dd>各RADIUSサーバのShared-Secretを指定します。セキュリティのため、Shared-Secretは定期的に変更することをお勧めします。</dd>
				
				<dt><% tran("hotspot.dhcp"); %></dt>
				<dd>HotSpotサービスを提供するインターフェースを指定します。このパラメータは必ず指定されている必要があります。</dd>
				
				<dt><% tran("hotspot.radnas"); %></dt>
				<dd>NAS(Network Access Server)の識別子属性を設定します</dd>
				
				<dt><% tran("hotspot.uam"); %></dt>
				<dd>ClilispotサーバとのShared-Secretを指定します。このShared-Secretは定期的に変更することをお勧めします。</dd>
				
				<dt><% tran("hotspot.uamdns"); %></dt>
				<dd>全てのDNSサーバへの問い合わせを許可します。通常未認証のクライアントはDNS1,DNS2で指定されたDNSサーバにのみアクセスすることができますが、このOptionを設定することにより、全てのDNSサーバへの問い合わせができるようになります。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>セキュリティ確保の都合上、このOptionを有効にする場合は、DNS通信に対してDNATを併用することをお勧めします。</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.allowuam"); %></dt>
				<dd>クライアントが認証を行うためのIPセグメントを指定します(コンマで区切られたドメイン名のリストで指定します)。例：www.chillispot.org,10.11.12.0/24 </dd>
				
				<dt><% tran("hotspot.macauth"); %></dt>
				<dd>ChilliSpotへの認証をMACアドレスのみを用いて行います。</dd>
				
				<dt><% tran("hotspot.option"); %></dt>
				<dd>その他のOptionを記述することができます。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>その他のOptionについては、<a href="http://www.chillispot.org/chilli.html">www.chillispot.org</a>を参照ください。</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.nodog_legend"); %></dt>
				<dd>NoDogSplashは有線・無線LAN上のクライアントが接続された時に、特定のページにRedirectさせるためのサービスです。</dd>
				
				<dt><% tran("hotspot.nodog_gateway"); %></dt>
				<dd>ゲートウェイに対し、任意の名称をつけます。ここで指定された文字列は、Redirect先のHTML(splash.html)内の$GatewayNameという文字列と置換されます。例：「ジョーのインターネットピザ屋」</dd>
				
				<dt><% tran("hotspot.nodog_home"); %></dt>
				<dd>「有効」にすることによって、NoCatSplashの機能が有効になります。</dd>
				
				<dt><% tran("hotspot.nocat_allowweb"); %></dt>
				<dd>スペースで区切られたホスト名の集合を入力します。ここで列挙されたリストは「Login」していないクライアントがアクセス可能なホストの一覧を表示します。これらのホストはTCP/80(HTTP)もしくはTCP/443(HTTPS)のいずれかを使用している必要があります。</dd>
				
				<dt><% tran("hotspot.nodog_docroot"); %></dt>
				<dd>SplashFormを含む、Application Templateがおかれている場所を設定します(splash.htmlについては、userからはCaptureを行うことによって見ることができます</dd>
				
				<dt><% tran("hotspot.nocat_splash"); %></dt>
				<dd>Splash Pageを表示するOptionalなURLを設定します。このURLはサフィクスが/splash.htmlである必要があります。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>この機器自身がSplash用のHTMLを持つ場合は何も設定しなくても問題ありません</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.nodog_port"); %></dt>
				<dd>スペースで区切ったポート番号を指定します。LoginしたPublic classのユーザに対し、パケットを拒否するTCPポートを指定します。ここで指定されないポートは、原則通信許可されます(指定しない場合は、全てのPortを許可します)。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>スパムの踏台になることを避けるため、25番ポートだけは設定しないでください。</div>
					</div>
				</dd>
				
				<dt><% tran("hotspot.nodog_timeout"); %></dt>
				<dd>クライアントがLoginしてからの有効時間を指定します(秒)。有効時間が経過するとSplash画面が表示され、ユーザは再びEURAに対する同意を求められます。</dd>
				
				<dt><% tran("hotspot.nocat_verbose"); %></dt>
				<dd>Debug用のLogを出力します。Syslogサービスが動作している必要があります。
					<ul>
						<li>0：ほとんどLogを記録しません</li>
						<li>10：様々さLogを記録します</li>
						<li>5：中程度のLogを記録します</li>
					</ul>
				</dd>
				
				<dt><% tran("hotspot.nocat_route"); %></dt>
				<dd>特別な場合を除き、「無効」状態に設定してください。Internet側へNATを行う場合は設定を行う必要はありません。</dd>
				<dd>設定を保存するためには「保存」ボタンをクリックしてください。設定を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWPA.asp"><% tran("bmenu.wirelessSecurity"); %></a></li>
				<li><a href="HWireless.asp"><% tran("bmenu.wirelessBasic"); %></a></li>
				<li><a href="HServices.asp"><% tran("bmenu.servicesServices"); %></a></li>
			</ul>
		</div>
	</body>
</html>
