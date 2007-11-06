<% do_hpagehead("wl_basic.titl"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2>基本(無線LAN)</h2>
			<dl>
				<dt>無線LAN MAC動作モード</dt>
				<dd>この製品の無線LANは、次のようなモードで動作することができます：
					<ul class="wide">
						<li>AP &ndash; Infrastructureモードと言われる、無線LANにおける一般的なモードです。通常の無線LANクライアントを接続することができます。</li>
						<li>Client &ndash; アクセスポイントに接続するクライアントとして動作します。インターネット側の接続ポートとして、上流のアクセスポイントなどに接続する場合などに使用されます。LAN側ネットワークとはNAPTされたネットワークとして動作します。</li>
						<li>Client Bridged &ndash; アクセスポイントに接続するクライアントとして動作します。LAN側とのブリッジポートとして接続されます。Bridgedモードとして指定した場合は、インターネット側ポートは使用しません(無効にしてください)。このモードは"WLAN Adapter"(接続先アクセスポイントのLANポート拡張)として使用することができます。</li>
						<li>Ad-Hoc &ndash; Ad-Hoc接続は、クライアント同士で通信を行う場合に使用されます。</li>
					</ul><br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div><a href="HWDS.asp">WDS</a>機能は、APモードの場合のみ動作します。</div>
					</div>
				</dd>
				<dt>無線LAN MAC動作モード</dt>
				<dd>11gと11bデバイスを同時に使用する場合は、「Mixed」のまま使用してください。11gに対応したデバイスのみ使用する場合は「G-Only」、11bに対応したデバイスのみ使用する場合には「B-Only」を選択します。「無効」を選択した場合は、全ての無線機能が無効になります。WDSモードを使用する場合には、「B-Only」モードを使用することができません。</dd>
				<dt>ネットワーク名(SSID)</dt>
				<dd>無線LANネットワークの識別名を設定します。SSIDは他のネットワークと区別しなければなりません。大文字・小文字が区別された32文字までの文字列を設定することができます。ネットワーク内の無線LANデバイスに対しては、全て同じSSIDを設定する必要があります。<br /><br />
					<div class="note">
						<h4>メモ</h4>
						<div>セキュリティ上、または識別上の理由から、SSIDは必ず独自のものに変更するようにしてください</div>
					</div>
				</dd>
				<dt>無線チャンネル</dt>
				<dd>適切なチャンネルを設定します(FCC地域では1〜11ch、EU地域では1〜13ch、日本では1〜14ch)。ネットワーク内の全ての無線LAN機器は同じチャンネルである必要があります。近隣に無線LANが最も少ないチャンネルを使用することをお勧めします。</dd>
				<dt>SSIDの公開</dt>
				<dd>無線LANクライアントが接続するために公開するSSIDについて、公開しない設定を行うことができます。「無効」に設定することによって、SSIDを知る無線LANクライアントのみ接続することができるようになります。</dd>
<% ifndef("ACK", "<!--"); %>
				<dt>通信距離の有効範囲 (ACK Timing)</dt>
				<dd>通信を行う最大距離を設定することによって、無駄な再送を防ぐことができます。
					<div class="note">
						<h4>メモ</h4>
						<div>Atheros製無線LANクライアントを搭載した製品では、0を設定することによって、通常時と同様のAck Timeoutを使用します</div>
					</div>
					<ul class="wide">
						<li> 0 Ack Timing設定を無効にします。</li>
						<li> 1 - 999999 Ack Timingを設定値に変更します。</li>
					</ul>
				</dd>
<% ifndef("ACK", "-->"); %>
				<dd>設定値を保存する場合は「設定」ボタンをクリックしてください。設定内容を破棄する場合は「キャンセル」ボタンをクリックしてください。</dd>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
				<li><a href="HWPA.asp">無線LANセキュリティ</a></li>
				<li><a href="HWirelessAdvanced.asp">詳細(無線LAN)</a></li>
			</ul>
		</div>
	</body>
</html>