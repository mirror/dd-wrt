BEGIN {
	FS=":"
	start_form("Local Subscribers")
	print "<table width=\"70%\" summary=\"Settings\">"
	print "<tr><th>SIP Username</th><th>Password</th></tr>"
	print "<tr><td colspan=\"3\"><hr class=\"separator\" /></td></tr>"
}

($1 !~ /[()]/) { print "<td>" $2 "</td><td>" $4 "</td><td align=\"right\" width=\"10%\"><a href=\"" url "?remove_subscriber=1&remove_username=" $2 "\">Remove</a></td></tr>"}

END {	
	print "<form enctype=\"multipart/form-data\" method=\"post\">"
        print "<tr><td><input type\"text\" name=\"subscriber_username\" value=\"username" username "\" /></td>"
	print "<td><input type=\"text\" name=\"subscriber_password\" value=\"password" password "\" /></td>"
	print "<td style=\"width: 10em\"><input type=\"submit\" name=\"add_subscriber\" value=\"Add\" /></td></tr>"
	print "</table>"
	end_form()
}

