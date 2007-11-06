<% do_hpagehead("service.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>サーバ/サービス</h2>
			<dl>
				
				<dt><% tran("service.dhcp_legend2"); %></dt>
				<dd>DHCPdは、「基本」-「基本」ページで有効にされている間、LAN側の機器に対してIPアドレスの配布を行います。
					<ul class="wide">
						<li>ドメイン名の取得 &ndash; Internet側・LAN側のどちらからドメイン名を内側ネットワークの設定に使用するか指定することができます。</li>
						<li>ドメイン名を手動設定 &ndash; LAN側ネットワークのドメイン名を任意の名前に指定します。このドメイン名はDHCPサーバによってクライアントPCへ通知されます。</li>
						<li>静的割り当て &ndash; 特定のクライアントPCに特定のアドレスを与えたい場合、ここに登録を行います。</li>
					</ul><br/>
					この他のオプション値を設定する場合は、「DHCPサービス追加オプション」に追加することができます。
				</dd>
				
				<dt><% tran("service.dnsmasq_legend"); %></dt>
				<dd>DNSmasqは、DNSサーバとして動作します。DNSMasqは外部のDNSへの要求を行う他、「ローカルDNS機能」を有効にする個とによって、DHCPによって配布したホスト名を他のホストにも配布することもできます。<br/>

				次のような追加オプションを「DNSMasqサービス 追加オプション」に記述することもできます(設定例) : <br/>
					<ul>
						<li>静的割り当て：dhcp-host=AB:CD:EF:11:22:33,192.168.0.10,myhost,myhost.domain,12h</li>
						<li>最大割り当て台数： dhcp-lease-max=2</li>
						<li>割り当てIPアドレス範囲： dhcp-range=192.168.0.110,192.168.0.111,12h</li>
					</ul>
				<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>これらのオプションは/tmp/dnsmasq.confに記録されます。また、オプションは複数のコマンドを同一行に書いてはいけません。<br/>使用可能なオプションは次のページを参照ください：<a href="http://thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html" target="_new">DNSMasq man</a>.</div>
					</div>
				</dd>
				
				<dt><% tran("service.kaid_legend"); %></dt>
				<dd>Xlink Kaiは、Internetを利用して通信ゲームを行うためのフレームワークです。このサービスを利用するためには、Kaidサービスを有効にし、XBoxのMACアドレスを登録します(複数のアドレスを入力する場合は";"で区切ります)
					<ul class="wide">
						<li><% tran("service.kaid_locdevnum"); %> &ndash; 何台のゲーム機を利用するか指定します。0を指定した場合は、上限を指定しないものとして扱われます。</li>
						<li><% tran("service.kaid_uibind"); %> &ndash; Controller UIを受け付けるポートを指定します。</li>
					</ul><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>Xboxは、本機のEthernetポートに接続されている必要があります。</div>
					</div>
				</dd>
<!--				
<< PPTP is already moved to other page:
				<dt><% tran("service.pptp_legend"); %></dt>
				<dd>PPTPはWindowsXPをはじめ、Microsoftや他のネットワークベンダで利用されているVPN技術です。PPTPを利用することによって、遠隔地からこの製品やLAN側のネットワークにアクセスすることができます。
					<ul class="wide">
						<li>サーバ側IPアドレス &ndash; サーバに割り当てるIPアドレスを設定します。</li>
						<li>クライアントIPアドレス &ndash; ピア・クライアントに対して割り当てるIPアドレスの範囲を指定します。この設定範囲はDHCPが配布するIPアドレスの範囲と重なってはいけません。A range of IP addresses for remotely connected machines. This range should not overlap with the DHCP range (例: 192.168.0.210-220).</li>
						<li>CHAP-Secrets &ndash; PPTPを利用する場合のユーザ名とパスワードを指定します(例： joe * joespassword *)。詳細については、pppdのmanを参照ください。</li>
					</ul>
				</dd>
				
				<dt><% tran("service.pptpd_legend"); %></dt>
				<dd>PPTPはWindowsXPをはじめ、Microsoftや他のネットワークベンダで利用されているVPN技術です。PPTPを利用することによって、遠隔地からこの製品やLAN側のネットワークにアクセスすることができます。
					<ul class="wide">
						<li>サーバ側IPアドレス・ホスト名 &ndash; VPNサーバのIPアドレスもしくはホスト名を設定します。(例: www.MyServer.com). </li>
						<li>サーバ側ネットワーク &ndash; 接続するサーバ内のネットワークを指定します。(例： 192.168.2.0). </li>
						<li>サーバ側サブネットマスク &ndash; サーバ側ネットワークのサブネットマスクを入力します。(例： 255.255.255.0). </li>
						<li>MPPE暗号化 &ndash; 接続時に用いるセキュリティの種類を指定します。(例： mppe required,no40,no56,stateless)</li>
						<li>MTU &ndash; Maximum Transmission Unitの初期値を指定します (デフォルト: 1450) </li>
						<li>MRU &ndash; Maximum Receiving Unitの初期値を指定します (デフォルト： 1450) </li>
						<li>ユーザ名 &ndash; PPTPサーバへ接続するためのユーザ名を指定します。(例: "root", "DOMAIN\\UserName"</li>
						<li>パスワード &ndash; PPTPサーバへ接続するためのパスワードを指定します。</li>
					</ul>
				</dd>
>> PPTP is already moved to other page:
-->
				<dt><% tran("service.rflow_legend"); %></dt>
				<dd>RFlow Collectorはこの製品のネットワーク利用状況を監視するための管理ツールです。<br/>
					<ul class="wide">
						<li>RFlowは2055番ポートを使用します</li>
						<li>MACupdは2056番ポートを使用します</li>
						<li>間隔は、サーバに情報を送信する間隔を指定します。</li>
						<li>インターフェース： どのインターフェースを監視するか指定します。</li>
					</ul><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>それぞれのサーバIPアドレスについては、RFlow Collectorが動作するPCのアドレスを指定してください。</div>
					</div>
				</dd>
				
				<dt><% tran("service.rstats_legend"); %></dt>
				<dd>rstatsは、Jonathan Zarateによって開発された帯域監視ツールです。この機能を利用するためにはAdobeの<a href="http://www.adobe.com/svg/viewer/install/main.html" title="Adobe SVG Viewer download area" target="_blank">SVG plugin</a>が必要です。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>データの保存にNVRAM, JFFS2の両方を指定した場合、保存間隔は2日毎に制限されます。</div>
					</div>
				</dd> 
				
				<dt><% tran("service.ssh_legend"); %></dt>
				<dd>SSHを利用するための設定を行うことができます。
					<ul class="wide">
						<li>Password login &ndash; SSHのLoginにWebUIのユーザ名・パスワードを利用します</li>
						<li>SSHd ポート &ndash; SSHdが使用するポートを指定します。(デフォルト：22)</li>
						<li>Authorized Keys &ndash; Authorized Keyを使用して認証を行う場合に、Keyをここにペーストします</li>
					</ul>
				</dd>
				
				<dt><% tran("service.syslog_legend"); %></dt>
				<dd>この製品のLogを他のサーバなどに転送します。工場出荷値では、これらのLogは/var/log/messagesに保存されます。利用する場合は転送先のサーバのIPアドレスを入力する必要があります。</dd>
				
				<dt><% tran("service.telnet_legend"); %></dt>
				<dd>Telnet経由でこの製品にアクセスすることができます。ユーザ名はroot, パスワードはWebUIのパスワードを利用します。<br/><br/>
					<div class="note">
						<h4>メモ</h4>
						<div>信頼できないネットワーク環境で利用する場合は、Telnetを無効にし、代わりにSSHを利用するようにしてください。</div>
					</div>
				</dd>
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HSetup.asp"><% tran("bmenu.setupbasic"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
			</ul>
		</div>
	</body>
</html>
