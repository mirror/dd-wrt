<% do_hpagehead("ddns.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("ddns.legend"); %></h2>
			<dl>
				<dd>この製品にはダイナミックDNS(DDNS)機能が搭載されています。DDNSを利用することにより、ISPなどから固有のIPアドレスを配布されなくても、常に同じホスト・ドメイン名でインターネット越しに接続することができます。この製品のLAN側ネットワークからWeb, FTPサーバなどを公開する場合に使用します。DDNSを使用する為には、DDNSサービスを提供する業者(プロバイダ)と契約を行う必要があります。<br>
DDNSサービスには次のようなものがあります：<br>
<ul>
 <li> <a href="http://www.dyndns.org" target="_new">www.dyndns.org</a>
</ul>
</dd>

				<dt><% tran("ddns.srv"); %></dt>
				<dd>DDNSサービスを停止するためには、「無効」に設定してください。有効にするためには次のように設定してください：
					<ol class="wide">
						<li>次のページにアクセスを行い、DDNSサービスに登録を行います：<a href="http://www.dyndns.org" target="_new">www.dyndns.org</a><br>このときのユーザ名・パスワード・ホスト名を記録しておきます。</li>
						<li>[基本]-[DDNS]設定ページにおいて、「DynDNS」を選択します。</li>
						<li>ユーザ名・パスワード・ホスト名を入力します。</li>
						<li>「設定」ボタンをクリックし、設定を反映します。設定を止める場合は「キャンセル」をクリックしてください。</li>
					</ol><br />
					以上の手順で、登録したドメイン名で本製品にアクセスすることができます。
				</dd>
				
				<dt><% tran("ddns.typ"); %></dt>
				<dd>「固定」サービスと「動的」サービスは、Internet側に割り当てられたIPアドレスを単一のドメイン名と対応させます。固定サービスは最大35日まで更新せずにIPアドレスを保持できますが、IPアドレスを変更した場合のDNSシステムの情報更新には時間がかかります。<br>
				「手動設定」DDNSサービスは、一般的なプライマリDNSのように動作します。登録したドメイン内のサブドメインやホストを事由に定義することができます。提供サービスの設定ページでは、これらのレベルを変更できます。本製品のサービスタイプは設定ページで指定したものと同一である必要があります。</dd>


				<dt><% tran("ddns.wildcard"); %></dt>
				<dd>このワイルドカードとは、サブドメイン・ホスト名を任意の文字列として指定した者です。通常「*」で示され、「*.yourhost.dyndns.org」のように表記されます。</dd>

				<dt><% tran("ddns.forceupd"); %></dt>
				<dd>定期的にDDNSサービスにこの機器のInternet側IPアドレスを通知する間隔を日数で指定します。この機能は通常必要ありませんが、「non donator」のdyndns.orgユーザの場合は、ホスト名が削除されるのを回避することができます。</dd>
				
				<dt><% tran("ddns.statu"); %></dt>
				<dd>DDNSサービスへの接続状況を表示します。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HManagement.asp"><% tran("bmenu.adminManagement"); %></a></li>
				<li><a href="HStatus.asp"><% tran("bmenu.statuRouter"); %></a></li>
			</ul>
		</div>
	</body>
</html>