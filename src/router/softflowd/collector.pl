#!/usr/bin/perl -w

# This is a Cisco NetFlow datagram collector

# Netflow protocol reference:
# http://www.cisco.com/en/US/products/sw/netmgtsw/ps1964/products_implementation_design_guide09186a00800d6a11.html 

# XXX Doesn't support NetFlow 9

my $af;

BEGIN {
	use strict;
	use warnings;
	use IO qw(Socket);
	use Socket;
	use Carp;
	use POSIX qw(strftime);
	use Getopt::Long;
	eval "use IO::Socket::INET6;";
	eval "use Socket6;";
}

############################################################################

sub timestamp()
{
	return strftime "%Y-%m-%dT%H:%M:%S", localtime;
}

sub fuptime($)
{
	my $t = shift;
	my $r = "";
	my $tmp;
	
	# Milliseconds
	$tmp = $t % 1000;
	$r = sprintf ".%03u%s", $tmp, $r;

	# Seconds
	$t = int($t / 1000);
	$tmp = $t % 60;
	$r = "${tmp}s${r}";

	# Minutes
	$t = int($t / 60);
	$tmp = $t % 60;
	$r = "${tmp}m${r}" if $tmp;

	# Hours
	$t = int($t / 60);
	$tmp = $t % 24;
	$r = "${tmp}h${r}" if $tmp;

	# Days
	$t = int($t / 24);
	$tmp = $t % 7;
	$r = "${tmp}d${r}" if $tmp;

	# Weeks
	$t = int($t / 7);
	$tmp = $t % 52;
	$r = "${tmp}w${r}" if $tmp;

	# Years
	$t = int($t / 52);
	$r = "${tmp}y${r}" if $tmp;

	return $r;
}

sub do_listen($$)
{
	my $port = shift
		or confess "No UDP port specified";

        my $socket;

	if ($af == 4) {
		$socket = IO::Socket::INET->new(Proto=>'udp', LocalPort=>$port)
			or croak "Couldn't open UDP socket: $!";
	} elsif ($af == 6) {
		$socket = IO::Socket::INET6->new(Proto=>'udp', LocalPort=>$port)
			or croak "Couldn't open UDP socket: $!";
 	} else {
		croak "Unsupported AF";
	}

	return $socket;
}

sub process_nf_v1($$)
{
	my $sender = shift;
	my $pkt = shift;
	my %header;
	my %flow;
	my $sender_s;

	%header = qw();

	$sender_s = inet_ntoa($sender) if $af == 4;
	$sender_s = inet_ntop(AF_INET6, $sender) if $af == 6;

	($header{ver}, $header{flows}, $header{uptime}, $header{secs}, 
	 $header{nsecs}) = unpack("nnNNN", $pkt);

	if (length($pkt) < (16 + (48 * $header{flows}))) {
		printf STDERR timestamp()." Short Netflow v.1 packet: %d < %d\n",
		    length($pkt), 16 + (48 * $header{flows});
		return;
	}

	printf timestamp() . " HEADER v.%u (%u flow%s)\n", $header{ver},
	    $header{flows}, $header{flows} == 1 ? "" : "s";

	for(my $i = 0; $i < $header{flows}; $i++) {
		my $off = 16 + (48 * $i);
		my $ptr = substr($pkt, $off, 52);

		%flow = qw();

		(my $src1, my $src2, my $src3, my $src4,
		 my $dst1, my $dst2, my $dst3, my $dst4, 
		 my $nxt1, my $nxt2, my $nxt3, my $nxt4, 
		 $flow{in_ndx}, $flow{out_ndx}, $flow{pkts}, $flow{bytes}, 
		 $flow{start}, $flow{finish}, $flow{src_port}, $flow{dst_port}, 
		 my $pad1, $flow{protocol}, $flow{tos}, $flow{tcp_flags}) =
		    unpack("CCCCCCCCCCCCnnNNNNnnnCCC", $ptr);

		$flow{src} = sprintf "%u.%u.%u.%u", $src1, $src2, $src3, $src4;
		$flow{dst} = sprintf "%u.%u.%u.%u", $dst1, $dst2, $dst3, $dst4;
		$flow{nxt} = sprintf "%u.%u.%u.%u", $nxt1, $nxt2, $nxt3, $nxt4;

		printf timestamp() . " " .
		    "from %s started %s finish %s proto %u %s:%u > %s:%u %u " . 
		    "packets %u octets\n",
		    $sender_s,
		    fuptime($flow{start}), fuptime($flow{finish}), 
		    $flow{protocol}, 
		    $flow{src}, $flow{src_port}, $flow{dst}, $flow{dst_port}, 
		    $flow{pkts}, $flow{bytes};
	}
}

