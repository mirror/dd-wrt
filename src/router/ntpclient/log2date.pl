# Adds human-readable UTC stamps at the end of
# ntpclient log files
while (<>) {
	if (/^ *#/) {
		print $_;
	} else {
		chomp();
		@A=split();
		# 15020: mysterious, got rid of it in ntpclient_2003
		# 25567:  Jan 1970 - Jan 1900
		$second = ($A[0]-25567)*86400+$A[1];
		$ss = gmtime($second);
		print "$_ $ss\n";
	}
}
