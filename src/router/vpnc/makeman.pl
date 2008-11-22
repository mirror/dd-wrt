#! /usr/bin/perl -w

# $Id: makeman.pl 312 2008-06-15 18:09:42Z Joerg Mayer $

# Written by Wolfram Sang (wolfram@the-dreams.de) in 2007,
# some inspiration from help2man by Brendan O'Dea and from Perl::Critic

# Generate the vpnc-manpage from a template and the --long-help-output.
# Version 0.2

# Command-line options: none
# Files needed        : ./vpnc ./vpnc.8.template ./VERSION
# Files created       : ./vpnc.8
# Exit status         : errno-values or 255 (Magic string not found)

# Distributed under the same licence as vpnc.

use strict;
use Fatal    qw(open close);
use filetest qw(access);	# to always get errno-values on filetests
use POSIX    qw(strftime setlocale LC_ALL);

my $vpnc = './vpnc';
-e $vpnc or die "$0: Can't find $vpnc. Did you compile it?\n";
-x $vpnc or die "$0: Can't execute $vpnc. Please check permissions.\n";

# The code converting the help-output to manpage format is lots of
# regex-fiddling, sorry. It got a bit more complicated by additionally
# indenting lists (those originally starting with an asterisk). I hope
# this pays off when converting the manpage to HTML or such.

open my $LONGHELP, '-|', "$vpnc --long-help";
my $vpnc_options    = '';
my $relative_indent = 0;
my $indent_needed   = 0;

while (<$LONGHELP>) {
    if (/^  /) {

	# Check if additional indent needs to be finished by comparing the
	# amount of spaces at the beginning. A bit ugly, but I don't see a
	# better way to do it.
	if ($relative_indent) {
	    /^( *)/;
	    if (length($1) < $relative_indent) {
		$vpnc_options .= ".RE\n";
		$relative_indent = 0;
		$indent_needed = 1;
	    }
	}
	
	# Highlight the option and make an optional argument italic.
	if (s/^ *(--[\w-]+)/\n.TP\n.BI "$1"/) {
	    s/(<.+>)/ " $1"/;
	}
	
	# Highlight conffile-only options.
	s/^ *(\(configfile only option\))/\n.TP\n.B $1/;

	# Position the Default-string
	s/^ *(Default:)/.IP\n$1/;

	# Highlight the conf-variable and make an optional argument italic.
	if (s/^ *(conf-variable:) (.+?) ?([<\n])/.P\n$1\n.BI "$2"$3/) {
	    s/(<.+>)/ " $1"/;
	}

	# Replace asterisk with bulletin; indent if needed.
	if (s/^( +)\* /.IP \\(bu\n/) {
	    if (not $relative_indent) {
		$vpnc_options .= ".RS\n";
	        $relative_indent = length $1;
	    }
	}

	# Do we need to add an .IP-command after .RE or is there already one?
	if ($indent_needed and not /^\n?\.[TI]?P/) {
	    $vpnc_options .= ".IP\n";
	    $indent_needed = 0;
	}
	
	# Finalize string and add it to buffer
        s/^ *//;
	s/ *$//;
	s/-/\\-/g;
        $vpnc_options .= $_;
    }
}
close $LONGHELP;

# Hopefully the code speaks for itself from now on...

setlocale( LC_ALL, 'C' );
my $date = strftime( '%B %Y', localtime );

open my $VERSION, '<', './VERSION';
my $vpnc_version = <$VERSION>;
close $VERSION;
chomp $vpnc_version;

open my $TEMPLATE, '<', './vpnc.8.template';
open my $MANPAGE , '>', './vpnc.8';
my $magic_found;
my $MAGIC_FOR_HEADER  = qq(.\\" ###makeman.pl: Replace header here!\n);
my $MAGIC_FOR_OPTIONS = qq(.\\" ###makeman.pl: Insert options from help-output here!\n);

# Skip the template-header
while (<$TEMPLATE>) {
    last if ($magic_found = ($_ eq $MAGIC_FOR_HEADER));
}
die "$0: Missing magic: $MAGIC_FOR_HEADER" if not $magic_found;

print {$MANPAGE} <<"END_MANPAGE_HEADER";
.\\" This manpage is generated!
.\\" Please edit the template-file in the source-distribution only.
.TH VPNC "8" "$date" "vpnc version $vpnc_version" "System Administration Utilities"
END_MANPAGE_HEADER

$magic_found = 0;

while (<$TEMPLATE>) {
    if ($_ ne $MAGIC_FOR_OPTIONS) {
	print {$MANPAGE} $_;
    } else {
	print {$MANPAGE} $vpnc_options;
	$magic_found = 1;
    }
}
die "$0: Missing magic: $MAGIC_FOR_OPTIONS" if not $magic_found;

close $TEMPLATE;
close $MANPAGE;
