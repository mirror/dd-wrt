/bin/echo "status 2" | /usr/bin/nc 127.0.0.1 5001  | grep -v "^>\|TITLE" | awk -F "," 'BEGIN{print "<table>"}{
	print "<tr>";
	if ($1 == "HEADER")
		{
		print "<tr><td colspan=6><hr></td></tr>";
		for (i=3;i<NF;i++)
			{
			#ueberschrift
			printf "<td><b>"$i"</b></td>"
			}
		}
	else if ($1 == "TITLE")
		{
		for (i=2;i<=NF;i++)
			{
			printf "<td>"$i"</td>";
			}
		}
	else
		{
		for (i=2;i<NF;i++)
			{
			printf "<td>"$i"</td>";
			}
		}
	print"</tr>"
	}
END{print "</table>"}'
