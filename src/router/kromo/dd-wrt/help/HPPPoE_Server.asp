<% do_hpagehead("service.pppoesrv_legend"); %>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();"><% tran("sbutton.clos"); %></a></div>
		</div>
		<div id="content">
			<h2><% tran("service.pppoesrv_legend"); %></h2>
			<dl>
				<!--dd>PPPoE Server.....<br /><br />
					<div class="note">
						<h4>Note</h4>
						<div>Please note....</div>
					</div>
				</dd-->
				<% tran("hpppoesrv.page1"); %>
				<dt><% tran("service.pppoesrv_legend"); %></dt>
				<!--dd>Click <i>Save Settings</i> to save your settings or click <i>Cancel Changes</i> to cancel your unsaved changes.</dd-->
				<% tran("hpppoesrv.page2"); %>
			</dl>
		</div>
		<div class="also">
			<h4><% tran("share.seealso"); %></h4>
			<ul>
			</ul>
		</div>
	</body>
</html>
