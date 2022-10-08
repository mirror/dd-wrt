<% do_pagehead_nopwc(""); %>
	<script type="text/javascript">
	//<![CDATA[

function do_logout() {
  var userAgent = navigator.userAgent.toLowerCase();

  if (userAgent.indexOf("msie") != -1) {
    document.execCommand("ClearAuthenticationCache", false);
  }

  xhr_objectCarte = null;

  if(window.XMLHttpRequest) {
    xhr_object = new XMLHttpRequest();
  } else if(window.ActiveXObject) {
    xhr_object = new ActiveXObject("Microsoft.XMLHTTP");
  } else {
    alert ("Your browser doesn't support XMLHTTPREQUEST");
  }
  if (document.location.port != "") {
		xhr_object.open ('GET', document.location.protocol + '//' + document.location.hostname + ':' + document.location.port + '/index.asp', false, 'logout', (new Date()).getTime().toString());
  } else {
		xhr_object.open ('GET', document.location.protocol + '//' + document.location.hostname + '/index.asp', false, 'logout', (new Date()).getTime().toString());  
  }
  xhr_object.send ("");
  xhr_object = null;

  if (document.location.port != "") {
		document.location = document.location.protocol + '//' + document.location.hostname + ':' + document.location.port + '/'; 
  } else {
		document.location = document.location.protocol + '//' + document.location.hostname + '/'; 
  }
  return false;
}

addEvent(window, "load", function() {
	setTimeout(do_logout, 4000);
});
	//]]>
	</script>
</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content">
				<div id="header">
					<div id="logo" style="margin: 0 auto;">
						<h1>DD-WRT Control Panel</h1>
					</div>
					<div class="message" style="width:450px;">
						<form id="bye_bye">
							<% tran("logout.message"); %><br><br>
						</form>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>
