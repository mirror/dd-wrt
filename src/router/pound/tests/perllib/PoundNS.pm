# This file is part of pound testsuite
# Copyright (C) 2024-2025 Sergey Poznyakoff
# Copyright (C) 2007 Dick Franks, Olaf Kolkman, Michael Fuhr
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.

# Large part of this file is derived from Net::DNS::Nameserver

package PoundNS;
use strict;
use warnings;
use Carp;
use integer;
use IO::Socket::IP;
use File::stat;
use Net::DNS;
use Net::DNS::ZoneFile;

use IO::Select;
use IO::Socket::IP 0.38;
use IO::Socket;
use Socket;

use PoundSub;

use constant {
    PROTO_TCP => 6,
    PROTO_UDP => 17
};

#------------------------------------------------------------------------------
# Constructor.
#------------------------------------------------------------------------------

sub new {
    my ($class, %args) = @_;

    my $self = bless {
	ZoneFile => $args{ZoneFile} // 'fakedns.zone',
	ZoneTime => 0,
	Verbose => $args{Verbose},
	Timeout => $args{Timeout} // 2,
	Truncate => $args{Truncate} // 1
    }, $class;

    if ($args{Verbose}) {
	if (ref($args{Verbose}) eq 'CODE') {
	    $self->{Verbose} = $args{Verbose}
	} else {
	    $self->{Verbose} = sub {
		print "# PoundNS: ", @_, "\n";
	    }
	}
    }

    unless (-e $self->{ZoneFile}) {
	my $zone = <<\EOT
$ORIGIN example.org.
@   IN SOA  mname rname 1 2h 1h 2w 1h
EOT
	    ;
	$self->ZoneUpdate($zone);
    }

    $self->ReadZoneFile($self->{ZoneFile});
    return $self;
}

#------------------------------------------------------------------------------
# Start DNS server
#------------------------------------------------------------------------------

sub start_server {
    my ($self, $ip) = @_;
    $ip //= '127.0.0.1';

    my $tcp_socket = IO::Socket::IP->new(
	LocalAddr => $ip,
	Proto	  => "tcp",
	Listen	  => SOMAXCONN,
	Type	  => SOCK_STREAM)
	or croak "tcp socket: $!";
    my $udp_socket = IO::Socket::IP->new(
	LocalAddr => $ip,
	Proto	  => "udp",
	Type	  => SOCK_DGRAM)
	or croak "udp socket: $!";

    PoundSub->start(sub {
	$self->DNS_server($udp_socket, $tcp_socket)
    });
    return ($udp_socket->sockport, $tcp_socket->sockport);
}


#------------------------------------------------------------------------------
# Update the zone file.
#------------------------------------------------------------------------------
sub ZoneUpdate {
    my $self = shift;
    my $text = join("\n", @_);
    open(my $fh, '>', $self->{ZoneFile})
	or croak "can't open $self->{ZoneFile}: $!";
    print $fh $text;
    close $fh;
}

#------------------------------------------------------------------------------
# ReadZoneFile - Read zone file used by default reply handler
#------------------------------------------------------------------------------

sub ReadZoneFile {
    my ($self, $file) = @_;
    my $zonefile = Net::DNS::ZoneFile->new($file);

    my $RRhash = $self->{index} = {};
    my $RRlist = [];
    my @zonelist;
    while (my $rr = $zonefile->read) {
	push @{$RRhash->{lc $rr->owner}}, $rr;
	# Warning: Nasty trick abusing SOA to reference zone RR list
	if ( $rr->type eq 'SOA' ) {
	    $RRlist = $rr->{RRlist} = [];
	    push @zonelist, lc $rr->owner;
	} else {
	    push @$RRlist, $rr;
	}
    }

    $self->{namelist}     = [sort { length($b) <=> length($a) } keys %$RRhash];
    $self->{zonelist}     = [sort { length($b) <=> length($a) } @zonelist];
    return;
}

#------------------------------------------------------------------------------
# DNS server
#------------------------------------------------------------------------------
sub DNS_server {
    my $self = shift;
    my $select = IO::Select->new(@_);

    while (1) {
	local $! = 0;
	scalar(my @ready = $select->can_read($self->{Timeout})) or do {
	    redo if $!{EINTR};	## retry if aborted by signal
	    last if $!;
	};

	foreach my $socket (@ready) {
	    my $proto = $socket->protocol;
	    if ($proto == PROTO_TCP) {
		if (grep { $_ == $socket } @_) {
		    $select->add($socket->accept);
		} else {
		    if (my $buffer = read_tcp($socket, $self->{Verbose})) {
			PoundSub->start(sub {
			    $self->TCP_connection($socket, $buffer)
			});
		    } else {
			close($socket);
			$select->remove($socket);
		    }
		}
	    } else {
		my $buffer = read_udp($socket, $self->{Verbose});
		PoundSub->start(sub {
		    $self->UDP_connection($socket, $buffer)
		});
	    }
	}
    }
}

