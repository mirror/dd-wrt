#!/usr/bin/perl
#
# The E.164 country code listing can be obtained from
# http://www.itu.ch/itudoc/itu-t/lists/tf_cc_e_*.rtf
#
# Usage of this program:
# perl rtf2e164_cc.pl <tf_cc_e_xxx.rtf >/etc/e164_cc

while (<>) {
    next unless
      /{\\fs18\\cf1\s(\d+)}{\\f7\\fs24\s\\tab\s}{\\fs18\\cf1\s([^}]+)}/;
    last if $1 == 999;
}
while (<>) {
    next unless
      /{\\fs18\\cf1\s(\d+)}{\\f7\\fs24\s\\tab\s}{\\fs18\\cf1\s([^}]+)}/;
    printf("%-3d %s\n",$1,$2) || die "printf: $!";
}
