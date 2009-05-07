/bin/echo "log 500" | /usr/bin/nc 127.0.0.1 5001 | /bin/grep -v "^>" | /usr/bin/awk -F "," '{
	printf strftime("%Y%m%d %H:%M:%S ",$1);
	for (i=2;i<=NF;i++)
		printf $i" "
	printf "<br>\n";
	}'
