#!/usr/bin/perl -w
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
# $Id: shatooc.pl,v 1.1 2005/03/06 16:58:42 jordan Exp $
#
#
# simple stupid example of a command line client -- oo version

package ShatComm;

use strict;
use warnings;
use IO::Socket::UNIX;
use File::Basename;


sub new {
    my ($type, %args) = @_ ;

    # set up for accepting the reply
    $args {SocketPath} = ($args {RootDirPfx} || '') . $args {ReplyPath};
    mkdir dirname ($args {SocketPath}), 0700 ;
    unlink $args {SocketPath};

    # set up the communication socket
    $args {SocketFD} = new IO::Socket::UNIX
        Type  => SOCK_DGRAM,
        Local => $args {SocketPath}
        or die "Cannot create Unix Domain Socket: $!";

    bless \%args, ref $type || $type;
}

DESTROY {
    unlink shift->{SocketPath};
}

sub send_request {
    my $self = shift ;

    # send the request
    send
        $self->{SocketFD}, "$self->{ReplyPath} @_", 0,
        sockaddr_un $self->{RequestPath} ;
}


sub recv_response {
    my $self = shift ;

    my $in = '';
    vec ($in, fileno ($self->{SocketFD}), 1) = 1 ;

    if ($self->{OK} = select $in, undef, undef, $self->{Timeout}) {
        
        # read the response
        my $buf;

        recv $self->{SocketFD}, $buf, 2000, 0 ;

        # pointer to the read buffer
        return \$buf ;
    }

    undef ;
}

sub timeout {
    defined shift->{OK} ;
}

# ---------------------------------------------------------------------------

package main;

use strict;
use warnings;
use File::Basename;
use Getopt::Long qw(:config gnu_getopt);
use vars qw ($ROOT $HELP);

my $CTRL_REQUEST_PATH = "/var/run/shat/server";
my $REPLY_SOCKET_TMPL = "/tmp/replyto-%s-%x" ;
my $me = basename $0 ;

sub usage {
    print STDERR
        "Usage: $me [options] <cmd> [<args> ...]\n\n",
        "Options: -r, --root=<PREFIX> server runs in a chroot environment\n",
        "         -h, --help          print this message\n";
    exit 2;
}

usage unless GetOptions ('root|r=s' => \$ROOT,
                         'help|h'   => \$HELP);
usage if $HELP || ! @ARGV;

# set up the communication socket
my $comm = new ShatComm
    RootDirPfx  => $ROOT,
    Timeout     => 10,
    RequestPath => $CTRL_REQUEST_PATH,
    ReplyPath   => sprintf $REPLY_SOCKET_TMPL, $me, $$;

# send the request
$comm->send_request (@ARGV)
    or die "Problems sending a request: $!";

# receive the answer
my $mesg = $comm->recv_response;

unless ($mesg) {
    die "Problems receiving the response: $!"
        unless $comm->timeout ;

    print STDERR "TIMEOUT\n";
    exit 2;
}

print "$$mesg\n" ;
exit 0;

# End


