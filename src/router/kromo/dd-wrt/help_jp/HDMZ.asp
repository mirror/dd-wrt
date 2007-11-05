<% do_hpagehead("dmz.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("dmz.h2"); %></h2>
			<dl>


				<dd>DMZ(DeMiliterized Zone)は、ゲームやビデオ会議システムなど、特定のサービスをインターネット上で用いる必要がある場合に使用します。DMZを有効にすると、宛先不明のInternet側からのパケットを全てDMZホストへ転送するようになります。PCを不用意にInternet側へ公開したくない場合は、DMZの代わりにPortFowardingを使用することをお勧めします。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>DHZホストを指定する場合は、PCのIPアドレスをDHCPではなく固定値にしておくことをお勧めします</div>
					</div>
				</dd>

				<dt><% tran("dmz.host"); %></dt>
				<dd>PCを外部に公開する為には、DMZアドレス設定を「有効」にし、DMZホストのIPアドレスに、公開するPCのIPアドレスを入力する必要があります。<br /><br />
					DMZを使用しない場合は、「無効」のままにしておいてください。</dd>
				<dd>「設定」をクリックすることで設定値を保存することができます。設定した内容をキャンセルする場合は「キャンセル」をクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HFirewall.asp"><% tran("bmenu.firwall"); %></a></li>
				<li><a href="HForwardSpec.asp"><% tran("bmenu.applicationspforwarding"); %></a></li>
				<li><a href="HTrigger.asp"><% tran("bmenu.applicationsptriggering"); %></a></li>
			</ul>
		</div>
	</body>
</html>
