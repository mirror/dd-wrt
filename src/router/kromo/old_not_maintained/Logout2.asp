<% do_pagehead(""); %>
	<script type="text/javascript">
	//<![CDATA[

function logoutmethod(x, safeLocation)
{
            if (x) {
                x.open("HEAD", safeLocation || location.href, true, "logout", (new Date()).getTime().toString());
                x.send("");
                return 1;
            } else {
                return 0;
            }

}

function do_logout(){
    var outcome, u, m = "You should be logged out now.";
    try { 
    outcome = document.execCommand("ClearAuthenticationCache"); 
    }catch(e){}
    safeLocation = document.location.protocol + '//' + document.location.hostname + '/index.asp';
    if (!outcome) {
        outcome =  logoutmethod(window.XMLHttpRequest ? new window.XMLHttpRequest() : ( window.ActiveXObject ? new ActiveXObject("Microsoft.XMLHTTP") : u), safeLocation);
    }
    if (!outcome) {
        m = "Your browser is too old or too weird to support log out functionality. Close all windows and restart the browser.";
    }
    alert(m);
}

addEvent(window, "load", function() {
	setTimeout(do_logout, 2000);
});
	//]]>
	</script>
</head>

	<body>
		<div class="message">
			<form>
				<% tran("logout.message"); %><br />
			</form>
		</div>
	</body>
</html>