#------------------------------------------------------------------------------
# ReplyHandler - Default reply handler serving RRs from zone file
#------------------------------------------------------------------------------

sub ReplyHandler {
    my ($self, $qname, $qclass, $qtype, $peerhost, $query, $conn) = @_;
    my $rcode;
    my %headermask;
    my @ans;
    my @auth;

    my $st = stat($self->{ZoneFile})
	or croak "can't stat $self->{ZoneFile}: $!";
    if ($st->mtime > $self->{ZoneTime}) {
	$self->ReadZoneFile($self->{ZoneFile});
	$self->{ZoneTime} = $st->mtime;
    }

    my $RRhash = $self->{index};

    if ($qtype eq 'AXFR') {
	my $RRlist = $RRhash->{lc $qname} || [];
	my ($soa) = grep { $_->type eq 'SOA' } @$RRlist;
	if ($soa) {
	    $rcode = 'NOERROR';
	    push @ans, $soa, @{$soa->{RRlist}}, $soa;
	} else {
	    $rcode = 'NOTAUTH';
	}
	return ($rcode, \@ans, [], [], {}, {});
    }

    my @RRname = @{$self->{namelist}};  # pre-sorted, longest first
    my $RRlist = $RRhash->{lc $qname} || [];
    my @match  = @$RRlist;		# assume $qclass always 'IN'
    if (scalar(@match)) {		# exact match
	$rcode = 'NOERROR';
    } elsif (grep {/\.$qname$/i} @RRname) {	# empty non-terminal
	$rcode = 'NOERROR';	  # [NODATA]
    } else {
	$rcode = 'NXDOMAIN';
	foreach (grep {/^[*][.]/} @RRname) {
	    my $wildcard = $_; # match wildcard per RFC4592
	    s/^\*//;	   # delete leading asterisk
	    s/([.?*+])/\\$1/g;
	    next unless $qname =~ /[.]?([^.]+$_)$/i;
	    my $encloser = $1;
	    $rcode = 'NOERROR';
	    last if grep {/(^|\.)$encloser$/i} @RRname;

	    # synthesise RR at qname
	    my ($q) = $query->question;
	    foreach my $rr (@{$RRhash->{$wildcard}}) {
		my $clone = bless({%$rr}, ref($rr));
		$clone->{owner} = $q->{qname};
		push @match, $clone;
	    }
	    last;
	}
    }
    push @ans, my @cname = grep { $_->type eq 'CNAME' } @match;
    $qname = $_->cname for @cname;
    redo if @cname;
    push @ans, @match if $qtype eq 'ANY';
    push @ans, grep { $_->type eq $qtype } @match;
    unless (@ans) {
	foreach ( @{$self->{zonelist}} ) {
	    s/([.?*+])/\\$1/g;
	    next unless $qname =~ /[^.]+[.]$_[.]?$/i;
	    push @auth,
		grep {
		    $_->type eq 'SOA'
	    } @{$RRhash->{$_}};
	    last;
	}
    }
    $headermask{aa} = 1;
    return ($rcode, \@ans, \@auth, [], \%headermask, {});
}

#------------------------------------------------------------------------------
# make_reply - Make a reply packet.
#------------------------------------------------------------------------------

