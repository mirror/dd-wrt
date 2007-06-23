#!/usr/bin/perl -w

# Usage: ./enum2debug.pl isakmp.h  >vpnc-debug.c 2>vpnc-debug.h

use strict;

my $in_enum = 0;
my $element;
my $arrayname;

print STDERR << 'EOF';
/* Automatically generated with enum2debug.pl: Don't edit! */

struct debug_strings {
	unsigned int id;
	const char *string;
};

extern const char *val_to_string(unsigned int, const struct debug_strings *);

EOF

print << 'EOF';
/* Automatically generated with enum2debug.pl: Don't edit! */

#include <stdio.h>

#include "vpnc-debug.h"
#include "isakmp.h"

const char *val_to_string(unsigned int val, const struct debug_strings *dstrings)
{
	static const char *unknown = " (unknown)";
	static const char *na = "";
	unsigned int i;
	
	if (dstrings == NULL)
		return na;
	
	for (i = 0; dstrings[i].id != 0 || dstrings[i].string != NULL; i++)
		if (dstrings[i].id == val)
			return dstrings[i].string;
	return unknown;
}

EOF

while (<>) {
	if (/^enum\W+(\w+)\W*/) {
		print STDERR "extern const struct debug_strings $1_array[];\n";
		print "const struct debug_strings $1_array[] = {\n";
		$in_enum = 1;
	} elsif ($in_enum && /^}/) {
		print "\t{ 0,\t(const char *) 0 }\n};\n\n";
		$in_enum = 0;
	} elsif ($in_enum && /^\W*(\w+)\W*/) {
		print "\t{ $1,\t\" ($1)\" },\n";
	}
}

exit 0;

__END__
