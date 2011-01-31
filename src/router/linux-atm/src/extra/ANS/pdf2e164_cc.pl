#!/usr/bin/perl
#
# The E.164 country code listing "List of ITU-T Recommendation E.164 Assigned
# Country Codes" can be obtained from
# The International Telecommunications Union (ITU) http://www.itu.org/
# at http://www.itu.int/itudoc/itu-t/ob-lists/icc/e164_717.html
#
# Usage of this program:
# perl pdf2e164_cc.pl e164_xxx.pdf >/etc/e164_cc
#

open(PDF2TXT, "pdftotext -raw $ARGV[0] - |");

while(<PDF2TXT>) {
	next unless /^(\d+)\s+(.+)\s+/;
	last if $1 == 999;
}

while(<PDF2TXT>) {
	next unless /^(\d+)\s+(.+)\s+/;
	($country, $junk) = split(/\s{2,}/, $2, 2);
	printf("%-3d %s\n", $1, $country) || die "printf: $!";
	last if $1 == 999;
}

close(PDF2TXT);

