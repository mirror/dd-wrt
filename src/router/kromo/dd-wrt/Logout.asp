<% do_pagehead(""); %>
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
  xhr_object.open ('GET', document.location.protocol + '//' + document.location.hostname + '/index.asp', false, 'logout', (new Date()).getTime().toString());
  xhr_object.send ("");
  xhr_object = null;

  document.location = document.location.protocol + '//' + document.location.hostname + '/index.asp'; 
  return false;
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
