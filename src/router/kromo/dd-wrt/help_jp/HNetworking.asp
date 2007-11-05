<% do_hpagehead("bmenu.networking"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>ネットワーク・VLAN設定</h2>
			<dl>
				<dd><% tran("networking.h2"); %> は、一つのインターフェースで異なる独立したネットワークを同時使用する機能です。VLANでは、ヘッダに所属するネットワークを示すTagを付加することによって区別され、スイッチはその情報を元に、どのPortに転送すべきか決定することができます。無線インターフェースの通信との分離や異なるRoutingポリシーを適用したい場合などに利用することができ、主として大規模ISPや企業などに利用されます。</dd>

				<dt><% tran("networking.legend"); %></dt>
				<dd>新たにPVIDを割り当てたVLANインターフェースを作成することができます。<dd>
				<dt><% tran("networking.h22"); %> - <% tran("networking.legend2"); %></dt>
				<dd>ブリッジデバイスを作成します。これに他のインターフェースを関連づけることによって、インターフェース間のブリッジを行うことができるようになります。STPは、スパニングツリーの設定を行うことができます。PRIOには適切なブリッジ・プライオリティを設定する必要があります。<dd>
				<dt><% tran("networking.h22"); %> - <% tran("networking.legend3"); %></dt>
				<dd>ブリッジデバイスに他のインターフェースを関連づけます。無線デバイスを関連づける場合は、「Wireless Interface」オプションを有効にします。標準のブリッジ構成は、ここで設定されたもので上書きされます。<dd>
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
	</body>
</html>
