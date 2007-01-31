BEGIN {
	FS=":"
	start_form("Aliases")
	print "<table width=\"70%\" summary=\"Settings\">"
	print "<tr><th>Alias</th><th>SIP URI</th><th></th></tr>"
	print "<tr><td colspan=\"3\"><hr class=\"separator\" /></td></tr>"
}

($1 !~ /[()]/) {  print "<td>" $1 "</td><td>" $4 "</td><td align=\"right\" width=\"10%\"><a href=\"" url "?remove_alias=1&remove_aor=" $1 "\">Remove</a></td></tr>"}

END {
	print "<form enctype=\"multipart/form-data\" method=\"post\">"
	print "<tr><td><input type\"text\" name=\"alias_aor\" value=\"" alias "\" /></td><td><input type=\"text\" name=\"location_aor\" value=\"sip\\:" location "\" /></td><td style=\"width: 10em\"><input type=\"submit\" name=\"add_alias\" value=\"Add\" /></td></tr>"
	print "</form>"
	print "</table>"
	end_form()
}

