<% do_pagehead("share.change"); %>
	<script type="text/javascript">//
	//<![CDATA[
document.title = "<% nvg("router_name"); %> - User Password Change";
/*
 * based on passwordmeter by Jeff Todnem
 * see http://www.passwordmeter.com/js/pwdmeter.js for full license information
 */
function addLoadEvent(func) {
	var oldonload = window.onload;
	if (typeof window.onload != "function") {
		window.onload = func;
	}
	else {
		window.onload = function() {
			if (oldonload) {
				oldonload();
			}
			func();
		};
	}
}

function $() {
	var arrElms = [];
	for (var i=0; i < arguments.length; i++) {
		var elm = arguments[i];
		if (typeof(elm == "string")) { elm = document.getElementById(elm); }
		if (arguments.length == 1) { return elm; }
		arrElms.push(elm);
	}
	return arrElms;
}

String.prototype.strReverse = function() {
	var newstring = "";
	for (var s=0; s < this.length; s++) {
		newstring = this.charAt(s) + newstring;
	}
	return newstring;
	//strOrig = ' texttotrim ';
	//strReversed = strOrig.revstring();
};

function chkPass(pwd) {
	var oScorebar = $("scorebar");
	var oScore = $("score");
	var oComplexity = $("complexity");
	// Simultaneous variable declaration and value assignment aren't supported in IE apparently
	// so I'm forced to assign the same value individually per var to support a crappy browser *sigh* 
	var nScore=0, nLength=0, nAlphaUC=0, nAlphaLC=0, nNumber=0, nSymbol=0, nMidChar=0, nRequirements=0, nAlphasOnly=0, nNumbersOnly=0, nUnqChar=0, nRepChar=0, nRepInc=0, nConsecAlphaUC=0, nConsecAlphaLC=0, nConsecNumber=0, nConsecSymbol=0, nConsecCharType=0, nSeqAlpha=0, nSeqNumber=0, nSeqSymbol=0, nSeqChar=0, nReqChar=0, nMultConsecCharType=0;
	var nMultRepChar=1, nMultConsecSymbol=1;
	var nMultMidChar=2, nMultRequirements=2, nMultConsecAlphaUC=2, nMultConsecAlphaLC=2, nMultConsecNumber=2;
	var nReqCharType=3, nMultAlphaUC=3, nMultAlphaLC=3, nMultSeqAlpha=3, nMultSeqNumber=3, nMultSeqSymbol=3;
	var nMultLength=4, nMultNumber=4;
	var nMultSymbol=6;
	var nTmpAlphaUC="", nTmpAlphaLC="", nTmpNumber="", nTmpSymbol="";
	var sAlphas = "abcdefghijklmnopqrstuvwxyz";
	var sNumerics = "01234567890";
	var sSymbols = ")!@#$%^&*()";
	var sComplexity = management.too_short;
	var sStandards = "Below";
	var nMinPwdLen = 8;
	if (document.all) { var nd = 0; } else { var nd = 1; }
	if (pwd) {
		nScore = parseInt(pwd.length * nMultLength);
		nLength = pwd.length;
		var arrPwd = pwd.replace(/\s+/g,"").split(/\s*/);
		var arrPwdLen = arrPwd.length;
		
		/* Loop through password to check for Symbol, Numeric, Lowercase and Uppercase pattern matches */
		for (var a=0; a < arrPwdLen; a++) {
			if (arrPwd[a].match(/[A-Z]/g)) {
				if (nTmpAlphaUC !== "") { if ((nTmpAlphaUC + 1) == a) { nConsecAlphaUC++; nConsecCharType++; } }
				nTmpAlphaUC = a;
				nAlphaUC++;
			}
			else if (arrPwd[a].match(/[a-z]/g)) { 
				if (nTmpAlphaLC !== "") { if ((nTmpAlphaLC + 1) == a) { nConsecAlphaLC++; nConsecCharType++; } }
				nTmpAlphaLC = a;
				nAlphaLC++;
			}
			else if (arrPwd[a].match(/[0-9]/g)) { 
				if (a > 0 && a < (arrPwdLen - 1)) { nMidChar++; }
				if (nTmpNumber !== "") { if ((nTmpNumber + 1) == a) { nConsecNumber++; nConsecCharType++; } }
				nTmpNumber = a;
				nNumber++;
			}
			else if (arrPwd[a].match(/[^a-zA-Z0-9_]/g)) { 
				if (a > 0 && a < (arrPwdLen - 1)) { nMidChar++; }
				if (nTmpSymbol !== "") { if ((nTmpSymbol + 1) == a) { nConsecSymbol++; nConsecCharType++; } }
				nTmpSymbol = a;
				nSymbol++;
			}
			/* Internal loop through password to check for repeat characters */
			var bCharExists = false;
			for (var b=0; b < arrPwdLen; b++) {
				if (arrPwd[a] == arrPwd[b] && a != b) { /* repeat character exists */
					bCharExists = true;
					/* 
					Calculate icrement deduction based on proximity to identical characters
					Deduction is incremented each time a new match is discovered
					Deduction amount is based on total password length divided by the
					difference of distance between currently selected match
					*/
					nRepInc += Math.abs(arrPwdLen/(b-a));
				}
			}
			if (bCharExists) { 
				nRepChar++; 
				nUnqChar = arrPwdLen-nRepChar;
				nRepInc = (nUnqChar) ? Math.ceil(nRepInc/nUnqChar) : Math.ceil(nRepInc); 
			}
		}

		/* Check for sequential alpha string patterns (forward and reverse) */
		for (var s=0; s < 23; s++) {
			var sFwd = sAlphas.substring(s,parseInt(s+3));
			var sRev = sFwd.strReverse();
			if (pwd.toLowerCase().indexOf(sFwd) != -1 || pwd.toLowerCase().indexOf(sRev) != -1) { nSeqAlpha++; nSeqChar++;}
		}

		/* Check for sequential numeric string patterns (forward and reverse) */
		for (var s=0; s < 8; s++) {
			var sFwd = sNumerics.substring(s,parseInt(s+3));
			var sRev = sFwd.strReverse();
			if (pwd.toLowerCase().indexOf(sFwd) != -1 || pwd.toLowerCase().indexOf(sRev) != -1) { nSeqNumber++; nSeqChar++;}
		}

		/* Check for sequential symbol string patterns (forward and reverse) */
		for (var s=0; s < 8; s++) {
			var sFwd = sSymbols.substring(s,parseInt(s+3));
			var sRev = sFwd.strReverse();
			if (pwd.toLowerCase().indexOf(sFwd) != -1 || pwd.toLowerCase().indexOf(sRev) != -1) { nSeqSymbol++; nSeqChar++;}
		}

	/* Modify overall score value based on usage vs requirements */

		/* General point assignment */
		if (nAlphaUC > 0 && nAlphaUC < nLength) {	
			nScore = parseInt(nScore + ((nLength - nAlphaUC) * 2));
		}
		if (nAlphaLC > 0 && nAlphaLC < nLength) {	
			nScore = parseInt(nScore + ((nLength - nAlphaLC) * 2)); 
		}
		if (nNumber > 0 && nNumber < nLength) {	
			nScore = parseInt(nScore + (nNumber * nMultNumber));
		}
		if (nSymbol > 0) {	
			nScore = parseInt(nScore + (nSymbol * nMultSymbol));
		}
		if (nMidChar > 0) {	
			nScore = parseInt(nScore + (nMidChar * nMultMidChar));
		}
		
		/* Point deductions for poor practices */
		if ((nAlphaLC > 0 || nAlphaUC > 0) && nSymbol === 0 && nNumber === 0) {  // Only Letters
			nScore = parseInt(nScore - nLength);
			nAlphasOnly = nLength;
		}
		if (nAlphaLC === 0 && nAlphaUC === 0 && nSymbol === 0 && nNumber > 0) {  // Only Numbers
			nScore = parseInt(nScore - nLength); 
			nNumbersOnly = nLength;
		}
		if (nRepChar > 0) {  // Same character exists more than once
			nScore = parseInt(nScore - nRepInc);
		}
		if (nConsecAlphaUC > 0) {  // Consecutive Uppercase Letters exist
			nScore = parseInt(nScore - (nConsecAlphaUC * nMultConsecAlphaUC)); 
		}
		if (nConsecAlphaLC > 0) {  // Consecutive Lowercase Letters exist
			nScore = parseInt(nScore - (nConsecAlphaLC * nMultConsecAlphaLC)); 
		}
		if (nConsecNumber > 0) {  // Consecutive Numbers exist
			nScore = parseInt(nScore - (nConsecNumber * nMultConsecNumber));  
		}
		if (nSeqAlpha > 0) {  // Sequential alpha strings exist (3 characters or more)
			nScore = parseInt(nScore - (nSeqAlpha * nMultSeqAlpha)); 
		}
		if (nSeqNumber > 0) {  // Sequential numeric strings exist (3 characters or more)
			nScore = parseInt(nScore - (nSeqNumber * nMultSeqNumber)); 
		}
		if (nSeqSymbol > 0) {  // Sequential symbol strings exist (3 characters or more)
			nScore = parseInt(nScore - (nSeqSymbol * nMultSeqSymbol)); 
		}
		/* Determine complexity based on overall score */
		if (nScore > 100) { nScore = 100; } else if (nScore < 0) { nScore = 0; }
		if (nScore >= 0 && nScore < 20) { sComplexity = management.very_weak; }
		else if (nScore >= 20 && nScore < 40) { sComplexity = management.weak; }
		else if (nScore >= 40 && nScore < 60) { sComplexity = management.good; }
		else if (nScore >= 60 && nScore < 80) { sComplexity = management.strong; }
		else if (nScore >= 80 && nScore <= 100) { sComplexity = management.very_strong; }
		
		/* Display updated score criteria to client */
		oScorebar.style.backgroundPosition = "-" + parseInt(nScore * 4) + "px";
		oScore.innerHTML = nScore + "%";
		oComplexity.innerHTML = sComplexity;
	}
	else {
		/* Display default score criteria to client */
		initPwdChk();
		oScore.innerHTML = nScore + "%";
		oComplexity.innerHTML = sComplexity;
	}
}