sub make_reply {
    my ($self, $query, $sock) = @_;
    my $verbose = $self->{Verbose};

    unless ($query) {
	my $empty = Net::DNS::Packet->new();
	my $reply = $empty->reply();
	$reply->header->rcode("FORMERR");
	return $reply;
    }

    if ($query->header->qr()) {
	&{$verbose}("ERROR: invalid packet (qr set), dropping") if $verbose;
	return;
    }

    my $reply  = $query->reply();
    my $header = $reply->header;
    my $headermask;
    my $optionmask;

    my $opcode  = $query->header->opcode;
    my $qdcount = $query->header->qdcount;

    unless ($qdcount) {
	$header->rcode("NOERROR");

    } elsif ($qdcount > 1) {
	$header->rcode("FORMERR");

    } else {
	my ($qr)   = $query->question;
	my $qname  = $qr->qname;
	my $qtype  = $qr->qtype;
	my $qclass = $qr->qclass;

	&{$verbose}($qr->string) if $verbose;

	my $conn = {
	    peerhost => my $peer = $sock->peerhost,
	    peerport => $sock->peerport,
	    protocol => $sock->protocol,
	    sockhost => $sock->sockhost,
	    sockport => $sock->sockport
	};

	my ($rcode, $ans, $auth, $add);
	my @arglist = ($qname, $qclass, $qtype, $peer, $query, $conn);

	if ($opcode eq "QUERY") {
	    ($rcode, $ans, $auth, $add, $headermask, $optionmask) =
		$self->ReplyHandler(@arglist);

	} elsif ($opcode eq "NOTIFY") {		#RFC1996
	    if (ref $self->{NotifyHandler} eq "CODE") {
		($rcode, $ans, $auth, $add,
		 $headermask, $optionmask) =
		    &{$self->{NotifyHandler}}(@arglist);
	    } else {
		$rcode = "NOTIMP";
	    }
	} elsif ($opcode eq "UPDATE") {		#RFC2136
	    if (ref $self->{UpdateHandler} eq "CODE") {
		($rcode, $ans, $auth, $add,
		 $headermask, $optionmask) =
		    &{$self->{UpdateHandler}}(@arglist);
	    } else {
		$rcode = "NOTIMP";
	    }
	} else {
	    &{$verbose}("ERROR: opcode $opcode unsupported") if $verbose;
	    $rcode = "FORMERR";
	}

	if (!defined($rcode)) {
	    &{$verbose}("remaining silent") if $verbose;
	    return;
	}

	$header->rcode($rcode);

	push @{$reply->{answer}},     @$ans  if $ans;
	push @{$reply->{authority}},  @$auth if $auth;
	push @{$reply->{additional}}, @$add  if $add;
    }

    while (my ($key, $value) = each %{$headermask || {}}) {
	$header->$key($value);
    }

    while (my ($option, $value ) = each %{$optionmask || {}}) {
	$reply->edns->option( $option, $value );
    }

    &{$verbose}($header->string) if $verbose && ($headermask || $optionmask);

    return $reply;
}

#------------------------------------------------------------------------------
# TCP_connection - Handle a TCP connection.
#------------------------------------------------------------------------------

sub TCP_connection {
    my ($self, $socket, $buffer) = @_;
    my $verbose = $self->{Verbose};

    my $query = Net::DNS::Packet->new(\$buffer);
    if ($@) {
	&{$verbose}("Error decoding query packet: $@") if $verbose;
	undef $query;		## force FORMERR reply
    }

    my $reply = $self->make_reply($query, $socket);
    die 'Failed to create reply' unless defined $reply;

    my $segment = $reply->data;
    my $length  = length $segment;
    my $res     = $socket->send(pack 'na*', $length, $segment);
    &{$verbose}("TCP response (2 + $length octets) - ",
		$res  ? "sent" : "failed: $!") if $verbose;
    return;
}

sub read_tcp {
    my ($socket, $verbose) = @_;

    my $l = '';
    local $! = 0;
    my $n = sysread($socket, $l, 2);
    unless (defined $n) {
	redo if $!{EINTR};	## retry if aborted by signal
	die "sysread: $!";
    }
    return if $n == 0;
    my $msglen = unpack 'n', $l;

    my $buffer = '';
    while ($msglen > (my $l = length $buffer)) {
	local $! = 0;
	my $n = sysread($socket, $buffer, ($msglen - $l), $l);
	unless (defined $n) {
	    redo if $!{EINTR};	## retry if aborted by signal
	    die "sysread: $!";
	}
	last if $n == 0;	## client closed (or lied) per RT#151240
    }

    if ($verbose) {
	my $peer = $socket->peerhost;
	my $port = $socket->peerport;
	my $size = length $buffer;
	&{$verbose}("Received $size octets from [$peer] port $port");
    }
    return $buffer;
}

#------------------------------------------------------------------------------
# UDP_connection - Handle a UDP connection.
#------------------------------------------------------------------------------

sub UDP_connection {
    my ($self, $socket, $buffer) = @_;
    my $verbose = $self->{Verbose};

    my $query = Net::DNS::Packet->new(\$buffer);
    if ($@) {
	&{$verbose}("Error decoding query packet: $@") if $verbose;
	undef $query;		## force FORMERR reply
    }

    my $reply = $self->make_reply($query, $socket);
    croak 'Failed to create reply' unless defined $reply;

    my @UDPsize = ($query && $self->{Truncate})
		     ? $query->edns->UDPsize || 512 : ();
    my $response = $reply->data(@UDPsize);
    my $res      = $socket->send($response);
    if ($verbose) {
	&{$verbose}('UDP response (', length($response), ' octets) - ',
		    $res ? "sent" : "failed: $!");
    }
    return;
}

sub read_udp {
    my ($socket, $verbose) = @_;
    my $buffer = '';
    $socket->recv($buffer, 9000); ## payload limit for Ethernet "Jumbo" packet
    if ($verbose) {
	my $peer = $socket->peerhost;
	my $port = $socket->peerport;
	my $size = length $buffer;
	&{$verbose}("Received $size octets from [$peer] port $port");
    }
    return $buffer;
}

1;
