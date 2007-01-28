#******************************************************************************
# $Id: mk-string-arrays.pl,v 1.5 2007/01/19 00:19:26 hno Exp $
#
# File:		mk-strs.pl
#
# Author:	Max Okumoto <okumoto@ucsd.edu>
#
# Abstract:	This perl script parses enums and builds an array of
#		printable strings.
#
# Warning:	The parser is very simplistic, and will prob not work for
#		things other than squid.
#******************************************************************************

$pat{'err_type'} = "err_type_str";
$pat{'icp_opcode'} = "icp_opcode_str";
$pat{'swap_log_op'} = "swap_log_op_str";
$pat{'lookup_t'} = "lookup_t_str";
$pat{'log_type'} = "log_tags";

print "#include \"squid.h\"\n";

$state = 0;	# start state
while (<>) {
	if ($state == 0) {
		# Looking for start of typedef
		if (/^typedef enum /) {
			$count = 0;	# enum index
			$state = 1;
		}
		next;

	} elsif ($state == 1) {
		# Looking for end of typedef
		if (/^} /) {
			($b, $t) = split(/[ \t;]/, $_);
			if (defined($pat{$t})) {
				print "const char *$pat{$t}\[\] = \n";
				print "{\n";
				for ($i = 0; $i < $count; $i++) {
					if ($ea[$i] =~ /^#/) {
						print $ea[$i];
					} else {
						printf "\t\"%s\"%s\n",
							$ea[$i],
							$i == $count - 1 ? '' : ',';
					}
				}
				print "};\n";
				print "\n";
			}
			$state = 0;
		} elsif (/^#/) {
			$ea[$count++] = $_;
		} else {
			($e) = split(' ', $_);
			$e =~ s/,//;
			$e =~ s/^LOG_TAG_//;
			$e =~ s/^LOG_//;
			$e =~ s/^REFRESH_FAIL_HIT$/REF_FAIL_HIT/;
			$ea[$count++] = $e;
		}
		next;
	}
}

exit 0;
