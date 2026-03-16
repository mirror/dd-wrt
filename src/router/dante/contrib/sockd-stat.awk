#
# sockd-stat.awk for the DANTE socks 4/5 daemon
# Copyleft 2001 Stephan Eisvogel <eisvogel@hawo.stw.uni-erlangen.de>
# This code is licensed under GPL v2
#
# If you use this regularly, help boost my ego ;) and do a:
# uname -a | mail -s "I use sockd-stat!" eisvogel@hawo.stw.uni-erlangen.de
#
# If you make any fixes or improvements, please do send do them to me!
# All testing was done with DANTE version 1.1.6 on RedHat(tm) Linux.
# If there are updates you can probably grab them from:
# http://www.hawo.stw.uni-erlangen.de/~eisvogel/sockd
#
#
# HOW TO USE
#
# Find out how your system rotates its logs and run this script
# post-rotating, e.g. by doing a
#
#    zcat /var/log/sockd.1.gz | awk -f /root/bin/sockd-stat.awk | \
#    mail -s "Weekly SOCKD usage" root
#
# If you have a directory called /etc/logrotate.d then take a look
# at the files in there for some run-post-rotate examples. Note that
# this script uses some basic GNU-tools to get things done, namely
# "sort", "head", and "rm", if you want reverse IP lookups you need
# the "nslookup" utility as well.
#
# This is my last big AWK script, I am going Perl.
#
#
# RELEASE NOTES
#
# v1.0 @ 01.01.2001
#   NEC's socks daemon keeps crashing on Linux with 200+ users no
#   matter what I do, so we switched to DANTE for the time being.
#   Didn't like the last remote exploit too much either, maybe NEC
#   needs more DJB-type developers?
# v1.1 @ 02.01.2001
#   + fixed issue with different syslog daemons (Michael Shuldman)
#   + added request count output for the port statistics
#   + blocked UDP requests were not counted
#   + changed regexp that matches lines with statistics
#   + rmfile() function
#   + code cleanups, renamed some variables for clarity
#   + beautified output IMO
#   + added request types to general statistics

BEGIN {
	#
	# Configurable items
	# (0 means all)
	#
	SHOW_PORTS=30;
	SHOW_CLIENTS=0;
	SHOW_DESTINATIONS=50;
	if (nodns == 1)
		LOOKUP_IPS=0;
	else
		LOOKUP_IPS=1;

	#
	# no need to change anything below
	#
	IGNORECASE=1;
	MEG=1024*1024;
	lastdns="(unknown)";

	lines=0;
	passed=0;
	denied=0;

	total_to_client_bytes=0;
	total_from_client_bytes=0;
	total_to_target_bytes=0;
	total_from_target_bytes=0;
}

#
# Sort 'filename' contents numerically in 'field'
# and display the 'count' largest lines, optionally
# treat first field as IP address and append it
# resolved into a hostname to the end of the line
#
function sorted_output (filename, field, count, rev_dns,	curhost,a,b)
{
	# sort and stuff all lines it into a temp file
	if (count==0)
	system("sort -nr +"field" "filename"  >"filename".1");
	else system("sort -nr +"field" "filename" | head -n "count" >"filename".1");

	# read temp file back in, print it out and rev-dns if needed
	b=filename".1";
	while ((getline curhost < b)>0) {
		printf "%s",curhost;
		if (rev_dns==1) {
			split(curhost,a);
			dns_lookup(a[1],filename".2");
			printf "%s",lastdns;
		}
		printf "\n";
	}
	close(b);
	rmfile(b);
}

