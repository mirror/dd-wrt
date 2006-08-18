<% do_hpagehead(); %>
		<title>Help - Access Restrictions</title>
	</head>
	<body>
		<div id="header">
			<div class="logo"> </div>
			<div class="navig"><a href="index.asp">Index</a> | <a href="javascript:self.close();">Close</a></div>
		</div>
		<div id="content">
			<h2><% tran("filter.h2"); %></h2>
			<dl>
				<dd>This screen allows you to block or allow specific kinds of Internet usage. You can set up Internet access policies for specific PCs and set up filters by using network port numbers.</dd>
				<dt><% tran("filter.legend"); %></dt>
				<dd>This feature allows you to customize up to ten different Internet Access Policies for particular PCs, which are identified by their IP or MAC addresses. For each policy designated PCs, during the days and time periods specified.<br /><br />
					To create or edit a policy, follow these instructions:
					<ol class="wide">
						<li>Select the policy number (1-10) in the drop-down menu.</li>
						<li>Enter a name in the Enter Profile Name field.</li>
						<li>Click the <i>Edit List of PCs</i> button.</li>
						<li>On the <i>List of PCs</i> screen, specify PCs by IP address or MAC address. Enter the appropriate IP addresses into the <i>IP</i> fields. If you have a range of IP addresses to filter, complete the appropriate <i>IP Range</i> fields. Enter the appropriate MAC addresses into the <i>MAC</i> fields.</li>
						<li>Click the <i>Apply</i> button to save your changes. Click the <i>Cancel</i> button to cancel your unsaved changes. Click the <i>Close</i> button to return to the Filters screen.</li>
						<li>If you want to block the listed PCs from Internet access during the designated days and time, then keep the default setting, <i>Deny</i>. If you want the listed PCs to be able to access the Internet during the designated days and time, then click the radio button next to <i>Allow</i>.</li>
						<li>Set the days when access will be filtered. Select <i>Everyday</i> or the appropriate days of the week.</li>
						<li>Set the time when access will be filtered. Select <i>24 Hours</i>, or check the box next to <i>From</i> and use the drop-down boxes to designate a specific time period.</li>
						<li>Click the <i>Add to Policy</i> button to save your changes and active it.</li>
						<li>To create or edit additional policies, repeat steps 1-9.</li>
					</ol><br />
					To delete an Internet Access Policy, select the policy number, and click the <i>Delete</i> button.
				</dd>
				<dt><% tran("sbutton.summary"); %></dt>
				<dd>To see a summary of all the policies, click the <i>Summary</i> button. The Internet Policy Summary screen will show each policy's number, Policy Name, Days, and Time of Day. To delete a policy, click its box, and then click the <i>Delete</i> button. Click the <i>Close</i> button to return to the Filters screen.</dd>
			</dl>
		</div>
	</body>
</html>
