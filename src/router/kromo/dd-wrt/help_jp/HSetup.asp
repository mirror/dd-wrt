<% do_hpagehead("idx.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("bmenu.setupbasic"); %></h2>
			<dl>
				<dd>このページでは、Internetへ接続するための基本的な項目についての設定を行います。いくつかのISPでは、ユーザ名・パスワードなどの認証情報や、その他の設定情報が求められることがあります。設定方法が不明な場合は、ご利用のISPに問い合わせください。
<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>セキュリティを保つため、初めてこのページの設定を行った場合はそのあと、<a href="HManagement.asp">管理</a>ページでこの機器のパスワードを設定するようにしてください。</div>
					</div>
				</dd>
				
				<dt><% tran("share.hostname"); %></dt>
				<dd>一部のISPでのみ必要となります(通常は空欄で問題ありません)</dd>
				
				<dt><% tran("share.domainname"); %></dt>
				<dd>一部のISPでのみ必要となります(通常は空欄で問題ありません</dd>
				
				<dt><% tran("idx.legend"); %></dt>
				<dd>この機器では、インターネットへの接続方法を4種類の接続方法の中から選択することができます：
					<ul>
						<li>DHCP</li>
						<li>手動設定</li>
						<li>PPPoE (Point-to-Point Protocol over Ethernet)</li>
						<li>PPTP (Point-to-Point Tunneling Protocol)</li>
					</ul><br />
					これらの中から、ISPの指定した方法をドロップダウンメニューの中から選択します。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>いくつかのISPでは、インターネット接続を行うために、特定のMACアドレスである必要がある場合があります。その場合はこのページの設定が完了した後、 <a href="HWanMAC.asp">MACアドレス</a>設定を行う必要があります。</div>
					</div>
				</dd>
				
				<dt><% tran("idx_pptp.wan_ip"); %> / <% tran("share.subnet"); %></dt>
				<dd>ISPから配布されたInternet側のIPアドレスを設定します。</dd>
				
				<dt><% tran("share.gateway"); %></dt>
				<dd>ISPから配布されたゲートウェイアドレスを設定します。</dd>
				
				<dt><% tran("idx_static.dns"); %></dt>
				<dd>ISPから配布されたDNSアドレスの中から、一つ以上のアドレスを指定します。</dd>
				
				<dt><% tran("share.usrname"); %> / <% tran("share.passwd"); %></dt>
				<dd>PPPoEもしくはPPTPを指定した場合は、ISPから配布されたユーザ名とパスワードを入力します。</dd>
				
				<dt><% tran("share.compression"); %></dt>
				<dd>The PPP Compression provides a method to negotiate and utilize compression protocols over PPP encapsulated links. It's based on the MPPC protocol (Microsoft Point-to-Point Compression). It is a protocol designed for transfering compressed datagrams over point-to-point links.</dd>
				
				<dt><% tran("service.pptpd_encry"); %></dt>
				<dd>MPPE stands for Microsoft Point-to-Point Encryption. It is a protocol designed for transfering encrypted datagrams over point-to-point links.</dd>
				
				<dt><% tran("idx_h.reconnect"); %></dt>
				<dd>PPPoE利用時、切断・再接続を自動的に行う設定ができます</dd>
				
				<dt><% tran("idx_h.max_idle"); %></dt>
				<dd>一定時間アクセスが無かった場合に、自動的に切断を行うように設定することができます。オンデマンドに設定した場合は、Internet側へのアクセスを検出し、自動的に再接続を行うことができるようになります。</dd>
				
				<dt><% tran("idx_h.alive"); %></dt>
				<dd>キープアライプパケットを転送し続け、Internet側への経路が切断されないようにします。</dd>
				
				<dt><% tran("idx.mtu"); %></dt>
				<dd>MTU(Maximum Transmission Unit)は、Internet側へ転送するパケットの最大長を指定することができます。「Auto」設定時は、Internet側の接続方法から、自動的に最適なMTUを設定します。特定の数値に変更したい場合は「手動」を選択し、MTUを入力してください。MTUは1200～1500の範囲で指定することができます。</dd>
				
				<dt><% tran("idx.lanip"); %> / <% tran("share.subnet"); %></dt>
				<dd>LAN側ネットワークにこの製品が持つIPアドレスを入力します。IPアドレスの初期値は192.168.11.1、サブネットマスクは255.255.255.0に設定されています。</dd>
				
				<dt><% tran("idx.dhcp_srv"); %></dt>
				<dd>「有効」にすることにより、DHCPサーバ機能が動作します。既にDHCPサーバがネットワーク内に存在する場合や、DHCPサーバが必要内場合は「無効」に設定してください。</dd>
				
				<dt><% tran("idx.dhcp_start"); %></dt>
				<dd>DHCPサーバがIPアドレスを配布する範囲の始点を指定します。この製品のLAN側IPアドレス(初期値：192.168.11.1)を開始アドレスにはしないでください。</dd>
				
				<dt><% tran("idx.dhcp_maxusers"); %></dt>
				<dd>DHCPサーバがIPアドレスを配布する数を指定します。最大で253まで設定することができます</dd>
				
				<dt><% tran("idx.dhcp_lease"); %></dt>
				<dd>リース時間は、DHCPサーバが配布したIPアドレスの有効期限を示します。</dd>
				
				<dt><% tran("idx_static.dns"); %> 1-3</dt>
				<dd>Domain Name System (DNS)は、Internet上のホスト・ドメイン名をIPアドレスに関連づけるためのサーバです。ISPから通知されたDNSサーバが少なくとも1つ以上ある場合は、ここにDNSサーバのアドレスを入力してください。</dd>
				
				<dt>WINS</dt>
				<dd>WINSサーバがISPから通知されている場合は設定します。通常は空欄のままで問題ありません。</dd>
				
				<dt><% tran("idx.legend3"); %></dt>
				<dd>タイムゾーンを、必要に応じて設定します。また、サマータイム期間を設定します。サマータイムを利用しない地域の場合は「なし」を設定してください</dd>
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWanMAC.asp"><% tran("bmenu.setupmacclone"); %></a></li>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HStatus.asp"><% tran("bmenu.statuRouter"); %></a></li>
			</ul>
		</div>
	</body>
</html>
