# This file is part of pound testsuite
# Copyright (C) 2018-2025 Sergey Poznyakoff
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
use strict;
use warnings;
use Socket qw(:DEFAULT :crlf);
use Getopt::Long;
use HTTP::Tiny;
use POSIX qw(:sys_wait_h);
use Cwd qw(abs_path);
use File::Spec;
use Socket;
use lib qw(perllib);
use PoundSub;

my $config = 'pound.cfi:pound.cfg';
my @preproc_files;
my $log_level = 2;
my $log_file = 'pound.log';
my $pid_file = 'pound.pid';
my $include_dir;
my $transcript_file;
my $verbose;
my $dry_run;
my $listeners = ListenerList->new(1);
my $backends = ListenerList->new(0);
my $pound_pid;
my $startup_timeout = 2;
my $statistics;
my $fakedns;
my $valgrind;
my @source_addr;

use constant {
    EX_SUCCESS => 0,
    EX_FAILURE => 1,
    EX_ERROR => 2,
    EX_USAGE => 3,
    EX_SKIP => 77,
    EX_EXEC => 127
};

my $nameserver;

## Early checks
## ------------

# Check for perl version.
eval "require 5.026_003";
if ($@) {
    print STDERR "$@";
    exit(EX_SKIP);
}

# Check for external modules
eval "require IO::FDPass";
if ($@) {
    print STDERR "required module IO::FDPass not present\n";
    exit(EX_SKIP);
}

eval "require JSON";
if ($@) {
    print STDERR "required module JSON not present\n";
    exit(::EX_SKIP);
}

sub cleanup {
    if ($pound_pid) {
	if ($verbose) {
	    print "$$ Stopping pound ($pound_pid)\n";
	}
	kill 'HUP', $pound_pid;
    }
}

## Signal handling and program cleanup
## -----------------------------------

my %status_codes;

$SIG{QUIT} = $SIG{HUP} = $SIG{TERM} = $SIG{INT} = \&cleanup;

sub handle_pound_status {
    if (WIFEXITED($?)) {
	if (WEXITSTATUS($?)) {
	    print STDERR "pound terminated with code " . WEXITSTATUS($?) . "\n";
	} else {
	    if ($verbose) {
		print "pound finished\n";
	    }
	    return 1;
	}
    } elsif (WIFSIGNALED($?)) {
	print STDERR "pound terminated on signal " . WTERMSIG($?) . "\n";
    } else {
	print STDERR "pound terminated with unrecognized status " . $? . "\n";
    }

    return 0;
}

sub sigchild {
    while (1) {
	my $pid = waitpid(-1, WNOHANG);
	if ($pid == -1) {
	    return
	}
	if (!defined($pound_pid) || $pid != $pound_pid) {
	    $status_codes{$pid} = $?;
	    $SIG{CHLD} = \&sigchild;
	    return;
	}
	$pound_pid = 0;
	if (handle_pound_status()) {
	    $SIG{CHLD} = \&sigchild;
	    return;
	}
	exit(EX_ERROR);
    }
};
$SIG{CHLD} = \&sigchild;

END {
#    print STDERR "XXX END $$\n";
    PoundSub->stop;
}

sub fakedns_self_check {
    my $params = 'ns1.example.org. rname@example.org. 42 7200 3600 1209600 3600';

    $nameserver->ZoneUpdate(
	'$ORIGIN example.org.',
	'@ IN SOA '.$params
    );

    local $ENV{LD_PRELOAD} = $fakedns;
    local %SIG;
    if (open(my $fh, '-|', 'getsoa', 'example.org', '192.0.2.24')) {
	chomp(my $s = <$fh>);
	if ($s ne $params) {
	    print STDERR "Self-check: got $s\n";
	    return;
	}
	if (<$fh>) {
	    print STDERR "Self-check: garbage after response: $_\n";
	    return;
	}
	close $fh;
	return 1;
    }
}

sub assert_source_ip {
    my ($ip) = @_;
    socket(my $sock, PF_INET, SOCK_STREAM, 0)
	or do {
	    warn "socket: $!";
	    exit 77;
    };
    setsockopt($sock, SOL_SOCKET, SO_REUSEADDR, 1);
    unless (bind($sock, sockaddr_in(0, inet_aton($ip)))) {
	# FIXME: To be strict, this should check for $!{EADDRNOTAVAIL} and
	# exit 77 if so.
	warn "bind($ip): $!";
	exit(77);
    }
    close($sock)
}

## Start the program
## -----------------
sub usage_error {
    my $msg = shift;
    print STDERR "$msg\n";
    exit(EX_USAGE);
}

GetOptions('config|f=s' => \$config,
	   'preproc=s@' => \@preproc_files,
	   'verbose|v+' => \$verbose,
	   'log-level|l=n' => \$log_level,
	   'transcript|x=s' => \$transcript_file,
	   'statistics|s' => \$statistics,
	   'startup-timeout|t=n' => \$startup_timeout,
	   'include-dir=s' => \$include_dir,
	   'fakedns=s' => \$fakedns,
	   'valgrind=s' => \$valgrind,
	   'source-address|a=s@' => sub {
	       my (undef, $ip) = @_;
	       assert_source_ip($ip);
	       push @source_addr, $ip;
	   },
	   'test-threads' => sub {
	       exit(PoundSub->using_threads == 0);
	   }
    )
    or exit(EX_USAGE);

my $script_file = shift @ARGV or usage_error "required parameter missing";
usage_error "too many arguments\n" if @ARGV;

if ($fakedns) {
    eval "require PoundNS";
    if ($@) {
	print STDERR "PoundNS: $@\n";
	exit(EX_SKIP);
    }
    unless (-e $fakedns) {
	print STDERR "$fakedns: file not found\n";
	exit(EX_SKIP);
    }

    if (!File::Spec->file_name_is_absolute($fakedns)) {
	$fakedns = File::Spec->rel2abs($fakedns);
    }

    $nameserver = PoundNS->new or die "can't create nameserver";
    ($ENV{FAKEDNS_UDP_PORT}, $ENV{FAKEDNS_TCP_PORT}) = $nameserver->start_server();

    unless (fakedns_self_check()) {
	print STDERR "Self-check failed\n";
	exit(EX_SKIP);
    }
}

foreach my $file (@preproc_files) {
    if ($file =~ m{(?<src>.+):(?<dst>.+)}) {
	preproc($+{src}, $+{dst});
    } else {
	preproc($file, "$file.cfg");
    }
}

if ($config =~ m{(?<src>.+):(?<dst>.+)}) {
    preproc($+{src}, $+{dst}, 1);
    $config = $+{dst};
} else {
    my $ofile = 'pound.cfg';
    preproc($config, $ofile, 1);
    $config = $ofile;
}

if ($include_dir) {
    $config = abs_path($config)
}

$ENV{POUNDCTL_CONF} = "";

# Start HTTP listeners
$backends->read_and_process;

# Start pound
runner();

# Parse and run the script file
my $ps = PoundScript->new($script_file, $transcript_file, $nameserver,
			  \@source_addr);
$ps->parse;

# Terminate

if ($statistics) {
    print "Total tests: ".$ps->tests. "\n";
    print "Failures: ".$ps->failures. "\n";
} elsif ($ps->failures) {
    print STDERR $ps->failures . " from " . $ps->tests . " tests failed\n";
}

if (defined($pound_pid) && $pound_pid > 0) {
    $SIG{CHLD} = sub { };
    cleanup;
    if (waitpid($pound_pid, 0) == $pound_pid) {
	if (handle_pound_status() == 0) {
	    exit(EX_ERROR);
	}
    }
}

exit ($ps->failures ? EX_FAILURE : EX_SUCCESS);

## Configuration file processing
## -----------------------------

sub dequote {
    my $arg = shift;
    $arg =~ s/^\"(.+)\"$/$1/;
    return $arg;
}

sub isglob {
    my $arg = shift;
    return $arg =~ m/(?<!\\)[*?\[\]]/;
}

sub addport {
    my ($infile, $outfile, $port) = @_;
    open(my $in, '<', $infile)
	or die "can't open $infile for reading: $!";
    open(my $out, '>', $outfile)
	or die "can't create $outfile: $!";
    while (<$in>) {
	chomp;
	print $out "$_:$port\n";
    }
    close $in;
    close $out;
}

