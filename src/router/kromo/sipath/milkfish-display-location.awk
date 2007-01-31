BEGIN {
	FS=":"
	start_form("Accounts")
	print "<table width=\"70%\" summary=\"Settings\">"
	print "<tr><th>Account</th><th></th></tr>"
	print "<tr><td colspan=\"3\"><hr class=\"separator\" /></td></tr>"
}

($1 !~ /[()]/) { print "<td>" $1 "</td><td align=\"right\" width=\"10%\"><a href=\"" url "?remove_location=1&remove_aor=" $1 "\">Remove</a></td></tr>"}

END {
	print "</table>"
	end_form()
}