function initPwdChk(restart) {
	$("scorebar").style.backgroundPosition = "0";
}

addLoadEvent(function() { initPwdChk(1); });

function valid_password(F) {
	if (F.http_passwd.value != F.http_passwdConfirm.value) {
		alert(errmsg.err10);
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}
	
	if (!F.http_passwd.value || F.http_passwd.value == "" || F.http_passwd.value.length == 0) {
		alert(errmsg.err6);
		F.http_passwdConfirm.focus();
		F.http_passwdConfirm.select();
		return false;
	}
	return true;
}

function to_submit(F) {
	if (valid_password(F)) {
		F.change_action.value = "gozila_cgi";
		F.submit_button.value = "index";
		F.submit_type.value = "changepass";
		F.changepassword.value = management.cpbutton_changed;
		F.action.value = "Apply";
		apply(F);
	}
}

	//]]>
	</script>
	</head>

	<body class="gui">
		<div id="wrapper">
			<div id="content" class="infopage">
				<div id="header">
					<div id="logo"><h1><% show_control(); %></h1></div>
					<div id="menu">
						<div id="menuMain">
							<ul id="menuMainList">
								<li><span><strong><% tran("bmenu.setup"); %></strong></span></li>
								<% ifndef("HASWIFI", "<!--"); %>
								<li><span><strong><% tran("bmenu.wireless"); %></strong></span></li>
								<% ifndef("HASWIFI", "-->"); %>								
								<% nvim("sipgate","1","<!--"); %>
								<li><span><strong><% tran("bmenu.sipath"); %></strong></span></li>
								<% nvim("sipgate","1","-->"); %>
								<li><span><strong><% tran("bmenu.services"); %></strong></span></li>
								<% ifdef("HAVE_ANTAIRA_MINI","<!--"); %>
								<li><span><strong><% tran("bmenu.security"); %></strong></span></li>
								<% ifdef("HAVE_ANTAIRA_MINI","-->"); %>
								<% ifdef("HAVE_ANTAIRA_MINI","<!--"); %>
								<li><span><strong><% tran("bmenu.accrestriction"); %></strong></span></li>
								<% ifdef("HAVE_ANTAIRA_MINI","-->"); %>
								<li><span><strong><% tran("bmenu.applications"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.admin"); %></strong></span></li>
								<li><span><strong><% tran("bmenu.statu"); %></strong></span></li>
							</ul>
						</div>
					</div>
				</div>
				<div id="main">
					<form name="changepassword" action="apply.cgi" method="post" autocomplete="new-password">
						<input type="hidden" name="submit_button" value="index" />
						<input type="hidden" name="submit_type" />
						<input type="hidden" name="next_page" value="<% getsetuppage(); %>" />
						<input type="hidden" name="change_action" />
						<input type="hidden" name="action" value="Apply" />
						<div id="contentsInfo">
							<h2><% tran("management.h2"); %></h2>
							<div class="warning">
								<div id="warning_text">
									<p><b><% tran("management.changepassword"); %></script></b></p>
								</div>
							</div><br />
							<fieldset>
								<legend><% tran("management.psswd_legend"); %></legend>
								<table id="tablePwdCheck" cellpadding="5" cellspacing="1" border="0">
									<tr>
										<th><% tran("management.psswd_user"); %></th>
										<td><input type="password" autocomplete="new-password" maxlength="63" value="d6nw5v1x2pc7st9m" name="http_username" onclick="this.select();" onblur="valid_name(this,management.psswd_user,SPACE_NO|CHECK)" /></td>
									</tr>
									<tr>
										<th><% tran("management.psswd_pass"); %></th>
										<td>
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input title=\"" + management.pwd_ti_aria + "\" aria-label=\"" + management.pwd_ti_aria + "\" type=\"password\" autocomplete=\"new-password\" maxlength=\"63\" value=\"d6nw5v1x2pc7st9m\" name=\"http_passwd\" onclick=\"this.select();\" onblur=\"valid_name(this,management.psswd_pass,SPACE_NO|CHECK)\" onkeyup=\"chkPass(this.value)\" />");
											//]]>
											</script>
										</td>
									</tr>
									<tr>
										<th><% tran("management.pass_conf"); %></th>
										<td>
											<script type="text/javascript">
											//<![CDATA[
											document.write("<input title=\"" + management.pwd_ti_aria + "\" aria-label=\"" + management.pwd_ti_aria + "\" type=\"password\" autocomplete=\"new-password\" maxlength=\"63\" value=\"d6nw5v1x2pc7st9m\" name=\"http_passwdConfirm\" onclick=\"this.select();\" onblur=\"valid_name(this,management.pass_conf,SPACE_NO|CHECK)\" />");
											//]]>
											</script>
										</td>
									</tr>
									<tr>
										<th><% tran("management.score"); %></th>
										<td>
											<div id="scorebarBorder">
												<div id="score">0%</div>
												<div id="scorebar">&nbsp;</div>
											</div>
										</td>
									</tr>
									<tr>
										<th><% tran("management.complexity"); %></th>
										<td><div id="complexity"><% tran("management.too_short"); %></div></td>
									</tr>
								</table>
							</fieldset><br />
							<div id="footer" class="submitFooter">
								<script type="text/javascript">
								//<![CDATA[
								document.write("<input class=\"button green_btn\" type=\"button\" name=\"changepassword\" value=\"" + management.cpbutton + "\" onclick=\"to_submit(this.form)\" />");
								//]]>
								</script>
							</div>
						</div>
					</form>
				</div>
				<div id="floatKiller"></div>
				<div id="statusInfo">
					<div class="info"><% tran("share.firmware"); %>:&nbsp;
						<script type="text/javascript">
						//<![CDATA[
						document.write("<a title=\"" + share.about + "\" href=\"<% get_firmware_version_href(); %>\"><% get_firmware_version(); %></a>");
						//]]>
						</script>
					</div>
					<div class="info"><% tran("share.time"); %>:  <span id="uptime"><% get_uptime(); %></span></div>
					<div class="info">WAN<span id="ipinfo"><% show_wanipinfo(); %></span></div>
				</div>
			</div>
		</div>
	</body>
</html>
