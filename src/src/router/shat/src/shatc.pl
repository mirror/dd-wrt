#! /usr/bin/perl -w
#
# Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#
# Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
#
# $Id: shatc.pl,v 1.4 2005/03/06 16:58:42 jordan Exp $
#
#
# simple stupid example of a command line client

use strict;
use warnings;
use Socket;
use File::Basename;
use Getopt::Long qw(:config gnu_getopt);
use vars qw ($ROOT $HELP);

my $CTRL_REQUEST_PATH = "/var/run/shat/server";
my $REPLY_SOCKET_TMPL = "/tmp/replyto-%s-%x" ;

my $self = basename $0 ;

sub usage {
    print STDERR
        "Usage: $self [options] <cmd> [<args> ...]\n\n",
        "Options: -r, --root=<PREFIX> server runs in a chroot environment\n",
        "         -h, --help          print this message\n";
    exit 2;
}

sub tmo_exit { 
    print STDERR "TIMEOUT\n";
    exit 2;
}

usage unless GetOptions ('root|r=s' => \$ROOT,
                         'help|h'   => \$HELP);
usage if $HELP || ! @ARGV;
$ROOT ||= "" ;

# set up the communication socket
socket FD, PF_UNIX, SOCK_DGRAM, 0 or
    die "Cannot create Unix Domain Socket: $!";

# set up for accepting the reply
my $reply = sprintf "$REPLY_SOCKET_TMPL", $self, $$;
my $un_path = ($ROOT || '') . $reply;
mkdir dirname ($un_path), 0700 ;
unlink $un_path;
bind FD, sockaddr_un $un_path or
    die "Cannot bind socket to $un_path: $!";

# send the request
send FD, "$reply @ARGV", 0, sockaddr_un $CTRL_REQUEST_PATH or
    die "Problems sending a request: $!";

# receive the answer
my $buf ;
eval {
    local $SIG {ALRM} = \&tmo_exit ;
    alarm 20;
    recv FD, $buf, 2000, 0 or
        die "Problems receiving the response: $!";
    alarm 0;
};
if ($@) {
    unlink $reply ;
    print STDERR $@ ;
    exit 2;
}

# display the answer
print "$buf\n" ;
unlink $reply ;

# END
