<html>
  <head>
    <script type="text/javascript" src="/intatstart/js/mdetect.js"></script>
    <script type="text/javascript">
	var detect = function() {
    		if(DetectTierIphone()) {
			document.location = "http://<% nvram_get("lan_ipaddr"); %>/InternetAtStart.asp?ias_detect";
    		} else {
			document.location = "http://<% nvram_get("lan_ipaddr"); %>/index.asp?ias_detect";
			//document.location.href = "/index.asp?ias_detect";
			//document.forms['redirectform'].action = "index.asp?ias_detect";
			//document.forms['redirectform'].submit();
    		}
	}
    </script>
  </head>
<body onload="detect();">
  <form name="redirectform" action="apply.cgi?ias_detect" method="post">
    
    <input type="hidden" name="submit_button" value="index">
    <input type="hidden" name="submit_type" value="ias_redirect">
    <input type="hidden" name="change_action" value="gozila_cgi">
    <input type="hidden" name="ias_startup" value="2">
    
    <img src="/0.jpg?ias_detect" width="1" height="1">

  </form> 
</body>
</html>
