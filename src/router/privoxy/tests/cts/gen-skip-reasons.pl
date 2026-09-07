#!/usr/bin/perl

################################################################################
#
# gen-skip-reasons.pl
#
# Generates an exclude file that can be passed to runtests.pl to skip certain
# tests that aren't expected to work when run through Privoxy.
#
# Copyright (c) 2012-2026 Fabian Keil <fk@fabiankeil.de>
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

use warnings;
use strict;

sub main() {
    my %related_tests;

    for my $testnum (1..3500) {
        my $why;
        if ($testnum == 1) {
            $why = "Expected to fail. Multiple values in Connection header that get removed by Privoxy.";
        } elsif ($testnum == 2) {
            $why = "Expected to fail. Missing space after status code that gets added by Privoxy.";
        } elsif ($testnum == 8) {
            $why = "Expected to fail. Tab in cookie that Privoxy converts to a space which the test does not expect.";
        } elsif ($testnum == 16 or
            $testnum == 45 or
            $testnum == 63) {
            $why = "Not supposed to work with Privoxy. Expected forwarding failure. Workaround probably possible.";
        } elsif ($testnum == 17) {
            $why = "Not supposed to work with Privoxy. Invalid METHODs get rejected.";
        } elsif ($testnum == 19 or
                 $testnum == 20) {
            $why = "Not supposed to work with Privoxy. Tests behaviour with non-existing server and doesn't deal with error messages coming from a proxy.";
        } elsif ($testnum == 30 or
                 $testnum == 37 or
                 $testnum == 66 or
                 $testnum == 1079) {
            $why = "Expected to fail with Privoxy. In case of missing server headers Privoxy generates an error message the test doesn't expect.";
        } elsif ($testnum == 31 or $testnum == 1105 or $testnum == 1160) {
            $why = "Expected to fail. Privoxy normalizes white-space in a cookie.";
        } elsif ($testnum == 34 or
                 $testnum == 457 or
                 $testnum == 1417 or
                 $testnum == 1493) {
            $why = "Expected to fail with Privoxy. Test contains non-standard 'Connection: mooo' header that Privoxy replaces with 'Connection: close'.";
        } elsif ($testnum == 46) {
            $why = "Invalid URL and use of --resolv.";
        } elsif ($testnum == 58) {
            $why = "Expected to fail when using Privoxy. Doesn't appear to be obviously Privoxy's fault.";
        } elsif ($testnum == 59) {
            $why = "Invalid URL gets rejected by Privoxy which the test can't handle.";
        } elsif ($testnum == 129) {
            $why = "Invalid HTTP version. Privoxy downgrades it to 1.1.";
        } elsif ($testnum == 187) {
            $why = "Expected to fail. Test doesn't deal with Privoxy's error message in case of invalid URLs.";
        } elsif ($testnum == 207) {
             $why = "Expected to fail. Test doesn't handle Privoxy's error message. Privoxy doesn't behave correctly, though.";
        } elsif ($testnum == 260) {
            $why = "Known to fail. Looks like a curl bug. The URL passed to Privoxy is invalid but the test expect a valid one when not using a proxy";
        } elsif ($testnum == 262) {
            $why = "Not supposed to work with Privoxy. Privoxy doesn't support nul bytes in headers and neither does the spec.";
        } elsif ($testnum == 266 or $testnum == 1116 or $testnum == 1540) {
            $why = "Known to fail. Uses chunk trailers which Privoxy currently doesn't support.";
        } elsif ($testnum == 309) {
            $why = "Known to fail. Uses https and test does not expect the 'Connection established' response from Privoxy";
        } elsif ($testnum == 339 or $testnum == 347 or $testnum == 1482 or $testnum == 1483 or $testnum == 1591) {
            $why = "Chunked transfer with trailers which Privoxy does not understand. Needs investigating.";
        } elsif ($testnum == 389) {
            $why = "Known to fail depending on the DNS resolver on the system as Privoxy does not implement RFC6761 internally.";
        } elsif ($testnum == 1052) {
            $why = "Expected to fail. Connection header expected in one response but not in the other. Not yet covered by runtests.pl's proxy mode.";
        } elsif ($testnum == 1118) {
            $why = "Expected to fail. Looks like a curl bug although Privoxy's behaviour seems subobtimal as well.";
        } elsif ($testnum == 1310) {
            $why = "Known to fail. NTLM-related. Cause not properly diagnosed yet. Privoxy's behaviour seems reasonable.";
        } elsif ($testnum == 155) {
            $why = "Known to fail. Not yet analyzed.";
        } elsif ($testnum == 158 or $testnum == 246 or $testnum == 565 or $testnum == 579) {
            $why = "Known to fail. Not properly analyzed. Looks like Privoxy's continue hack is insufficient.";
        } elsif ($testnum == 412 or $testnum == 413) {
            $why = "Known to fail as curl is tunneling the request even though it's vanilla HTTP.";
        } elsif ($testnum == 415) {
            $why = "Known to fail. Control code in Content-Length header.";
        } elsif ($testnum == 435) {
            $why = "Expected to fail. Uses %{remote_port} and expects the port of the server and not the one from Privoxy.";
        } elsif ($testnum == 439) {
            $why = "Expected to fail. Privoxy adds a 'Connection established' response the test does not expect.";
        } elsif ($testnum == 461) {
            $why = "Expected to fail. Privoxy adds a 'Host' header.";
        } elsif ($testnum == 471 or $testnum == 1704) {
            $why = "Expected to fail. Uses HTTP/2 which Privoxy does not support.";
        } elsif ($testnum == 500) {
            $why = "Known to fail when curl has been build with TrackMemory suppport. Proxy use results in 'TOO MANY ALLOCS'.";
        } elsif ($testnum == 507) {
            $why = "Expected to fail. DNS failures cause a Privoxy error message the test doesn't handle.";
        } elsif ($testnum == 501) {
            $why = "Not relevant for a proxy.";
        } elsif ($testnum == 530 or
                 $testnum == 584) {
            $why = "Known to fail. Test server expects pipelined requests and doesn't respond otherwise.";
        } elsif ($testnum == 556) {
            $why = "Expected to fail. Uses HTTP/1.2 which Privoxy rejects as invalid.";
        } elsif ($testnum == 581) {
            $why = "Expected to fail. Privoxy removes second Content-Type header.";
        } elsif ($testnum == 587 or $testnum == 644) {
            $why = "Expected to fail. POST request doesn't make it to the server. Needs investigating.";
        } elsif ($testnum == 655) {
            $why = "Expected to fail. Uses tool. Failure reason not yet analyzed";
        } elsif ($testnum == 696) {
            $why = "Expected to fail. Test uses tool that does not support proxies.";
        } elsif ($testnum == 970 or $testnum == 972) {
            $why = "Expected to fail. Privoxy adds a Proxy-Connection header which results in a modified num_headers value in the JSON output";
        } elsif ($testnum == 1074) {
            $why = "Expected to fail. Privoxy doesn't downgrade the forwarded request and doesn't have ".
                   "to as long as the client is treated like a HTTP/1.0 client. Needs double-checking.";
        } elsif ($testnum == 1144) {
            $why = "Expected to fail. Server response is invalid and results in 502 message from Privoxy";
        } elsif ($testnum == 1147) {
            $why = "Expected to fail. Privoxy merges a two-line cookie into a one line cookie.";
        } elsif ($testnum == 1151) {
            $why = "Expected to fail. Large cookies that don't make it to the cookie file. Needs investigating.";
        } elsif ($testnum == 1188) {
            $why = "Expected to fail. Relies on a connection failure which results in a Privoxy error message the test does not expect";
        } elsif ($testnum == 1223) {
            $why = "Expected to fail. Tests remote address which doesn't work with proxies.";
        } elsif ($testnum == 1274) {
            $why = "Expected to fail. Privoxy unfolds the folded headers which the test does not expect.";
        } elsif ($testnum == 1433) {
            $why = "Expected to fail. Privoxy will enforce a valid HTTP version number";
        } elsif ($testnum == 1479) {
            $why = "Expected to fail. Uses HTTP/0.9 response.";
        } elsif ($testnum == 1480) {
            $why = "Expected to fail. Uses HTTP Continue response but no final response.";
        } elsif ($testnum == 1506 or $testnum == 1510) {
            $why = "Expected to fail when using a proxy. Hardcoded addresses in expected output.";
        } elsif ($testnum == 1156) {
            $why = "Expected to fail as it relies on Range requests making it to the server.";
        } elsif ($testnum == 1164 or $testnum == 1172 or $testnum == 1174) {
            $why = "Expected to fail as Privoxy does not support HTTP/0.9.";
        } elsif ($testnum == 1292) {
            $why = "Expected to fail as Privoxy replaces the empty Host header.";
        } elsif ($testnum == 473 or $testnum == 1499) {
            $why = "Expected to fail. Privoxy does not support chunked trailers.";
        } elsif ($testnum == 1533) {
            $why = "Sends an invalid method. Needs investigating.";
        } elsif ($testnum == 1542) {
            $why = "Expected to fail. Expects HTTPPORT in logs which gets replaced by Privoxy's listening port.";
        } elsif ($testnum == 1543) {
            $why = "Expected to fail as the URL contains spaces. XXX: Looks like a curl bug that should be investigated.";
        } elsif ($testnum == 1556) {
            $why = "Known to fail. Body value changes from 100008 to 100009. Needs investigating.";
        } elsif ($testnum == 1598) {
            $why = "Expected to fail. Privoxy does not support chunk-encoded POST requests with trailers.";
        } elsif ($testnum == 1665) {
            $why = "Expected to fail. Trigger Privoxy's MS IIS 5 hack which the test does not expect.";
        } elsif ($testnum == 1671) {
            $why = "Known to fail as curl adds a Proxy-Connection header to the JSON output which the test doesn't expect.";
        } elsif ($testnum == 1685) {
            $why = "Known to fail as Privoxy converts a tab in a Cookie header to space.";
        } elsif ($testnum == 1915) {
            $why = "Known to fail. Uses tool that doesn't expect a proxy.";
        } elsif ($testnum == 1933) {
            $why = "Known to fail. Modified signature in Authorization header. Needs investigating";
        } elsif ($testnum == 2032 or $testnum == 2033) {
            $why = "Known to fail due to a limitation of the test which doesn't properly deal with interleaved output from two parallel connections";
        } elsif ($testnum == 2049 or $testnum == 2052 or $testnum == 2053 or $testnum == 2054) {
            $why = "Uses --connect-to. Need investigating.";
        } elsif ($testnum == 2082 or $testnum == 2084 or $testnum == 2085) {
            $why = "Known to fail. Uses %HTTPPORT and does not expect Privoxy's port but the remote one.";
        } elsif ($testnum == 96) {
            $why = "Test 96 is incomplete";
        } elsif ($testnum == 1901 or $testnum == 1902 or $testnum == 1903) {
            $why = "Known to fail due to different response orders.";
        } elsif ($testnum == 1960) {
            $why = "Known to fail. Test does not expect proxy request.";
        } elsif ($testnum == 2100) {
            $why = "Known to fail. Use DNS-over-HTTP.";
        } elsif ($testnum == 3014 or $testnum == 3015) {
            $why = "Known to fail. Curl adds a Proxy-Connection header while test expect a certain number of headers.";
        }

        next unless defined $why;
        $why =~ s/%/%%/g; # quote %, since this is used in sprintf() format string

        if (exists $related_tests{$why}) {
            $related_tests{$why} = $related_tests{$why} . ", $testnum";
        } else {
            $related_tests{$why} = "$testnum";
        }
    }

    foreach my $why (keys %related_tests) {
        printf("test:%s: %s\n", $related_tests{$why}, $why);
    }

    foreach my $protocol ('FTP', 'POP3', 'IMAP', 'SMTP', 'GOPHER', 'TELNET', 'FILE', 'RTSP') {
        # Curl's behaviour when combining --proxy, -H and telnet:// seems strange and may be a bug."
        printf("keyword:%s: Protocol %s is not supported by Privoxy.\n", $protocol, $protocol);
    }

    foreach my $protocol ('SOCKS4', 'SOCKS5') {
        # Curl's behaviour when combining --proxy, -H and telnet:// seems strange and may be a bug."
        printf("keyword:%s: Protocol %s is supported by Privoxy but the tests need additional magic before they can be used.\n", $protocol, $protocol);
    }

    foreach my $misc ('proxy', '--resolve', '--libcurl', 'CURLOPT_RESOLVE') {
        printf("keyword:%s: Tests with keyword '%s' currently don't work with Privoxy as they need additional runtests.pl changes.\n", $misc, $misc);
    }
    printf("keyword:%s: Tests with keyword '%s' aren't expected to work with Privoxy running in a jail without IPv6 connectivity.\n", 'IPv6', 'IPv6');

    printf("keyword:%s: Tests with keyword '%s' don't work with Privoxy as they use the OPTIONS method which is currently not properly supported (TODO #186).\n", '--request-target', '--request-target');
    printf("keyword:%s: Tests with keyword '%s' don't work with Privoxy.\n", 'DOH', 'DOH');

    printf("keyword:%s: Tests with keyword '%s' obviously should be skipped. Check the full keyword for details.\n", 'skip', 'skip');

    foreach my $keyword ('FAILURE', 'unsupported', 'curl-config') {
        printf("keyword:%s: Tests with keyword '%s' do not reach the proxy. Or do they?\n", $keyword, $keyword);
    }

    foreach my $tool ('lib517', 'lib543', 'lib543', '--manual', '--help', 'symbols-in-versions', 'memory-includes', 'unittest') {
        printf("tool:%s: Tests with tool '%s' are not relevant for proxies.\n", $tool, $tool);
    }

}

main();
