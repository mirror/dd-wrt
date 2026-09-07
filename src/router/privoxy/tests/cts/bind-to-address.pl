#!/usr/bin/perl

################################################################################
# bind-to-address.pl
#
# Tries to bind to the IP address given as argument. Used as precheck by
# some of the ACL tests to figure out if a test should be executed.
#
# Copyright (c) 2026 Fabian Keil <fk@fabiankeil.de>
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

use Socket;
use warnings;
use strict;

sub main() {
    my $ip_address = $ARGV[0] || die "No address passed as argument!\n";
    my $port = 12345;

    socket(SOCKET,PF_INET,SOCK_STREAM,(getprotobyname('tcp'))[2]);
    unless (bind(SOCKET, pack_sockaddr_in($port, inet_aton($ip_address)))) {
        print "Failed to bind to $ip_address:$port: $!\n";
        return 1;
    }
    return 0;
}

main();