sub preproc {
    my ($infile, $outfile, $init, $initstate) = @_;
    if (!$init && $include_dir) {
	unless (File::Spec->file_name_is_absolute($infile)) {
	    $infile = File::Spec->catfile($include_dir, $infile)
	}
	unless (File::Spec->file_name_is_absolute($outfile)) {
	    $outfile = File::Spec->catfile($include_dir, $outfile)
	}
    }
    if ($verbose) {
	print "Preprocessing $infile into $outfile\n";
    }
    open(my $in, $infile)
	or die "can't open $infile for reading: $!";
    open(my $out, '>', $outfile)
	or die "can't create $outfile: $!";

    use constant {
	ST_INIT => 0,
	ST_LISTENER => 1,
	ST_SERVICE => 2,
	ST_BACKEND => 3,
	ST_BACKEND_STAT => 4,
	ST_BACKEND_DYN => 5,
	ST_SESSION => 6,
	ST_SECTION => 7
    };
    my @state;
    unshift @state, $initstate // ST_INIT;
    if ($init) {
	print $out <<EOT;
# Initial settings by $0
Daemon 0
Control "pound.ctl"
LogFacility -
EOT
    ;
	if ($log_level >= 0) {
	    print $out "LogLevel $log_level\n";
	}
	if ($fakedns) {
	    print $out <<EOT;
Resolver
    ConfigText
	nameserver 192.0.2.24
    End
    RetryInterval 10
End
EOT
    ;
	}
    }
    my $be_loc;
    while (<$in>) {
	chomp;
	if (/^\s*(?:#.)?$/) {
	    ;
	} elsif (/^\s*(Daemon|LogFacility)/i) {
	    $_ = "# Commented out: $_";
	} elsif ($log_level >= 0 && /^\s*LogLevel/i) {
	    $_ = "# Commented out: $_";
	} elsif (/^(\s*)Listen(HTTPS?)/i) {
	    unshift @state, ST_LISTENER;
	    print $out "$_\n";
	    my $lst = $listeners->create("$infile:$.", $2);
	    if ($verbose) {
		print $lst->ident . ": Listener ".$lst->address."\n";
	    }
	    print $out "# Addition by $0\n";
	    print $out "$1\tSocketFrom \"".$lst->sockname."\"\n";
	    next;
	} elsif (/^(?<indent>\s*)Include\s+(?<arg>.+)/) {
	    my $arg = dequote($+{arg});
	    if (isglob($arg)) {
		foreach my $file (glob $arg) {
		    my $outfile = $file . '.pha';
		    preproc($file, $outfile, 0, $state[0]);
		}
		print $out "$+{indent}# Edited by $0\n";
		$_ = "$+{indent}Include \"$arg.pha\"";
	    } else {
		my $file = $arg . '.pha';
		preproc($arg, $file, 0, $state[0]);
		print $out "$+{indent}# Edited by $0\n";
		$_ = "$+{indent}Include \"$file\"";
	    }
	} elsif (/^\s*Service/i) {
	    unshift @state, ST_SERVICE;
	} elsif (/^\s*Session/i) {
	    unshift @state, ST_SESSION;
	} elsif (/^\s*(Backend|Emergency)/i) {
	    $be_loc = "$infile:$.";
	    unshift @state, ST_BACKEND;
	} elsif (/^\s*((Match)|(Rewrite)|(TrustedIP)|(ACL)|(CombineHeaders)|(Condition)|(Lua))\b/i) {
	    unshift @state, ST_SECTION
	} elsif (/^(\s*)End/i) {
	    if ($state[0] == ST_BACKEND) {
		my $be = $backends->create($be_loc);
		if ($verbose) {
		    print "$infile:$.: Backend ".$be->ident . ": " . $be->address."\n";
		}
		print $out "# Inserted by $0\n";
		print $out "$1\tAddress ".$be->host."\n";
		print $out "$1\tPort ".$be->port."\n";
	    }
	    shift @state
	} elsif ($state[0] == ST_BACKEND) {
	    shift @state;
	    if (/^\s*Resolve/) {
		unshift @state, ST_BACKEND_DYN;
	    } else {
		unshift @state, ST_BACKEND_STAT;
		my $be = $backends->create($be_loc);
		if ($verbose) {
		    print "$infile:$.: Backend ".$be->ident . ": " . $be->address."\n";
		}
		/^(\s*)([\S]+)/;
		my ($indent, $kw) = ($1, $2);
		print $out "# Inserted by $0\n";
		print $out "${indent}Address ".$be->host."\n";
		print $out "${indent}Port ".$be->port."\n";
		next if ($kw =~ /^Address/i || $kw =~ /^Port/i);
	    }
	} elsif ($state[0] == ST_BACKEND_STAT) {
	    next if (/^\s*Address/i || /^\s*Port/i);
	} elsif ($state[0] == ST_LISTENER) {
	    if (/^\s*(Address|Port|SocketFrom)/i) {
		    $_ = "# Commented out: $_";
	    }
	} elsif ($state[0] == ST_SERVICE) {
	    if (/^(\s*)Host\s+(.+)\s*$/) {
		if (my $lst = $listeners->last()) {
		    my $indent = $1;
		    my $hostline = $2;
		    my @opts;
		    my $exact = 1;
		    my $file;
		    while ($hostline =~ m/^\s*(-\S+)(.*)/) {
			push @opts, $1;
			if ($1 eq '-re' || $1 eq '-beg') {
			    $exact = 0;
			} elsif ($1 eq '-file' || $1 eq '-filewatch') {
			    $file = 1;
			}
			$hostline = $2;
		    }
		    $hostline =~ s/^\s+//;
		    $hostline = dequote($hostline);
		    if ($exact || @opts == 0) {
			if ($file) {
			    my $ofile = $hostline . '.port';
			    addport($hostline, $ofile, $lst->port);
			    $hostline = $ofile;
			} else {
			    $hostline = $hostline . ':' . $lst->port;
			}
			print $out "$indent# Edited by $0\n"
		    }
		    $_ = $indent . "Host";
		    if (@opts) {
			$_ .= ' ' . join(' ', @opts);
		    }
		    $_ .= " \"$hostline\"";
		}
	    }
	} elsif (/^\s*Socket/) {
	    $_ = "# Commented out: $_";
	}
	print $out "$_\n";
    }
    close $in;
    close $out;
}

## Start pound
## -----------
sub runner {
    $pound_pid = fork();
    die "fork: $!" unless defined $pound_pid;
    if ($pound_pid > 0) {
	# Send all listener descriptors to pound
	$listeners->send_fd;

	# Wait for the things to settle.
	if ($verbose) {
	    print "waiting for pound to start up\n";
	}

	$listeners->wait;
	if ($verbose) {
	    print "pound is ready\n";
	}
	return;
    }
    open(STDOUT, '>', $log_file);
    open(STDERR, ">&STDOUT");
    STDOUT->autoflush(1);
    STDERR->autoflush(1);
    my @cmd = (
	'pound', '-p', $pid_file, '-f', $config, '-v', '-e',
	'-W', $include_dir ? "include-dir=$include_dir" : 'no-include-dir'
    );
    if ($valgrind) {
	unshift @cmd, 'valgrind', '--log-file=' . $valgrind;
    }
    if ($fakedns) {
	$ENV{LD_PRELOAD} = $fakedns;
    } else {
	push @cmd, '-W', 'no-dns';
    }
    exec @cmd or exit(EX_EXEC);
}

package PoundScript;
use strict;
use warnings;
use Carp;
use HTTP::Tiny;
use Data::Dumper;
use IO::Select;
use IPC::Open3;
use Symbol 'gensym';

sub new {
    my ($class, $file, $xscript, $ns, $srcaddr) = @_;
    my $self = bless { tests => 0, failures => 0, ns => $ns }, $class;
    if ($file ne '-') {
	open(my $fh, '<', $file)
	    or croak "can't open script $file: $!";
	$self->{filename} = $file;
	$self->{fh} = $fh;
    } else {
	$self->{filename} = "<stdin>";
	$self->{fh} = \*STDIN;
    }
    if ($xscript) {
	open($self->{xscript}, '>', $xscript)
	    or croak "can't create transcript file $xscript: $!";
    }
    $self->{server} = 0;
    $self->{source_addr} = '127.0.0.1';
    if ($srcaddr) {
	unshift @$srcaddr, $self->{source_addr}
    } else {
	$srcaddr = [ $self->{source_addr} ]
    }
    foreach my $addr (@$srcaddr) {
	$self->{http}{$addr} = HTTP::Tiny->new(
			  max_redirect => 0,
			  verify_SSL => 0,
			  local_address => $addr
	    );
    }
    return $self;
}

sub tests { shift->{tests} }
sub failures { shift->{failures} }

sub http {
    my ($self) = @_;
    return $self->{http}{$self->{source_addr}}
}

sub source_address {
    my ($self, $addr) = @_;
    croak "unknown source address" unless exists $self->{http}{$addr};
    $self->{source_addr} = $addr
}

sub transcript {
    my ($self, $text, %opt) = @_;
    if (my $fh = $self->{xscript}) {
	if (my $locus = delete $opt{locus}) {
	    print $fh "$self->{filename}:$locus->{BEG}-$locus->{END}";
	} else {
	    print $fh "$self->{filename}:$self->{line}";
	}
	print $fh ": ";
	print $fh $text;
	print $fh "\n";
    }
}

sub transcript_ml {
    my $self = shift;
    if (my $fh = $self->{xscript}) {
	print $fh join("\n", @_)."\n";
    }
}

sub transcript_ok { shift->transcript("OK", @_) }
sub transcript_fail { shift->transcript("FAIL", @_) }

sub send_and_expect {
    my ($self, $lst, $host, $stats) = @_;
    my $url = $lst->proto . '://' .
	      $host .
	      ':' .
	      $lst->port .
	      $self->{REQ}{URI};
    my %options;
    $options{peer} = $lst->host;
    if (exists($self->{REQ}{HEADERS})) {
	$options{headers} = $self->{REQ}{HEADERS};
    }
    if (exists($self->{REQ}{BODY})) {
	my $body = $self->{REQ}{BODY};
	if ($body =~ /@@/) {
	    my @chunks = split /@@/, $body;
	    $options{content} = sub {
		return shift(@chunks)
	    }
	} else {
	    $options{content} = $body;
	}
    }
    if ($verbose) {
	print "URL $self->{REQ}{METHOD} $url\n";
    }

    $self->transcript("sending $self->{REQ}{METHOD} $url to server $self->{server}",
		      locus => $self->{REQ});

    my $response = $self->http->request($self->{REQ}{METHOD}, $url, \%options);
    $self->transcript("got:\n".Dumper([$response]), locus => $self->{REQ});

    $self->{tests}++;
    my $ok = $self->assert($response);
    if ($ok) {
	if ($stats) {
	    $stats->incr($response->{headers}{'x-backend-number'});
	}
    } else {
	$self->{failures}++;
    }
    $self->transcript($ok ? "OK" : "FAIL", locus => $self->{EXP});
    return $ok
}

sub check_expect {
    my ($self, $value, $exp) = @_;
    return $exp->{min} <= $value && $value <= $exp->{max};
}

sub send {
    my $self = shift;
    my $lst = $listeners->get($self->{server});
    my $host = delete $self->{REQ}{HEADERS}{host} || $lst->host;

    if ($self->{stats}) {
	my $n = $self->{stats}{samples}{value};
	my $s = Stats->new($backends->count);

	for (my $i = 0; $i < $n; $i++) {
	    $self->send_and_expect($lst, $host, $s) or goto end;
	}

	if (defined(my $i = $self->{stats}{index}{value})) {
	    $s = $s->elemstat($i);
	}

	# Assume success
	my $ok = 1;

	# Iterate over criteria applying them to the received statistics
	# and correcting $ok accordingly.
	foreach my $k (qw(sum min max avg stddev)) {
	    if (exists($self->{stats}{$k})) {
		unless ($self->check_expect($s->${ \$k }, $self->{stats}{$k})) {
		    $ok = 0;
		    $self->transcript("$k: expected value mismatch",
				      locus => $self->{EXP});
		}
	    }
	}

	# Additionally, apply any individual element criteria.
	if (exists($self->{stats}{expect})) {
	    foreach my $i (sort keys %{$self->{stats}{expect}}) {
		my $val = $s->sample($i);
		unless ($self->check_expect($s->sample($i),
					    $self->{stats}{expect}{$i})) {
		    $ok = 0;
		    $self->transcript("element $i: expected value mismatch",
				      locus => $self->{EXP});
		}
	    }
	}

	$self->{failures}++ unless $ok;

	if ($self->{xscript}) {
	    $self->transcript(sprintf("min=%f, avg=%f, max=%f, stddev=%f",
				      $s->min, $s->avg, $s->max, $s->stddev));
	    local $Data::Dumper::Sortkeys=1;
	    $self->transcript(Dumper([$s->samples]));
	    $self->transcript($ok ? "OK" : "FAIL", locus => $self->{EXP});
	}
    } else {
	$self->send_and_expect($lst, $host);
    }
 end:
    delete $self->{REQ};
    delete $self->{EXP};
    delete $self->{stats};
}

sub body_match {
    my ($self, $body) = @_;
    if ($self->{EXP}{BODY} =~ m{^:re\s*\n(.+)}s) {
	my $rx = $1;
	return $body =~ m{$rx}ms
    } elsif ($self->{EXP}{BODY} =~ m{^:exact\s*\n(.+)}s) {
	return $1 eq $body
    } else {
	return $self->{EXP}{BODY} eq $body
    }
}

sub assert {
    my ($self, $response) = @_;
    if ($self->{EXP}) {
	if ($self->{EXP}{STATUS} != $response->{status}) {
	    print STDERR "$self->{filename}:$self->{EXP}{BEG}: expected status $self->{EXP}{STATUS}, but got $response->{status}\n";
	    return 0;
	}
	if ($self->{EXP}{HEADERS}) {
	    foreach my $h (sort keys %{$self->{EXP}{HEADERS}}) {
		unless (exists($response->{headers}{$h})) {
		    print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: expected header $h not present in the response\n";
		    return 0;
		}

		if ($self->{EXP}{HEADERS}{$h} =~ m{^/(.+)/$}) {
		    my $rx = qr($1);
		    if ($response->{headers}{$h} !~ $rx) {
			print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: expected header $h value mismatch (regexp)\n";
			return 0;
		    }
		} else {
		    (my $v = $self->{EXP}{HEADERS}{$h}) =~ s{^\\}{};
		    if ($v ne $response->{headers}{$h}) {
			print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: expected header $h value mismatch (expected \"$v\" got \"$response->{headers}{$h}\")\n";
			return 0;
		    }
		}
	    }
	}
	if ($self->{EXP}{NOTHEADERS}) {
	    foreach my $h (sort keys %{$self->{EXP}{NOTHEADERS}}) {
		if (exists($response->{headers}{$h}) &&
		    $response->{headers}{$h} eq $self->{EXP}{NOTHEADERS}{$h}) {
		    print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: header \"$h: $self->{EXP}{NOTHEADERS}{$h}\" is present in the response\n";
		}
	    }
	}

	if (exists($self->{EXP}{BODY}) && !$self->body_match($response->{content})) {
	    print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: response content mismatch\n";
	    print STDERR "EXP '".$self->{EXP}{BODY}."'\n";
	    print STDERR "GOT '".$response->{content}."'\n";
	}
    }
    return 1
}


sub parse {
    my $self = shift;
    eval {
	while (!$self->{eof}) {
	    $self->parse_req;
	}
    };
    if ($@) {
	print STDERR "$@";
	print STDERR "Abnormal termination\n";
	$self->{failures}++;
    }
}

sub syntax_error {
    my ($self, $message) = @_;
    $message //= "syntax error\n";
    my $locus = $self->{filename} . ':' . $self->{line};
    confess "$locus: $message";
}

sub parse_req {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^(?:#.*)?$/) {
	    next;
	}
	if (/^server\s+(\d+)/) {
	    $self->{server} = $1;
	    next;
	}
	if (/^source\s+(.+)/) {
	    $self->source_address($1);
	    next;
	}
	if (/^mkbackend\s+(127(\.\d+){3})/) {
	    $backends->create("$self->{filename}:$self->{line}", 'HTTP', $1);
	    next;
	}
	if (/^sleep\s+(\d+)/) {
	    sleep $1;
	    next;
	}
	if (/^echo\s+(.+)/) {
	    print STDERR "# ".$self->expandvars($1)."\n";
	    next;
	}

	if (/^zonefile/) {
	    $self->parse_zonefile;
	    next;
	}

	if (m{^backends\s+(\d+)\s+(\d+)(?:\s+(.+))?}) {
	    $self->parse_control_backends($1, $2);
	    next;
	}

	if (s/^stats//) {
	    foreach my $k (qw(samples min max avg stddev index)) {
		if (s{\s+$k = (?:
				(?: (?<value>\d+(?:\.\d+)?)
				    (?: % (?: (?<dev>\d+(?:\.\d+)?) ) )? ) |
				(?: \[ \s* (?<min>\d+(?:\.\d+)?)
				      \s* , \s*
				      (?<max>\d+(?:\.\d+)?) \] )
			      )}{}x) {
		    if (defined($+{value})) {
			my $v = $+{value};
			$self->{stats}{$k}{value} = $v;
			if (defined($+{dev})) {
			    $self->{stats}{$k}{min} = $v - $+{dev} * $v / 100;
			    $self->{stats}{$k}{max} = $v + $+{dev} * $v / 100;
			} else {
			    my $d;
			    if ($v =~ /\.(\d+)/) {
				$d = (1/(10**length($1)))/2;
			    } else {
				$d = 0;
			    }
			    $self->{stats}{$k}{min} = $v - $d;
			    $self->{stats}{$k}{max} = $v + $d;
			}
		    } else {
			$self->{stats}{$k}{min} = $+{min};
			$self->{stats}{$k}{max} = $+{max};
		    }
		}
	    }

	    if (s/\s+expect\s+(?<i>\d+)\s+(?<value>\d+(?:\.\d+)?)(?:%(?<dev>\d+(?:\.\d+)?))?//) {
		$self->{stats}{expect}{$+{i}}{value} = $+{value};
		$self->{stats}{expect}{$+{i}}{dev} = $+{dev};
	    }
	    unless (/^\s*$/) {
		$self->syntax_error("unrecognized keywords in stats statement: $_");
	    }
	    unless (exists($self->{stats}{samples})) {
		$self->syntax_error("required keyword \"samples\" is missing");
	    }

	    next;
	}
	if (m/^run\s+(.+)$/) {
	    $self->parse_runcom($1);
	    next;
	}
	if (/^end$/) {
	    if ($self->{REQ}{BEG}) {
		$self->{REQ}{END} = $self->{line};
		$self->parse_expect;
	    } else {
		$self->syntax_error("unexpected end");
	    }
	    return;
	}
	if (/^(?<method>[A-Z_]+)\s+(?<uri>.+)$/) {
	    ($self->{REQ}{METHOD}, $self->{REQ}{URI}) = ($+{method}, $+{uri});
	    unless ($self->{REQ}{URI} =~ m{^/}) {
		$self->{REQ}{URI} = "/$self->{REQ}{URI}";
	    }
	    $self->{REQ}{PROTO} = 'http';
	    $self->{REQ}{BEG} = $self->{line};

	    $self->parse_headers;
	    return;
	}
	$self->syntax_error;
	return;
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file") if $self->{REQ}{BEG};
}

sub parse_zonefile {
    my $self = shift;
    my $fh = $self->{fh};

    my @zonetext;
    while (<$fh>) {
	$self->{line}++;
	chomp;
	last if /^end$/;
	push @zonetext, $self->expandvars($_)
    }

    $self->{ns}->ZoneUpdate(@zonetext);
}

sub jsoncmp {
    my ($a, $b) = @_;

    my $rtype = ref($a);
    return 0 if $rtype ne ref($b);
    if ($rtype eq '') {
	return $a eq $b;
    } elsif ($rtype eq 'ARRAY') {
	return 0 if @{$a} != @{$b};
	for (my $i = 0; $i <= $#{$a}; $i++) {
	    return 0 unless jsoncmp($a->[$i], $b->[$i]);
	}
    } elsif ($rtype eq 'HASH') {
	foreach my $k (keys %$a) {
	    return 0 unless jsoncmp($a->{$k}, $b->{$k});
	}
    } else {
	return 0
    }
    return 1
}

sub parse_control_backends {
    my ($self, $ls, $sv) = @_;

    if ($verbose) {
	print "querying control interface\n";
    }

    $self->transcript("querying control interface");

    my $fh = $self->{fh};
    $self->{tests}++;
    my @exp;
    while (<$fh>) {
	$self->{line}++;
	chomp;
	last if /^end$/;
	push @exp, $self->expandvars($_)
    }

    my $ctl = PoundControl->new();
    my $belist = $ctl->backends($ls, $sv, sort => 1);
    my $json = JSON->new->boolean_values(0,1);
    my $exp = $json->decode(join(' ', @exp));

    if (jsoncmp($exp, $belist)) {
	$self->transcript_ok;
    } else {
	$self->{failures}++;
	$self->transcript_ml("backend listings don't match",
			     "exp: " . $json->canonical->pretty->encode($exp),
			     "got: " . $json->canonical->pretty->encode($belist));
	$self->transcript_fail;
    }
}

sub parse_headers {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^$/) {
	    $self->parse_body;
	    return;
	}

	if (/^end$/) {
	    $self->{REQ}{END} = $self->{line};
	    $self->parse_expect;
	    return;
	}

	if (/^#.*$/) {
	    next;
	}

	if (/^(?<name>[A-Za-z][A-Za-z0-9_-]*):\s*(?<value>.*)$/) {
	    my $name = lc($+{name});
	    if (exists($self->{REQ}{HEADERS}{$name})) {
		my $val = $self->{REQ}{HEADERS}{$name};
		if (ref($val) ne 'ARRAY') {
		    $self->{REQ}{HEADERS}{$name} = [$val];
		}
		push @{$self->{REQ}{HEADERS}{$name}}, $self->expandvars($+{value})
	    } else {
		$self->{REQ}{HEADERS}{lc($+{name})} = $self->expandvars($+{value});
	    }
	} else {
	    $self->syntax_error;
	    return;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub parse_body {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^end(nonl)?$/) {
	    $self->{REQ}{END} = $self->{line};
	    if ($self->{REQ}{BODY}) {
		$self->{REQ}{BODY} = join("\n", @{$self->{REQ}{BODY}});
		$self->{REQ}{BODY} .= "\n" unless /nonl/;
	    }
	    $self->parse_expect;
	    return;
	}

	if (/^(?:#.*)?$/) {
	    next;
	}

	if (/\\(.*)/) {
	    push @{$self->{REQ}{BODY}}, $1;
	} else {
	    push @{$self->{REQ}{BODY}}, $_;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub parse_runcom {
    my ($self, $command) = @_;
    my $fh = $self->{fh};

    my $collect;

    $self->{RUNCOM} = {
	command => $command,
	BEG => $self->{line}
    };
    while (<$fh>) {
	$self->{line}++;
	chomp;

	if ($collect && s{^\\}{}) {
	    push @{$self->{RUNCOM}{$collect}}, $_;
	    next;
	}

	if (/^end$/) {
	    if ($collect) {
		$collect = undef;
		next;
	    }
	    $self->{RUNCOM}{END} = $self->{line};
	    $self->runcom;
	    return;
	}

	if ($collect) {
	    push @{$self->{RUNCOM}{$collect}}, $_;
	    next;
	}

	if (/^status\s+(\d+)/) {
	    $self->{RUNCOM}{status} = $1;
	    next;
	}

	if (/^(stdout|stderr)\s*$/) {
	    $collect = $1;
	    next;
	}

	if (/^logtail\s+\"(.+)\"(:?\s+([[:digit:]]+))?\s*$/) {
	    $self->{RUNCOM}{tail} = [ $log_file, $1, $2 ];
	    next;
	}

	$self->syntax_error;
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub runcom {
    my $self = shift;

    my $tail;
    if ($self->{RUNCOM}{tail}) {
	$tail = Tail->new(@{$self->{RUNCOM}{tail}});
    }

    my ($child_stdin, $child_stdout, $child_stderr);
    $child_stderr = gensym();
    my $pid = open3($child_stdin, $child_stdout, $child_stderr,
		    $self->{RUNCOM}{command});
    close $child_stdin;

    my $sel = IO::Select->new();
    $sel->add($child_stdout, $child_stderr);
    my $CHUNK_SIZE = 1000;
    my @ready;
    my %data = ( $child_stdout => '', $child_stderr => '' );
    my $timeout = $self->{RUNCOM}{timeout};
    my @args;
    push @args, 1 if $timeout;
    my $start = time;
    while (1) {
	$! = 0;
	@ready = $sel->can_read(@args);
	unless (@ready) {
	    if ($!) {
		$self->transcript("waiting for output: $!",
				  locus => $self->{RUNCOM});
		last;
	    }
	    last unless $timeout;
	}
	if ($timeout && time - $start > $timeout) {
	    $self->transcript("timed out", locus => $self->{RUNCOM});
	    kill 'KILL', $pid unless defined($status_codes{$pid});
	    last;
	}

	foreach my $fh (@ready) {
	    while (1) {
		my $len = sysread($fh, $data{$fh}, $CHUNK_SIZE,
				  length($data{$fh}));
		die "sysread: $!" unless defined($len);
		if ($len == 0) {
		    $sel->remove($fh);
		    $fh->close;
		    last;
		}
	    }
	}
    }

    if (!defined($status_codes{$pid})) {
	sleep 10; # FIXME: hardcoded timeout
    }

    my $code;
    if (!defined($status_codes{$pid})) {
	die "failed to execute $self->{RUNCOM}{command}";
    } elsif ($status_codes{$pid} & 127) {
	die "\"".$self->{RUNCOM}{command}."\" terminated on signal ".($status_codes{$pid} & 127);
    } else {
	$code = $status_codes{$pid} >> 8;
	delete $status_codes{$pid};
    }

    $self->transcript_ml(
	"Command: " . $self->{RUNCOM}{command},
	"Status code: $code",
	"Stdout:",
	$data{$child_stdout},
	"End",
	"Stderr:",
	$data{$child_stderr},
	"End"
    );

    $self->{tests}++;
    my $ok = 1;

    if (exists($self->{RUNCOM}{status}) && $code != $self->{RUNCOM}{status}) {
	$self->{failures}++;
	$self->transcript("exit code differs", locus => $self->{RUNCOM});
	$ok = 0;
    }

    if (exists($self->{RUNCOM}{stdout})) {
	my $s = join("\n", @{$self->{RUNCOM}{stdout}});
	if ($data{$child_stdout} !~ m{$s}ms) {
	    $self->{failures}++;
	    $self->transcript("stdout differs:\nrx: {$s}",
			      locus => $self->{RUNCOM});
	    $ok = 0;
	}
    }

    if (exists($self->{RUNCOM}{stderr})) {
	my $s = join("\n", @{$self->{RUNCOM}{stderr}});
	if ($data{$child_stderr} !~ m{$s}ms) {
	    $self->{failures}++;
	    $self->transcript("stderr differs:\nrx: {$s}",
			      locus => $self->{RUNCOM});
	    $ok = 0;
	}
    }

    if ($ok && $tail) {
	$ok = $tail->wait;
	if ($ok == 0) {
	    $self->transcript("expected string didn't appear in logfile");
	    $self->{failures}++;
	}
    }

    $self->transcript($ok ? "OK" : "FAIL");
    return $ok
}

sub parse_expect {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^end$/) {
	    $self->send;
	    return;
	}

	if (/^(?:#.*)?$/) {
	    next;
	}

	if (/^\d{3}$/) {
	    $self->{EXP}{BEG} = $self->{line};
	    $self->{EXP}{STATUS} = $_;

	    $self->parse_expect_headers;
	    return;
	} else {
	    $self->syntax_error;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub replvar {
    my ($self, $var) = @_;
    if ($var =~ m{(LISTENER|BACKEND)(\d+)?(?::(PORT|IP))?}) {
	my $list = ($1 eq 'LISTENER') ? $listeners : $backends;
	my $meth = 'address';
	if (defined($3)) {
	    $meth = ($3 eq 'PORT') ? 'port' : 'host';
	}
	if (my $obj = $list->get($2//0)) {
	    return $obj->${ \$meth };
	} else {
	    $self->syntax_error("unrecognized variable $var");
	}
    }
    return '${' . $var . '}';
}

sub expandvars {
    my ($self, $v) = @_;
    $v =~ s{ \$ \{ ([A-Za-z][A-Za-z0-9:]*) \} }
	   { $self->replvar($1) }xeg;
    return $v;
}

sub parse_expect_headers {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^$/) {
	    $self->parse_expect_body;
	    return;
	}

	if (/^end$/) {
	    $self->{EXP}{END} = $self->{line};
	    $self->send;
	    return;
	}

	if (/^#.*$/) {
	    next;
	}

	if (/^(?<name>[A-Za-z][A-Za-z0-9_-]*):\s*(?<value>.*)$/) {
	    $self->{EXP}{HEADERS}{lc($+{name})} = $self->expandvars($+{value});
	} elsif (/^-(?<name>[A-Za-z][A-Za-z0-9_-]*):\s*(?<value>.*)$/) {
	    $self->{EXP}{NOTHEADERS}{lc($+{name})} = $self->expandvars($+{value});
	} else {
	    $self->syntax_error;
	    return;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub parse_expect_body {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^end(nonl)?$/) {
	    $self->{EXP}{END} = $self->{line};
	    if ($self->{EXP}{BODY}) {
		$self->{EXP}{BODY} = join("\n", @{$self->{EXP}{BODY}});
		$self->{EXP}{BODY} .= "\n" unless /nonl/;
	    }
	    $self->send;
	    return;
	}

        push @{$self->{EXP}{BODY}}, $_;
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

package Tail;
use strict;
use warnings;
use Carp;
use Fcntl qw(:seek);

sub new {
    my ($class, $file, $expect, $ttl) = @_;
    open(my $fh, '<', $file) or croak "can't open $file: $!";
    seek $fh, 0, SEEK_END;
    return bless { file => $file, fh => $fh, expect => $expect,
		   ttl => $ttl // 2 }, $class;
}

sub wait {
    my $self = shift;
    my $line = '';
    my $fh = $self->{fh};
    my $start = time;
    while (1) {
	if (my $s = <$fh>) {
	    $line .= $s;
	    if ($line =~ /\n/m) {
		chomp($line);
		return 1 if $line =~ /$self->{expect}/;
		$line = '';
	    }
	} elsif (time - $start >= $self->{ttl}) {
	    return 0;
	} else {
	    # Clear EOF indicator
	    seek($fh, 0, 1);
	}
	sleep 0.2;
    }
}

package Stats;
use strict;
use warnings;
use Carp;

sub new {
    my ($class, $n) = @_;
    bless { samples => [ (0) x ($n //= 0) ] }, $class;
}

sub incr {
    my ($self, $n, $v) = @_;
    $v //= 1;
    $self->{samples}[$n] += $v;
    $self->{total} += $v;
    $self->clear;
}

sub clear { shift->{dirty} = 1 }

sub total { shift->{total} }

sub numsamples {
    my $self = shift;
    return 0 + @{$self->{samples}}
}

sub sample {
    my ($self, $n) = @_;
    if ($n < 0 || $n >= $self->numsamples) {
	croak "n out of range";
    }
    return $self->{samples}[$n]
}

sub samples { @{shift->{samples}} }
sub ratio {
    my $self = shift;
    return map { $_ / $self->avg } $self->samples
}

sub elemstat {
    my ($self, $n) = @_;
    my $s = Stats->new();
    $s->incr(0, $self->sample($n) / $self->sum);
    return $s;
}

sub _compute {
    my ($self, $what) = @_;
    if ($self->{dirty}) {
	my $nsamples = $self->numsamples;
	my $sum = 0;
	my $sumq = 0;
	my $min = $self->{samples}[0] // 0;
	my $max = $self->{samples}[0] // 0;

	foreach my $v (@{$self->{samples}}) {
	    $v //= 0;
	    $sum += $v;
	    $sumq += $v * $v;
	    if ($v < $min) {
		$min = $v;
	    }
	    if ($v > $max) {
		$max = $v;
	    }
	}

	$self->{sum} = $sum;
	$self->{min} = $min;
	$self->{max} = $max;
	$self->{avg} = $sum / $nsamples;
	$self->{stddev} = sqrt($sumq / $nsamples - $self->{avg} * $self->{avg});

	$self->{dirty} = 0;
    }
    return $self->{$what}
}

sub sum { shift->_compute('sum') }
sub min { shift->_compute('min') }
sub max { shift->_compute('max') }
sub avg { shift->_compute('avg') }
sub stddev { shift->_compute('stddev') }

#
# Connection
#
package Listener;
use strict;
use warnings;
use Carp;
use Socket qw(:DEFAULT :crlf);;
use Fcntl;

eval "require IO::FDPass";
if ($@) {
    print STDERR "required module IO::FDPass not present\n";
    exit(::EX_SKIP);
}

sub new {
    my ($class, $number, $ident, $keepopen, $proto, $ip) = @_;
    my $socket;
    socket($socket, PF_INET, SOCK_STREAM, 0)
	or croak "socket: $!";
    setsockopt($socket, SOL_SOCKET, SO_REUSEADDR, 1);
    bind($socket, pack_sockaddr_in(0, inet_aton($ip // '127.0.0.1')))
	or die "bind: $!";
    if ($keepopen) {
	listen($socket, 128);
	my $flags = fcntl($socket, F_GETFD, 0)
	    or croak "fcntl F_GETFD: $!";
	fcntl($socket, F_SETFD, $flags & ~FD_CLOEXEC)
	    or croak "fcntl F_SETFD: $!";
    }
    my $sa = getsockname($socket);
    my ($port, $ipaddr) = sockaddr_in($sa);
    bless {
	number => $number,
	ident => $ident,
	socket => $socket,
	host => inet_ntoa($ipaddr),
	port => $port,
	proto => lc($proto // "http")
    }, $class;
}

sub proto { shift->{proto} }
sub number { shift->{number} }
sub ident { shift->{ident} }
sub socket_handle { shift->{socket} }
sub fd { fileno(shift->socket_handle) }
sub host { shift->{host} }
sub port { shift->{port} }
sub address {
    my $ls = shift;
    return $ls->host . ":" . $ls->port
}
sub sockname { shift->{sockname} }
sub pass_fd { shift->{pass_fd} }
sub set_pass_fd {
    my ($self, $sockname) = @_;
    unlink($sockname);
    socket(my $pfs, PF_UNIX, SOCK_STREAM, 0)
	or croak "socket: $!";
    bind($pfs, pack_sockaddr_un($sockname))
	or croak "bind($sockname): $!";
    listen($pfs, 8);
    $self->{sockname} = $sockname;
    $self->{pass_fd} = $pfs;
}

sub send_fd {
    my $self = shift;
    croak "not applicable" unless $self->pass_fd;
    accept(my $fh, $self->pass_fd)
	or croak "accept on pass_fd failed: $!";
    IO::FDPass::send(fileno($fh), $self->fd)
	or croak "failed to pass socket: $!";
    close $self->socket_handle;
    $self->{socket_handle} = undef;
    close $self->pass_fd;
    $self->{pass_fd} = undef;
}

sub listen {
    my ($self, $backlog) = @_;
    listen($self->socket_handle, $backlog // 8)
	or croak "listen: $!";
}

package ListenerList;
use strict;
use warnings;
use Carp;
use Socket;

sub new {
    my ($class, $keepopen) = @_;
    return bless { listeners => [], keepopen => $keepopen };
}
sub keepopen { shift->{keepopen} }
sub count { scalar @{shift->{listeners}} }
sub create {
    my ($self, $ident, $proto, $ip) = @_;
    if (defined($proto) && lc($proto) eq 'HTTPS') {
	my ($ok, $why) = HTTP::Tiny->can_ssl;
	unless ($ok) {
	    print STDERR "testing HTTPS is not supported: $why\n";
	    exit(main::EX_SKIP);
	}
    }
    my $lst = Listener->new($self->count(), $ident, $self->keepopen, $proto, $ip);
    if ($self->keepopen) {
	$lst->set_pass_fd("lst".$self->count().".sock");
    }
    push @{$self->{listeners}}, $lst;
    return $lst;
}
sub get {
    my ($self, $n) = @_;
    return ${$self->{listeners}}[$n];
}
sub last {
    my ($self) = @_;
    return $self->get($self->count - 1)
}
sub send_fd {
    my $self = shift;
    croak "not applicable" unless $self->keepopen;
    foreach my $lst (@{$self->{listeners}}) {
	$lst->send_fd()
    }
}

sub find_socket {
    my ($self, $sock) = @_;
    foreach my $lst (@{$self->{listeners}}) {
	return $lst if ($lst->socket_handle == $sock);
    }
    croak "Listener not found";
}

sub wait {
    my $self = shift;
    my @lst = @{$self->{listeners}};
    my $start = time();
    while (my $s = shift @lst) {
	if (time() - $start > $startup_timeout) {
	    print STDERR "pound didn't start up within $startup_timeout seconds\n";
	    print STDERR "examine $log_file\n";
	    exit(::EX_ERROR);
	}
	socket(my $sock, PF_INET, SOCK_STREAM, 0);
	my $res = connect($sock, pack_sockaddr_in($s->port, inet_aton($s->host)));
	unless ($res) {
	    push @lst, $s;
	}
	close($sock);
    }
}

sub read_and_process {
    my $self = shift;

    foreach my $lst (@{$self->{listeners}}) {
	$lst->listen;
    }

    foreach my $lst (@{$self->{listeners}}) {
	PoundSub->start(sub {
	    my $lst = shift;
	    $lst->listen;
	    while (1) {
		my $fh;
		accept($fh, $lst->socket_handle) or PoundSub->exit();
		process_http_request($fh, $lst)
	    }
	}, $lst);
    }
}

sub http_echo {
    my $http = shift;
    my %headers = (
	'x-backend-ident' => $http->header('x-backend-ident'),
	'x-backend-number' => $http->backend->number,
	'x-orig-uri' => $http->uri,
	);
    while (my ($k, $v) = each %{$http->header}) {
	$headers{'x-orig-header-' . $k} = $v;
    }
    my @argv = (200, "OK", headers => \%headers);

    if (my $body = $http->body) {
	push @argv, body => $body
    }
    $http->reply(@argv);
}

sub http_redirect {
    my ($http, $rest) = @_;
    my $redir = $http->header('x-redirect')//'';
    $http->reply(301, "Moved Permanently", headers => {
	'location' => $redir . '/echo' . $rest,
	'x-orig-location' => $redir . '/echo' . $rest
    });
}

sub http_status {
    my ($http, $rest) = @_;
    my $status = $rest;
    $status =~ s{^/}{};
    $status = 500 unless $status =~ /^\d{3}$/;
    my $reason = $http->header('x-reason')//'Unknown';
    my $text = <<EOT;
====================
$status $reason
====================

This response page was generated for HTTP status code $status
by poundharness.
EOT
    $http->reply($status, $reason,
		 headers => {
		     'content-type' => 'text/plain'
		 },
		 body => $text);
}

sub process_http_request {
    my ($sock, $backend) = @_;

    my %endpoints = (
	'echo' => \&http_echo,
	'redirect' => \&http_redirect,
	'status' => \&http_status
    );

    local $| = 1;
    my $http = HTTPServ->new($sock, $backend);
    $http->parse();
    if ($http->uri =~ m{^/([^/]+)(/.*)?}) {
	my ($dir, $rest) = ($1, $2);
	if (my $ep = $endpoints{$dir}) {
	    &{$ep}($http, $2);
	} else {
	    $http->reply(404, "Not found",
			 headers => {
			     'x-orig-uri' => $http->uri,
			     map { ('x-orig-header-' . $_) => $http->header->{$_} } keys %{$http->header}
			 });
	}
    } else {
	$http->reply(500, "Malformed URI: ".$http->uri);
    }
    $http->close;
}

#
# HTTP server
#
package HTTPServ;
use strict;
use warnings;
use Socket qw(:crlf);
use Carp;

sub new {
    my ($class, $fh, $backend) = @_;
    bless { fh => $fh, backend => $backend }, $class;
}

sub backend { shift->{backend} }
sub ident { shift->backend->ident }
sub method { shift->{METHOD} }
sub version { shift->{VERSION} }
sub uri { shift->{URI} }
sub body { shift->{BODY} }
sub header {
    my ($http, $name) = @_;
    if (defined($name)) {
	return $http->{HEADERS}{lc($name)};
    }
    return $http->{HEADERS}
}

sub close {
    my $http = shift;
    close $http->{fh};
}

sub getline {
    my $http = shift;
    local $/ = $CRLF;
    my $fh = $http->{fh};
    my $ret = <$fh>;
    chomp($ret) if $ret;
    return $ret
}

sub ParseRequest {
    my $http = shift;

    my $input = $http->getline() or PoundSub->exit();
    #    print "GOT $input\n";
    my @res = split " ", $input;
    if (@res != 3) {
	croak "Invalid input: $input";
    }

    ($http->{METHOD}, $http->{URI}, $http->{VERSION}) = @res;
}

sub ParseHeader {
    my $http = shift;

    while (my $input = $http->getline()) {
	last if $input eq '';
	#print "INPUT $input\n";
	my ($name, $value) = split ":", $input, 2;
	$value =~ s/^\s+//;
	$http->{HEADERS}{lc($name)} = $value;
    }
    $http->{HEADERS}{'x-backend-ident'} = $http->ident;
}

sub GetBody {
    my $http = shift;
    if ($http->header('Transfer-Encoding')//'' eq 'chunked') {
	my @chunks;
	my $chunk_ext;
	while (my $chunk_size_ext = $http->getline()) {
	    (my $chunk_size, $chunk_ext) = split /;/, $chunk_size_ext, 2;
	    my $len = hex($chunk_size);
	    last if $len == 0;
	    read($http->{fh}, my $chunk, $len);
	    push @chunks, $chunk_size_ext, $chunk;
	    read($http->{fh}, my $s, 2);
	    # FIXME: error checking
	}
	$http->{BODY} = join("\n", @chunks);
    } elsif (my $len = $http->header('Content-Length')) {
	read($http->{fh}, $http->{BODY}, $len);
    } # FIXME: else...
}

sub parse {
    my $http = shift;
    $http->ParseRequest;
    $http->ParseHeader;
    $http->GetBody;
}

sub reply {
    my ($http, $code, $descr, %opt) = @_;
    my $fh = $http->{fh};
    print $fh "$http->{VERSION} $code ${descr}$CRLF";
    if ($opt{headers}) {
	foreach my $h (keys %{$opt{headers}}) {
	    print $fh "$h: ".$opt{headers}{$h}.$CRLF;
	}
    }
    print $fh "connection: close$CRLF";
    print $fh "content-length: ". ($opt{body} ? length($opt{body}) : 0) . $CRLF;
    print $fh $CRLF;
    if ($opt{body}) {
	print $fh $opt{body};
    }
}

package PoundControl;
use strict;
use warnings;
use Carp;
use IPC::Open3;
use IO::Select;
use Symbol 'gensym';
use Data::Dumper;

sub new {
    my ($class, $config) = @_;
    return bless {
	config => $config//'pound.cfg',
    }, $class
}

sub request {
    my ($self, $command, $arg) = @_;

    local $SIG{CHLD} = sub {};
    my ($child_stdin, $child_stdout, $child_stderr);
    $child_stderr = gensym();
    my $pid = open3($child_stdin, $child_stdout, $child_stderr,
		    'poundctl', '-f', $self->{config}, '-j', $command, $arg);
    close $child_stdin;

    my $sel = IO::Select->new();
    $sel->add($child_stdout, $child_stderr);
    my $CHUNK_SIZE = 1000;
    my @ready;
    my %data = ( $child_stdout => '', $child_stderr => '' );

    while (@ready = $sel->can_read) {
	foreach my $fh (@ready) {
	    my $data;
	    while (1) {
		my $len = sysread($fh, $data{$fh}, $CHUNK_SIZE,
				  length($data{$fh}));
		croak "sysread: $!" unless defined($len);
		if ($len == 0) {
		    $sel->remove($fh);
		    $fh->close;
		    last;
		}
	    }
	}
    }

    waitpid($pid, 0);

    my $code;
    if ($? == -1) {
	croak "failed to run poundctl: $!";
    } elsif ($? & 127) {
	croak "poundctl terminated on signal ".($? & 127);
    } elsif ($? >> 8) {
	croak "poundctl failed: " . $data{$child_stderr};
    }
    return JSON->new->boolean_values(0,1)->decode($data{$child_stdout});
}

sub list {
    my $self = shift;
    my $arg;
    if (@_) {
	$arg .= '/' . join('/', @_)
    }
    return $self->request('list', $arg);
}

sub backends {
    my ($self, $lst, $srv, %opts) = @_;
    my $res = $self->list($lst, $srv);
    if ($opts{sort}) {
	$res->{backends} = [sort {
	    if ($a->{weight} != $b->{weight}) {
		$a->{weight} <=> $b->{weight}
	    } elsif ($a->{priority} != $b->{priority}) {
		$a->{priority} <=> $b->{priority}
	    } else {
		($a->{address}//'') cmp ($b->{address}//'')
	    }
	} @{$res->{backends}}];
    }
    return $res->{backends};
}

1;
__END__

=head1 NAME

poundharness - run pound tests

=head1 SYNOPSIS

B<poundharness>
[B<-sv>]
[B<-a> I<IP>]
[B<-f I<FILE>>]
[B<-l I<N>>]
[B<-t I<N>>]
[B<-x I<FILE>>]
[B<--config=>I<SRC>[B<:>I<DST>]]
[B<--fakedns=>[I<PORT>:]I<LIB>]
[B<--include-dir=>I<DIR>]
[B<--log-level=>I<N>]
[B<--preproc=>I<FILE>[:I<DST>]]
[B<--source-address=>I<IP>]
[B<--startup-timeout=>I<N>]
[B<--statistics>]
[B<--test-threads>]
[B<--transcript=>I<FILE>]
[B<--verbose>]
I<SCRIPT>

=head1 DESCRIPTION

Upon startup, B<poundharness> reads B<pound> configuration file F<pound.cfi>,
modifies the settings as described below and writes the resulting configuration
to B<pound.cfg>.  During modification, the following statements are removed:
B<Daemon>, B<LogFacility> and B<LogLevel>.  The settings suitable for running
B<pound> as a subordinate process are inserted instead.  In particular,
B<LogLevel> is set from the value passed with the B<--log-level> command
line option.  For each B<ListenHTTP> and B<ListenHTTPS> section, the actual
socket configuration (i.e. the B<Address>, B<Port>, or B<SocketFrom> statements)
is removed, an IPv4 socket is opened, bound to the arbitrary unused port at
127.0.0.1, and then passed to B<pound> using the B<SocketFrom> statement.
Similar operation is performed on each B<Backend>, except that the created
socket information is stored in the B<Address> and B<Port> statements that
replace the removed ones and a backend HTTP server is started listening
on that socket.

A B<Control> statement is inserted at the beginning of the file, enabling
the control interface via the F<pound.ctl> socket.

B<Include> statements are processed in the following manner: the file
supplied as the argument is preprocessed and the result is written to
a file with the name obtained by appending suffix C<.pha> to the original
name.  An B<Include> statement with this name is output.

If B<Include> argument is a shell globbing pattern, all files matching that
pattern are processed as described.

When a B<Host> statement with exact comparison method is encountered,
a colon and actual port number of the enclosing listener is appended
to its argument.  If the argument is a file name, this operation is performed
on each line of the file and the result is written to the file F<I<file>.port>.
This file name is used in the output.  If B<Host> appears in a top-level
B<Service>, port number of the last declared listener is used instead.
If no listeners are defined (i.e. if the B<Service> statement appears
before the first listener), the statement is output unmodified.  Notice,
that this means that it cannot be matched.

When this configuration processing is finished, B<pound> is started in
foreground mode.  When it is up and ready to serve requests, the I<SCRIPT>
file is opened and processed.  It consists of a series of HTTP requests and
expected responses.  Each request is sent to the specified listener (the one
created in the preprocessing step described above), and the obtained response
is compared with the expectation.  If it matches, the test succeeds.  See
the section B<SCRIPT FILE>, for a detailed discussion of the script file
format.

When all the tests from I<SCRIPT> are run, the program terminates.  Its
status code reflects the results of the run: 0 if all tests succeeded, 1
if some of them failed, 2 if another error occurred and 77 if tests cannot
be run because of missing Perl module.

B<Poundharness> requires Perl 5.22.1 or later and B<IO::FDPass> module.
If these requirements are not met, it exits with code 77.

Status 3 is returned if B<poundharness> is used with wrong command line
switches or arguments,

=head2 Multi-process model

Whenever available, B<poundharness> uses I<perl threads> (I<ithreads>) to run its
subordinate tasks (HTTP and DNS servers, in particular).  If perl is built
without I<ithreads>, the traditional UNIX forking model is used, with each task
served by a separate child process.  To see if threads are available, use

  perl poundharness.pl --test-threads && echo OK

(see the description of B<--test-threads> below).

To force using forking module instead of the default threads, set the
B<POUNDSUB_FORK> environment variable to a non-zero value.

=head1 OPTIONS

=over 4

=item B<-f>, B<--config=> I<SRC>[B<:>I<DST>]

Read source configuration file from I<SRC>, write processed configuration
to I<DST> and use it as configuration file when running B<pound>.  If
I<DST> is omitted, F<pound.cfg> is assumed.  If this option is not given,
I<SRC> defaults to F<pound.cfi>.

=item B<--fakedns=>I<LIB>

Start a mock DNS server listening on arbitrary free ports (UDP and TCP),
initialize environment variables B<FAKEDNS_UDP_PORT> and B<FAKEDNS_TCP_PORT>
to these port numbers, and preload the library I<LIB> before exec'ing
B<pound>.  I<LIB> must be the absolute pathname of the B<libfakedns.so> file.
See B<fakedns.c> for details about this library.

This option also instructs B<poundharness> to emit a B<Resolv> section to
the created B<pound.cfg> file and to extend the script file syntax with
the statements described in the B<DNS Statements> section (see below).

If B<Net::DNS> perl module is not available and B<poundharness>
is given this option, it will exit with status code 77.

To avoid spurious failures, before actually using this functionality
B<poundharness> performs a self-test, consisting in querying a SOA record of
a mock DNS zone, using external helper program B<getsoa>.  If the test
returns the correct data (indicating thereby that the trick with B<fakedns>
works), B<poundharness> continues.  Otherwise, it exists with status code 77.

=item B<--include-dir=>I<DIR>

Look for relative file names in I<DIR>.  In particular, relative file names
given as arguments to the B<--preproc> option or appearing in B<Include>
statements in B<pound> configuration file will be searched in I<DIR>.
This directory is also passed to B<pound> via the B<-Winclude-dir=I<DIR>>
option.

=item B<-l>, B<--log-level=> I<N>

Set B<pound> I<LogLevel> configuration parameter.  If I<N> is B<-1>,
I<LogLevel> set in the configuration file is used.

=item B<--preproc=>I<FILE>[:I<DST>]

Preprocess I<FILE> as described in the B<DESCRIPTION>.  Write the resulting
material to I<DST>.  If the latter is omitted, the destination file will be
named I<FILE>B<.cfg>.

=item B<-a>, B<--source-address=> I<IP>

Register I<IP> as a possible source address for HTTP requests.  Argument
should match C<127.0.0.0/8>.  Registered source addresses can be used in
B<source> statement in the script file.

=item B<-s>, B<--statistics>

Print short statistics at the end of the run.

=item B<--test-threads>

Test if Perl threads are available.  Exit with success if so.  Use

  perl poundharness.pl --test-threads && echo OK

if you wish to get a textual response.

=item B<-t>, B<--startup-timeout=> I<N>

Timeout for B<pound> startup, in seconds.  Default is 2.

=item B<-v>, B<--verbose>

Increase output verbosity.

=item B<-x>, B<--transcript=> I<FILE>

Write test transcript to I<FILE>.

=back

=head1 SCRIPT FILE

B<Poundharness> script file consists of a sequence of send/expect
stanzas.  Empty lines end comments (lines starting with B<#>)
are allowed between them.

Two types of send/expect stanzas are available:

=head2 HTTP send/expect

An HTTP send/expect defines a HTTP requests and expected response.

A request starts with a line specifying the request method in uppercase
(e.g. B<GET>, B<POST>, etc.) followed by an URI.  This line may be followed
by arbitrary number of HTTP headers, newline and request body.  The request
is terminated with the word B<end> or B<endnonl> on a line alone.  If
terminated with B<endnonl>, last newline will not be included in the request
body.

The sequence B<@@> has a special meaning when used in the body.  These
characters instruct B<poundharness> to send the body using B<chunked> transfer
coding and indicate the beginning of each chunk.

An expected response must follow each request.  It begins with a
three digit response code on a line alone.  The code may be followed by
any number of response headers.  If present, the test will succeed only
if the actual response contains all expected headers and their corresponding
values coincide.  Header values are compared literally, except when the
expected header value begins and ends with a slash, in which case regular
expression matching is used.  To start header value with a literal slash,
precede it with a backslash.

Header lines starting with a minus sign denote headers, that must be absent
in the response. E.g. the header line

    -X-Forwarded-Proto: https

means that the response may not contain B<X-Forwarded-Proto: https> header.
The test will fail if such header is present.

Headers may be followed by a newline and response body (content), optionally
preceded by B<:re> or B<:exact> on a separate line.  The B<:re> line instructs
the program to treat the following text as a multiline regular expression.
B<:exact> means use literal match.  This is also the default.

The response is terminated with the word B<end> or B<endnonl> on a line
alone (the semantics of the latter is described above).

Values of both request and expected headers may contain the following
I<variables>, which are expanded when reading the file:

=over 4

=item B<${LISTENERI<n>}>

Expands to the full address of the I<n>th listener (I<IP>:I<PORT>).

=item B<${LISTENERI<n>:IP}>

Expands to the IP address of the I<n>th listener.

=item B<${LISTENERI<n>:PORT}>

Expands to the port number of the I<n>th listener.

=item B<${LISTENER}>, B<${LISTENER:IP}>, B<${LISTENER:PORT}>

Same as B<${LISTENER0}>, B<${LISTENER0:IP}>, and B<${LISTENER0:PORT}>,
correspondingly.

=item B<${BACKENDI<n>}>

Expands to the address of the I<n>th backend (I<IP>:I<PORT>).

=item B<${BACKENDI<n>:IP}>

Expands to the IP address of the I<n>th backend.

=item B<${BACKENDI<n>:PORT}>

Expands to the port number of the I<n>th backend.

=item B<${BACKEND}>, B<${BACKEND:IP}>, B<${BACKEND:PORT}>

Same as B<${BACKEND0}>, B<${BACKEND0:IP}>, and B<${BACKEND0:PORT}>,
correspondingly.

=back

Numbering of listeners and backends starts from 0.

By default, requests sent by B<poundharness.pl> originate from 127.0.0.1.
Another source address can be set using the B<source> statement:

  source IP

The IP must have been previously declared as a source using the
B<--source-address> command line option.  Obviously, the networking
configuration of the host machine must allow it to be used as a
source IP, otherwise B<poundharness.pl> issue a diagnostic message and
exit with code 77.

The B<server> statement can be used to specify the B<pound> listener to
send the requests to.  It has the form

    server N

where N is the ordinal 0-based number of the B<ListenHTTP> statement in
the configuration file.  The B<server> statement affects all requests that
follow it up to the next B<server> statement or end of file, whichever
occurs first.

=head2 External program send/expect

This type of stanza allow you to run an external program and examine its
exit code, standard output and error streams.  It is included mainly for
testing the B<poundctl> command.

The stanza begins with the keyword B<run> followed by the command
and its argument.  It can be followed by one or more of expect statements:

=over 4

=item B<status> I<N>

Expect program to return exit status I<N> (a decimal number).

=item B<stdout>

Expect text on stdout.  Everything below this keyword and up to the
B<end> keyword appearing on a line alone is taken to be the expected
text.  When matching actual program output, this text is treated as
Perl multi-line regular expression (see the B<m> and B<s> flags in
B<perlre>).  To expect a line containing the word C<end> alone, prefix
it with a backslash.

=item B<stderr>

Expect text on stderr.  See the description of B<stdout> above for
its syntax.

=item B<logtail> "I<expect>" [I<T>]
Wait for a line matching I<expect> to appear in the pound log file.  The
optional argument I<T> sets time to wait, in seconds.  The default is 2
seconds.  The test will fail if no matching string appears in the log within
that time.

=back

=head2 Backend Usage Analysis

The statement

=over 4

=item B<stats> B<samples>=I<N> I<K>=I<V>...

=back

causes the following send/expect statements to be executed I<N> times in
turn.  After this, backend usage statistics will be computed and the
resulting values compared with the expected values supplied by I<K>=I<V>
pairs.  Allowed values for I<K> are:

=over 4

=item B<samples>

Number of samples to run.

=item B<min>

Minimum number of requests served by a backend.

=item B<max>

Maximum number of requests served by a backend.

=item B<avg>

Average number of requests served by a backend.

=item B<stddev>

Standard deviation of the number of requests served by a backend.

=item B<index>

Apply the above values to this backend (0-based index).

=back

=head2 Querying the Control Interface

The following statement query the B<pound> control interface and verify
if the response matches the expectation.  The latter is supplied after the
statement, in JSON format, and ends with the keyword B<end> on a line by
itself.  Only attributes present in the expectation are compared.

=over 4

=item backends I<LN> I<SN>

Query for backends in service I<SN> of listener I<LN>.
The returned backend array is sorted by weight, priority and address.

=back

=head2 Additional Directives

=over 4

=item B<mkbackend> I<IP>

Declares new backend with the given IP address.  I<IP> must fall within
127.0.0.0/8.

Normally backends are created automatically and this statement is not needed.
Use it for testing dynamic backends.

=item B<sleep> I<N>

Pause execution for I<N> seconds.

=item B<echo> I<WORD> [I<WORD>...]

Expand variables (see B<HTTP send/expect> above) in I<WORD>s and output
the result on standard error, prefixed with the B<#> sign.

=head2 DNS Statements

The following keywords are designed for testing dynamic DNS-based backends.
They become available if the following two conditions are met:

=over 4

=item *

The module B<Net::DNS> is available.

=item *

Full pathname of the B<libfakedns.so> library is given using the
B<--fakedns> option.

=back

When these conditions are met, B<poundharness> will start a subsidiary DNS
server and modify the produced B<pound.cfg> file to use it.  The following
extended keywords can then be used:

=over 4

=item zonefile

This statement defines the DNS zone (or zones) served by the subsidiary
DNS.  The zone definition follows the keyword and ends with the keyword
B<end> on a line by itself.  Zone file syntax is defined by RFC1035.

=back

=head1 BACKENDS

Each B<Backend> statement in the configuration file causes creation of
a HTTP server behind it.  These built-in servers currently two endpoints:

=head2 /echo

Any requests with URLs under that endpoint are replied with exact copy of the
incoming requests, with the addition of the following headers:

=over 4

=item B<x-backend-number>

Ordinal number of the backend in configuration (0-based).

=item B<x-backend-ident>

Identifier of the backend, in form B<I<FILE>:I<LINE>>, where I<FILE> is the
source configuration file and I<LINE> is the number of line in it where the
corresponding B<Backend> keyword is located.

=item B<x-orig-uri>

Copy of the original URI.

=item B<x-orig-header->I<header>

The value of the header I<header> in the request.

=back

If the body was sent using B<chunked> encoding, it is reproduced
verbatim, instead of reconstructing it as per RFC 9112, 7.1.3.

=head2 /redirect

Redirects the request to the B</echo> endpoint.  The value of the
B<x-redirect> header, if any, is prepended to the value of the B<Location>
header.  E.g. the following request

    GET /redirect/foo?bar=0 HTTP/1.1
    X-Redirect: https://example.org

will get the following response:

    HTTP/1.1 301 Moved permanently
    Location: https://example.org/echo/foo?bar=0

This backend is used to test the B<RewriteLocation> functionality.

=head2 /status/CODE

Returns HTTP status I<CODE>.  The reason text may be supplied in
the B<X-Reason> header.

=head1 FILES

=over 4

=item pound.cfi

Source configuration file.  Can be changed using the B<--config> option.

=item pound.cfg

Destination configuration file.  The modified configuration is written to
it and it is then passed to B<pound> as its configuration file.

Can be changed using the B<--config> option.

=item pound.log

Log output from running instance of B<pound> is written to this file.

=item pound.pid

Keeps PID of the running B<pound> instance.

=item lstI<N>.sock

Temporary UNIX sockets for passing created socket descriptors to B<pound>.
These are removed on success.

=item I<FILE>.pha

Preprocessed output of the include file I<FILE>.

=item I<FILE>.port

Preprocessed output of the I<FILE> from B<Host -file "I<FILE>"> statement.

=back

=head1 SEE ALSO

B<pound> (8)

=head1 AUTHOR

Sergey Poznyakoff <gray@gnu.org>

=head1 LICENSE

GPLv3+: GNU GPL version 3 or later, see L<http://gnu.org/licenses/gpl.html>

This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

=cut
