################################################################################
# privoxy-runtests.pm
#
# Code that has to be loaded by curl's runtests.pl with the -L option
# to deal with modifications required when using the tests with Privoxy.
#
# Copyright (c) 2014-2022 Fabian Keil <fk@fabiankeil.de>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
################################################################################

use strict;
use warnings;
no warnings "redefine";

my $verbose = 0;
my $use_external_proxy = 0;

BEGIN {
    # Keep a couple of functions from getpart.pm accessible so
    # our redefinitions don't have to reimplement them.
    our $real_showdiff = \&showdiff;
    our $real_getpart = \&getpart;
    our $real_getpartattr = \&getpartattr;
    our $real_compareparts = \&compareparts;
    our $real_startnew = \&startnew;
}

sub print_skipped_header($) {
    my $skipped_header = shift;
    $skipped_header =~ s@\r?\n$@@;
    print "Skipping '$skipped_header'\n";
}

# Process headers to ignore differences that are to be expected
# when Privoxy is being used.
#
# - Filter out "Proxy-Connection:" headers when checking for
#   test success.
# - Filter out a header that is specified with a "X-Ignore-Header" header.
# - Deal with tests that don't expect CRLF header endings as
#   long as the test uses it consistently.
# - Reduce spaces in server headers with a too-simplistic heuristic
#   that happens to work for the existing tests.
sub process_headers($$) {
    my ($head1_ref, $head2_ref) = @_;
    my @head1;
    my @head2;
    my $crlf_expected = 0;
    my $connection_header_expected = 0;
    my $proxy_connection_header_expected = 0;
    my $parsing_server_headers = 0;
    my $ignore_header;
    my $ignored_header;

    foreach (@$head2_ref) {
        if (/^HTTP/) {
            # If it starts like a response line, assume we are
            # looking at server headers.
            $parsing_server_headers = 1;
        }
        if (/^\r?\n$/) {
            $parsing_server_headers = 0;
        }
        if (/\r\n$/) {
            $crlf_expected = 1; # XXX: assume the expectancy is consistent.
        }

        if (/^Connection:/) {
            $connection_header_expected = 1;
        }
        if (/^Proxy-Connection:/) {
            $proxy_connection_header_expected = 1;
        }
        if (/^X-Ignore-Header: (.*)/) {
            $ignore_header = $1;
            print "Ignoring header '$ignore_header'\n" if $verbose;
        }
        if (defined $ignore_header and /^$ignore_header: .*/) {
            $ignored_header = $_;
        }

        if ($parsing_server_headers and not /"/) {
            # Normalize spaces in server headers similar to the way Privoxy
            # does. This is required for curl tests 29, 40, 42 and 54.
            s@  +@ @g;
        }
    }

    if ($verbose) {
        print "Expecting " . ($crlf_expected ? "" : "no ") . "crlf\n";
        print "Expecting " . ($connection_header_expected ? "a" : "no") . " Connection: header\n";
        print "Expecting " . ($proxy_connection_header_expected ? "a" : "no") . " Proxy-Connection: header\n";
    }

    foreach (@$head1_ref) {

        s@\r\n$@\n@ unless ($crlf_expected);

        if ((m/^Connection:/ and not $connection_header_expected) or
            (m/^Proxy-Connection:/ and not $proxy_connection_header_expected)) {
            print_skipped_header($_) if ($verbose);
            next;
        }
        if (defined $ignore_header) {
            if (m/^$ignore_header:/) {
                push @head1, "X-Ignore-Header: $ignore_header\n";
                $_ = $ignored_header;
            }
        }
        push @head1, $_;
    }
    $head1_ref = \@head1;

    return ($head1_ref, $head2_ref);
}

# Behaves like the real compareparts(), but if a proxy is being used,
# headers are run through process_headers() before checking them for
# differences.
sub compareparts {
    my ($head1_ref, $head2_ref) = @_;
    our $real_compareparts;

    if ($use_external_proxy) {
        ($head1_ref, $head2_ref) = process_headers($head1_ref, $head2_ref);
    }

    return &$real_compareparts($head1_ref, $head2_ref);
}

# Behaves like the real getpart() but if a proxy is being used
# and a proxy-reply section exists, it is returned instead of
# a common reply section.
sub getpart {
    my ($section, $part) = @_;
    our $real_getpart;

    if ($use_external_proxy and
        $section eq 'reply' and
        partexists("proxy-reply", $part)) {
        $section = "proxy-reply";
    }

    return &$real_getpart($section, $part);
}

# Behaves like the real getpartattr() but if a proxy is being used
# and a proxy-reply section exists, it is being used instead of
# a common reply section.
sub getpartattr {
    my ($section, $part)=@_;
    our $real_getpartattr;

    if ($use_external_proxy and
        $section eq 'reply' and
        partexists("proxy-reply", $part)) {
        $section = "proxy-reply";
    }

    return &$real_getpartattr($section, $part);
}

# Behaves like the real logmsg but suppresses warnings
# about unknown tests
sub logmsg {
    for (@_) {
        next if /^Warning: test\d+ not present in/;
        print "$_";
    }
}

# Behaves like the real showdiff() but diffs twice,
# the second time after processing the headers.
sub showdiff {
    my ($logdir, $head1_ref, $head2_ref) = @_;
    our $real_showdiff;

    print "Unprocessed headers:\n";
    print &$real_showdiff($logdir, $head1_ref, $head2_ref);

    print "Processed headers:\n";
    ($head1_ref, $head2_ref) = process_headers($head1_ref, $head2_ref);
    return &$real_showdiff($logdir, $head1_ref, $head2_ref);
}

# Behaves like the real startnew() but sets a static port if
# the started server is httpserver.pl.
sub startnew {
    my ($cmd, $pidfile, $timeout, $fake) = @_;
    our $real_startnew;

    if ($cmd =~ /httpserver\.pl/) {
        $cmd =~ s@--port 0@--port 20000@;
    } elsif ($cmd =~ m@server/socksd@) {
        $cmd =~ s@--port 0@--port 20001@;
    }

    return &$real_startnew($cmd, $pidfile, $timeout, $fake);
}

sub main() {

    # Look but don't touch, @ARGV is still needed elsewhere
    foreach my $arg (@ARGV) {
        $use_external_proxy = 1 if ($arg eq "-P");
        $verbose = 1 if ($arg eq "-v");
    }

    return 1;
}

main();
