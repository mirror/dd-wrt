#!/usr/bin/perl
while (<>) {
    next unless /^#define\s+(E\S+)\s*/;
    printf("  { \"%s\", %s },\n",$1,$1) || die "print: $!";
}