sub process_nf_v5($$)
{
	my $sender = shift;
	my $pkt = shift;
	my %header;
	my %flow;
	my $sender_s;

	%header = qw();

	$sender_s = inet_ntoa($sender) if $af == 4;
	$sender_s = inet_ntop(AF_INET6, $sender) if $af == 6;

	($header{ver}, $header{flows}, $header{uptime}, $header{secs}, 
	 $header{nsecs}, $header{flow_seq}, ) = unpack("nnNNNN", $pkt);

	if (length($pkt) < (24 + (48 * $header{flows}))) {
		printf STDERR timestamp()." Short Netflow v.1 packet: %d < %d\n",
		    length($pkt), 24 + (48 * $header{flows});
		return;
	}

	printf timestamp() . " HEADER v.%u (%u flow%s) seq %u\n", $header{ver},
	    $header{flows}, $header{flows} == 1 ? "" : "s", $header{flow_seq};

	for(my $i = 0; $i < $header{flows}; $i++) {
		my $off = 24 + (48 * $i);
		my $ptr = substr($pkt, $off, 52);

		%flow = qw();

		(my $src1, my $src2, my $src3, my $src4,
		 my $dst1, my $dst2, my $dst3, my $dst4, 
		 my $nxt1, my $nxt2, my $nxt3, my $nxt4, 
		 $flow{in_ndx}, $flow{out_ndx}, $flow{pkts}, $flow{bytes}, 
		 $flow{start}, $flow{finish}, $flow{src_port}, $flow{dst_port}, 
		 my $pad1, $flow{tcp_flags}, $flow{protocol}, $flow{tos},
		 $flow{src_as}, $flow{dst_as},
		 $flow{src_mask}, $flow{dst_mask}) =
		    unpack("CCCCCCCCCCCCnnNNNNnnCCCCnnCC", $ptr);

		$flow{src} = sprintf "%u.%u.%u.%u", $src1, $src2, $src3, $src4;
		$flow{dst} = sprintf "%u.%u.%u.%u", $dst1, $dst2, $dst3, $dst4;
		$flow{nxt} = sprintf "%u.%u.%u.%u", $nxt1, $nxt2, $nxt3, $nxt4;

		printf timestamp() . " " .
		    "from %s started %s finish %s proto %u %s:%u > %s:%u %u " . 
		    "packets %u octets\n",
		    $sender_s,
		    fuptime($flow{start}), fuptime($flow{finish}), 
		    $flow{protocol}, 
		    $flow{src}, $flow{src_port}, $flow{dst}, $flow{dst_port}, 
		    $flow{pkts}, $flow{bytes};
	}
}

############################################################################

# Commandline options
my $debug = 0;
my $af4 = 0;
my $af6 = 0;
my $port;

#		Long option		Short option
GetOptions(	'debug+' => \$debug,	'd+' => \$debug,
					'4+' => \$af4,
					'6+' => \$af6,
		'port=i' => \$port,	'p=i' => \$port);

# Unbuffer output
$| = 1;

die "The -4 and -6 are mutually exclusive\n" if $af4 && $af6;
die "You must specify a port (collector.pl -p XXX).\n" unless $port;

$af4 = $af = 4 if $af4 || (!$af4 && !$af6);
$af6 = $af = 6 if $af6;

# These modules aren't standard everywhere, so load them only if necessary

# Main loop - receive and process a packet
for (;;) {
	my $socket;
	my $from;
	my $payload;
	my $ver;
	my $failcount = 0;
	my $netflow;
	my $junk;
	my $sender;

	# Open the listening port if we haven't already
	$socket = do_listen($port, $af) unless defined $socket;

	# Fetch a packet
	$from = $socket->recv($payload, 8192, 0);
	
	($junk, $sender) = unpack_sockaddr_in($from) if $af4;
	($junk, $sender) = unpack_sockaddr_in6($from) if $af6;

	# Reopen listening socket on error
	if (!defined $from) {
		$socket->close;
		undef $socket;

		$failcount++;
		die "Couldn't recv: $!\n" if ($failcount > 5);
		next; # Socket will be reopened at start of loop
	}
	
	if (length($payload) < 16) {
		printf STDERR timestamp()." Short packet recevied: %d < 16\n",
		    length($payload);
		next;
	}

	# The version is always the first 16 bits of the packet
	($ver) = unpack("n", $payload);

	if	($ver == 1)	{ process_nf_v1($sender, $payload); }
	elsif	($ver == 5)	{ process_nf_v5($sender, $payload); }
	else {
		printf STDERR timestamp()." Unsupported netflow version %d\n",
		    $ver;
		next;
	}
	
	undef $payload;
	next;	
}

exit 0;