#
# Wade through /etc/services and grab the descriptive service name of a port
#
function get_service (port,	myline,a,s)
{
	split(port,s,/\//);
	curr_service=s[1];
	while ((getline myline < "/etc/services")>0) {
		split(myline,a);
		if (a[2]==port) {
			curr_service=a[1];
			break;
		}
	}
	close ("/etc/services");
}

#
# Somewhere we just have to draw a line
#
function draw_line (n,	i)
{
	for (i=0; i<n; i++) printf "-";
	printf "\n";
}

#
# Simple DNS lookup using the nslookup command
#
function dns_lookup (ip,tmpfile, name,host,a)
{
	lastdns=ip; # unsuccessful lookup -> show IP
	system("nslookup "ip" 2>/dev/null >"tmpfile);
	while ((getline name < tmpfile)>0) {
		split(name,host);
		if (index(host[1],"Name:")!=0) {
			lastdns=host[2];
			break;
		}
	}
	close(tmpfile)
	rmfile(tmpfile);
}

#
# Remove file sans output
#
function rmfile (filename)
{
	system("rm "filename" 2>/dev/null >/dev/null");
}

#
# Matches all lines, count them
#
/.*/ {
	lines++;
}

#
# Count blocked requests
#
/ sockd\[[0-9]+\]: block\([0-9]+\): / {
	denied++;
	#
	# compensate for different syslog daemons
	#
	field_ofs=0;
	split($0,curr_line);
	for (i=1; i<=NF; i++)
		if (match(curr_line[i],/sockd\[[0-9]+\]:/)) {
			field_ofs = i + 1;
			break;
		}
	#
	# count BLOCK item
	#
	m_block[curr_line[field_ofs+2]] += 1;
	next;
}

#
# Matches lines with statistical information
#
/ sockd\[[0-9]+\]: pass\([0-9]+\): .* \]: .* -> .* -> .* -> .* -> / {

	passed++;

	#
	# compensate for different syslog daemons
	#
	field_ofs=0;
	split($0,curr_line);
	for (i=1; i<=NF; i++)
		if (match(curr_line[i],/sockd\[[0-9]+\]:/)) {
			field_ofs = i + 1;
			break;
		}
	#
	# count PASS item
	#
	m_pass[curr_line[field_ofs+2]] += 1;

	#
	# clean up fields
	#
	gsub(/,/,"",curr_line[field_ofs+9]);
	gsub(/:/,"",curr_line[field_ofs+15]);
	split(curr_line[field_ofs+6],a,/\./);
	c_ip=a[1]"."a[2]"."a[3]"."a[4];
	if (match(curr_line[field_ofs+12],/^`world'$/))
		curr_line[field_ofs+12]="0.0.0.0."a[5];
	split(curr_line[field_ofs+12],a,/\./);
	t_ip=a[1]"."a[2]"."a[3]"."a[4];
	t_port=a[5];

	#
	# add to totals
	#
	total_to_client_bytes += curr_line[field_ofs+4];
	total_from_client_bytes += curr_line[field_ofs+9];
	total_to_target_bytes += curr_line[field_ofs+10];
	total_from_target_bytes += curr_line[field_ofs+15];

	#
	# update client
	# (but exclude broken socksified Half-Life requests)
	#
	if (match(c_ip,/0\.0\.0\.0/)==0) {
		client[c_ip]=1;
		to_client[c_ip] += curr_line[field_ofs+4];
		from_client[c_ip] += curr_line[field_ofs+9];
	}

	#
	# update target
	#
	target[t_ip]=1;
	to_target[t_ip] += curr_line[field_ofs+10];
	from_target[t_ip] += curr_line[field_ofs+15];

	#
	# update port
	#
	if (match(curr_line[field_ofs+2],/tcp/)) {
		tcp_port[t_port]=1;
		tcp_port_to[t_port] += curr_line[field_ofs+10];
		tcp_port_from[t_port] += curr_line[field_ofs+15];
		tcp_port_used[t_port] += 1;
	} else {
		udp_port[t_port]=1;
		udp_port_to[t_port] += curr_line[field_ofs+10];
		udp_port_from[t_port] += curr_line[field_ofs+15];
		udp_port_used[t_port] += 1;
	}
}


#
# After all lines are done, sort and print the statistics
#
END {
	# create a random filename for sorting
	srand();
	CONVFMT="%d"
	r=rand()*999999999;
	tmpfile="/tmp/tmp.sockd-stat."r;

	printf "SOCKD statistics version 1.1\n";
	printf "Copyleft 2001 Stephan Eisvogel <eisvogel@hawo.stw.uni-erlangen.de>\n";

	total_clients=0;
	total_targets=0;
	for (c in client) total_clients++;
	for (t in target) total_targets++;

	printf "\nGeneral statistics\n"; draw_line(41);
	printf "Lines parsed               : %12d\n", lines;
	printf "Unique clients             : %12d\n", total_clients;
	printf "Unique targets             : %12d\n", total_targets;
	printf "Passed requests            : %12d\n", passed;

	for (m in m_pass) printf "    %-22s : %12d\n", m, m_pass[m] >> tmpfile
	close(tmpfile);
	sorted_output(tmpfile,2,0,0);
	rmfile(tmpfile);

	printf "Blocked requests           : %12d\n", denied;
	for (m in m_block) printf "    %-22s : %12d\n", m, m_block[m] >> tmpfile
	close(tmpfile);
	sorted_output(tmpfile,2,0,0);
	rmfile(tmpfile);
	draw_line(41);

	printf "\nVolume totals\n"; draw_line(28);
	printf "Clients <--    %10.1f MB\n",total_to_client_bytes/MEG;
	printf "Clients -->    %10.1f MB\n",total_from_client_bytes/MEG;
	printf "Targets <--    %10.1f MB\n",total_to_target_bytes/MEG;
	printf "Targets -->    %10.1f MB\n",total_from_target_bytes/MEG;
	draw_line(28);

	#
	# TCP/UDP port stats
	#
	printf "\nPort   Service                      ";
	printf "To         From        Total     Requests\n";
	draw_line(77);
	for (p in tcp_port) {
		printf "%-6s ",p >> tmpfile;
		get_service(p"/tcp");
		outline="("curr_service"/tcp)";
		printf "%-23s",outline >>tmpfile;
		printf "%8.1f MB  %8.1f MB  %8.1f MB  %8d\n",
			tcp_port_to[p]/MEG,
			tcp_port_from[p]/MEG,
			(tcp_port_from[p]+tcp_port_to[p])/MEG,
			tcp_port_used[p] >> tmpfile;
	}
	for (p in udp_port) {
		printf "%-6s ",p >> tmpfile;
		get_service(p"/udp");
		outline="("curr_service"/udp)";
		printf "%-23s",outline >>tmpfile;
		printf "%8.1f MB  %8.1f MB  %8.1f MB  %8d\n",
			udp_port_to[p]/MEG,
			udp_port_from[p]/MEG,
			(udp_port_from[p]+udp_port_to[p])/MEG,
			udp_port_used[p] >> tmpfile;
	}
	close(tmpfile);
	sorted_output(tmpfile,6,SHOW_PORTS,0);
	rmfile(tmpfile);
	draw_line(77);

	#
	# Client stats
	#
	print "\nClient IP              To         From        Total     FQDN";
	draw_line(77);
	for (c in client) {
		printf "%-16s %8.1f MB  %8.1f MB  %8.1f MB  \n",
			c,
			to_client[c]/MEG,
			from_client[c]/MEG,
			(from_client[c]+to_client[c])/MEG >> tmpfile;
	}
	close(tmpfile);
	sorted_output(tmpfile,5,SHOW_CLIENTS,LOOKUP_IPS);
	rmfile(tmpfile);
	draw_line(77);

	#
	# Destination stats
	#
	print "\nDestination IP         To         From        Total     FQDN";
	draw_line(77);
	for (t in target) {
		printf "%-16s %8.1f MB  %8.1f MB  %8.1f MB  \n",
			t,
			to_target[t]/MEG,
			from_target[t]/MEG,
			(from_target[t]+to_target[t])/MEG >> tmpfile;
	}
	close(tmpfile);
	sorted_output(tmpfile,5,SHOW_DESTINATIONS,LOOKUP_IPS);
	rmfile(tmpfile);
	draw_line(77);
	printf "\n";
}
